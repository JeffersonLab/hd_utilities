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
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TGraphErrors, TGraph, TLine
from ROOT import gBenchmark, gDirectory, gROOT, gStyle, gPad

#My Stuff
#Required: add to PYTHONPATH environment variable, e.g.
#setenv PYTHONPATH /gpfs/home/j/z/jzarling/Karst/MyAnalyses/Python_stuff/jz_library/:$PYTHONPATH
from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *


histbasename = "pedmeansub_"


temp = "10_C"
year = "2015"
base_directory = "/w/halld-scifs17exp/home/jzarling/BCAL_stuff/pedestal_width_study/"+year+"/"+temp+"/"

subtract_floor_term = True

def main(argv):
	#Usage controls from OptionParser
	parser_usage = "outputfile.root"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) != 1):
		parser.print_help()
		return

	#Get files, add to another dictionary
	run_dict = get_run_dict()
	f_list = {}
	for runnum in run_dict:
		print "Run: " + str(runnum)
		filesmatching = glob.glob(base_directory+"*_"+str(runnum)+"*.root")
		if(len(filesmatching)!=1):
			print "ERROR FINDING FILE: " + base_directory+"hd_root_"+str(runnum)+"*.root"
			print "exiting..." 
			return
		f = TFile.Open(filesmatching[0])
		f_list[runnum] = f

	rocid_arr = array('i',[31,32,34,35,37,38,40,41]) #These are the rocids associated with BCAL fADC250s
	slot_arr = array('i',[3,4,5,6,7,8,9,10,13,14,15,16]) #These are the slots used for BCAL fADC250s, common to all crates
	channel_arr = array('i',[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]) #These are the channels used on each slot, common to all slots
	if(year=="2015"): rocid_arr = array('i',[31,32,34,35]) #These are the rocids associated with BCAL fADC250s

	h_layer1_US_RMS_arr = []
	h_layer2_US_RMS_arr = []
	h_layer3_US_RMS_arr = []
	h_layer4_US_RMS_arr = []
	h_global_US_RMS_arr = []
	h_layer1_DS_RMS_arr = []
	h_layer2_DS_RMS_arr = []
	h_layer3_DS_RMS_arr = []
	h_layer4_DS_RMS_arr = []
	h_global_DS_RMS_arr = []
	h_global_all_RMS_arr = []

	#A whole big bunch of arrays for TGraphErrors
	mean_layer1_US_RMS  = array('d',[])
	mean_layer2_US_RMS  = array('d',[])
	mean_layer3_US_RMS  = array('d',[])
	mean_layer4_US_RMS  = array('d',[])
	mean_global_US_RMS  = array('d',[])
	mean_layer1_DS_RMS  = array('d',[])
	mean_layer2_DS_RMS  = array('d',[])
	mean_layer3_DS_RMS  = array('d',[])
	mean_layer4_DS_RMS  = array('d',[])
	mean_global_DS_RMS  = array('d',[])
	mean_global_all_RMS = array('d',[])
	mean_err_layer1_US_RMS  = array('d',[])
	mean_err_layer2_US_RMS  = array('d',[])
	mean_err_layer3_US_RMS  = array('d',[])
	mean_err_layer4_US_RMS  = array('d',[])
	mean_err_global_US_RMS  = array('d',[])
	mean_err_layer1_DS_RMS  = array('d',[])
	mean_err_layer2_DS_RMS  = array('d',[])
	mean_err_layer3_DS_RMS  = array('d',[])
	mean_err_layer4_DS_RMS  = array('d',[])
	mean_err_global_DS_RMS  = array('d',[])
	mean_err_global_all_RMS = array('d',[])
	bias_arr     = array('d',[])
	bias_err_arr = array('d',[])
	mean_layer1_diff_RMS  = array('d',[])
	mean_layer2_diff_RMS  = array('d',[])
	mean_layer3_diff_RMS  = array('d',[])
	mean_layer4_diff_RMS  = array('d',[])
	mean_global_diff_RMS  = array('d',[])
	mean_err_layer1_diff_RMS  = array('d',[])
	mean_err_layer2_diff_RMS  = array('d',[])
	mean_err_layer3_diff_RMS  = array('d',[])
	mean_err_layer4_diff_RMS  = array('d',[])
	mean_err_global_diff_RMS  = array('d',[])

	#Loop over all files
	for curr_run in f_list:
		print "Runnum: " + str(curr_run)
		print "bias V: " + str(run_dict[curr_run])
	
		curr_file = f_list[curr_run]
		curr_bias = run_dict[curr_run]
	
		#Create histograms
		hist_min = 0.
		hist_max = 4.
		h_layer1_US_RMS = TH1F("h_layer1_US_RMS_bias"+str(curr_bias),"RMS of US Layer 1 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer2_US_RMS = TH1F("h_layer2_US_RMS_bias"+str(curr_bias),"RMS of US Layer 2 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer3_US_RMS = TH1F("h_layer3_US_RMS_bias"+str(curr_bias),"RMS of US Layer 3 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer4_US_RMS = TH1F("h_layer4_US_RMS_bias"+str(curr_bias),"RMS of US Layer 4 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_global_US_RMS = TH1F("h_global_US_RMS_bias"+str(curr_bias),"RMS of All US BCAL Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer1_DS_RMS = TH1F("h_layer1_DS_RMS_bias"+str(curr_bias),"RMS of DS Layer 1 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer2_DS_RMS = TH1F("h_layer2_DS_RMS_bias"+str(curr_bias),"RMS of DS Layer 2 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer3_DS_RMS = TH1F("h_layer3_DS_RMS_bias"+str(curr_bias),"RMS of DS Layer 3 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_layer4_DS_RMS = TH1F("h_layer4_DS_RMS_bias"+str(curr_bias),"RMS of DS Layer 4 Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_global_DS_RMS = TH1F("h_global_DS_RMS_bias"+str(curr_bias),"RMS of All DS BCAL Channels;RMS (ADC units)",1000,hist_min,hist_max)
		h_global_all_RMS = TH1F("h_global_all_RMS_bias"+str(curr_bias),"RMS of All BCAL Channels;RMS (ADC units)",1000,hist_min,hist_max)
		#Loop over all channels
		for rocid in rocid_arr:
			for slot in slot_arr:
				for channel in channel_arr:
					hist = get_hist_from_rocid_slot_channel(curr_file,rocid,slot,channel)
					quadrant = getquadrant(rocid)
					layer    = getlayer(slot,channel)
					is_downstream    = getend(slot,channel)
					# if(quadrant==3): continue #Skip quadrant 3, it has LEDs firing (in 2019 at least)
					if(quadrant==3 or quadrant==2 or quadrant==4): continue #Skip quadrant 3, it has LEDs firing (in 2019 at least)
					#Fill appropriate histograms
					if(layer==1 and is_downstream==0): h_layer1_US_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(layer==2 and is_downstream==0): h_layer2_US_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(layer==3 and is_downstream==0): h_layer3_US_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(layer==4 and is_downstream==0): h_layer4_US_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(is_downstream==0): h_global_US_RMS.Fill(hist.GetRMS())
					if(layer==1 and is_downstream==1): h_layer1_DS_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(layer==2 and is_downstream==1): h_layer2_DS_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(layer==3 and is_downstream==1): h_layer3_DS_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(layer==4 and is_downstream==1): h_layer4_DS_RMS.Fill(hist.GetRMS()/sqrt(layer))
					if(is_downstream==1): h_global_DS_RMS.Fill(hist.GetRMS())
					h_global_all_RMS.Fill(hist.GetRMS())
		#End of file: add histograms to list
		h_layer1_US_RMS_arr.append(h_layer1_US_RMS)
		h_layer2_US_RMS_arr.append(h_layer2_US_RMS)
		h_layer3_US_RMS_arr.append(h_layer3_US_RMS)
		h_layer4_US_RMS_arr.append(h_layer4_US_RMS)
		h_global_US_RMS_arr.append(h_global_US_RMS)
		h_layer1_DS_RMS_arr.append(h_layer1_DS_RMS)
		h_layer2_DS_RMS_arr.append(h_layer2_DS_RMS)
		h_layer3_DS_RMS_arr.append(h_layer3_DS_RMS)
		h_layer4_DS_RMS_arr.append(h_layer4_DS_RMS)
		h_global_DS_RMS_arr.append(h_global_DS_RMS)
		h_global_all_RMS_arr.append(h_global_all_RMS)
		
		mean_layer1_US_RMS.append(h_layer1_US_RMS.GetMean())
		mean_layer2_US_RMS.append(h_layer2_US_RMS.GetMean())
		mean_layer3_US_RMS.append(h_layer3_US_RMS.GetMean())
		mean_layer4_US_RMS.append(h_layer4_US_RMS.GetMean())
		mean_global_US_RMS.append(h_global_US_RMS.GetMean())
		mean_layer1_DS_RMS.append(h_layer1_DS_RMS.GetMean())
		mean_layer2_DS_RMS.append(h_layer2_DS_RMS.GetMean())
		mean_layer3_DS_RMS.append(h_layer3_DS_RMS.GetMean())
		mean_layer4_DS_RMS.append(h_layer4_DS_RMS.GetMean())
		mean_global_DS_RMS.append(h_global_DS_RMS.GetMean())
		mean_global_all_RMS.append(h_global_all_RMS.GetMean())
		mean_err_layer1_US_RMS.append(h_layer1_US_RMS.GetRMS())
		mean_err_layer2_US_RMS.append(h_layer2_US_RMS.GetRMS())
		mean_err_layer3_US_RMS.append(h_layer3_US_RMS.GetRMS())
		mean_err_layer4_US_RMS.append(h_layer4_US_RMS.GetRMS())
		mean_err_global_US_RMS.append(h_global_US_RMS.GetRMS())
		mean_err_layer1_DS_RMS.append(h_layer1_DS_RMS.GetRMS())
		mean_err_layer2_DS_RMS.append(h_layer2_DS_RMS.GetRMS())
		mean_err_layer3_DS_RMS.append(h_layer3_DS_RMS.GetRMS())
		mean_err_layer4_DS_RMS.append(h_layer4_DS_RMS.GetRMS())
		mean_err_global_DS_RMS.append(h_global_DS_RMS.GetRMS())
		mean_err_global_all_RMS.append(h_global_all_RMS.GetRMS())
		bias_arr.append(curr_bias)
		bias_err_arr.append(0)
		
		mean_layer1_diff_RMS.append(h_layer1_DS_RMS.GetMean()-h_layer1_US_RMS.GetMean())
		mean_layer2_diff_RMS.append(h_layer2_DS_RMS.GetMean()-h_layer2_US_RMS.GetMean())
		mean_layer3_diff_RMS.append(h_layer3_DS_RMS.GetMean()-h_layer3_US_RMS.GetMean())
		mean_layer4_diff_RMS.append(h_layer4_DS_RMS.GetMean()-h_layer4_US_RMS.GetMean())
		mean_global_diff_RMS.append(h_global_DS_RMS.GetMean()-h_global_US_RMS.GetMean())
		mean_err_layer1_diff_RMS.append(sqrt(h_layer1_DS_RMS.GetRMS()**2+h_layer1_US_RMS.GetRMS()**2))
		mean_err_layer2_diff_RMS.append(sqrt(h_layer2_DS_RMS.GetRMS()**2+h_layer2_US_RMS.GetRMS()**2))
		mean_err_layer3_diff_RMS.append(sqrt(h_layer3_DS_RMS.GetRMS()**2+h_layer3_US_RMS.GetRMS()**2))
		mean_err_layer4_diff_RMS.append(sqrt(h_layer4_DS_RMS.GetRMS()**2+h_layer4_US_RMS.GetRMS()**2))
		mean_err_global_diff_RMS.append(sqrt(h_global_DS_RMS.GetRMS()**2+h_global_US_RMS.GetRMS()**2))

	gr_layer1_US_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer1_US_RMS,bias_err_arr,mean_err_layer1_US_RMS)
	gr_layer2_US_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer2_US_RMS,bias_err_arr,mean_err_layer2_US_RMS)
	gr_layer3_US_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer3_US_RMS,bias_err_arr,mean_err_layer3_US_RMS)
	gr_layer4_US_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer4_US_RMS,bias_err_arr,mean_err_layer4_US_RMS)
	gr_global_US_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_global_US_RMS,bias_err_arr,mean_err_global_US_RMS)
	gr_layer1_DS_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer1_DS_RMS,bias_err_arr,mean_err_layer1_DS_RMS)
	gr_layer2_DS_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer2_DS_RMS,bias_err_arr,mean_err_layer2_DS_RMS)
	gr_layer3_DS_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer3_DS_RMS,bias_err_arr,mean_err_layer3_DS_RMS)
	gr_layer4_DS_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer4_DS_RMS,bias_err_arr,mean_err_layer4_DS_RMS)
	gr_global_DS_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_global_DS_RMS,bias_err_arr,mean_err_global_DS_RMS)
	gr_global_all_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_global_all_RMS,bias_err_arr,mean_err_global_DS_RMS)
	gr_layer1_diff_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer1_diff_RMS,bias_err_arr,mean_err_layer1_diff_RMS)
	gr_layer2_diff_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer2_diff_RMS,bias_err_arr,mean_err_layer2_diff_RMS)
	gr_layer3_diff_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer3_diff_RMS,bias_err_arr,mean_err_layer3_diff_RMS)
	gr_layer4_diff_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_layer4_diff_RMS,bias_err_arr,mean_err_layer4_diff_RMS)
	gr_global_diff_RMS = TGraphErrors(len(bias_arr),bias_arr,mean_global_diff_RMS,bias_err_arr,mean_err_global_diff_RMS)
	if(subtract_floor_term):
		gr_layer1_US_RMS   = remove_floor_term_from_gr(gr_layer1_US_RMS)
		gr_layer2_US_RMS   = remove_floor_term_from_gr(gr_layer2_US_RMS)
		gr_layer3_US_RMS   = remove_floor_term_from_gr(gr_layer3_US_RMS)
		gr_layer4_US_RMS   = remove_floor_term_from_gr(gr_layer4_US_RMS)
		gr_global_US_RMS   = remove_floor_term_from_gr(gr_global_US_RMS)
		gr_layer1_DS_RMS   = remove_floor_term_from_gr(gr_layer1_DS_RMS)
		gr_layer2_DS_RMS   = remove_floor_term_from_gr(gr_layer2_DS_RMS)
		gr_layer3_DS_RMS   = remove_floor_term_from_gr(gr_layer3_DS_RMS)
		gr_layer4_DS_RMS   = remove_floor_term_from_gr(gr_layer4_DS_RMS)
		gr_global_DS_RMS   = remove_floor_term_from_gr(gr_global_DS_RMS)
		gr_global_all_RMS  = remove_floor_term_from_gr(gr_global_all_RMS)
		gr_layer1_diff_RMS = remove_floor_term_from_gr_diff(gr_layer1_US_RMS,gr_layer1_DS_RMS)
		gr_layer2_diff_RMS = remove_floor_term_from_gr_diff(gr_layer2_US_RMS,gr_layer2_DS_RMS)
		gr_layer3_diff_RMS = remove_floor_term_from_gr_diff(gr_layer3_US_RMS,gr_layer3_DS_RMS)
		gr_layer4_diff_RMS = remove_floor_term_from_gr_diff(gr_layer4_US_RMS,gr_layer4_DS_RMS)
		gr_global_diff_RMS = remove_floor_term_from_gr_diff(gr_global_US_RMS,gr_global_DS_RMS)
	gr_layer1_US_RMS.SetNameTitle("gr_layer1_US_RMS","Layer 1 Upstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer2_US_RMS.SetNameTitle("gr_layer2_US_RMS","Layer 2 Upstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer3_US_RMS.SetNameTitle("gr_layer3_US_RMS","Layer 3 Upstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer4_US_RMS.SetNameTitle("gr_layer4_US_RMS","Layer 4 Upstream;Bias (V);Pedestal Width (ADC units)")
	gr_global_US_RMS.SetNameTitle("gr_global_US_RMS","ALL Upstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer1_DS_RMS.SetNameTitle("gr_layer1_DS_RMS","Layer 1 Downstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer2_DS_RMS.SetNameTitle("gr_layer2_DS_RMS","Layer 2 Downstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer3_DS_RMS.SetNameTitle("gr_layer3_DS_RMS","Layer 3 Downstream;Bias (V);Pedestal Width (ADC units)")
	gr_layer4_DS_RMS.SetNameTitle("gr_layer4_DS_RMS","Layer 4 Downstream;Bias (V);Pedestal Width (ADC units)")
	gr_global_DS_RMS.SetNameTitle("gr_global_DS_RMS","ALL Downstream;Bias (V);Pedestal Width (ADC units)")
	gr_global_all_RMS.SetNameTitle("gr_global_all_RMS","ALL Channels;Bias (V);Pedestal Width (ADC units)")
	gr_layer1_diff_RMS.SetNameTitle("gr_layer1_diff_RMS","Layer 1 Downstream - Upstream;Bias (V);Pedestal Width Difference (ADC units)")
	gr_layer2_diff_RMS.SetNameTitle("gr_layer2_diff_RMS","Layer 2 Downstream - Upstream;Bias (V);Pedestal Width Difference (ADC units)")
	gr_layer3_diff_RMS.SetNameTitle("gr_layer3_diff_RMS","Layer 3 Downstream - Upstream;Bias (V);Pedestal Width Difference (ADC units)")
	gr_layer4_diff_RMS.SetNameTitle("gr_layer4_diff_RMS","Layer 4 Downstream - Upstream;Bias (V);Pedestal Width Difference (ADC units)")
	gr_global_diff_RMS.SetNameTitle("gr_global_diff_RMS","ALL Downstream - Upstream;Bias (V);Pedestal Width Difference (ADC units)")

	
	
	#Save results to file
	f_out = TFile(argv[0],"RECREATE")
	f_out.cd()
	gr_layer1_US_RMS.Write()
	gr_layer2_US_RMS.Write()
	gr_layer3_US_RMS.Write()
	gr_layer4_US_RMS.Write()
	gr_global_US_RMS.Write()
	gr_layer1_DS_RMS.Write()
	gr_layer2_DS_RMS.Write()
	gr_layer3_DS_RMS.Write()
	gr_layer4_DS_RMS.Write()
	gr_global_DS_RMS.Write()
	gr_global_all_RMS.Write()
	gr_layer1_diff_RMS.Write()
	gr_layer2_diff_RMS.Write()
	gr_layer3_diff_RMS.Write()
	gr_layer4_diff_RMS.Write()
	gr_global_diff_RMS.Write()
	for i in range(0,len(h_layer1_US_RMS_arr)):
		h_layer1_US_RMS_arr[i].Write()
		h_layer2_US_RMS_arr[i].Write()
		h_layer3_US_RMS_arr[i].Write()
		h_layer4_US_RMS_arr[i].Write()
		h_global_US_RMS_arr[i].Write()
		h_layer1_DS_RMS_arr[i].Write()
		h_layer2_DS_RMS_arr[i].Write()
		h_layer3_DS_RMS_arr[i].Write()
		h_layer4_DS_RMS_arr[i].Write()
		h_global_DS_RMS_arr[i].Write()
		h_global_all_RMS_arr[i].Write()
	f_out.Close()

	print("Done")
	return














# Dictionary of runs and corresponding bias voltages (negative actually corresponds to OFF)
# Hardcoded is fine --- these run numbers ain't gonna change ever
def get_run_dict():
	run_dict = {}
	if(year=="2019"):
		if(temp=="18_C"):
			run_dict = {
				70432:-0.2,
				70434:0.0,
				70435:0.2,
				70436:0.4,
				70437:0.6,
				70438:0.8,
				70439:1.0,
				70441:1.2,
				70442:1.4,
				70443:1.6,
				70444:1.8,
				70445:2.0,
			}
		if(temp=="10_C"):
			run_dict = {
				70446:-0.2,
				70447:0.0,
				70448:0.2,
				70449:0.4,
				70450:0.6,
				70451:0.8,
				70452:1.0,
				70453:1.2,
				70454:1.4,
				70455:1.6,
				70456:1.8,
				70457:2.0,
			}
		if(temp=="5_C"):
			run_dict = {
				# 2019 5 degree data
				70407:-0.2,
				70408:0.0,
				70409:0.2,
				70410:0.4,
				70411:0.6,
				70413:0.8,
				70414:1.0,
				70415:1.2,
				70417:1.4,
				70418:1.6,
				70419:1.8,
				70420:2.0,
			}
	
	if(year=="2017"):
		if(temp=="18_C"):
			run_dict = {
				31443:2.0 ,
				31445:1.8 ,
				31447:1.6 ,
				31449:1.4 ,
				31451:1.2 ,
				31454:1.0 ,
				31456:0.8 ,
				31458:0.6 ,
				31460:0.4 ,
				31462:0.2 ,
				31464:0.0 ,
				31466:-0.2,
			}
		if(temp=="10_C"):
			run_dict = {
				31473:2.0 ,
				31475:1.8 ,
				31477:1.6 ,
				31479:1.4 ,
				31482:1.2 ,
				31484:1.0 ,
				31486:0.8 ,
				31488:0.6 ,
				31490:0.4 ,
				31493:0.2 ,
				31495:0.0 ,
				31497:-0.2,
			}
		if(temp=="5_C"):
			run_dict = {
				31499:2.0 ,
				31501:1.8 ,
				31504:1.6 ,
				31506:1.4 ,
				31508:1.2 ,
				31511:1.0 ,
				31513:0.8 ,
				31515:0.6 ,
				31517:0.4 ,
				31519:0.2 ,
				31521:0.0 ,
				31523:-0.2,
			}
	if(year=="2015"):
		if(temp=="18_C"):
			run_dict = {
				2832:-0.2,
				2803:0.2,
				2807:0.4,
				2808:0.6,
				2813:0.8,
				2816:1.0,
				2825:1.2,
				2827:1.4,
				2829:1.6,
				2830:1.8,
				2831:2.0,
			}
		if(temp=="10_C"):
			run_dict = {
				2787:-0.2,
				2772:0.0,
				2782:0.2,
				2775:0.6,
				2786:0.8,
				2778:1.0,
				2779:1.2,
				2760:1.4,
				2780:1.6,
				2795:1.8,
				2798:2.0,
			}
	return run_dict


def get_hist_from_rocid_slot_channel(f,rocid,slot,channel):
	slot_str = str(slot)
	if(slot<=9): slot_str="0"+str(slot) #add leading zero to one digit names
	channel_str = str(channel)
	if(channel<=9): channel_str="0"+str(channel) #add leading zero to one digit names
	histname_str = histbasename+str(rocid)+"_"+slot_str+"_"+channel_str
	# print "Histogram name: " + histname_str
	hist = f.Get(histname_str)
	if(hist==None): 
		print "ERROR: histogram not found"
		print "searching for histogram: " + histname_str
	return hist

def remove_floor_term_from_gr(gr):
	
	bias_V_arr = array('d',[])
	RMS_arr    = array('d',[])
	
	bias_V_arr = gr.GetX()
	RMS_arr = gr.GetY()
	
	min_RMS = 1000
	for i in range(0,len(RMS_arr)):
		if RMS_arr[i]<min_RMS:
			min_RMS = RMS_arr[i]
	print "min RMS is: " + str(min_RMS)
	
	# RMS_floorsub_arr    = array('d',[])
	# for i in range(0,len(RMS_arr)): RMS_floorsub_arr.append(RMS_arr[i]-min_RMS)
	
	new_gr = gr.Clone()
	for i in range(0,len(RMS_arr)): 
		new_gr.SetPoint(i,bias_V_arr[i],RMS_arr[i]-min_RMS)
		
	return new_gr
	
def remove_floor_term_from_gr_diff(gr_up,gr_down):
	
	bias_V_arr_up = array('d',[])
	RMS_arr_up    = array('d',[])
	bias_V_arr_up = gr_up.GetX()
	RMS_arr_up = gr_up.GetY()
	min_RMS_up = 1000
	for i in range(0,len(RMS_arr_up)):
		if RMS_arr_up[i]<min_RMS_up:
			min_RMS_up = RMS_arr_up[i]
	print "min RMS upstream: " + str(min_RMS_up)
	
	bias_V_arr_down = array('d',[])
	RMS_arr_down    = array('d',[])
	bias_V_arr_down = gr_down.GetX()
	RMS_arr_down = gr_down.GetY()
	min_RMS_down = 1000
	for i in range(0,len(RMS_arr_down)):
		if RMS_arr_down[i]<min_RMS_down:
			min_RMS_down = RMS_arr_down[i]
	print "min RMS downstream: " + str(min_RMS_down)
	
	min_RMS_down = RMS_arr_down[0]
	min_RMS_up = RMS_arr_up[0]
	
	
	new_gr = gr_down.Clone()
	for i in range(0,len(RMS_arr_down)): 
		down_subtracted = RMS_arr_down[i]-min_RMS_down
		up_subtracted = RMS_arr_up[i]-min_RMS_up
		# print "First bin down: " + str(down_subtracted)
		# print "First bin up: " + str(up_subtracted)
		
		
		new_gr.SetPoint(i,bias_V_arr_up[i],down_subtracted-up_subtracted)
		
	return new_gr
	
	
	
	
# Conversion functions
###################################
###################################
###################################
###################################

#Very helpful link! https://www.jlab.org/Hall-D/test/TranslationTable/index.php?table=BCAL

#Determine what layer a channel belongs to. Same convention for all crates
def getlayer(slot,channel):
	layer = 0
	if (slot%2==1): #slot is odd
		if (channel==0 or channel==1 or channel==2 or channel==3 or channel==8 or channel==9 or channel==10 or channel==11):
			layer=1;
		else:
			layer=2;
	else:  #slot is even
		if(channel==0 or channel==1 or channel==2 or channel==3 or channel==8 or channel==9 or channel==10 or channel==11):
			layer=3;
		else:
			layer=4;
	return layer;


#Upstream or downstream end. Upstream = end 0, Downstream = end 1 here.
###Upstream = channels 0-7
###Downstream = 8 and up
def getend(slot, channel):
	end=1;
	if (channel<=7): end=0;
	return end;

#Get quadrant from crates
def getquadrant(rocid):
    if(rocid==31 or rocid==34): return 1
    if(rocid==37 or rocid==40): return 2
    if(rocid==38 or rocid==41): return 3
    if(rocid==32 or rocid==35): return 4
    #shouldn't ever get to this point...
    print "ERROR: QUADRANT NOT FOUND FOR ROCID=" + str(rocid)
    print "RETURNING -1..." 
    return -1






# dictionary of Run:bias V. 
# run_dict = {	
	# # 2019 5 degree data
	# 70407:-0.2,
	# 70408:0.0,
	# 70409:0.2,
	# 70410:0.4,
	# 70411:0.6,
	# 70413:0.8,
	# 70414:1.0,
	# 70415:1.2,
	# 70417:1.4,
	# 70418:1.6,
	# 70419:1.8,
	# 70420:2.0,
	
	# # 2019 10 degree data
	# 70446:-0.2,
	# 70447:0.0,
	# 70448:0.2,
	# 70449:0.4,
	# 70450:0.6,
	# 70451:0.8,
	# 70452:1.0,
	# 70453:1.2,
	# 70454:1.4,
	# 70455:1.6,
	# 70456:1.8,
	# 70457:2.0,
	
	# # 2019 18 degree data
	# 70432:-0.2,
	# 70434:0.0,
	# 70435:0.2,
	# 70436:0.4,
	# 70437:0.6,
	# 70438:0.8,
	# 70439:1.0,
	# 70441:1.2,
	# 70442:1.4,
	# 70443:1.6,
	# 70444:1.8,
	# 70445:2.0,
	
	# # 2017 5 Degree data
	# 31499:2.0 ,
	# 31501:1.8 ,
	# 31504:1.6 ,
	# 31506:1.4 ,
	# 31508:1.2 ,
	# 31511:1.0 ,
	# 31513:0.8 ,
	# 31515:0.6 ,
	# 31517:0.4 ,
	# 31519:0.2 ,
	# 31521:0.0 ,
	# 31523:-0.2,
	
	# # 2017 10 Degree data
	# 31473:2.0 ,
	# 31475:1.8 ,
	# 31477:1.6 ,
	# 31479:1.4 ,
	# 31482:1.2 ,
	# 31484:1.0 ,
	# 31486:0.8 ,
	# 31488:0.6 ,
	# 31490:0.4 ,
	# 31493:0.2 ,
	# 31495:0.0 ,
	# 31497:-0.2,
	
	# # 2017 18 Degree data
	# 31443:2.0 ,
	# 31445:1.8 ,
	# 31447:1.6 ,
	# 31449:1.4 ,
	# 31451:1.2 ,
	# 31454:1.0 ,
	# 31456:0.8 ,
	# 31458:0.6 ,
	# 31460:0.4 ,
	# 31462:0.2 ,
	# 31464:0.0 ,
	# 31466:-0.2,
		
	# # 2015 10 degree data (min=2760,max=2787)
	# 2787:-0.2,
	# 2772:0.0,
	# 2782:0.2,
	# 2775:0.6,
	# 2786:0.8,
	# 2778:1.0,
	# 2779:1.2,
	# 2760:1.4,
	# 2780:1.6,
	# 2795:1.8,
	# 2798:2.0,
	
	# 2015 18 degree data (min=2803,max=2832)
	# 2832:-0.2,
	# 2803:0.2,
	# 2807:0.4,
	# 2808:0.6,
	# 2813:0.8,
	# 2816:1.0,
	# 2825:1.2,
	# 2827:1.4,
	# 2829:1.6,
	# 2830:1.8,
	# 2831:2.0,
	# }




if __name__ == "__main__":
   main(sys.argv[1:])