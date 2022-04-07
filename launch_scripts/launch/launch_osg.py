#!/usr/bin/env python

# Paul Mattione
# Built off of hdswif by Kei Moriya
#
# SWIF DOCUMENTATION:
# https://scicomp.jlab.org/docs/swif
# https://scicomp.jlab.org/docs/swif-cli

from optparse import OptionParser
import os.path
import rcdb
import sys
import time
import glob
import re
import time
from subprocess import Popen, PIPE

VERBOSE = False

####################################################### TRY COMMAND ######################################################

def try_command(command, sleeptime = 5):
	# Try an os command and if the exit code is non-zero then return an error
	return_code = -999
	while return_code != 0:
		process = Popen(command.split(), stdout=PIPE)
		output = process.communicate()[0] # is stdout. [1] is stderr
		print output
		return_code = process.returncode

		if return_code == 0:
			break #successful: leave

		# sleep for a few seconds between tries
		print 'sleeping for ' + str(sleeptime) + ' sec...'
		time.sleep(sleeptime)

####################################################### READ CONFIG ######################################################

def read_config(CONFIG_FILENAME):
	# Read in user config file
	config_dict = {}

	infile_config = open(CONFIG_FILENAME, 'r')
	for line in infile_config:

		# Ignore empty lines, and lines that begin with #
		if (len(line.split()) == 0) or (line.split()[0][0] == '#'):
			continue

		# Add new key/value pair into config_dict
		key = str(line.split()[0])
		value = line.split()[1]
		# if is string in quotes (e.g. RCDB query) continue reading until end-quote
		read_index = 2
		while(value[0] == "\"") and (value[-1:] != "\""):
			value += " " + line.split()[read_index]
			read_index += 1
		# now, strip leading and tail quotation marks (if present)
		if read_index > 2:
			value = value[1:-1]
		config_dict[key] = value
		if(VERBOSE == True):
			print "Job Config key, value = " + key + " " + value

	# Some of the keys may depend on other configuration parameters, so update the values
	# containing [key] within the values corresponding to those keys.
	#
	# Example:
	# OUTPUT_TOPDIR /volatile/halld/test/RunPeriod-[RUNPERIOD]/ver[VERSION]
	# depends on other config parameters RUNPERIOD and VERSION
	# 
	# NOTE: The method assumes there are no circular dependencies
	  
	# Iterate over key/value pairs in dictionary. If we find a replacement, we need to start over.
	# The parameter found keeps track of whether we found a replacement or not.
	found = 1
	while(found):
		for key, value in config_dict.items():
			found = 0

			# For each one see if any values contain [P] where P is a different value
			for other_key, other_value in config_dict.items():
				if str(value).find(str('[' + other_key + ']')) != -1:

					# Found replacement
					found = 1
					new_value = value.replace(str('[' + other_key + ']'),other_value)

					# Replace new key/value pair into config_dict
					new_pair = {key : new_value}
					config_dict.update(new_pair)
					del new_pair

					# Break out of loop over other_key, other_value
					break

			# Break out of loop over key, value, to restart loop again
			if found == 1:
				break

		# If we do not find a replacement we will finish the loop

	return config_dict

##################################################### VALIDATE CONFIG ####################################################

def validate_config(config_dict):

	# JOB RESOURCES
	if("NCORES" not in config_dict) or ("DISK" not in config_dict) or ("RAM" not in config_dict) or ("TIMELIMIT" not in config_dict):
		print "ERROR: JOB RESOURCES NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING"
		sys.exit(1)

	# WORKFLOW DEFINITION
	if("WORKFLOW" not in config_dict):
		print "ERROR: WORKFLOW DEFINITION NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING"
		sys.exit(1)

	# JOB, SCRIPT CONTROL
	if("ENVFILE" not in config_dict) or ("SCRIPTFILE" not in config_dict):
		print "ERROR: JOB, SCRIPT CONTROL NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING"
		sys.exit(1)

	# FILE INPUT, OUTPUT BASE DIRECTORIES
	if("INDATA_TOPDIR" not in config_dict) or ("OUTDIR_LARGE" not in config_dict) or ("OUTDIR_SMALL" not in config_dict):
		print "ERROR: FILE INPUT, OUTPUT BASE DIRECTORIES NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING"
		sys.exit(1)

####################################################### FIND FILES #######################################################

def find_files(INDATA_DIR, FORMATTED_RUN, FORMATTED_FILE):
	# Note: This won't work if the file names don't contain ".", or if there are directories that DO contain "."

	# If need specific file #
	if(FORMATTED_FILE != "*"):
		pathstring = INDATA_DIR + '/*' + FORMATTED_RUN + '*_*' + FORMATTED_FILE + '*.*'
		return glob.glob(pathstring)

	# Else just require run # in name
	pathstring = INDATA_DIR + '/*' + FORMATTED_RUN + '*.*'
	return glob.glob(pathstring)

######################################################## ADD JOB #########################################################

def find_num_threads(JANA_CONFIG_FILENAME):
	num_threads = "1"

	# Read in JANA config file
	infile_config = open(JANA_CONFIG_FILENAME, 'r')
	for line in infile_config:

		# Ignore empty lines, and lines that begin with #
		if (len(line.split()) == 0) or (line.split()[0][0] == '#'):
			continue

		# Save #-threads if correct key
		key = str(line.split()[0])
		if (key != "NTHREADS"):
			continue
		num_threads = line.split()[1]
		break;

	return num_threads


def add_job(WORKFLOW, FILEPATH, config_dict):

	# EXTRACT PATH, RUNNO, & FILE #: ASSUME THE FORM IS EITHER */*_RUNNO_FILENO.* OR */*_RUNNO.*
	match = re.search(r"(.*)/(.*)_(\d\d\d\d\d\d)_(\d\d\d).(.*)", FILEPATH)
	if(match is not None):
		INDATA_DIR = match.group(1)
		PREFIX = match.group(2)
		RUNNO = match.group(3)
		FILENO = match.group(4)
		EXTENSION = match.group(5)
	else: # Try with no file #
		match = re.search(r"(.*)/(.*)_(\d\d\d\d\d\d).(.*)", FILEPATH)
		if(match is None):
			print "WARNING: FILE " + FILEPATH + " DOESN'T MATCH EXPECTED NAME FORMAT. SKIPPING."
			return
		INDATA_DIR = match.group(1)
		PREFIX = match.group(2)
		RUNNO = match.group(3)
		FILENO = "-1"
		EXTENSION = match.group(4)
	if(VERBOSE == True):
		print "FILEPATH, COMPONENTS: " + FILEPATH + " " + INDATA_DIR + " " + PREFIX + " " + RUNNO + " " + FILENO + " " + EXTENSION

	# PREPARE NAMES
	#DATE = time.strftime("%Y-%m-%d")
	#STUBNAME = RUNNO if(FILENO == "-1") else RUNNO + "_" + FILENO
	#FILENAME = PREFIX + "_" + RUNNO + "." + EXTENSION if(FILENO == "-1") else PREFIX + "_" + RUNNO + "_" + FILENO + "." + EXTENSION
	#if(WORKFLOW.find("ver") == -1):
	#	JOBNAME = WORKFLOW + "_" + STUBNAME + "_" + DATE
	#else:
	#	JOBNAME = WORKFLOW + "_" + STUBNAME

	# SETUP OTHER VARIABLES:
	#INPUTDATA_TYPE = "mss" if(INDATA_DIR[:5] == "/mss/") else "file"
	JANA_CONFIG = config_dict["JANA_CONFIG"] if ("JANA_CONFIG" in config_dict) else "NA"
        JANA_CONFIG_LOCAL = JANA_CONFIG.split('/')[-1]
        if len(JANA_CONFIG_LOCAL) == 0:
                JANA_CONFIG_LOCAL = "NA.config"
	NUM_THREADS = find_num_threads(JANA_CONFIG) if ("JANA_CONFIG" in config_dict) else "1"

        ENVFILE = config_dict["ENVFILE"] if ("ENVFILE" in config_dict) else "NA"
        SCRIPTFILE = config_dict["SCRIPTFILE"] if ("SCRIPTFILE" in config_dict) else "NA"
        SCRIPTFILE_LOCAL = SCRIPTFILE.split('/')[-1]
        if len(SCRIPTFILE_LOCAL) == 0:
                SCRIPTFILE_LOCAL = "NA.sh"
        OSGWRAPPER = config_dict["OSGWRAPPER"] if ("OSGWRAPPER" in config_dict) else "NA"

        # SETUP LOG DIRECTORY FOR SLURM
        if(FILENO != "-1"):
                LOG_DIR = config_dict["OUTDIR_SMALL"] + "/log/" + RUNNO
	else:
                LOG_DIR = config_dict["OUTDIR_SMALL"] + "/log"
        make_log_dir = "mkdir -p " + LOG_DIR
        try_command(make_log_dir)
        if(VERBOSE == True):
                print "LOG DIRECTORY " + LOG_DIR + " CREATED"

# arg 1:  JANA config file
# arg 2:  halld_recon version
# arg 3:  run period
# arg 4:  run period version
# arg 5:  run
# arg 6:  file

        # make job submission
        outf_name = "job_%s_%s.sub"%(RUNNO,FILENO)
        with open(outf_name, "w") as outf:
                # general job settings
                outf.write("Executable = %s\n"%OSGWRAPPER)
                outf.write("Arguments  = ./%s %s %s %s %s %s %s\n"%(SCRIPTFILE_LOCAL,JANA_CONFIG_LOCAL,ENVFILE,config_dict["RUNPERIOD"],config_dict["RUNPERIOD_VER"],RUNNO,FILENO))
                outf.write("Error      = %s/error.log\n"%LOG_DIR)
                outf.write("Output     = %s/out.log\n"%LOG_DIR)
                outf.write("Log        = %s/sub.log\n"%LOG_DIR)
                outf.write("RequestCpus = 1\n\n")   # NTHREADS - assume 1 for now

                # required settings
                outf.write("+Project = \"gluex\"\n")
                outf.write("Requirements = (HAS_SINGULARITY == TRUE) && (HAS_CVMFS_oasis_opensciencegrid_org == True) && (HAS_CVMFS_gluex_osgstorage_org == true)\n")
                outf.write("+SingularityImage = \"/cvmfs/singularity.opensciencegrid.org/markito3/gluex_docker_prod:latest\"\n")
                outf.write("+SingularityBindCVMFS = True\n")
                outf.write("+SingularityAutoLoad = True\n\n")

                # file input/output 
                outf.write("should_transfer_files = YES\n")
                outf.write("when_to_transfer_output = ON_EXIT\n")
                outf.write("concurrency_limits = GluexProduction\n")
                outf.write("on_exit_remove = true\n")
                outf.write("on_exit_hold = false\n")
                outf.write("transfer_input_files = %s,%s\n"%(SCRIPTFILE,JANA_CONFIG))
                outf.write("transfer_output_files = out_%s_%s\n\n"%(RUNNO,FILENO))

                outf.write("Queue 1\n")
         
        # submit job
        submit_cmd = "condor_submit -batch-name %s %s"%(WORKFLOW,outf_name)
        try_command(submit_cmd)  # check result of command?

        # delete submit file
        rm_cmd = "rm %s"%outf_name
        try_command(rm_cmd)


def add_fullrun_job(WORKFLOW, file_list, config_dict):

        fileno_list = []

        for FILEPATH in file_list:

                # EXTRACT PATH, RUNNO, & FILE #: ASSUME THE FORM IS EITHER */*_RUNNO_FILENO.* OR */*_RUNNO.*
                match = re.search(r"(.*)/(.*)_(\d\d\d\d\d\d)_(\d\d\d).(.*)", FILEPATH)
                if(match is not None):
                        INDATA_DIR = match.group(1)
                        PREFIX = match.group(2)
                        RUNNO = match.group(3)
                        FILENO = match.group(4)
                        EXTENSION = match.group(5)

                        fileno_list.append(FILENO)
                else: 
                        print "WARNING: FILE " + FILEPATH + " DOESN'T MATCH EXPECTED NAME FORMAT. SKIPPING."
                        return
                if(VERBOSE == True):
                        print "FILEPATH, COMPONENTS: " + FILEPATH + " " + INDATA_DIR + " " + PREFIX + " " + RUNNO + " " + FILENO + " " + EXTENSION

	# PREPARE NAMES
	#DATE = time.strftime("%Y-%m-%d")
	#STUBNAME = RUNNO if(FILENO == "-1") else RUNNO + "_" + FILENO
	#FILENAME = PREFIX + "_" + RUNNO + "." + EXTENSION if(FILENO == "-1") else PREFIX + "_" + RUNNO + "_" + FILENO + "." + EXTENSION
	#if(WORKFLOW.find("ver") == -1):
	#	JOBNAME = WORKFLOW + "_" + STUBNAME + "_" + DATE
	#else:
	#	JOBNAME = WORKFLOW + "_" + STUBNAME

	# SETUP OTHER VARIABLES:
	#INPUTDATA_TYPE = "mss" if(INDATA_DIR[:5] == "/mss/") else "file"
	JANA_CONFIG = config_dict["JANA_CONFIG"] if ("JANA_CONFIG" in config_dict) else "NA"
        JANA_CONFIG_LOCAL = JANA_CONFIG.split('/')[-1]
        if len(JANA_CONFIG_LOCAL) == 0:
                JANA_CONFIG_LOCAL = "NA.config"
	NUM_THREADS = find_num_threads(JANA_CONFIG) if ("JANA_CONFIG" in config_dict) else "1"

        ENVFILE = config_dict["ENVFILE"] if ("ENVFILE" in config_dict) else "NA"
        SCRIPTFILE = config_dict["SCRIPTFILE"] if ("SCRIPTFILE" in config_dict) else "NA"
        SCRIPTFILE_LOCAL = SCRIPTFILE.split('/')[-1]
        if len(SCRIPTFILE_LOCAL) == 0:
                SCRIPTFILE_LOCAL = "NA.sh"
        OSGWRAPPER = config_dict["OSGWRAPPER"] if ("OSGWRAPPER" in config_dict) else "NA"

        # SETUP LOG DIRECTORY FOR SLURM
        if(FILENO != "-1"):
                LOG_DIR = config_dict["OUTDIR_SMALL"] + "/log/" + RUNNO
	else:
                LOG_DIR = config_dict["OUTDIR_SMALL"] + "/log"
        make_log_dir = "mkdir -p " + LOG_DIR
        try_command(make_log_dir)
        if(VERBOSE == True):
                print "LOG DIRECTORY " + LOG_DIR + " CREATED"

# arg 1:  JANA config file
# arg 2:  halld_recon version
# arg 3:  run period
# arg 4:  run period version
# arg 5:  run
# arg 6:  file

        # save list of file numbers
        with open("filenos.txt","w") as filenof:
                for fileno in sorted(fileno_list):
                        filenof.write("%s\n"%fileno)

        # make job submission
        outf_name = "job_%s.sub"%(RUNNO)
        with open(outf_name, "w") as outf:
                # general job settings
                outf.write("Executable = %s\n"%OSGWRAPPER)
                outf.write("Arguments  = ./%s %s %s %s %s %s %s\n"%(SCRIPTFILE_LOCAL,JANA_CONFIG_LOCAL,ENVFILE,config_dict["RUNPERIOD"],config_dict["RUNPERIOD_VER"],RUNNO,"$(fileno)"))
                outf.write("Error      = %s/error_$(fileno).log\n"%LOG_DIR)
                outf.write("Output     = %s/out_$(fileno).log\n"%LOG_DIR)
                outf.write("Log        = %s/sub_$(fileno).log\n"%LOG_DIR)
                outf.write("RequestCpus = 1\n\n")   # NTHREADS - assume 1 for now
                outf.write("RequestMemory = %s\n\n"%config_dict["RAM"])

                # required settings
                outf.write("+Project = \"gluex\"\n")
                outf.write("Requirements = (HAS_SINGULARITY == TRUE) && (HAS_CVMFS_oasis_opensciencegrid_org == True)\n")
                outf.write("+SingularityImage = \"/cvmfs/singularity.opensciencegrid.org/markito3/gluex_docker_prod:latest\"\n")
                outf.write("+SingularityBindCVMFS = True\n")
                outf.write("+SingularityAutoLoad = True\n\n")

                # file input/output 
                outf.write("should_transfer_files = YES\n")
                outf.write("when_to_transfer_output = ON_EXIT\n")
                outf.write("concurrency_limits = GluexProduction\n")
                outf.write("on_exit_remove = true\n")
                outf.write("on_exit_hold = false\n")
                outf.write("transfer_input_files = %s,%s\n"%(SCRIPTFILE,JANA_CONFIG))
                outf.write("transfer_output_files = out_%s_$(fileno)\n\n"%(RUNNO))

                outf.write("Queue fileno from filenos.txt\n")
      
        # submit job
        submit_cmd = "condor_submit -batch-name %s %s"%(WORKFLOW,outf_name)
        try_command(submit_cmd)  # check result of command?

        # delete submit file
        rm_cmd = "rm %s"%outf_name
        try_command(rm_cmd)



########################################################## MAIN ##########################################################

def main(argv):
	global VERBOSE # so can modify here

	# PARSER
	parser_usage = "launch_osg.py job_configfile minrun maxrun\n\n"
	parser_usage += "optional: -f file_num: file_num must be 3 digits, with leading 0's if necessary)\n"
	parser_usage += "          but, it can be a search string for glob (e.g. first 5 files: -f '00[0-4]' (MUST include quotes!))\n\n"
	parser_usage += "optional: -v True: verbose output\n\n"
	parser_usage += "optional: -j True: perform one condor_submit for each file instead of per run - this will be slow!\n\n"
	parser = OptionParser(usage = parser_usage)

	# PARSER OPTIONS
	parser.add_option("-f", "--file", dest="file", help="specify file(s) to run over")
	parser.add_option("-v", "--verbose", dest="verbose", help="verbose output")
	parser.add_option("-j", "--one-job-per-file", dest="file_submit", help="one submit command per file")

	# GET ARGUMENTS
	(options, args) = parser.parse_args(argv)
	if(len(args) < 3):
		parser.print_help()
		return

	# SET INPUT VARIABLES
	JOB_CONFIG_FILE = args[0]
	MINRUN = int(args[1])
	MAXRUN = int(args[2])
	VERBOSE = True if(options.verbose) else False
	INPUT_FILE_NUM = options.file if(options.file) else "*" #must be three digits, with leading 0's if necessary

	# READ CONFIG
	config_dict = read_config(JOB_CONFIG_FILE)
	validate_config(config_dict)

	# SET CONTROL VARIABLES
	WORKFLOW = config_dict["WORKFLOW"]
	RCDB_QUERY = config_dict["RCDB_QUERY"] if ("RCDB_QUERY" in config_dict) else ""
	INDATA_TOPDIR = config_dict["INDATA_TOPDIR"] if ("INDATA_TOPDIR" in config_dict) else ""

	# GET THE LIST OF GOOD RUNS
	db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb")
	good_runs = []
	if(VERBOSE == True):
		print "RCDB_QUERY = " + RCDB_QUERY
	if(RCDB_QUERY != ""):
		good_runs = db.select_runs(RCDB_QUERY, MINRUN, MAXRUN)
	if(VERBOSE == True):
		print str(len(good_runs)) + " good runs in range: " + str(MINRUN) + " - " + str(MAXRUN)

	# FIND & ADD JOBS
	for RUN in range(MINRUN, MAXRUN + 1):

		# See if is good run
		rcdb_run_info = db.get_run(int(RUN))
		if(RCDB_QUERY != "") and (rcdb_run_info not in good_runs):
			continue

		# Format run number
		FORMATTED_RUN = "%06d" % RUN

		# Find files for run number: First try separate folder for each run
		INDATA_DIR = INDATA_TOPDIR + "/*" + FORMATTED_RUN + "*/"
		file_list = find_files(INDATA_DIR, FORMATTED_RUN, INPUT_FILE_NUM)
		if(len(file_list) == 0): # No files. Now just try the input dir
			INDATA_DIR = INDATA_TOPDIR + "/"
			file_list = find_files(INDATA_DIR, FORMATTED_RUN, INPUT_FILE_NUM)

		if(VERBOSE == True):
			print str(len(file_list)) + " files found for run " + str(RUN)

		# Add jobs to workflow
                if options.file_submit:
                        # this will be slow!
                        for FILEPATH in file_list:
                                add_job(WORKFLOW, FILEPATH, config_dict)
                else:
                        # run some condor magic to speed things up
                        add_fullrun_job(WORKFLOW, file_list, config_dict)


if __name__ == "__main__":
   main(sys.argv[1:])

