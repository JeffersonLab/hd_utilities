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

VERBOSE = 1
RAW_DATA_BASE_DIR = "/mss/halld/"
#RAW_DATA_BASE_DIR = "/cache/halld/"

######################################################## UTILITIES #######################################################

def try_command(command, sleeptime = 10):
	# Try an os command and if the exit code is non-zero return an error
	return_code = -999
	while return_code != 0:
		process = Popen(command.split(), stdout=PIPE)
		output = process.communicate()[0] # is stdout. [1] is stderr
		#print output

		return_code = process.returncode
		if return_code == 0:
			break

		# sleep for N seconds between
		print 'sleeping for ' + str(sleeptime) + ' sec...'
		time.sleep(sleeptime)

	return output

################################################### BUILD DICTIONARIES ###################################################

def build_file_dictionary(RUN_PERIOD):
	file_path = RAW_DATA_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/rawdata/Run*/hd_rawdata_*_*.evio"
	file_list = glob.glob(file_path)

	# Build a dictionary of all the files on tape:
	file_dictionary = defaultdict(set) # Run string, list of file strings
	num_files = 0
	for file_path in file_list:
		file_name = file_path[(file_path.rfind("/") + 1):] #hd_rawdata_<run_string>_<file_string>.evio"
		run_string = file_name[11:17] #skip "hd_rawdata_" 
		file_string = file_name[18:-5] #skip "hd_rawdata_run#_" and ".evio" on the end 
		file_dictionary[run_string].add(file_string)
		num_files += 1
		if VERBOSE > 0:
			print "File found, run, file = " + run_string + " " + file_string

	if VERBOSE > 0:
		print "Num files = " + str(num_files)
	return file_dictionary

def build_job_dictionary(WORKFLOW):
	command = "swif status -workflow " + WORKFLOW + " -jobs"
	if VERBOSE > 0:
		print command
	process = Popen(command.split(), stdout=PIPE)
	status_output = process.communicate()[0] # is stdout. [1] is stderr
	if VERBOSE > 99:
		print status_output

	# Build a dictionary of all the jobs in the workflow:
	start_loop_flag = True
	job_dictionary = defaultdict(set) # Run string, list of file strings
	run_string = "-1"
	file_string = "-1"
	num_jobs = 0
	for line in status_output.splitlines():
		if VERBOSE > 9:
			print line
		line_length = len(line)
		if (line_length > 2) and (line[:2] == "id"): 
			if start_loop_flag:
				start_loop_flag = False # Don't register yet: Just started
				continue
			# Register the job
			job_dictionary[run_string].add(file_string)
			num_jobs += 1
			if VERBOSE > 0:
				print "Job found, run, file = " + run_string + " " + file_string
		elif (line_length > 8) and (line[:8] == "user_run"): 
			run_string = line[11:]
		elif (line_length > 9) and (line[:9] == "user_file"): 
			file_string = line[12:]

	# Register the last job
	if(run_string != "-1"):
		job_dictionary[run_string].add(file_string)

	if VERBOSE > 0:
		print "Num jobs = " + str(num_jobs)
	return job_dictionary

def add_job(WORKFLOW, CONFIG_PATH, RUN_STRING, FILE_STRING):
	command = "python " + os.path.expanduser("~/monitoring/hdswif/hdswif.py") + " add " + WORKFLOW + " -c " + CONFIG_PATH + " -r " + str(int(RUN_STRING)) + " -f " + FILE_STRING
	if VERBOSE > 0:
		print command
	process = Popen(command.split(), stdout=PIPE)
	output = process.communicate()[0] # is stdout. [1] is stderr
	if VERBOSE > 0:
		print output

########################################################## MAIN ##########################################################

def main(argv):
	parser_usage = "process_incoming.py <run_period> <config_path> <num_files_per_run>"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) != 3):
		parser.print_help()
		return

	# GET ARGUMENTS
	INPUT_RUN_PERIOD = args[0] #e.g. 2016-02 OR 2016_02 (both will work)
	CONFIG_PATH = args[1]
	NUM_FILES_PER_RUN = args[2]

	# BUILD WORKFLOW NAME
	RUN_PERIOD_WITH_DASH = INPUT_RUN_PERIOD.replace("_", "-")
	RUN_PERIOD_WITH_UNDERSCORE = INPUT_RUN_PERIOD.replace("-", "_")
	WORKFLOW = "offline_monitoring_RunPeriod" + RUN_PERIOD_WITH_UNDERSCORE + "_ver01_hd_rawdata"

	# SEARCH FOR TAPE FILES
	# Loop over the tape directories, finding all of the input files, and determining their run & file #'s
	file_dictionary = build_file_dictionary(RUN_PERIOD_WITH_DASH)

	# SEARCH FOR JOBS
	# For the workflow, get the full status output from SWIF about what jobs are in the workflow
	# Loop through all of these jobs, and for every run, record which file #'s have jobs submitted for them
	job_dictionary = build_job_dictionary(WORKFLOW)

	# ADD JOBS
	# For every run # on tape, see how many jobs are in the workflow. If > NUM_FILES_PER_RUN, continue to the next run
	# If num files on tape same as in workflow, entire job was submitted: continue
	# For the first N files not-yet submitted, make sure their last-modified is at least 5 minutes old (fully formed file). else continue
	# Add the job (with -create, so not needed)
	for run_string in file_dictionary:
		num_jobs_submitted = len(job_dictionary[run_string])
		num_files = len(file_dictionary[run_string])
		if(num_jobs_submitted >= NUM_FILES_PER_RUN):
			if VERBOSE > 0:
				print "Max jobs (" + NUM_FILES_PER_RUN + ") submitted for run " + run_string
			continue # This run is done
		if(num_jobs_submitted == num_files):
			if VERBOSE > 0:
				print "All jobs (" + num_files + ") submitted for run " + run_string
			continue # This run is done

		# Submit jobs, IF the file (file = mss stub) timestamp hasn't been modified in at least 5 minutes
		files_not_submitted = sorted(list(file_dictionary[run_string] - job_dictionary[run_string]))
		for file_string in files_not_submitted:
			if(num_jobs_submitted >= NUM_FILES_PER_RUN):
				break

			# Check the timestamp
			file_path = RAW_DATA_BASE_DIR + "/RunPeriod-" + RUN_PERIOD_WITH_DASH + "/rawdata/Run" + run_string + "/hd_rawdata_" + run_string + "_" + file_string + ".evio"
			modified_time = os.path.getmtime(file_path) # time that the file was last modified, in seconds since the epoch
			current_time = time.time() # in seconds since the epoch
			delta_t = (current_time - modified_time)/60.0
			if VERBOSE > 0:
				print "Current time (s), mod time (s), delta_t (min) = " + str(current_time) + ", " + str(modified_time) + ", " + str(delta_t)
			if(delta_t < 5.0):
				continue # file (stub) is not at least five minutes old: file may not be fully resident on tape yet: don't submit job

			# Add job
			add_job(WORKFLOW, CONFIG_PATH, run_string, file_string)
			num_jobs_submitted += 1

	# RUN WORKFLOW (IN CASE NOT RUNNING ALREADY)
	command = "python " + os.path.expanduser("~/monitoring/hdswif/hdswif.py") + " run " + WORKFLOW
	if VERBOSE > 0:
		print command
#	try_command(command)

if __name__ == "__main__":
	main(sys.argv[1:])


