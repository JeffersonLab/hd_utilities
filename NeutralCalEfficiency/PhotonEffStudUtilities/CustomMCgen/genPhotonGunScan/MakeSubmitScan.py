#!/usr/bin/env python

from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
import glob
import fnmatch
import shutil

VERBOSE_LEVEL = 0

#Specify base output directory here (or use current directory if blank)
BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/GunOnly_ScanTheta_G3"
# BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/GunESaveTraj_ThetaScanG3"
# BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/GunHighStat_250MeV_geant4INTERACTIVESHORT"
# BASE_DIR = "/home/fs7/jzarling/MCPhotonEffOut/GunESaveTraj_ThetaScanTestg3"

# SCAN_OVER_ENERGY = True
# SCAN_OVER_THETA  = False
SCAN_OVER_ENERGY = False
SCAN_OVER_THETA  = True

THETA   = "6." #Used if scanning over E
E_GAMMA = "1." #Used if scanning over theta
# THETA = "25."

def main(argv):
	#Usage controls from OptionParser
	parser_usage = "this_program.py run min_scan_val max_scan_val events_per_val(default=10,000) step(default=0.05GeV/degrees)"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)
	if(len(args) < 3 or len(args) > 6):
			parser.print_help()
			return
	
	print "Checking for MCWrapper... "
	MCWRAPPER_CENTRAL = os.environ['MCWRAPPER_CENTRAL']
	print "Found MCWrapper. \n"
	
	original_directory = os.getcwd()
	
	RUN = argv[0]
	MIN_VAL = float(argv[1])
	MAX_VAL = float(argv[2])
	EVENTS_PER_val = "10000"
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
		print "output directory is: " + current_dirname
		
		
		#replacement dictionary of format dictionary = [old:new]
		if(SCAN_OVER_ENERGY): replace_particlegun_dict = {"KINE_TOREPLACE":"KINE   101      "+str(current_val)+"       "+THETA+"   0.      0.            0.        1000."}
		if(SCAN_OVER_THETA): replace_particlegun_dict = {"KINE_TOREPLACE":"KINE   101      "+E_GAMMA+"       "+str(current_val)+"   0.      0.            0.        1000."}
		create_from_template("./particle_gun_TEMPLATE.cfg",current_dirname+"/particle_gun.cfg", replace_particlegun_dict)
		replace_MCconfig_dict = {
			'RUNNING_DIRECTORY_toreplace':'RUNNING_DIRECTORY='+current_dirname,
			'DATA_OUTPUT_BASE_DIR_toreplace':'DATA_OUTPUT_BASE_DIR='+current_dirname,
			'GENERATOR_CONFIG_toreplace':'GENERATOR_CONFIG='+current_dirname+"/particle_gun.cfg",
		}
		print "Setting up workspace"
		create_from_template("./MC_TEMPLATE.config",current_dirname+"/MC.config", replace_MCconfig_dict)
	
		print "Submitting MCWrapper job"
		#Run individual thing
		if(not gen_interactive): shell_exec(MCWRAPPER_CENTRAL+"/gluex_MC.py "+current_dirname+"/MC.config "+RUN+" "+EVENTS_PER_val+" cleanmcsmear=0 batch=1 per_file=100000") #Run MC generation
		if(gen_interactive):     shell_exec(MCWRAPPER_CENTRAL+"/gluex_MC.py "+current_dirname+"/MC.config "+RUN+" "+EVENTS_PER_val+" cleanmcsmear=0  per_file=100000") #Run MC generation
	
		current_val += VAL_STEP
			
	print "DONE!"
			
			
#General purpose function: 
#First argument: input text file
#Second argument: output file path
#Third argument: dictionary. First entry of each pair is SEARCH_STRING_TO_REPLACE, second entry is REPLACEMENT_STRING
#Fourth argument: make executable flag
def create_from_template(template_full_path,ofile_full_path, replacements,make_executable=False):
	shell_exec("rm  -f "+ofile_full_path) #Remove script in directory if one existed
	with open(template_full_path) as infile, open(ofile_full_path, 'w') as outfile:
		for line in infile:
			for src, target in replacements.iteritems():
				line = line.replace(src, target)
			outfile.write(line)
	if(make_executable): shell_exec("chmod 777 "+ofile_full_path,"")
	return
			
			
	
def shell_exec(command):
	if(VERBOSE_LEVEL>=1): print("Executing command: "+command)
	status = subprocess.call(command.split(" "))
	# status = subprocess.call(command)
	return

	
if __name__ == "__main__":
   main(sys.argv[1:])
   
