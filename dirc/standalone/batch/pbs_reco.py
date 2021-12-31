#!/usr/bin/env python

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

# RESOURCES
NCORES     = "2"              # Number of CPU cores
DISK       = "10GB"           # Max Disk usage
RAM        = "4GB"            # Max RAM usage
TIMELIMIT  = "720minutes"     # Max walltime
OS         = "centos79"       # Specify CentOS79 machines
CLUSTER    = "vortex" #x5672
#vx06 and vx25 have PERL read_file error

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/sciclone/gluex10/jrstevens01/RunPeriod-2019-11/standalone/"

# JOB EXECUTION
SCRIPTFILE        = "/sciclone/home10/jrstevens01/2020-dirc/standalone/batch/script_reco.sh"
ENVFILE           = "/sciclone/home10/jrstevens01/analysisGluexI/builds/setup.csh"
CODE_DIR          = "/sciclone/home10/jrstevens01/2020-dirc/standalone/"
RUNNING_DIR       = "/sciclone/scr20/jrstevens01/TMPDIR/standalone_reco/"
#INPUT_FILE        = "/sciclone/home10/jrstevens01/2020-dirc/standalone/runs_72645_72749.txt"
INPUT_FILE        = "/sciclone/pscr/jrstevens01/TMPDIR/RunPeriod-2019-11/recon/ver01_pass03/merged/hd_root_sum_bar"
LUT_FILE          = "/sciclone/home10/jrstevens01/2020-dirc/standalone/lut_all_avr.root"

######################################################## ADD JOB #########################################################

def add_job(WORKFLOW, BAR, BIN):

	# PREPARE NAMES
	STUBNAME = "%02d_%02d" % (BAR, BIN)
	JOBNAME = WORKFLOW + "_%s" % STUBNAME

	# CREATE ADD-JOB COMMAND
        add_command = "qsub -l nodes=1:" + CLUSTER + ":ppn=" + NCORES + " -l walltime=8:00:00 -d " + RUNNING_DIR + "/bar%02d_bin%02d/" % (BAR, BIN)
        add_command += " -o " + DATA_OUTPUT_BASE_DIR + "/log/std." + JOBNAME + ".out" 
        add_command += " -e " + DATA_OUTPUT_BASE_DIR + "/log/std." + JOBNAME + ".err"
        add_command += " " + SCRIPTFILE
        add_command += " -v ENV_FILE=" + ENVFILE + ",INPUT_FILE=" + INPUT_FILE + "%02d.root,LUT_FILE=" % BAR + LUT_FILE + ",CODE_DIR=" + CODE_DIR  + ",OUT_DIR=" + DATA_OUTPUT_BASE_DIR + ",BAR=%d,BIN=%d" % (BAR, BIN)
        add_command += " -N reco_%d_%d" % (BAR, BIN)

	if(VERBOSE == True):
		print "job add command is \n" + str(add_command)

	# ADD JOB
        subprocess.call(["mkdir","-p",RUNNING_DIR + "/bar%02d_bin%02d/" % (BAR, BIN)])
	status = subprocess.call(add_command.split(" "))


########################################################## MAIN ##########################################################
	
def main(argv):
	parser_usage = "pbs_reco.py workflow"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) != 1):
		parser.print_help()
		return

	# GET ARGUMENTS
	WORKFLOW = args[0]

        subprocess.call(["mkdir","-p",RUNNING_DIR])
        subprocess.call(["mkdir","-p",DATA_OUTPUT_BASE_DIR])
        subprocess.call(["mkdir","-p",DATA_OUTPUT_BASE_DIR+"/log"])

	# ADD JOBS
	for BAR in range(0, 48):
                for BIN in range(0, 10):
		
                        print BAR, BIN
                        add_job(WORKFLOW, BAR, BIN)

if __name__ == "__main__":
   main(sys.argv[1:])

