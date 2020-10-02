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
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TGraphErrors, TAxis, TF1, TGraph
from ROOT import gBenchmark, gDirectory, gROOT

#My Stuff
#Required: add to PYTHONPATH environment variable, e.g.
#setenv PYTHONPATH /gpfs/home/j/z/jzarling/Karst/MyAnalyses/Python_stuff/jz_library/:$PYTHONPATH
from jz_pyroot_helper import *
from jz_pyroot_FitMacros import *

# PLOT_TAG = ""
PLOT_TAG = "G4"
# PLOT_TAG = "G3"

# SCAN_OVER_ENERGY = True
# SCAN_OVER_THETA  = False
SCAN_OVER_ENERGY = False
SCAN_OVER_THETA  = True

IS_FCAL = True # Name of folders and whether to use NFCALShowers or NBCALShowers. That's it.
USE_EDIST_FOR_FITTING = True #If true, get efficiency from gaussian core of energy. If false, use Delta_phi gaussian core instead.
ZERO_OR_ONE_SHOWERS_ONLY = False #If true, remove 2+ showers from numerator and denominator

shower_hist_name = "h_foundE_DeltaPhiCuts_dist"
# shower_hist_name = "h_foundE_1show_dist"
# shower_hist_name = "h_DeltaPhi_all"

POLY_ORDER = 2 #Set to 0 for pure gaussian
# VAL_BELOWTHROWN_TOFIT = 0.7 # Good for fitting E in FCAL. lower range of histogram fitting: E_thrown - this_val
# VAL_ABOVETHROWN_TOFIT = 0.5 # Good for fitting E in FCAL. upper range of histogram fitting: E_thrown + this_val
VAL_BELOWTHROWN_TOFIT = 0.5 # Good for fitting deltaPhi (comparing theta). lower range of histogram fitting: E_thrown - this_val
VAL_ABOVETHROWN_TOFIT = 0.3 # Good for fitting deltaPhi (comparing theta). upper range of histogram fitting: E_thrown + this_val
# VAL_BELOWTHROWN_TOFIT = 3 # Good for fitting deltaPhi (scanning E). lower range of histogram fitting: E_thrown - this_val
# VAL_ABOVETHROWN_TOFIT = 3 # Good for fitting deltaPhi (scanning E). upper range of histogram fitting: E_thrown + this_val
TWO_GAUS = True #Not implemented properly yet

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
		
		
	gStyle.SetOptFit(11111) #Show fit results in panel
	c1 = TCanvas("c1","c1",1600,900)
	
	BASE_FOLDER_STRING = ""
	if(IS_FCAL is True): BASE_FOLDER_STRING="FCAL_photon_gun_hists"
	else: BASE_FOLDER_STRING="BCAL_photon_gun_hists"
	
	file_list = []
	curr_E_val = 0.1
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
	
	avg_reconE_arr = array('d',[])
	
	# curr_E_val = 0.1
	for i in range(0,len(file_list)):
		print "Current file: " + argv[i+1]
		
		
		#For normalization
		if IS_FCAL: h_curr_showers = file_list[i].Get(BASE_FOLDER_STRING+"/h_NFCALShowers")
		else: h_curr_showers = file_list[i].Get(BASE_FOLDER_STRING+"/h_NBCALShowers")
		h_ThrownPhotonE_curr = TH1F()
		h_ThrownPhotonTheta_curr = TH1F()
		h_ThrownPhotonE_curr = file_list[i].Get(BASE_FOLDER_STRING+"/h_ThrownPhotonE")
		h_ThrownPhotonTheta_curr  = file_list[i].Get(BASE_FOLDER_STRING+"/h_thrownTheta")
		norm_count = h_ThrownPhotonE_curr.GetEntries()
		print "Denominator: " + str(norm_count)
		
		# print "Norm count is: " + str(norm_count)
		for j in range(0,h_curr_showers.GetNbinsX()):
			if(h_curr_showers.GetBinCenter(j) > 1 and h_curr_showers.GetBinContent(j)>0):
				# print "reducing norm count by " + str(h_curr_showers.GetBinContent(j))
				if ZERO_OR_ONE_SHOWERS_ONLY: norm_count-=h_curr_showers.GetBinContent(j)
		# print "Now norm count is: " + str(norm_count)
		
		if(norm_count < MIN_EVENTS): continue
		atleast_one_shower = norm_count-h_curr_showers.GetBinContent(1) #Bin 1 corresponds to 0 showers
		# curr_E_val = my_gaus_fit.GetParameter(1)
		# curr_E_val = h_ThrownPhotonE_curr.GetBinLowEdge( h_ThrownPhotonE_curr.GetMaximumBin()+1 )
		curr_E_val = h_ThrownPhotonE_curr.GetMean()
		curr_theta_val = h_ThrownPhotonTheta_curr.GetMean()
		print "Current theta: " + str(curr_theta_val)
		
		# h_curr = file_list[i].Get("h_foundE_all_dist")
		h_curr = file_list[i].Get(BASE_FOLDER_STRING+"/"+shower_hist_name)
		#Fit once with pure gaussian
		fit_center = curr_E_val #Fiting around reconstructed energy.
		if(not USE_EDIST_FOR_FITTING): fit_center = 0 #Fitting DeltaPhi instead, should be centered around 0.
		my_gaus_fit = TF1("my_gaus_fit","gausn+gausn(3)",0.001,3.)
		my_gaus_fit.SetParLimits(0,0,10000)
		my_gaus_fit.SetParLimits(1,fit_center-0.2,fit_center+0.1)
		my_gaus_fit.SetParLimits(2,0.005,0.4)
		my_gaus_fit.SetParLimits(0+3,0,10000)
		my_gaus_fit.SetParLimits(1+3,fit_center-0.2,fit_center+0.1)
		my_gaus_fit.SetParLimits(2+3,0.005,0.4)
		my_gaus_fit.SetNpx(1000);
		if(curr_E_val < 0.26 and not IS_FCAL): my_gaus_fit.FixParameter(3,0) # One gaussian only
		h_curr.GetXaxis().SetRangeUser(fit_center-VAL_BELOWTHROWN_TOFIT*2,fit_center+VAL_ABOVETHROWN_TOFIT*2)
		h_curr.Fit(my_gaus_fit,"Q","",fit_center-VAL_BELOWTHROWN_TOFIT,fit_center+VAL_ABOVETHROWN_TOFIT)
		h_curr.Fit(my_gaus_fit,"QL","",fit_center-VAL_BELOWTHROWN_TOFIT,fit_center+VAL_ABOVETHROWN_TOFIT)
		
		
		if(POLY_ORDER>=1):
			gaus_fit_amplitude = my_gaus_fit.GetParameter(0)
			gaus_fit_mean	  = my_gaus_fit.GetParameter(1)
			gaus_fit_sigma	 = my_gaus_fit.GetParameter(2)
			gaus_fit_amplitude2 = my_gaus_fit.GetParameter(0+3)
			gaus_fit_mean2	  = my_gaus_fit.GetParameter(1+3)
			gaus_fit_sigma2	 = my_gaus_fit.GetParameter(2+3)
						
			gaus_plus_poly_fit = TF1("gaus_plus_poly_fit","gausn+gausn(3)+pol"+str(POLY_ORDER)+"(6)",0.001,3.)
			gaus_plus_poly_fit.SetParameter(0,gaus_fit_amplitude)
			gaus_plus_poly_fit.SetParameter(1,gaus_fit_mean)
			gaus_plus_poly_fit.SetParameter(2,gaus_fit_sigma)
			gaus_plus_poly_fit.SetParameter(0+3,gaus_fit_amplitude2)
			gaus_plus_poly_fit.SetParameter(1+3,gaus_fit_mean2)
			gaus_plus_poly_fit.SetParameter(2+3,gaus_fit_sigma2)
			gaus_plus_poly_fit.SetParLimits(0,0,10000)
			gaus_plus_poly_fit.SetParLimits(1,fit_center-0.2,fit_center+0.1)
			gaus_plus_poly_fit.SetParLimits(2,0.005,0.7)
			gaus_plus_poly_fit.SetParNames("YieldUnnorm","Mean","#sigma","YieldUnnorm2","Mean2","#sigma2","pol0","pol1","pol2","pol3")
			
			# Need different fitting routine for low energy BCAL showers
			if(curr_E_val < 0.26 and not IS_FCAL):
				my_gaus_fit.FixParameter(3,0) # One gaussian only
				for i in range(0,POLY_ORDER+1):
					my_gaus_fit.FixParameter(6+i,0)
			
			h_curr.Fit(gaus_plus_poly_fit,"Q","",fit_center-VAL_BELOWTHROWN_TOFIT,fit_center+VAL_ABOVETHROWN_TOFIT)
			h_curr.Fit(gaus_plus_poly_fit,"QL","",fit_center-VAL_BELOWTHROWN_TOFIT,fit_center+VAL_ABOVETHROWN_TOFIT)
		
		
		if SCAN_OVER_ENERGY: c1.SaveAs(".plots/FitE_"+str(curr_E_val)+"_"+PLOT_TAG+".png")
		if SCAN_OVER_THETA: c1.SaveAs(".plots/FitTheta_"+str(curr_theta_val)+"_"+PLOT_TAG+".png")
		if(curr_E_val> 0.28 or IS_FCAL):
			effic_gauscore_arr.append(((my_gaus_fit.GetParameter(0)+my_gaus_fit.GetParameter(3))/h_curr.GetBinWidth(0))/norm_count)
			effic_gauscore_err_arr.append( sqrt((my_gaus_fit.GetParameter(0)+my_gaus_fit.GetParameter(3))//h_curr.GetBinWidth(0))/norm_count)
		if(curr_E_val<0.28 and not IS_FCAL):
			effic_gauscore_arr.append(h_curr.GetEntries()/norm_count)
			effic_gauscore_err_arr.append(  sqrt(h_curr.GetEntries())/norm_count)
		
		avg_reconE_arr.append(h_curr.GetMean())
		
		effic_anyquality_arr.append(atleast_one_shower/norm_count)
		effic_anyquality_err_arr.append(0)
		if(SCAN_OVER_ENERGY): scan_val_arr.append(curr_E_val)
		if(SCAN_OVER_THETA):  scan_val_arr.append(curr_theta_val)
		scan_val_arr_arr_err.append(0)
		# curr_E_val+=0.05
		
	# curr_E_val = 0.1
	# for i in range(0,len(file_list)):
		# print "Current file: " + argv[i]
		# h_curr = file_list[i].Get("h_foundE_1orless_all_dist")
		# my_gaus_fit = TF1("my_gaus_fit","gausn",0.001,3.)
		# my_gaus_fit.SetParLimits(0,0,100)
		# my_gaus_fit.SetNpx(1000);
		# h_curr.GetXaxis().SetRangeUser(0.01,3.5)
		# h_curr.Fit(my_gaus_fit,"Q")
		
		# curr_E_val = my_gaus_fit.GetParameter(1)
		
		# c1.SaveAs(".plots/FitE_1show_"+str(curr_E_val)+".png")
		# effic_gauscore_1show_arr.append((my_gaus_fit.GetParameter(0)/h_curr.GetBinWidth(0))/10000)
		# effic_gauscore_1show_err_arr.append(my_gaus_fit.GetParError(0)/h_curr.GetBinWidth(0)/10000)
		# effic_anyquality_1show_arr.append((10000.-h_curr.GetBinContent(1))/10000)
		# effic_anyquality_1show_err_arr.append(0)
	
	for i in range(0,len(scan_val_arr)):
		print "Value: " + str(scan_val_arr[i])
		print "Efficiency: " + str(effic_gauscore_arr[i])
		print ""
	
	gr_gauscore_effic = TGraphErrors( len(file_list), scan_val_arr, effic_gauscore_arr, scan_val_arr_arr_err, effic_gauscore_err_arr)
	gr_gauscore_effic.SetMarkerStyle(15)
	gr_gauscore_effic.SetMarkerSize(1.2)
	gr_gauscore_effic.SetMarkerColor(kBlue)
	gr_gauscore_effic.SetName("gr_gauscore_effic")
	gr_gauscore_effic.SetTitle("Efficiency at 1 GeV")
	if(SCAN_OVER_ENERGY): gr_gauscore_effic.GetXaxis().SetTitle("E_{#gamma} (GeV)")
	if(SCAN_OVER_THETA): gr_gauscore_effic.GetXaxis().SetTitle("Photon #theta (degrees)")
	gr_gauscore_effic.GetYaxis().SetTitle("Efficiency")
	# gr_gauscore_effic.GetXaxis().SetRangeUser(0,12.)
	
	gr_anyquality_effic = TGraphErrors( len(file_list), scan_val_arr, effic_anyquality_arr, scan_val_arr_arr_err, effic_anyquality_err_arr)
	gr_anyquality_effic.SetName("gr_anyquality_effic")
	gr_anyquality_effic.SetMarkerStyle(15)
	gr_anyquality_effic.SetMarkerSize(1.2)
	gr_anyquality_effic.SetMarkerColor(kBlue)
		
	# gr_gauscore_1show_effic = TGraphErrors( len(file_list), E_arr, effic_gauscore_1show_arr, E_arr_err, effic_gauscore_1show_err_arr)
	# gr_gauscore_1show_effic.SetMarkerStyle(15)
	# gr_gauscore_1show_effic.SetMarkerSize(1.2)
	# gr_gauscore_1show_effic.SetMarkerColor(kBlue)
	# gr_gauscore_1show_effic.SetName("gr_gauscore_1show_effic")
	# gr_anyquality_1show_effic = TGraphErrors( len(file_list), E_arr, effic_anyquality_1show_arr, E_arr_err, effic_anyquality_1show_err_arr)
	# gr_anyquality_1show_effic.SetName("gr_anyquality_1show_effic")
	# gr_anyquality_1show_effic.SetMarkerStyle(15)
	# gr_anyquality_1show_effic.SetMarkerSize(1.2)
	# gr_anyquality_1show_effic.SetMarkerColor(kBlue)
		
	gr_gauscore_effic.Draw("AP")
	c1.SaveAs("GaussianCoreEfficiency.png")
	gr_anyquality_effic.Draw("AP")
	c1.SaveAs("AnyQualityEfficiency.png")
		
	gr_avg_reconE = TGraph( len(file_list), scan_val_arr, avg_reconE_arr)
	gr_avg_reconE.SetName("gr_avg_reconE")
		
		
	f_out = TFile(argv[0],"RECREATE")
	f_out.cd()
	gr_gauscore_effic.Write()
	gr_anyquality_effic.Write()
	gr_avg_reconE.Write()
	# gr_gauscore_1show_effic.Write()
	# gr_anyquality_1show_effic.Write()
	f_out.Close()
		
	print("Done ")


if __name__ == "__main__":
   main(sys.argv[1:])