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
string0 = "2017 Data (\"tag-and-probe\")"
string1 = "geant3 signal MC (\"tag-and-probe\")"
string2 = "geant4 signal MC (\"tag-and-probe\")"
# string3 = "photon gun @ 1 GeV"
string3 = "photon gun, #omega phase space, E_{#gamma} gaussian yield"
# string0 = "#omega efficiency 2017 data (spectator photon > 300 MeV)"
# string1 = "#omega efficiency 2017 data (spectator photon > 600 MeV)"
# string2 = "#omega efficiency 2017 data (spectator photon > 1000 MeV)"

# METHOD = "1"
METHOD = "2"
PLOT_VAL = "E"
# PLOT_VAL = "Theta"


def main(argv):
	#Usage controls from OptionParser
	parser_usage = ""
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(not len(args) >= 2):
		parser.print_help()
		return

	c1 = TCanvas("c1","c1",1000,900)
		
	file_list = []
	for i in range(0,len(args)): file_list.append(TFile.Open(argv[i],'read'))

	gr_effic_list = []
		
	hist_name_string = "gr_m"+METHOD+"_effic_"+PLOT_VAL+"bins"
	
	#Draw first efficiency plot
	gr_effic_showerlvl = file_list[0].Get(hist_name_string)
	# gr_effic_showerlvl.GetXaxis().SetTitle("Missing Photon Polar Angle (degrees)")
	if(PLOT_VAL=="E"): gr_effic_showerlvl.GetXaxis().SetTitle("E_{#gamma} (GeV)")
	if(PLOT_VAL=="Theta"): gr_effic_showerlvl.GetXaxis().SetTitle("Missing Photon Polar Angle (degrees)")
	gr_effic_showerlvl.SetTitle("")
	gr_effic_showerlvl.GetYaxis().SetTitle("Efficiency")
	gr_effic_showerlvl.SetMarkerStyle(20)
	gr_effic_showerlvl.SetMarkerSize(1.15)
	gr_effic_showerlvl.SetLineColor(kBlack)
	gr_effic_showerlvl.SetMarkerColor(kBlack)
	if(PLOT_VAL=="Theta"): gr_effic_showerlvl.GetXaxis().SetRangeUser(0.,12.)
	gr_effic_showerlvl.GetYaxis().SetRangeUser(0.4,1.)
	gr_effic_showerlvl.Draw("AP")
	gr_effic_list.append(gr_effic_showerlvl)
	
	for i in range(1,len(file_list)):
		print "i is: " + str(i)
		gr_effic_showerlvl_curr = file_list[i].Get(hist_name_string)
		if(i==3): gr_effic_showerlvl_curr = file_list[i].Get("gr_gauscore_effic")
		gr_effic_showerlvl_curr.SetMarkerStyle(20)
		gr_effic_showerlvl_curr.SetMarkerSize(1.15)
		if(i==3): gr_effic_showerlvl_curr.SetMarkerSize(0.75)
		if(i==1): gr_effic_showerlvl_curr.SetLineColor(kRed)
		if(i==1): gr_effic_showerlvl_curr.SetMarkerColor(kRed)
		if(i==2): gr_effic_showerlvl_curr.SetLineColor(kBlue)
		if(i==2): gr_effic_showerlvl_curr.SetMarkerColor(kBlue)
		if(i==3): gr_effic_showerlvl_curr.SetLineColor(kGreen+2)
		if(i==3): gr_effic_showerlvl_curr.SetMarkerColor(kGreen+2)
		if(i==3): gr_effic_showerlvl_curr.SetMarkerStyle(26)
		gr_effic_showerlvl_curr.Draw("psame")
		gr_effic_list.append(gr_effic_showerlvl_curr)
	
	
	
	legend = TLegend(0.28, 0.15, 0.888, 0.45) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(gr_effic_list[0],string0,"pl")
	legend.AddEntry(gr_effic_list[1],string1,"pl")
	if(len(file_list)>=3): legend.AddEntry(gr_effic_list[2],string2,"pl")
	if(len(file_list)>=4): legend.AddEntry(gr_effic_list[3],string3,"pl")
	legend.Draw()
	
	
	c1.SaveAs("OmegaCompare"+PLOT_VAL+".png")
	
	print("Done ")


if __name__ == "__main__":
   main(sys.argv[1:])