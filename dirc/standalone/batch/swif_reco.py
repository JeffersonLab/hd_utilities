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

#################################################### GLOBAL VARIABLES ####################################################

# DEBUG
VERBOSE    = True

# PROJECT INFO
PROJECT    = "halld"          # http://scicomp.jlab.org/scicomp/#/projects
TRACK      = "ifarm"          # https://scicomp.jlab.org/docs/batch_job_tracks

# RESOURCES
NCORES     = "1"              # Number of CPU cores
DISK       = "10GB"           # Max Disk usage
RAM        = "4GB"            # Max RAM usage
TIMELIMIT  = "720minutes"     # Max walltime
OS         = "centos79"       # Specify CentOS79 machines

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/work/halld/home/%s/RunPeriod-2019-11/standalone/"%(os.environ['USER']) 

# JOB EXECUTION
SCRIPTFILE        = "/work/halld2/home/gxproj7/monitoring/builds/hd_utilities/dirc/standalone/batch/script_reco.sh"
ENVFILE           = "/work/halld2/home/gxproj7/monitoring/builds/setup_gluex.csh"
CODE_DIR          = "/work/halld2/home/gxproj7/monitoring/builds/hd_utilities/dirc/standalone/"
INPUT_FILE        = "/volatile/halld/home/jrsteven/RunPeriod-2019-11/recon/ver01_pass03/merged/hd_root_72645_72679.root"
LUT_FILE          = "/work/halld/home/gxproj7/RunPeriod-2019-11/dircsim-2019_11-ver03/lut/lut_all_avr.root"

######################################################## ADD JOB #########################################################

def add_job(WORKFLOW, BAR, BIN):

	# PREPARE NAMES
	STUBNAME = "%d_%d" % (BAR, BIN)
	JOBNAME = WORKFLOW + "_%s" % STUBNAME

	# CREATE ADD-JOB COMMAND
	# job
	add_command = "swif2 add-job -workflow " + WORKFLOW + " -name " + JOBNAME
	# project/track
	add_command += " -account " + PROJECT + " -partition " + TRACK
	# resources
	add_command += " -cores " + NCORES + " -disk " + DISK + " -ram " + RAM + " -time " + TIMELIMIT + " -os " + OS
	# stdout
	add_command += " -stdout " + DATA_OUTPUT_BASE_DIR + "/log/stdout." + JOBNAME + ".out"
	# stderr
	add_command += " -stderr " + DATA_OUTPUT_BASE_DIR + "/log/stderr." + JOBNAME + ".err"
        # sbatch (export local environment to batch job: needed for PATH variable to be set?) 
        add_command += " -sbatch --export=ALL ::"

	# command
	add_command += " " + SCRIPTFILE + " " + ENVFILE + " " + INPUT_FILE + " " + LUT_FILE + " " + CODE_DIR  + " " + DATA_OUTPUT_BASE_DIR + " %d %d" % (BAR, BIN)

	if(VERBOSE == True):
		print "job add command is \n" + str(add_command)

	# ADD JOB
	status = subprocess.call(add_command.split(" "))


########################################################## MAIN ##########################################################
	
def main(argv):
	parser_usage = "swif_reco.py workflow"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) != 1):
		parser.print_help()
		return

	# GET ARGUMENTS
	WORKFLOW = args[0]

	# CREATE WORKFLOW IF IT DOESN'T ALREADY EXIST
	WORKFLOW_LIST = subprocess.check_output(["swif2", "list"])
	if WORKFLOW not in WORKFLOW_LIST:
	    status = subprocess.call(["swif2", "create", "-workflow", WORKFLOW])

	# ADD JOBS
	for BAR in range(0, 48):
                for BIN in range(0, 10):
		
                        print BAR, BIN
                        add_job(WORKFLOW, BAR, BIN)

        # START JOBS
        subprocess.call(["swif2", "run", WORKFLOW])

if __name__ == "__main__":
   main(sys.argv[1:])

