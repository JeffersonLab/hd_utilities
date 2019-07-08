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
import re


#Root stuff
from ROOT import TFile, TTree, TBranch, TLorentzVector, TLorentzRotation, TVector3
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TGraphErrors
from ROOT import gBenchmark, gDirectory, gROOT, gPad

#My Stuff
#Required: add to PYTHONPATH environment variable, e.g.
#setenv PYTHONPATH /gpfs/home/j/z/jzarling/Karst/MyAnalyses/Python_stuff/jz_library/:$PYTHONPATH
# from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *

# REL_DIR = "photon_pipiproton_MATCHINGOFF/"
# REL_DIR = "photon_pipiprotonREACTIONFILTER_out/"
# SAVE_PNG = False


#For gun and pi+pi-p 
WITH_GAMMA_TREENAME = "gamma_pi+pi-p_nofit_Tree"
NORM_TREENAME = "pi+pi-p_nofit_Tree"
# NORM_BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_pipiP/"
NORM_BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_pipiP_match_off/"
NORM_FILENAME = "tree_pi+pi-p_nofit.root"

#For gun and p 
# WITH_GAMMA_TREENAME = "gamma_p_nofit_Tree"
# NORM_TREENAME = "gamma_p_nofit_Tree"
# NORM_BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_P/"
# NORM_FILENAME = "tree_p_anything_4norm.root"

#Colors
kRed = 632
kOrange = 800
kYellow = 400
kGreen = 416
kBlue = 600
kViolet = 880
kMagenta = 616
kBlack = 1




POLY_ORDER = 2 #Set to 0 for pure gaussian
E_BELOWTHROWN_TOFIT = 0.7 # lower range of histogram fitting: E_thrown - this_val
E_ABOVETHROWN_TOFIT = 0.5 # upper range of histogram fitting: E_thrown + this_val

DELTA_THETA_CUT = "0.35" #This should probably match whatever is used in photon_gun_hists plugin
DELTA_PHI_CUT   = "3.35" #This should probably match whatever is used in photon_gun_hists plugin

MIN_EVENTS = 10

MAKE_HISTS = True #False: already made. True: make here.
NORMALIZE_EXTERNAL = False #True: use different set of files to normalize (embedded photon gun). False: use same file for nomalization (omega->3pi)
WITH_GAMMA_TREENAME = "pi0pippim__pi0_gmissg__B2_F1_T0_M7_Tree"


#Fitting stuff
POLY_ORDER = 2 #Set to 0 for pure gaussian
E_BELOWTHROWN_TOFIT = 0.7 # lower range of histogram fitting: E_thrown - this_val
E_ABOVETHROWN_TOFIT = 0.5 # upper range of histogram fitting: E_thrown + this_val


def main(argv):
	#Usage controls from OptionParser
	parser_usage = "infile.root min_E max_E E_step hist_outfile.root effic_outfile.root"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) != 6):
		parser.print_help()
		return

	infile_with_gamma = argv[0]
	
	MIN_E = float(argv[1])
	MAX_E = float(argv[2])
	E_STEP = float(argv[3])
	hist_out_name = (argv[4])
	effic_out_name = (argv[5])
		
	scale_factor = 2.66
	if(MAKE_HISTS): scale_factor =  MakeHistsToFit(infile_with_gamma,MIN_E,MAX_E,E_STEP,hist_out_name)
	FitHistsGetEffic(hist_out_name,effic_out_name,MIN_E,MAX_E,E_STEP,scale_factor)
		
	print("Done ")

	
def FitHistsGetEffic(hist_fname,fit_out_fname,MIN_E,MAX_E,E_STEP,scale_factor):

	E_arr = array('d',[])
	E_arr_err = array('d',[])
	effic_arr = array('d',[])
	effic_arr_err = array('d',[])
	norm_arr = array('d',[])

	curr_E_val = MIN_E
	while curr_E_val <= MAX_E+0.0001: 
		E_arr.append(curr_E_val)
		if(NORMALIZE_EXTERNAL): E_arr_err.append(0)
		if(not NORMALIZE_EXTERNAL): E_arr_err.append(E_STEP/2.)
		curr_E_val+=E_STEP		

	c1 = TCanvas("c1","c1",1600,900)
	c1.SetGridx(1)
	c1.SetGridy(1)

	
	#Get normalization from external files assuming matching directory scheme for embedded photon gun data
	if(NORMALIZE_EXTERNAL):
		#Get normalization
		curr_E_val = MIN_E
		while curr_E_val <= MAX_E+0.0001:
		
		
			print "Getting normalization for energy value: " + str(curr_E_val)
		
			thisfile_string = NORM_BASE_DIR+str(curr_E_val)+"/"+NORM_FILENAME
			
			if(not os.path.isfile(thisfile_string) ):
				print "file not found, normalization will be 0: " + thisfile_string
				norm_arr.append(0.)
				curr_E_val+=E_STEP		
				continue
		
			normfile = TFile.Open(thisfile_string,'read')
		
			my_tr = TTree()
			my_tr.Print()
			my_tr = normfile.Get(NORM_TREENAME)
			norm_arr.append( my_tr.GetEntries() )
			curr_E_val+=E_STEP		
		#Done getting normalization
	
	
	# Get normalization from same root file
	f_in = TFile.Open(hist_fname) #Open rootfile
	if(not NORMALIZE_EXTERNAL): 
		for i in range(0,len(E_arr)):
			E_cut_str = str(E_arr[i]-E_STEP/2.)+"<ThrownE&&ThrownE<"+str(E_arr[i]+E_STEP/2.)+"&&ThrownTheta<9&&ThrownTheta>4"
			h_curr = f_in.Get("h_EThrown_"+E_cut_str)
			norm_arr.append( h_curr.GetEntries()/scale_factor )
		
		
	#Fit stuff
	for i in range(0,len(E_arr)):
		h_curr = TH1F()
		E_cut_str = str(E_arr[i]-E_STEP/2.)+"<ThrownE&&ThrownE<"+str(E_arr[i]+E_STEP/2.)+"&&ThrownTheta<9&&ThrownTheta>4"
		if(NORMALIZE_EXTERNAL): h_curr = f_in.Get("h_EGammaPostCuts_"+str(E_arr[i]))
		if(not NORMALIZE_EXTERNAL): h_curr = f_in.Get("h_EGammaPostCuts_"+E_cut_str)
		curr_E_val = E_arr[i]
		
		norm_count = norm_arr[i]
		if(norm_count < MIN_EVENTS): 
			effic_arr.append(-1)
			effic_arr_err.append(0)
			continue
		
		
		my_gaus_fit = TF1("my_gaus_fit","gausn",0.001,3.)
		my_gaus_fit.SetParLimits(0,0,10000)
		my_gaus_fit.SetParLimits(1,curr_E_val-0.2,curr_E_val+0.1)
		my_gaus_fit.SetParLimits(2,0.005,0.4)
		my_gaus_fit.SetNpx(1000);		
		h_curr.GetXaxis().SetRangeUser(curr_E_val-1.2,curr_E_val+0.5)
		h_curr.Fit(my_gaus_fit,"Q","",curr_E_val-E_BELOWTHROWN_TOFIT,curr_E_val+E_ABOVETHROWN_TOFIT)
		h_curr.Fit(my_gaus_fit,"QL","",curr_E_val-E_BELOWTHROWN_TOFIT,curr_E_val+E_ABOVETHROWN_TOFIT)
	
		if(POLY_ORDER>=1):
			gaus_fit_amplitude = my_gaus_fit.GetParameter(0)
			gaus_fit_mean      = my_gaus_fit.GetParameter(1)
			gaus_fit_sigma     = my_gaus_fit.GetParameter(2)
			
			gaus_plus_poly_fit = TF1("gaus_plus_poly_fit","gausn+pol"+str(POLY_ORDER)+"(3)",0.001,3.)
			gaus_plus_poly_fit.SetParameter(0,gaus_fit_amplitude)
			gaus_plus_poly_fit.SetParameter(1,gaus_fit_mean)
			gaus_plus_poly_fit.SetParameter(2,gaus_fit_sigma)
			gaus_plus_poly_fit.SetParLimits(0,0,10000)
			gaus_plus_poly_fit.SetParLimits(1,curr_E_val-0.2,curr_E_val+0.1)
			gaus_plus_poly_fit.SetParLimits(2,0.005,0.4)
			
			h_curr.Fit(gaus_plus_poly_fit,"Q","",curr_E_val-E_BELOWTHROWN_TOFIT,curr_E_val+E_ABOVETHROWN_TOFIT)
			h_curr.Fit(gaus_plus_poly_fit,"QL","",curr_E_val-E_BELOWTHROWN_TOFIT,curr_E_val+E_ABOVETHROWN_TOFIT)
	
		c1.SaveAs(".plots/FitE_"+str(curr_E_val)+".png")
		effic_arr.append((my_gaus_fit.GetParameter(0)/h_curr.GetBinWidth(0))/norm_count)
		effic_arr_err.append(my_gaus_fit.GetParError(0)/h_curr.GetBinWidth(0)/norm_count)
	
	gr_gauscore_effic = TGraphErrors( len(E_arr), E_arr, effic_arr, E_arr_err, effic_arr_err)
	gr_gauscore_effic.SetMarkerStyle(15)
	gr_gauscore_effic.SetMarkerSize(1.2)
	gr_gauscore_effic.SetMarkerColor(kBlue)
	gr_gauscore_effic.SetName("gr_gauscore_effic")
	
	f_out = TFile(fit_out_fname,"RECREATE")
	f_out.cd()
	gr_gauscore_effic.Write()
	f_out.Close()
	
	
def MakeHistsToFit(file,MIN_E,MAX_E,E_STEP,f_out_str):

	E_arr = array('d',[])
	
	c1 = TCanvas("c1","c1",1600,900)
	c1.SetGridx(1)
	c1.SetGridy(1)

	norm_arr = array('d',[])
	curr_E_val = MIN_E 
	while curr_E_val <= MAX_E+0.0001: 
		E_arr.append(curr_E_val)
		curr_E_val+=E_STEP		
		
	
		
		
	#Create and fit histograms in larger file
	f = TFile.Open(file,'read')
	my_tr = TTree()
	my_tr =  f.Get(WITH_GAMMA_TREENAME)
	h_DeltaPhi_list = [] #List of deltaPhi histograms to save
	h_DeltaTheta_list = [] #List of deltaTheta histograms to save
	h_EGammaPreCuts_list = [] #List of E_gamma histograms to save
	h_EGammaPostCuts_list = [] #List of E_gamma histograms to save
	h_EThrown_list = [] #List of E_gamma histograms to save
	
	num_tree_entries   = float(my_tr.GetEntries())
	num_branch_entries = my_tr.Draw("ThrownE","")
	norm_scale_factor = num_branch_entries/num_tree_entries
	
	# print "Num tree entries: " + str(num_tree_entries)
	# print "Num branch entries: " + str(num_branch_entries)
	
	print "Need to scale by: " + str(norm_scale_factor)
	
	for i in range(0,len(E_arr)):
	
		E_cut_str = ""
		if(NORMALIZE_EXTERNAL): E_cut_str = "abs(ThrownE-"+str(E_arr[i])+")<0.0001"
		if(not NORMALIZE_EXTERNAL): E_cut_str = str(E_arr[i]-E_STEP/2.)+"<ThrownE&&ThrownE<"+str(E_arr[i]+E_STEP/2.)+"&&ThrownTheta<9&&ThrownTheta>4"
		print "Cut string: " + E_cut_str
	
		#Make DeltaPhi and DeltaTheta histograms
		h_DeltaPhi_curr = TH1F()
		my_tr.Draw("DeltaPhi>>h_DeltaPhi_curr(1000,-180.,180.)",E_cut_str)
		h_DeltaPhi_curr = gPad.GetPrimitive("h_DeltaPhi_curr")
		if(NORMALIZE_EXTERNAL): h_DeltaPhi_curr.SetNameTitle("h_DeltaPhi_"+str(E_arr[i]),"h_DeltaPhi_"+str(E_arr[i]))
		if(not NORMALIZE_EXTERNAL): h_DeltaPhi_curr.SetNameTitle("h_DeltaPhi_"+E_cut_str,"h_DeltaPhi_"+E_cut_str)
		h_DeltaPhi_list.append(h_DeltaPhi_curr)
		print "h_DeltaPhi_curr hist entries:" + str(h_DeltaPhi_curr.GetEntries())
		
		h_DeltaTheta_curr = TH1F()
		my_tr.Draw("DeltaTheta>>h_DeltaTheta_curr(1000,-25.,10.)",E_cut_str)
		h_DeltaTheta_curr = gPad.GetPrimitive("h_DeltaTheta_curr")
		if(NORMALIZE_EXTERNAL): h_DeltaTheta_curr.SetNameTitle("h_DeltaTheta_"+str(E_arr[i]),"h_DeltaTheta_"+str(E_arr[i]))
		if(not NORMALIZE_EXTERNAL): h_DeltaTheta_curr.SetNameTitle("h_DeltaTheta_"+E_cut_str,"h_DeltaTheta_"+E_cut_str)
		h_DeltaTheta_list.append(h_DeltaTheta_curr)
		print "h_DeltaTheta_curr hist entries:" + str(h_DeltaTheta_curr.GetEntries())
	
		h_EGammaPreCuts_curr = TH1F()
		my_tr.Draw("FoundE>>h_EGammaPreCuts_curr(1000,0,"+str(MAX_E+1)+")",E_cut_str)
		h_EGammaPreCuts_curr = gPad.GetPrimitive("h_EGammaPreCuts_curr")
		if(NORMALIZE_EXTERNAL): h_EGammaPreCuts_curr.SetNameTitle("h_EGammaPreCuts_"+str(E_arr[i]),"h_EGammaPreCuts_"+str(E_arr[i]))
		if(not NORMALIZE_EXTERNAL): h_EGammaPreCuts_curr.SetNameTitle("h_EGammaPreCuts_"+E_cut_str,"h_EGammaPreCuts_"+E_cut_str)
		h_EGammaPreCuts_list.append(h_EGammaPreCuts_curr)
		print "h_EGammaPreCuts_curr hist entries:" + str(h_EGammaPreCuts_curr.GetEntries())
		
		h_EGammaPostCuts_curr = TH1F()
		my_tr.Draw("FoundE>>h_EGammaPostCuts_curr(1000,0,"+str(MAX_E+1)+")","abs(DeltaPhi)<"+DELTA_PHI_CUT+"&&abs(DeltaTheta)<"+DELTA_THETA_CUT+"&&"+E_cut_str)
		h_EGammaPostCuts_curr = gPad.GetPrimitive("h_EGammaPostCuts_curr")
		if(NORMALIZE_EXTERNAL): h_EGammaPostCuts_curr.SetNameTitle("h_EGammaPostCuts_"+str(E_arr[i]),"h_EGammaPostCuts_"+str(E_arr[i]))
		if(not NORMALIZE_EXTERNAL): h_EGammaPostCuts_curr.SetNameTitle("h_EGammaPostCuts_"+E_cut_str,"h_EGammaPostCuts_"+E_cut_str)
		h_EGammaPostCuts_curr.SetTitle("E_{#gamma} thrown = "+str(E_arr[i]) )
		h_EGammaPostCuts_curr.GetXaxis().SetTitle("E_{#gamma}")
		h_EGammaPostCuts_curr.GetYaxis().SetTitle("Yield")
		h_EGammaPostCuts_list.append(h_EGammaPostCuts_curr)
		print "h_EGammaPostCuts_curr hist entries:" + str(h_EGammaPostCuts_curr.GetEntries())
		
		h_EThrown_curr = TH1F()
		# my_tr.Draw("ThrownE>>h_EThrown_curr(1000,0,10",E_cut_str)
		my_tr.Draw("MCWeight>>h_EThrown_curr(1000,-10,10",E_cut_str)
		h_EThrown_curr = gPad.GetPrimitive("h_EThrown_curr")
		if(NORMALIZE_EXTERNAL): h_EThrown_curr.SetNameTitle("h_EThrown_"+str(E_arr[i]),"h_EThrown_"+str(E_arr[i]))
		if(not NORMALIZE_EXTERNAL): h_EThrown_curr.SetNameTitle("h_EThrown_"+E_cut_str,"h_EThrown_"+E_cut_str)
		h_EThrown_curr.SetTitle("E_{#gamma} thrown = "+str(E_arr[i]) )
		h_EThrown_curr.GetXaxis().SetTitle("E_{#gamma}")
		h_EThrown_curr.GetYaxis().SetTitle("Yield")
		h_EThrown_list.append(h_EThrown_curr)
		print "h_EThrown_curr hist entries:" + str(h_EThrown_curr.GetEntries())
	
	#Save to file
	f_out = TFile(f_out_str,"RECREATE")
	# gr_gauscore_effic.Write()
	
	for i in range(0,len(E_arr)): h_EGammaPostCuts_list[i].Write()
	for i in range(0,len(E_arr)): h_DeltaPhi_list[i].Write()
	for i in range(0,len(E_arr)): h_DeltaTheta_list[i].Write()
	for i in range(0,len(E_arr)): h_EGammaPreCuts_list[i].Write()
	for i in range(0,len(E_arr)): h_EThrown_list[i].Write()
	f_out.Close()	
	
	return norm_scale_factor
	
	
def MakeHistsToFit2(file,f_out_str):

	E_arr = array('d',[])
	E_arr_err = array('d',[])
	effic_arr = array('d',[])
	effic_arr_err = array('d',[])

	c1 = TCanvas("c1","c1",1600,900)
	c1.SetGridx(1)
	c1.SetGridy(1)

	
	#Find which energy files exist, create file list and energy array
	f_wogamma_list = []
	curr_E_val = 0.1
	while curr_E_val <= 3.0:
		normfile = REL_DIR+"tree_pi+pi-p_nofit_"+str(curr_E_val)+".root"
		if(os.path.isfile(normfile)): 
			E_arr.append(curr_E_val)
			E_arr_err.append(0)
			f_wogamma_list.append(TFile.Open(normfile,'read'))
			# print "File found!!!"
			# print "Was looking for file: " + normfile
			# print "Current E val: " + str(curr_E_val)
		# else:
			# print "File not found!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
			# print "Was looking for file: " + normfile
			# print "Current E val: " + str(curr_E_val)
		curr_E_val+=0.05

	#Get normalization factor
	norm_arr = array('d',[])
	for i in range(0,len(f_wogamma_list)):
		my_tr = TTree()
		my_tr =  f_wogamma_list[i].Get("pi+pi-p_nofit_Tree")
		norm_arr.append(my_tr.GetEntries())
		# print "Energy: "  + str(E_arr[i])
		# print "Normalization: "  + str(norm_arr[i])

	#Create and fit histograms in larger file
	f = TFile.Open(file,'read')
	my_tr = TTree()
	my_tr =  f.Get("gamma_pi+pi-p_nofit_Tree")
	h_DeltaPhi_list = []
	h_DeltaTheta_list = []
	h_EGammaPostCuts_list = []
	for i in range(0,len(E_arr)):
		h_DeltaPhi_curr = TH1F()
		my_tr.Draw("DeltaPhi>>h_DeltaPhi_curr(1000,-180.,180.)","")
		h_DeltaPhi_curr = gPad.GetPrimitive("h_DeltaPhi_curr")
		h_DeltaPhi_curr.SetName("h_DeltaPhi_"+str(E_arr[i]))
		h_DeltaPhi_list.append(h_DeltaPhi_curr)
		h_DeltaTheta_curr = TH1F()
		my_tr.Draw("DeltaTheta>>h_DeltaTheta_curr(1000,-25.,10.)","")
		h_DeltaTheta_curr = gPad.GetPrimitive("h_DeltaTheta_curr")
		h_DeltaTheta_curr.SetName("h_DeltaTheta_"+str(E_arr[i]))
		h_DeltaTheta_list.append(h_DeltaTheta_curr)
		
		h_EGammaPostCuts_curr = TH1F()
		E_cut_str = "abs(ThrownE-"+str(E_arr[i])+")<0.0001"
		print "Cut string: " + E_cut_str
		my_tr.Draw("FoundE>>h_EGammaPostCuts_curr(1000,0,4)","abs(DeltaPhi)<2.&&abs(DeltaTheta)<0.5&&"+E_cut_str)
		h_EGammaPostCuts_curr = gPad.GetPrimitive("h_EGammaPostCuts_curr")
		h_EGammaPostCuts_curr.SetName("h_EGammaPostCuts_"+str(E_arr[i]))
		h_EGammaPostCuts_list.append(h_EGammaPostCuts_curr)
	
		my_gaus_fit = TF1("my_gaus_fit","gausn")
		my_gaus_fit.SetParLimits(0,0,30000)
		my_gaus_fit.SetParameter(1,E_arr[i])
		my_gaus_fit.SetParLimits(1,E_arr[i]-0.5,E_arr[i]+0.1)
		my_gaus_fit.SetParLimits(2,0.02,0.5)
		my_gaus_fit.SetNpx(1000);
		h_EGammaPostCuts_curr.Fit(my_gaus_fit,"Q","",E_arr[i]-0.3,E_arr[i]+0.2)
		c1.SaveAs(".plots/ReactionFilterFit_pipip_"+str(E_arr[i])+".png")
		effic = my_gaus_fit.GetParameter(0)/h_EGammaPostCuts_curr.GetBinWidth(0)/norm_arr[i]
		effic_err = my_gaus_fit.GetParError(0)/h_EGammaPostCuts_curr.GetBinWidth(0)/norm_arr[i]
		
		if(E_arr[i]<0.2):
			effic = h_EGammaPostCuts_curr.GetEntries()/norm_arr[i]
			effic_err = sqrt(h_EGammaPostCuts_curr.GetEntries())/norm_arr[i]
		if(h_EGammaPostCuts_curr.GetEntries()<100.):
			effic = -1
			effic_err = 0
	
	
		print "File Entries: "  + str( my_tr.GetEntries())
		print "Histogram Entries: "  + str( h_EGammaPostCuts_curr.GetEntries())
		print "Gaussian yield: "  + str( my_gaus_fit.GetParameter(0)/h_EGammaPostCuts_curr.GetBinWidth(0))
		print "Normalization: "  + str( norm_arr[i])
		print "Efficiency: "  + str( effic)
		
		effic_arr.append(effic)
		effic_arr_err.append(effic_err)
	
	gr_gauscore_effic = TGraphErrors( len(E_arr), E_arr, effic_arr, E_arr_err, effic_arr_err)
	gr_gauscore_effic.SetName("gr_gauscore_effic")
	
	f_out = TFile(f_out_str,"RECREATE")
	gr_gauscore_effic.Write()
	for i in range(0,len(E_arr)): h_DeltaPhi_list[i].Write()
	for i in range(0,len(E_arr)): h_DeltaTheta_list[i].Write()
	for i in range(0,len(E_arr)): h_EGammaPostCuts_list[i].Write()
	f_out.Close()	
	
	# curr_E_val = 0.1
	# curr_E_val = 2.0
	# while curr_E_val <= 3.:
	# while curr_E_val <= 2.2:
		# print "Curr E thing " + str(curr_E_val)
		
		# normfile = REL_DIR+"tree_pi+pi-p_nofit_"+str(curr_E_val)+".root"
		# my_tr2 = TTree()
		# my_tr2 =  f_wogamma_list[i].Get("pi+pi-p_nofit_Tree")
		# NormalizationFactor = my_tr2.GetEntries()
		
		
		# curr_E_val+=0.05
		
	
	return
		

if __name__ == "__main__":
   main(sys.argv[1:])