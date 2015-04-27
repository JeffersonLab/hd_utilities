#!/usr/bin/env python
#
# 2014/12/7  Sean Dobbs (s-dobbs@northwestern.edu)
#
# Script for extracing condition information from run log files
#

import sys,os,time
from os import listdir
from os.path import isfile, join

import subprocess
from optparse import OptionParser
import xml.etree.ElementTree as ET

from math import fabs
import struct
from datetime import datetime

from datamon_db import datamon_db

#RAWDATA_DIR = "/gluonraid1/rawdata/volatile/RunPeriod-2014-10/rawdata"
#RAWDATA2_DIR = "/gluonraid2/rawdata/volatile/RunPeriod-2014-10/rawdata"
RAWDATA_DIR = "/gluonraid1/rawdata/volatile/RunPeriod-2015-03/rawdata"
#RAWDATA2_DIR = "/gluonraid2/rawdata/volatile/RunPeriod-2015-03/rawdata"
RAWDATA2_DIR = "/gluonraid1/rawdata/volatile/RunPeriod-2015-03/rawdata"
#VERBOSE = True

# set default values
def init_property_mapping():
    run_properties = {
        'trigger_config_file'  : '<None>',
    }
    return run_properties


# convert dates from "MM-DD-YY HH:MM:SS" -> "YYYY-MM-DD HH:MM:SS" 
def reformat_datetime(dtstr):
    try:
        (date,time) = dtstr.split()
        (month,day,year) = date.split('-')
        return "20%s-%s-%s %s" %(year,month,day,time)
    except:
        # if there's any problems, then just return what we were given
        return dtstr


def format_date(timestamp):
    dt = datetime.fromtimestamp(timestamp)
    return str(datetime(dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second))


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
        run_properties["start_time"] = reformat_datetime(properties["start_time"])
    if "end_time" in properties:
        run_properties["end_time"] = reformat_datetime(properties["end_time"])
    if "num_events" in properties:
        run_properties["num_events"] = properties["num_events"]
    return run_properties


def extract_epics_info(run_properties):
    EPICS_VARS = "IBCAD00CRCUR6,IPM5C11.VAL,IPM5C11A.VAL,IPM5C11B.VAL,IPM5C11C.VAL,MMSHLDE,HALLD:p,hd:collimator:motor.RBV,hd:converter:motor.RBV,HallD-PXI:Data:I_Shunt"
    EPICS_VARS += ",hd:converter_at_home,hd:converter_at_a,hd:converter_at_b,hd:converter_at_c"
    EPICS_VARS += ",hd:collimator_at_a,hd:collimator_at_b,hd:collimator_at_block"
    EPICS_VARS += ",hd:radiator_at_a,hd:radiator_at_b,hd:radiator_at_c"
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
        
    time.sleep(1.)  # delay so that we don't overload MYA archive
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
        if key == "hd:radiator_at_a":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["radiator_type"] = "2x10-5 RL"
        if key == "hd:radiator_at_b":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["radiator_type"] = "1x10-4 RL"
        if key == "hd:radiator_at_c":
            if abs(float(value) - 1.) < TOLERANCE:
                run_properties["radiator_type"] = "3x10-4 RL"
            

    return run_properties


######################################
# EVIO parsing stuff

def EVIO_SWAP64(x):
    return ( (((x) >> 56) & 0x00000000000000FFL) | \
                 (((x) >> 40) & 0x000000000000FF00L) | \
                 (((x) >> 24) & 0x0000000000FF0000L) | \
                 (((x) >> 8)  & 0x00000000FF000000L) | \
                 (((x) << 8)  & 0x000000FF00000000L) | \
                 (((x) << 24) & 0x0000FF0000000000L) | \
                 (((x) << 40) & 0x00FF000000000000L) | \
                 (((x) << 56) & 0xFF00000000000000L) )


def EVIO_SWAP32(x):
    return ( (((x) >> 24) & 0x000000FF) | \
                 (((x) >> 8)  & 0x0000FF00) | \
                 (((x) << 8)  & 0x00FF0000) | \
                 (((x) << 24) & 0xFF000000) )

# algorithm taken from evio2db
def ParseEVIOFiles(filelist):
    BUFFER_SIZE_WORDS = 4000  
    all_properties = []
    # should double check file names
    for fnamepath in filelist:
        print "processing " + fnamepath + "..."
        try:
            # extract file number & check 
            fname = fnamepath.split('/')[-1]
            if len(fname) < 6 or fname[-5:] != ".evio":
                print "bad file " + fname + " , skipping..."
                continue
            fparts = fname[:-5].split('_')
            if len(fparts) < 4:
                print "bad file " + fname + " , skipping..."
                continue
            try:
                file_num = int(fparts[3])
            except ValueError:
                print "bad file " + fname + " , skipping..."
                continue                

            filesize = os.path.getsize(fnamepath)
            f = open(fnamepath,"rb")
            # initialize values
            properties = {}
            properties["file_num"] = file_num
            # figure out the first event
            in_words = BUFFER_SIZE_WORDS
            if filesize < BUFFER_SIZE_WORDS*4:
                in_words = filesize/4
            data_in = f.read(4*in_words)
            data = struct.unpack(str(in_words)+"I", data_in)
            for i in xrange(len(data)):
                w = data[i]
                # Look for a "Go" event that we can use for the beginning of the run
                if( (w & 0x0001D2FF) == 0x0001D2FF ): 
                    properties["start_time"] = EVIO_SWAP32(data[i+1]);
                    if VERBOSE:
                        print "Found start time = " + format_date(properties["start_time"])
                # Physics Event Header bits that should be set
                if( (w & 0x001050FF) != 0x001050FF ):
                    continue
                # Physics Event Header bits that should not be set
                if( (w & 0x000F0E00) != 0x00000000 ):
                    continue
                # Jump 2 words to Trigger bank
                w = data[i+2];
                # Built Trigger Bank bits that should be set
                if( (w & 0x002020FF) != 0x002020FF ):
                    continue
                # First bank in Trigger bank should be 64bit int type
                w = data[i+3];
                if( (w & 0x00000A00) != 0x00000A00 ):
                    continue
                        
                properties["first_event"]  = EVIO_SWAP64( (long(data[i+5])<<32) | long(data[i+4]) )
                properties["tfirst_event"] = EVIO_SWAP64( (long(data[i+7])<<32) | long(data[i+6]) )
                break

            # figure out the last event - search through the file in chunks from the back
            n = 0  # number of chunks we have looked through
            last_event = 0
            while True:
                if(last_event > 0):
                    break
                n += 1
                f.seek(-n*4*in_words, 2)
                data_in = f.read(4*in_words)
                Nwords = len(data_in)/4
                data = struct.unpack(str(in_words)+"I", data_in)                
                #for i in xrange(len(data)):
                i = Nwords - 7
                while True:
                    i -= 1
                    if i < 0:
                        break
                    w = data[i]
                    # Physics Event Header bits that should be set
                    if( (w & 0x001050FF) != 0x001050FF ):
                        continue
                    # Physics Event Header bits that should not be set
                    if( (w & 0x000F0E00) != 0x00000000 ):
                        continue
                    # Jump 2 words to Trigger bank
                    w = data[i+2];
                    # Built Trigger Bank bits that should be set
                    if( (w & 0x002020FF) != 0x002020FF ):
                        continue
                    # First bank in Trigger bank should be 64bit int type
                    w = data[i+3];
                    if( (w & 0x00000A00) != 0x00000A00 ):
                        continue

                    last_event = EVIO_SWAP64( (long(data[i+5])<<32) | long(data[i+4]) )
                    properties["last_event"]  = last_event
                    properties["tlast_event"] = EVIO_SWAP64( (long(data[i+7])<<32) | long(data[i+6]) )
                    break

            # calculate results for a file
            # do a better job handling bad files?
            N = 0
            end_time = 0
            if "first_event" in properties and "last_event" in properties:    
                N = (properties["last_event"] - properties["first_event"]) + 1
                end_time = 5.0E-9 * (float(properties["tlast_event"]) - float(properties["tfirst_event"]))
            if N == 0:
                N = 1
            properties["num_events"] = N
            properties["end_time"] = end_time

            # we're all done!
            all_properties.append(properties)

        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
            return {}

        
    # build results
    file_properties = {}

    if VERBOSE:
        print "results from EVIO parsing = " + str(all_properties)
        print "processing: "

    file_properties["num_files"] = len(all_properties)
    file_properties["num_events"] = -1
    file_properties["start_time"] = -1
    file_properties["end_time"] = -1
    for props in sorted(all_properties, key=lambda prop: int(prop["file_num"])):
        if VERBOSE:
            print str(props)
        if file_properties["start_time"] < 0 :
            # if there are DAQ problems, the GO event might not be properly written out
            if "start_time" in props:
                file_properties["start_time"] = props["start_time"]
            else:
                file_properties["start_time"] = 0
            file_properties["end_time"] = file_properties["start_time"] + props["end_time"]
            file_properties["num_events"] = props["num_events"]
        else:
            file_properties["end_time"] += props["end_time"] 
            file_properties["num_events"] += props["num_events"] 
    file_properties["start_time"] = format_date(file_properties["start_time"])
    file_properties["end_time"] = format_date(file_properties["end_time"])
    return file_properties


######################################

def process_logs(db, run, d):
    runfile = "%06d" % run
    run_properties = init_property_mapping()

    # check to see if the log directory exists
    logdir = join(d,"RunLog"+runfile)
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

    # parse EVIO files to extract useful information
    rawdata_evio_dir = d
    if os.path.isdir(rawdata_evio_dir) :
        filelist = [ join(rawdata_evio_dir,f) for f in listdir(rawdata_evio_dir) if ((f[:10]=="hd_rawdata" or f[:6]=="hd_raw")and(f[-5:]=='.evio')) ]
        filelist.sort()
        file_properties = ParseEVIOFiles(filelist)
        if len(file_properties) > 0:
            run_properties['num_events'] = file_properties['num_events']
            run_properties['num_files'] = file_properties['num_files']
            run_properties['start_time'] = file_properties['start_time']
            run_properties['end_time'] = file_properties['end_time']

    #print str(run_properties)
    db.UpdateRunInfo(run, run_properties)


def main(argv):
    # allow us to run over just one run at a time
    MIN_RUN = -1
    MAX_RUN = 1000000
    parser = OptionParser(usage = "process_runlog_files.py [options]")
    parser.add_option("-b","--min_run", dest="min_run",
                      help="Minimum run number to process")
    parser.add_option("-e","--max_run", dest="max_run",
                      help="Maximum run number to process")
    (options, args) = parser.parse_args(argv)
    if options.min_run:
        MIN_RUN = int(options.min_run)
    if options.max_run:
        MAX_RUN = int(options.max_run)

    # run over all files
    dirs_on_disk  = [ join(RAWDATA_DIR,d) for d in listdir(RAWDATA_DIR) if os.path.isdir(join(RAWDATA_DIR,d)) ]
    dirs_on_disk += [ join(RAWDATA2_DIR,d) for d in listdir(RAWDATA2_DIR) if os.path.isdir(join(RAWDATA2_DIR,d)) ]
    #runs_on_disk = [ int(d[3:]) for d in dirs_on_disk ]
    runs_on_disk = {}
    for d in dirs_on_disk:
        try:
            path = d.split('/')
            run = int(path[-1][3:])
            runs_on_disk[run] = d
        except:
            print "invalid directory = " + d
    #runs_on_disk.sort()

    # Add information to DB
    ## initialize DB
    db = datamon_db()
    for (run,d) in sorted(runs_on_disk.items()):
        if run<MIN_RUN or run>MAX_RUN:
            continue
        print "processing run %s ..." % (run)
        ## add blank run to DB if it doesn't exist
        if(db.GetRunID(run) < 0):
            db.CreateRun(run)

        # get info from logs and EPICS archive and EVIO files
        process_logs(db,run,d)


if __name__ == "__main__":
    main(sys.argv[1:])
