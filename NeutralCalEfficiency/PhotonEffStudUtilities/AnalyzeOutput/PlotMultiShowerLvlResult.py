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
# from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *

#Colors
kRed = 632
kOrange = 800
kYellow = 400
kGreen = 416
kBlue = 600
kViolet = 880
kMagenta = 616
kBlack = 1

# string0 = "Photon Gun"
# string1 = "Photon Gun + proton"
# string2 = "Photon Gun + #pi^{+}#pi^{-} proton"
# string0 = "Shower Level Efficiency"
# string1 = "ReactionFilter Level Efficiency (no hadronic shower cuts)"
# string2 = "ReactionFilter Level Efficiency (with hadronic shower cuts)"
string0 = "Geant4 Photon Gun (#theta=6 degrees)"
string1 = "Geant3 Photon Gun (#theta=6 degrees)"
string2 = "ReactionFilter Level Efficiency (with hadronic shower cuts)"

def main(argv):
	#Usage controls from OptionParser
	parser_usage = ""
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(not len(args) >= 2):
		parser.print_help()
		return

	c1 = TCanvas("c1","c1",1600,900)
		
	file_list = []
	for i in range(0,len(args)): file_list.append(TFile.Open(argv[i],'read'))

	gr_effic_list = []
	
	#Draw first efficiency plot
	gr_effic_showerlvl = file_list[0].Get("gr_gauscore_effic")
	gr_effic_showerlvl.GetXaxis().SetTitle("E_{#gamma} (GeV)")
	gr_effic_showerlvl.GetYaxis().SetTitle("Efficiency")
	gr_effic_showerlvl.SetTitle("Photon Efficiency at #theta = 6 degrees")
	gr_effic_showerlvl.SetMarkerStyle(20)
	gr_effic_showerlvl.SetMarkerSize(0.5)
	gr_effic_showerlvl.SetLineColor(kBlack)
	gr_effic_showerlvl.SetMarkerColor(kBlack)
	gr_effic_showerlvl.GetYaxis().SetRangeUser(0.01,1.)
	gr_effic_showerlvl.Draw("AP")
	gr_effic_list.append(gr_effic_showerlvl)
	
	for i in range(1,len(file_list)):
		print "i is: " + str(i)
		gr_effic_showerlvl_curr = file_list[i].Get("gr_gauscore_effic")
		gr_effic_showerlvl_curr.SetMarkerStyle(20)
		gr_effic_showerlvl_curr.SetMarkerSize(0.5)
		if(i==1): gr_effic_showerlvl_curr.SetLineColor(kRed)
		if(i==1): gr_effic_showerlvl_curr.SetMarkerColor(kRed)
		if(i==2): gr_effic_showerlvl_curr.SetLineColor(kBlue)
		if(i==2): gr_effic_showerlvl_curr.SetMarkerColor(kBlue)
		gr_effic_showerlvl_curr.Draw("psame")
		gr_effic_list.append(gr_effic_showerlvl_curr)
	
	legend = TLegend(0.6, 0.1, 0.9, 0.4) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_effic_list[0],string0,"pl")
	legend.AddEntry(gr_effic_list[1],string1,"pl")
	if(len(file_list)>=3): legend.AddEntry(gr_effic_list[2],string2,"pl")
	legend.Draw()
	
	
	c1.SaveAs("GaussianCoreEfficiencyMulti.png")
	
	print("Done ")


if __name__ == "__main__":
   main(sys.argv[1:])