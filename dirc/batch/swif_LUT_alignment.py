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
TRACK      = "simulation"	  # https://scicomp.jlab.org/docs/batch_job_tracks
ACCOUNT    = "halld"
PARTITION  = "production"

# RESOURCES
NCORES     = "1"              # Number of CPU cores
DISK       = "100GB"          # Max Disk usage
RAM        = "2GB"            # Max RAM usage
TIMELIMIT  = "3600minutes"    # Max walltime
OS         = "el9"        # Specify CentOS65 machines

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/work/halld/home/%s/RunPeriod-2019-11/dircsim-2019_11-ver05/lut_alignment/"%(os.environ['USER']) 

# JOB EXECUTION
BUILD_DIR         = "/work/halld2/home/jrsteven/analysisGluexII/builds/"
SCRIPTFILE        = BUILD_DIR + "hd_utilities/dirc/batch/script_LUT_alignment.sh"
ENVFILE           = BUILD_DIR + "setup_gluex.csh"
CONFIG_FILE_PATH  = BUILD_DIR + "hd_utilities/dirc/batch/"

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

################################################## GENERATE GEOMETRY CONFIG ##################################################
def geom_config(GEOM_FILE,MRASROT,MRASOFFSET,MRANROT,MRANOFFSET):

    geom_config_file = open("geom_config.cfg", 'w')
    geom_config_file.write("[FILES] \n")
    geom_config_file.write("NOMINAL = ../alignment/DIRC_HDDS_Nominal.xml \n")
    geom_config_file.write("OUTPUT  = %s \n" % GEOM_FILE)
    geom_config_file.write("[OBCS] \n")
    geom_config_file.write("MRAS_rot = %f,%f,%f \n" % (MRASROT[0],MRASROT[1],MRASROT[2]))
    geom_config_file.write("MRAS_offset = %f,%f,%f \n" % (MRASOFFSET[0],MRASOFFSET[1],MRASOFFSET[2]))
    geom_config_file.write("[OBCN] \n")
    geom_config_file.write("MRAN_rot = %f,%f,%f \n" % (MRANROT[0],MRANROT[1],MRANROT[2]))
    geom_config_file.write("MRAN_offset = %f,%f,%f \n" % (MRANOFFSET[0],MRANOFFSET[1],MRANOFFSET[2]))
    geom_config_file.close()
    subprocess.call(["python","../alignment/tweak_DIRC.py","geom_config.cfg"])
    
######################################################## ADD JOB #########################################################

def add_job(WORKFLOW, VARIATION):

	# PREPARE NAMES
	STUBNAME = VARIATION
	JOBNAME = WORKFLOW + "_%d" % STUBNAME

	# CREATE ADD-JOB COMMAND
	# job
	add_command = "swif2 add-job -workflow " + WORKFLOW + " -name " + JOBNAME
	# project/track
	add_command += " -account " + ACCOUNT + " -partition " + PARTITION
	# resources
	add_command += " -cores " + NCORES + " -disk " + DISK + " -ram " + RAM + " -time " + TIMELIMIT + " -os " + OS
	# stdout
	add_command += " -stdout " + DATA_OUTPUT_BASE_DIR + "/var%d" % VARIATION + "/log/stdout." + JOBNAME + ".out"
	# stderr
	add_command += " -stderr " + DATA_OUTPUT_BASE_DIR + "/var%d" % VARIATION + "/log/stderr." + JOBNAME + ".err"
	# command
	add_command += " " + SCRIPTFILE + " " + ENVFILE + " " + BUILD_DIR + " " + DATA_OUTPUT_BASE_DIR + "/var%d/ %d " % (VARIATION, VARIATION)

	if(VERBOSE == True):
		print("job add command is \n" + str(add_command))

	# ADD JOB
	status = subprocess.call(add_command.split(" "))


########################################################## MAIN ##########################################################
	
def main(argv):
    parser_usage = "swif_lut_alignment.py workflow"
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

    # create config files for each bar
    for BAR in range(0, 48):
        CONFIG_FILE = CONFIG_FILE_PATH + "control_%d.in" % BAR
        generate_config(CONFIG_FILE, BAR)

    # submit job for no variation
    VARIATION = 0
    subprocess.call(["mkdir","-p",DATA_OUTPUT_BASE_DIR+"/var%d" % VARIATION+"/log/"])
    subprocess.call(["cp","../alignment/DIRC_HDDS_Nominal.xml","%s/var%d/DIRC_HDDS.xml" % (DATA_OUTPUT_BASE_DIR,VARIATION)])
    add_job(WORKFLOW, VARIATION)
        
    # loop over variations
    MRASOFFSETS = [-2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0]
    
    for MRASXOFFSET in MRASOFFSETS:
        VARIATION += 1
        
        MRASOFFSET = [MRASXOFFSET,0.,0.]
        MRASROT = [0.,0.,0.]
        MRANOFFSET = [0.,0.,0.]
        MRANROT = [0.,0.,0.]

        subprocess.call(["mkdir","-p",DATA_OUTPUT_BASE_DIR+"/var%d" % VARIATION+"/log/"])
        
        # create alternative geometry file for each variation
        geom_config("%s/var%d/DIRC_HDDS.xml" % (DATA_OUTPUT_BASE_DIR,VARIATION),MRASROT,MRASOFFSET,MRANROT,MRANOFFSET)
        add_job(WORKFLOW, VARIATION)

if __name__ == "__main__":
   main(sys.argv[1:])

