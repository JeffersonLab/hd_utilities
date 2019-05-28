#!/usr/bin/env python

#PyRoot file created from template located at:
#/gpfs/home/j/z/jzarling/Karst/bin/source/PyRoot_template_file.py

from optparse import OptionParser
import os.path
import os
import sys, stat
import subprocess
import glob
from array import array
from math import sqrt, exp
import shutil
from shutil import copyfile

#Root stuff
from ROOT import TFile, TTree, TBranch, TLorentzVector, TLorentzRotation, TVector3
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TRandom3, TGraphErrors
from ROOT import gBenchmark, gDirectory, gROOT

#My Stuff
#Required: add to PYTHONPATH environment variable, e.g.
#setenv PYTHONPATH /gpfs/home/j/z/jzarling/Karst/MyAnalyses/Python_stuff/jz_library/:$PYTHONPATH
# from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *

SCAN_OVER_ENERGY = False
SCAN_OVER_THETA  = True
THETA   = "6." #Used if scanning over E
E_GAMMA = "1." #Used if scanning over theta


HADRONIC_CLUSTER_VETO = True #True use standard FCAL track veto algorith (default in GlueX code). False: turn off by changing cut parameters to let everything pass cut.
# HADRONIC_CLUSTER_VETO = False #True use standard FCAL track veto algorith (default in GlueX code). False: turn off by changing cut parameters to let everything pass cut.
CCDB_LOC = "sqlite:////home/fs7/jzarling/PhotonEffStudUtilities/CustomMCgen/genPhotonGunScan/ccdb.sqlite"
RCDB_LOC = "sqlite:////home/fs7/jzarling/PhotonEffStudUtilities/CustomMCgen/genPhotonGunScan/rcdb.sqlite"
GEANT_VERSION = 4

#Specify base output directory here (or use current directory if blank)
# BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_P_matchoff"
BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_P_scantheta_G4"

def main(argv):
	#Usage controls from OptionParser
	parser_usage = "this_program.py run min_scan_val max_scan_val events_per_val(default=10,000) step(default=0.05GeV/degrees)"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) < 3 or len(args) > 6):
			parser.print_help()
			return

	RUN = argv[0]
	MIN_VAL = float(argv[1])
	MAX_VAL = float(argv[2])
	EVENTS_PER_val = "30000"
	if(len(args)>=4): EVENTS_PER_val = argv[3]
	VAL_STEP = 0.05
	if(len(args)>=5): VAL_STEP = float(argv[4])
	gen_interactive = False
	if(len(args)>=6):
		if(argv[5]=="1"): gen_interactive = True
		
	if(SCAN_OVER_ENERGY and SCAN_OVER_THETA):
		print "ERROR: both scan flags turned on! Please select either energy or theta to scan over..."
		return
	if(not SCAN_OVER_ENERGY and not SCAN_OVER_THETA):
		print "ERROR: neither scan flags turned on! Please select either energy or theta to scan over..."
		return
		
	omega_config_loc = os.getcwd()+"/Omega3Pi.conf"
	controlin_loc = os.getcwd()+"/control.in"
	
	print "Run number to generate in: " + RUN
	print "Events per point to generate: " + EVENTS_PER_val
	print "Minimum value " + str(MIN_VAL)
	print "Max value " + str(MAX_VAL)
	print "Step value " + str(VAL_STEP)
	print "Generate interactive: " + str(gen_interactive)

		
	global BASE_DIR
	if(BASE_DIR==""): BASE_DIR = os.getcwd()
	if not os.path.exists(BASE_DIR): os.makedirs(BASE_DIR)
		
		
	current_val = MIN_VAL
	while current_val < MAX_VAL:
		current_dirname = BASE_DIR+"/"+str(current_val)
		if not os.path.exists(current_dirname): os.makedirs(current_dirname)
		os.chdir(current_dirname)
		print "output directory is: " + current_dirname
		
		ascii_to_prune = "genr8_omega_"+str(current_val)+".ascii"
		ascii_pruned = "pruned_gamma_p_"+str(current_val)+".ascii"
		pruned_hddm = "pruned_gamma_p_"+str(current_val)+".hddm"
		
		MakeGenr8File(ascii_to_prune,int(EVENTS_PER_val),omega_config_loc)
		shell_exec("rm " + ascii_pruned , "") #Remove old ascii file, if any
		if(SCAN_OVER_ENERGY): PruneGenr8File(ascii_to_prune,ascii_pruned,current_val,float(THETA),False)
		if(SCAN_OVER_THETA): PruneGenr8File(ascii_to_prune,ascii_pruned,float(E_GAMMA),current_val,False)
		# SubmitQSubJob(rel_dir_name,ascii_pruned,pruned_hddm,energy)
		SubmitQSubJob(ascii_pruned,pruned_hddm,current_val,controlin_loc,int(EVENTS_PER_val),int(RUN),gen_interactive)
		
		
		
		
		shell_exec("rm " + ascii_to_prune , "")
		# shell_exec("rm pruned_gamma_p_"+str(energy)+".ascii" , "")
		
		current_val+=0.05
		#Change back to base directory
		# os.chdir("../")
		
		
		
	print("Done ")

	
#Assumes you have already changed to running/saving directory
# def SubmitQSubJob(rel_dir_name,ascii_file,hddm_name,e_gamma):
def SubmitQSubJob(ascii_file,hddm_name,e_gamma,controlin_loc,NEVENTS,run,gen_interactive):
	
	
	
	#First, run genr8_2_hddm
	shell_exec("genr8_2_hddm "+ascii_file+ " -P8.5", "")
	shell_exec("mv "+hddm_name+" geant3_in.hddm","")
	
	curr_full_dir = os.getcwd()
	
	copyfile(controlin_loc, "./control.in")
	
	#Create run.mac file read by geant4
	if(GEANT_VERSION == 4):
		shell_exec("rm run.mac","")
		with open("run.mac", "w") as myfile:
			myfile.write("/run/beamOn "+str(NEVENTS)+"\n")
			myfile.write("exit\n")
	
	with open("ReplaceAndRun.csh", "w") as myfile:
		myfile.write("#!/bin/tcsh \n")
		myfile.write("#$ -e "+curr_full_dir+"/output.err\n")
		myfile.write("#$ -o "+curr_full_dir+"/output.log\n")
		myfile.write("\n")
		myfile.write("setenv JANA_CALIB_CONTEXT \"variation=mc_sim1\"\n")
		myfile.write("setenv CCDB_CONNECTION "+CCDB_LOC+"\n")
		myfile.write("setenv JANA_CALIB_URL $CCDB_CONNECTION\n")
		myfile.write("setenv RCDB_CONNECTION "+RCDB_LOC+" \n")
		myfile.write("env \n")
		myfile.write("\n")
		myfile.write("cd " + curr_full_dir + " \n")
		myfile.write("perl -p -i -e \"s/INDUMMY/geant3_in.hddm/g\" control.in\n")
		myfile.write("perl -p -i -e \"s/OUTDUMMY/geant3_out.hddm/g\" control.in\n")
		myfile.write("perl -p -i -e \"s/TRIG 1234/TRIG "+str(NEVENTS)+"/g\" control.in\n")
		if(GEANT_VERSION == 3): myfile.write("hdgeant -xml=ccdb://GEOMETRY/main_HDDS.xml,run="+str(run)+" \n")
		if(GEANT_VERSION == 4): myfile.write("hdgeant4 -t4 run.mac \n")
		myfile.write("mcsmear -PTHREAD_TIMEOUT=300 geant3_out.hddm -ogeant3_out_smeared.hddm\n")
		if(HADRONIC_CLUSTER_VETO):   myfile.write("hd_root -PTRIG:BYPASS=1 geant3_out_smeared.hddm -PNTHREADS=6 -PPLUGINS=ReactionFilter,photon_gun_hists,danarest,monitoring_hists -PReaction1=1_14__1_14_m0 -PReaction1:Flags=F0_B8_T9 -PReaction1:Name=gamma_p_nofit -PReaction2=1_14__14_m0 -PReaction2:Flags=F0_B8_T9 -PReaction2:Name=p_anything_4norm \n")
		if(not HADRONIC_CLUSTER_VETO): myfile.write("hd_root -PFCAL:CUT_PAR1=-100 -PFCAL:CUT_PAR2=-100 -PTRIG:BYPASS=1 geant3_out_smeared.hddm -PNTHREADS=6 -PPLUGINS=ReactionFilter,photon_gun_hists,danarest,monitoring_hists -PReaction1=1_14__1_14_m0 -PReaction1:Flags=F0_B8_T9 -PReaction1:Name=gamma_p_nofit -PReaction2=1_14__14_m0 -PReaction2:Flags=F0_B8_T9 -PReaction2:Name=p_anything_4norm \n")
		# myfile.write("rm hdgeant.rz geant.hbook geant3_in.hddm geant3_out.hddm smear.root \n")
		myfile.write(" 	")
		myfile.write("\n")
	
	shell_exec("chmod +x ReplaceAndRun.csh","")
	if(not gen_interactive): shell_exec("qsub -N g_prot_gun ReplaceAndRun.csh","")
	else: shell_exec("./ReplaceAndRun.csh","")
	
	return

def PruneGenr8File(fname_in,fname_out,e_gamma_curr,theta_curr,verbose = False):
	# print "input genr8 file: " + fname_in
	# print "output file: " + fname_out
	
	counter = 1
	
	line1_out = "" #Run and event info
	line2_out = "" #Gamma ID info
	line3_out = "" #Gamma P4 info
	line4_out = "" #Proton ID info
	line5_out = "" #Proton P4 info
	
	rand = TRandom3(int(e_gamma_curr*1000))
	p4_proton = TLorentzVector()
	for line in open(fname_in, 'r'):
		if(counter%9==1): #Event info
			value_in_line=1
			run = ""
			event = ""
			for value in line.split():
				if(value_in_line==1): run = value
				if(value_in_line==2): event = value
				if(verbose and value_in_line==1): print "Run "+value
				if(verbose and value_in_line==2): print "Event "+value
				if(verbose and value_in_line==3): print "NParticles "+value
				value_in_line+=1
				line1_out = run + " " + event + " " + "2\n"
		if(counter%9==2): #Pi0 info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%9==3): #Pi0 p4
			value_in_line=1
			p4_string = line.split()
			p4_gam1   = TLorentzVector(float(p4_string[1]),float(p4_string[2]),float(p4_string[3]),float(p4_string[4]))
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
		if(counter%9==4): #Pi+ info
			value_in_line=1
			line2_str = line
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%9==5): #Pi+ p4
			value_in_line=1
			p4_string = line.split()
			p4_gam1   = TLorentzVector(float(p4_string[1]),float(p4_string[2]),float(p4_string[3]),float(p4_string[4]))
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
		if(counter%9==6): #Pi- info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%9==7): #Pi- p4
			value_in_line=1
			p4_string = line.split()
			p4_gam1   = TLorentzVector(float(p4_string[1]),float(p4_string[2]),float(p4_string[3]),float(p4_string[4]))
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
		if(counter%9==8): #proton info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
				line4_out = "2 14 0.938272\n"
		if(counter%9==0): #proton p4
			value_in_line=1
			p4_string = line.split()
			p4_proton   = TLorentzVector(float(p4_string[1]),float(p4_string[2]),float(p4_string[3]),float(p4_string[4]))
			p4_proton_boost_px = p4_proton.Px()
			p4_proton_boost_py = p4_proton.Py()
			p4_proton_boost_pz = p4_proton.Pz()
			p4_proton_boost_E = sqrt(abs(p4_proton_boost_px**2+p4_proton_boost_py**2+p4_proton_boost_pz**2-0.938272**2))
			p4_proton_boost = TLorentzVector(p4_proton_boost_px,p4_proton_boost_py,p4_proton_boost_pz,p4_proton_boost_E)
			
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
				line5_out = "   1 "+str(p4_proton.Px()) + " " + str(p4_proton.Py()) + " " + str(p4_proton.Pz()) + " " + str(p4_proton.E())+"\n"

			my_phi = p4_proton_boost.Phi()+3.14159 #Opposite proton
			my_theta = theta_curr * (3.14159/180.)
			gamma_px = str(e_gamma_curr*TMath.Sin(my_theta)*TMath.Cos(my_phi))
			gamma_py = str(e_gamma_curr*TMath.Sin(my_theta)*TMath.Sin(my_phi))
			gamma_pz = str(e_gamma_curr*TMath.Cos(my_theta))
			line2_out = "1 1 0\n"
			line3_out = "   0 "+gamma_px+" "+gamma_py+" "+gamma_pz+" "+str(e_gamma_curr)+"\n"
			p4_gam_fcal = TLorentzVector(float(gamma_px),float(gamma_py),float(gamma_pz),e_gamma_curr)
					
			# print "Beam photon: " + str( (p4_gam_fcal+p4_proton_boost).E())
					
			with open(fname_out, "a") as myfile:
				myfile.write(line1_out)	
				myfile.write(line2_out)	
				myfile.write(line3_out)	
				myfile.write(line4_out)	
				myfile.write(line5_out)	
				
		counter+=1
				
				
		# if(counter>200): break
	
	return

def MakeGenr8File(fname,NEVENTS,genr8_input_f):
	shell_exec("rm "+fname,"")
	genr8_command = "genr8 -r30300 -M"+str(NEVENTS)+" -A"+fname
	# genr8_input_f = "../Omega3Pi.conf"
	shell_exec(genr8_command, genr8_input_f)
	return
	
def shell_exec(command, stdin_str):
	if(stdin_str==""): 
		print("Executing command: "+command)
		status = subprocess.call(command.split(" "))
	else:
		print("Executing command: "+command+" < "+stdin_str)
		stdin_f = open(stdin_str)
		status = subprocess.call(command.split(" "), stdin=stdin_f )
	return
	
	
if __name__ == "__main__":
   main(sys.argv[1:])