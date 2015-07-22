#!/usr/bin/env python

###############################################################
#
# 2015/07/22 Kei Moriya
# 
# Control swif workflow and set up all configuration
# parameters.
#
# The functions create, delete, run are just wrappers
# for swif.
#
# The function add will add jobs with configurable
# run parameters.
#
###############################################################

from optparse import OptionParser
import os.path
import sys
import re

#--------------------------------------------------------+
#                        Globals                         |
#--------------------------------------------------------+
global PROJECT, TRACK, NCORES, DISK, RAM, TIMELIMIT, OS, VERBOSE, PLUGINS, SCRIPTFILE, ENVFILE

# Job options that shouldn't change during lifetime of a workflow
PLUGINS       = "TAGH_online,TAGM_online,BCAL_online,CDC_online,CDC_expert,FCAL_online,FDC_online,ST_online_lowlevel,ST_online_tracking,TOF_online,PS_online,PSC_online,PSPair_online,TPOL_online,TOF_TDC_shift,monitoring_hists,danarest,BCAL_Eff,p2pi_hists,p3pi_hists,HLDetectorTiming,BCAL_inv_mass,trackeff_missing,TRIG_online,CDC_drift,RF_online"
SCRIPTFILE    = "/home/gxproj1/halld/hdswif/script.sh"
ENVFILE       = "/home/gxproj1/halld/jproj/projects/template/setup_jlab-2015-03.csh"

# Set job configuration parameters.
# All jobs will be sent in with this configuration:

PROJECT       = "gluex"
TRACK         = "reconstruction"       # Use debug for quick debugging
NCORES        = 6                      # Number of CPU cores
DISK          = 40                     # Disk usage in GB
RAM           = 5                      # Max RAM usage in GB
TIMELIMIT     = 8                      # Max walltime in hours
OS            = "centos65"             # Specify CentOS65 machines

VERBOSE = False

def init():
    PROJECT       = "gluex"
    TRACK         = "reconstruction"       # Use debug for quick debugging
    NCORES        = 6                      # Number of CPU cores
    DISK          = 40                     # Disk usage in GB
    RAM           = 5                      # Max RAM usage in GB
    TIMELIMIT     = 8                      # Max walltime in hours
    OS            = "centos65"             # Specify CentOS65 machines

def create(workflow):
    os.system("swif create " + workflow)

def cancel(workflow):
    os.system("swif cancel " + workflow)

def delete(workflow):
    os.system("swif cancel " + workflow + " -delete")

def run(workflow):
    os.system("swif run " + workflow)

def runnjobs(workflow, n):
    os.system("swif run " + workflow + " -joblimit " + n)

def status(workflow):
    os.system("swif status " + workflow)

def fullstatus(workflow, format):
    os.system("swif status " + workflow + " -runs -display " + format)

def is_number(string):
    try:
        int(string)
        return True
    except ValueError:
        return False

def find_files(RUNPERIOD, FORMATTED_RUN, FORMATTED_FILE):
    # NOTE:
    # Since we search for files with names
    # Run-[run]/hd_rawdata_[run]_[file].evio
    # we do automatically take out any files that
    # were placed in the wrong directory.
    #
    # If option "all" is used for either run or file,
    # find will be run with *.
    topdir = "/mss/halld/RunPeriod-" + RUNPERIOD + "/rawdata/Run" + FORMATTED_RUN
    os.system("find " + topdir + " -name 'hd_rawdata_" + FORMATTED_RUN + "_" + FORMATTED_FILE + ".evio' > ___files.txt")
    file_handler = open("___files.txt",'r')
    count = 0
    _file_list = [] # create empty list

    # Fill list with files found
    for line in file_handler:
        line = line.rstrip() # remove newline
        _file_list.insert(len(_file_list),line)
        if(VERBOSE == True):
            print str(len(_file_list)) + " "  + line
        count += 1
    os.system("rm -f ___files.txt")
    return _file_list

def add_job(WORKFLOW, OUTPUT_TOPDIR, RUNPERIOD, mssfile):
    # These variables are defined at the beginning of the file for
    # clarity, so are in global scope but are used
    # only in this function
    global PLUGINS, SCRIPTFILE, ENVFILE

    # This is so VERBOSE is consistent throughout the program
    global VERBOSE

    # Get back FORMATTED_RUN, FORMATTED_FILE from mssfile (full path to evio file)
    # set name for regexp run_file
    regexp = re.compile(r"(?P<run_file>\d\d\d\d\d\d_\d\d\d)")
    run_file = regexp.search(mssfile).group('run_file')
    # print "run_line = " + str(run_file)

    # Replace _\d\d\d to get run
    pattern = r"_\d\d\d"
    regexp = re.compile(pattern)
    thisrun  = regexp.sub("",str(run_file))
    # print "thisrun = " + thisrun

    # Replace \d\d\d\d\d\d_ to get run
    pattern = r"\d\d\d\d\d\d_"
    regexp = re.compile(pattern)
    thisfile = regexp.sub("",run_file)
    # print "thisfile = " + thisfile

    # Get input file basename
    basename = os.path.basename(mssfile)
    if(VERBOSE == True):
        print "basename = " + basename

    # Create SCRIPT_ARGS
    SCRIPT_ARGS = str(ENVFILE + " " + basename + " " + PLUGINS + " " + thisrun + " " + thisfile + " " + OUTPUT_TOPDIR + " " + str(NCORES))
    if(VERBOSE == True):
        print "SCRIPT_ARGS = " + SCRIPT_ARGS

    if(VERBOSE == True):
        print "SCRIPTFILE = " + SCRIPTFILE

    add_command = str("swif add-job -workflow " + WORKFLOW + " -project " + PROJECT + " -track " + TRACK + " \\\n") \
        + str(" -cores " + str(NCORES) + " -disk " + str(DISK) + "g -ram " + str(RAM) + "g -time " + str(TIMELIMIT) + "h -os " + OS + " \\\n") \
        + str(" -input " + basename + " " + mssfile + " \\\n") \
    + str(" -stdout " + OUTPUT_TOPDIR + "/log/" + thisrun + "/stdout_" + thisrun + "_" + thisfile + ".out \\\n") \
    + str(" -stderr " + OUTPUT_TOPDIR + "/log/" + thisrun + "/stderr_" + thisrun + "_" + thisfile + ".err \\\n") \
    + str(" -name " + WORKFLOW + "_" + thisrun + "_" + thisfile + " \\\n") \
    + str(SCRIPTFILE + " " + SCRIPT_ARGS)

    if(VERBOSE == True):
        print "job add command is \n" + str(add_command)

    # Execute swif add for this job
    os.system(add_command)
    
    
def main(argv):
    global PROJECT, TRACK, NCORES, DISK, RAM, TIMELIMIT, OS, VERBOSE

    # Default to run over all runs, files
    RUN            = "all"
    FILE           = "all"
    FORMATTED_RUN  = ""
    FORMATTED_FILE = ""

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "hdswif.py [option] [workflow]\n"
                                      + "[option] = {create, run (n), status, add, cancel, delete}\n"
                                      + "Options for add:\n"
                                      + "-r (run) -f (file)\n"
                                      + "-p (project) -T (track) -n (cores) -d (disk) -m (RAM) -t (time) -o (OS)\n"
                                      + "options in [ ] are required, options in ( ) are optional for running\n"
                                      + ""
                                      + "(use -V 1 for verbose mode)"))
    parser.add_option("-r","--run    ", dest="run",
                      help="run")
    parser.add_option("-f","--file   ", dest="file",
                      help="file")

    parser.add_option("-p","--project", dest="project",
                      help="Project name")
    parser.add_option("-T","--track  ", dest="track",
                      help="Track name")
    parser.add_option("-n","--ncores  ", dest="ncores",
                      help="Ncores")
    parser.add_option("-d","--disk   ", dest="disk",
                      help="Disk usage")
    parser.add_option("-m","--ram    ", dest="ram",
                      help="RAM usage")
    parser.add_option("-t","--time   ", dest="timelimit",
                      help="time limit")
    parser.add_option("-o","--OS      ", dest="os",
                      help="OS")
    parser.add_option("-V","--verbose    ",dest="verbose",
                      help="verbose")
    
    (options, args) = parser.parse_args(argv)

    if(options.run):
        RUN = options.run
    if(options.file):
        FILE = options.file

    if(options.project):
        PROJECT = options.project
    if(options.track):
        TRACK = options.track
    if(options.ncores):
        NCORES = options.ncores
    if(options.disk):
        DISK = options.disk
    if(options.ram):
        RAM = options.ram
    if(options.timelimit):
        TIMELIMIT = options.timelimit
    if(options.os):
        OS = options.os
    if(options.verbose):
        VERBOSE = True

    # Make sure we have at least two arguments,
    # swif command and workflow
    if(len(args) < 2):
        parser.print_help()
        return

    WORKFLOW = args[1]

    # If we want to create workflow, create it and exit
    if(args[0] == "create"):
        create(WORKFLOW)
        return

    # If we want to cancel workflow, cancel and exit
    elif(args[0] == "cancel"):
        cancel(WORKFLOW)
        return

    # If we want to delete workflow, delete it and exit
    elif(args[0] == "delete"):
        delete(WORKFLOW)
        return

    # If we want to run workflow, run it and exit
    elif(args[0] == "run"):
        run(WORKFLOW)
        if(len(args) == 3):
            runnjobs(WORKFLOW, args[2])
        return

    # If we want to check status of workflow, check it and exit
    elif(args[0] == "status"):
        if(len(args) == 2):
            status(WORKFLOW)
        if(len(args) == 3):
            if(not(args[2] == "xml" or args[2] == "json" or args[2] == "simple")):
                print "hdswif.py status [workflow] [display format]"
                print "display format = {xml, json, simple}"
                return
            fullstatus(WORKFLOW, str(args[2]))
        return
    else:
        if(args[0] != "add"):
            print "hdswif.py options:"
            print "create   delete   run  status   add"
            return

    #------------------------------------------+
    #       We are in add mode now             |
    #------------------------------------------+

    # Initialize all configuration parameters
    init()

    # Make sure we have at least 4 arguments,
    # ./hdswif.py run [workflow] [run period] [version]
    if(len(args) < 4):
        print "To add jobs,"
        print "hdswif.py run [workflow] [run period] [version]"
        return
        create(args[1])
        return
    RUNPERIOD = str(args[2])
    VERSION   = str(args[3])

    if(not(RUNPERIOD == "2015-03" or RUNPERIOD == "2014-10")):
        print "hdswif.py add [workflow] [run period] [version]"
        print "run period must be {2014-10, 2015-03}"
        return

    OUTPUT_TOPDIR = "/volatile/halld/offline_monitoring/RunPeriod-" + RUNPERIOD + "/ver" + VERSION

    print "---   creating workflow   " + WORKFLOW + "   ---"
    print "run period        = " + RUNPERIOD
    print "version           = " + VERSION
    print "output dir        = " + OUTPUT_TOPDIR
    print ""
    print "---   Job configuration parameters:   ---"
    print "PROJECT           = " + PROJECT
    print "TRACK             = " + TRACK
    print "NCORES            = " + str(NCORES)
    print "DISK              = " + str(DISK)
    print "RAM               = " + str(RAM)
    print "TIMELIMIT         = " + str(TIMELIMIT)
    print "OS                = " + OS

    # Format run and file numbers
    if(is_number(RUN) == True):
        FORMATTED_RUN = "{:0>6d}".format(int(RUN))
    elif(RUN == "all"):
        FORMATTED_RUN = "*"
    else:
        print "hdswif.py add [workflow] [run period] [version] -r (run)"
        print "run must be number or \"all\""
        return

    if(is_number(FILE) == True):
        FORMATTED_FILE = "{:0>3d}".format(int(FILE))
    elif(FILE == "all"):
        FORMATTED_FILE = "*"
    else:
        print "hdswif.py add [workflow] [run period] [version] -f (file)"
        print "file must be number or \"all\""
        return

    if(VERBOSE == True):
        print "FORMATTED_RUN = " + FORMATTED_RUN + " FORMATTED_FILE = " + FORMATTED_FILE

    #------------------------------------------+
    #    Find raw evio files to submit         |
    #------------------------------------------+

    file_list = []
    file_list = find_files(RUNPERIOD, FORMATTED_RUN, FORMATTED_FILE)
    if(VERBOSE == True):
        for file in file_list:
            print file
        print "size of file_list is " + str(len(file_list))

    #------------------------------------------+
    #         Add job to workflow              |
    #------------------------------------------+

    # Loop over files found for given run and file
    for mssfile in file_list:
        add_job(WORKFLOW, OUTPUT_TOPDIR, RUNPERIOD, mssfile)

if __name__ == "__main__":
   main(sys.argv[1:])
