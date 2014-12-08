#!/usr/bin/env python
#
# 2014/12/7  Sean Dobbs (s-dobbs@northwestern.edu)
#
# Script for extracing condition information from run log files
#

import sys,os
from os import listdir
from os.path import isfile, join

from optparse import OptionParser
import xml.etree.ElementTree as ET

from datamon_db import datamon_db

RAWDATA_DIR = "/gluonraid1/rawdata/volatile/RunPeriod-2014-10/rawdata"
VERBOSE = False

# set default values
def init_property_mapping():
    run_properties = {
        'trigger_config_file'  : '<None>',
    }
    return run_properties


# get information from comment file 
# format of non-comment fields:  char(12), " : ", char(*)
# TODO: doublecheck run number
def parse_comment_file(fname, run_properties):
    properties = {}
    try:
        if VERBOSE:
            print "opening comment file = "  + fname
        infile = open(fname)
        for line in infile:
            if len(line)<15:
                continue
            key = line[:11].strip()
            sep = line[11:14]
            value = line[14:].strip()

            properties[key] = value

    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
        return run_properties
    except:
        print "Error: ", sys.exc_info()[0]
        #return None

    # save information from file
    if "RUN CONFIG" in properties:
        run_properties["trigger_config_file"] = properties["RUN CONFIG"]

    return run_properties

def parse_log_file(fname, run_properties):
    properties = {}
    try:
        if VERBOSE:
            print "opening log file = "  + fname

        tree = ET.parse(fname)
        root = tree.getroot()
        #ET.dump(root)

        run_start = root.find("run-start")
        #print "run-number = " + run_start.find("run-number").text
        #print "start-time = " + run_start.find("start-time").text
        properties["run_number"] = run_start.find("run-number").text
        properties["start_time"] = run_start.find("start-time").text.replace("/","-")

        run_end = root.find("run-end")
        properties["end_time"] = run_end.find("end-time").text.replace("/","-")
        properties["num_events"] = run_end.find("total-evt").text

    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
        return run_properties
    except:
        print "Error: ", sys.exc_info()[0]
        #return None

    if "start_time" in properties:
        run_properties["start_time"] = properties["start_time"]
    if "end_time" in properties:
        run_properties["end_time"] = properties["end_time"]
    if "num_events" in properties:
        run_properties["num_events"] = properties["num_events"]
    return run_properties

def process_logs(db, run):
    runfile = "%06d" % run
    run_properties = init_property_mapping()

    # check to see if the log directory exists
    logdir = join(RAWDATA_DIR,"Run"+runfile,"RunLog"+runfile)
    if not os.path.isdir(logdir):
        print "could not find log directory = " + logdir
        return

    comment_file_name = join(logdir,"run_%06d_comments.txt" % run)
    run_properties = parse_comment_file(comment_file_name, run_properties)

    log_file_name = join(logdir,"current_run.log")
    run_properties = parse_log_file(log_file_name,  run_properties)
    
    # for now, only update numbers of events if we haven't been able to extract them from the files
    nevents = db.GetNumEvents(run)
    if nevents >= 0 and "num_events" in run_properties:
        del run_properties["num_events"]

    #print str(run_properties)
    db.UpdateRunInfo(run, run_properties)

def main(argv):
    # allow us to run over just one run at a time


    # run over all files
    dirs_on_disk = [ d for d in listdir(RAWDATA_DIR) if os.path.isdir(join(RAWDATA_DIR,d)) ]
    #runs_on_disk = [ int(d[3:]) for d in dirs_on_disk ]
    runs_on_disk = []
    for d in dirs_on_disk:
        try:
            run = int(d[3:])
            runs_on_disk.append(run)
        except:
            print "invalid directory = " + d
    runs_on_disk.sort()

    # Add information to DB
    ## initialize DB
    db = datamon_db()
    for run in runs_on_disk:
        print "processing run %s ..." % (run)
        ## add blank run to DB if it doesn't exist
        if(db.GetRunID(run) < 0):
            db.CreateRun(run)

        process_logs(db,run)


if __name__ == "__main__":
    main(sys.argv[1:])
