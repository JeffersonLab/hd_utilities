#--------------------------------------------------------------------#
#                                                                    #
# 2015/11/19 Kei Moriya                                              #
#                                                                    #
# Sort stderr files by size and make symlinks of stderr and stdout   #
# files in directories.                                              #
#                                                                    #
#--------------------------------------------------------------------#

import sys, os
import glob
import shutil
from optparse import OptionParser

# To use PyROOT, we need environment variable PYTHONPAT,
# for that we need ROOTSYS to be defined
ROOTSYS = str(os.environ.get('ROOTSYS','NOTSET'))
if ROOTSYS == 'NOTSET':
    print 'Environment variable ROOTSYS must be set to a version of ROOT built with PyROOT support'
    exit()

PATH = str(os.environ.get('PATH','NOTSET'))
if PATH == 'NOTSET':
    PATH = ROOTSYS + '/bin'
else:
    PATH = ROOTSYS + '/bin:' + PATH

PYTHONPATH = str(os.environ.get('PYTHONPATH','NOTSET'))
if PYTHONPATH == 'NOTSET':
    PYTHONPATH = ROOTSYS + '/lib'
else:
    PYTHONPATH = ROOTSYS + '/lib:' + PYTHONPATH

os.environ["PYTHONPATH"]      = PYTHONPATH
sys.path.append(str(ROOTSYS + '/lib/'))

from ROOT import gROOT, gStyle, TCanvas, TH1F, TGraph, TH2F, TF1, TLine, TLatex
TLine.DrawClone._creates = False # This will allow use of DrawClone
import ROOT
gStyle.SetOptStat(False)
ROOT.gROOT.SetBatch(True)

def main(argv):

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "stderr_by_size.py [run period] [version]\n"
                                      + "example: stderr_by_size.py 2015_03 15 \n"))
                          
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 2:
        print "Usage:"
        print "stderr_by_size.py [run period] [version]\n"
        print "example: stderr_by_size.py 2015_03 15\n"
        exit()

    RUNPERIOD = args[0]
    VERSION   = args[1]
    # replace underscore if necessary
    RUNPERIOD = RUNPERIOD.replace('_', '-')

    BASEDIR    = '/volatile/halld/offline_monitoring/RunPeriod-' + RUNPERIOD + '/ver' + VERSION + '/log/'
    if not os.path.isdir(BASEDIR):
        print 'Directory ' + BASEDIR + ' does not exist...'
        print 'aborting...'
        exit()

    # Copy stderr files to directory based on size
    OUTPUT_BASEDIR = '/volatile/halld/offline_monitoring/RunPeriod-' + RUNPERIOD + '/ver' + VERSION + '/log/bysize'
    if not os.path.exists(OUTPUT_BASEDIR):
        os.makedirs(OUTPUT_BASEDIR)
    else:
        while (1):
            answer = raw_input('Delete and remake ' + OUTPUT_BASEDIR + ' ? (y/n)')
            if answer == 'n':
                print 'Quitting stderr_by_size.py'
                exit()
            elif answer == 'y':
                shutil.rmtree(OUTPUT_BASEDIR)
                os.makedirs(OUTPUT_BASEDIR)


    # Dictionary for size of stderr file and number of files
    dict_stderrsize_numfiles = {}
    list_stderrsize = []

    nfiles = 0

    # Get all stderr files
    for stderrfile in glob.glob(BASEDIR + '*/stderr*.err'):

        if nfiles % 1000 == 0:
            print 'processing ' + str(nfiles) + '...'

        filesize = os.path.getsize(stderrfile)
        # print file + ' : ' + str(filesize)

        OUTPUT_DIR = OUTPUT_BASEDIR + '/' + str(filesize)

        if filesize not in list_stderrsize:
            dict_stderrsize_numfiles[filesize] = 1
            list_stderrsize.append(filesize)

            # create directory
            if not os.path.exists(OUTPUT_DIR):
                os.makedirs(OUTPUT_DIR)

        else:
            dict_stderrsize_numfiles[filesize] += 1

        # Copy stderr file to directory based on size
        os.symlink(stderrfile,OUTPUT_DIR + '/' + os.path.basename(stderrfile))
        stdoutfile = stderrfile.replace('err','out',2)
        os.symlink(stdoutfile,OUTPUT_DIR + '/' + os.path.basename(stdoutfile))

        nfiles += 1

    print 'processed ' + str(nfiles) + ' files'

    outfile = open("stderrsize_numfiles_" + RUNPERIOD + "_" + "ver" + VERSION + ".txt","w+")

    # returns a list of tuples
    sorted_list = sorted(dict_stderrsize_numfiles.items(), key=lambda x : x[1])
    sorted_list.reverse()
    for tup in sorted_list:
        print 'size ' + "{:>8d}".format(tup[0]) + ' : counts :  ' + "{:>4d}".format(tup[1])
        outfile.write('size ' + "{:>8d}".format(tup[0]) + ' : counts :  ' + "{:>4d}".format(tup[1]) + '\n')

#------------------------------         end of main function          ---------------------------------#
    
## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
    
