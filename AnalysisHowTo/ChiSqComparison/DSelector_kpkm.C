#include "DSelector_kpkm.h"
#include <TEnv.h>

void DSelector_kpkm::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "kpkm.root"; //"" for none
	dOutputTreeFileName = "tree_kpkm.root"; //"" for none
	dFlatTreeFileName = "flat_tree_kpkm.root"; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = "kpkm_Tree"; //if blank, default name will be chosen

	//Because this function gets called for each TTree in the TChain, we must be careful:
		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms
	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree
	//gDirectory now points to the output file with name dOutputFileName (if any)
	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	Get_ComboWrappers();
	dPreviousRunNumber = 0;

	/*********************************** EXAMPLE USER INITIALIZATION: ANALYSIS ACTIONS **********************************/
	// Define particles of interest for histogramming
        std::deque<Particle_t> MyPhi;
        MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);
        std::deque<Particle_t> MyLambda;
        MyLambda.push_back(KMinus); MyLambda.push_back(Proton);

	// Set up default cut parameters
	dSlope = 1.0;
	dYint = 0.0;

	// Read in parameters from configuration file
	TEnv *env = new TEnv(dOption);
	std::cout << "dOption: " << dOption << std::endl;
	if (!dOption)
		std::cout << "No configuration file provided for TEnv" << std::endl;
	else
	{
		dSlope = env->GetValue("dSlope", dSlope);
		dYint = env->GetValue("dYint", dYint);
		env->PrintEnv();
	}

	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data

	// Initial histograms
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Pre-cuts"));
        dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Pre-cuts"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "Phi_pre-cut"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyLambda, 1000, 1.0, 3.0, "Lambda_pre-cut"));
        dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1, "Pre-cuts"));
        dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper, "Pre-cuts"));

	// Cut on missing mass squared and histogram the invariant mass
        dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.02, 0.02));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "Phi_MM2-cut"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyLambda, 1000, 1.0, 3.0, "Lambda_MM2-cut"));

	// Cut on dEdx and histogram the invariant mass
        dAnalysisActions.push_back(new DCutAction_dEdx(dComboWrapper, false, Proton, SYS_CDC, "dEdx-meas"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "Phi_dEdx-cut"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyLambda, 1000, 1.0, 3.0, "Lambda_dEdx-cut"));

	// Cut on the PID delta t and histogram the invariant mass
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, Proton, SYS_TOF, "p_TOF"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, Proton, SYS_BCAL, "p_BCAL"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, Proton, SYS_FCAL, "p_FCAL"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.4, KPlus, SYS_TOF, "Kp_TOF"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL, "Kp_BCAL"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, KPlus, SYS_FCAL, "Kp_FCAL"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.4, KMinus, SYS_TOF, "Km_TOF"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KMinus, SYS_BCAL, "Km_BCAL"));
        dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, KMinus, SYS_FCAL, "Km_FCAL"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "Phi_PID-cuts"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyLambda, 1000, 1.0, 3.0, "Lambda_PID-cuts"));

	// Histograms
        dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));
        dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Post_timing"));
        dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Post_timing"));

	// Chisq comparison cut and histogram the invariant mass
	TF1 *f1 = new TF1("f1", "pol1", 0, 1000);
	f1->SetParameter(0, dYint);
	f1->SetParameter(1, dSlope);
        dAnalysisActions.push_back(new DCutAction_ChiSqOrCL(dComboWrapper, "pippim", true, f1, "Post_timing"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "Phi_chisq-cut"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyLambda, 1000, 1.0, 3.0, "Lambda_chisq-cut"));

	// Histograms
        dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper,"Post_chisq-cut"));
        dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Post_chisq-cut"));

	// Cut on Chisq or confidence level after performing comparison cut. Histogram the invariant mass
        dAnalysisActions.push_back(new DCutAction_KinFitFOM(dComboWrapper, 1E-10));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "Phi_CL-cut"));
        dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyLambda, 1000, 1.0, 3.0, "Lambda_CL-cut"));

	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	//dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL));

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 600, -0.06, 0.06);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 600, 0.0, 12.0);

	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - MAIN TREE *************************/

	//EXAMPLE MAIN TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):
	//The type for the branch must be included in the brackets
	//1st function argument is the name of the branch
	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)
	/*
	dTreeInterface->Create_Branch_Fundamental<Int_t>("my_int"); //fundamental = char, int, float, double, etc.
	dTreeInterface->Create_Branch_FundamentalArray<Int_t>("my_int_array", "my_int");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("my_combo_array", "NumCombos");
	dTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("my_p4");
	dTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("my_p4_array");
	*/

	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/

	//EXAMPLE FLAT TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):
	//The type for the branch must be included in the brackets
	//1st function argument is the name of the branch
	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)
	/*
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("flat_my_int"); //fundamental = char, int, float, double, etc.
	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>("flat_my_int_array", "flat_my_int");
	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("flat_my_p4");
	dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("flat_my_p4_array");
	*/

	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/

	//TO SAVE PROCESSING TIME
		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up
		//By default, for each event, the data is retrieved for all branches
		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need
		//Do this by doing something similar to the commented code below

	//dTreeInterface->Clear_GetEntryBranches(); //now get none
	//dTreeInterface->Register_GetEntryBranch("Proton__P4"); //manually set the branches you want
}

Bool_t DSelector_kpkm::Process(Long64_t locEntry)
{
	// The Process() function is called for each entry in the tree. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	//
	// This function should contain the "body" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms.
	//
	// The processing can be stopped by calling Abort().
	// Use fStatus to set the return value of TTree::Process().
	// The return value is currently not used.

	//CALL THIS FIRST
	DSelector::Process(locEntry); //Gets the data from the tree for the entry
	//cout << "RUN " << Get_RunNumber() << ", EVENT " << Get_EventNumber() << endl;
	//TLorentzVector locProductionX4 = Get_X4_Production();

	/******************************************** GET POLARIZATION ORIENTATION ******************************************/

/*
	//Only if the run number changes
	//RCDB environment must be setup in order for this to work! (Will return false otherwise)
	UInt_t locRunNumber = Get_RunNumber();
	if(locRunNumber != dPreviousRunNumber)
	{
		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);
		dPreviousRunNumber = locRunNumber;
	}
*/

	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/

	//ANALYSIS ACTIONS: Reset uniqueness tracking for each action
	//For any actions that you are executing manually, be sure to call Reset_NewEvent() on them here
	Reset_Actions_NewEvent();

	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING
		//Sometimes, some content is the exact same between one combo and the next
			//e.g. maybe two combos have different beam particles, but the same data for the final-state
		//When histogramming, you don't want to double-count when this happens: artificially inflates your signal (or background)
		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)
		//Then for each combo, just compare to what you used before, and make sure it's unique

	//EXAMPLE 1: Particle-specific info:
	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search

	//EXAMPLE 2: Combo-specific info:
		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMass;

	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE

	/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

	/*
	Int_t locMyInt = 7;
	dTreeInterface->Fill_Fundamental<Int_t>("my_int", locMyInt);

	TLorentzVector locMyP4(4.0, 3.0, 2.0, 1.0);
	dTreeInterface->Fill_TObject<TLorentzVector>("my_p4", locMyP4);

	for(int loc_i = 0; loc_i < locMyInt; ++loc_i)
		dTreeInterface->Fill_Fundamental<Int_t>("my_int_array", 3*loc_i, loc_i); //2nd argument = value, 3rd = array index
	*/

	/************************************************* LOOP OVER COMBOS *************************************************/

	//Loop over combos
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);

		// Is used to indicate when combos have been cut
		if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created
			continue; // Combo has been cut previously

		/********************************************** GET PARTICLE INDICES *********************************************/

		//Used for tracking uniqueness when filling histograms, and for determining unused particles

		//Step 0
		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();
		Int_t locKPlusTrackID = dKPlusWrapper->Get_TrackID();
		Int_t locKMinusTrackID = dKMinusWrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locKPlusP4 = dKPlusWrapper->Get_P4();
		TLorentzVector locKMinusP4 = dKMinusWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlusP4_Measured = dKPlusWrapper->Get_P4_Measured();
		TLorentzVector locKMinusP4_Measured = dKMinusWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locKPlusP4_Measured + locKMinusP4_Measured + locProtonP4_Measured;

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		//if you manually execute any actions, and it fails a cut, be sure to call:
			//dComboWrapper->Set_IsComboCut(true);

		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

		/*
		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);
		//for arrays below: 2nd argument is value, 3rd is array index
		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)
			//So, when you draw the branch, be sure to cut on "IsComboCut" to avoid these.
		dTreeInterface->Fill_Fundamental<Float_t>("my_combo_array", -2*loc_i, loc_i);
		dTreeInterface->Fill_TObject<TLorentzVector>("my_p4_array", locMyComboP4, loc_i);
		*/

		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/

		//Histogram beam energy (if haven't already)
		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
		{
			dHist_BeamEnergy->Fill(locBeamP4.E());
			locUsedSoFar_BeamEnergy.insert(locBeamID);
		}

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();

		//Uniqueness tracking: Build the map of particles used for the missing mass
			//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlusTrackID);
		locUsedThisCombo_MissingMass[KMinus].insert(locKMinusTrackID);
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);

		//compare to what's been used so far
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
			dHist_MissingMassSquared->Fill(locMissingMassSquared);
			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
		}

		//E.g. Cut
		//if((locMissingMassSquared < -0.04) || (locMissingMassSquared > 0.04))
		//{
		//	dComboWrapper->Set_IsComboCut(true);
		//	continue;
		//}

		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/

		/*
		//FILL ANY CUSTOM BRANCHES FIRST!!
		Int_t locMyInt_Flat = 7;
		dFlatTreeInterface->Fill_Fundamental<Int_t>("flat_my_int", locMyInt_Flat);

		TLorentzVector locMyP4_Flat(4.0, 3.0, 2.0, 1.0);
		dFlatTreeInterface->Fill_TObject<TLorentzVector>("flat_my_p4", locMyP4_Flat);

		for(int loc_j = 0; loc_j < locMyInt_Flat; ++loc_j)
		{
			dFlatTreeInterface->Fill_Fundamental<Int_t>("flat_my_int_array", 3*loc_j, loc_j); //2nd argument = value, 3rd = array index
			TLorentzVector locMyComboP4_Flat(8.0, 7.0, 6.0, 5.0);
			dFlatTreeInterface->Fill_TObject<TLorentzVector>("flat_my_p4_array", locMyComboP4_Flat, loc_j);
		}
		*/

		//FILL FLAT TREE
		//Fill_FlatTree(); //for the active combo
	} // end of combo loop

	//FILL HISTOGRAMS: Num combos / events surviving actions
	Fill_NumCombosSurvivedHists();

	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/
/*
	//Thrown beam: just use directly
	if(dThrownBeam != NULL)
		double locEnergy = dThrownBeam->Get_P4().E();

	//Loop over throwns
	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dThrownWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}
*/
	/****************************************** LOOP OVER OTHER ARRAYS (OPTIONAL) ***************************************/
/*
	//Loop over beam particles (note, only those appearing in combos are present)
	for(UInt_t loc_i = 0; loc_i < Get_NumBeam(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dBeamWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}

	//Loop over charged track hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dChargedHypoWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}

	//Loop over neutral particle hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}
*/

	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/
/*
	Bool_t locIsEventCut = true;
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i) {
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);
		// Is used to indicate when combos have been cut
		if(dComboWrapper->Get_IsComboCut())
			continue;
		locIsEventCut = false; // At least one combo succeeded
		break;
	}
	if(!locIsEventCut && dOutputTreeFileName != "")
		Fill_OutputTree();
*/

	return kTRUE;
}

void DSelector_kpkm::Finalize(void)
{
	//Save anything to output here that you do not want to be in the default DSelector output ROOT file.

	//Otherwise, don't do anything else (especially if you are using PROOF).
		//If you are using PROOF, this function is called on each thread,
		//so anything you do will not have the combined information from the various threads.
		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem.

	//DO YOUR STUFF HERE

	//CALL THIS LAST
	DSelector::Finalize(); //Saves results to the output file
}
