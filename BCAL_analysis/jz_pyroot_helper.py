#!/usr/bin/env python

#PyRoot file created from template located at:
#/gpfs/home/j/z/jzarling/Karst/bin/source/PyRoot_template_file.py

from optparse import OptionParser
import os.path
import os
import sys
import subprocess
import glob
from array import array
from math import sqrt, exp

from ROOT import TRandom, TF1, TMath, Math, TCanvas, ROOT
from ROOT import gBenchmark, gDirectory, gROOT, gStyle, gSystem

#See here for recipe on making custom PyRoot modules
#http://wlav.web.cern.ch/wlav/pyroot/tpymyclass.html
# gROOT.LoadMacro( 'RooSymmCB.cxx' ) #Doesn't need to be compiled, needs to be found in $ROOTSYS/macros/
# gROOT.LoadMacro( 'RooSDSCBShape.cxx' ) #Doesn't need to be re-compiled, needs to be found in $ROOTSYS/macros/
# gROOT.LoadMacro( 'RooMyGausExp.cxx' ) #Doesn't need to be re-compiled, needs to be found in $ROOTSYS/macros/
# # from ROOT import RooSymmCB, RooSDSCBShape #For instructions on making custom RooFit classes http://people.na.infn.it/~lista/Statistics/slides/10%20-%20prob-roofit.pdf
# from ROOT import RooSDSCBShape, RooSymmCB, RooMyGausExp #For instructions on making custom RooFit classes http://people.na.infn.it/~lista/Statistics/slides/10%20-%20prob-roofit.pdf

# gROOT.LoadMacro( 'RooMyPdf1.cxx' )
# from ROOT import RooMyPdf1 #For instructions on making custom RooFit classes http://people.na.infn.it/~lista/Statistics/slides/10%20-%20prob-roofit.pdf


#//************************************//
#GLOBAL STUFF
gVERBOSE = True #For debugging mostly
gROOT.SetBatch(True) #Batch mode: don't actually create a canvas (much faster, avoids X11 forwarding headaches)
gPlotRelativeDirectory = "./.plots/" #Name of directory for output plots, relative path
gStyle.SetOptFit(11111) #Show fit results in panel
gStyle.SetOptStat(0) #Don't show mean rms etc by default

#ENUMS: put in by hand to shadow kName in root
#Styles
kDashed = 2

#Colors
kRed = 632
kOrange = 800
kYellow = 400
kGreen = 416
kBlue = 600
kViolet = 880
kMagenta = 616
kBlack = 1
# kWhite  = 0,   kBlack  = 1,   kGray    = 920,  kRed    = 632,  kGreen  = 416,
# kBlue   = 600, kYellow = 400, kMagenta = 616,  kCyan   = 432,  kOrange = 800,
# kSpring = 820, kTeal   = 840, kAzure   =  860, kViolet = 880,  kPink   = 900

#Palettes
kDeepSea = 51

#END GLOBAL STUFF
#//************************************//

def shell_exec(command, stdin_str): #stdin_str for things fed via command line with "<"
	if(gVERBOSE): print("Executing command: "+command)
	status = subprocess.call(command.split(" "))
	return

def Quiet(func,level = 0 + 1):
    def qfunc(*args,**kwargs):
        oldlevel = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = level
        try:
            return func(*args,**kwargs)
        finally:
            ROOT.gErrorIgnoreLevel = oldlevel
    return qfunc
	
#Save .C and .png to gPlotRelativeDirectory
#Warning: pretty sure TCanvases leak
def jz_SaveTObject(my_TObject,str_outfiletag,opt_str=""):
	if not os.path.isdir(gPlotRelativeDirectory): 
		os.makedirs(gPlotRelativeDirectory)
	c1 = TCanvas(str_outfiletag,str_outfiletag,1200,800)
	c1.cd()
	my_TObject.Draw(opt_str)
	out_path = gPlotRelativeDirectory+str_outfiletag
	c1.SaveAs(out_path+".png")
	c1.SaveAs(out_path+".C")
	
#Get histogram bin index
#Histogram with nbins, min value h_minl, max of h_max
#Get bin that my_val should go into
def jz_get_hist_index(my_val,nbins,h_min,h_max):
	return TMath.FloorNint( (my_val-h_min)*nbins/ (h_max-h_min) )
	
#Returns index of quantile that data falls into
#Second argument array of quantile values
def jz_get_quantile_index(my_data,quantiles):
	return_index = -1
	for j in range(0,len(quantiles)):
		if(j==0):
			if(my_data<quantiles[j]): return_index = j
		if(j>0 and j<len(quantiles)):
			if(quantiles[j-1] < my_data and my_data<quantiles[j]): return_index = j
		if(j==len(quantiles)):
			if(my_data<quantiles[j]): return_index = j
	return return_index
	
#Dumb: make sorted list, fill new array 
def jz_sort_array_dumb(my_array):
	my_list = sorted(my_array)
	my_array_out = array('d',[])
	for i in range(0,len(my_array)):
		my_array_out.append(my_list[i])
	return my_array_out
		
		
		
#Accepts an array of floats and array size, number of evently spaced quantiles
#Sorts array kind of wastefully
#Returns a list of quantile value and average within quantile
def jz_get_quantiles(my_array,n_quantiles):

	my_array = jz_sort_array_dumb(my_array)
	n_entries = len(my_array)
	prob      = array("d",[])
	quantiles = array("d",[])
	for i in range(1,n_quantiles+1):
		prob.append(float(i)/float(n_quantiles))
		quantiles.append(-1) #Container needs placeholder
	TMath.Quantiles(n_entries,len(prob),my_array,quantiles,prob,True)

	sum     = array('d',[])
	entries = array('d',[])
	for i in range(0,n_quantiles):
		sum.append(0)
		entries.append(0)

	for i in range(0,len(my_array) ):
		for j in range(0,n_quantiles):
			if(j==0):
				if(my_array[i]<quantiles[j]): #TRUE: j index correct, so fill
					sum[j]+=my_array[i]
					entries[j]+=1
			if(j>0 and j<n_quantiles):
				if(quantiles[j-1] < my_array[i] and my_array[i]<quantiles[j]): #TRUE: j index correct, so fill
					sum[j]+=my_array[i]
					entries[j]+=1
			if(j==n_quantiles):
				if(my_array[i]<quantiles[j]): #TRUE: j index correct, so fill
					sum[j]+=my_array[i]
					entries[j]+=1
	quantile_avg = array('d',[])
	for i in range(0,n_quantiles):
		quantile_avg.append(sum[i]/entries[i])

	output_list = []
	output_list.append(quantiles)
	output_list.append(quantile_avg)
	return output_list
