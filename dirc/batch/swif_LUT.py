#!/usr/bin/env python

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
PROJECT    = "gluex-pro"          # http://scicomp.jlab.org/scicomp/#/projects
TRACK      = "simulation"		   # https://scicomp.jlab.org/docs/batch_job_tracks
ACCOUNT    = "halld"
PARTITION  = "production"

# RESOURCES
NCORES     = "1"               # Number of CPU cores
DISK       = "20GB"            # Max Disk usage
RAM        = "10GB"            # Max RAM usage
TIMELIMIT  = "600minutes"      # Max walltime
OS         = "el9"        # Specify CentOS65 machines

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/work/halld/home/%s/analysisGluexII/dircsim-2019_11-ver05/lut_50M/"%(os.environ['USER']) 

# JOB EXECUTION
BUILD_DIR         = "/work/halld2/home/jrsteven/analysisGluexII/builds/"
SCRIPTFILE        = BUILD_DIR + "hd_utilities/dirc/batch/script_LUT.sh"
ENVFILE           = BUILD_DIR + "setup_gluex.csh"
CONFIG_FILE_PATH  = BUILD_DIR + "hd_utilities/dirc/batch/"
MAC_FILE          = BUILD_DIR + "hd_utilities/dirc/batch/run.mac"

################################################## GENERATE CONTROL.IN FILE ##################################################

def generate_config(CONFIG_FILE, BAR):

    config_file = open(CONFIG_FILE, 'w')
    config_file.truncate() #wipe it out

    config_file.write("RUNG 70000 \n")
    config_file.write("OUTFILE 'out.hddm' \n")
    config_file.write("DIRCLUT %d \n" % BAR)
    config_file.write("HADR 1 \n")
    config_file.write("CKOV 1 \n")
    config_file.write("LABS 1 \n\n")
    
    config_file.write("ABAN 0 \n")
    config_file.write("DEBU 1 10 1000 \n")
    config_file.write("NOSECONDARIES 0 \n\n")
    
    config_file.write("SAVEHITS  0 \n")
    config_file.write("SHOWERSINCOL 0 \n")
    config_file.write("DRIFTCLUSTERS 0 \n\n")
    
    config_file.write("END \n")
    config_file.close()

######################################################## ADD JOB #########################################################

def add_job(WORKFLOW, CONFIG_FILE, MAC_FILE, BAR):

	# PREPARE NAMES
	STUBNAME = BAR
	JOBNAME = WORKFLOW + "_%d" % STUBNAME

	# CREATE ADD-JOB COMMAND
	# job
	add_command = "swif2 add-job -workflow " + WORKFLOW + " -name " + JOBNAME
	# project/track
	add_command += " -account " + ACCOUNT + " -partition " + PARTITION
	# resources
	add_command += " -cores " + NCORES + " -disk " + DISK + " -ram " + RAM + " -time " + TIMELIMIT + " -os " + OS
	# stdout
	add_command += " -stdout " + DATA_OUTPUT_BASE_DIR + "/log/stdout." + JOBNAME + ".out"
	# stderr
	add_command += " -stderr " + DATA_OUTPUT_BASE_DIR + "/log/stderr." + JOBNAME + ".err"
	# command
	add_command += " " + SCRIPTFILE + " " + ENVFILE + " " + CONFIG_FILE + " " + MAC_FILE + " " + DATA_OUTPUT_BASE_DIR + " %d" % BAR

	if(VERBOSE == True):
		print("job add command is \n" + str(add_command))

	# ADD JOB
	status = subprocess.call(add_command.split(" "))


########################################################## MAIN ##########################################################
	
def main(argv):
    parser_usage = "swif_lut.py workflow"
    parser = OptionParser(usage = parser_usage)
    (options, args) = parser.parse_args(argv)
    
    if(len(args) != 1):
        parser.print_help()
        return
    
	# GET ARGUMENTS
    WORKFLOW = args[0]
    
	# CREATE WORKFLOW IF IT DOESN'T ALREADY EXIST
    #WORKFLOW_LIST = subprocess.check_output(["swif2", "list"])
    #if WORKFLOW not in WORKFLOW_LIST:
    #    status = subprocess.call(["swif2", "create", "-workflow", WORKFLOW])
    
    subprocess.call(["mkdir","-p",DATA_OUTPUT_BASE_DIR+"/log/"])
    
	# ADD JOBS
    for BAR in range(0, 48):
        print(BAR)
        CONFIG_FILE = CONFIG_FILE_PATH + "control_%d.in" % BAR
        generate_config(CONFIG_FILE, BAR)
        
        add_job(WORKFLOW, CONFIG_FILE, MAC_FILE, BAR)

if __name__ == "__main__":
   main(sys.argv[1:])

