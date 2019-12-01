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

# has_5deg = False

kOpenCircle = 24 #18 degree data
kSquare = 21     #10 degree data
kRad = 26        #rad

year = "2019"
# year = "2017"
# year = "2015"

temp_4PlottingDiff = "10"

def main(argv):
	#Usage controls from OptionParser
	parser_usage = ""
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) != 2 and len(args)!=3):
		parser.print_help()
		return

	c1 = TCanvas("c1","c1",1200,800)

	f_18deg = TFile.Open(argv[0])
	f_10deg = TFile.Open(argv[1])
	if(len(args)==3): f_5deg  = TFile.Open(argv[2])
	
	size = 1.3
	
	
	gr_L1_US_18deg = f_18deg.Get("gr_layer1_US_RMS")
	gr_L2_US_18deg = f_18deg.Get("gr_layer2_US_RMS")
	gr_L3_US_18deg = f_18deg.Get("gr_layer3_US_RMS")
	gr_L4_US_18deg = f_18deg.Get("gr_layer4_US_RMS")
	gr_global_US_18deg = f_18deg.Get("gr_global_US_RMS")
	gr_L1_US_10deg = f_10deg.Get("gr_layer1_US_RMS")
	gr_L2_US_10deg = f_10deg.Get("gr_layer2_US_RMS")
	gr_L3_US_10deg = f_10deg.Get("gr_layer3_US_RMS")
	gr_L4_US_10deg = f_10deg.Get("gr_layer4_US_RMS")
	gr_global_US_10deg = f_10deg.Get("gr_global_US_RMS")
	if(len(args)==3): gr_L1_US_5deg = f_5deg.Get("gr_layer1_US_RMS")
	if(len(args)==3): gr_L2_US_5deg = f_5deg.Get("gr_layer2_US_RMS")
	if(len(args)==3): gr_L3_US_5deg = f_5deg.Get("gr_layer3_US_RMS")
	if(len(args)==3): gr_L4_US_5deg = f_5deg.Get("gr_layer4_US_RMS")
	if(len(args)==3): gr_global_US_5deg = f_5deg.Get("gr_global_US_RMS")
	
	color = kRed
	gr_L1_US_18deg.SetLineColor(color)
	gr_L1_US_18deg.SetMarkerColor(color)
	gr_L1_US_18deg.SetMarkerSize(size)
	gr_L1_US_18deg.SetMarkerStyle(kOpenCircle)
	gr_L1_US_10deg.SetLineColor(color)
	gr_L1_US_10deg.SetMarkerColor(color)
	gr_L1_US_10deg.SetMarkerSize(size)
	gr_L1_US_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L1_US_5deg.SetLineColor(color)
	if(len(args)==3): gr_L1_US_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L1_US_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L1_US_5deg.SetMarkerStyle(kRad)
	color = kGreen+3
	gr_L2_US_18deg.SetLineColor(color)
	gr_L2_US_18deg.SetMarkerColor(color)
	gr_L2_US_18deg.SetMarkerSize(size)
	gr_L2_US_18deg.SetMarkerStyle(kOpenCircle)
	gr_L2_US_10deg.SetLineColor(color)
	gr_L2_US_10deg.SetMarkerColor(color)
	gr_L2_US_10deg.SetMarkerSize(size)
	gr_L2_US_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L2_US_5deg.SetLineColor(color)
	if(len(args)==3): gr_L2_US_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L2_US_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L2_US_5deg.SetMarkerStyle(kRad)
	color = kBlue
	gr_L3_US_18deg.SetLineColor(color)
	gr_L3_US_18deg.SetMarkerColor(color)
	gr_L3_US_18deg.SetMarkerSize(size)
	gr_L3_US_18deg.SetMarkerStyle(kOpenCircle)
	gr_L3_US_10deg.SetLineColor(color)
	gr_L3_US_10deg.SetMarkerColor(color)
	gr_L3_US_10deg.SetMarkerSize(size)
	gr_L3_US_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L3_US_5deg.SetLineColor(color)
	if(len(args)==3): gr_L3_US_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L3_US_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L3_US_5deg.SetMarkerStyle(kRad)
	color = kMagenta
	gr_L4_US_18deg.SetLineColor(color)
	gr_L4_US_18deg.SetMarkerColor(color)
	gr_L4_US_18deg.SetMarkerSize(size)
	gr_L4_US_18deg.SetMarkerStyle(kOpenCircle)
	gr_L4_US_10deg.SetLineColor(color)
	gr_L4_US_10deg.SetMarkerColor(color)
	gr_L4_US_10deg.SetMarkerSize(size)
	gr_L4_US_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L4_US_5deg.SetLineColor(color)
	if(len(args)==3): gr_L4_US_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L4_US_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L4_US_5deg.SetMarkerStyle(kRad)
	color = kBlack
	gr_global_US_18deg.SetLineColor(color)
	gr_global_US_18deg.SetMarkerColor(color)
	gr_global_US_18deg.SetMarkerSize(size)
	gr_global_US_18deg.SetMarkerStyle(kOpenCircle)
	gr_global_US_10deg.SetLineColor(color)
	gr_global_US_10deg.SetMarkerColor(color)
	gr_global_US_10deg.SetMarkerSize(size)
	gr_global_US_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_global_US_5deg.SetLineColor(color)
	if(len(args)==3): gr_global_US_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_global_US_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_global_US_5deg.SetMarkerStyle(kRad)
	
	
	gr_L1_US_18deg.SetTitle("Upstream Pedestal Width "+year)
	gr_L1_US_18deg.GetXaxis().SetTitle("SiPM bias voltage (V)")
	gr_L1_US_18deg.GetYaxis().SetTitle("Pedestal RMS (ADC units)")
	gr_L1_US_18deg.GetYaxis().SetRangeUser(1,2.5)
	gr_L1_US_18deg.Draw("APx")
	gr_L2_US_18deg.Draw("Pxsame")
	gr_L3_US_18deg.Draw("Pxsame")
	gr_L4_US_18deg.Draw("Pxsame")
	gr_global_US_18deg.Draw("Pxsame")
	gr_L1_US_10deg.Draw("Pxsame")
	gr_L2_US_10deg.Draw("Pxsame")
	gr_L3_US_10deg.Draw("Pxsame")
	gr_L4_US_10deg.Draw("Pxsame")
	gr_global_US_10deg.Draw("Pxsame")
	if(len(args)==3): gr_L1_US_5deg.Draw("Pxsame")
	if(len(args)==3): gr_L2_US_5deg.Draw("Pxsame")
	if(len(args)==3): gr_L3_US_5deg.Draw("Pxsame")
	if(len(args)==3): gr_L4_US_5deg.Draw("Pxsame")
	if(len(args)==3): gr_global_US_5deg.Draw("Pxsame")
	
	legend = TLegend(0.13, 0.45, 0.7, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_L1_US_18deg,"Layer 1 (18 Celcius)","pl")
	legend.AddEntry(gr_L1_US_10deg,"Layer 1 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L1_US_5deg,"Layer 1 (5 Celcius)","pl")
	legend.AddEntry(gr_L2_US_18deg,"Layer 2 (18 Celcius)","pl")
	legend.AddEntry(gr_L2_US_10deg,"Layer 2 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L2_US_5deg,"Layer 2 (5 Celcius)","pl")
	legend.AddEntry(gr_L3_US_18deg,"Layer 3 (18 Celcius)","pl")
	legend.AddEntry(gr_L3_US_10deg,"Layer 3 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L3_US_5deg,"Layer 3 (5 Celcius)","pl")
	legend.AddEntry(gr_L4_US_18deg,"Layer 4 (18 Celcius)","pl")
	legend.AddEntry(gr_L4_US_10deg,"Layer 4 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L4_US_5deg,"Layer 4 (5 Celcius)","pl")
	legend.AddEntry(gr_global_US_18deg,"ALL upstream (18 Celcius)","pl")
	legend.AddEntry(gr_global_US_10deg,"ALL upstream (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_global_US_5deg,"ALL upstream (5 Celcius)","pl")
	legend.Draw()
	c1.SaveAs("plots/upstream"+year+".png")
	
	
	
	
	gr_L1_DS_18deg = f_18deg.Get("gr_layer1_DS_RMS")
	gr_L2_DS_18deg = f_18deg.Get("gr_layer2_DS_RMS")
	gr_L3_DS_18deg = f_18deg.Get("gr_layer3_DS_RMS")
	gr_L4_DS_18deg = f_18deg.Get("gr_layer4_DS_RMS")
	gr_global_DS_18deg = f_18deg.Get("gr_global_DS_RMS")
	gr_L1_DS_10deg = f_10deg.Get("gr_layer1_DS_RMS")
	gr_L2_DS_10deg = f_10deg.Get("gr_layer2_DS_RMS")
	gr_L3_DS_10deg = f_10deg.Get("gr_layer3_DS_RMS")
	gr_L4_DS_10deg = f_10deg.Get("gr_layer4_DS_RMS")
	gr_global_DS_10deg = f_10deg.Get("gr_global_DS_RMS")
	if(len(args)==3): gr_L1_DS_5deg = f_5deg.Get("gr_layer1_DS_RMS")
	if(len(args)==3): gr_L2_DS_5deg = f_5deg.Get("gr_layer2_DS_RMS")
	if(len(args)==3): gr_L3_DS_5deg = f_5deg.Get("gr_layer3_DS_RMS")
	if(len(args)==3): gr_L4_DS_5deg = f_5deg.Get("gr_layer4_DS_RMS")
	if(len(args)==3): gr_global_DS_5deg = f_5deg.Get("gr_global_DS_RMS")
	
	color = kRed
	gr_L1_DS_18deg.SetLineColor(color)
	gr_L1_DS_18deg.SetMarkerColor(color)
	gr_L1_DS_18deg.SetMarkerSize(size)
	gr_L1_DS_18deg.SetMarkerStyle(kOpenCircle)
	gr_L1_DS_10deg.SetLineColor(color)
	gr_L1_DS_10deg.SetMarkerColor(color)
	gr_L1_DS_10deg.SetMarkerSize(size)
	gr_L1_DS_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L1_DS_5deg.SetLineColor(color)
	if(len(args)==3): gr_L1_DS_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L1_DS_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L1_DS_5deg.SetMarkerStyle(kRad)
	color = kGreen+3
	gr_L2_DS_18deg.SetLineColor(color)
	gr_L2_DS_18deg.SetMarkerColor(color)
	gr_L2_DS_18deg.SetMarkerSize(size)
	gr_L2_DS_18deg.SetMarkerStyle(kOpenCircle)
	gr_L2_DS_10deg.SetLineColor(color)
	gr_L2_DS_10deg.SetMarkerColor(color)
	gr_L2_DS_10deg.SetMarkerSize(size)
	gr_L2_DS_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L2_DS_5deg.SetLineColor(color)
	if(len(args)==3): gr_L2_DS_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L2_DS_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L2_DS_5deg.SetMarkerStyle(kRad)
	color = kBlue
	gr_L3_DS_18deg.SetLineColor(color)
	gr_L3_DS_18deg.SetMarkerColor(color)
	gr_L3_DS_18deg.SetMarkerSize(size)
	gr_L3_DS_18deg.SetMarkerStyle(kOpenCircle)
	gr_L3_DS_10deg.SetLineColor(color)
	gr_L3_DS_10deg.SetMarkerColor(color)
	gr_L3_DS_10deg.SetMarkerSize(size)
	gr_L3_DS_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L3_DS_5deg.SetLineColor(color)
	if(len(args)==3): gr_L3_DS_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L3_DS_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L3_DS_5deg.SetMarkerStyle(kRad)
	color = kMagenta
	gr_L4_DS_18deg.SetLineColor(color)
	gr_L4_DS_18deg.SetMarkerColor(color)
	gr_L4_DS_18deg.SetMarkerSize(size)
	gr_L4_DS_18deg.SetMarkerStyle(kOpenCircle)
	gr_L4_DS_10deg.SetLineColor(color)
	gr_L4_DS_10deg.SetMarkerColor(color)
	gr_L4_DS_10deg.SetMarkerSize(size)
	gr_L4_DS_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_L4_DS_5deg.SetLineColor(color)
	if(len(args)==3): gr_L4_DS_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_L4_DS_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_L4_DS_5deg.SetMarkerStyle(kRad)
	color = kBlack
	gr_global_DS_18deg.SetLineColor(color)
	gr_global_DS_18deg.SetMarkerColor(color)
	gr_global_DS_18deg.SetMarkerSize(size)
	gr_global_DS_18deg.SetMarkerStyle(kOpenCircle)
	gr_global_DS_10deg.SetLineColor(color)
	gr_global_DS_10deg.SetMarkerColor(color)
	gr_global_DS_10deg.SetMarkerSize(size)
	gr_global_DS_10deg.SetMarkerStyle(kSquare)
	if(len(args)==3): gr_global_DS_5deg.SetLineColor(color)
	if(len(args)==3): gr_global_DS_5deg.SetMarkerColor(color)
	if(len(args)==3): gr_global_DS_5deg.SetMarkerSize(size)
	if(len(args)==3): gr_global_DS_5deg.SetMarkerStyle(kRad)
	
	
	gr_L1_DS_18deg.SetTitle("Downstream Pedestal Width "+year)
	gr_L1_DS_18deg.GetXaxis().SetTitle("SiPM bias voltage (V)")
	gr_L1_DS_18deg.GetYaxis().SetTitle("Pedestal RMS (ADC units)")
	gr_L1_DS_18deg.GetYaxis().SetRangeUser(1,2.5)
	gr_L1_DS_18deg.Draw("APx")
	gr_L2_DS_18deg.Draw("Pxsame")
	gr_L3_DS_18deg.Draw("Pxsame")
	gr_L4_DS_18deg.Draw("Pxsame")
	gr_global_DS_18deg.Draw("Pxsame")
	gr_L1_DS_10deg.Draw("Pxsame")
	gr_L2_DS_10deg.Draw("Pxsame")
	gr_L3_DS_10deg.Draw("Pxsame")
	gr_L4_DS_10deg.Draw("Pxsame")
	gr_global_DS_10deg.Draw("Pxsame")
	if(len(args)==3): gr_L1_DS_5deg.Draw("Pxsame")
	if(len(args)==3): gr_L2_DS_5deg.Draw("Pxsame")
	if(len(args)==3): gr_L3_DS_5deg.Draw("Pxsame")
	if(len(args)==3): gr_L4_DS_5deg.Draw("Pxsame")
	if(len(args)==3): gr_global_DS_5deg.Draw("Pxsame")
	
	legend = TLegend(0.13, 0.45, 0.7, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_L1_DS_18deg,"Layer 1 (18 Celcius)","pl")
	legend.AddEntry(gr_L1_DS_10deg,"Layer 1 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L1_DS_5deg,"Layer 1 (5 Celcius)","pl")
	legend.AddEntry(gr_L2_DS_18deg,"Layer 2 (18 Celcius)","pl")
	legend.AddEntry(gr_L2_DS_10deg,"Layer 2 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L2_DS_5deg,"Layer 2 (5 Celcius)","pl")
	legend.AddEntry(gr_L3_DS_18deg,"Layer 3 (18 Celcius)","pl")
	legend.AddEntry(gr_L3_DS_10deg,"Layer 3 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L3_DS_5deg,"Layer 3 (5 Celcius)","pl")
	legend.AddEntry(gr_L4_DS_18deg,"Layer 4 (18 Celcius)","pl")
	legend.AddEntry(gr_L4_DS_10deg,"Layer 4 (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_L4_DS_5deg,"Layer 4 (5 Celcius)","pl")
	legend.AddEntry(gr_global_DS_18deg,"ALL downstream (18 Celcius)","pl")
	legend.AddEntry(gr_global_DS_10deg,"ALL downstream (10 Celcius)","pl")
	if(len(args)==3):legend.AddEntry(gr_global_DS_5deg,"ALL downstream (5 Celcius)","pl")
	legend.Draw()
	c1.SaveAs("plots/downstream"+year+".png")
	
	#Difference plots
	size = 0.8
	gr_diff_L1 = TGraphErrors()
	gr_diff_L2 = TGraphErrors()
	gr_diff_L3 = TGraphErrors()
	gr_diff_L4 = TGraphErrors()
	gr_diff_G  = TGraphErrors()
	if temp_4PlottingDiff == "5": 
		gr_diff_L1 = f_5deg.Get("gr_layer1_diff_RMS")
		gr_diff_L2 = f_5deg.Get("gr_layer2_diff_RMS")
		gr_diff_L3 = f_5deg.Get("gr_layer3_diff_RMS")
		gr_diff_L4 = f_5deg.Get("gr_layer4_diff_RMS")
		gr_diff_G  = f_5deg.Get("gr_global_diff_RMS")
	if temp_4PlottingDiff == "10": 
		gr_diff_L1 = f_10deg.Get("gr_layer1_diff_RMS")
		gr_diff_L2 = f_10deg.Get("gr_layer2_diff_RMS")
		gr_diff_L3 = f_10deg.Get("gr_layer3_diff_RMS")
		gr_diff_L4 = f_10deg.Get("gr_layer4_diff_RMS")
		gr_diff_G  = f_10deg.Get("gr_global_diff_RMS")
	if temp_4PlottingDiff == "18": 
		gr_diff_L1 = f_18deg.Get("gr_layer1_diff_RMS")
		gr_diff_L2 = f_18deg.Get("gr_layer2_diff_RMS")
		gr_diff_L3 = f_18deg.Get("gr_layer3_diff_RMS")
		gr_diff_L4 = f_18deg.Get("gr_layer4_diff_RMS")
		gr_diff_G  = f_18deg.Get("gr_global_diff_RMS")
	gr_diff_L1.SetMarkerColor(kBlack)
	gr_diff_L2.SetMarkerColor(kRed)
	gr_diff_L3.SetMarkerColor(kBlue)
	gr_diff_L4.SetMarkerColor(kMagenta)
	gr_diff_G.SetMarkerColor(kBlack)
	gr_diff_L1.SetMarkerStyle(20)
	gr_diff_L2.SetMarkerStyle(20)
	gr_diff_L3.SetMarkerStyle(20)
	gr_diff_L4.SetMarkerStyle(20)
	gr_diff_G.SetMarkerStyle(kOpenCircle)
	gr_diff_L1.SetMarkerSize(size)
	gr_diff_L2.SetMarkerSize(size)
	gr_diff_L3.SetMarkerSize(size)
	gr_diff_L4.SetMarkerSize(size)
	gr_diff_G.SetMarkerSize(size)
	
	gr_diff_L1.Draw("APx")
	c1.SaveAs("plots/diff_L1_"+year+"_"+temp_4PlottingDiff+"C.png")
	gr_diff_L2.Draw("APx")
	c1.SaveAs("plots/diff_L2_"+year+"_"+temp_4PlottingDiff+"C.png")
	gr_diff_L3.Draw("APx")
	c1.SaveAs("plots/diff_L3_"+year+"_"+temp_4PlottingDiff+"C.png")
	gr_diff_L4.Draw("APx")
	c1.SaveAs("plots/diff_L4_"+year+"_"+temp_4PlottingDiff+"C.png")
	gr_diff_G.Draw("APx")
	c1.SaveAs("plots/diff_global_"+year+"_"+temp_4PlottingDiff+"C.png")
	
	legend = TLegend(0.13+0.15, 0.65, 0.5+0.15, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_diff_L1,"Layer 1","pl")
	legend.AddEntry(gr_diff_L2,"Layer 2 / 2","pl")
	legend.AddEntry(gr_diff_L3,"Layer 3 / 3","pl")
	legend.AddEntry(gr_diff_L4,"Layer 4 / 4","pl")
	gr_diff_L1.SetTitle("")
	gr_diff_L1.Draw("APx")
	gr_diff_L2.Draw("pxsame")
	gr_diff_L3.Draw("pxsame")
	gr_diff_L4.Draw("pxsame")
	legend.Draw()
	c1.SaveAs("plots/diff"+year+"_"+temp_4PlottingDiff+"C.png")
	
	
	
	
	
	
	
	
	
	
	
	
	
	


	print("Done ")


if __name__ == "__main__":
   main(sys.argv[1:])