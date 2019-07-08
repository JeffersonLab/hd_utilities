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
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TGraphErrors, TAxis, TF1
from ROOT import gBenchmark, gDirectory, gROOT

# SCAN_OVER_ENERGY = True
# SCAN_OVER_THETA  = False
SCAN_OVER_ENERGY = False
SCAN_OVER_THETA  = True

POLY_ORDER = 2 #Set to 0 for pure gaussian
E_BELOWTHROWN_TOFIT = 0.6 # lower range of histogram fitting: E_thrown - this_val
E_ABOVETHROWN_TOFIT = 0.6 # upper range of histogram fitting: E_thrown + this_val

MIN_EVENTS = 10

#Colors
kRed = 632
kOrange = 800
kYellow = 400
kGreen = 416
kBlue = 600
kViolet = 880
kMagenta = 616
kBlack = 1


def main(argv):
	#Usage controls from OptionParser
	parser_usage = "outputfile.root filename1.root ... filenameN.root"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) == 0):
		parser.print_help()
		return
		
	if(SCAN_OVER_ENERGY and SCAN_OVER_THETA):
		print "ERROR: both scan flags turned on! Please select either energy or theta to scan over..."
		return
	if(not SCAN_OVER_ENERGY and not SCAN_OVER_THETA):
		print "ERROR: neither scan flags turned on! Please select either energy or theta to scan over..."
		return
		
	c1 = TCanvas("c1","c1",1600,900)
	
	
	
	file_list = []
	curr_P_val = 0.1
	for i in range(1,len(args)): file_list.append(TFile.Open(argv[i],'read'))
		
	scan_val_arr = array('d',[])
	scan_val_arr_arr_err = array('d',[])
	
	
	effic_gauscore_arr = array('d',[])
	effic_gauscore_err_arr = array('d',[])
	effic_anyquality_arr = array('d',[])
	effic_anyquality_err_arr = array('d',[])
	
	effic_gauscore_1show_arr = array('d',[])
	effic_gauscore_1show_err_arr = array('d',[])
	effic_anyquality_1show_arr = array('d',[])
	effic_anyquality_1show_err_arr = array('d',[])
		
	# curr_P_val = 0.1
	for i in range(0,len(file_list)):
		print "Current file: " + argv[i+1]
		
		
		#For normalization
		# h_curr_showers = file_list[i].Get("pi_pm_gun_hists/h_NFCALShowers")
		h_ThrownPionP_curr = TH1F()
		h_ThrownPhotonTheta_curr = TH1F()
		h_ThrownPionP_curr = file_list[i].Get("pi_pm_gun_hists/h_ThrownPionP")
		h_ThrownPhotonTheta_curr  = file_list[i].Get("pi_pm_gun_hists/h_ThrownPionTheta")
		norm_count = h_ThrownPionP_curr.GetEntries()
		if(norm_count < MIN_EVENTS): continue
		# atleast_one_shower = norm_count-h_curr_showers.GetBinContent(1) #Bin 1 corresponds to 0 showers
		# curr_P_val = my_gaus_fit.GetParameter(1)
		curr_P_val = h_ThrownPionP_curr.GetBinLowEdge( h_ThrownPionP_curr.GetMaximumBin()+1 )
		curr_theta_val = h_ThrownPhotonTheta_curr.GetBinLowEdge( h_ThrownPhotonTheta_curr.GetMaximumBin()+1 )
		print "Current P: " + str(curr_P_val)
		print "Current theta: " + str(curr_theta_val) + "\n"
		
		BELOW_MEAN_TO_FIT = E_BELOWTHROWN_TOFIT*curr_P_val/2
		ABOVE_MEAN_TO_FIT = E_BELOWTHROWN_TOFIT*curr_P_val/2
		
		h_curr = file_list[i].Get("pi_pm_gun_hists/h_ChargedTrackHypothesis_P")
		my_gaus_fit = TF1("my_gaus_fit","gausn",0.001,12.)
		my_gaus_fit.SetParLimits(0,0,100000)
		my_gaus_fit.SetParLimits(1,curr_P_val-0.2,curr_P_val+0.1)
		my_gaus_fit.SetParLimits(2,0.005,0.4)
		my_gaus_fit.SetNpx(1000);
		h_curr.GetXaxis().SetRangeUser(curr_P_val-3,curr_P_val+3.0)
		h_curr.Fit(my_gaus_fit,"Q","",curr_P_val-BELOW_MEAN_TO_FIT,curr_P_val+ABOVE_MEAN_TO_FIT)
		h_curr.Fit(my_gaus_fit,"QL","",curr_P_val-BELOW_MEAN_TO_FIT,curr_P_val+ABOVE_MEAN_TO_FIT)
		
		if(POLY_ORDER>=1):
			gaus_fit_amplitude = my_gaus_fit.GetParameter(0)
			gaus_fit_mean      = my_gaus_fit.GetParameter(1)
			gaus_fit_sigma     = my_gaus_fit.GetParameter(2)
			
			gaus_plus_poly_fit = TF1("gaus_plus_poly_fit","gausn+pol"+str(POLY_ORDER)+"(3)",0.001,12.)
			gaus_plus_poly_fit.SetParameter(0,gaus_fit_amplitude)
			gaus_plus_poly_fit.SetParameter(1,gaus_fit_mean)
			gaus_plus_poly_fit.SetParameter(2,gaus_fit_sigma)
			gaus_plus_poly_fit.SetParLimits(0,0,100000)
			gaus_plus_poly_fit.SetParLimits(1,curr_P_val-0.2,curr_P_val+0.1)
			gaus_plus_poly_fit.SetParLimits(2,0.005,0.4)
			
			h_curr.Fit(gaus_plus_poly_fit,"Q","",curr_P_val-BELOW_MEAN_TO_FIT,curr_P_val+ABOVE_MEAN_TO_FIT)
			h_curr.Fit(gaus_plus_poly_fit,"QL","",curr_P_val-BELOW_MEAN_TO_FIT,curr_P_val+ABOVE_MEAN_TO_FIT)
		
			twogaus_plus_poly_fit = TF1("twogaus_plus_poly_fit","gausn+gausn(3)+pol"+str(POLY_ORDER)+"(6)",0.001,12.)
			twogaus_plus_poly_fit.SetParameter(0,gaus_plus_poly_fit.GetParameter(0))
			twogaus_plus_poly_fit.SetParameter(1,gaus_plus_poly_fit.GetParameter(1))
			twogaus_plus_poly_fit.SetParameter(2,gaus_plus_poly_fit.GetParameter(2))
			twogaus_plus_poly_fit.SetParameter(6,gaus_plus_poly_fit.GetParameter(3))
			twogaus_plus_poly_fit.SetParameter(7,gaus_plus_poly_fit.GetParameter(4))
			twogaus_plus_poly_fit.SetParameter(8,gaus_plus_poly_fit.GetParameter(5))
			twogaus_plus_poly_fit.SetParLimits(0,0,100000)
			twogaus_plus_poly_fit.SetParLimits(1,curr_P_val-0.2,curr_P_val+0.1)
			twogaus_plus_poly_fit.SetParLimits(2,0.005,0.4)
			twogaus_plus_poly_fit.SetParLimits(3,0,100000)
			twogaus_plus_poly_fit.SetParLimits(4,curr_P_val-0.2,curr_P_val+0.1)
			twogaus_plus_poly_fit.SetParLimits(5,0.005,0.4)
			
			h_curr.Fit(twogaus_plus_poly_fit,"Q","",curr_P_val-BELOW_MEAN_TO_FIT,curr_P_val+ABOVE_MEAN_TO_FIT)
			h_curr.Fit(twogaus_plus_poly_fit,"QL","",curr_P_val-BELOW_MEAN_TO_FIT,curr_P_val+ABOVE_MEAN_TO_FIT)
			
		
		
		c1.SaveAs(".plots/FitE_"+str(curr_P_val)+".png")
		effic_gauscore_arr.append((my_gaus_fit.GetParameter(0)/h_curr.GetBinWidth(0))/norm_count)
		effic_gauscore_err_arr.append(my_gaus_fit.GetParError(0)/h_curr.GetBinWidth(0)/norm_count)
		# effic_anyquality_arr.append(atleast_one_shower/norm_count)
		# effic_anyquality_err_arr.append(0)
		if(SCAN_OVER_ENERGY): scan_val_arr.append(curr_P_val)
		if(SCAN_OVER_THETA):  scan_val_arr.append(curr_theta_val)
		scan_val_arr_arr_err.append(0)
		# curr_P_val+=0.05
		
	
	gr_gauscore_effic = TGraphErrors( len(file_list), scan_val_arr, effic_gauscore_arr, scan_val_arr_arr_err, effic_gauscore_err_arr)
	gr_gauscore_effic.SetMarkerStyle(15)
	gr_gauscore_effic.SetMarkerSize(1.2)
	gr_gauscore_effic.SetMarkerColor(kBlue)
	gr_gauscore_effic.SetName("gr_gauscore_effic")
	gr_gauscore_effic.SetTitle("Efficiency at 1 GeV")
	gr_gauscore_effic.GetXaxis().SetTitle("Photon #theta (degrees)")
	gr_gauscore_effic.GetYaxis().SetTitle("Efficiency")
	# gr_gauscore_effic.GetXaxis().SetRangeUser(0,12.)
	
	gr_gauscore_effic.Draw("AP")
	c1.SaveAs("GaussianCoreEfficiency.png")
	# gr_anyquality_effic.Draw("AP")
	# c1.SaveAs("AnyQualityEfficiency.png")
		
	f_out = TFile(argv[0],"RECREATE")
	f_out.cd()
	gr_gauscore_effic.Write()
	# gr_anyquality_effic.Write()
	# gr_gauscore_1show_effic.Write()
	# gr_anyquality_1show_effic.Write()
	f_out.Close()
		
	print("Done ")


if __name__ == "__main__":
   main(sys.argv[1:])