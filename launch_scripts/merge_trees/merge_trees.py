#!/usr/bin/env python

##########################################################################################################################
#
# 2017/08/07 Alex Austregesilo
#
# Python script for merging analysis trees as soon as all jobs for the run are done
# 2022/01/27: upgrade to swif2
#
##########################################################################################################################

import sys
import os
import glob
import time
from optparse import OptionParser
from subprocess import Popen, PIPE
from collections import defaultdict, deque

#################################################### GLOBAL VARIABLES ####################################################

VERBOSE = 0

####################################################### TRY COMMAND ######################################################

def try_command(command, sleeptime = 5):
	# Try an os command and if the exit code is non-zero then return an error
	return_code = -999
	while return_code != 0:
		process = Popen(command.split(), stdout=PIPE)
		output = process.communicate()[0].decode('ASCII') # is stdout. [1] is stderr
		print(output)
		return_code = process.returncode

		if return_code == 0:
			break #successful: leave

		# sleep for a few seconds between tries
		print('sleeping for ' + str(sleeptime) + ' sec...')
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
		if(VERBOSE > 0):
			print("Job Config key, value = " + key + " " + value)

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

	# JOB ACCOUNTING
	if("PROJECT" not in config_dict) or ("TRACK" not in config_dict) or ("OS" not in config_dict):
		print("ERROR: JOB ACCOUNTING NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING")
		sys.exit(1)

	# JOB RESOURCES
	if("NCORES" not in config_dict) or ("DISK" not in config_dict) or ("RAM" not in config_dict) or ("TIMELIMIT" not in config_dict):
		print("ERROR: JOB RESOURCES NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING")
		sys.exit(1)

	# WORKFLOW DEFINITION
	if("WORKFLOW" not in config_dict):
		print("ERROR: WORKFLOW DEFINITION NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING")
		sys.exit(1)

	# JOB, SCRIPT CONTROL
	if("ENVFILE" not in config_dict) or ("SCRIPTFILE" not in config_dict):
		print("ERROR: JOB, SCRIPT CONTROL NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING")
		sys.exit(1)

	# FILE INPUT, OUTPUT BASE DIRECTORIES
	if("INDATA_TOPDIR" not in config_dict) or ("OUTDIR_LARGE" not in config_dict) or ("OUTDIR_SMALL" not in config_dict):
		print("ERROR: FILE INPUT, OUTPUT BASE DIRECTORIES NOT FULLY SPECIFIED IN CONFIG FILE. ABORTING")
		sys.exit(1)

	# CHECK FILE EXISTENCE
	if(not os.path.isfile(config_dict["ENVFILE"])): #Check if ENVFILE exists
		# Also accept ENVFILE if it is an xml file found in standard group disk location
		if(not os.path.isfile("/group/halld/www/halldweb/html/halld_versions/"+config_dict["ENVFILE"])): #
			print("ERROR: ENVFILE does not exist (or is inaccessible) \n ENVFILE: " + config_dict["ENVFILE"])
			sys.exit(1)
	if(not os.path.isfile(config_dict["SCRIPTFILE"])):
		print("ERROR: SCRIPTFILE does not exist (or is inaccessible) \n SCRIPTFILE: " + config_dict["SCRIPTFILE"])
		sys.exit(1)

	# CHECK INPUT FOLDER EXISTENCE
	if(not os.path.isdir(config_dict["INDATA_TOPDIR"])):
		print("ERROR: INDATA_TOPDIR does not exist! \n INDATA_TOPDIR: " + config_dict["INDATA_TOPDIR"])
		sys.exit(1)

	# CHECK OUTPUT (LARGE) FOLDER EXISTENCE
	if(not os.path.isdir(config_dict["OUTDIR_LARGE"])):
		# First try to create folder if it does not exist
		NEW_DIR = str(config_dict["OUTDIR_LARGE"])
		make_large_dir = "mkdir -p " + NEW_DIR
		try_command(make_large_dir)
		if(VERBOSE == True):
			print("OUTDIR_LARGE " + make_large_dir + " CREATED")
	# If creating OUTDIR_LARGE unsuccessful, we should exit
	if(not os.path.isdir(config_dict["OUTDIR_LARGE"])):
		print("ERROR: OUTDIR_LARGE does not exist and could not be created \n OUTDIR_LARGE: " + config_dict["OUTDIR_LARGE"])
		sys.exit(1)

	# CHECK OUTPUT (SMALL) FOLDER EXISTENCE
	if(not os.path.isdir(config_dict["OUTDIR_SMALL"])):
		# First try to create folder if it does not exist
		LOG_DIR = config_dict["OUTDIR_SMALL"] + "/log"
		make_log_dir = "mkdir -p " + LOG_DIR
		try_command(make_log_dir)
		if(VERBOSE == True):
			print("LOG DIRECTORY " + LOG_DIR + " CREATED")
	# If creating OUTDIR_SMALL unsuccessful, we should exit
	if(not os.path.isdir(config_dict["OUTDIR_SMALL"])):
		print("ERROR: OUTDIR_SMALL does not exist and could not be created \n OUTDIR_SMALL: " + config_dict["OUTDIR_SMALL"])
		sys.exit(1)

################################################### BUILD DICTIONARIES ###################################################

def build_launch_dictionary(WORKFLOW):
	command = "/usr/local/bin/swif2 status -workflow " + WORKFLOW + " -jobs"
	if VERBOSE > 1:
		print(command)
	process = Popen(command.split(), stdout=PIPE)
	status_output = process.communicate()[0].decode('ASCII') # is stdout. [1] is stderr
	return_code = process.returncode
	if return_code != 0:
		print("swif2 status bad return code, exiting")
		sys.exit()

	if VERBOSE > 99:
		print(status_output)

	# Build a dictionary of all the jobs in the workflow:
	job_dictionary = defaultdict(int) # Run string, list of file strings
	run_string = "-1"
	run_done = 1
	job_results = "-1"
	for line in status_output.splitlines():
		if VERBOSE > 9:
                        print(line)
		if(len(line.split()) < 3):
			continue
		field = line.split()[0]

		if (field == "run_number"):
                        # ... but if job was not successful, set back to 0
                        if run_string != "-1":
                                job_dictionary[run_string] *= run_done

                        run_string = line.split()[2]
                        run_done = 0

                        # new run is first set to done ...
                        if run_string not in job_dictionary:
                                job_dictionary[run_string] = 1


		elif (field == "slurm_exitcode"):
			job_result = line.split()[2]
			if (job_result == "0"):
                                run_done = 1

		elif (field == "slurm_state"):
			job_state = line.split()[2]
			if (job_state != "COMPLETED"):
                                run_done = 0

	# Register the last job
	if(run_string != "-1"):
	  	job_dictionary[run_string] *= run_done

	# if VERBOSE > 0:
	# 	print("Num jobs = " + str(num_jobs))
	return job_dictionary

def build_merge_deque(WORKFLOW):
	command = "/usr/local/bin/swif2 status -workflow " + WORKFLOW + " -jobs"
	if VERBOSE > 1:
		print(command)
	process = Popen(command.split(), stdout=PIPE)
	status_output = process.communicate()[0].decode('ASCII') # is stdout. [1] is stderr
	return_code = process.returncode
	if return_code != 0:
		print("swif2 status bad return code, exiting")
		sys.exit()

	if VERBOSE > 99:
		print(status_output)

	# Build a dictionary of all the jobs in the workflow:
	start_loop_flag = True
	job_deque = deque()
	run_string = "-1"
	for line in status_output.splitlines():
		if VERBOSE > 9:
			print(line)
		if(len(line.split()) < 3):
			continue
		field = line.split()[0]
		if (field == "run_number"):
			run_string = line.split()[2]
			job_deque.append(run_string)

	return job_deque

################################################### ADD JOBS ################################################################

def add_job(MERGE_WORKFLOW, RUNNO, config_dict):

	if(VERBOSE == True):
		print("WORKFLOW, RUN: " + MERGE_WORKFLOW + " " + RUNNO)

	# PREPARE NAMES
	#DATE = time.strftime("%Y-%m-%d")
	JOBNAME = MERGE_WORKFLOW + "_" + RUNNO

        # SETUP LOG DIRECTORY FOR SLURM
	LOG_DIR = config_dict["OUTDIR_SMALL"] + "/log/merged"
	make_log_dir = "mkdir -p " + LOG_DIR
	try_command(make_log_dir)
	if(VERBOSE == True):
		print("LOG DIRECTORY " + LOG_DIR + " CREATED")

	# CREATE ADD-JOB COMMAND
	# job
	add_command = "/usr/local/bin/swif2 add-job -workflow " + MERGE_WORKFLOW + " -name " + JOBNAME
	# accounting
	add_command += " -account " + config_dict["PROJECT"] + " -partition " + config_dict["TRACK"] + " -os " + config_dict["OS"]
	# resources
	add_command += " -cores " + config_dict["NCORES"] + " -disk " + config_dict["DISK"] + " -ram " + config_dict["RAM"] + " -time " + config_dict["TIMELIMIT"]
	# stdout, stderr
	add_command += " -stdout " + config_dict["OUTDIR_SMALL"] + "/log/merged/stdout." + RUNNO + ".out"
	add_command += " -stderr " + config_dict["OUTDIR_SMALL"] + "/log/merged/stderr." + RUNNO + ".err"

	# tags
	add_command += " -tag run_number " + RUNNO + " -tag num_threads " + config_dict["NCORES"]

	# command + arguments
	add_command += " " + config_dict["SCRIPTFILE"] + " " + config_dict["ENVFILE"]
	# command arguments continued
	add_command +=  " " + config_dict["INDATA_TOPDIR"] + " " + config_dict["OUTDIR_LARGE"] + " " + RUNNO + " " + config_dict["CACHE_PIN_DAYS"]


	if(VERBOSE == True):
                print("job add command is \n" + str(add_command))

        # ADD JOB
        #print(str(add_command))
	status = try_command(add_command)


########################################################## MAIN ##########################################################

def main(argv):
	parser_usage = "merge_tree.py <config_file> [<merge_incomplete_runs> = 0]"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) == 0 or len(args) > 2):
		parser.print_help()
		return

        # SET INPUT VARIABLES
	JOB_CONFIG_FILE = args[0]

	if (len(args) == 2) and not args[1] == "0":
                print("Incomplete Runs will be merged!")
                MERGE_INCOMPLETE_RUNS = args[1]
	else:
                MERGE_INCOMPLETE_RUNS = 0

        # READ CONFIG
	config_dict = read_config(JOB_CONFIG_FILE)

	LAUNCH_WORKFLOW = config_dict["WORKFLOW"]
	print("Merging workflow " + LAUNCH_WORKFLOW)

	# BUILD WORKFLOW NAME
	MERGE_WORKFLOW = LAUNCH_WORKFLOW + "_merge"

	# SEARCH FOR COMPLETED RUNS
	# For the launch workflow, get the full status output from SWIF about what jobs are in the workflow
	# Loop through all of these jobs, and for every run, record if all jobs are complete
	launch_dictionary = build_launch_dictionary(LAUNCH_WORKFLOW)
	print("Runs in the launch: " + str(len(launch_dictionary)))

        # SEARCH FOR JOBS
	merge_deque = build_merge_deque(MERGE_WORKFLOW)
	print("Runs previously submitted to merge: " + str(len(merge_deque)))

	# ADD JOBS
        # For every completed run # in the launch, check if there is already a merge job
        # if not, create one
	n_submit = 0
	for run_string in launch_dictionary:

                if launch_dictionary[run_string] == 0 and not MERGE_INCOMPLETE_RUNS:
                        continue # This run is not entirely processed yet

                if run_string in merge_deque:
                        continue # This run is already in the queue or done

                # else sumbit job
                print(run_string)
                add_job(MERGE_WORKFLOW, run_string, config_dict)
                n_submit += 1

	# RUN WORKFLOW (IN CASE NOT RUNNING ALREADY)
	command = "/usr/local/bin/swif2 run -workflow " + MERGE_WORKFLOW
	if VERBOSE > 1:
		print(command)
	try_command(command)

	print("New runs complete and submitted:" + str(n_submit))

	# RETRY FAILED JOBS
	print("\n")
	print("Retry failed jobs in the launch workflow: ")
	command = "/usr/local/bin/swif2 retry-jobs -workflow " + LAUNCH_WORKFLOW + " -problems SLURM_FAILED SLURM_CANCELLED SLURM_NODE_FAIL SITE_LAUNCH_FAIL SITE_PREP_FAIL SWIF_INPUT_FAIL SWIF_SYSTEM_ERROR"
	if VERBOSE > 1:
			print(command)
	try_command(command)

        # MODIFY FAILED JOBS
        command = "swif2 modify-jobs -ram add 4gb -workflow " + LAUNCH_WORKFLOW + " -problems SLURM_OUT_OF_MEMORY"
	if VERBOSE > 1:
		print command
        try_command(command)

        command = "swif2 modify-jobs -time add 4h -workflow " + LAUNCH_WORKFLOW + " -problems SLURM_TIMEOUT"
	if VERBOSE > 1:
		print command
        try_command(command)


	print("Retry failed jobs in the merge workflow: ")
	command = "/usr/local/bin/swif2 retry-jobs -workflow " + MERGE_WORKFLOW + " -problems SLURM_CANCELLED SLURM_TIMEOUT SLURM_NODE_FAIL SITE_LAUNCH_FAIL SITE_PREP_FAIL SWIF_INPUT_FAIL SWIF_SYSTEM_ERROR"
	if VERBOSE > 1:
			print (command)
	try_command(command)


if __name__ == "__main__":
	main(sys.argv[1:])

