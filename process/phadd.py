#!/usr/bin/env python
# phadd.py

import os,sys
import multiprocessing
from multiprocessing.pool import ThreadPool
from optparse import OptionParser
import tempfile
import uuid


def CleanTempFiles(filelist):
    for fname in filelist:
        os.system("rm %s"%fname)

def run_hadd(args):
    haddargs  = args[0]
    targetfile  = args[1]
    sourcefiles = args[2]
    clear_files_when_done = args[3]
    if sourcefiles is None or len(sourcefiles) < 0:
        raise RuntimeError("run_hadd: No files passed to add!")
    os.system("hadd %s %s %s"%(haddargs,targetfile, " ".join(sourcefiles)))
    if clear_files_when_done:
        CleanTempFiles(sourcefiles)
    return targetfile 

class phadd:
    def __init__(self, in_targetfile, in_sourcefiles, in_args="", in_nthreads=6, in_chunksize=20, in_stagefiles=False):
        self.nthreads = 0
        self.p = None
        self.stage_files = in_stagefiles
        #self.tempdir = tempfile.gettempdir()
        self.tempdir = "/scratch/gxproj5"
        self.Init(in_targetfile, in_sourcefiles, in_args, in_nthreads, in_chunksize)

    def __del__(self):
        # stop all threads
        self.p.terminate()
        self.p.join()

    def CopyToTempDir(filelist):
        newfilelist = []
        for fname in filelist:
            newfilename = os.path.join(self.tempdir,"phadd_tmp_%s.root"%str(uuid.uuid4()))
            os.system("cp %s %s"%(fname,newfilename))
            newfilelist += [ newfilename ]
        return newfilelist

    def Init(self, in_targetfile, in_sourcefiles, in_args="", in_nthreads=6, in_chunksize=20):
        self.targetfile = in_targetfile   # final file name to save result in
        if in_sourcefiles is None or len(in_sourcefiles) < 0:
            raise RuntimeError("phadd: No files passed to add!")
        else:
            self.sourcefiles = in_sourcefiles  # ROOT files to add
        if in_chunksize<1:
            raise RuntimeError("phadd: Invalid chunk size = "+str(in_chunksize))
        self.chunksize = in_chunksize  # how many files should each thread add at once?
        self.haddargs = in_args

        if in_nthreads<1:
            raise RuntimeError("phadd: Invalid number of threads = "+str(in_nthreads))
        # only remake the thread pool if we're changing the number of threads we're using
        if self.nthreads != in_nthreads:
            self.nthreads = in_nthreads    # how many threads should be split this up into?
            if self.p is not None:
                # stop all of the threads
                self.p.terminate()
                self.p.join()
                del self.p

            ## Multiprocessing in python generally requires separate processes due to
            ## the global interpreter lock, but since we are just executing shell processes
            ## in parallel, we can use threads to save on memory usage
            ## Each thread will block until the call to os.system() is done, and another
            ## thread can be started while the other is blocking
            self.p = ThreadPool(self.nthreads)

    def Add(self):
        if len(self.sourcefiles) == 1:
            # merging one file is easy!  just copy it
            os.system("cp %s %s"%(self.sourcefiles[0],self.targetfile))
        elif len(self.sourcefiles) <= self.chunksize:
            # if we have fewer files than our chunk size, then we can just do one round of adding files
            if self.stage_files:
                staged_sourcefiles = self.CopyToTempDir(self.sourcefiles)
                os.system("hadd %s %s %s"%(self.haddargs,self.targetfile," ".join(staged_sourcefiles)))
                CleanTempFiles(staged_sourcefiles)
            else:
                os.system("hadd %s %s %s"%(self.haddargs,self.targetfile," ".join(self.sourcefiles)))
        else:
            # do two rounds of merging - first split up the files
            njobs = len(self.sourcefiles)/self.chunksize + 1
            jobs = []
            for x in xrange(njobs):
                #sumfilename = os.path.join(tempfile.gettempdir(),"phadd_tmp_%s.root"%next(tempfile._get_candidate_names()))
                sumfilename = os.path.join(self.tempdir,"phadd_tmp_%s.root"%str(uuid.uuid4()))
                if x == njobs-1: 
                    if self.stage_files:
                        job_sourcefiles = staged_sourcefiles = self.CopyToTempDir(self.sourcefiles[x*self.chunksize:])
                    else:
                        job_sourcefiles = self.sourcefiles[x*self.chunksize:]
                    jobs += [ [self.haddargs, sumfilename, job_sourcefiles, self.stage_files ] ]
                else:
                    if self.stage_files:
                        job_sourcefiles = staged_sourcefiles = self.CopyToTempDir(self.sourcefiles[x*self.chunksize:(x+1)*self.chunksize])
                    else:
                        job_sourcefiles = self.sourcefiles[x*self.chunksize:(x+1)*self.chunksize]
                    jobs += [ [self.haddargs, sumfilename, job_sourcefiles, self.stage_files ] ]
            # run over them
            tempsumfiles = self.p.map(run_hadd,jobs)
            # add the results up
            os.system("hadd %s %s %s"%(self.haddargs,self.targetfile," ".join(tempsumfiles)))
            # delete temp files
            for fname in tempsumfiles:
                os.system("rm "+fname)

## main function 
if __name__ == "__main__":
    # Define command line options
    parser = OptionParser(usage = "phadd [options] targetfile source1 [source2 source3 ...]")
    parser.add_option("-f","--force", dest="force", 
                      help="Force file creation and specify compression level (0-6)")
    parser.add_option("-k","--skip-bad", dest="skip", 
                      help="Don't exit on corrupt or non-existant input files")
    parser.add_option("-T","--no-trees", dest="no_trees", 
                      help="Trees are not merged")
    parser.add_option("-O","--optimize-trees", dest="optimize_trees", 
                      help="When merging TTrees, the basket size is re-optimized")
    parser.add_option("-v","--verbosity", dest="verbosity", 
                      help="Explicitly set the verbosity level; 0 request no output, 99 is the default")
    parser.add_option("-n","--max-open", dest="max_open", 
                      help="Open at most 'maxopenedfiles' at once, use 0 to request to use the system maximum.")
    

    (options, args) = parser.parse_args(sys.argv)

    if(len(args) < 3):
        parser.print_help()
        sys.exit(0)

    haddargs = ""
    # note that there is currently no argument checking
    if options.force:
        haddargs = " -f %s"%(options.force)
    if options.skip:
        haddargs = " -k"
    if options.no_trees:
        haddargs = " -T"
    if options.optimize_trees:
        haddargs = " -O"
    if options.verbosity:
        haddargs = " -v %s"%(options.verbosity)
    if options.max_open:
        haddargs = " -n %s"%(options.max_open)

    hadder = phadd(args[1], args[2:], haddargs)
    hadder.Add()
