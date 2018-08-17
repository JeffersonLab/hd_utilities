#!/usr/bin/env python

##########################################################################################################################
#
# 2015/07/24 Paul Mattione
# Heavily based off of work by Kei Moriya at:
# https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/hdswif/hdswif.py
#
# SWIF DOCUMENTATION:
# https://scicomp.jlab.org/docs/swif
# https://scicomp.jlab.org/docs/swif-cli
# https://scicomp.jlab.org/help/swif/add-job.txt #consider phase!
#
##########################################################################################################################

from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
import glob

import time

#################################################### RCDB ENVIRONMENT ####################################################
os.environ["RCDB_HOME"] = "/group/halld/www/halldweb/html/rcdb_home"
sys.path.append("/group/halld/www/halldweb/html/rcdb_home/python")
import rcdb
db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb")

#################################################### GLOBAL VARIABLES ####################################################

# DEBUG
VERBOSE    = False

# PROJECT INFO
PROJECT    = "gluex"          # http://scicomp.jlab.org/scicomp/#/projects
TRACK      = "analysis"		   # https://scicomp.jlab.org/docs/batch_job_tracks

# RESOURCES
NCORES     = "4"               # Number of CPU cores
DISK       = "10GB"            # Max Disk usage
RAM        = "5000MB"            # Max RAM usage
TIMELIMIT  = "600minutes"      # Max walltime
OS         = "centos7"        # Specify CentOS65 machines

# SOURCE DATA INFORMATION
# FILES ARE SEARCHED-FOR WITH THE PATH: DATA_SOURCE_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/" + VERSION + "/REST/" + FORMATTED_RUN + "/dana_rest_*.hddm"
# Where FORMATTED_RUN is the run number you chose, with leading zeros. 
DATA_SOURCE_TYPE      = "file" #"mss" or "file"
DATA_SOURCE_BASE_DIR  = "/volatile/halld/home/jrsteven/2018-dirc/dircsim-2018_08-ver01/"

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/volatile/halld/home/%s/2018-dirc/dircsim-2018_08-ver01/analysis/"%(os.environ['USER'])   ## CHANGE IF YOU WANT TO

# JOB EXECUTION
SCRIPTFILE        = "/work/halld2/home/jrsteven/2018-dirc/dirc/batch/analysis/script.sh"
ENVFILE           = "/work/halld2/home/jrsteven/2018-dirc/builds/setup_gluex.csh"
CONFIG_FILE_PATH  = "/work/halld2/home/jrsteven/2018-dirc/dirc/batch/analysis/analysis_dirc.conf"
TREE_NAMES        = "dirc_reactions" #,npi_missing,compton,gg"

# CONFIG FILE CONTENTS
CONFIG_DICT = {}
CONFIG_DICT["PLUGINS"] =            "monitoring_hists,dirc_hists,dirc_reactions,truth_dirc"
CONFIG_DICT["NTHREADS"] =           "4" #Ncores if you have the whole node
CONFIG_DICT["THREAD_TIMEOUT"] =     "300"
CONFIG_DICT["JANA_CALIB_CONTEXT"] = '"variation=mc"' #'"variation=,calibtime="'
CONFIG_DICT["TRIG:BYPASS"] =            "1"
CONFIG_DICT["JANA:MAX_RELAUNCH_THREADS"] = "10"

################################################## GENERATE CONFIG FILE ##################################################

def generate_config():
	config_file = open(CONFIG_FILE_PATH, 'w')
	config_file.truncate() #wipe it out

	for key in CONFIG_DICT:
		value = CONFIG_DICT[key]
		config_file.write(key + " " + value + "\n")

	config_file.close()

####################################################### FIND FILES #######################################################

def find_files(DATA_SOURCE_DIR):

	# CHANGE TO THE DIRECTORY CONTAINING THE INPUT FILES
	current_dir = os.getcwd()
        if not os.path.isdir(DATA_SOURCE_DIR):
                return []
	os.chdir(DATA_SOURCE_DIR)

	# SEARCH FOR THE FILES
	file_signature = "*_smeared.hddm"
	file_list = glob.glob(file_signature)
	if(VERBOSE == True):
		print "size of file_list is " + str(len(file_list))

	# CHANGE BACK TO THE PREVIOUS DIRECTORY
	os.chdir(current_dir)
	return file_list

######################################################## ADD JOB #########################################################

def add_job(WORKFLOW, DATA_SOURCE_DIR, DATA_OUTPUT_DIR, FILENAME, RUNNO, FILENO):

	# PREPARE NAMES
	STUBNAME = RUNNO + "_" + FILENO
	JOBNAME = WORKFLOW + "_" + STUBNAME

	# CREATE ADD-JOB COMMAND
	# job
	add_command = "swif add-job -workflow " + WORKFLOW + " -name " + JOBNAME
	# project/track
	add_command += " -project " + PROJECT + " -track " + TRACK
	# resources
	add_command += " -cores " + NCORES + " -disk " + DISK + " -ram " + RAM + " -time " + TIMELIMIT + " -os " + OS
	# inputs
	add_command += " -input " + FILENAME + " " + DATA_SOURCE_TYPE + ":" + DATA_SOURCE_DIR + "/" + FILENAME
	# stdout
	add_command += " -stdout " + DATA_OUTPUT_DIR + "/log/" + RUNNO + "/stdout." + STUBNAME + ".out"
	# stderr
	add_command += " -stderr " + DATA_OUTPUT_DIR + "/log/" + RUNNO + "/stderr." + STUBNAME + ".err"
	# tags
	add_command += " -tag run_number " + RUNNO
	# tags
	add_command += " -tag file_number " + FILENO
	# command
	add_command += " " + SCRIPTFILE + " " + ENVFILE + " " + FILENAME + " " + CONFIG_FILE_PATH + " " + DATA_OUTPUT_DIR + " " + RUNNO + " " + FILENO + " " + TREE_NAMES

	if(VERBOSE == True):
		print "job add command is \n" + str(add_command)

	# ADD JOB
	#time.sleep(120)
	status = subprocess.call(add_command.split(" "))


########################################################## MAIN ##########################################################
	
def main(argv):
	parser_usage = "swif_gluex_analysis.py workflow generator minrun maxrun"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) != 4):
		parser.print_help()
		return

	# GET ARGUMENTS
	WORKFLOW = args[0]
	GENERATOR = args[1]
	MINRUN = int(args[2])
	MAXRUN = int(args[3])

	# CREATE WORKFLOW IF IT DOESN'T ALREADY EXIST
	WORKFLOW_LIST = subprocess.check_output(["swif", "list"])
	if WORKFLOW not in WORKFLOW_LIST:
	    status = subprocess.call(["swif", "create", "-workflow", WORKFLOW])

	DATA_OUTPUT_DIR = DATA_OUTPUT_BASE_DIR + "/" + GENERATOR

	# GENERATE CONFIG
	generate_config()

	# GET LISTS OF GOOD RUNS TO CHOOSE FROM
	RCDB_RUNS = db.select_runs("@is_production and @status_approved")
	RCDB_RUN_NUMBERS = [ run.number for run in RCDB_RUNS ]

	# FIND/ADD JOBS
	for RUN in range(MINRUN, MAXRUN + 1):

		# Check RCDB status for each run number
		#if RUN not in RCDB_RUN_NUMBERS:
		#	continue

		# Format run and file numbers
		FORMATTED_RUN = "%06d" % RUN

		# Find files for run number
		DATA_SOURCE_DIR = DATA_SOURCE_BASE_DIR + "/" + GENERATOR + "/hddm"
		print DATA_SOURCE_DIR
		if(os.path.exists(DATA_SOURCE_DIR)):
			file_list = find_files(DATA_SOURCE_DIR)
		else:
			continue
		if(len(file_list) == 0):
			continue

		# Add jobs to workflow
		for FILENAME in file_list:
			FILENO = FILENAME[-23:-20] #e.g. dana_rest_003185_015.hddm #Cheat!
			print FILENAME
			print FILENO
			add_job(WORKFLOW, DATA_SOURCE_DIR, DATA_OUTPUT_DIR, FILENAME, FORMATTED_RUN, FILENO)

if __name__ == "__main__":
   main(sys.argv[1:])

