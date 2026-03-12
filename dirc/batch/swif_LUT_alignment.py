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
DISK       = "50GB"          # Max Disk usage
RAM        = "3GB"            # Max RAM usage
TIMELIMIT  = "1200minutes"    # Max walltime
OS         = "el9"        # Specify CentOS65 machines

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/work/halld/home/%s/analysisGluexII/dircsim-2019_11-ver05/lut_alignment_MRAROT_Nominal/"%(os.environ['USER']) 

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
def geom_config(GEOM_FILE,MRASROT,MRASOFFSET,MRANROT,MRANOFFSET,hdds_iter):

    geom_config_file = open("geom_config.cfg", 'w')
    geom_config_file.write("[FILES] \n")
    geom_config_file.write("NOMINAL = ../alignment/DIRC_HDDS_%s.xml \n" % hdds_iter)
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
    parser_usage = "swif_lut_alignment.py [--hdds-iter ITER] workflow"
    parser = OptionParser(usage = parser_usage)
    parser.add_option("--hdds-iter", dest="hdds_iter", default="Nominal",
                      help="Use DIRC_HDDS_<ITER>.xml as nominal geometry input (default: %default)")
    parser.add_option("--scan-mode", dest="scan_mode", type="choice", choices=["MRAROT", "MRAOFF"], default="MRAROT",
                      help="Scan rotations (MRAROT) or offsets (MRAOFF) (default: %default)")
    (options, args) = parser.parse_args(argv)
    
    if(len(args) != 1):
        parser.print_help()
        return
    
    # GET ARGUMENTS
    WORKFLOW = args[0]
    HDDS_ITER = options.hdds_iter
    SCAN_MODE = options.scan_mode

    global DATA_OUTPUT_BASE_DIR
    DATA_OUTPUT_BASE_DIR = "/work/halld/home/%s/analysisGluexII/dircsim-2019_11-ver05/lut_alignment_%s_%s/" % (os.environ['USER'], SCAN_MODE, HDDS_ITER)
    
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
    subprocess.call(["cp","../alignment/DIRC_HDDS_%s.xml" % HDDS_ITER,"%s/var%d/DIRC_HDDS.xml" % (DATA_OUTPUT_BASE_DIR,VARIATION)])
    add_job(WORKFLOW, VARIATION)
    
    # loop over variations
    scan_values = [-1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5]
    if SCAN_MODE == "MRAROT":
        MRASXOFFSETS = [0]
        MRASYOFFSETS = [0]
        MRASZOFFSETS = [0]

        MRASXROTATIONS = scan_values
        MRASYROTATIONS = scan_values
        MRASZROTATIONS = scan_values
    else:
        MRASXOFFSETS = scan_values
        MRASYOFFSETS = scan_values
        MRASZOFFSETS = scan_values

        MRASXROTATIONS = [0]
        MRASYROTATIONS = [0]
        MRASZROTATIONS = [0]
    
    MRANXOFFSETS = MRASXOFFSETS
    MRANYOFFSETS = MRASYOFFSETS
    MRANZOFFSETS = MRASZOFFSETS
    
    MRANXROTATIONS = MRASXROTATIONS
    MRANYROTATIONS = MRASYROTATIONS
    MRANZROTATIONS = MRASZROTATIONS
    
    for MRASXOFFSET,MRANXOFFSET in zip(MRASXOFFSETS, MRANXOFFSETS):
        for MRASYOFFSET,MRANYOFFSET in zip(MRASYOFFSETS, MRANYOFFSETS):
            for MRASZOFFSET,MRANZOFFSET in zip(MRASZOFFSETS, MRANZOFFSETS):
                for MRASXROTATION,MRANXROTATION in zip(MRASXROTATIONS, MRANXROTATIONS):
                    for MRASYROTATION,MRANYROTATION in zip(MRASYROTATIONS, MRANYROTATIONS):
                        for MRASZROTATION,MRANZROTATION in zip(MRASZROTATIONS, MRANZROTATIONS):
                
                            VARIATION += 1
        
                            MRASOFFSET = [MRASXOFFSET,MRASYOFFSET,MRASZOFFSET]
                            MRASROT = [MRASXROTATION,MRASYROTATION,MRASZROTATION]
                            MRANOFFSET = [MRANXOFFSET,MRANYOFFSET,MRANZOFFSET]
                            MRANROT = [MRANXROTATION,MRANYROTATION,MRANZROTATION]

                            subprocess.call(["mkdir","-p",DATA_OUTPUT_BASE_DIR+"/var%d" % VARIATION+"/log/"])
        
                            # create alternative geometry file for each variation
                            geom_config("%s/var%d/DIRC_HDDS.xml" % (DATA_OUTPUT_BASE_DIR,VARIATION),MRASROT,MRASOFFSET,MRANROT,MRANOFFSET,HDDS_ITER)
                            add_job(WORKFLOW, VARIATION)

if __name__ == "__main__":
   main(sys.argv[1:])

