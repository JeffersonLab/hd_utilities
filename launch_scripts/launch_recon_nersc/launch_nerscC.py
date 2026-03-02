#!/usr/bin/env python3
#
# Submit multi-file jobs to run at NERSC Cori II via swif2
#
# See more detailed documentation here:
#  https://halldweb.jlab.org/wiki/index.php/HOWTO_Execute_a_Launch_using_NERSC
#
# This will run commands submitting several recon jobs
# with the run/file numbers hardcoded into this script.
# Here is how this is supposed to work:
#
# launch_nerscB.py
#    |
#    |-> swif2 add-job (this will eventually run sbatch at cori ...)
#         |
#         |-> script_nersc_multi.sh
#               |
#               |-> script_nersc_multi.py
#                     |
#                     |-> run_shifter_multi.sh
#                           |
#                           |-> shifter
#                                |
#                                |-> script_nersc.sh (this is run from inside shifter)
#                                     |
#                                     |-> hd_root
#
#
# This will run swif2 with *some* of the SBATCH(slurm) options
# passed via command line. This includes the singularity image
# that should be used. swif2 will then take care of getting
# the file from tape and transferring it to PSC. Once
# the file is there, it will submit the job to Bridges slurm.
#
# When the job wakes up, it will be in a subdirectory of the
# NERSC scratch disk that swif2 has already setup.
# This directory will contain a symbolic link pointing
# to the raw data file which is somewhere else on the scratch
# disk.
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
# using the /launch/jana_recon_nersc.config file.
#
# A couple of more notes:
#
# 1. The CCDB and RCDB used comes from an sqlite file in
# CVMFS. These are copied to the local node in /tmp at
# the beginning of the job and deleted at the end. The
# timestamp used is hardcoded in /launch/jana_recon_nersc.config
#
# 2. NERSC requires that the program being run is actually
# a script that starts with #!/XXX/YYY . It is actually a
# SLURM script where additional SLURM options could be set.
# We do not put them there though. All SLURM options are
# passed via the sbatch command swif2 runs and that we specify
# here. The /launch/script_nersc.sh script is trivial and only
# runs shifter passing any arguments we give to it here in
# the swif2 command.
#
# 3. The output directory is created by this script
# to allow group writing since the files are copied using
# the davidl account on globus but swif2 is being run from
# the gxproj4 account.
#

import glob
import math
import mysql.connector
import os
import subprocess
import sys

import rcdb


TESTMODE = True  # True=only print commands, but don't actually submit jobs
VERBOSE  = 3     # 1 is default

RUNPERIOD  = '2022-05'
LAUNCHTYPE = 'recon'  # 'offmon' or 'recon'
VER        = '02-perl'
BATCH      = 'NERSC-multi'
WORKFLOW   = f'{LAUNCHTYPE}_{RUNPERIOD}_ver{VER}_{BATCH}'
NAME       = f'GLUEX_{LAUNCHTYPE}'

RCDB_QUERY    = "@is_cpp_production and @status_approved"  # Comment out for all runs in range MINRUN-MAXRUN
RUNS          = []      # List of runs to process. If empty, MINRUN-MAXRUN are searched in RCDB
EXCLUDE_RUNS  = []      # Runs that should be excluded from processing
MINRUN        = 100000  # If RUNS is empty, then RCDB queried for this range
MAXRUN        = 109999  # If RUNS is empty, then RCDB queried for this range
MINFILENO     = 0       # Min file number to process for each run (n.b. file numbers start at 0!)
MAXFILENO     = 999     # Max file number to process for each run (n.b. file numbers start at 0!)
FILE_FRACTION = 1.0     # Fraction of files to process for each run in specified range (see GetFileNumbersToProcess)

MAX_CONCURRENT_JOBS = '100'      # Maximum number of jobs swif2 will have in flight at once
PROJECT             = 'm3120'    # run "projects" command on bridges and look for "Charge ID"
QOS                 = 'regular'  # debug, regular, premium, low, flex, scavenger
NODETYPE            = 'cpu'      # cpu, gpu
TIMELIMIT           = '5:00:00'  # time limit for full file jobs on KNL

IMAGE        = 'docker:jeffersonlab/gluex_almalinux_9:latest'
RECONVERSION = 'halld_recon/halld_recon-5.8.1'  # must exist in /group/halld/Software/builds/Linux_CentOS7-x86_64-gcc4.8.5-cntr
LAUNCHDIR    = f'/global/cfs/cdirs/{PROJECT}/launch-{VER}'  # will get mapped to /launch in singularity container
MASTERSCRIPT = 'script_nersc_multi.sh'     # top-level script run by slurm job (outside container). Should be in LAUNCHDIR
SCRIPTFILE   = f'/launch-{VER}/script_nerscc.sh'  # script run inside container
CONFIG       = f'/launch-{VER}/jana_{LAUNCHTYPE}_nersc.config'

RCDB_HOST = 'hallddb.jlab.org'
RCDB_USER = 'rcdb'
RCDB      = None
BAD_RCDB_QUERY_RUNS = []  # will be filled with runs that are missing evio_file_count field in RCDB query
BAD_FILE_COUNT_RUNS = []  # will be filled with runs where number of evio files could not be obtained by any method
BAD_MSS_FILE_RUNS   = {}  # will be filled with runs/files where the stub file in /mss is missing

# Set output directory depending on launch type
# We set two variables here whose purposes are:
#    OUTPUTDIR - Temporary staging directory for output files copied from offsite
#    OUTPUTMSS - Ultimate destination on tape
# An external (cron??) job needs to be run to move them to tape.
OUTPUTDIR = f'/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-{RUNPERIOD}/{LAUNCHTYPE}/ver{VER}'
OUTPUTMSS = f'mss:/mss/halld/halld-scratch/RunPeriod-{RUNPERIOD}/recon/ver{VER}'


#----------------------------------------------------
# MakeJob
#
# Make a single job with enough tasks(=nodes) to process
# all of the files in the given list.
#
# TODO: This takes RUN as an argument and assumes all files
# are for that given run. The scripts downstream of this
# have some ability to handle jobs containing files from
# multiple runs. This script, however, will need to be modified
# to support those types of jobs.
def MakeJob(RUN,files_to_process):

        global NUM, DIRS_CREATED

        # Make list of EVIO filenames in /mss to process in this job
        mss_files = {}
        outdirs   = {}
        for FILE in files_to_process:
                JOB_STR   = '%s_%06d_%03d' % (NAME, RUN, FILE)
                EVIOFILE  = 'hd_rawdata_%06d_%03d.evio' % (RUN, FILE)
                MSSFILE   = '/mss/halld/RunPeriod-%s/rawdata/Run%06d/%s' % (RUNPERIOD, RUN, EVIOFILE)

                # Verify stub file exists before submitting job
                if not os.path.exists( MSSFILE ):
                        if RUN not in BAD_MSS_FILE_RUNS.keys(): BAD_MSS_FILE_RUNS[RUN] = []
                        BAD_MSS_FILE_RUNS[RUN].append(FILE)
                else:
                        mss_files[EVIOFILE] = MSSFILE
                        outpath = 'RUN%06d/FILE%03d' % (RUN, FILE)
                        outdirs[EVIOFILE] = outpath

        # Make sure we have at least one raw data file to process
        if len(mss_files) == 0: return

        NUM['files_submitted'] += len(mss_files)

        # The OUTPUTDIR variable is a fully qualified path where the output
        # files copied back to JLab are placed. The files will later be moved
        # to tape by some external process (likely a cron job).
        #
        # A dedicated output directory is pre-created for each raw data file.
        # We do this here as opposed to letting swif2 do it so we avoid permission
        # errors if the Globus user is different from the one running swif2.
#       outdirs = []
#       for FILE in files_to_process:
#               outpath = 'RUN%06d/FILE%03d' % (RUN, FILE)
#               outdirs.append(outpath)

        # Pare down list of outdirs to only those that don't already exist
        new_outdirs = [x for x in list(outdirs.values()) if x not in DIRS_CREATED]

        # Set umask to make directories group writable (but not world writable)
        os.umask(0o002)

        # Create output directories at JLab
        for d in sorted(new_outdirs):
                mydir = OUTPUTDIR + '/' + d
                if not os.path.exists(mydir) :
                        if VERBOSE > 1: print('mkdir -p ' + mydir)
                        if not TESTMODE:
                                os.makedirs(mydir)
                                DIRS_CREATED.append(mydir)

        # SLURM options
        SBATCH  = ['-sbatch']
        SBATCH += ['-A', PROJECT]
        SBATCH += ['--volume="%s:/launch-%s"' % (LAUNCHDIR,VER)]
        SBATCH += ['--image=%s' % IMAGE]
        SBATCH += ['--module=cvmfs']
        SBATCH += ['--time=%s' % TIMELIMIT]
        SBATCH += ['-N', str(len(files_to_process))] # Number of nodes requested (1 per raw data file)
        SBATCH += ['--tasks-per-node=1']
        SBATCH += ['--cpus-per-task=256']  # I believe this is ignored since we always get whole nodes
        SBATCH += ['--qos='+QOS]
        SBATCH += ['-C', NODETYPE]

        # Command for job to run
        CMD  = ['%s/%s' % (LAUNCHDIR, MASTERSCRIPT)]  # Initial script on first node that runs srun to run jobs on all nodes
        CMD += [LAUNCHDIR]      # arg 1:  launch directory on host (will be mapped to /launch in image)
        CMD += [SCRIPTFILE]     # arg 2:  script to run inside container
        CMD += [CONFIG]         # arg 3:  JANA config file
        CMD += [RECONVERSION]   # arg 4:  sim-recon version
                                # n.b. RUN and FILE are not passed here. MASTERSCRIPT figures them out for the tasks based on the evio files it finds

        # Make swif2 command
        SWIF2_CMD  = ['swif2']
        SWIF2_CMD += ['add-job']
        SWIF2_CMD += ['-workflow', WORKFLOW]
        SWIF2_CMD += ['-name', '%s_%06d' % (NAME, RUN)]

        for EVIOFILE in sorted(list(mss_files.keys())):
                MSSFILE = mss_files[EVIOFILE]
                outdir  = outdirs[EVIOFILE]
                SWIF2_CMD += ['-input', EVIOFILE, 'mss:'+MSSFILE]
                SWIF2_CMD += ['-output', 'match:'+outdir+'/*', OUTPUTDIR + '/' + outdir]
        SWIF2_CMD += SBATCH + ['::'] + CMD

        # Print commands
        if VERBOSE > 1:
                if VERBOSE > 2 : print (' '.join(SWIF2_CMD))
                elif VERBOSE > 1 : print (' --- Job will be created for run:' + str(RUN) + ' file:' + str(FILE))

        NUM['jobs_to_process'] += 1

        if not TESTMODE:
                subprocess.check_call(SWIF2_CMD)
                NUM['jobs_submitted'] += 1


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

        global RUNS, MINRUN, MAXRUN, RCDB_QUERY, RUN_LIST_SOURCE, BAD_RCDB_QUERY_RUNS, BAD_FILE_COUNT_RUNS
        good_runs = {}

        # If RCDB_QUERY is not defined, define with value None
        try: RCDB_QUERY
        except : RCDB_QUERY = None

        # Query through RCDB API
        if len(RUNS)==0 and RCDB_QUERY!=None:
                RUN_LIST_SOURCE = 'RCDB ' + str(MINRUN) + '-' + str(MAXRUN) + ' (query="' + RCDB_QUERY + '")'
                print ('Querying RCDB for run list ....')

                # Import RCDB python module. Add a path on the CUE just in case
                # PYTHONPATH is not already set
                sys.path.append('/group/halld/Software/builds/Linux_Alma9-x86_64-gcc11.5.0-cntr/rcdb/rcdb_0.09.01/python')
                db = rcdb.RCDBProvider('mysql://' + RCDB_USER + '@' + RCDB_HOST + '/rcdb2')
                print ('RCDB_QUERY = ' + RCDB_QUERY)
                for r in db.select_runs(RCDB_QUERY, MINRUN, MAXRUN):
                        evio_files_count = r.get_condition_value('evio_files_count')
                        if evio_files_count == None:
                                print('ERROR in RCDB: Run ' + str(r.number) + ' has no value for evio_files_count!...')
                                BAD_RCDB_QUERY_RUNS.append( int(r.number) )
                                print('Attempting to extract number of files by examining /mss ...')
                                rawdatafiles = glob.glob('/mss/halld/RunPeriod-'+RUNPERIOD+'/rawdata/Run%06d/hd_rawdata_%06d_*.evio' % (r.number,r.number))
                                if len(rawdatafiles) > 0: evio_files_count = len(rawdatafiles)
                        if evio_files_count == None:
                                print('ERROR getting number of files for: Run ' + str(r.number) )
                                BAD_FILE_COUNT_RUNS.append( int(r.number) )
                                continue
                        good_runs[r.number] = int(evio_files_count)
        elif len(RUNS)==0:
                RUN_LIST_SOURCE = 'All runs in range ' + str(MINRUN) + '-' + str(MAXRUN)
                print ('Getting info for all runs in range ' + str(MINRUN) + '-' + str(MAXRUN) + ' ....')
                for RUN in range(MINRUN, MAXRUN+1): good_runs[RUN] = GetNumEVIOFiles(RUN)
        else:
                RUN_LIST_SOURCE = 'Custom list: ' + ' '.join([str(x) for x in RUNS])
                print ('Getting info for runs : ' + ' '.join([str(x) for x in RUNS]))
                for RUN in RUNS: good_runs[RUN] = GetNumEVIOFiles(RUN)

        # Filter out runs in the EXCLUDE_RUNS list
        global EXCLUDE_RUNS
        good_runs_filtered = {}
        for run in good_runs.keys():
                if run not in EXCLUDE_RUNS: good_runs_filtered[run] = good_runs[run]

        return good_runs_filtered


#----------------------------------------------------
def GetNumEVIOFiles(RUN):

        global BAD_RCDB_QUERY_RUNS, BAD_FILE_COUNT_RUNS

        # Access RCDB to get the number of EVIO files for this run.
        # n.b. the file numbers start from 0 so the last valid file
        # number will be one less than the value returned
        global RCDB, cnx, cur
        if not RCDB :
                try:
                        RCDB = 'mysql://' + RCDB_USER + '@' + RCDB_HOST + '/rcdb'
                        cnx = mysql.connector.connect(user=RCDB_USER, host=RCDB_HOST, database='rcdb')
                        cur = cnx.cursor()  # using dictionary=True crashes when running on ifarm (??)
                except Exception as e:
                        print (f'Error connecting to RCDB: {RCDB}')
                        print (str(e))
                        sys.exit(-1)

        Nfiles = 0
        sql  = 'SELECT int_value from conditions,condition_types WHERE condition_type_id=condition_types.id'
        sql += ' AND condition_types.name="evio_files_count" AND run_number=' + str(RUN);
        cur.execute(sql)
        c_rows = cur.fetchall()
        if len(c_rows)>0 :
                Nfiles = int(c_rows[0][0])
        else:
                BAD_RCDB_QUERY_RUNS.append(RUN)
                print('Attempting to extract number of files by examining /mss ...')
                rawdatafiles = glob.glob('/mss/halld/RunPeriod-'+RUNPERIOD+'/rawdata/Run%06d/hd_rawdata_%06d_*.evio' % (RUN,RUN))
                if len(rawdatafiles) > 0:
                        Nfiles = len(rawdatafiles)
                else:
                        BAD_FILE_COUNT_RUNS.append(RUN)


        return Nfiles


#----------------------------------------------------
def GetFileNumbersToProcess(Nfiles):

        # This will return a list of file numbers to process for a run
        # given the number of files given by Nfiles. The list is determined
        # by the values MINFILENO, MAXFILENO and FILE_FRACTION.
        # First, the actual range of file numbers for the run is determined
        # by MINFILENO-MAXFILENO, but clipped if necessary to Nfiles.
        # Next, a list of files in that range representing FILE_FRACTION
        # of the range is formed and returned.
        #
        # Example 1: Process first 10 files of each run:
        #             MINFILENO = 0
        #             MAXFILENO = 9
        #         FILE_FRACTION = 1.0
        #
        # Example 2: Process first 5% of files of each run of
        #            files distributed throughout run:
        #             MINFILENO = 0
        #             MAXFILENO = 1000    n.b. set this to something really big
        #         FILE_FRACTION = 0.05
        #

        global MINFILENO, MAXFILENO, FILE_FRACTION

        # Make sure MINFILENO is not greater than max file number for the run
        if MINFILENO >= Nfiles : return []

        # Limit max file number to how many there are for this run according to RCDB
        maxfile = MAXFILENO+1  # set to 1 past the actual last file number we want to process
        if Nfiles < maxfile : maxfile = Nfiles

        # If FILE_FRACTION is 1.0 then we want all files in the range.
        if FILE_FRACTION == 1.0: return range( MINFILENO, maxfile)

        # At this point, maxfile should be one greater than the last file
        # number we want to process. If the last file we want to process
        # is the last file in the run, then it could be a short file. Thus,
        # use the next to the last file in the run to determine the range.
        if Nfiles < MAXFILENO : maxfile -= 1

        # Number of files in run to process
        Nrange = float(maxfile-1) - float(MINFILENO)
        N = math.ceil(FILE_FRACTION * Nrange)
        if N<2 : return [MINFILENO]
        nskip = Nrange/(N-1)
        filenos = []
        for i in range(0, int(N)): filenos.append(int(i*nskip))
#       print ('Nrange=%f N=%f nskip=%f ' % (Nrange, N, nskip))

        return filenos


#----------------------------------------------------
def PrintConfigSummary():
        print ('=================================================')
        print ('Launch Summary  ' + ('**** TEST MODE ****' if TESTMODE else ''))
        print ('-----------------------------------------------')
        print ('             RunPeriod: ' + RUNPERIOD)
        print ('           Launch type: ' + LAUNCHTYPE)
        print ('               Version: ' + VER)
        print ('                 batch: ' + BATCH)
        print ('              WORKFLOW: ' + WORKFLOW)
        print ('    Origin of run list: ' + RUN_LIST_SOURCE)
        print ('        Number of runs: ' + str(len(good_runs)))
        print ('       Number of files: ' + str(NUM['files_to_process']) + ' (maximum ' + str(MAXFILENO-MINFILENO+1) + ' files/run)')
        print ('         Min. file no.: ' + str(MINFILENO))
        print ('         Max. file no.: ' + str(MAXFILENO))
        print ('                   QOS: ' + QOS)
        print ('    Time limit per job: ' + TIMELIMIT)
        print ('      JANA config file: ' + CONFIG)
        print ('         NERSC project: ' + PROJECT)
        print ('         Shifter image: ' + IMAGE)
        print ('   halld_recon version: ' + RECONVERSION + ' (from CVMFS)')
        print ('      launch directory: ' + LAUNCHDIR + ' (at NERSC)')
        print ('      Output directory: ' + OUTPUTDIR)
        print ('=================================================')


#----------------------------------------------------
if __name__ == "__main__":
        # Initialize some counters
        NUM = {}
        NUM['files_to_process'] = 0
        NUM['files_submitted'] = 0
        NUM['jobs_to_process'] = 0
        NUM['jobs_submitted'] = 0

        # Get list of runs with number of evio files for each.
        # (parameters for search set at top of file)
        good_runs = GetRunInfo()
        #good_runs = db.select_runs(RCDB_QUERY, MINRUN, MAXRUN)

        # Print some info before doing anything
        for n in [x for (y,x) in good_runs.items()]:
                NUM['files_to_process'] += len(GetFileNumbersToProcess(n))

        if VERBOSE > 0: PrintConfigSummary()

        # Create workflow
        cmd =  ['swif2', 'create', '-workflow', WORKFLOW]
        cmd += ['-site', 'nersc/perlmutter']
        #cmd += ['-site', 'nersc/perlmutter', '-site-storage', 'nersc:'+PROJECT]
        cmd += ['-max-concurrent', MAX_CONCURRENT_JOBS]
        if VERBOSE>0 : print ('Workflow creation command: ' + ' '.join(cmd))
        if TESTMODE:
                print ('(TEST MODE so command will not be run)')
        else:
                (cmd_out, cmd_err) = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
                if VERBOSE>0:
                        if len(cmd_err)>0 :
                                if VERBOSE>1 : print (cmd_err)
                                print ('Command returned error message. Assuming workflow already exists')
                        else:
                                print (cmd_out)

        # Run workflow
        cmd =  ['swif2', 'run', '-workflow', WORKFLOW]
        if VERBOSE>0 : print ('Command to start workflow: ' + ' '.join(cmd))
        if TESTMODE:
                print ('(TEST MODE so command will not be run)')
        else:
                (cmd_out, cmd_err) = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
                if VERBOSE>0:
                        print (cmd_out)
                        print (cmd_err)

        # Loop over runs
        DIRS_CREATED = []   # keeps track of local directories we create so we don't create them twice
        if VERBOSE>0 :
                print ('Submitting jobs ....')
                print ('-----------------------------------------------')
        for (RUN, Nfiles) in good_runs.items():

                # Get list of files to process
                files_to_process = GetFileNumbersToProcess( Nfiles )

                # Loop over files, creating job for each
                MakeJob(RUN, files_to_process)
        #       for FILE in files_to_process:
        #               MakeJob(RUN, FILE)
        #               if VERBOSE>0:
        #                       sys.stdout.write('  ' + str(NUM['files_submitted']) + '/' + str(NUM['files_to_process']) + ' jobs \r')
        #                       sys.stdout.flush()

        print('\n')
        print('NOTE: The values in BAD_RCDB_QUERY_RUNS is informative about what is missing from')
        print('      the RCDB. An attempt to recover the information from the /mss filesystem')
        print('      was also made. Values in BAD_FILE_COUNT_RUNS are ones for which that failed.')
        print('      Thus, only runs listed in BAD_FILE_COUNT_RUNS will not have any jobs submitted')
        print ('BAD_RCDB_QUERY_RUNS=' + str(BAD_RCDB_QUERY_RUNS))
        print ('BAD_FILE_COUNT_RUNS=' + str(BAD_FILE_COUNT_RUNS))
        print ('BAD_MSS_FILE_RUNS='   + str(BAD_MSS_FILE_RUNS))

        # If more than 5 jobs were submitted then the summary printed above probably
        # rolled off of the screen. Print it again.
        if (VERBOSE > 0) : PrintConfigSummary()

        NUM['missing_mss_files'] = 0
        for run,files in BAD_MSS_FILE_RUNS.items(): NUM['missing_mss_files'] += len(files)

        print('')
        print('WORKFLOW: ' + WORKFLOW)
        print('------------------------------------')
        print('Number of runs: ' + str(len(good_runs)) + '  (only good runs)')
        print(str(NUM['files_submitted']) + '/' + str(NUM['files_to_process']) + ' total files submitted  (' + str(NUM['missing_mss_files']) + ' files missing from mss)')
        print(str(NUM['jobs_submitted']) + '/' + str(NUM['jobs_to_process']) + ' total jobs submitted')
        print(str(len(DIRS_CREATED)) + ' directories created for output')
        print('')
