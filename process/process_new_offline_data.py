#!/usr/bin/env python
#
# 2014/11/10  Sean Dobbs (s-dobbs@northwestern.edu)
#
# This script should be run after offline monitoring ROOT files are generated
# Runs through offline monitoring ROOT files, generates PNG images and inserts summary data to the website
#
# Takes three arguments:
#  
#  1. the date string that specifies when the monitoring jobs were started
#  2. the base directory where the monitoring files are stored
#     We assume a directory of the form /volatile/halld/RunPeriod-2014-10/offline_monitoring/RRRRRR/$DATE/
#  3. the directory to store the PNG files (and other associated processing files)
#

import sys,os,errno
from os import listdir
from os.path import isfile, join
from optparse import OptionParser

# monitoring libraries
from datamon_db import datamon_db
import make_monitoring_plots
import process_monitoring_data
import process_run_conditions

############################################
### GLOBALS
PROCESSED_RUN_LIST_FILE = "processedrun.lst"
VERSION_NUMBER  =  5   ## hardcode for now

MAKE_PLOTS = True
MAKE_DB_SUMMARY = True
MAKE_RUN_CONDITIONS = False

FORCE_PROCESSING = False
RUN_NUMBER = None
############################################

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise


### START PROCESSING

# read in command line args
parser = OptionParser(usage = "process_new_offline_data.py input_directory output_directory")
parser.add_option("-p","--disable_plots", dest="disable_plotting", action="store_true",
                  help="Don't make PNG files for web display")
parser.add_option("-d","--disable_summary", dest="disable_db_summary", action="store_true",
                  help="Don't calculate summary information and store it in the DB")
#parser.add_option("-c","--disable_conditions", dest="disable_run_conditions", action="store_true",
#                  help="Don't process and store run conditions information")
parser.add_option("-C","--process_conditions", dest="process_run_conditions", action="store_true",
                  help="Process and store run conditions information")
parser.add_option("-f","--force", dest="force", action="store_true",
                  help="Ignore list of already processed runs")
parser.add_option("-R","--run_number", dest="run_number", 
                  help="Process only this particular run number")

(options, args) = parser.parse_args(sys.argv)

if(len(args) < 3):
    parser.print_help()
    sys.exit(0)

RUN_DATE = args[1]
INPUT_DIRECTORY = args[2]
OUTPUT_DIRECTORY = args[3]

if(options.disable_plotting):
    MAKE_PLOTS = False
if(options.disable_db_summary):
    MAKE_DB_SUMMARY = False
#if(options.disable_run_conditions):
#    MAKE_RUN_CONDITIONS = False
if(options.process_run_conditions):
    MAKE_RUN_CONDITIONS = True
if(options.force):
    FORCE_PROCESSING = True
if(options.run_number):
    try:
        RUN_NUMBER = int(options.run_number)
    except ValueError:
        print "Invalid run number = " + options.run_number
        sys.exit(0)
    if RUN_NUMBER <= 0:
        print "Invalid run number = " + options.run_number
        sys.exit(0)

# check to see if the input directory is real
if not os.path.isdir(INPUT_DIRECTORY):
    print "Invalid input directory specified = " + INPUT_DIRECTORY
    sys.exit(0)

# make the output directiory if it doesn't already exist
if os.path.exists(OUTPUT_DIRECTORY) and not os.path.isdir(OUTPUT_DIRECTORY):
    print "File already exists and is not a directory = " + OUTPUT_DIRECTORY
    sys.exit(0)

if not os.path.exists(OUTPUT_DIRECTORY):
    print "Creating directory " + OUTPUT_DIRECTORY + " ... "
    os.system("mkdir -p " + OUTPUT_DIRECTORY)  ## need error checks
    

# allow for incremental processing ...
run_list = []
if not FORCE_PROCESSING and os.path.exists( join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE) ):
    # read in list of runs we've already processed
    try:
        runlist_file = open(join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE))
        for line in runlist_file:
            try:
                runnum = int(line.strip())
            except ValueError:
                print "Unexpected value in run file = " + line.strip() + " , skipping..."
            else:
                #print "processed run number = " + str(runnum)
                run_list.append( runnum )
        runlist_file.close()
    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    except:
        print "Unexpected error:", sys.exc_info()[0]
        sys.exit(0)


###
rundirs_on_disk = []
dirs_on_disk = [ d for d in listdir(INPUT_DIRECTORY) if os.path.isdir(join(INPUT_DIRECTORY,d,RUN_DATE)) ]
for dirname in sorted(dirs_on_disk):
    try:
        runnum = int(dirname)
    except ValueError:
        print "skipping directory " + dirname + " ..."
    else:
        if runnum not in run_list :
            #print "run number = " + str(runnum)
            if RUN_NUMBER is None:
                rundirs_on_disk.append(dirname)
            else:
                if runnum == RUN_NUMBER:
                    rundirs_on_disk.append(dirname)


# save processed runs 
try:
    runlist_file = open(join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE),'a')
except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
except:
    print "Unexpected error:", sys.exc_info()[0]
    sys.exit(0)

# initialize DB
db = datamon_db()

# do the heavy work for each directory - one run per directory
for rundir in rundirs_on_disk:
    runnum = int(rundir)    

    ## add blank run to DB if it doesn't exist
    if(db.GetRunID(runnum) < 0):
        db.CreateRun(runnum)
    ## TODO: create version info?    

    root_files = [ f for f in listdir(join(INPUT_DIRECTORY,rundir,RUN_DATE)) if (isfile(join(INPUT_DIRECTORY,rundir,RUN_DATE,f))and(f[-5:]=='.root')) ]
    
    os.system("mkdir -p " + join(OUTPUT_DIRECTORY,"rootfiles"))
    # check each file and extract which file number it corresponds to
    monitoring_files = open(join(INPUT_DIRECTORY,rundir,RUN_DATE,'rootfiles.txt'),"w")
    for fname in sorted(root_files):
        filenum = -1
        fname_fields = fname[:-5].split("_")
        # sanity checks
        if(len(fname_fields) < 4):
            print "invalid filename = " + fname + ", skipping ..."
            continue
        if( (fname_fields[0]!="hd") or (fname_fields[1]!="root") ):
            print "invalid filename = " + fname + ", skipping ..."
            continue
        try:
            file_runnum = int(fname_fields[2])
            filenum = int(fname_fields[3])
        except ValueError:
            print "invalid filename = " + fname + ", skipping ..."
            continue
        if file_runnum != runnum :
            print "invalid filename = " + fname + ", skipping ..."
            continue
        # save a list of the files to be processed
        print>>monitoring_files, join(INPUT_DIRECTORY,rundir,RUN_DATE,fname)
        # also copy the files so they can be viewed on the web
        os.system("cp " + join(INPUT_DIRECTORY,rundir,RUN_DATE,fname) + " " + join(OUTPUT_DIRECTORY,"rootfiles"))

        # we are good!  let's get some work done
        print "processing run " + str(runnum) + " file " + str(filenum) + " ..."

        #  process monitoring data for each file
        if MAKE_DB_SUMMARY:
            cmdargs  = "--file_number " + str(filenum) + " " + str(runnum) + " " + str(VERSION_NUMBER) + " " + join(INPUT_DIRECTORY,rundir,RUN_DATE,fname)
            print "  analyzing DB info..."
            print "process_monitoring_data " + cmdargs
            process_monitoring_data.main(cmdargs.split()) 
    monitoring_files.close()

    # make plots for the sum of all files in the run
    if MAKE_PLOTS:
        cmdargs  = " --histogram_list /u/home/gluex/halld/monitoring/process/histograms_to_monitor" 
        cmdargs += " --macro_list /u/home/gluex/halld/monitoring/process/macros_to_monitor "
        monitoring_data_dir = join(OUTPUT_DIRECTORY,("Run%06d" % runnum))
        #mkdir_p(monitoring_data_dir)
        os.system("mkdir -p " + monitoring_data_dir)  ## need error checks
        cmdargs += " --output_dir " + monitoring_data_dir
        cmdargs += " --file_list " + join(INPUT_DIRECTORY,rundir,RUN_DATE,'rootfiles.txt')
        print "  creating plots..."
        make_monitoring_plots.main(cmdargs.split())

    if MAKE_RUN_CONDITIONS:
        # update the run metadata
        cmdargs = str(runnum)
        print "  saving conditions..."
        process_run_conditions.main(cmdargs.split())

    ## we did process the run!
    if runnum not in run_list:
        run_list.append(runnum)
        print>>runlist_file, str(runnum)


# cleanup 
runlist_file.close()

    
