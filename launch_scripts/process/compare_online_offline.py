#!/usr/bin/env python
#
# Compares contents of two ROOT files with similar structure
# Intended to compare results of mointoring for offline and online processing
#
# Author: Sean Dobbs (s-dobbs@northwestern.edu), 2014

from ROOT import TFile,TIter,TDirectory,TH1,TH2,TCanvas
from optparse import OptionParser
import os.path

##########################################################
## Globals
##########################################################

NUMBER_OF_HISTOGRAMS = 0
CURRENT_HISTOGRAM_NUM = 0

CANVAS_WIDTH = 800
CANVAS_HEIGHT = 600

OUTPUT_DIRECTORY = "."
PDF_FILE_NAME = "output"
WEB_OUTPUT = False

# the canvas
c1  = TCanvas("c1","",CANVAS_WIDTH,CANVAS_HEIGHT)

##########################################################
## Histogram drawing definitions
##########################################################

def plot_offline_hist(h):
    c1.Clear()
    h.SetStats(0)
    h.Draw()

def plot_online_hist(h):
    h.SetStats(0)
    h.SetLineColor(418)
    h.SetLineWidth(2)
    h.SetLineStyle(2)
    h.Draw("SAME")

def plot_offline_2dhist(h):
    h.SetStats(0)
    h.Draw("COLZ")

def plot_online_2dhist(h):
    h.SetStats(0)
    h.Draw("COLZ")

def plot_2d_hists(h1,h2):
    c1.Clear()
    c1.Divide(2,1)
    c1.cd(1)
    plot_offline_2dhist(h1)
    c1.cd(2)
    plot_online_2dhist(h2)

##########################################################

def print_canvas_pdf():
    pdf_suffix = ""
    if(CURRENT_HISTOGRAM_NUM == 1):
        pdf_suffix = "("
    elif(CURRENT_HISTOGRAM_NUM == NUMBER_OF_HISTOGRAMS):
        pdf_suffix = ")"
    c1.Print( OUTPUT_DIRECTORY + "/" + PDF_FILE_NAME+".pdf"+pdf_suffix )

#def print_canvas_png(fullpath):
#    c1.Print( OUTPUT_DIRECTORY + "/" + fullpath.replace("/","_") + ".png" )             
def print_canvas_svg(fullpath):
    c1.Print( OUTPUT_DIRECTORY + "/" + fullpath.replace("/","_") + ".svg" )
             

def CountNumberOfHistograms(the_dir):
    global NUMBER_OF_HISTOGRAMS

    # loop over all keys in the current directory
    key_iter = TIter(the_dir.GetListOfKeys())
    key = key_iter()
    while( key ):
        obj = key.ReadObj()
        if(isinstance(obj,TH1)):
            NUMBER_OF_HISTOGRAMS += 1
            #print obj.GetName() + " " + str(NUMBER_OF_HISTOGRAMS)
        # if the object is a directory, access what's inside
        if(isinstance(obj, TDirectory)):
            CountNumberOfHistograms(obj)
        # go to next entry
        key = key_iter()

def AccessDataRecursively(the_dir, path, online_file):
    global CURRENT_HISTOGRAM_NUM

    # loop over all keys in the current directory
    key_iter = TIter(the_dir.GetListOfKeys())
    key = key_iter()
    while( key ):
        obj = key.ReadObj()
        obj_pathname = path+"/"+key.GetName()
        #print key.GetName() + " " + str(type(obj))

        if(isinstance(obj,TH1)):
            CURRENT_HISTOGRAM_NUM += 1
            #print "histogram =  " + str(obj.GetName())

            # get corresponding online hist
            # be lazy and don't worry about iterating over the other file in parallel
            h_online = online_file.Get(obj_pathname)
            if not h_online:
                print "Could not find online histogram: " + obj_pathname
                continue
                
            if(isinstance(obj,TH2)):
                #print "Is 2D histo!"
                plot_2d_hists(obj,h_online)
            else:
                # make plots
                plot_offline_hist(obj)
                plot_online_hist(h_online)

            # output canvas
            print_canvas_pdf()
            if(WEB_OUTPUT):
                #print_canvas_png(path+"/"+key.GetName())
                print_canvas_svg(path+"/"+key.GetName())

        # if the object is a directory, access what's inside
        if(isinstance(obj, TDirectory)):
            AccessDataRecursively(obj, obj_pathname, online_file)
        # move to next item in the directory
        key = key_iter()
    

if __name__ == "__main__":
    
    # read in command line args
    parser = OptionParser(usage = "compare_online_offline.py [options] offline_file.root online_file.root")
    parser.add_option("-W","--web_output", action="store_true", dest="web_output",
                      help="Create PNG files of comparisons for web display")
    parser.add_option("-D","--output_dir", dest="output_dir",
                      help="Directory where output files will be stored")
    parser.add_option("-F","--pdf_file_name", dest="file_name",
                      help="Name of PDF output file")
    parser.add_option("-S","--canvas_size", dest="canvas_size",
                      help="Size of canvas in the format '[width]x[height]', default = 800x600")

    (options, args) = parser.parse_args()

    if(len(args) < 2):
        parser.print_help()
        exit(0)

    offline_file_name = str(args[0])
    online_file_name = str(args[1])

    if(options.web_output):
        WEB_OUTPUT = True
    if(options.output_dir):
        if(os.path.isdir(options.output_dir)):
            OUTPUT_DIRECTORY = options.output_dir
        else:
            print "WARNING: Output directory '"+options.output_dir+"' does not exist, defaulting to current directory..."
    if(options.file_name):
        PDF_FILE_NAME = options.file_name
    if(options.canvas_size):
        (new_width,new_height) = options.canvas_size.split("x")
        if( not new_width or not new_height ):
            print "Invalid canvas sizes specified, using defaults..."
        else:
            CANVAS_WIDTH  = int(new_width)
            CANVAS_HEIGHT = int(new_height)
        
        
    # sanity checks
    if(not os.path.isfile(offline_file_name)):
        print " no such file: " + offline_file_name
    if(not os.path.isfile(online_file_name)):
        print " no such file: " + online_file_name
    
    c1.SetCanvasSize(CANVAS_WIDTH,CANVAS_HEIGHT)

    # run over data
    offline_file = TFile(offline_file_name)
    online_file = TFile(online_file_name)

    # we take the offline file as our reference and try to find the corresponding data
    # in the online file
    CountNumberOfHistograms(offline_file)  ## needed for PDF output
    AccessDataRecursively(offline_file, "/", online_file)  ## make the comparison
