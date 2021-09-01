#!/usr/bin/env python

from gluupy_histmaker import *

# CUTS!
# # Each cut on single line is a list: branch name, cut string, value
# # Cut is applied by parsing a cut string. It expects the format: "cut stringname [operator] " followed by float
# # # 1: cut quantity (this is for human readability only, has no impact on code) 
# # # 2: arithmetic operator (examples: ">",  "<",  "==",   "!=" for greater than, less than, equals, and not equals )  
# # # If a cut is supplied that doesn't follow this form, we exit early 
ALL_CUTS_LIST = [
	["beam_p4_meas__E","Beam E > ",6.0],
	["p_p4_pmag"  ,"Proton momentum > ",0.350],
	["chi2_ndf","chi^2/ndf < ",5.0],
]

def main(argv):

	# Usage: 	
	# >> thisScript.py [OUTFILE.root] [INFILE1.root] [INFILE2.root ...]
	# Additional options: -M [num] to process only first [num] entries, -V verbose output, -f force overwrite output, --max-entries-perfile [num] process first [num] entries from each file
	args = gluupy_setup(argv,ALL_CUTS_LIST) # Initial standardized setup, returns all parsed "args" from argparse module. It also checks that cuts in ALL_CUTS_LIST appear valid
	
	# Set from arguments read in
	infile_name = args.infile_list[0]
	max_entries=int(args.max_entries) # Convert float to int (float was desirable in options parsing so that scientific notation like 1e6 for 1,000,000 was accepted)	
	
	nbins = 500
	# Create, fill histograms
	h_chi2_ndf_nocuts            = TH1F("h_chi2_ndf_nocuts","",nbins,0.,10.)
	h_eta_kin_nocuts             = TH1F("h_eta_kin_nocuts","",nbins,0.,1.)
	# Histograms filled after cuts
	h_chi2_ndf_postcuts          = TH1F("h_chi2_ndf_postcuts","",nbins,0.,10.)
	h_eta_kin                    = TH1F("h_eta_kin","",nbins,0.,1.)
	h_eta_kin_doublecountallowed = TH1F("h_eta_kin_doublecountallowed","",nbins,0.,1.)
	h_NumFS_ThisBeam             = TH1F("h_NumFS_ThisBeam","",nbins,0.,20.)

	# Reaction independent (assuming you reconstruct a proton)
	# branch_names_to_use = [] # Empty list fetches all branches (will be slower)
	branch_names_to_use = ["event","chi2_ndf", "accidweight","beam_p4_meas__E","p_p4_meas_px","p_p4_meas_py","p_p4_meas_pz",]
	branch_names_to_use.extend(["eta_mass_meas","eta_mass_kin","pi0_1_mass_meas","pi0_2_mass_meas","pi0_3_mass_meas","pi0_1_mass_kin","pi0_2_mass_kin","pi0_3_mass_kin",])

	# Get branches using uproot
	branches_dict = GetBranchesUproot(args.infile_list[0],max_entries,branch_names_to_use) # Arguments: filename, OPTIONAL: max entries to parse (default=-1 => all entries), OPTIONAL: list of string branchnames to retrieve (default=get all branches)
	# If you want to define any of your own branches, do so here
	# branches_dict["x4_prot_meas_R"] = np.sqrt( branches_dict["x4_prot_meas_x"]**2 + branches_dict["x4_prot_meas_y"]**2  )
	branches_dict["p_p4_pmag"] = np.sqrt( branches_dict["p_p4_meas_px"]**2 + branches_dict["p_p4_meas_py"]**2 + branches_dict["p_p4_meas_pz"]**2)

	# Fill these histograms before any cuts (neglect FS weighting, some double counting will occur)
	FillHistFromBranchDict(h_chi2_ndf_nocuts,branches_dict,"chi2_ndf",DoFSWeighting=False)
	FillHistFromBranchDict(h_eta_kin_nocuts,branches_dict,"eta_mass_kin",DoFSWeighting=False)
	
	# Apply cuts (and add final state weight factors as new branch "FS_weight")
	branches_dict = ApplyCutsReduceArrays(branches_dict,ALL_CUTS_LIST)
	branches_dict["Num_FS"] = 1./branches_dict["FS_weight"]
	
	# Fill histograms after cuts. Adding FS weight factors prevents double counting.
	FillHistFromBranchDict(h_chi2_ndf_postcuts,branches_dict,"chi2_ndf")
	FillHistFromBranchDict(h_eta_kin,branches_dict,"eta_mass_kin")
	FillHistFromBranchDict(h_eta_kin_doublecountallowed,branches_dict,"eta_mass_kin",DoFSWeighting=False)
	FillHistFromBranchDict(h_NumFS_ThisBeam,branches_dict,"Num_FS",DoAccidentalSub=False,DoFSWeighting=False)
	
	SaveAllHists(args.outfile) #Saves ALL histograms opened/created to this point. Print overall processing rate
	



if __name__ == "__main__":
   main(sys.argv[1:])

