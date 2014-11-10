#!/usr/bin/env python
#
# 2014/11/10  Sean Dobbs (s-dobbs@northwestern.edu)
#
# This script should be run after offline monitoring ROOT files are generated
# Runs through offline monitoring ROOT files, generates PNG images and inserts summary data to the website
#
# Takes two arguments:
#  1. the directory to find the monitoring ROOT files
#  2. the directory to store the PNG files (and other associated processing files)
#

import sys,os,errno
from os import listdir
from os.path import isfile, join
from optparse import OptionParser

# monitoring libraries
from datamon_db import datamon_db
import make_monitoring_plots
import process_monitoring_data

############################################
### GLOBALS
PROCESSED_RUN_LIST_FILE = "processedrun.lst"
VERSION_NUMBER  =  2   ## hardcode for now

MAKE_PLOTS = True
MAKE_DB_SUMMARY = True
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
parser = OptionParser(usage = "process_monitoring_data.py [options] run_number version_number file.root")
parser.add_option("-p","--disable_plots", dest="disable_plotting", action="store_true",
                  help="Don't make PNG files for web display")
parser.add_option("-d","--disable_summary", dest="disable_db_summary", action="store_true",
                  help="Don't calculate summary information and store it in the DB")

(options, args) = parser.parse_args(sys.argv)

if(len(args) < 3):
    parser.print_help()
    sys.exit(0)

INPUT_DIRECTORY = args[1]
OUTPUT_DIRECTORY = args[2]

if(options.disable_plotting):
    MAKE_PLOTS = False
if(options.disable_db_summary):
    MAKE_DB_SUMMARY = False

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
if os.path.exists( join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE) ):
    # read in list of runs we've already processed
    try:
        runlist_file = open(join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE))
        for line in runlist_file:
            try:
                runnum = int(line.strip())
            except ValueError:
                print "Unexpected value in run file = " + line.strip() + " , skipping..."
            else:
                print "processed run number = " + str(runnum)
                run_list.append( runnum )
        runlist_file.close()
    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    except:
        print "Unexpected error:", sys.exc_info()[0]
        sys.exit(0)


###
rundirs_on_disk = []
dirs_on_disk = [ d for d in listdir(INPUT_DIRECTORY) if os.path.isdir(join(INPUT_DIRECTORY,d)) ]
for dirname in sorted(dirs_on_disk):
    try:
        runnum = int(dirname)
    except ValueError:
        print "skipping directory " + dirname + " ..."
    else:
        if runnum not in run_list :
            print "run number = " + str(runnum)
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

    root_files = [ f for f in listdir(join(INPUT_DIRECTORY,rundir)) if (isfile(join(INPUT_DIRECTORY,rundir,f))and(f[-5:]=='.root')) ]
    monitoring_root_files = []

    # check each file and extract which file number it corresponds to
    monitoring_files = open(join(INPUT_DIRECTORY,rundir,'rootfiles.txt'),"w")
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
        print>>monitoring_files, join(INPUT_DIRECTORY,rundir,fname)

        # we are good!  let's get some work done
        print "processing run " + str(runnum) + " file " + str(filenum) + " ..."

        #  process monitoring data for each file
        if MAKE_DB_SUMMARY:
            cmdargs  = "--file_number " + str(filenum) + " " + str(runnum) + " " + str(VERSION_NUMBER) + " " + join(INPUT_DIRECTORY,rundir,fname)
            print "  analyzing DB info..."
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
        cmdargs += " --file_list " + join(INPUT_DIRECTORY,rundir,'rootfiles.txt')
        print "  creating plots..."
        make_monitoring_plots.main(cmdargs.split())

    ## we did process the run!
    run_list.append(runnum)
    # save this face
    print>>runlist_file, str(runnum)


# cleanup 
runlist_file.close()

    
