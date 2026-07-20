#!/usr/bin/env python

##########################################################################################################################
#
# 2016/02/03 Paul Mattione
#
# Python script for launching jobs over the incoming data
#
##########################################################################################################################

import sys
import os
import glob
import time
from optparse import OptionParser
from subprocess import Popen, PIPE
from collections import defaultdict

#################################################### GLOBAL VARIABLES ####################################################

VERBOSE = 6
RAW_DATA_BASE_DIR = "/mss/halld/"

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

################################################### BUILD DICTIONARIES ###################################################

def build_file_dictionary(RUN_PERIOD, NUM_FILES_PER_RUN):
	file_path = RAW_DATA_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/rawdata/Run*/hd_rawdata_*_*.evio"
	file_list = glob.glob(file_path)

	# Build a dictionary of all the files on tape:
	file_dictionary = defaultdict(set) # Run string, list of file strings
	num_files = 0
	for file_path in file_list:
		file_name = file_path[(file_path.rfind("/") + 1):] #hd_rawdata_<run_string>_<file_string>.evio"
		run_string = file_name[11:17] #skip "hd_rawdata_" 
		file_string = file_name[18:-5] #skip "hd_rawdata_run#_" and ".evio" on the end 
		if(len(file_string) != 3): # sometimes in 2018-01, there is a "_0" at the end
                        file_string = file_string[0:-2] # this will not produce these files, but also not crash
		if(int(file_string) >= NUM_FILES_PER_RUN):
                        continue
		file_dictionary[run_string].add(file_string)
		num_files += 1
		if VERBOSE > 1:
			print("File found, run, file = " + run_string + " " + file_string)

	if VERBOSE > 0:
		print("Num files = " + str(num_files))
	return file_dictionary

def build_job_dictionary(WORKFLOW):
	command = "swif2 status -workflow " + WORKFLOW + " -jobs"
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
	job_dictionary = defaultdict(set) # Run string, list of file strings
	run_string = "-1"
	file_string = "-1"
	num_jobs = 0
	for line in status_output.splitlines():
		if VERBOSE > 9:
		        print(line)
		if(len(line.split()) < 3):
			continue
		field = line.split()[0]
		if (field == "job_id"):
			if start_loop_flag:
				start_loop_flag = False # Don't register yet: Just started
				continue
			# Register the job
			job_dictionary[run_string].add(file_string)
			num_jobs += 1
			if VERBOSE > 1:
				print("Job found, run, file = " + run_string + " " + file_string + " size: " + str(len(job_dictionary[run_string])))
		elif (field == "run_number"): 
			run_string = line.split()[2]
		elif (field == "file_number"): 
			file_string = line.split()[2]

	# Register the last job
	if(run_string != "-1"):
		job_dictionary[run_string].add(file_string)

	if VERBOSE > 0:
		print("Num jobs = " + str(num_jobs))
	return job_dictionary

def add_job(WORKFLOW, CONFIG_PATH, RUN_STRING, FILE_STRING):
	command = "python " + os.path.expanduser("~/hd_utilities/launch_scripts/launch/launch.py") + " " + CONFIG_PATH + " " + str(int(RUN_STRING)) + " " + str(int(RUN_STRING)) + " -f " + FILE_STRING
	if VERBOSE > 0:
		print(command)
	process = Popen(command.split(), stdout=PIPE)
	output = process.communicate()[0].decode('ASCII') # is stdout. [1] is stderr
	if VERBOSE > 0:
		print(output)

########################################################## MAIN ##########################################################

def main(argv):
	parser_usage = "process_incoming.py <run_period> <config_path> <num_files_per_run>"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) != 3):
		parser.print_help()
		return

	# GET ARGUMENTS
	RUN_PERIOD = args[0] #e.g. 2016-02
	CONFIG_PATH = args[1]
	NUM_FILES_PER_RUN = int(args[2])

	# BUILD WORKFLOW NAME
	WORKFLOW = "offmon_" + RUN_PERIOD + "_ver01"

	# SEARCH FOR TAPE FILES
	# Loop over the tape directories, finding all of the input files, and determining their run & file #'s
	file_dictionary = build_file_dictionary(RUN_PERIOD, NUM_FILES_PER_RUN)

	# SEARCH FOR JOBS
	# For the workflow, get the full status output from SWIF about what jobs are in the workflow
	# Loop through all of these jobs, and for every run, record which file #'s have jobs submitted for them
	job_dictionary = build_job_dictionary(WORKFLOW)
	print("job dict size = " + str(len(job_dictionary)))
	# ADD JOBS
	# For every run # on tape, see how many jobs are in the workflow. If > NUM_FILES_PER_RUN, continue to the next run
	# If num files on tape same as in workflow, entire job was submitted: continue
	# For the first N files not-yet submitted, make sure their last-modified is at least 5 minutes old (fully formed file). else continue
	# Add the job (with -create, so not needed)
	for run_string in file_dictionary:
		num_jobs_submitted = len(job_dictionary[run_string])
		for file_string in job_dictionary[run_string]:
			print(file_string)
		num_files = len(file_dictionary[run_string])
		if VERBOSE > 1:
			print("run string: " + run_string + ", num files = " + str(num_files) + ", num jobs submitted previously: " + str(num_jobs_submitted) + ", max per run = " + str(NUM_FILES_PER_RUN))
		if(num_jobs_submitted >= NUM_FILES_PER_RUN):
			if VERBOSE > 1:
				print("Max jobs (" + str(NUM_FILES_PER_RUN) + ") submitted for run " + run_string)
			continue # This run is done
		if(num_jobs_submitted >= num_files):
			if VERBOSE > 1:
				print("All jobs (" + str(num_files) + ") submitted for run " + run_string)
			continue # This run is done

		# Submit jobs, IF the file (file = mss stub) timestamp hasn't been modified in at least 5 minutes
		files_not_submitted = sorted(list(file_dictionary[run_string] - job_dictionary[run_string]))
		for file_string in files_not_submitted:
			if VERBOSE > 5:
				print("file string = " + file_string)
			if(num_jobs_submitted >= NUM_FILES_PER_RUN):
				break

			# Check the timestamp
			file_path = RAW_DATA_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/rawdata/Run" + run_string + "/hd_rawdata_" + run_string + "_" + file_string + ".evio"
			if not os.path.exists(file_path):
				print("FILE IS IN WRONG DIRECTORY: " + file_path)
				continue
			modified_time = os.path.getmtime(file_path) # time that the file was last modified, in seconds since the epoch
			current_time = time.time() # in seconds since the epoch
			delta_t = (current_time - modified_time)/60.0
			if VERBOSE > 1:
				print("Current time (s), mod time (s), delta_t (min) = " + str(current_time) + ", " + str(modified_time) + ", " + str(delta_t))
			if(delta_t < 5.0):
				continue # file (stub) is not at least five minutes old: file may not be fully resident on tape yet: don't submit job

			# Add job
			add_job(WORKFLOW, CONFIG_PATH, run_string, file_string)
			num_jobs_submitted += 1

	# RUN WORKFLOW (IN CASE NOT RUNNING ALREADY)
	command = "swif2 run -workflow " + WORKFLOW
	if VERBOSE > 1:
		print(command)
	try_command(command)

if __name__ == "__main__":
	main(sys.argv[1:])


