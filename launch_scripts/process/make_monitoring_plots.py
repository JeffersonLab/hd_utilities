#!/usr/bin/env python
#
# Run over ROOT files containing monitoring data
# and make images to show on the web
#
# TODO: Use full hnamepaths!!
# TODO: throw exceptions?
#
# Author: Sean Dobbs (s-dobbs@northwestern.edu), 2014

from ROOT import TFile,TIter,TDirectory,TH1,TH2,TH1I,TH2I,TCanvas,gROOT,TMemFile,gStyle,gDirectory
from optparse import OptionParser
import os.path
import sys
from array import array
import re
import logging

from datamon_db import datamon_db

# debugging
#from memory_profiler import profile

##########################################################
class make_monitoring_plots:

    def __init__(self):
        self.CANVAS_WIDTH = 1200
        self.CANVAS_HEIGHT = 800

        self.VERBOSE = 1

        # initialize lists used to store data
        self.histlist_filename = None
        self.macrolist_filename = None
        self.hists_to_plot = []
        self.macros_to_run = []

        # name of file that containes the histogram - should get rid of this?
        self.rootfile_name = None
        self.root_file = None       # the actual ROOT file

        self.base_root_dir = ""
        self.output_directory = "."

        # the canvas we plot on
        self.c1  = TCanvas("c1","",self.CANVAS_WIDTH,self.CANVAS_HEIGHT)

    def __del__(self):
        del self.c1
        del self.hists_to_plot
        del self.macros_to_run
        if self.root_file:
            self.root_file.Close()
            #del self.root_file


    ##########################################################
    ## Helper functions
    ##########################################################

    def plot_hist(self, h):
        #c1.Clear()
        h.SetStats(0)
        h.Draw()

    def plot_2dhist(self, h):
        # by request, plot certain histograms with a logarithmic Z scale
        LOGZ_HISTS = [ "cdc_raw_int_vs_n", "cdc_raw_t_vs_n", "cdc_ped_vs_n", "cdc_windata_ped_vs_n" ]
        if h.GetName() in LOGZ_HISTS:
            self.c1.SetLogz(1)
            
        h.SetStats(0)
        h.Draw("COLZ")

    def ClearPad(self, pad):
        #gROOT.SetStyle("Plain")   # reset style
        self.c1.Clear()
        self.c1.SetLogx(0)
        self.c1.SetLogy(0)
        self.c1.SetLogz(0)

    def print_canvas_png(self, fullpath):
        #print "Making plot = " +  self.output_directory + "/" + fullpath.replace("/","_") + ".png" # tmp
        self.c1.Print( self.output_directory + "/" + fullpath.replace("/","_") + ".png" )

    def SumHistContents(self, the_file, hist_path):
        h = the_file.Get(hist_path)
        if(h == None):
            logging.warn("Could not load " + hist_path)
            return -1

        sum = 0
        for bin in range(h.GetNbinsX()+1):
            sum += (bin-1)*h.GetBinContent(bin)
        return sum
             

    ##########################################################
    # These functions are still under development - they work but are slooooow

    # function to see if a histogram "in_hnamepath" matches the path definition given by "hnamepath_def"
    # we make the following assumptions for the path definiton:
    # - if the path does not begin with a '/', then we assume that this specifies a histogram name
    # - if the path does begin with a '/', the we assume that this specifies a full path
    # - we allow the "*" wildcard in the path definition
    # NOTE: apparently this is very slow, need to optimize
    def match_hnamepath(self, hnamepath_def, in_hnamepath):
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

    def match_hnamepaths(self, hnamepath_defs, in_hnamepath):
        for hnamepath_def in hnamepath_defs:
            if self.match_hnamepath(hnamepath_def, in_hnamepath):
                return True
        return False

    ############################################################

    # parse file containing items, one on each line, and turn it into a list
    def ParseFile(self, filename):
        data = []
        with open(filename) as f:
            for line in f:
                name = line.strip()
                if name == "":
                    continue
                if name[0] == '#':
                    continue        
                data.append(name)
        return data

    def PlotHistsRecursive(self, the_dir, path, hists_to_plot):
        # loop over all keys in the current directory, ROOT-style
        key_iter = TIter(the_dir.GetListOfKeys())
        key = key_iter()
        while( key ):
            obj = key.ReadObj()
            obj_pathname = path+"/"+key.GetName()

            # if the object is a histogram, then see if we should plot it
            if(isinstance(obj,TH1)):
                if self.VERBOSE>1:
                    logging.info("matching hname = %s, hnamepath = %s" %(obj.GetName(),obj_pathname))
                #if not match_hnamepaths(hists_to_sum, path+"/"+obj.GetName()):
                # match either histogram names or full path names
                if (obj.GetName() not in hists_to_plot) and (obj_pathname not in hists_to_plot):
                    key = key_iter()
                    continue

                # plot histogram
                if(isinstance(obj,TH2)):
                    self.plot_2dhist(obj)
                else:
                    self.plot_hist(obj)

                # save image to disk
                self.print_canvas_png("_"+obj_pathname)  ## name hack for backward compatability
                #print_canvas_pdf()
            
            # if the object is a directory, access what's inside
            if(isinstance(obj, TDirectory)):
                self.PlotHistsRecursive(obj, obj_pathname, hists_to_plot)

            # END OF LOOP - move to next item in the directory
            key = key_iter()
        del key_iter

    def SavePlots(self, root_dir, dir_path, hists_to_plot, macros_to_run):
        # plot individual histograms 
        if(len(hists_to_plot) > 0):
            # since we select histograms by their full path or just their name
            # we have to search through the directory tree, so the 
            # plotting logic is inside this function
            self.PlotHistsRecursive(root_dir,dir_path,hists_to_plot)

        # plot RootSpy macros
        # the basic algorithm is for each macro, first build a clean slate, then execute the macro
        if(len(macros_to_run) > 0):
            for macro_file in macros_to_run: 
                if os.path.isfile(macro_file):
                    if self.VERBOSE>0:
                        logging.info("running macro = " + macro_file)
                    # run the macro inside CINT
                    self.ClearPad(self.c1)
                    root_dir.cd()
                    try:
                        # reset interpreter to handle a succession of unnamed scripts
                        #gROOT.Reset()     ## currently dealing with ROOT6 bug - sdobbs, Feb. 6, 2017
                        #gROOT.ProcessLine(".x " + macro_file)
                        img_fname = macro_file.split('/')[-1]
                        if img_fname[-2:] == ".C":
                            img_fname = img_fname[0:-2]
                        img_fname = self.output_directory + "/" + img_fname.replace("/","_")
                        os.system("python run_single_root_command.py -F %s -O %s %s"%(self.rootfile_name,img_fname,macro_file))
                    except:
                        logging.error("Error processing "+macro_file)
                    # save the canvas - the name depends just on the file name
                    img_fname = macro_file.split('/')[-1]
                    #if img_fname[-2:] == ".C":
                    #    img_fname = img_fname[0:-2]
                    #self.print_canvas_png(img_fname)
                else:
                    logging.info("could not find macro = " + macro_file + " !")
                    
    """
    ### this was needed when we were adding together histograms, but I don't think that we need it anymore
    def extract_macro_hists(macro):
        if VERBOSE>1:
            logging.info("Extracting histograms needed for macros...")
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
                hnamepath = " ".join(tokens[2:])
                if VERBOSE>1:
                    print hnamepath
                hname = hnamepath.split('/')[-1]
                if hname not in macro_hists:
                    macro_hists.append(hname)
        return macro_hists
    """


    def MakePlots(self):
        ## some sanity checks
        if self.rootfile_name is None:
            logging.critical("No ROOT file set!")
            return
        if self.histlist_filename is None and self.macrolist_filename is None:
            logging.critical("No files containing the histograms or macros to plot set!")
            return

        # decide which histograms/macros we should make plots of
        if self.histlist_filename:
            self.hists_to_plot = self.ParseFile(self.histlist_filename)
        else:
            self.hists_to_plot = []
        if self.macrolist_filename:
            self.macros_to_run = self.ParseFile(self.macrolist_filename)
        else:
            self.macros_to_run = []

        ## more sanity checks
        if len(self.hists_to_plot) == 0 and len(self.macros_to_run) == 0:
            logging.critical("No histograms or macro to save!")
            return

        # make the plots
        self.root_file = TFile(self.rootfile_name)
        # should handle bad files better?
        if(self.root_file is None):
            logging.critical("Could not open file: " + self.rootfile_name)
            return

        self.ClearPad(self.c1)
        self.c1.SetCanvasSize(self.CANVAS_WIDTH,self.CANVAS_HEIGHT)
        self.SavePlots(self.root_file.GetDirectory(self.base_root_dir), self.base_root_dir, self.hists_to_plot, self.macros_to_run)



if __name__ == "__main__":
    # read in command line args
    parser = OptionParser(usage = "make_monitoring_plots.py [options] ROOT_file")
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

    (options, args) = parser.parse_args(sys.argv)

    # set up the main class
    plotter = make_monitoring_plots()

    if(options.output_dir):
        if(os.path.isdir(options.output_dir)):
            plotter.output_directory = options.output_dir
        else:
            print "WARNING: Output directory '"+options.output_dir+"' does not exist, defaulting to current directory..."
    if(options.canvas_size):
        (new_width,new_height) = options.canvas_size.split("x")
        if( not new_width or not new_height ):
            print "Invalid canvas sizes specified, using defaults..."
        else:
            plotter.CANVAS_WIDTH  = int(new_width)
            plotter.CANVAS_HEIGHT = int(new_height)
    if(options.root_dir):
        plotter.base_root_dir = options.root_dir

    if(options.hname_list):
        plotter.histlist_filename = options.hname_list
    if(options.macroname_list):
        plotter.macrolist_filename = options.macroname_list

    # extract the name of the file to process
    if len(args) < 1:
        parser.print_help()
    else:
        plotter.rootfile_name = args[1]

        # do the heavy work
        plotter.MakePlots()
