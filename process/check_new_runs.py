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
import pickle
import logging

from ROOT import gROOT,gSystem

# monitoring libraries
from datamon_db import datamon_db
import make_monitoring_plots
import summarize_monitoring_data
import process_run_conditions

############################################

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

############################################

class CheckNewRuns:
    def __init__(self, logfile=""):
        ## initialize DB
        self.db = datamon_db()

        self.BASE_ONLINEMON_DIR = "/work/halld/online_monitoring"
        self.PROCESSED_RUN_LIST_FILE = "processedrun.lst.online"
        self.ONLINE_ROOT_DIR = self.BASE_ONLINEMON_DIR + '/root'
        self.ONLINE_CONDITION_DIR = self.BASE_ONLINEMON_DIR + '/conditions'

        self.MIN_RUN_NUMBER = 3386
        #self.MIN_RUN_NUMBER = 3421
        self.MAX_RUN_NUMBER = 100000
        self.VERSION_NUMBER  =  27   ## hardcode default
        self.MONITORING_OUTPUT_DIR = "/work/halld/data_monitoring"
        self.RUN_PERIOD = "RunPeriod-2015-06"

        self.MAKE_PLOTS = True
        self.MAKE_DB_SUMMARY = True
        self.MAKE_RUN_CONDITIONS = True

        self.NEWDIR_MODE = "775"
        self.VERBOSE = 1
        self.FORCE_PROCESSING = False

        # limit ROOT output
        gROOT.ProcessLine("gErrorIgnoreLevel = 2001;")   # only output at least "Error" level messages

        # optionally log the output to a file
        # use separate log files for ROOT and all other output
        # append by default
        self.LOGFILE = logfile
        if self.LOGFILE is not "":
            logging.basicConfig(filename=self.LOGFILE)
            gSystem.RedirectOutput(self.LOGFILE)
            #gSystem.RedirectOutput(self.LOGFILE+"_root")

    def LoadProcessedRunList(self,filename):
        run_list = []
        try:
            with open(filename) as runlist_file:
                for line in runlist_file:
                    try:
                        runnum = int(line.strip())
                    except ValueError:
                        logging.error( "Unexpected value in run file = " + line.strip() + " , skipping..." )
                    else:
                        run_list.append( runnum )
        except IOError as e:
            logging.error( "I/O error({0}): {1}".format(e.errno, e.strerror) )
        except:
            logging.critical( "Unexpected error in CheckNewRuns.LoadProcessedRunList(): " + str(sys.exc_info()[0]) )
            sys.exit(0)
        return run_list

    def Run(self):
        # read in list of runs we've already processed
        #run_list = self.LoadProcessedRunList(self.PROCESSED_RUN_LIST_FILE)
        try:
            oldrun_list = pickle.load( open(self.PROCESSED_RUN_LIST_FILE,"r") )
        except IOError:
            oldrun_list = []

        # we decide if there's a new run by checking if a new online monitoring ROOT file has shown up
        # load the list of ROOT files, making sure that they have the right suffix
        # also, allow for a run range to optionally process only a subset of the runs that exist
        # this is mainly for going back to process files that were created previously
        files_on_disk = [ f for f in listdir(self.ONLINE_ROOT_DIR) if (isfile(join(self.ONLINE_ROOT_DIR,f))and(f[-5:]=='.root')) ]
        run_numbers_on_disk = [ (int(fname[13:18]),fname) for fname in files_on_disk if (int(fname[13:18])>=self.MIN_RUN_NUMBER and int(fname[13:18])<=self.MAX_RUN_NUMBER) ]

        # kludge for now, since the online ROOT files have stopped since run 1764, get a new run if a new condition file shows up
        #condition_files_on_disk = [ f for f in listdir(ONLINE_CONDITION_DIR) if (isfile(join(ONLINE_CONDITION_DIR,f))and(f[-4:]=='.dat')) ]
        #run_numbers_on_disk = [ (int(fname[15:20]),fname) for fname in condition_files_on_disk if (int(fname[15:20])>=MIN_RUN_NUMBER) ]
        run_numbers_on_disk.sort(key=lambda runinfo: int(runinfo[0]))

        # do the heavy work
        for (run,fname) in run_numbers_on_disk:
            ## if we haven't already processed the run, make the plots and add its info to the DB
            if run not in oldrun_list or self.FORCE_PROCESSING:
                if self.VERBOSE>0:
                    print "processing " + str(run) + "..."

                ## add blank run to DB if it doesn't exist
                if(self.db.GetRunID(run) < 0):
                    self.db.CreateRun(run)
                ## TODO: create version info?

                # this is the online monitoring ROOT file
                rootfile_name = join(self.ONLINE_ROOT_DIR,fname)
        
                if self.MAKE_PLOTS:
                    if self.VERBOSE>1:
                        print "  creating plots..."
                    monitoring_data_dir = join(self.MONITORING_OUTPUT_DIR, self.RUN_PERIOD, "ver00", ("Run%06d" % run))
                    #mkdir_p(monitoring_data_dir)
                    os.system("mkdir -m"+self.NEWDIR_MODE+" -p " + monitoring_data_dir)  ## need error checks

                    plotter = make_monitoring_plots.make_monitoring_plots()
                    plotter.histlist_filename = "/home/gluex/halld/monitoring/process/histograms_to_monitor"
                    plotter.macrolist_filename = "/home/gluex/halld/monitoring/process/macros_to_monitor"
                    plotter.base_root_dir = "rootspy/"
                    plotter.output_directory = monitoring_data_dir
                    plotter.rootfile_name = rootfile_name

                    plotter.MakePlots()

                if self.MAKE_DB_SUMMARY:
                    if self.VERBOSE>1:
                        print "  analyzing DB info..."
                    summarizer = summarize_monitoring_data.summarize_monitoring_data()
                    summarizer.RUN_NUMBER = run
                    summarizer.VERSION_NUMBER = self.VERSION_NUMBER
                    summarizer.FILE_NAME = rootfile_name
                    summarizer.ROOTDIR_PREFIX = "rootspy/"

                    summarizer.ProcessRun()
        
                    #if self.MAKE_RUN_CONDITIONS:
                    #    # update the run metadata
                    #    cmdargs = str(run)
                    #    print "  saving conditions..."
                    #    process_run_conditions.main(cmdargs.split())

                ## we successfully processed the run!  make a note of that
                oldrun_list.append(run)
                pickle.dump( oldrun_list, open(self.PROCESSED_RUN_LIST_FILE,"w") )



## main function 
if __name__ == "__main__":
     parser = OptionParser(usage = "process_new_offline_data.py input_directory output_directory")
     parser.add_option("-p","--disable_plots", dest="disable_plotting", action="store_true",
                       help="Don't make PNG files for web display")
     parser.add_option("-d","--disable_summary", dest="disable_db_summary", action="store_true",
                       help="Don't calculate summary information and store it in the DB")
     parser.add_option("-v","--verbose", dest="verbose", action="count",
                       help="Verbose output. Specifying this switch multiple times results in more verbose output.")
     #parser.add_option("-c","--disable_conditions", dest="disable_run_conditions", action="store_true",
     #                  help="Don't process and store run conditions information")
     parser.add_option("-S","--version_number", dest="version_number", 
                       help="Save summary results with this DB version ID")
     parser.add_option("-V","--version", dest="version_string",
                       help="Save summary results with a particular data version, specified using the string \"RunPeriod,Revision\", e.g., \"RunPeriod-2014-10,5\"")
     parser.add_option("-f","--force", dest="force", action="store_true",
                       help="Ignore list of already processed runs")
     parser.add_option("-L","--logfile", dest="logfile", 
                       help="Base file name to save logs to")

     (options, args) = parser.parse_args(sys.argv)
     
     # make driver class
     if options.logfile:
         checker = CheckNewRuns(options.logfile)
     else:
         checker = CheckNewRuns()

     # handle command line options
     if(options.disable_plotting):
         checker.MAKE_PLOTS = False
     if(options.disable_db_summary):
         checker.MAKE_DB_SUMMARY = False
     #if(options.disable_run_conditions):
     #    checker.MAKE_RUN_CONDITIONS = False
     if(options.force):
         checker.FORCE_PROCESSING = True

     # parse data version options
     if(options.version_string):
         try: 
             revision = -1
             (run_period,revision_str) = options.version_string.split(",")
             try:
                 revision = int(revision_str)
             except ValueError:
                 logging.critical("Invalid revision = " + revision)
                 sys.exit(0)
             db = datamon_db()
             checker.VERSION_NUMBER = db.GetVersionIDRunPeriod(run_period, revision)
             if(checker.VERSION_NUMBER<0):
                 logging.critical("version not found in DB = " + options.version_string)
                 sys.exit(0)
         except:
             logging.critical("Invalid version specification = " + options.version_string)
             sys.exit(0)
         print "Configured RunPeriod = %s  Revision = %d  ->  VersionID = %d" % (run_period,revision,checker.VERSION_NUMBER)
     if(options.version_number):
         try:
             checker.VERSION_NUMBER = int(options.version_number) 
         except ValueError:
             logging.critical("Invalid version number = " + options.version_number)
             sys.exit(0)
         if checker.VERSION_NUMBER <= 0:
             logging.critical("Invalid version number = " + options.version_number)
             sys.exit(0)

     # Run the processing
     checker.Run()




    
