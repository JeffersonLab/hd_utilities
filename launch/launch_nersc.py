#!/usr/bin/env python
#
# Submit jobs to run at NERSC via swif2
#
# This will run commands submitting several recon jobs
# with the run/file numbers hardcoded into this script.
# Here is how this is supposed to work:
#
# This will run swif2 with all of the SBATCH(slurm) options
# passed via command line. This includes the shifter image
# that should be used. swif2 will then take care of getting
# the file from tape and transferring it to NERSC. Once
# the file is there, it will submit the job to Cori.
#
# When the job wakes up, it will be in a subdirectory of the
# NERSC project directory that swif2 has already setup.
# This directory will contain a symbolic link pointing
# to the raw data file which is somewhere else in the project
# directory tree.
#
# The container will run the /launch/script_nersc.sh script
# where /launch has been mounted in the container from the
# "launch" directory in the project directory. The jana
# config file is also kept in the launch directory.
#
# The container will also have /cvmfs mounted. The standard
# gluex containers have links built in so that /group will
# point to the appropriate subdirectory of /cvmfs making the
# the GlueX software available. The script_nersc.sh script
# will use this to setup the environment and then run hd_root
# using the /launch/jana_recon.config file.
#
# A couple of more notes:
#
# 1. The CCDB and RCDB used comes from an sqlite file in
# CVMFS. These are copied to the local node in /tmp at
# the beginning of the job and deleted at the end. The
# timestamp used is built into the /launch/jana_recon.config
#
# 2. NERSC requires that the program being run is actually
# a script that starts with #!/XXX/YYY . Thus, the command
# we give to swif2 to run for the job is:
#
#       /launch/jana_recon_nersc.config
#
# which is a simple wrapper script to run the
# /launch/script_nersc.sh script using shifter
#
# 3. The output directory is created here
# to allow group writing since the files are copied using
# the davidl account on globus but swif2 is being run from
# the gxproj4 account.
#

import subprocess
import sys
import os

# mysql.connector not available via system and must come via PYTHONPATH
if not os.getenv('PYTHONPATH') : sys.path.append('/group/halld/Software/builds/Linux_CentOS7-x86_64-gcc4.8.5/ccdb/ccdb_1.06.06/python')
import mysql.connector


TESTMODE  = True  # True=only print commands, but don't actually submit jobs


WORKFLOW     = 'nersc_test_03'
NAME         = 'GLUEX_OFFMON'
PROJECT      = 'm3120'
TIMELIMIT    = '4:45:00'  # Set time limit (~3.25hr for recon. ~4.5hr for monitoring)
QOS          = 'regular'  # debug, regular, premium
NODETYPE     = 'haswell'  # haswell, knl  (quad,cache)

IMAGE        = 'docker:markito3/gluex_docker_devel'
#RECONVERSION = 'sim-recon/sim-recon-recon-2017_01-ver03'
RECONVERSION = 'halld_recon/halld_recon-recon-2017_01-ver03.1'
SCRIPTFILE   = '/launch/script_nersc.sh'
CONFIG       = '/launch/jana_offmon_nersc.config'
OUTPUTTOP    = 'mss:/mss/halld/halld-scratch/RunPeriod-2018-01/offmon/verN00'  # prefix with mss: for tape or file: for filesystem

RUNPERIOD = 'RunPeriod-2018-01'
RCDB_QUERY = '@is_2018production and @status_approved'  # Comment out for all runs in range MINRUN-MAXRUN
RUNS      = []      # List of runs to process. If empty, MINRUN-MAXRUN are searched in RCDB
MINRUN    = 40949   # If RUNS is empty, then RCDB queried for this range
MAXRUN    = 49999   # If RUNS is empty, then RCDB queried for this range
MINFILENO = 0       # Min file number to process for each run (n.b. file numbers start at 0!)
MAXFILENO = 100     # Max file number to process for each run (n.b. file numbers start at 0!)

RCDB_HOST = 'hallddb.jlab.org'
RCDB_USER = 'rcdb'
RCDB      = None

#----------------------------------------------------
def MakeJob(RUN,FILE):
	JOB_STR   = '%s_%06d_%03d' % (NAME, RUN, FILE)
	EVIOFILE  = 'hd_rawdata_%06d_%03d.evio' % (RUN, FILE)
	MSSFILE   = '/mss/halld/%s/rawdata/Run%06d/%s' % (RUNPERIOD, RUN, EVIOFILE)
	
	# The OUTPUTDIR variable is a fully qualified path used to pre-create
	# the output directories for the job files. If the files are going to
	# /mss, then we must replace the mss:/mss part at the beginning with
	# /lustre/expphy/cache. Otherwise, just use the path as given in OUTPUTTOP
	OUTPUTDIR = OUTPUTTOP.split(':',1)[1]  # just directory part
	if OUTPUTTOP.startswith('mss:/mss'): OUTPUTDIR = OUTPUTDIR.replace('/mss','/lustre/expphy/cache')
	
	# Get list of output directories and files.
	# Normally, we wouldn't have to make the directories, but if using
	# a Globus account with a different user than the one running swif2,
	# the directories must be premade with appropriate permissions.
	# The outfiles variable is a map of local file(key) to output file(value)
	if 'recon' in CONFIG:
		(outdirs, outfiles) = ReconOutFiles(RUN, FILE)
	elif 'offmon' in CONFIG:
		(outdirs, outfiles) = OffmonOutFiles(RUN, FILE)
	else:
		print 'Unknown config type! Unable to form output file list'

	# SLURM options
	SBATCH  = ['-sbatch']
	SBATCH += ['-A', PROJECT]
	SBATCH += ['--volume="/global/project/projectdirs/%s/launch:/launch"' % PROJECT]
	SBATCH += ['--image=%s' % IMAGE]
	SBATCH += ['--time=%s' % TIMELIMIT]
	SBATCH += ['--nodes=1']
	SBATCH += ['--tasks-per-node=1']
	SBATCH += ['--cpus-per-task=64']
	SBATCH += ['--qos=regular']
	SBATCH += ['-C', NODETYPE]
	SBATCH += ['-L', 'project']

	# Command for job to run
	CMD  = ['/global/project/projectdirs/%s/launch/run_shifter.sh' % PROJECT]
	CMD += ['--module=cvmfs']
	CMD += ['--']
	CMD += [SCRIPTFILE]
	CMD += [CONFIG]           # arg 1:  JANA config file
	CMD += [RECONVERSION]     # arg 2:  sim-recon version
	CMD += [str(RUN)]         # arg 3:  run     <--+ run and file number used to name job_info
	CMD += [str(FILE)]        # arg 4:  file    <--+ directory only.

	# Make swif2 command
	SWIF2_CMD  = ['swif2']
	SWIF2_CMD += ['add-job']
	SWIF2_CMD += ['-workflow', WORKFLOW]
	SWIF2_CMD += ['-name', JOB_STR]
	SWIF2_CMD += ['-input', EVIOFILE, 'mss:'+MSSFILE]
	for src,dest in outfiles.iteritems(): SWIF2_CMD += ['-output', src, OUTPUTTOP + '/' + dest]
	SWIF2_CMD += SBATCH + ['::'] + CMD

	# Print commands
	if OUTPUTTOP.startswith('file:') :
		for d in outdirs: print 'mkdir -p ' + OUTPUTDIR + '/' + d
		print 'chmod -R 777 ' + OUTPUTDIR
	print ' '.join(SWIF2_CMD)
	
	if not TESTMODE:
		if OUTPUTTOP.startswith('file:') :
			for d in outdirs: subprocess.check_call(['mkdir', '-p', OUTPUTDIR + '/' + d])
			subprocess.check_call(['chmod', '-R', '777', OUTPUTDIR])
		subprocess.check_call(SWIF2_CMD)

#----------------------------------------------------
def OffmonOutFiles(RUN, FILE):

	# Return list of output directories and file mappings for a
	# offline monitoring job.

	# List of output directories.
	outdirs = []
	outdirs += ['job_info']
	outdirs += ['dana_rest_coherent_peak/%06d' % RUN]
	outdirs += ['REST/%06d' % RUN]
	outdirs += ['omega/%06d' % RUN]
	outdirs += ['hists/%06d' % RUN]
	outdirs += ['p3pi_excl_skim/%06d' % RUN]
	outdirs += ['tree_bcal_hadronic_eff/%06d' % RUN]
	outdirs += ['tree_PSFlux/%06d' % RUN]
	outdirs += ['tree_trackeff/%06d' % RUN]
	outdirs += ['TOF_TDC_shift/%06d' % RUN]
	outdirs += ['tofcalib/%06d' % RUN]
	outdirs += ['random/%06d' % RUN]
	outdirs += ['BCAL-LED/%06d' % RUN]
	outdirs += ['FCAL-LED/%06d' % RUN]
	outdirs += ['sync/%06d' % RUN]
	outdirs += ['TPOL/%06d' % RUN]

	# Map of local file(key) to output file(value)
	RFSTR = '%06d_%03d' % (RUN, FILE)
	outfiles = {}
	outfiles['job_info_%s.tgz'  % RFSTR               ] = 'job_info/%06d/job_info_%s.tgz' % (RUN, RFSTR)
	outfiles['dana_rest_coherent_peak.hddm'           ] = 'dana_rest_coherent_peak/%06d/dana_rest_coherent_peak_%s.hddm' % (RUN, RFSTR)
	outfiles['dana_rest.hddm'                         ] = 'REST/%06d/dana_rest_%s.hddm' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.omega.evio' % RFSTR       ] = 'omega/%06d/omega_%s.evio' % (RUN, RFSTR)
	outfiles['hd_root.root'                           ] = 'hists/%06d/hd_root_%s.root' % (RUN, RFSTR)
	outfiles['p3pi_excl_skim.root'                    ] = 'p3pi_excl_skim/%06d/p3pi_excl_skim_%s.root' % (RUN, RFSTR)
	outfiles['tree_bcal_hadronic_eff.root'            ] = 'tree_bcal_hadronic_eff/%06d/tree_bcal_hadronic_eff_%s.root' % (RUN, RFSTR)
	outfiles['tree_PSFlux.root'                       ] = 'tree_PSFlux/%06d/tree_PSFlux_%s.root' % (RUN, RFSTR)
	outfiles['tree_trackeff.root'                     ] = 'tree_trackeff/%06d/tree_trackeff_%s.root' % (RUN, RFSTR)
	outfiles['TOF_TDC_shift_%06d.txt' % RUN           ] = 'TOF_TDC_shift/%06d/TOF_TDC_shift_%s.txt' % (RUN, RFSTR)
	outfiles['hd_root_tofcalib.root'                  ] = 'tofcalib/%06d/hd_root_tofcalib_%s.root' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.random.evio' % RFSTR      ] = 'random/%06d/hd_rawdata_%s.random.evio' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.BCAL-LED.evio' % RFSTR    ] = 'BCAL-LED/%06d/hd_rawdata_%s.BCAL-LED.evio' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.FCAL-LED.evio' % RFSTR    ] = 'FCAL-LED/%06d/hd_rawdata_%s.FCAL-LED.evio' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.sync.evio' % RFSTR        ] = 'sync/%06d/hd_rawdata_%s.sync' % (RUN, RFSTR)
	outfiles['tree_TPOL.root'                         ] = 'TPOL/%06d/tree_TPOL_%s.root' % (RUN, RFSTR)

	return (outdirs, outfiles)

#----------------------------------------------------
def ReconOutFiles(RUN, FILE):

	# Return list of output directories and file mappings for a
	# reconstruction job.

	# List of output directories.
	outdirs = []
	outdirs += ['job_info']
	outdirs += ['dana_rest_coherent_peak/%06d' % RUN]
	outdirs += ['REST/%06d' % RUN]
	outdirs += ['exclusivepi0/%06d' % RUN]
	outdirs += ['omega/%06d' % RUN]
	outdirs += ['hists/%06d' % RUN]
	outdirs += ['p3pi_excl_skim/%06d' % RUN]
	outdirs += ['tree_bcal_hadronic_eff/%06d' % RUN]
	outdirs += ['tree_fcal_hadronic_eff/%06d' % RUN]
	outdirs += ['tree_PSFlux/%06d' % RUN]
	outdirs += ['tree_sc_eff/%06d' % RUN]
	outdirs += ['tree_tof_eff/%06d' % RUN]
	outdirs += ['tree_trackeff/%06d' % RUN]
	outdirs += ['tree_TS_scaler/%06d' % RUN]

	# Map of local file(key) to output file(value)
	RFSTR = '%06d_%03d' % (RUN, FILE)
	outfiles = {}
	outfiles['job_info_%s.tgz'  % RFSTR               ] = 'job_info/%06d/job_info_%s.tgz' % (RUN, RFSTR)
	outfiles['dana_rest_coherent_peak.hddm'           ] = 'dana_rest_coherent_peak/%06d/dana_rest_coherent_peak_%s.hddm' % (RUN, RFSTR)
	outfiles['dana_rest.hddm'                         ] = 'REST/%06d/dana_rest_%s.hddm' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.exclusivepi0.evio' % RFSTR] = 'exclusivepi0/%06d/exclusivepi0_%s.evio' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.omega.evio' % RFSTR       ] = 'omega/%06d/omega_%s.evio' % (RUN, RFSTR)
	outfiles['hd_root.root'                           ] = 'hists/%06d/hd_root_%s.root' % (RUN, RFSTR)
	outfiles['p3pi_excl_skim.root'                    ] = 'p3pi_excl_skim/%06d/p3pi_excl_skim_%s.root' % (RUN, RFSTR)
	outfiles['tree_bcal_hadronic_eff.root'            ] = 'tree_bcal_hadronic_eff/%06d/tree_bcal_hadronic_eff_%s.root' % (RUN, RFSTR)
	outfiles['tree_fcal_hadronic_eff.root'            ] = 'tree_fcal_hadronic_eff/%06d/tree_fcal_hadronic_eff_%s.root' % (RUN, RFSTR)
	outfiles['tree_PSFlux.root'                       ] = 'tree_PSFlux/%06d/tree_PSFlux_%s.root' % (RUN, RFSTR)
	outfiles['tree_sc_eff.root'                       ] = 'tree_sc_eff/%06d/tree_sc_eff_%s.root' % (RUN, RFSTR)
	outfiles['tree_tof_eff.root'                      ] = 'tree_tof_eff/%06d/tree_tof_eff_%s.root' % (RUN, RFSTR)
	outfiles['tree_trackeff.root'                     ] = 'tree_trackeff/%06d/tree_trackeff_%s.root' % (RUN, RFSTR)
	outfiles['tree_TS_scaler.root'                    ] = 'tree_TS_scaler/%06d/tree_TS_scaler_%s.root' % (RUN, RFSTR)
	outfiles['hd_rawdata_%s.cal_high_energy_skim.evio' % RFSTR] = 'cal_high_energy_skim/%06d/cal_high_energy_skim_%s.evio'% (RUN, RFSTR)

	return (outdirs, outfiles)

#----------------------------------------------------
def GetRunInfo():

	# Get the list of runs to process and the number of EVIO files for each.
	# The list is returned in the form of a dictionary with the run numbers
	# as keys and the maximum evio file number for that run as values.
	# Which runs show up in the list depends on how the RUNS and RCDB_QUERY
	# globals are set:
	#
	# RUNS is not None: All runs in the list are included
	# RUNS is empty and RCDB_QUERY is None: All runs in the range MINRUN-MAXRUN inclusive are included
	# RUNS is empty and RCDB_QUERY is not None: RCDB is queried for the list of runs.
	#
	# n.b. that for the first 2 options above, the GetNumEVIOFiles routine
	# below is called which queries the RCDB via mysql directly so the RCDB
	# python module does not actually need to be in PYTHONPATH. For the 3rd
	# option, the RCDB python API is used so it is needed.

	global RUNS, MINRUN, MAXRUN, RCDB_QUERY

	# Query through RCDB API
	if len(RUNS)==0 and RCDB_QUERY!=None:
		print 'Querying RCDB for run list ....'
		import rcdb
		db = rcdb.RCDBProvider('mysql://' + RCDB_USER + '@' + RCDB_HOST + '/rcdb')
		good_runs = {}
		print 'RCDB_QUERY = ' + RCDB_QUERY
		for r in db.select_runs(RCDB_QUERY, MINRUN, MAXRUN):
			good_runs[r.number] = int(r.get_condition_value('evio_files_count'))
	elif len(RUNS)==0 :
		print 'Getting info for all runs in range ' + str(MINRUN) + '-' + str(MAXRUN) + ' ....'
		for RUN in range(MINRUN, MAXRUN+1): good_runs[RUN] = GetNumEVIOFiles(RUN)
	else:
		print 'Getting info for runs : ' + ' '.join([str(x) for x in RUNS])
		for RUN in RUNS: good_runs[RUN] = GetNumEVIOFiles(RUN)

	return good_runs

#----------------------------------------------------
def GetNumEVIOFiles(RUN):

	# Access RCDB to get the number of EVIO files for this run.
	# n.b. the file numbers start from 0 so the last valid file
	# number will be one less than the value returned
	global RCDB
	if not RCDB :
		try:
			RCDB = 'mysql://' + RCDB_USER + '@' + RCDB_HOST + '/rcdb'
			cnx = mysql.connector.connect(user=RCDB_USER, host=RCDB_HOST, database='rcdb')
			cur = cnx.cursor()  # using dictionary=True crashes when running on ifarm (??)
		except Exception as e:
			print 'Error connecting to RCDB: ' + RCDB
			print str(e)
			sys.exit(-1)

	Nfiles = 0
	sql  = 'SELECT int_value from conditions,condition_types WHERE condition_type_id=condition_types.id'
	sql += ' AND condition_types.name="evio_files_count" AND run_number=' + str(RUN);
	cur.execute(sql)
	c_rows = cur.fetchall()
	if len(c_rows)>0 : Nfiles = int(c_rows[0][0])

	return Nfiles

#----------------------------------------------------

# --------------- MAIN --------------------

# Get list of runs with number of evio files for each.
# (parameters for search set at top of file)
good_runs = GetRunInfo()

# Loop over runs
for (RUN, Nfiles) in good_runs.iteritems:

	# Limit max file number to how many there are for this run according to RCDB
	maxfile = MAXFILENO+1
	if Nfiles < maxfile : maxfile = Nfiles
	
	# Loop over files, creating job for each
	for FILE in range(MINFILENO, maxfile):
		MakeJob(RUN, FILE)



