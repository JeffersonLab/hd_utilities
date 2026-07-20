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
import json
import rcdb
import hddm_s

from ROOT import gROOT,gSystem

# debugging
#from memory_profiler import profile
#from pympler import tracker

# monitoring libraries
from datamon_db import datamon_db
import summarize_monitoring_data
import make_monitoring_plots
import phadd
#import process_run_conditions

from sys import getsizeof, stderr

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
        self.NTHREAD = 2
        self.VERSION_NUMBER  =  -1

        # switches to control what type of processing we do
        self.MAKE_PLOTS = True
        self.MAKE_DB_SUMMARY = True
        self.MAKE_SUMMED_ROOTFILE = True
        #self.MAKE_RUN_CONDITIONS = False
        self.COPY_REST_FILES = False

        # ignore checks if we've previously processed all monitoring ROOT files
        self.FORCE_PROCESSING = False   
        # control level of output
        self.VERBOSE = 2

        self.REVISION = None           # data period revision (see DB)
        self.INPUT_DIRECTORY = None    # base directory where output of monitoring jobs is stored
        self.INPUT_SMALLFILE_DIRECTORY = None    # base directory where small output files (e.g. log files) are stored
        #self.ROOTFILE_DIR = "ROOT" # OLD     # monitoring ROOT files are stored in INPUT_DIRECTORY/REVISION/ROOTFILE_DIR
        self.ROOTFILE_DIR = "hists"     # monitoring ROOT files are stored in INPUT_DIRECTORY/REVISION/ROOTFILE_DIR
        #self.ROOTFILE_DIR = "monitoring_hists"     # for bggen
        self.OUTPUT_DIRECTORY = None   # base directory to store output files
        self.MERGED_ROOTFILE_OUTPUT_DIRECTORY = None  # optional directory to save merged files in

        self.NEWDIR_MODE = "775"       # permissions used for calls to mkdir

        self.MIN_RUN = -1              # range of runs to process
        self.MAX_RUN = 1000000          
        self.RUN_NUMBER = None         # optionally process just one run
        self.MERGE_INCREMENT = False
        self.EOR_PROCESSING = True
        self.BATCH_TEMP_DIR = False

        self.EVIO_SKIMS_TO_MERGE = []
        self.ROOT_TREES_TO_MERGE = []
        self.HDDM_FILES_TO_MERGE = []

    def ProcessCommandline(self,args,options,db):
        """
        Set configuration variables based on command line options
        """ 
        # these three arguments are required to be set for every invocation
        self.REVISION = args[1]
        self.INPUT_DIRECTORY = args[2]
        self.INPUT_SMALLFILE_DIRECTORY = args[3]
        self.OUTPUT_DIRECTORY = args[4]

        # fix directories used in MC production
        if self.REVISION == "mc":
            #self.ROOTFILE_DIR = "hd_root"
            self.ROOTFILE_DIR = "monitoring_hists" # for bggen
            self.COPY_REST_FILES = False    # sanity check

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
        if(options.disable_hadd):
            self.MAKE_SUMMED_ROOTFILE = False
        if(options.force):
            self.FORCE_PROCESSING = True
        if(options.save_rest):
            self.COPY_REST_FILES = True
        if(options.merge_increment):
            self.MERGE_INCREMENT = True
        if(options.batch_tempdir):
            self.BATCH_TEMP_DIR = True

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

        if options.noendofjob_processing:
            self.EOR_PROCESSING = False

        if options.root_output_dir:
            self.MERGED_ROOTFILE_OUTPUT_DIRECTORY = options.root_output_dir

        if options.root_trees_to_merge:
            self.ROOT_TREES_TO_MERGE = options.root_trees_to_merge.split(",")
        if options.evio_skims_to_merge:
            self.EVIO_SKIMS_TO_MERGE = options.evio_skims_to_merge.split(",")
        if options.hddm_files_to_merge:
            self.HDDM_FILES_TO_MERGE = options.hddm_files_to_merge.split(",")


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
        if self.REVISION == "mc":
            # the monitoring ROOT files are all stored in one directory
            monitoring_files = [ f for f in listdir(join(self.INPUT_DIRECTORY,self.ROOTFILE_DIR)) 
                                 if (isfile(join(self.INPUT_DIRECTORY,self.ROOTFILE_DIR,f))and(f[-5:]=='.root')) ]
            for fname in monitoring_files:
                tokens = fname[:-5].split('_')
                if len(tokens) < 3:
                    continue
                try:
                    runnum = int(tokens[3])
                except ValueError:
                    logging.error("skipping file " + fname + " ...")
                if tokens[3] not in rundirs_on_disk:
                    rundirs_on_disk.append(tokens[3])
                
        else:
            # The monitoring ROOT files are stored in one directory per run
            dirs_on_disk = [ d for d in listdir(join(self.INPUT_DIRECTORY,self.REVISION,self.ROOTFILE_DIR)) 
                             if os.path.isdir(join(self.INPUT_DIRECTORY,self.REVISION,self.ROOTFILE_DIR,d)) ]
        
            for dirname in sorted(dirs_on_disk):
                try:
                    runnum = int(dirname)
                except ValueError:
                    logging.error("skipping directory " + dirname + " ...")
                rundirs_on_disk.append(dirname)

        return sorted(rundirs_on_disk)

    def BuildROOTFileList(self,rundir):
        """
        Input: directory name where the monitoring ROOT files for a given run are store
        Ouputs map of the monitoring ROOT files for the run
          key: full path of the ROOT file    value: number of the file in the run it corresponds to
        """
        monitoring_files = {}

        try:
            runnum = int(rundir)
        except ValueError:
            logging.error("invalid run directory = " + fname + ", skipping ...")
            return

        if self.REVISION == "mc":
            rootfilespath = join(self.INPUT_DIRECTORY,self.ROOTFILE_DIR)
        else:
            rootfilespath = join(self.INPUT_DIRECTORY,self.REVISION,self.ROOTFILE_DIR)  # base directory where the ROOT files are stored

        if self.REVISION == "mc":
            #print "rootfilespath = " + rootfilespath
            #print "rundir = " + rundir
            #exit(0)
            
            # for MC monitoring files, since they are all in one directory, only keep files that have the correct run
            #root_files = [ f for f in root_files if rundir==f[8:14] ]
            #print "checking rundir %s"%(rundir)
            root_files = [ join(rootfilespath,f) for f in listdir(rootfilespath) 
                           #if (isfile(join(rootfilespath,f))and(f[-5:]=='.root')and(rundir==f[8:14])) ]
                           if (isfile(join(rootfilespath,f))and(f[-5:]=='.root')and(rundir==f[14:20])) ] 
            #for  f in listdir(rootfilespath):
            #    print "%s  %s  %s"%(join(rootfilespath,f),f[-5:],f[8:14])
        else:
            # get all files ending in ".root" for the given run
            root_files = [ join(rootfilespath,rundir,f) for f in listdir(join(rootfilespath,rundir)) 
                           if (isfile(join(rootfilespath,rundir,f))and(f[-5:]=='.root')) ]

        if self.VERBOSE>2:
            print("Looking for ROOT files in "+join(rootfilespath,rundir))
            print("  found = "+str(root_files))

        for filepath in sorted(root_files):
            fname = filepath.split('/')[-1]   # for a full path separated by '/'s, the file name should be the last element
            filenum = -1
            # extract the file number - ROOT file should be of the form hd_root_RRRRRR_FFF.root
            fname_fields = fname[:-5].split("_")
            # sanity checks
            if( (fname_fields[0]!="hd") or (fname_fields[1]!="root") ):
                logging.error("invalid filename = " + fname + ", skipping ...")
                continue
            # skip over summed monitoring files without printing error messages
            if( (len(fname_fields) == 3) and (fname_fields[2][-5:]=='.root') ):
                continue
            if(len(fname_fields) < 4):
                #logging.error("invalid filename = " + fname + ", skipping ...")  # should be summed files, 
                continue
            try:
                file_runnum = int(fname_fields[2]) #for bggen +1
                filenum = int(fname_fields[3]) # for bggen +1
                # HACK - only save 100 files for now (MC)
                #if filenum > 100:
                #    continue
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
        print("Processing run %d ..."%run)

    # STEP 1
    # Generate the summary information for each file
    # This part is pretty fast, and is the only processing we do for 
    # individual files, so run it for each file in any case
    # The MySQL indexes will prevent us from overwriting data in the DB
    # We might want to remove this?
    # don't do this for MC.  also handle this better
    summarizer = summarize_monitoring_data.summarize_monitoring_data()
    summarizer.RUN_NUMBER = run
    summarizer.VERSION_NUMBER = config.VERSION_NUMBER
    if config.MAKE_DB_SUMMARY and (config.REVISION != "mc"):
        #summarizer = summarize_monitoring_data.summarize_monitoring_data()
        #summarizer.RUN_NUMBER = run
        #summarizer.VERSION_NUMBER = config.VERSION_NUMBER
        for (fname,filenum) in sorted(monitoring_files.items()):
            if config.VERBOSE>1:
                print("  summarizing run %d file %d ..."%(run,filenum))
            summarizer.FILE_NUMBER = filenum
            summarizer.FILE_NAME = fname
            summarizer.ProcessFile()


    # STEP 2
    # sum all the files and place them in a web-viewable location
    rundir = "%06d"%run
    if config.MERGED_ROOTFILE_OUTPUT_DIRECTORY is None:
        summed_root_dir = join(config.OUTPUT_DIRECTORY,"rootfiles")
    else:
        summed_root_dir = config.MERGED_ROOTFILE_OUTPUT_DIRECTORY
        
    summed_rootfile_name = "hd_root_" + rundir + ".root"
    summed_rootfile = join(summed_root_dir,summed_rootfile_name)  # figure out the output filename
    if config.MAKE_SUMMED_ROOTFILE and ( len(monitoring_files.keys()) > 0 ):   # only merge if there are files to process
        if config.VERBOSE>1:
            print("  summing ROOT files...")
        # make sure the output cirectory exists
        if not isdir(summed_root_dir):
            os.system("mkdir -m"+config.NEWDIR_MODE+" -p " + summed_root_dir)
        if isfile(summed_rootfile):
            os.system("rm -f " + summed_rootfile)  # hadd requires us to 
        # note hadd -k skips corrupt or missing files - we want to do our best but not fail here
        #os.system("hadd -k -v 0 " +  " ".join([summed_rootfile] + monitoring_files.keys() ))
        #print "MAKE SUMMED FILE = " + summed_rootfile
        #print "SUM THESE FILES = " + str(monitoring_files.keys())
        #os.system("hadd -v 0 " +  " ".join([summed_rootfile] + monitoring_files.keys() ))
        if config.MERGE_INCREMENT:
            hadder = phadd.phadd(summed_rootfile, sorted(monitoring_files.keys()) + ["hd_root_" + rundir + ".root"], " -v 0 ", 6, 10)
        else:
            hadder = phadd.phadd(summed_rootfile, sorted(monitoring_files.keys()), " -k ", 6, 10)
            #hadder = phadd.phadd(summed_rootfile, sorted(monitoring_files.keys()), " -v 0 ", 6, 10)
        if config.BATCH_TEMP_DIR:
            hadder.tempdir = os.environ['BATCH_TMPDIR']
        hadder.Add()
        del hadder

        # also, save a copy of the summed file on the write-through cache
        if config.REVISION == "mc":
            rootfilespath = join(config.INPUT_DIRECTORY,config.ROOTFILE_DIR)
        else:
            rootfilespath = join(config.INPUT_DIRECTORY,config.REVISION,config.ROOTFILE_DIR)  # base directory where the ROOT files are stored
        cache_summed_root_dir = join(rootfilespath,"hists_merged")
        if not isdir(cache_summed_root_dir):
            os.system("mkdir -m"+config.NEWDIR_MODE+" -p " + cache_summed_root_dir)

        ##
        print("Adding %s to swif2 output: %s/%s"%(summed_rootfile, cache_summed_root_dir, summed_rootfile_name))
        ##
        os.system("swif2 output %s %s/%s"%(summed_rootfile, cache_summed_root_dir, summed_rootfile_name))

        # this option motivated by new plans for file management when working off of the write-through cache
        # instead of keeping individual ROOT files semi-permanently, for some jobs we merge ROOT files as they
        # come in and keep a copy of the final merged file in the original directory
        # This is probably a bad idea for the write-through cache, though... this plan needs more thought
        if config.MERGE_INCREMENT:
            rootfile_dir = "/".join(monitoring_files.keys()[0].split('/')[:-1])
            # delete old files
            #for filename in monitoring_files.keys():
            #    #os.system("rm "+filename)   
            #    #os.system("mv %s %s.old"%(filename,filename))   # THIS IS HERE FOR DEBUGGING PURPOSES
            os.system("cp -v %s %s"%(summed_rootfile,rootfile_dir))

    # sanity check - does the summed file exist?
    if not isfile(summed_rootfile):
        logging.error("Summed ROOT file doesn't exist for run %d , exiting ..."%run)
        return

    # STEP 3
    # summarize the full run
    if config.MAKE_DB_SUMMARY:
        if config.VERBOSE>1:
            print("  summarizing run %d ..."%(run))
        summarizer.FILE_NUMBER = -1
        summarizer.FILE_NAME = summed_rootfile
        summarizer.ProcessRun()

    # STEP 4
    # generate monitoring plots
    if config.MAKE_PLOTS:
        if config.VERBOSE>1:
            print("  creating plots...")
        monitoring_data_dir = join(config.OUTPUT_DIRECTORY,("Run%06d" % run))
        if not isdir(monitoring_data_dir):
            retval = os.system("mkdir -m"+config.NEWDIR_MODE+" -p " + monitoring_data_dir)  ## need error checks
            if retval != 0:
                print("ERROR MAKING DIRECTORY (#"+str(retval)+") = "+"mkdir -m"+config.NEWDIR_MODE+" -p " + monitoring_data_dir)
                sys.exit(0)

        plotter = make_monitoring_plots.make_monitoring_plots()
        plotter.histlist_filename = "histograms_to_monitor"
        if config.REVISION == "mc":
            plotter.macrolist_filename = "mc_macros_to_monitor"
        else:
            plotter.macrolist_filename = "macros_to_monitor"
        plotter.output_directory = monitoring_data_dir
        plotter.rootfile_name = summed_rootfile
            
        plotter.MakePlots()

    #if MAKE_RUN_CONDITIONS:
    #    # update the run metadata
    #    cmdargs = str(runnum)
    ##    print "  saving conditions..."
    #    process_run_conditions.main(cmdargs.split())

    if config.BATCH_TEMP_DIR:
        temp_dir = os.environ['BATCH_TMPDIR']
    else:
        temp_dir = "."

    # STEP 5
    # MERGE OTHER FILES
    # merge trees
    if len(config.ROOT_TREES_TO_MERGE) > 0:
        for tree in config.ROOT_TREES_TO_MERGE:
            if config.VERBOSE>2:
                print("  merging %s ..."%tree)
            tree_dir = join(config.INPUT_DIRECTORY,config.REVISION,tree,"%06d"%run)
            if not isdir(tree_dir):
                continue # folder does not exist
            if not os.listdir(tree_dir):
                continue # or folder is empty
            merged_tree_dir = join(config.INPUT_DIRECTORY,config.REVISION,tree,"merged")
            
            merged_tree_file = "%s_%06d.root"%(tree,run)
            if config.VERBOSE>0:
                print("merging into %s/%s ..."%(temp_dir,merged_tree_file))
            os.system("hadd -k %s/%s %s/*.root"%(temp_dir,merged_tree_file,tree_dir))
            print("Adding %s to swif2 output: %s/%s"%(merged_tree_file, merged_tree_dir, merged_tree_file))
            os.system("swif2 output %s %s/%s"%(merged_tree_file, merged_tree_dir, merged_tree_file))
    # merge evio files
    """
    if len(config.EVIO_SKIMS_TO_MERGE) > 0:
        for skim in config.EVIO_SKIMS_TO_MERGE:
            if config.VERBOSE>2:
                print "  merging %s ..."%skim
            tree_dir = join(config.INPUT_DIRECTORY,config.REVISION,skim,"%06d"%run)
            merged_skim_file = "hd_rawdata_%06d.%s.evio"%(skim,run)
            os.system("rm -f %s/%s"%(tree_dir,merged_skim_file))
            os.system("")  # SOME COMMAND
            os.system("jcache put %s/%s"%(tree_dir,merged_skim_file))
            os.system("jcache unpin %s/*.root"%(tree_dir))
    """
    # merge hddm files
    if len(config.HDDM_FILES_TO_MERGE) > 0:
        for file in config.HDDM_FILES_TO_MERGE:
            #if config.VERBOSE>2:
            print("  merging %s ..."%file)
            hddm_dir = join(config.INPUT_DIRECTORY,config.REVISION,file,"%06d"%run)
            if not isdir(hddm_dir):
                continue # folder does not exist
            if not os.listdir(hddm_dir):
                continue # or folder is empty
            merged_hddm_dir = join(config.INPUT_DIRECTORY,config.REVISION,file,"merged")

            merged_hddm_file = "%s_%06d.hddm"%(file,run)
            if config.VERBOSE>0:
                print("merging into %s/%s ..."%(temp_dir,merged_hddm_file))
            fout=hddm_s.ostream(os.path.join(temp_dir,merged_hddm_file))
            fout.compression = hddm_s.k_z_compression
            for fin in os.listdir(hddm_dir):
                for rec in hddm_s.istream(os.path.join(hddm_dir,fin)):
                    fout.write(rec)
            print("Adding %s to swif2 output: %s/%s"%(merged_hddm_file, merged_hddm_dir, merged_hddm_file))
            os.system("swif2 output %s %s/%s"%(merged_hddm_file, merged_hddm_dir, merged_hddm_file))

    # CLEANUP
    ## save some information about what has been processed so far
    #rootfiles = []
    try:
        json.dump( sorted(monitoring_files.keys()), open(join(monitoring_data_dir,"processed_files.dat"),"w") )
    except Exception as e:
        logging.error("Couldn't save list of processed files: %s"%str(e))

    # cleanup memory
    if config.MAKE_DB_SUMMARY:
        del summarizer
    if config.MAKE_PLOTS:
        del plotter
    del monitoring_files

    if config.VERBOSE>0:
        print("Done with run %d !"%run)



############################################


def main():
    # Define command line options
    parser = OptionParser(usage = "process_new_offline_data.py input_directory output_directory")
    parser.add_option("-p","--disable_plots", dest="disable_plotting", action="store_true",
                      help="Don't make PNG files for web display")
    parser.add_option("-d","--disable_summary", dest="disable_db_summary", action="store_true",
                      help="Don't calculate summary information and store it in the DB")
    parser.add_option("-s","--disable_hadd", dest="disable_hadd", action="store_true",
                      help="Don't sum ouptut histograms into one combined file.")
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
    parser.add_option("-A","--parallel", dest="parallel", action="store_true",
                      help="Enable parallel processing.")
    parser.add_option("-S","--save_rest", dest="save_rest", action="store_true",
                      help="Save REST files to conventional location.")
    parser.add_option("-M","--merge-incrementally", dest="merge_increment", action="store_true",
                      help="Merge ROOT files incrementally and delete old ones.")
    parser.add_option("-E","--no-end-of-job-processing", dest="noendofjob_processing", action="store_true",
                      help="Disable end of run processing.")
    parser.add_option("--merge-trees", dest="root_trees_to_merge",
                      help="Merge these ROOT trees.")
    parser.add_option("--merge-skims", dest="evio_skims_to_merge",
                      help="Merge these EVIO skims.")
    parser.add_option("--merge-hddms", dest="hddm_files_to_merge",
                      help="Merge these HDDM files.")
    parser.add_option("-T","--merged-root-output-dir", dest="root_output_dir",
                      help="Directory to save merged ROOT files")
    parser.add_option("-B","--batchfarm-tempdir", dest="batch_tempdir", action="store_true",
                      help="Merge ROOT files in the local directory, as required by current batch farm configuration")
    
    (options, args) = parser.parse_args(sys.argv)

    if(len(args) < 3):
        parser.print_help()
        sys.exit(0)

    # initialize configuration
    config = ProcessMonDataConfig()
    db = datamon_db()
    config.ProcessCommandline(args,options,db)

    # try to connect to RCDB
    rcdb_conn = None
    try:
        rcdb_conn = rcdb.RCDBProvider(os.environ.get('RCDB_CONNECTION'))
    except:
        e = sys.exc_info()[0]
        print("Could not connect to RCDB: " + str(e))

    # Set up directories and any other prep work that needs to be done
    config.BuildEnvironment()

    # Check which runs have been already processed
    rundirs_on_disk = config.BuildListOfProcessedRunsOnDisk()

    # For each run, check to see if there are any ROOT files we haven't processed yet
    # If that is true, then we need to process the run - N.B. most of our outputs
    # depend on the full results from a run
    runs_to_process = []
    for rundir in rundirs_on_disk:
        try:
            runnum = int(rundir)    
        except:
            continue

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
            # skip run if it's not in RCDB for some weird reason
            if rcdb_conn.get_run(runnum) is None:
                continue
            # add run start time, needed for monitoring web pages
            rcdb_run = rcdb_conn.get_run(runnum)
            run_properties = {}
            run_properties['start_time'] = rcdb_run.start_time
            run_properties['num_events'] = rcdb_run.get_condition_value('event_count')
            db.UpdateRunInfo(runnum, run_properties)

        ## make sure we have a directory to store some meta-information
        rootfiles_already_processed = []  # let's not do this anymore
        monitoring_data_dir = join(config.OUTPUT_DIRECTORY,("Run%06d" % runnum))
        if isfile(join(monitoring_data_dir,"processed_files.dat")):
            rootfiles_already_processed = json.load( open(join(monitoring_data_dir,"processed_files.dat"),"r") )

        #if config.REVISION == "mc":
        #    misc_dir = join(config.INPUT_DIRECTORY,"misc","%06d"%(int(rundir)))
        #else:
        #    misc_dir = join(config.INPUT_DIRECTORY,config.REVISION,"misc",rundir)
        #rootfiles_already_processed = []
        #if not os.path.exists(misc_dir):
        #    os.system("mkdir -p " + misc_dir)
        #if not os.path.isdir(misc_dir):
        #    logging.error("file %s exists and is not a directory, skipping this run ..."%misc_dir)
        #    continue
        #else:
        #    # check to see if we've already processed some of the ROOT files
        #    if isfile(join(misc_dir,"processed_files.dat")):
        #        rootfiles_already_processed = pickle.load( open(join(misc_dir,"processed_files.dat"),"r") )
        #    #except Exception, e:
        #    #    logging.error("Couldn't load list of processed files: %s"%str(e))

        ## figure out which files for this run are currently on disk
        rootfile_map = config.BuildROOTFileList(rundir)
        if not config.FORCE_PROCESSING and len(rootfile_map)==0:
            continue

        ## figure out which files are new from the last run
        rootfiles_to_process = [ f for f in sorted(rootfile_map.keys()) if f not in rootfiles_already_processed ]

        #print "ROOTFILES_ALREADY_PROCESSED = " + str(rootfiles_already_processed)
        #print "ROOTFILE_MAP = " + str(rootfile_map)
        #print "ROOTFILES_TO_PROCESS = " + str(rootfiles_to_process)

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

    if options.parallel is None:
        # process serially
        for run_args in runs_to_process:
            ProcessOfflineData(run_args)
    else:
        # process in parallel
        p = multiprocessing.Pool(config.NTHREAD)
        p.map(ProcessOfflineData, runs_to_process)

    # save tarballs of log files and PNGs
    if config.EOR_PROCESSING and len(runs_to_process)>0:
        # save log files
        logdir = join(config.INPUT_SMALLFILE_DIRECTORY,config.REVISION,"log")
        if isdir(logdir):
            os.system("tar cf %s/%s/log.tar %s"%(config.INPUT_DIRECTORY,config.REVISION,logdir))  # overwrite any existing file
        os.system("jcache put %s/%s/log.tar"%(config.INPUT_DIRECTORY,config.REVISION))  # save to tape
        # save IDXA files (for EventStore)
        idxadir = join(config.INPUT_SMALLFILE_DIRECTORY,config.REVISION,"IDXA")
        if isdir(idxadir):
            os.system("tar cf %s/%s/IDXA.tar %s"%(config.INPUT_DIRECTORY,config.REVISION,idxadir))  # overwrite any existing file
        os.system("jcache put %s/%s/IDXA.tar"%(config.INPUT_DIRECTORY,config.REVISION))  # save to tape
        # save web figures
        os.system("tar cf %s/%s/web_figures.tar %s/Run*"%(config.INPUT_DIRECTORY,config.REVISION,config.OUTPUT_DIRECTORY))  # overwrite any existing file


## main function 
if __name__ == "__main__":
    main()
