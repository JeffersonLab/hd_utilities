#!/usr/bin/env python
#
# 2014/11/3  Sean Dobbs (s-dobbs@northwestern.edu)
#
# Checks directory of online monitoring histograms 
# and processes any new ones
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
BASE_ONLINEMON_DIR = "/work/halld/online_monitoring"
PROCESSED_RUN_LIST_FILE = "processedrun.lst.online"
ONLINE_ROOT_DIR = BASE_ONLINEMON_DIR + '/root'
ONLINE_CONDITION_DIR = BASE_ONLINEMON_DIR + '/conditions'

MIN_RUN_NUMBER = 670
VERSION_NUMBER  =  1   ## hardcode for now
MONITORING_OUTPUT_DIR = "/work/halld/data_monitoring"
RUN_PERIOD = "RunPeriod-2014-10"

MAKE_PLOTS = True
MAKE_DB_SUMMARY = True
MAKE_RUN_CONDITIONS = True

############################################

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise


### START PROCESSING

parser = OptionParser(usage = "process_new_offline_data.py input_directory output_directory")
parser.add_option("-p","--disable_plots", dest="disable_plotting", action="store_true",
                  help="Don't make PNG files for web display")
parser.add_option("-d","--disable_summary", dest="disable_db_summary", action="store_true",
                  help="Don't calculate summary information and store it in the DB")
parser.add_option("-c","--disable_conditions", dest="disable_run_conditions", action="store_true",
                  help="Don't process and store run conditions information")

(options, args) = parser.parse_args(sys.argv)

if(options.disable_plotting):
    MAKE_PLOTS = False
if(options.disable_db_summary):
    MAKE_DB_SUMMARY = False
if(options.disable_run_conditions):
    MAKE_RUN_CONDITIONS = False

# read in list of runs we've already processed
run_list = []
try:
    runlist_file = open(PROCESSED_RUN_LIST_FILE)
    for line in runlist_file:
        try:
            runnum = int(line.strip())
        except ValueError:
            print "Unexpected value in run file = " + line.strip() + " , skipping..."
        else:
            run_list.append( runnum )
    runlist_file.close()
except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
except:
    print "Unexpected error:", sys.exc_info()[0]
    sys.exit(0)


# we decide if there's a new run by checking if a new online monitoring ROOT file has shown up
# load the list of ROOT files, making sure that they have the right suffix
# also, allow for a minimum run number to mask out early commissioning runs
files_on_disk = [ f for f in listdir(ONLINE_ROOT_DIR) if (isfile(join(ONLINE_ROOT_DIR,f))and(f[-5:]=='.root')) ]
#run_numbers_on_disk = [ (int(fname[13:18]),fname) for fname in files_on_disk if (int(fname[13:18])>=MIN_RUN_NUMBER) ]

# kludge for now, since the online ROOT files have stopped since run 1764, get a new run if a new condition file shows up
condition_files_on_disk = [ f for f in listdir(ONLINE_CONDITION_DIR) if (isfile(join(ONLINE_CONDITION_DIR,f))and(f[-4:]=='.dat')) ]
run_numbers_on_disk = [ (int(fname[15:20]),fname) for fname in condition_files_on_disk if (int(fname[15:20])>=MIN_RUN_NUMBER) ]
run_numbers_on_disk.sort(key=lambda runinfo: runinfo[0])

# save processed runs - overwrite save files
try:
    runlist_file = open(PROCESSED_RUN_LIST_FILE,'a')
except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
except:
    print "Unexpected error:", sys.exc_info()[0]
    sys.exit(0)

# do the heavy work
for (run,fname) in run_numbers_on_disk:
    ## if we haven't already processed the run, make the plots and add its info to the DB
    if run not in run_list:
        print "processing " + str(run) + "..."

        ## initialize DB
        db = datamon_db()
        ## add blank run to DB if it doesn't exist
        if(db.GetRunID(run) < 0):
            db.CreateRun(run)
        ## TODO: create version info?
        
        if MAKE_PLOTS:
            ## process monitoring data
            cmdargs  = " --histogram_list /u/home/gluex/halld/monitoring/process/histograms_to_monitor" 
            cmdargs += " --macro_list /u/home/gluex/halld/monitoring/process/macros_to_monitor "
            cmdargs += " --root_dir rootspy/"
            monitoring_data_dir = join(MONITORING_OUTPUT_DIR, RUN_PERIOD, ("Run%06d" % run))
            #mkdir_p(monitoring_data_dir)
            os.system("mkdir -p " + monitoring_data_dir)  ## need error checks
            cmdargs += "--output_dir " + monitoring_data_dir
            cmdargs += "  " + join(ONLINE_ROOT_DIR,fname)
            print "  creating plots..."
            make_monitoring_plots.main(cmdargs.split())

        if MAKE_DB_SUMMARY:
            cmdargs  = str(run) + " " + str(VERSION_NUMBER) + " " + join(ONLINE_ROOT_DIR,fname)
            print "  analyzing DB info..."
            process_monitoring_data.main(cmdargs.split()) 
        
        if MAKE_RUN_CONDITIONS:
            # update the run metadata
            cmdargs = str(run)
            print "  saving conditions..."
            process_run_conditions.main(cmdargs.split())

        ## we did process the run!
        run_list.append(run)
        # save this face
        print>>runlist_file, str(run)




    
