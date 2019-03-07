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
# from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *

FLATTENED_ROOT_FILE = "/home/fs7/jzarling/PhotonEffStudUtilities/AnalyzeOutput/2017_DATA_flattened.root"

COMMON_OUT_DIR = "/home/fs7/jzarling/MCPhotonEffOut/omega_fitting_out/" #This should already exist, or else!

#Template scripts to execute after replacing search strings
SHELL_TEMPLATE = "/home/fs7/jzarling/PhotonEffStudUtilities/HelpfulScripts/template/GetEfficFromFlattened_TEMPLATE.csh"
MAKE_HISTS_TEMPLATE = "/home/fs7/jzarling/PhotonEffStudUtilities/HelpfulScripts/template/MakeOmegaHists.C"
FIT_HISTS_TEMPLATE = "/home/fs7/jzarling/PhotonEffStudUtilities/HelpfulScripts/template/FitOmegaHists.C"

QSUB_SUBMIT_FLAG = True
RUN_INTERACTIVE  = False

def main(argv):
	#Usage controls from OptionParser
	parser_usage = ""
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) != 0):
		parser.print_help()
		return

	#Setup directory structure
	# create_dirs()
	
	
	curr_E_cut = 0.5
	while curr_E_cut < 1.0001:
		CURRENT_OUT_FULLPATH = COMMON_OUT_DIR+str(curr_E_cut)
		if(not os.path.exists(CURRENT_OUT_FULLPATH)):
			print "Current base output directory does not exist, creating..."
			os.makedirs(CURRENT_OUT_FULLPATH)
		os.chdir(CURRENT_OUT_FULLPATH)
		
		#First: csh script
		create_from_template(SHELL_TEMPLATE,"ExecuteROOTActions.csh",{'WORKING_DIRECTORY':CURRENT_OUT_FULLPATH},True)
		#Second: make hist script
		create_from_template(MAKE_HISTS_TEMPLATE,"MakeOmegaHists.C",{'E_MIN_TO_REPLACE':str(curr_E_cut),"FNAME_TO_REPLACE":FLATTENED_ROOT_FILE})
		#Third: fit hist script
		create_from_template(FIT_HISTS_TEMPLATE,"FitOmegaHists.C",{})
		
		if(QSUB_SUBMIT_FLAG): 
			# job_name = ""+str(i)+"_job"+str(j)
			qsub_command = "qsub ExecuteROOTActions.csh"
			shell_exec(qsub_command,"")
		
		if(RUN_INTERACTIVE): 
			# job_name = ""+str(i)+"_job"+str(j)
			command = "./ExecuteROOTActions.csh"
			shell_exec(command,"")
		
		curr_E_cut+=0.1
	
	print("Done ")

	
# def create_dirs():
	
	# curr_E_cut = 0.5
	
	# while curr_E_cut < 1.0001:
		
	
	# if(not os.path.exists(CURRENT_OUT_FULLPATH)):

#General purpose function: 
#First argument: input text file
#Second argument: output file path
#Third argument: dictionary. First entry of each pair is SEARCH_STRING_TO_REPLACE, second entry is REPLACEMENT_STRING
#Fourth argument: make executable flag
def create_from_template(template_full_path,ofile_full_path, replacements,make_executable=False):
	shell_exec("rm  -f "+ofile_full_path,"") #Remove script in directory if one existed
	with open(template_full_path) as infile, open(ofile_full_path, 'w') as outfile:
		for line in infile:
			for src, target in replacements.iteritems():
				line = line.replace(src, target)
			outfile.write(line)
	if(make_executable): shell_exec("chmod 777 "+ofile_full_path,"")
	return

def shell_exec(command, stdin_str,VERBOSE=False):
	if(stdin_str==""): 
		if(VERBOSE): print("Executing command: "+command)
		status = subprocess.call(command.split(" "))
	else:
		if(VERBOSE): print("Executing command: "+command+" < "+stdin_str)
		stdin_f = open(stdin_str)
		status = subprocess.call(command.split(" "), stdin=stdin_f )
	return

	
if __name__ == "__main__":
   main(sys.argv[1:])