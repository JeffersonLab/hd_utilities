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
from shutil import copyfile


#Root stuff
from ROOT import TFile, TTree, TBranch, TLorentzVector, TLorentzRotation, TVector3
from ROOT import TCanvas, TMath, TH2F, TH1F, TRandom, TGraphErrors, TGraph, TLine
from ROOT import gBenchmark, gDirectory, gROOT, gStyle, gPad

#My Stuff
#Required: add to PYTHONPATH environment variable, e.g.
#setenv PYTHONPATH /gpfs/home/j/z/jzarling/Karst/MyAnalyses/Python_stuff/jz_library/:$PYTHONPATH
# from jz_pyroot_helper import *
#from jz_pyroot_FitMacros import *

#Note! To change beam energy or t-distribution of omegas, see .conf file
#Limitations of this method: assumes monoenergetic beam E and has no omega P-wave or SDME modeling (pure phase space alone)
#Still, should be sufficient to get things correct in broad strokes

SCAN_OVER_ENERGY = False
SCAN_OVER_THETA  = True
# SCAN_OVER_ENERGY = True
# SCAN_OVER_THETA  = False

THETA_MIN_STUDY = 2.5 #If scanning over energy, this is minimum allowed 
THETA_MAX_STUDY = 10.5 #If scanning over energy, this is minimum allowed 
E_MIN_STUDY = 0.7 #If scanning over theta, this is minimum allowed 
E_MAX_STUDY = 100. #If scanning over thtea, this is minimum allowed 

CCDB_LOC = "sqlite:////home/fs7/jzarling/PhotonEffStudUtilities/CustomMCgen/genPhotonGunScan/ccdb.sqlite"
RCDB_LOC = "sqlite:////home/fs7/jzarling/PhotonEffStudUtilities/CustomMCgen/genPhotonGunScan/rcdb.sqlite"
GEANT_VERSION = 4

BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/Gun_omegaweighted_ScanThetaG4"

#Spectator photon cut (to match omega->3pi data cleanup selection)
E_SPECTATOR_MIN = 0.5

# NEVENTS = 100000 #To accept. Running genr8 step will require even more.

# NACCEPTED = 0 #DON'T TOUCH HERE! modified later

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
	while current_val < MAX_VAL+0.0001:
		current_dirname = BASE_DIR+"/"+str(current_val)
		if not os.path.exists(current_dirname): os.makedirs(current_dirname)
		os.chdir(current_dirname)
		print "output directory is: " + current_dirname
		
		ascii_to_prune = "genr8_omega_"+str(current_val)+".ascii"
		ascii_pruned = "pruned_gamma_p_"+str(current_val)+".ascii"
		pruned_hddm = "pruned_gamma_p_"+str(current_val)+".hddm"
		
		NACCEPTED = 0
		counter = 0 #Number of iterations
		#Generate events 10,000 at a time, check to accept afterwards
		while NACCEPTED < int(EVENTS_PER_val):
			counter += 1
			print "counter: "   + str(counter)
			print "NACCEPTED: " + str(NACCEPTED)
			MakeGenr8File(ascii_to_prune,10000,omega_config_loc) #Will overwrite the last one, uses process ID as seed so will end up different from last one too
			if(SCAN_OVER_ENERGY): NACCEPTED = PruneGenr8File(ascii_to_prune,ascii_pruned,True,current_val-VAL_STEP/2.,current_val+VAL_STEP/2.,NACCEPTED,int(EVENTS_PER_val),False)
			if(SCAN_OVER_THETA):  NACCEPTED = PruneGenr8File(ascii_to_prune,ascii_pruned,False,current_val-VAL_STEP/2.,current_val+VAL_STEP/2.,NACCEPTED,int(EVENTS_PER_val),False)
		SubmitQSubJob(ascii_pruned,pruned_hddm,controlin_loc,int(EVENTS_PER_val),int(RUN),gen_interactive)
		
		current_val+=VAL_STEP
		
		# MakeGenr8File(ascii_to_prune,int(EVENTS_PER_val),omega_config_loc)
		# shell_exec("rm " + ascii_pruned , "")
		# if(SCAN_OVER_ENERGY): PruneGenr8File(ascii_to_prune,ascii_pruned,current_val,float(THETA),False)
		# if(SCAN_OVER_THETA): PruneGenr8File(ascii_to_prune,ascii_pruned,float(E_GAMMA),current_val,False)
		# # SubmitQSubJob(rel_dir_name,ascii_pruned,pruned_hddm,energy)
		# SubmitQSubJob(ascii_pruned,pruned_hddm,current_val,controlin_loc,int(EVENTS_PER_val),int(RUN),gen_interactive)
		
	print("Done ")

	
#Assumes you have already changed to running/saving directory
# def SubmitQSubJob(rel_dir_name,ascii_file,hddm_name,e_gamma):
def SubmitQSubJob(ascii_file,hddm_name,controlin_loc,NEVENTS,run,gen_interactive):
	
	
	
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
		if(SCAN_OVER_ENERGY): myfile.write("hd_root -PPHOTONHISTS:THETAMIN_CUTPARM="+str(THETA_MIN_STUDY)+" -PPHOTONHISTS:THETAMAX_CUTPARM="+str(THETA_MAX_STUDY)+" -PTRIG:BYPASS=1 geant3_out_smeared.hddm -PNTHREADS=6 -PPLUGINS=photon_gun_hists,danarest,monitoring_hists \n")
		if(SCAN_OVER_THETA):  myfile.write("hd_root -PPHOTONHISTS:EMIN_CUTPARM="+str(E_MIN_STUDY)+" -PPHOTONHISTS:EMAX_CUTPARM="+str(E_MAX_STUDY)+" -PTRIG:BYPASS=1 geant3_out_smeared.hddm -PNTHREADS=6 -PPLUGINS=photon_gun_hists,danarest,monitoring_hists \n")
		myfile.write(" 	")
		myfile.write("\n")
	
	shell_exec("chmod +x ReplaceAndRun.csh","")
	if(not gen_interactive): shell_exec("qsub -N g_prot_gun ReplaceAndRun.csh","")
	else: shell_exec("./ReplaceAndRun.csh","")
	
	return
	
	
def PruneGenr8File(fname_in,fname_out,E_GAMMA_VARY,MIN_VAL,MAX_VAL,NACCEPTED,MAX_EVENTS,verbose = False):
	# print "input genr8 file: " + fname_in
	# print "output file: " + fname_out
	
	counter = 1
	
	line1_out = "" #Run and event info
	line2_out = "1 1 0.000000\n" #Gamma ID info
	line3_out = "" #Gamma P4 info
	
	for line in open(fname_in, 'r'):
	
		if(NACCEPTED >= MAX_EVENTS):
			# print "Done generating this point!"
			break
	
		if(counter%11==1): #Event info
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
				# line1_out = run + " " + event + " " + "1\n"
				line1_out = run + " " + str(NACCEPTED+1) + " " + "1\n"
		if(counter%11==2): #Gamma1 info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%11==3): #Gamma1 p4
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
		if(counter%11==4): #Gamma2 info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%11==5): #Gamma2 p4
			value_in_line=1
			p4_string = line.split()
			p4_gam2   = TLorentzVector(float(p4_string[1]),float(p4_string[2]),float(p4_string[3]),float(p4_string[4]))
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
		if(counter%11==6): #Pi+ info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%11==7): #Pi+ p4
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
		if(counter%11==8): #Pi- info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%11==9): #Pi- p4
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
		if(counter%11==10): #proton info
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Particle number in event: "+value
				if(verbose and value_in_line==2): print "Particle enum value "+value
				if(verbose and value_in_line==3): print "Particle mass "+value
				value_in_line+=1
		if(counter%11==0): #proton p4
			value_in_line=1
			for value in line.split():
				if(verbose and value_in_line==1): print "Charge: "+value
				if(verbose and value_in_line==2): print "Px "+value
				if(verbose and value_in_line==3): print "Py "+value
				if(verbose and value_in_line==4): print "Pz "+value
				if(verbose and value_in_line==5): print "E "+value
				value_in_line+=1
			
			#Finished with this event. Check if one of the two photons passes cuts.
			#If so, save event and increment NACCEPTED
			if(verbose):
				print "Cut on energy? " + str(E_GAMMA_VARY)
				print "Cut on theta? "  + str(not E_GAMMA_VARY)
				print "Min value: "     + str(MIN_VAL)
				print "Max value: "       + str(MAX_VAL)
				print "Gamma1 energy: " + str(p4_gam1.E())					
				print "Gamma2 energy: " + str(p4_gam2.E())
			
			GAMMA1_PASSES = False
			if(E_GAMMA_VARY and MIN_VAL < p4_gam1.E() and p4_gam1.E() < MAX_VAL and p4_gam2.E()>E_SPECTATOR_MIN): GAMMA1_PASSES = True
			if(not E_GAMMA_VARY and MIN_VAL < p4_gam1.Theta()*180/3.14159 and p4_gam1.Theta()*180/3.14159 < MAX_VAL): GAMMA1_PASSES = True
			if(verbose and GAMMA1_PASSES):
				print "Gamma1 passes: "		
				print "Min value: "     + str(MIN_VAL)
				print "Max value: "       + str(MAX_VAL)
				print "Gamma1 energy: " + str(p4_gam1.E())		
			if(GAMMA1_PASSES):
				with open(fname_out, "a") as myfile:
					line3_out = "   0 "+str(p4_gam1.Px())+" "+str(p4_gam1.Py())+" "+str(p4_gam1.Pz())+" "+str(p4_gam1.E())+"\n"
					myfile.write(line1_out)	
					myfile.write(line2_out)	
					myfile.write(line3_out)	
					NACCEPTED+=1
					
			GAMMA2_PASSES = False
			if(E_GAMMA_VARY and MIN_VAL < p4_gam2.E() and p4_gam2.E() < MAX_VAL and p4_gam1.E()>E_SPECTATOR_MIN): GAMMA2_PASSES = True
			if(not E_GAMMA_VARY and MIN_VAL < p4_gam2.Theta()*180/3.14159 and p4_gam2.Theta()*180/3.14159 < MAX_VAL): GAMMA2_PASSES = True
			if(verbose and GAMMA2_PASSES):
				print "Gamma2 passes: "		
				print "Min value: "     + str(MIN_VAL)
				print "Max value: "       + str(MAX_VAL)
				print "Gamma2 energy: " + str(p4_gam2.E())					
			if(GAMMA2_PASSES):
				with open(fname_out, "a") as myfile:
					line3_out = "   0 "+str(p4_gam2.Px())+" "+str(p4_gam2.Py())+" "+str(p4_gam2.Pz())+" "+str(p4_gam2.E())+"\n"
					myfile.write(line1_out)	
					myfile.write(line2_out)	
					myfile.write(line3_out)	
					NACCEPTED+=1
			
			
			if(verbose):
				print "\n"
			
			
			
			
			# with open(fname_out, "a") as myfile:
				# myfile.write(line1_out)	
				# myfile.write(line2_out)	
				
		counter+=1
				
				
		# if(counter>200): break
	
	return NACCEPTED
	
	
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