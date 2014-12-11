#!/usr/bin/env python
#
# 2014/12/7  Sean Dobbs (s-dobbs@northwestern.edu)
#
# Script for extracing condition information from run log files
#

import sys,os
from os import listdir
from os.path import isfile, join

import subprocess
from optparse import OptionParser
import xml.etree.ElementTree as ET

from datamon_db import datamon_db

RAWDATA_DIR = "/gluonraid1/rawdata/volatile/RunPeriod-2014-10/rawdata"
VERBOSE = True

# set default values
def init_property_mapping():
    run_properties = {
        'trigger_config_file'  : '<None>',
    }
    return run_properties


# convert dates from "MM-DD-YY HH:MM:SS" -> "YYYY-MM-DD HH:MM:SS" 
def format_datetime(dtstr):
    try:
        (date,time) = dtstr.split()
        (month,day,year) = date.split('-')
        return "20%s-%s-%s %s" %(year,month,day,time)
    except:
        # if there's any problems, then just return what we were given
        return dtstr

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
        run_properties["start_time"] = format_datetime(properties["start_time"])
    if "end_time" in properties:
        run_properties["end_time"] = format_datetime(properties["end_time"])
    if "num_events" in properties:
        run_properties["num_events"] = properties["num_events"]
    return run_properties


def extract_epics_info(run_properties):
    EPICS_VARS = "IBCAD00CRCUR6,IPM5C11.VAL,IPM5C11A.VAL,IPM5C11B.VAL,IPM5C11C.VAL,MMSHLDE,HALLD:p,hd:collimator:motor.RBV,hd:converter:motor.RBV,HallD-PXI:Data:I_Shunt"
    EPICS_VARS += ",hd:converter_at_home,hd:converter_at_a,hd:converter_at_b,hd:converter_at_c"
    EPICS_VARS += ",hd:collimator_at_a,hd:collimator_at_b,hd:collimator_at_block"
    run_endtime = "^10m"     ## if we couldn't figure out when the run ended, assume it was short and integrate variables over a 10 minute span
    TOLERANCE = 0.05         ## 5% tolerance for checking if EPICS var is equal to 1 - needed since we are averaging over run

    # don't do anything if we don't have a start time
    if not "start_time" in run_properties:
        return run_properties

    # average EPICS variables over the whole run
    if "end_time" in run_properties:
        run_endtime = run_properties["end_time"]

    cmds = []
    cmds.append("myStats")
    cmds.append("-b")
    cmds.append(run_properties["start_time"])
    cmds.append("-e")
    cmds.append(run_endtime)
    cmds.append("-l")
    cmds.append(EPICS_VARS)

    if VERBOSE:
        print "MYA CMD = " + " ".join(cmds)

    p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
    for line in p.stdout:
        print line.strip()
        tokens = line.strip().split()
        if len(tokens) < 3:
            continue
        key = tokens[0]
        value = tokens[2]    ## average value
        if value == "N/A":
            continue
        if key == "IBCAD00CRCUR6":
            run_properties["beam_current"] = value
        if key == "HALLD:p":
            run_properties["beam_energy"] = str(float(value)/1000.)
        if key == "HallD-PXI:Data:I_Shunt":
            run_properties["solenoid_current"] = value
        if key == "hd:converter_at_home":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["ps_converter"] = "Retracted"
        if key == "hd:converter_at_a":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["ps_converter"] = "1x10-3 RL"
        if key == "hd:converter_at_b":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["ps_converter"] = "3x10-4 RL"
        if key == "hd:converter_at_c":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["ps_converter"] = "5x10-3 RL"
        if key == "hd:collimator_at_block":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["collimator_diameter"] = "Blocking"
        if key == "hd:collimator_at_a":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["collimator_diameter"] = "3.4mm hole"
        if key == "hd:collimator_at_b":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["collimator_diameter"] = "5.0mm hole"
            

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
    
    run_properties = extract_epics_info(run_properties)
    
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
