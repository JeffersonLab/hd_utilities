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
#     We assume a directory of the form /volatile/halld/RunPeriod-2014-10/offline_monitoring/$DATE/$DATATYPE/RRRRRR/
#  3. the directory to store the PNG files (and other associated processing files)
#

import sys,os,errno
from os import listdir
from os.path import isfile, join
from optparse import OptionParser

from ROOT import gROOT

# monitoring libraries
from datamon_db import datamon_db
import make_monitoring_plots
import process_monitoring_data
import process_run_conditions

############################################
### GLOBALS
#PROCESSED_RUN_LIST_FILE = "processedrun.lst"
ROOTFILE_DIR = "ROOT"
VERSION_NUMBER  =  -1

MAKE_PLOTS = True
MAKE_DB_SUMMARY = True
MAKE_RUN_CONDITIONS = False

FORCE_PROCESSING = False
RUN_NUMBER = None

NEWDIR_MODE = "775"

MIN_RUN = -1
MAX_RUN = 1000000
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
parser.add_option("-V","--version_number", dest="version_number", 
                  help="Save summary results with this DB version ID")
parser.add_option("-v","--version", dest="version_string",
                  help="Save summary results with a particular data version, specified using the string \"RunPeriod,Revision\", e.g., \"RunPeriod-2014-10,5\"")
parser.add_option("-b","--min_run", dest="min_run",
                  help="Minimum run number to process")
parser.add_option("-e","--max_run", dest="max_run",
                  help="Maximum run number to process")

(options, args) = parser.parse_args(sys.argv)

if(len(args) < 3):
    parser.print_help()
    sys.exit(0)

REVISION = args[1]
INPUT_DIRECTORY = args[2]
OUTPUT_DIRECTORY = args[3]

# initialize DB
db = datamon_db()

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
if(options.version_string):
    try: 
        revision = -1
        (run_period,revision_str) = options.version_string.split(",")
        try:
            revision = int(revision_str)
        except ValueError:
            print "Invalid revision = " + revision
            sys.exit(0)
        VERSION_NUMBER = db.GetVersionIDRunPeriod(run_period, revision)
        if(VERSION_NUMBER<0):
            print "version not found in DB = " + options.version_string
            sys.exit(0)
    except:
        print "Invalid version specification = " + options.version_string
        sys.exit(0)
    print "Configured RunPeriod = %s  Revision = %d  ->  VersionID = %d" % (run_period,revision,VERSION_NUMBER)
if(options.version_number):
    try:
        VERSION_NUMBER = int(options.version_number) 
    except ValueError:
        print "Invalid version number = " + options.version_number
        sys.exit(0)
    if VERSION_NUMBER <= 0:
        print "Invalid version number = " + options.version_number
        sys.exit(0)
if options.min_run:
    MIN_RUN = int(options.min_run)
if options.max_run:
    MAX_RUN = int(options.max_run)


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
    os.system("mkdir -m"+NEWDIR_MODE+" -p " + OUTPUT_DIRECTORY)  ## need error checks

# see if there are libraries to load
LIBDIR = os.getenv("MONITORING_LIBDIR", join(os.getcwd(),"lib"))    
lib_macros = [ join(LIBDIR,f) for f in listdir(LIBDIR) if f[-2:]=='.C' ]
for macro in lib_macros:
    gROOT.ProcessLine(".L "+macro)

# allow for incremental processing ...
#run_list = []
#if not FORCE_PROCESSING and os.path.exists( join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE) ):
#    # read in list of runs we've already processed
#    try:
#        runlist_file = open(join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE))
#        for line in runlist_file:
#            try:
#                runnum = int(line.strip())
#            except ValueError:
#                print "Unexpected value in run file = " + line.strip() + " , skipping..."
#            else:
#                #print "processed run number = " + str(runnum)
#                run_list.append( runnum )
#        runlist_file.close()
#    except IOError as e:
#        print "I/O error({0}): {1}".format(e.errno, e.strerror)
#    except:
#        print "Unexpected error:", sys.exc_info()[0]
#        sys.exit(0)


###
rundirs_on_disk = []
dirs_on_disk = [ d for d in listdir(join(INPUT_DIRECTORY,REVISION,ROOTFILE_DIR)) if os.path.isdir(join(INPUT_DIRECTORY,REVISION,ROOTFILE_DIR,d)) ]
for dirname in sorted(dirs_on_disk):
    try:
        runnum = int(dirname)
    except ValueError:
        print "skipping directory " + dirname + " ..."
    else:
        #if runnum not in run_list :
        #print "run number = " + str(runnum)
        if RUN_NUMBER is None:
            rundirs_on_disk.append(dirname)
        else:
            if runnum == RUN_NUMBER:
                rundirs_on_disk.append(dirname)


# save processed runs 
#try:
#    runlist_file = open(join(OUTPUT_DIRECTORY,PROCESSED_RUN_LIST_FILE),'a')
#except IOError as e:
#    print "I/O error({0}): {1}".format(e.errno, e.strerror)
#except:
#    print "Unexpected error:", sys.exc_info()[0]
#    sys.exit(0)


# do the heavy work for each directory - one run per directory
for rundir in rundirs_on_disk:
    runnum = int(rundir)    
    if runnum<MIN_RUN or runnum>MAX_RUN:
        continue

    print "checking run " + str(runnum)

    ## add blank run to DB if it doesn't exist
    if(db.GetRunID(runnum) < 0):
        db.CreateRun(runnum)

    rootfilespath = join(INPUT_DIRECTORY,REVISION,ROOTFILE_DIR)
    root_files = [ join(rootfilespath,rundir,f) for f in listdir(join(rootfilespath,rundir)) if (isfile(join(rootfilespath,rundir,f))and(f[-5:]=='.root')) ]

    # add directory if it doesn't exist
    misc_dir = join(INPUT_DIRECTORY,REVISION,"misc",rundir)
    if not os.path.exists(misc_dir):
        os.system("mkdir -p " + misc_dir)
    if not os.path.isdir(misc_dir):
        print "file %s exists and is not a directory, skipping this run ..."%misc_dir
    rootfilelist_fname = join(misc_dir,"rootfiles.txt")

    ## only run over files that haven't already been processed
    ## we use the "rootfiles.txt" file as a store of which files have been already processed
    new_files_exist = False
    monitoring_files = {}
    new_monitoring_files = {}
    processed_files = []
    if isfile(rootfilelist_fname):
        rootfilelist_file = open(rootfilelist_fname)
        processed_files = rootfilelist_file.read().splitlines()
        rootfilelist_file.close()

    #print "procssed files = " + str(processed_files)
    #print "current files = " + str(root_files)

    for filepath in sorted(root_files):
        fname = filepath.split('/')[-1]
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
        # save a mapping of the files to processed with their number within the run
        monitoring_files[filepath] = filenum
        # check to see if we've processed this file already
        if filepath not in processed_files:
            new_monitoring_files[filepath] = filenum
            new_files_exist = True

    ## skip further processing if it's not needed
    if not FORCE_PROCESSING and not new_files_exist:
        continue
    if len(monitoring_files) == 0:
        continue

    files_to_process = {}
    if FORCE_PROCESSING:
        files_to_process = monitoring_files
    else:
        files_to_process = new_monitoring_files

    # loop over the files and do any pre-file processing we need to do
    for (fname,filenum) in monitoring_files.items():
        # we are good!  let's get some work done
        print "processing run " + str(runnum) + " file " + str(filenum) + " ..."

        #  process monitoring data for each file
        if MAKE_DB_SUMMARY:
            cmdargs  = "--file_number " + str(filenum) + " " + str(runnum) + " " + str(VERSION_NUMBER) + " " + fname
            print "  analyzing DB info..."
            print "process_monitoring_data " + cmdargs
            process_monitoring_data.main(cmdargs.split()) 


    # sum all the files and place them in a web-viewable location
    summed_rootfile = join(OUTPUT_DIRECTORY,"rootfiles","hd_root_" + rundir + ".root")
    if isfile(summed_rootfile):
        os.system("rm -f " + summed_rootfile)
    os.system("mkdir -m"+NEWDIR_MODE+" -p " + join(OUTPUT_DIRECTORY,"rootfiles"))
    # note hadd -k skips corrupt or missing files - we want to do our best but not fail here
    ##os.system("hadd -v 0 " +  " ".join([summed_rootfile] + monitoring_files.keys() ))

    # save the current list of files
    monitoring_file_list = open(rootfilelist_fname,"w")
    for fname in sorted(monitoring_files.keys()):
        print>>monitoring_file_list, fname
    monitoring_file_list.close()

    # make plots for the sum of all files in the run
    if MAKE_PLOTS:
        cmdargs  = " --histogram_list histograms_to_monitor" 
        cmdargs += " --macro_list macros_to_monitor "
        monitoring_data_dir = join(OUTPUT_DIRECTORY,("Run%06d" % runnum))
        #mkdir_p(monitoring_data_dir)
        os.system("mkdir -m"+NEWDIR_MODE+" -p " + monitoring_data_dir)  ## need error checks
        cmdargs += " --output_dir " + monitoring_data_dir
        cmdargs += " --file_list " + rootfilelist_fname
        print "  creating plots..."
        make_monitoring_plots.main(cmdargs.split())

    if MAKE_RUN_CONDITIONS:
        # update the run metadata
        cmdargs = str(runnum)
        print "  saving conditions..."
        process_run_conditions.main(cmdargs.split())


# cleanup 
#runlist_file.close()

    
