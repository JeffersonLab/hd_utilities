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

#from datamon_db import datamon_db

RAWDATA_DIR = "/gluonraid1/rawdata/volatile/RunPeriod-2014-10/rawdata"
VERBOSE = True

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
            key = line[:11]
            sep = line[11:14]
            value = line[14:]

            properties[key] = value

    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
        return None
    except:
        print "Error: ", sys.exc_info()[0]
        return None

    # save information from file
    if "RUN CONFIG" in properties:
        run_properties["RUN CONFIG"] = properties["RUN CONFIG"]

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
        return None
    except:
        print "Error: ", sys.exc_info()[0]
        #return None

    print str(properties)

def process_logs(run):
    runfile = "%06d" % run
    run_properties = init_property_mapping()

    # check to see if the log directory exists
    logdir = join(RAWDATA_DIR,"Run"+runfile,"RunLog"+runfile)
    if not os.path.isdir(logdir):
        print "could not find log directory = " + logdir
        return

    comment_file_name = join(logdir,"run_%06d_comments.txt" % run)
    parse_comment_file(comment_file_name, run_properties)

    log_file_name = join(logdir,"current_run.log")
    parse_log_file(log_file_name,  run_properties)


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

    for run in runs_on_disk:
        process_logs(run)


if __name__ == "__main__":
    main(sys.argv[1:])
