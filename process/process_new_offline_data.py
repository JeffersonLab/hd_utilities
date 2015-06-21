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
from os.path import isfile, join, isdir
from optparse import OptionParser
import multiprocessing
import logging
import pickle

from ROOT import gROOT,gSystem

# monitoring libraries
from datamon_db import datamon_db
import summarize_monitoring_data
import make_monitoring_plots
#import process_run_conditions

############################################

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

############################################

class ProcessMonDataConfig:
    def __init__(self):
        self.NTHREAD = 4
        self.VERSION_NUMBER  =  -1

        # switches to control what type of processing we do
        self.MAKE_PLOTS = True
        self.MAKE_DB_SUMMARY = True
        self.MAKE_SUMMED_ROOTFILE = True
        #self.MAKE_RUN_CONDITIONS = False

        # ignore checks if we've previously processed all monitoring ROOT files
        self.FORCE_PROCESSING = False   
        # control level of output
        self.VERBOSE = 2

        self.REVISION = None           # data period revision (see DB)
        self.INPUT_DIRECTORY = None    # base directory where output of monitoring jobs is stored
        self.ROOTFILE_DIR = "ROOT"     # monitoring ROOT files are stored in INPUT_DIRECTORY/REVISION/ROOTFILE_DIR
        self.OUTPUT_DIRECTORY = None   # base directory to store output files

        self.NEWDIR_MODE = "775"       # permissions used for calls to mkdir

        self.MIN_RUN = -1              # range of runs to process
        self.MAX_RUN = 1000000          
        self.RUN_NUMBER = None         # optionally process just one run


    def ProcessCommandline(self,args,options,db):
        """
        Set configuration variables based on command line options
        """ 
        # these three arguments are required to be set for every invocation
        self.REVISION = args[1]
        self.INPUT_DIRECTORY = args[2]
        self.OUTPUT_DIRECTORY = args[3]

        # set up output logging
        if options.logfile:
            logging.basicConfig(filename=options.logfile,level=logging.INFO)
            gSystem.RedirectOutput(options.logfile)
        else:
            logging.basicConfig(level=logging.INFO)
        # limit ROOT output  
        gROOT.ProcessLine("gErrorIgnoreLevel = 2001;")   # only output at least "Error" level messages

        # see if we should disable any processing steps
        if(options.disable_plotting):
            self.MAKE_PLOTS = False
        if(options.disable_db_summary):
            self.MAKE_DB_SUMMARY = False
        #if(options.process_run_conditions):
        #    MAKE_RUN_CONDITIONS = True
        if(options.force):
            self.FORCE_PROCESSING = True

        # should we process only one run?
        if(options.run_number):
            try:
                self.RUN_NUMBER = int(options.run_number)
            except ValueError:
                logging.error("Invalid run number = " + options.run_number)
                sys.exit(0)
            if self.RUN_NUMBER <= 0:
                logging.error("Invalid run number = " + options.run_number)
                sys.exit(0)

        # set which data version in the DB this processing run corresponds to
        if(options.version_string):
            try: 
                revision = -1
                (run_period,revision_str) = options.version_string.split(",")
                try:
                    revision = int(revision_str)
                except ValueError:
                    logging.error("Invalid revision = " + revision)
                    sys.exit(0)
                self.VERSION_NUMBER = db.GetVersionIDRunPeriod(run_period, revision)
                if(self.VERSION_NUMBER<0):
                    logging.error("version not found in DB = " + options.version_string)
                    sys.exit(0)
            except:
                logging.error("Invalid version specification = " + options.version_string)
                raise
                sys.exit(0)
            logging.info("Configured RunPeriod = %s  Revision = %d  ->  VersionID = %d" % (run_period,revision,self.VERSION_NUMBER))

        if(options.version_number):
            try:
                self.VERSION_NUMBER = int(options.version_number) 
            except ValueError:
                logging.error("Invalid version number = " + options.version_number)
                sys.exit(0)
            if self.VERSION_NUMBER <= 0:
                logging.error("Invalid version number = " + options.version_number)
                sys.exit(0)

        # process a range of runs?
        if options.min_run:
            self.MIN_RUN = int(options.min_run)
            logging.info("Minimum run = %d"%self.MIN_RUN)
        if options.max_run:
            self.MAX_RUN = int(options.max_run)
            logging.info("Maximum run = %d"%self.MAX_RUN)

        if options.nthreads:
            self.NTHREADS = options.nthreads

    def BuildEnvironment(self):
        """
        Perform any pre-processing we need for this run
        """
        # check to see if the input directory is real
        if not os.path.isdir(self.INPUT_DIRECTORY):
            logging.error("Invalid input directory specified = " + self.INPUT_DIRECTORY)
            sys.exit(0)

        # make the output directiory if it doesn't already exist
        if os.path.exists(self.OUTPUT_DIRECTORY) and not os.path.isdir(self.OUTPUT_DIRECTORY):
            logging.error("File already exists and is not a directory = " + self.OUTPUT_DIRECTORY)
            sys.exit(0)

        if not os.path.exists(self.OUTPUT_DIRECTORY):
            logging.error("Creating directory " + self.OUTPUT_DIRECTORY + " ... ")
            os.system("mkdir -m"+self.NEWDIR_MODE+" -p " + self.OUTPUT_DIRECTORY)  ## need error checks

        # see if there are libraries to load
        LIBDIR = os.getenv("MONITORING_LIBDIR", join(os.getcwd(),"lib"))    
        if isdir(LIBDIR):
            lib_macros = [ join(LIBDIR,f) for f in listdir(LIBDIR) if f[-2:]=='.C' ]
            for macro in lib_macros:
                try:
                    gROOT.ProcessLine(".L "+macro)
                except: 
                    logging.info("Error processing "+macro)
        
    def BuildListOfProcessedRunsOnDisk(self):
        """
        Outputs list of runs that the batch jobs have finished processing
        """
        rundirs_on_disk = []
        # The monitoring ROOT files are stored in one directory per run
        dirs_on_disk = [ d for d in listdir(join(self.INPUT_DIRECTORY,self.REVISION,self.ROOTFILE_DIR)) 
                         if os.path.isdir(join(self.INPUT_DIRECTORY,self.REVISION,self.ROOTFILE_DIR,d)) ]
        
        for dirname in sorted(dirs_on_disk):
            try:
                runnum = int(dirname)
            except ValueError:
                logging.error("skipping directory " + dirname + " ...")
            rundirs_on_disk.append(dirname)
        return rundirs_on_disk

    def BuildROOTFileList(self,rundir):
        """
        Input: directory name where the monitoring ROOT files for a given run are store
        Ouputs map of the monitoring ROOT files for the run
          key: full path of the ROOT file    value: number of the file in the run it corresponds to
        """
        monitoring_files = {}

        rootfilespath = join(self.INPUT_DIRECTORY,self.REVISION,self.ROOTFILE_DIR)  # base directory where the ROOT files are stored
        # get all files ending in ".root" for the given run
        root_files = [ join(rootfilespath,rundir,f) for f in listdir(join(rootfilespath,rundir)) 
                       if (isfile(join(rootfilespath,rundir,f))and(f[-5:]=='.root')) ]

        if config.VERBOSE>2:
            print "Looking for ROOT files in "+join(rootfilespath,rundir)
            print "  found = "+str(root_files)

        for filepath in sorted(root_files):
            fname = filepath.split('/')[-1]   # for a full path separated by '/'s, the file name should be the last element
            filenum = -1
            # extract the file number - ROOT file should be of the form hd_root_RRRRRR_FFF.root
            fname_fields = fname[:-5].split("_")
            # sanity checks
            if(len(fname_fields) < 4):
                logging.error("invalid filename = " + fname + ", skipping ...")
                continue
            if( (fname_fields[0]!="hd") or (fname_fields[1]!="root") ):
                logging.error("invalid filename = " + fname + ", skipping ...")
                continue
            try:
                file_runnum = int(fname_fields[2])
                filenum = int(fname_fields[3])
            except ValueError:
                logging.error("invalid filename = " + fname + ", skipping ...")
                continue
            if file_runnum != runnum :
                logging.error("invalid filename = " + fname + ", skipping ...")
                continue
           # save a mapping of the file paths with their numbers
            monitoring_files[filepath] = filenum

        return monitoring_files

############################################

def ProcessOfflineData(args):
    """
    Process the monitoring data for a run.  The arguments are:
    - the run number
    - class containing various configuration settings
    - a mapping with the details of the ROOT files for the run
    This is a standalone function so that we can use it in a Python multiprocessing pool
    """
    # unpack arguments
    run = args[0]
    config = args[1]
    monitoring_files = args[2]

    if config.VERBOSE>0:
        print "Processing run %d ..."%run

    # STEP 1
    # Generate the summary information for each file
    # This part is pretty fast, and is the only processing we do for 
    # individual files, so run it for each file in any case
    # The MySQL indexes will prevent us from overwriting data in the DB
    # We might want to remove this?
    summarizer = summarize_monitoring_data.summarize_monitoring_data()
    summarizer.RUN_NUMBER = run
    summarizer.VERSION_NUMBER = config.VERSION_NUMBER
    for (fname,filenum) in monitoring_files.items():
        if config.MAKE_DB_SUMMARY:
            if config.VERBOSE>1:
                print "  summarizing run %d file %d ..."%(run,filenum)
            summarizer.FILE_NUMBER = filenum
            summarizer.FILE_NAME = fname
            summarizer.ProcessFile()


    # STEP 2
    # sum all the files and place them in a web-viewable location
    rundir = "%06d"%run
    summed_rootfile = join(config.OUTPUT_DIRECTORY,"rootfiles","hd_root_" + rundir + ".root")  # figure out the output filename
    if config.MAKE_SUMMED_ROOTFILE:
        # make sure the output cirectory exists
        if not isdir(join(config.OUTPUT_DIRECTORY,"rootfiles")):
            os.system("mkdir -m"+config.NEWDIR_MODE+" -p " + join(config.OUTPUT_DIRECTORY,"rootfiles"))
        if isfile(summed_rootfile):
            os.system("rm -f " + summed_rootfile)  # hadd requires us to 
        # note hadd -k skips corrupt or missing files - we want to do our best but not fail here
        #os.system("hadd -k -v 0 " +  " ".join([summed_rootfile] + monitoring_files.keys() ))
        #print "MAKE SUMMED FILE = " + summed_rootfile
        #print "SUM THESE FILES = " + str(monitoring_files.keys())
        os.system("hadd -v 0 " +  " ".join([summed_rootfile] + monitoring_files.keys() ))

    # sanity check - does the summed file exist?
    if not isfile(summed_rootfile):
        logging.error("Summed ROOT file doesn't exist for run %d , exiting ..."%run)
        return

    # STEP 3
    # summarize the full run
    if config.MAKE_DB_SUMMARY:
        if config.VERBOSE>1:
            print "  summarizing run %d ..."%(run)
        summarizer.FILE_NUMBER = -1
        summarizer.FILE_NAME = summed_rootfile
        summarizer.ProcessRun()

    # STEP 4
    # generate monitoring plots
    if config.MAKE_PLOTS:
        if config.VERBOSE>1:
            print "  creating plots..."
        monitoring_data_dir = join(config.OUTPUT_DIRECTORY,("Run%06d" % run))
        retval = os.system("mkdir -m"+config.NEWDIR_MODE+" -p " + monitoring_data_dir)  ## need error checks
        if retval != 0:
            print "ERROR MAKING DIRECTORY (#"+str(retval)+") = "+"mkdir -m"+config.NEWDIR_MODE+" -p " + monitoring_data_dir
            sys.exit(0)

        plotter = make_monitoring_plots.make_monitoring_plots()
        plotter.histlist_filename = "histograms_to_monitor"
        plotter.macrolist_filename = "macros_to_monitor"
        plotter.output_directory = monitoring_data_dir
        plotter.rootfile_name = summed_rootfile
            
        plotter.MakePlots()

    #if MAKE_RUN_CONDITIONS:
    #    # update the run metadata
    #    cmdargs = str(runnum)
    ##    print "  saving conditions..."
    #    process_run_conditions.main(cmdargs.split())

    # CLEANUP
    ## save some information about what has been processed so far
    rootfiles = []
    with open(join(misc_dir,"rootfiles.txt"),"w") as outf:
        for (fname,filenum) in monitoring_files.items():
            rootfiles.append(fname)
            print>>outf,fname
    try:
        pickle.dump( rootfiles, open(join(misc_dir,"processed_files.dat"),"w") )
    except Exception, e:
        logging.error("Couldn't save list of processed files: %s"%str(e))

    # cleanup memory
    del summarizer
    del plotter
    del monitoring_files

    if config.VERBOSE>0:
        print "Done with run %d !"%run


############################################


## main function 
if __name__ == "__main__":
    # Define command line options
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
    parser.add_option("-L","--logfile", dest="logfile",
                      help="Base file name to save logs to")
    parser.add_option("-t","--nthreads", dest="nthreads",
                      help="Number of threads to use")
    
    (options, args) = parser.parse_args(sys.argv)

    if(len(args) < 3):
        parser.print_help()
        sys.exit(0)

    # initialize configuration
    config = ProcessMonDataConfig()
    db = datamon_db()
    config.ProcessCommandline(args,options,db)

    # Set up directories and any other prep work that needs to be done
    config.BuildEnvironment()

    # Check which runs have been already processed
    rundirs_on_disk = config.BuildListOfProcessedRunsOnDisk()

    print "rundirs_on_disk = " + str(rundirs_on_disk)
    
    # For each run, check to see if there are any ROOT files we haven't processed yet
    # If that is true, then we need to process the run - N.B. most of our outputs
    # depend on the full results from a run
    runs_to_process = []
    for rundir in rundirs_on_disk:
        runnum = int(rundir)    
        # handle any options about which runs to process
        # specifying a particular run to process beats specifying a range
        if config.RUN_NUMBER is not None:
            if runnum != config.RUN_NUMBER:
                continue
        else:
            if runnum < config.MIN_RUN or runnum > config.MAX_RUN:
                continue

        if config.VERBOSE>0:
            logging.info("checking run " + str(runnum))

        ## add blank run to DB if it doesn't exist
        if(db.GetRunID(runnum) < 0):
            db.CreateRun(runnum)

        ## make sure we have a directory to store some meta-information
        misc_dir = join(config.INPUT_DIRECTORY,config.REVISION,"misc",rundir)
        rootfiles_already_processed = []
        if not os.path.exists(misc_dir):
            os.system("mkdir -p " + misc_dir)
        if not os.path.isdir(misc_dir):
            logging.error("file %s exists and is not a directory, skipping this run ..."%misc_dir)
            continue
        else:
            # check to see if we've already processed some of the ROOT files
            if isfile(join(misc_dir,"processed_files.dat")):
                rootfiles_already_processed = pickle.load( open(join(misc_dir,"processed_files.dat"),"r") )
            #except Exception, e:
            #    logging.error("Couldn't load list of processed files: %s"%str(e))

        ## figure out which files for this run are currently on disk
        rootfile_map = config.BuildROOTFileList(rundir)
        if len(rootfile_map)==0:
            continue

        ## figure out which files are new from the last run
        rootfiles_to_process = [ f[0] for f in rootfile_map if f[0] not in rootfiles_already_processed ]

        ## if there's new information, or if the user wants us to anyway, 
        ## add the run to the list of the ones we should process
        if config.FORCE_PROCESSING or len(rootfiles_to_process)>0:
            runs_to_process.append( (runnum, config, rootfile_map) )

    ## loop DONE 

    ## Start processing all the runs!
    if config.VERBOSE>0:
        logging.info("%d runs to process..."%(len(runs_to_process)))
    if len(runs_to_process)==0:
        sys.exit(0)

    # process in parallel
    p = multiprocessing.Pool(config.NTHREAD)
    p.map(ProcessOfflineData, runs_to_process)
    # process serially
    #for run_args in runs_to_process:
    #    ProcessOfflineData(run_args)


    
