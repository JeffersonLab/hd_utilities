#!/usr/bin/env python
#
# Run over ROOT files containing monitoring data
# and make images to show on the web
#
# TODO: Use hnamepaths!!
# TODO: throw exceptions?
#
# Author: Sean Dobbs (s-dobbs@northwestern.edu), 2014

from ROOT import TFile,TIter,TDirectory,TH1,TH2,TH1I,TH2I,TCanvas,gROOT,TMemFile,gStyle
from optparse import OptionParser
import os.path
import sys
from array import array
import re

from datamon_db import datamon_db

##########################################################
## Globals
##########################################################


CANVAS_WIDTH = 800
CANVAS_HEIGHT = 600

BASE_ROOT_DIR = ""
OUTPUT_DIRECTORY = "."
#PDF_FILE_NAME = "output"
WEB_OUTPUT = True
VERBOSE = False

def init():
    CANVAS_WIDTH = 800
    CANVAS_HEIGHT = 600

    OUTPUT_DIRECTORY = "."
    WEB_OUTPUT = True
    

# the canvas
c1  = TCanvas("c1","",CANVAS_WIDTH,CANVAS_HEIGHT)


##########################################################

"""
## low level histograms
HISTS_TO_PLOT = [ "NumSCHits", "NumBCALHits", "NumFCALHits", "NumCDCHits", "NumTOFHits",
                  "NumFDCWireHits", "NumFDCCathodeHits", "NumTAGMHits", "NumTAGHHits" ]
HISTS_TO_PLOT += [ "cdc_o", "cdc_e", "cdc__t",     ## CDC
                   "fdcow", "fdcos",               ## FDC
                   "digHitE", "digT", "digOcc2D",  ## FCAL 
                   'bcal_fadc_occ', 'bcal_fadc_E', 'bcal_fadc_t',  ## BCAL
                   "tofe", "toft", "tofo1", "tofo2",   ## TOF
                   "st_sec_adc_dhit", "st_pi_dhit", "st_pt_dhit" ]      ## START COUNTER

# higher level histograms
HISTS_TO_PLOT += [ "NumReconstructedParticles", "NumGoodReconstructedParticles",
                   "NumPosChargedTracks", "NumNegChargedTracks", "NumFCALShowers",
                   "NumBCALShowers", "NumTOFPoints", "NumTrackBCALMatches",
                   "NumTrackFCALMatches","NumTrackTOFMatches","NumTrackSCMatches" ]
"""

#HISTS_TO_PLOT = []
#HISTS_TO_SUM = []
#MACROS_TO_RUN = []

##########################################################
## Histogram drawing definitions
##########################################################

def plot_hist(h):
    #c1.Clear()
    h.SetStats(0)
    h.Draw()

def plot_2dhist(h):
    LOGZ_HISTS = [ "cdc_raw_int_vs_n", "cdc_raw_t_vs_n", "cdc_ped_vs_n", "cdc_windata_ped_vs_n" ]
    if h.GetName() in LOGZ_HISTS:
        c1.SetLogz(1)

    h.SetStats(0)
    h.Draw("COLZ")

def ClearPad(pad):
    gROOT.SetStyle("Plain")   # reset style
    c1.Clear()
    c1.SetLogx(0)
    c1.SetLogy(0)
    c1.SetLogz(0)


##########################################################
## image creation has been moved to a different set of macros

# function to see if a histogram "in_hnamepath" matches the path definition given by "hnamepath_def"
# we make the following assumptions for the path definiton:
# - if the path does not begin with a '/', then we assume that this specifies a histogram name
# - if the path does begin with a '/', the we assume that this specifies a full path
# - we allow the "*" wildcard in the path definition
# NOTE: apparently this is very slow, need to optimize
def match_hnamepath(hnamepath_def, in_hnamepath):
    if len(hnamepath_def) == 0:
        return False
    
    if hnamepath_def[0] != '/':
        #  match based on histogram name
        return hnamepath_def == in_hnamepath
    else:
        # translate '*' wildcard into regex language
        hnamepath_def.replace('*', '.*')
        result = re.match(hnamepath_def, in_hnamepath)
        if result is None:
            return False
        else:
            return True

def match_hnamepaths(hnamepath_defs, in_hnamepath):
    for hnamepath_def in hnamepath_defs:
        if match_hnamepath(hnamepath_def, in_hnamepath):
            return True
    return False
############################################################


def print_canvas_png(fullpath):
    global OUTPUT_DIRECTORY
    c1.Print( OUTPUT_DIRECTORY + "/" + fullpath.replace("/","_") + ".png" )


def SumHistContents(root_file, hist_path):
    h = root_file.Get(hist_path)
    if(h == None):
        print "Could not load " + hist_path
        return -1

    sum = 0
    for bin in range(h.GetNbinsX()+1):
        sum += (bin-1)*h.GetBinContent(bin)
    return sum
             

def AccessHistogramsRecursively(the_dir, path, sum_hists, sum_dir, hists_to_sum):
    # loop over all keys in the current directory
    key_iter = TIter(the_dir.GetListOfKeys())
    key = key_iter()
    while( key ):
        obj = key.ReadObj()
        obj_pathname = path+"/"+key.GetName()
        #print key.GetName() + " " + str(type(obj))

        if(isinstance(obj,TH1)):
            if VERBOSE:
                print "matching hname = %s, hnamepath = %s" %(obj.GetName(),obj_pathname)
            #print "histogram =  " + str(obj.GetName())
            #if not match_hnamepaths(hists_to_sum, path+"/"+obj.GetName()):
            # match either histogram names or full path names
            if (obj.GetName() not in hists_to_sum) and (obj_pathname not in hists_to_sum):
                key = key_iter()
                continue

            # we want to sum up histograms over multiple ROOT files
            hnamepath = obj_pathname
            if not hnamepath in sum_hists.keys():
                sum_hists[hnamepath] = obj.Clone()
                sum_hists[hnamepath].SetDirectory(sum_dir)
            else:
                sum_hists[hnamepath].Add(obj)
            
        # if the object is a directory, access what's inside
        if(isinstance(obj, TDirectory)):
            new_sum_dir = sum_dir.GetDirectory(key.GetName())
            if(not new_sum_dir):
                new_sum_dir = sum_dir.mkdir(key.GetName())
            AccessHistogramsRecursively(obj, obj_pathname, sum_hists, new_sum_dir, hists_to_sum)
        # move to next item in the directory
        key = key_iter()
    

def SavePlots(sum_hists, sum_dir, hists_to_plot, macros_to_run):
    global WEB_OUTPUT
    # plot individual histograms
    if(len(sum_hists) > 0):
        for (hnamepath,h) in sum_hists.items():
            #if not match_hnamepaths(hists_to_plot, hnamepath):
            if (h.GetName() not in hists_to_plot) and (hnamepath not in hists_to_plot): 
                continue
            if(isinstance(h,TH2)):
                plot_2dhist(h)
            else:
                plot_hist(h)
            
            # Output canvas
            #print_canvas_pdf()
            if(WEB_OUTPUT):
                print_canvas_png("_"+hnamepath)  ## name hack for backward compatability

    # plot RootSpy macros
    if(len(macros_to_run) > 0):
        for macro_file in macros_to_run: 
            if os.path.isfile(macro_file):
                if VERBOSE:
                    print "running macro = " + macro_file
                # run the macro
                ClearPad(c1)
                sum_dir.cd()
                gROOT.ProcessLine(".x " + macro_file)
                # save the canvas - the name depends just on the file name
                img_fname = macro_file.split('/')[-1]
                #print "SPLIT =  " +  img_fname[0:-2] + " / " + img_fname[-2:]
                if img_fname[-2:] == ".C":
                    img_fname = img_fname[0:-2]
                print_canvas_png(img_fname)
            else:
                print "could not find macro = " + macro_file + " !"

def extract_macro_hists(macro):
    if VERBOSE:
        print "Extracting histograms needed for macros..."
    macro_hists = []
    if not os.path.isfile(macro):
        # this file doesn't exist - should we say something here?
        return macro_hists
    # read through the file and extract the histograms tagged for RootSpy summing
    f = open(macro)
    for line in f:
        tokens = line.strip().split()
        #print str(tokens)
        # histograms are tagged as  "// hnamepath: /path/to/a/hist"
        if (len(tokens) < 3):
            continue
        if ( (tokens[0] == "//") and (tokens[1] == "hnamepath:") ):
            hnamepath = tokens[2]
            if VERBOSE:
                print hnamepath
            #if hnamepath not in macro_hists:
            #    macro_hists.append(hnamepath)
            hname = hnamepath.split('/')[-1]
            if hname not in macro_hists:
                macro_hists.append(hname)
    return macro_hists


def main(argv):
    global CANVAS_WIDTH,CANVAS_HEIGHT,OUTPUT_DIRECTORY
    
    init()
    # initialize lists used to store data
    hists_to_plot = []
    hists_to_sum  = []
    macros_to_run = []
    
    # read in command line args
    parser = OptionParser(usage = "make_monitoring_plots.py [options] <list of files to process>")
    parser.add_option("-D","--output_dir", dest="output_dir",
                      help="Directory where output files will be stored")
    parser.add_option("-S","--canvas_size", dest="canvas_size",
                      help="Size of canvas in the format '[width]x[height]', default = 800x600")
    parser.add_option("-f","--file_list", dest="file_list",
                      help="Specify the list of files to process")
    parser.add_option("-H","--histogram_list", dest="hname_list",
                      help="Specify the list of names of histograms to plot")
    parser.add_option("-M","--macro_list", dest="macroname_list",
                      help="Specify the list of macro files to make plots of")
    parser.add_option("-R","--root_dir", dest="root_dir",
                      help="Specify the base directory in the ROOT file")
#    parser.add_option("-i","--input_file", dest="input_file",
#                      help="Specify the file name to process.")

    (options, args) = parser.parse_args(argv)

#    if( options.file_list is None and  options.input_file is None ):
#        print "Either an input file (-i) or file list (-f) must be specified!"
#        parser.print_help()
#        exit(0)

#    if( options.file_list is not None and options.input_file is not None ):
#        print "Please specify either -i or -f options, not both!"
#        parser.print_help()
#        exit(0)


    if(options.output_dir):
        if(os.path.isdir(options.output_dir)):
            OUTPUT_DIRECTORY = options.output_dir
        else:
            print "WARNING: Output directory '"+options.output_dir+"' does not exist, defaulting to current directory..."
    if(options.canvas_size):
        (new_width,new_height) = options.canvas_size.split("x")
        if( not new_width or not new_height ):
            print "Invalid canvas sizes specified, using defaults..."
        else:
            CANVAS_WIDTH  = int(new_width)
            CANVAS_HEIGHT = int(new_height)
    if(options.root_dir):
        BASE_ROOT_DIR = options.root_dir
        
    
    ## build List of files to run over
    file_list = []
    # get files passed in on command line
    for line in args:
        file_name = line.strip()
        if file_name == "":
            continue
        if file_name[0] == '#':
            continue
        if os.path.isfile(file_name):
            file_list.append(file_name)
        else:
            print "input file does not exist: " + file_name
    if(options.file_list):
        f = open(options.file_list)
        for line in f:
            file_name = line.strip()
            if file_name == "":
                continue
            if file_name[0] == '#':
                continue
            if os.path.isfile(file_name):
                file_list.append(file_name)
            else:
                print "input file does not exist: " + file_name
        f.close()

    ## load lists of objects to save
    histf = open(options.hname_list)
    for line in histf:
        hname = line.strip()
        if hname == "":
            continue
        if hname[0] == '#':
            continue        
        hists_to_plot.append(hname)
    histf.close()
    macrof = open(options.macroname_list)
    for line in macrof:
        macroname = line.strip()
        if macroname == "":
            continue
        if macroname[0] == '#':
            continue        
        macros_to_run.append(macroname)
    macrof.close()

    ## sanity checks
    if(len(file_list) == 0):
        print "No input files given!"
        return
        #sys.exit(0)
    if(len(hists_to_plot) == 0):
        print "No histograms to save!"
    if(len(macros_to_run) == 0):
        print "No macros to save!"


    ## we need to sum both the histograms that we plotting by themselves
    ## and the histograms used by the macros we want
    hists_to_sum = hists_to_plot
    if(len(macros_to_run) > 0):
        for macro in macros_to_run:
            new_macros = extract_macro_hists(macro)
            # merge lists without duplicates
            if not new_macros or len(new_macros) == 0:
                continue
            hists_to_sum = list( set(hists_to_sum) | set(new_macros) )

    ## initializing monitoring DB connection
    mondb = datamon_db()

    ## save mapping of  "hname or hnamepath => ROOT histogram object"
    sum_hists = {}
    sum_dir = TMemFile(".monitor_tmp.root","RECREATE")

    ## run over data to make some plots
    for filename in file_list:
        root_file = TFile(filename)
        # should handle bad files better!
        if(root_file is None):
            print "Could not open file: " + filename
            continue
        else:
            print "processing file " + filename + " ..." 
        AccessHistogramsRecursively(root_file, BASE_ROOT_DIR, sum_hists, sum_dir, hists_to_sum)
        root_file.Close()


    ## finally, make the plots and save them as files
    ClearPad(c1)
    c1.SetCanvasSize(CANVAS_WIDTH,CANVAS_HEIGHT)
    SavePlots(sum_hists, sum_dir, hists_to_plot, macros_to_run)

    ## clean up some memory
    sum_dir.Close()
    del sum_hists
    del hists_to_sum

if __name__ == "__main__":
    main(sys.argv[1:])
