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

# monitoring libraries
from datamon_db import datamon_db
import make_monitoring_plots
import process_monitoring_data

############################################
### GLOBALS
BASE_ONLINEMON_DIR = "/work/halld/online_monitoring"
PROCESSED_RUN_LIST_FILE = "processedrun.lst"
ONLINE_ROOT_DIR = BASE_ONLINEMON_DIR + '/root'

MIN_RUN_NUMBER = 750
VERSION_NUMBER  =  -1   ## hardcode for now
MONITORING_OUTPUT_DIR = "/work/halld/data_monitoring"
RUN_PERIOD = "RunPeriod-2014-10"

############################################

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise


### START PROCESSING

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
            print "run number = " + str(runnum)
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
run_numbers_on_disk = [ (int(fname[13:18]),fname) for fname in files_on_disk if (int(fname[13:18])>=MIN_RUN_NUMBER) ]
run_numbers_on_disk.sort(key=lambda runinfo: runinfo[0])

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
        
        ## process monitoring data
        cmdargs  = "--histogram_list monitoring_histograms_to_plot --macro_list monitoring_macros_to_plot "
        monitoring_data_dir = join(MONITORING_OUTPUT_DIR, RUN_PERIOD, ("Run%06d" % run))
        mkdir_p(monitoring_data_dir)
        cmdargs += "--output_dir " + monitoring_data_dir
        cmdargs += join(ONLINE_ROOT_DIR,fname)
        make_monitoring_plots.main(cmdargs.split())
        cmdargs  = str(run) + " " + str(VERSION_NUMBER) + " 0 " + join(ONLINE_ROOT_DIR,fname)
        process_monitoring_data.main(cmdargs.split()) 
        
        ## we did process the run!
        run_list.append(run)


# save processed runs - overwrite save files
try:
    runlist_file = open(PROCESSED_RUN_LIST_FILE,'w')
    for run in sorted(run_list): 
        print>>runlist_file, str(run)
    runlist_file.close()
except IOError as e:
    print "I/O error({0}): {1}".format(e.errno, e.strerror)
except:
    print "Unexpected error:", sys.exc_info()[0]
    sys.exit(0)


    
