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

#Root stuff
from ROOT import TFile, TTree, TBranch, TLorentzVector, TLorentzRotation, TVector3
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TGraphErrors, TGraph, TLine, TLegend
from ROOT import gBenchmark, gDirectory, gROOT, gStyle, gPad

#My Stuff
#Required: add to PYTHONPATH environment variable, e.g.
#setenv PYTHONPATH /gpfs/home/j/z/jzarling/Karst/MyAnalyses/Python_stuff/jz_library/:$PYTHONPATH
from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *

#NOTES FOR OTHERS USING THIS SCRIPT!!!!
## You need both PyRoot and the helper script above before this script will work


LAYER = "4"
TEMP = "10"
kSquare = 20     #10 degree data



def main(argv):
	#Usage controls from OptionParser
	parser_usage = ""
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) != 3):
		parser.print_help()
		return

	c1 = TCanvas("c1","c1",1200,800)

	#pick a layer, plot results from 2015,2017, and 2019
	## For upstream, downstream, and difference
	f_2015  = TFile.Open(argv[0])
	f_2017  = TFile.Open(argv[1])
	f_2019  = TFile.Open(argv[2])

	size = 1.0

	gr_US_2015 = f_2015.Get("gr_layer"+LAYER+"_US_RMS")
	gr_US_2017 = f_2017.Get("gr_layer"+LAYER+"_US_RMS")
	gr_US_2019 = f_2019.Get("gr_layer"+LAYER+"_US_RMS")
	
	color = kRed
	gr_US_2015.SetLineColor(color)
	gr_US_2015.SetMarkerColor(color)
	gr_US_2015.SetMarkerSize(size)
	gr_US_2015.SetMarkerStyle(kSquare)
	color = kBlue
	gr_US_2017.SetLineColor(color)
	gr_US_2017.SetMarkerColor(color)
	gr_US_2017.SetMarkerSize(size)
	gr_US_2017.SetMarkerStyle(kSquare)
	color = kMagenta
	gr_US_2019.SetLineColor(color)
	gr_US_2019.SetMarkerColor(color)
	gr_US_2019.SetMarkerSize(size)
	gr_US_2019.SetMarkerStyle(kSquare)
	gr_US_2019.SetTitle("Upstream Pedestal RMS, Layer "+LAYER+" 18C")
	gr_US_2019.GetXaxis().SetTitle("SiPM bias voltage (V)")
	gr_US_2019.GetYaxis().SetTitle("Pedestal RMS (ADC units)")
	gr_US_2019.GetYaxis().SetRangeUser(1,2.5)
	legend = TLegend(0.13, 0.5, 0.6, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_US_2015,"2015 Upstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")
	legend.AddEntry(gr_US_2017,"2017 Upstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")
	legend.AddEntry(gr_US_2019,"2019 Upstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")

	gr_US_2015.Draw("APx")
	gr_US_2017.Draw("Pxsame")
	gr_US_2019.Draw("Pxsame")
	legend.Draw()
	c1.SaveAs("plots/upstreamMultiyearLayer"+LAYER+".png")
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	gr_DS_2015 = f_2015.Get("gr_layer"+LAYER+"_DS_RMS")
	gr_DS_2017 = f_2017.Get("gr_layer"+LAYER+"_DS_RMS")
	gr_DS_2019 = f_2019.Get("gr_layer"+LAYER+"_DS_RMS")
	
	color = kRed
	gr_DS_2015.SetLineColor(color)
	gr_DS_2015.SetMarkerColor(color)
	gr_DS_2015.SetMarkerSize(size)
	gr_DS_2015.SetMarkerStyle(kSquare)
	color = kBlue
	gr_DS_2017.SetLineColor(color)
	gr_DS_2017.SetMarkerColor(color)
	gr_DS_2017.SetMarkerSize(size)
	gr_DS_2017.SetMarkerStyle(kSquare)
	color = kMagenta
	gr_DS_2019.SetLineColor(color)
	gr_DS_2019.SetMarkerColor(color)
	gr_DS_2019.SetMarkerSize(size)
	gr_DS_2019.SetMarkerStyle(kSquare)
	gr_DS_2019.SetTitle("Downstream Pedestal RMS, Layer "+LAYER+" 18C")
	gr_DS_2019.GetXaxis().SetTitle("SiPM bias voltage (V)")
	gr_DS_2019.GetYaxis().SetTitle("Pedestal RMS (ADC units)")
	gr_DS_2019.GetYaxis().SetRangeUser(1,2.5)
	legend = TLegend(0.13, 0.5, 0.6, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_DS_2015,"2015 Downstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")
	legend.AddEntry(gr_DS_2017,"2017 Downstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")
	legend.AddEntry(gr_DS_2019,"2019 Downstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")

	gr_DS_2015.Draw("APx")
	gr_DS_2017.Draw("Pxsame")
	gr_DS_2019.Draw("Pxsame")
	legend.Draw()
	c1.SaveAs("plots/downstreamMultiyearLayer"+LAYER+"_"+TEMP+"C.png")

	print("Done ")


if __name__ == "__main__":
   main(sys.argv[1:])