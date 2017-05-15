#!/usr/bin/env python
##########################################################################################################################
#
# 2017/03 Thomas Britton
#
#   Options:
#      MC variation can be changed by supplying "variation=xxxxx" option otherwise default: mc
#      the number of events to be generated per file (except for any remainder) can be set by "per_file=xxxx" default: 1000
#
#      If the user does not want genr8, geant, smearing, reconstruction to be performed the sequence will be terminated at the first instance of genr8=0,geant=0,mcsmear=0,recon=0 default: all on
#      Similarly, if the user wishes to retain the files created by any step you can supply the cleangenr8=0, cleangeant=0, cleanmcsmear=0, or cleanrecon=0 options.  By default all but the reconstruction files #      are cleaned. 
#
#      The reconstruction step is multi-threaded, for this step, if enabled, the script will use 4 threads.  This threading can be changed with the "numthreads=xxx" option 
#
#      By default the job will run interactively in the local directory.  If the user wishes to submit the jobs to swif the option "swif=1" must be supplied.
#
# SWIF DOCUMENTATION:
# https://scicomp.jlab.org/docs/swif
# https://scicomp.jlab.org/docs/swif-cli
# https://scicomp.jlab.org/help/swif/add-job.txt #consider phase!
#
##########################################################################################################################
from os import environ
from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
from subprocess import call
import glob

def add_job(WORKFLOW, RUNNO, FILENO,SCRIPT,COMMAND, VERBOSE,PROJECT,TRACK,NCORES,DISK,RAM,TIMELIMIT,OS,DATA_OUTPUT_BASE_DIR):

        
	# PREPARE NAMES
	STUBNAME = str(RUNNO) + "_" + str(FILENO)
	JOBNAME = WORKFLOW + "_" + STUBNAME

	# CREATE ADD-JOB COMMAND
	# job
        #try removing the name specification
	add_command = "swif add-job -workflow " + WORKFLOW #+ " -name " + JOBNAME
	# project/track
	add_command += " -project " + PROJECT + " -track " + TRACK
	# resources
	add_command += " -cores " + NCORES + " -disk " + DISK + " -ram " + RAM + " -time " + TIMELIMIT + " -os " + OS
	# stdout
	add_command += " -stdout " + DATA_OUTPUT_BASE_DIR + "/log/" + str(RUNNO) + "_stdout." + STUBNAME + ".out"
	# stderr
	add_command += " -stderr " + DATA_OUTPUT_BASE_DIR + "/log/" + str(RUNNO) + "_stderr." + STUBNAME + ".err"
	# tags
	add_command += " -tag run_number " + str(RUNNO)
	# tags
	add_command += " -tag file_number " + str(FILENO)
	# script with options command
	add_command += " "+SCRIPT  +" "+ COMMAND

	if(VERBOSE == True):
		print "job add command is \n" + str(add_command)

        if(int(NCORES)==1 and int(RAM[:-2]) >= 10 and RAM[-2:]=="GB"):
                print "SciComp has a limit on RAM requested per thread, as RAM is the limiting factor."
                print "This will likely cause an AUGER-SUBMIT error."
                print "Please either increase NCORES or decrease RAM requested and try again."
                exit(1)
	# ADD JOB
	status = subprocess.call(add_command.split(" "))
		
def showhelp():
        helpstring= "variation=%s where %s is a valid jana_calib_context variation string (default is \"mc\")\n"
        helpstring+= " per_file=%i where %i is the number of events you want per file/job (default is 10000)\n"
        helpstring+= " numthreads=%i sets the number of threads to use to %i.  Note that this will overwrite the NCORES set in MC.config\n"
        helpstring+= " generate=[0/1] where 0 means that the generation step and any subsequent step will not run (default is 1)\n"
        helpstring+= " geant=[0/1] where 0 means that the geant step and any subsequent step will not run (default is 1)\n"
        helpstring+= " mcsmear=[0/1] where 0 means that the mcsmear step and any subsequent step will not run (default is 1)\n"
        helpstring+= " recon=[0/1] where 0 means that the reconstruction step will not run (default is 1)\n"
        helpstring+= " cleangenerate=[0/1] where 0 means that the generation step will not be cleaned up after use (default is 1)\n"
        helpstring+= " cleangeant=[0/1] where 0 means that the geant step will not be cleaned up after use (default is 1)\n"
        helpstring+= " cleanmcsmear=[0/1] where 0 means that the mcsmear step will not be cleaned up after use (default is 1)\n"
        helpstring+= " cleanrecon=[0/1] where 0 means that the reconstruction step will not run (default is 1)\n"
        helpstring+= " swif=[0/1] where 1 means that a workflow will be created and jobs added to it (default is 0)\n"
        return helpstring

########################################################## MAIN ##########################################################
	
def main(argv):
	parser_usage = "gluex_MC.py config_file Run_Number num_events [all other options]\n\n where [all other options] are:\n\n "
        parser_usage += showhelp()
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	#check if there are enough arguments
	if(len(argv)<3):
		parser.print_help()
		return

	#check if the needed arguments are valid
	if len(args[1].split("="))>1 or len(args[2].split("="))>1:
		parser.print_help()
		return

        #!!!!!!!!!!!!!!!!!!REQUIRED COMMAND LINE ARGUMENTS!!!!!!!!!!!!!!!!!!!!!!!!
        CONFIG_FILE = args[0]
        RUNNUM = int(args[1])
	EVTS = int(args[2])
        #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        print "*********************************"
        print "Welcome to v1.5.1 of the MCwrapper"
        print "Thomas Britton 05/03/17"
        print "*********************************"


	#load all argument passed in and set default options
        VERBOSE    = False

        TAGSTR="I_dont_have_one"

        DATA_OUTPUT_BASE_DIR    = "UNKNOWN_LOCATION"#your desired output location (only needed for SWIF jobs
        
        ENVFILE = "my-environment-file"#change this to your own environment file
        
        GENERATOR = "genr8"
        GENCONFIG = "NA"

        eBEAM_ENERGY="12"
        COHERENT_PEAK="9"
        MIN_GEN_ENERGY="4"
        MAX_GEN_ENERGY="12"
        

        GEANTVER = 4        
        BGFOLD="DEFAULT"



        CUSTOM_MAKEMC="DEFAULT"
        CUSTOM_GCONTROL="0"
        CUSTOM_PLUGINS="None"

        #-------SWIF ONLY-------------
        # PROJECT INFO
        PROJECT    = "gluex"          # http://scicomp.jlab.org/scicomp/#/projects
        TRACK      = "simulation"     # https://scicomp.jlab.org/docs/batch_job_tracks
        
        # RESOURCES for swif jobs
        NCORES     = "8"               # Number of CPU cores
        DISK       = "10GB"            # Max Disk usage
        RAM        = "20GB"            # Max RAM usage
        TIMELIMIT  = "300minutes"      # Max walltime
        OS         = "centos7"        # Specify CentOS65 machines

        #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        VERSION  = "mc"
	PERFILE=10000
	GENR=1
	GEANT=1
	SMEAR=1
	RECON=1
	CLEANGENR=1
	CLEANGEANT=1
	CLEANSMEAR=1
	CLEANRECON=0
        MCSWIF=0
        #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        #loop over config file and set the "parameters"
        f = open(CONFIG_FILE,"r")

        for line in f:
                if len(line)==0:
                       continue
                if line[0]=="#":
                       continue

                parts=line.split("=")

                if len(parts)==1:
                        #print "Warning! No Sets given"
                        continue
                
                if len(parts)>2:
                        print "warning! I am going to have a really difficult time with:"
                        print line
                        print "I'm going to just ignore it and hope it isn't a problem...."
                        continue
                        
                        
                rm_comments=[]
                if len(parts)>1:
                        rm_comments=parts[1].split("#")
                        
                j=-1
                for i in parts:
                        j=j+1
                        i=i.strip()
                        parts[j]=i
                
                if str(parts[0]).upper()=="VERBOSE" :
                        if rm_comments[0].strip().upper()=="TRUE" or rm_comments[0].strip() == "1":
                                VERBOSE=True
                elif str(parts[0]).upper()=="PROJECT" :
                        PROJECT=rm_comments[0].strip()
                elif str(parts[0]).upper()=="TRACK" :
                        TRACK=rm_comments[0].strip()
                elif str(parts[0]).upper()=="NCORES" :
                        NCORES=rm_comments[0].strip()
                elif str(parts[0]).upper()=="DISK" :
                        DISK=rm_comments[0].strip()
                elif str(parts[0]).upper()=="RAM" :
                        RAM=rm_comments[0].strip()
                elif str(parts[0]).upper()=="TIMELIMIT" :
                        TIMELIMIT=rm_comments[0].strip()
                elif str(parts[0]).upper()=="OS" :
                        OS=rm_comments[0].strip()
                elif str(parts[0]).upper()=="DATA_OUTPUT_BASE_DIR" :
                        DATA_OUTPUT_BASE_DIR=rm_comments[0].strip()
                elif str(parts[0]).upper()=="ENVIRONMENT_FILE" :
                        ENVFILE=rm_comments[0].strip()
                elif str(parts[0]).upper()=="GENERATOR" :
                        GENERATOR=rm_comments[0].strip()
                elif str(parts[0]).upper()=="GEANT_VERSION" :
                        GEANTVER=rm_comments[0].strip()
                elif str(parts[0]).upper()=="WORKFLOW_NAME" :
                        WORKFLOW=rm_comments[0].strip()
                elif str(parts[0]).upper()=="GENERATOR_CONFIG" :
                        GENCONFIG=rm_comments[0].strip()
                elif str(parts[0]).upper()=="CUSTOM_MAKEMC" :
                        CUSTOM_MAKEMC=rm_comments[0].strip()
                elif str(parts[0]).upper()=="CUSTOM_GCONTROL" :
                        CUSTOM_GCONTROL=rm_comments[0].strip()
                elif str(parts[0]).upper()=="BKG" :
                        BGFOLD=rm_comments[0].strip()
                elif str(parts[0]).upper()=="EBEAM_ENERGY" :
                        eBEAM_ENERGY=rm_comments[0].strip()
                elif str(parts[0]).upper()=="COHERENT_PEAK" :
                        COHERENT_PEAK=rm_comments[0].strip()
                elif str(parts[0]).upper()=="GEN_MIN_ENERGY" :
                        MIN_GEN_ENERGY=rm_comments[0].strip()
                elif str(parts[0]).upper()=="GEN_MAX_ENERGY" :
                        MAX_GEN_ENERGY=rm_comments[0].strip()
                elif str(parts[0]).upper()=="TAG" :
                        TAGSTR=rm_comments[0].strip()
                elif str(parts[0]).upper()=="CUSTOM_PLUGINS" :
                        CUSTOM_PLUGINS=rm_comments[0].strip()
                else:
                        print "unknown config parameter!! "+str(parts[0])
	#loop over command line arguments 
	
        for argu in args:
		argfound=0
		flag=argu.split("=")
		#redundat check to jump over the first 4 arguments
		if(len(flag)<2):
			continue
		else:#toggle the flags as user defines
			if flag[0]=="variation":
				argfound=1
				VERSION=flag[1]
			if flag[0]=="per_file":
				argfound=1
				PERFILE=int(flag[1])
			if flag[0]=="genr8":
				argfound=1
				GENR=int(flag[1])
			if flag[0]=="geant":
				argfound=1
				GEANT=int(flag[1])
			if flag[0]=="mcsmear":
				argfound=1
				SMEAR=int(flag[1])
			if flag[0]=="recon":
				argfound=1
				RECON=int(flag[1])
			if flag[0]=="cleangenerate":
				argfound=1
				CLEANGENR=int(flag[1])
			if flag[0]=="cleangeant":
				argfound=1
				CLEANGEANT=int(flag[1])
			if flag[0]=="cleanmcsmear":
				argfound=1
				CLEANSMEAR=int(flag[1])
			if flag[0]=="cleanrecon":
				argfound=1
				CLEANRECON=int(flag[1])
			if flag[0]=="swif":
				argfound=1
				MCSWIF=int(flag[1])
			if flag[0]=="numthreads":
				argfound=1
				NCORES=str(flag[1])
			if argfound==0:
				print "WARNING OPTION: "+argu+" NOT FOUND!"

	
      #  if str(GEANTVER)=="3":
      #          print "!!!  Warning: Geant 3 detected! NumThreads has been set to 1"
      #          print "!!!  This is done to ensure efficient use of resources while running and should provide faster job starts."
      #          NCORES="2"
      #          print ""
                
        if DATA_OUTPUT_BASE_DIR == "UNKNOWN_LOCATION" and MCSWIF==1:
                print "I doubt that SWIF will find "+DATA_OUTPUT_BASE_DIR+" so I am saving you the embarassment and stopping this"
                return

        name_breakdown=GENCONFIG.split("/")
        CHANNEL = name_breakdown[len(name_breakdown)-1].split(".")[0]

	#print a line indicating SWIF or Local run
	if MCSWIF != 1:
		print "Locally simulating "+args[2]+" "+CHANNEL+" Events"
	else:
		print "Creating "+WORKFLOW+" to simulate "+args[2]+" "+CHANNEL+" Events"
	# CREATE WORKFLOW
		status = subprocess.call(["swif", "create", "-workflow", WORKFLOW])

	#calculate files needed to gen
	FILES_TO_GEN=EVTS/PERFILE
	REMAINING_GEN=EVTS%PERFILE

	indir=os.environ.get('MCWRAPPER_CENTRAL')
        
        script_to_use = "/MakeMC.csh"
        if environ['SHELL']=="/bin/bash" :
                script_to_use = "/MakeMC.sh"
        
        indir+=script_to_use

        if len(CUSTOM_MAKEMC)!= 0 and CUSTOM_MAKEMC != "DEFAULT":
                indir=CUSTOM_MAKEMC

        

        if str(indir) == "None":
                print "MCWRAPPER_CENTRAL not set"
                return

	outdir=DATA_OUTPUT_BASE_DIR
	
        #if local run set out directory to cwd
	if outdir[len(outdir)-1] != "/" :
                outdir+= "/"

	#for every needed file call the script with the right options
	for FILENUM in range(1, FILES_TO_GEN + 2):
		num=PERFILE
		#last file gets the remainder
		if FILENUM == FILES_TO_GEN +1:
			num=REMAINING_GEN
		#if ever asked to generate 0 events....just don't
		if num == 0:
			continue
                
		COMMAND=ENVFILE+" "+GENCONFIG+" "+str(outdir)+" "+str(RUNNUM)+" "+str(FILENUM-1)+" "+str(num)+" "+str(VERSION)+" "+str(GENR)+" "+str(GEANT)+" "+str(SMEAR)+" "+str(RECON)+" "+str(CLEANGENR)+" "+str(CLEANGEANT)+" "+str(CLEANSMEAR)+" "+str(CLEANRECON)+" "+str(MCSWIF)+" "+str(NCORES)+" "+str(GENERATOR)+" "+str(GEANTVER)+" "+str(BGFOLD)+" "+str(CUSTOM_GCONTROL)+" "+str(eBEAM_ENERGY)+" "+str(COHERENT_PEAK)+" "+str(MIN_GEN_ENERGY)+" "+str(MAX_GEN_ENERGY)+" "+str(TAGSTR)+" "+str(CUSTOM_PLUGINS)+" "+str(PERFILE)

               
		#print COMMAND
		#either call MakeMC.csh or add a job depending on swif flag
                if MCSWIF == 0:
			os.system(str(indir)+" "+COMMAND)
		else:
			add_job(WORKFLOW, RUNNUM, FILENUM,str(indir),COMMAND,VERBOSE,PROJECT,TRACK,NCORES,DISK,RAM,TIMELIMIT,OS,DATA_OUTPUT_BASE_DIR)

if __name__ == "__main__":
   main(sys.argv[1:])
