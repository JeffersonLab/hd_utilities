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
# Much of this code has been borrowed from Sean's monitoring
# scripts.
#
###############################################################

from optparse import OptionParser
import os.path
import sys
import re
from subprocess import Popen, PIPE
import glob
import time

import parse_swif
import read_config
import createXMLfiles
import output_job_details

VERBOSE = False

def trycommand(command,sleeptime = 10):
# Try an os command and if the exit code is non-zero
# return an error
    rc = -999
    while rc != 0:
        p = Popen(command.split(), stdout=PIPE)
        output = p.communicate()[0]
        print output
        rc = p.returncode
        
        # This condition is necessary since we don't
        # want to sleep if the command succeeded
        if rc != 0:
            # sleep for 10 seconds between
            print 'sleeping for ' + str(sleeptime) + ' sec...'
            time.sleep(sleeptime)

def create(workflow,USERCONFIGFILE):
# If a config file has NOT been specified, just
# create the workflow and exit.
# If a config file IS specified, create xml output
# based on the config file
    trycommand("swif create " + workflow)

    if USERCONFIGFILE == '':
        pass
    else:
        if VERBOSE == True:
            VERBOSE_INT = 1
        else:
            VERBOSE_INT = 0
        createXMLfiles.main([USERCONFIGFILE, str(VERBOSE_INT)])

def cancel(workflow):
    trycommand("swif cancel " + workflow)

def delete(workflow):
    trycommand("swif cancel " + workflow + " -delete")

def list():
    trycommand("swif list")

def run(workflow):
    trycommand("swif run " + workflow + " -errorlimit none")

def runnjobs(workflow, n):
    trycommand("swif run " + workflow + " -joblimit " + n + " -errorlimit none")

def status(workflow):
    trycommand("swif status " + workflow)

def freeze(workflow):
    trycommand("swif freeze " + workflow)

def unfreeze(workflow):
    trycommand("swif unfreeze " + workflow)

def fullstatus(workflow, format):
    trycommand("swif status " + workflow + " -runs -summary -display " + format)

def resubmit(workflow, problem, num):

    if problem == 'RLIMIT':
        print 'RLIMIT:'
        trycommand("swif modify-jobs " + workflow + " -ram add " + str(num) + "gb -problems AUGER-OVER_RLIMIT")
    elif problem == 'TIMEOUT':
        print 'TIMEOUT:'
        trycommand("swif modify-jobs " + workflow + " -time add " + str(num) + "h -problems AUGER-TIMEOUT")
    elif problem == 'SYSTEM':
        # Resubmit all system failure jobs
        for problemtype in ['AUGER-SUBMIT', 'AUGER-FAILED', 'AUGER-INPUT-FAIL', 'AUGER-OUTPUT-FAIL', 'SWIF-SYSTEM-ERROR']:
            print problemtype + ':'
            trycommand("swif retry-jobs " + workflow + " -problems " + problemtype)
    else:
        print 'Unknown problem ', problem, ', cannot resolve.'
        print 'Check SWIF help menu for more options'
        return

    run(workflow)

def is_number(string):
    try:
        float(string)
        return True
    except ValueError:
        return False

def find_files(RUNPERIOD, FORMATTED_RUN, FORMATTED_FILE):
    # NOTE:
    # If the run number in the directory name (Run00XXXX)
    # does not agree with what is in the file name (hd_rawdata_00XXXX_YYY.evio)
    # then this will be taken care of in add_jobs.
    # Here we will just grab all evio files
    #
    # If option "all" is used for either run or file,
    # find will be run with *.
    topdir = "/mss/halld/RunPeriod-" + RUNPERIOD + "/rawdata/Run*" + FORMATTED_RUN + "*"

    pathstring = topdir + '/hd_rawdata_*' + FORMATTED_RUN + '*_*' + FORMATTED_FILE + '*.evio'
    if VERBOSE:
        print 'find pathstring = ' + pathstring
    return glob.glob(pathstring)

def add_job(WORKFLOW, config_dict, mssfile):

    # This is so VERBOSE is consistent throughout the program
    global VERBOSE

    # Get back FORMATTED_RUN, FORMATTED_FILE from mssfile (full path to evio file)
    # set name for regexp run_file
    match = ""
    rundir   = 0
    thisrun  = 0
    thisfile = 0
    try:
        match = re.search(r"Run(\d\d\d\d\d\d)/hd_rawdata_(\d\d\d\d\d\d)_(\d\d\d)",mssfile) # _\d\d\d
        rundir   = match.group(1)
        thisrun  = match.group(2)
        thisfile = match.group(3)
    except AttributeError:
        "could not find regexp for " + mssfile

    # Check that run # in directory and file name agree
    if rundir != thisrun:
        print 'skipping file ' + mssfile + ' (run number in directory and file name do not agree)'
        return

    if(rundir == 0 or thisrun == 0 or thisfile == 0):
        print "couldn't find run and file number in " + mssfile

    else:
        if(VERBOSE == True):
            print "thisrun = " + thisrun + "  thisfile = " + thisfile

        # Get input file basename
        basename = os.path.basename(mssfile)
        if(VERBOSE == True):
            print "basename = " + basename

        # Create SCRIPT_ARGS
        num_threads = str(config_dict['NCORES'])
        if(num_threads == "24"):
            num_threads = "Ncores"
        SCRIPT_ARGS = str(config_dict['ENVFILE'] + " " + basename + " " + thisrun + " " + thisfile + " " +
                          config_dict['OUTPUT_TOPDIR'] + " " + num_threads + " " + config_dict['PLUGINS'])
        if(VERBOSE == True):
            print "SCRIPT_ARGS = " + SCRIPT_ARGS

        add_command = "swif add-job -workflow " + WORKFLOW + " -project " + config_dict['PROJECT'] \
            + " -track " + config_dict['TRACK'] + " -cores " + str(config_dict['NCORES']) + " -disk " + str(config_dict['DISK']) + "g" \
            + " -ram " + str(config_dict['RAM']) + "g -time " + str(config_dict['TIMELIMIT']) + "h -os " + config_dict['OS'] \
            + " -input " + basename + " " + mssfile \
            + " -tag user_run " + thisrun + " -tag user_file " + thisfile \
            + " -name " + str(config_dict['JOBNAMEBASE']) + "_" + thisrun + "_" + thisfile \
            + " -stdout " + config_dict['OUTPUT_TOPDIR'] + "/log/" + thisrun + "/stdout_" + thisrun + "_" + thisfile + ".out" \
            + " -stderr " + config_dict['OUTPUT_TOPDIR'] + "/log/" + thisrun + "/stderr_" + thisrun + "_" + thisfile + ".err" \
            + " " + config_dict['SCRIPTFILE'] + " " + SCRIPT_ARGS
        
        if(VERBOSE == True):
            print "job add command is \n" + str(add_command)
            
        # Execute swif add for this job
        trycommand(add_command)
    
def main(argv):
    global VERBOSE

    # Default to run over all runs, files
    RUN            = "all"
    FILE           = "all"
    FORMATTED_RUN  = ""
    FORMATTED_FILE = ""
    USERCONFIGFILE = ""

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "hdswif.py [option] [workflow]\n"
                                      + "[option] = {create, list, run (n), status, freeze, unfreeze, \n"
                                      + "            add, resubmit, summary, cancel, delete, details}\n"
                                      + "----------------------------------\n"
                                      + "Options for add:\n"
                                      + "-c [config] -r (run) -f (file)\n"
                                      + "options in [ ] are required, options in ( ) are optional for running\n"
                                      + ""
                                      + "----------------------------------\n"
                                      + "Options for resubmit:\n"
                                      + "[problem] (additional resources)\n"
                                      + "[problem] = TIMEOUT, RLIMIT, SYSTEM\n"
                                      + "Additional resources in units of hrs for TIMEOUT, GB for RLIMIT\n"
                                      + "Default is to add 2 hrs for TIMEOUT, 2GB for RLIMIT\n"
                                      + "System error jobs are resubmitted with the same resources\n"
                                      + "----------------------------------\n"
                                      + "Options for details:\n"
                                      + "-r [run] -f [file] \n"
                                      + "----------------------------------\n"
                                      + "options in [ ] are required, options in ( ) are optional for running\n"
                                      + "(use -V 1 for verbose mode)"))
    parser.add_option("-r","--run", dest="run",
                      help="specify run(s) to run over")
    parser.add_option("-f","--file", dest="file",
                      help="specify file(s) to run over")

    parser.add_option("-c","--config", dest="config",
                      help="specify config file")
    parser.add_option("-V","--verbose",dest="verbose",
                      help="verbose output")
    
    (options, args) = parser.parse_args(argv)

    if(options.run):
        RUN = options.run
    if(options.file):
        FILE = options.file

    if(options.config):
        USERCONFIGFILE = options.config
    if(options.verbose):
        VERBOSE = True

    # If we want to list workflows, list and exit
    if(len(args)==1 and args[0] == "list"):
        list()
        return

    # For all other cases, make sure we have at least two arguments,
    # swif command and workflow
    if(len(args) < 2):
        parser.print_help()
        return

    WORKFLOW = args[1]

    # If we want to create workflow, create it and exit
    if(args[0] == "create"):
        create(WORKFLOW,USERCONFIGFILE)
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
        if(len(args) == 2):
            run(WORKFLOW)
        if(len(args) == 3):
            runnjobs(WORKFLOW, args[2])
        return

    # If we want to freeze workflow, freeze it and exit
    elif(args[0] == "freeze"):
        if(len(args) == 2):
            freeze(WORKFLOW)
        return

    # If we want to check status of workflow, check it and exit
    elif(args[0] == "status"):
        if(len(args) == 2):
            status(WORKFLOW)
        elif(len(args) == 3):
            if(not(args[2] == "xml" or args[2] == "json" or args[2] == "simple")):
                print "hdswif.py status [workflow] [display format]"
                print "display format = {xml, json, simple}"
                return
            fullstatus(WORKFLOW, str(args[2]))
        else:
            print "hdswif.py status [workflow] [display format]"
            print "display format = {xml, json, simple}"
            return
        return

    # If we want to create a summary of the workflow, call summary
    elif(args[0] == "summary"):
        # create xml output dir
        if not os.path.exists('./xml'):
            os.makedirs('./xml')
        filename = './xml/swif_output_' + WORKFLOW + '.xml'
        if VERBOSE == True:
            print 'output file name is ', filename

        # Check if xml output file exists
        recreate = True

        if os.path.isfile(filename):
            print 'File ', filename, ' already exists'
            
            while(1):
                answer = raw_input('Overwrite? (y/n)   ')
                if answer == 'n':
                    print 'Not recreating summary file for [', WORKFLOW, ']'
                    recreate = False
                    break
                elif answer == 'y':
                    recreate = True
                    break

        # Create the xml file to parse
        if recreate == True:
            print 'Creating XML output file........'
            os.system("swif status " + WORKFLOW + " -runs -summary -display xml > " + filename)
            print 'Created summary file ', filename, '..............'

        # Call parse_swif
        parse_swif.main([filename])
        return

    # Resubmit jobs by problem
    elif(args[0] == "resubmit"):
        if(len(args) == 3):
            # Assume args[1] is workflow,
            # args[2] is problem
            # Currently supports RLIMIT, TIMEOUT, SYSTEM
            # Default is to add 2GB of RAM for RLIMIT,
            # 2 hrs for TIMEOUT, and nothing for SYSTEM
            resubmit(args[1],args[2],2)
            exit()
        elif(len(args) == 4):
            if(is_number(args[3]) == True):
                # Assume args[1] is problem
                # Currently supports RLIMIT, TIMEOUT
                resubmit(args[1], args[2], int(args[3]))
                exit()
            else:
                print "hdswif.py resubmit [workflow] [problem] [resource to add]"
                print "[problem] = TIMEOUT, RLIMIT, SYSTEM"
                print "[resource to add] is in units of hrs for TIMEOUT, GB for RLIMIT"
                exit()
        else:
            print "hdswif.py resubmit [workflow] [problem] [resource to add]"
            print "[problem] = TIMEOUT, RLIMIT, SYSTEM"
            print "[resource to add] is in units of hrs for TIMEOUT, GB for RLIMIT"
            exit()

    # Check registerd job details using the run and file number
    elif(args[0] == "details"):

        # If run and file have not been set, warn and exit
        if(RUN == 'all' or FILE == 'all'):
            print 'Usage:'
            print 'hdswif.py details [workflow] -r [run] -f [file]'
            print 'Run and file MUST be specified'
            exit()

        output_job_details.main([WORKFLOW,RUN,FILE])
        exit()

    # We should only have add left at this stage
    else:
        if(args[0] != "add"):
            print "hdswif.py options:"
            print "create, list, run (n), status, add, resubmit, summary, cancel, delete"
            exit()

    #------------------------------------------+
    #       We are in add mode now             |
    #------------------------------------------+
    if VERBOSE == True:
        VERBOSE_INT = 1
    else:
        VERBOSE_INT = 0
    config_dict = read_config.main([USERCONFIGFILE, str(VERBOSE_INT)])

    # Format run and file numbers
    if(is_number(RUN) == True):
        FORMATTED_RUN = "{:0>6d}".format(int(RUN))
    elif(RUN == "all"):
        FORMATTED_RUN = "*"
    else:
        FORMATTED_RUN = RUN

    if(is_number(FILE) == True):
        FORMATTED_FILE = "{:0>3d}".format(int(FILE))
    elif(FILE == "all"):
        FORMATTED_FILE = "*"
    else:
        FORMATTED_FILE = FILE

    if(VERBOSE == True):
        print "FORMATTED_RUN = " + FORMATTED_RUN + " FORMATTED_FILE = " + FORMATTED_FILE

    #------------------------------------------+
    #    Find raw evio files to submit         |
    #------------------------------------------+
    file_list = []
    file_list = find_files(config_dict['RUNPERIOD'], FORMATTED_RUN, FORMATTED_FILE)
    if(VERBOSE == True):
        for file in file_list:
            print file
        print "size of file_list is " + str(len(file_list))

    #------------------------------------------+
    #         Add job to workflow              |
    #------------------------------------------+

    # Loop over files found for given run and file
    for mssfile in file_list:
        add_job(WORKFLOW, config_dict, mssfile)

if __name__ == "__main__":
   main(sys.argv[1:])
