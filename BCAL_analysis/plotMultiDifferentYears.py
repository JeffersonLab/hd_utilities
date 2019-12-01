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

TEMP = "10"
kSquare = 20     #marker style, 10 degree data
size = 1.0 #Marker size for plotting

is_floor_subtracted = True #If true, modify y-axis labels


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

	diff_list_US = []
	diff_list_DS = []

	for l in range(1,5): #Loop over layers 1-4
		
		LAYER = str(l)
		
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
		gr_US_2015.SetTitle("Upstream Pedestal RMS, Layer "+LAYER+" "+TEMP+"C")
		gr_US_2015.GetXaxis().SetTitle("SiPM bias voltage (V)")
		if(is_floor_subtracted): gr_US_2015.GetYaxis().SetTitle("Pedestal RMS - Floor Term (ADC units)")
		if(not is_floor_subtracted): gr_US_2015.GetYaxis().SetTitle("Pedestal RMS (ADC units)")
		# gr_US_2019.GetYaxis().SetRangeUser(1,2.5)
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
		gr_DS_2015.SetTitle("Downstream Pedestal RMS, Layer "+LAYER+" "+TEMP+"C")
		gr_DS_2015.GetXaxis().SetTitle("SiPM bias voltage (V)")
		if(is_floor_subtracted): gr_DS_2015.GetYaxis().SetTitle("Pedestal RMS - Floor Term (ADC units)")
		if(not is_floor_subtracted): gr_DS_2015.GetYaxis().SetTitle("Pedestal RMS (ADC units)")
		# gr_DS_2015.GetYaxis().SetRangeUser(0.,1.5)
		legend = TLegend(0.13, 0.5, 0.6, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
		legend.AddEntry(gr_DS_2015,"2015 Downstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")
		legend.AddEntry(gr_DS_2017,"2017 Downstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")
		legend.AddEntry(gr_DS_2019,"2019 Downstream Layer "+LAYER+" ("+TEMP+" Celcius)","pl")

		gr_DS_2015.Draw("APx")
		gr_DS_2017.Draw("Pxsame")
		gr_DS_2019.Draw("Pxsame")
		legend.Draw()
		c1.SaveAs("plots/downstreamMultiyearLayer"+LAYER+"_"+TEMP+"C.png")


		gr_US_diff = get_avg_diff(gr_US_2015,gr_US_2017,gr_US_2019,LAYER,True)
		gr_DS_diff = get_avg_diff(gr_DS_2015,gr_DS_2017,gr_DS_2019,LAYER,False)
		
		gr_US_diff.Draw("APx")
		c1.SaveAs("plots/DIFF_upstream_multiyearLayer"+LAYER+"_"+TEMP+"C.png")
		gr_DS_diff.Draw("APx")
		c1.SaveAs("plots/DIFF_downstream_multiyearLayer"+LAYER+"_"+TEMP+"C.png")
		
		diff_list_US.append(gr_US_diff)
		diff_list_DS.append(gr_DS_diff)



	legend = TLegend(0.13, 0.65, 0.6, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(diff_list_US[0],"Upstream Layer 1","pl")
	legend.AddEntry(diff_list_US[1],"Upstream Layer 2 / 2","pl")
	legend.AddEntry(diff_list_US[2],"Upstream Layer 3 / 3","pl")
	legend.AddEntry(diff_list_US[3],"Upstream Layer 4 / 4","pl")
	diff_list_US[0].SetTitle("Pedestal RMS Difference (upstream)")
	diff_list_US[0].Draw("APx")
	diff_list_US[1].Draw("pxsame")
	diff_list_US[2].Draw("pxsame")
	diff_list_US[3].Draw("pxsame")
	legend.Draw()
	c1.SaveAs("plots/UPSTREAM_yearlydiff.png")

	legend = TLegend(0.13, 0.65, 0.6, 0.85) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
	legend.AddEntry(diff_list_DS[0],"Downstream Layer 1","pl")
	legend.AddEntry(diff_list_DS[1],"Downstream Layer 2 / 2","pl")
	legend.AddEntry(diff_list_DS[2],"Downstream Layer 3 / 3","pl")
	legend.AddEntry(diff_list_DS[3],"Downstream Layer 4 / 4","pl")
	diff_list_DS[0].SetTitle("Pedestal RMS Difference (downstream)")
	diff_list_DS[0].Draw("APx")
	diff_list_DS[1].Draw("pxsame")
	diff_list_DS[2].Draw("pxsame")
	diff_list_DS[3].Draw("pxsame")
	legend.Draw()
	c1.SaveAs("plots/DOWNSTREAM_yearlydiff.png")



	print("Done ")


def get_avg_diff(gr_2015,gr_2017,gr_2019,LAYER,is_upstream):
	
	bias_V_arr_2015   = array('d',[])
	bias_V_arr_2017   = array('d',[])
	bias_V_arr_2019   = array('d',[])
	RMS_arr_2015 = array('d',[])
	RMS_arr_2017 = array('d',[])
	RMS_arr_2019 = array('d',[])
	
	bias_V_arr_2015 = gr_2015.GetX()
	bias_V_arr_2017 = gr_2017.GetX()
	bias_V_arr_2019 = gr_2019.GetX()
	RMS_arr_2015 = gr_2015.GetY()
	RMS_arr_2017 = gr_2017.GetY()
	RMS_arr_2019 = gr_2019.GetY()
	
	dict_2015 = {}
	for i in range(0,len(bias_V_arr_2015)): dict_2015[bias_V_arr_2015[i]]=RMS_arr_2015[i]
	dict_2017 = {}
	for i in range(0,len(bias_V_arr_2017)): dict_2017[bias_V_arr_2017[i]]=RMS_arr_2017[i]
	dict_2019 = {}
	for i in range(0,len(bias_V_arr_2019)): dict_2019[bias_V_arr_2019[i]]=RMS_arr_2019[i]
	
	new_gr = gr_2015.Clone()
	# Note: bias V in 2015 is missing a few entries. Creating and looping over 2015 dictionary should avoid issue.
	counter = 0
	for biasV in dict_2015:
		# print "bias V: " + str(biasV)
		if biasV not in dict_2017: 
			print "Error: bias V not found in 2017!"
			return
		if biasV not in dict_2019: 
			print "Error: bias V not found in 2019!"
			return
		# If we get here, then bias V entry found for all years
		avg_2015_2017 = (dict_2015[biasV]+dict_2017[biasV])/2.
		diff = dict_2019[biasV]-avg_2015_2017
		new_gr.SetPoint(counter,biasV,diff)
		counter+=1

	if(is_upstream==True): new_gr.SetNameTitle("gr_layer"+LAYER+"_US_RMS","Pedestal RMS Diference Upstream Layer "+LAYER+";SiPM Bias (V);Pedestal RMS Difference (ADC units)")
	if(is_upstream==False): new_gr.SetNameTitle("gr_layer"+LAYER+"_DS_RMS","Pedestal RMS Diference Downstream Layer "+LAYER+";SiPM Bias (V);Pedestal RMS Difference (ADC units)")
	new_gr.SetMarkerStyle(20)
	new_gr.SetMarkerSize(size)
	if(LAYER=="1"): new_gr.SetMarkerColor(kBlack)
	if(LAYER=="2"): new_gr.SetMarkerColor(kRed)
	if(LAYER=="3"): new_gr.SetMarkerColor(kBlue)
	if(LAYER=="4"): new_gr.SetMarkerColor(kMagenta)
	
	
	return new_gr

if __name__ == "__main__":
   main(sys.argv[1:])