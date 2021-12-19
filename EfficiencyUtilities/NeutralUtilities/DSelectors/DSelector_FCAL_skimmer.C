#include "DSelector_FCAL_skimmer.h"

void DSelector_FCAL_skimmer::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	// dOutputFileName = "FCAL_skimmer.root"; //"" for none
	// dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = "p3pi_FCALStudy_DSelectorOut.root"; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = "p3pi_FCALStudy"; //if blank, default name will be chosen

	jz_get_thrown_info = true;
	
	//Because this function gets called for each TTree in the TChain, we must be careful:
		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms
	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree
	//gDirectory now points to the output file with name dOutputFileName (if any)
	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	Get_ComboWrappers();
	dPreviousRunNumber = 0;
	jz_NumUnusedCombosSaved = 1;
    jz_UnusedPhotonThetaCut = 0; // Degrees. Candidates that we try to match to missing photon: don't save photons below this theta.

	/*********************************** EXAMPLE USER INITIALIZATION: ANALYSIS ACTIONS **********************************/

	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data

	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));
	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	//dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL));

	//MASSES
	//dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, Lambda, 1000, 1.0, 1.2, "Lambda"));
	//dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));

	//CUT MISSING MASS
	//dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.03, 0.02));

	//BEAM ENERGY
	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));
	//dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 8.4, 9.05));

	//KINEMATICS
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));

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
	

	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("RecM_Proton");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("RecM_Proton_pkf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("threepi_mass");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("threepi_mass_pkf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("twogamma_mass");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("twogamma_mass_pkf");
	
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_px");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_py");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_pz");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_E");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_px");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_py");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_pz");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_E");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_px");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_py");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_pz");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_E");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_px");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_py");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_pz");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_E");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_px");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_py");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_pz");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_E");
	
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("PiPlus_PIDhit");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_TrackingChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("PiPlus_Tracking_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_TimingChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_dEdxChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_beta_meas");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("PiMinus_PIDhit");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_TrackingChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("PiMinus_Tracking_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_TimingChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_dEdxChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_beta_meas");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("Proton_PIDhit");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_TrackingChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("Proton_Tracking_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_TimingChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_dEdxChi2_ndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_beta_meas");
	
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("MM2");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("gam2_theta");
	
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("rf_deltaT");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beamE");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("vertex_z");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("vertex_r");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("NExtraTracks");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ExtraShowerE");
	
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kinfit_CL");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kinfit_chi2_ndf");
	
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("Run");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("Event");

	
	string locArraySizeString  =  "NumMaxMissingCandidates";
	dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>(locArraySizeString);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingCan_Px","NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingCan_Py","NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingCan_Pz","NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingCan_E" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingFound_Angle" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingFound_DelPhi" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingFound_DelTheta" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>("MissingCan_ISFCAL" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>("CandidateCounter" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("threepi_can_mass" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("twogamma_can_mass" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	if(jz_get_thrown_info) 	dFlatTreeInterface->Create_Branch_FundamentalArray<Double_t>("MissingCan_ThrOpangle" ,"NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("NMissingCandidates");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("NFCALCandidates");
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("NBCALCandidates");

	if(jz_get_thrown_info) {
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_px_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_py_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_pz_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiPlus_E_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_px_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_py_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_pz_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("PiMinus_E_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_px_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_py_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_pz_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Proton_E_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_px_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_py_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_pz_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_E_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_px_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_py_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_pz_th");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_E_th");
		
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_FOM");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_FOM");
		
		// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_DeltaPhi_th");
		// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma1_DeltaTheta_th");
		// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_DeltaPhi_th");
		// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("Gamma2_DeltaTheta_th");
		
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("two_gamma_opangle_thr");
		dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("gam2_ThrRecon_opangle");
	}	
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

Bool_t DSelector_FCAL_skimmer::Process(Long64_t locEntry)
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

	//Only if the run number changes
	//RCDB environment must be setup in order for this to work! (Will return false otherwise)
	UInt_t locRunNumber = Get_RunNumber();
	if(locRunNumber != dPreviousRunNumber)
	{
		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);
		dPreviousRunNumber = locRunNumber;
	}

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
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		//Step 1
		Int_t locPiPlusTrackID = dPiPlusWrapper->Get_TrackID();
		Int_t locPiMinusTrackID = dPiMinusWrapper->Get_TrackID();

		//Step 2
		Int_t locPhotonNeutralID = dPhotonWrapper->Get_NeutralID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();
		//Step 1
		TLorentzVector locPiPlusP4 = dPiPlusWrapper->Get_P4();
		TLorentzVector locPiMinusP4 = dPiMinusWrapper->Get_P4();
		//Step 2
		TLorentzVector locMissingPhotonP4 = dMissingPhotonWrapper->Get_P4();
		TLorentzVector locPhotonP4 = dPhotonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPiPlusP4_Measured = dPiPlusWrapper->Get_P4_Measured();
		TLorentzVector locPiMinusP4_Measured = dPiMinusWrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPhotonP4_Measured = dPhotonWrapper->Get_P4_Measured();

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locProtonP4_Measured + locPiPlusP4_Measured + locPiMinusP4_Measured + locPhotonP4_Measured;

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
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);
		locUsedThisCombo_MissingMass[PiPlus].insert(locPiPlusTrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinusTrackID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhotonNeutralID);

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

		
		//Fiducial cuts
		if(dPiPlusWrapper->Get_ChiSq_Tracking() / dPiPlusWrapper->Get_NDF_Tracking() > 100) continue;
		if(dPiPlusWrapper->Get_ChiSq_Timing_Measured() / dPiPlusWrapper->Get_NDF_Timing() > 100) continue;
		if(dPiPlusWrapper->Get_ChiSq_DCdEdx() / dPiPlusWrapper->Get_NDF_DCdEdx() > 100) continue;
		if(dPiMinusWrapper->Get_ChiSq_Tracking() / dPiMinusWrapper->Get_NDF_Tracking() > 100) continue;
		if(dPiMinusWrapper->Get_ChiSq_Timing_Measured() / dPiMinusWrapper->Get_NDF_Timing() > 100) continue;
		if(dPiMinusWrapper->Get_ChiSq_DCdEdx() / dPiMinusWrapper->Get_NDF_DCdEdx() > 100) continue;
		if(dProtonWrapper->Get_ChiSq_Tracking() / dProtonWrapper->Get_NDF_Tracking() > 100) continue;
		if(dProtonWrapper->Get_ChiSq_Timing_Measured() / dProtonWrapper->Get_NDF_Timing() > 100) continue;
		if(dProtonWrapper->Get_ChiSq_DCdEdx() / dProtonWrapper->Get_NDF_DCdEdx() > 100) continue;
		
		if(dPiPlusWrapper->Get_Beta_Timing_Measured()<-1 || dPiPlusWrapper->Get_Beta_Timing_Measured()>5) continue;
		if(dPiMinusWrapper->Get_Beta_Timing_Measured()<-1 || dPiMinusWrapper->Get_Beta_Timing_Measured()>5) continue;
		if(dProtonWrapper->Get_Beta_Timing_Measured()<-1 || dProtonWrapper->Get_Beta_Timing_Measured()>5) continue;
		
		if(Get_NumNeutralHypos()>jz_NumUnusedCombosSaved+1) continue; // +1 needed to count the originally found photon
		
		//Beam energy cut
		if(locBeamP4.E()<8.2 || locBeamP4.E()>8.8) continue;
		
		dFlatTreeInterface->Fill_Fundamental<Double_t>("RecM_Proton", (locBeamP4_Measured+dTargetP4-locProtonP4).M() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("RecM_Proton_pkf", (locBeamP4_Measured+dTargetP4-locProtonP4_Measured).M() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("threepi_mass", (locPiPlusP4+locPiMinusP4+locMissingPhotonP4+locPhotonP4).M() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("threepi_mass_pkf", (locPiPlusP4_Measured+locPiMinusP4_Measured+locMissingPhotonP4+locPhotonP4_Measured).M() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("twogamma_mass",(locMissingPhotonP4+locPhotonP4).M());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("twogamma_mass_pkf",(locMissingPhotonP4+locPhotonP4_Measured).M());
		
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_px",locPiPlusP4.Px());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_py",locPiPlusP4.Py());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_pz",locPiPlusP4.Pz());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_E",locPiPlusP4.E());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_px",locPiMinusP4.Px());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_py",locPiMinusP4.Py());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_pz",locPiMinusP4.Pz());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_E",locPiMinusP4.E());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_px",locProtonP4.Px());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_py",locProtonP4.Py());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_pz",locProtonP4.Pz());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_E",locProtonP4.E());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_px",locMissingPhotonP4.Px());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_py",locMissingPhotonP4.Py());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_pz",locMissingPhotonP4.Pz());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_E",locMissingPhotonP4.E());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_px",locPhotonP4.Px());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_py",locPhotonP4.Py());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_pz",locPhotonP4.Pz());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_E",locPhotonP4.E());

		DetectorSystem_t locSystemPiPlus = dPiPlusWrapper->Get_Detector_System_Timing();
		DetectorSystem_t locSystemPiMinus = dPiMinusWrapper->Get_Detector_System_Timing();
		DetectorSystem_t locSystemProton = dProtonWrapper->Get_Detector_System_Timing();
		dFlatTreeInterface->Fill_Fundamental<Int_t>("PiPlus_PIDhit",locSystemPiPlus);		
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_TrackingChi2_ndf",dPiPlusWrapper->Get_ChiSq_Tracking() / dPiPlusWrapper->Get_NDF_Tracking() );
		dFlatTreeInterface->Fill_Fundamental<Int_t>("PiPlus_Tracking_ndf",dPiPlusWrapper->Get_NDF_Tracking() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_TimingChi2_ndf",dPiPlusWrapper->Get_ChiSq_Timing_Measured() / dPiPlusWrapper->Get_NDF_Timing() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_dEdxChi2_ndf",dPiPlusWrapper->Get_ChiSq_DCdEdx() / dPiPlusWrapper->Get_NDF_DCdEdx() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_beta_meas",dPiPlusWrapper->Get_Beta_Timing_Measured());
		dFlatTreeInterface->Fill_Fundamental<Int_t>("PiMinus_PIDhit",locSystemPiMinus);
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_TrackingChi2_ndf",dPiMinusWrapper->Get_ChiSq_Tracking() / dPiMinusWrapper->Get_NDF_Tracking() );
		dFlatTreeInterface->Fill_Fundamental<Int_t>("PiMinus_Tracking_ndf", dPiMinusWrapper->Get_NDF_Tracking() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_TimingChi2_ndf",dPiMinusWrapper->Get_ChiSq_Timing_Measured() / dPiMinusWrapper->Get_NDF_Timing() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_dEdxChi2_ndf",dPiMinusWrapper->Get_ChiSq_DCdEdx() / dPiMinusWrapper->Get_NDF_DCdEdx() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_beta_meas",dPiMinusWrapper->Get_Beta_Timing_Measured());
		dFlatTreeInterface->Fill_Fundamental<Int_t>("Proton_PIDhit",locSystemProton);
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_TrackingChi2_ndf",dProtonWrapper->Get_ChiSq_Tracking() / dProtonWrapper->Get_NDF_Tracking() );
		dFlatTreeInterface->Fill_Fundamental<Int_t>("Proton_Tracking_ndf", dProtonWrapper->Get_NDF_Tracking() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_TimingChi2_ndf",dProtonWrapper->Get_ChiSq_Timing_Measured() / dProtonWrapper->Get_NDF_Timing() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_dEdxChi2_ndf",dProtonWrapper->Get_ChiSq_DCdEdx() / dProtonWrapper->Get_NDF_DCdEdx() );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_beta_meas",dProtonWrapper->Get_Beta_Timing_Measured());		
		
		dFlatTreeInterface->Fill_Fundamental<Double_t>("MM2",locMissingP4_Measured.M2());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("gam2_theta", locPhotonP4.Theta()*180./3.14159 );
		
		TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured(); //JZEDIT: get beam photon timing info
		Double_t rf_timing = locBeamX4_Measured.T() - (dComboWrapper->Get_RFTime_Measured() + (locBeamX4_Measured.Z()- dComboWrapper->Get_TargetCenter().Z())/29.9792458 );
		dFlatTreeInterface->Fill_Fundamental<Double_t>("rf_deltaT",rf_timing);
		dFlatTreeInterface->Fill_Fundamental<Double_t>("beamE",locBeamP4.E());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("vertex_z",locBeamX4_Measured.Z());
		dFlatTreeInterface->Fill_Fundamental<Double_t>("vertex_r",locBeamX4_Measured.Pt());
		dFlatTreeInterface->Fill_Fundamental<Int_t>("NExtraTracks",-100);
		dFlatTreeInterface->Fill_Fundamental<Double_t>("ExtraShowerE",dComboWrapper->Get_Energy_UnusedShowers());
		
		Double_t kinfit_CL = dComboWrapper->Get_ConfidenceLevel_KinFit();
		Double_t kinfit_chi2 = dComboWrapper->Get_ChiSq_KinFit();
		Double_t kinfit_ndf = dComboWrapper->Get_NDF_KinFit();
		Double_t kinfit_chi2_ndf = kinfit_chi2/kinfit_ndf;
		dFlatTreeInterface->Fill_Fundamental<Double_t>("kinfit_CL",kinfit_CL);
		dFlatTreeInterface->Fill_Fundamental<Double_t>("kinfit_chi2_ndf",kinfit_chi2_ndf);
		
		dFlatTreeInterface->Fill_Fundamental<Int_t>("Run",Get_RunNumber());
		dFlatTreeInterface->Fill_Fundamental<Int_t>("Event",Get_EventNumber());
		
		
		// Loop over charged shower hypotheses
		Int_t candidate_counter = 0;
		Int_t BCAL_counter = 0;
		Int_t FCAL_counter = 0;
		dFlatTreeInterface->Fill_Fundamental<UInt_t>("NumMaxMissingCandidates",jz_NumUnusedCombosSaved);
		// for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)
		TLorentzVector curr_missing_can_p4;
	
	

		
		
		
		
		TLorentzVector my_p4_pipl_th;
		Int_t pipl_thr_reconindex=-1234; //Index in reconstruction ordering
		TLorentzVector my_p4_pim_th;
		Int_t pim_thr_reconindex=-1234; //Index in reconstruction ordering
		TLorentzVector my_p4_proton_th;
		Int_t proton_thr_reconindex=-1234; //Index in reconstruction ordering
		TLorentzVector my_p4_gam_first_th; //Naming to emphasize that numbering may be backwards from what I call "gam1" and "gam2" later
		Int_t gam_thr_first_reconindex=-1234; //Index in reconstruction ordering
		TLorentzVector my_p4_gam_second_th; //Naming to emphasize that numbering may be backwards from what I call "gam1" and "gam2" later
		Int_t gam_thr_second_reconindex=-1234; //Index in reconstruction ordering
		Int_t my_match_id=-1234;
		
		Double_t gamma_first_FOM = -100;
		Double_t gamma_second_FOM = -100;
		
		if(jz_get_thrown_info) {
			for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)
			{
				dThrownWrapper->Set_ArrayIndex(loc_i);
				
				my_match_id = dThrownWrapper->Get_MatchID();
				
				// i = 0 pi+
				// i = 1 pi-
				// i = 2 proton
				// i = 3 gamma1
				// i = 4 gamma2
				// i = 5 pi0
				if(dThrownWrapper->Get_PID()==PiPlus) my_p4_pipl_th  = dThrownWrapper->Get_P4();
				if(dThrownWrapper->Get_PID()==PiMinus) my_p4_pim_th  = dThrownWrapper->Get_P4();
				if(dThrownWrapper->Get_PID()==Proton) my_p4_proton_th  = dThrownWrapper->Get_P4();
				if(dThrownWrapper->Get_PID()==Gamma) my_p4_gam_first_th  = dThrownWrapper->Get_P4();
				if(loc_i==4&&dThrownWrapper->Get_PID()==Gamma) my_p4_gam_second_th  = dThrownWrapper->Get_P4();
				
				if(loc_i==0&&dThrownWrapper->Get_PID()==PiPlus) pipl_thr_reconindex  = my_match_id;
				if(loc_i==1&&dThrownWrapper->Get_PID()==PiMinus) pim_thr_reconindex  =  my_match_id;
				if(loc_i==2&&dThrownWrapper->Get_PID()==Proton) proton_thr_reconindex  =  my_match_id;
				if(loc_i==3&&dThrownWrapper->Get_PID()==Gamma) gam_thr_first_reconindex  =  my_match_id;
				if(loc_i==4&&dThrownWrapper->Get_PID()==Gamma) gam_thr_second_reconindex  =  my_match_id;
			
				if(loc_i==3&&dThrownWrapper->Get_PID()==Gamma) gamma_first_FOM  =  dThrownWrapper->Get_MatchFOM();
				if(loc_i==4&&dThrownWrapper->Get_PID()==Gamma) gamma_second_FOM  =  dThrownWrapper->Get_MatchFOM();
			}
		}
		
		Double_t angle1 = (my_p4_gam_first_th.Vect()).Angle((locPhotonP4.Vect()))*180./3.14159;
		Double_t angle2 = (my_p4_gam_second_th.Vect()).Angle((locPhotonP4.Vect()))*180./3.14159;
		Double_t thrown_pi0_opangle = (my_p4_gam_second_th.Vect()).Angle((my_p4_gam_first_th.Vect()))*180./3.14159;
		
		
		
		
		TLorentzVector my_p4_gam1_th = my_p4_gam_first_th; //indexing that actually matches reconstructed
		TLorentzVector my_p4_gam2_th = my_p4_gam_second_th; //indexing that actually matches reconstructed
		Double_t gam1_FOM = gamma_first_FOM;
		Double_t gam2_FOM = gamma_second_FOM;
		Int_t gam1_index = gam_thr_first_reconindex;
		Int_t gam2_index = gam_thr_second_reconindex;
		Double_t gam1_ThrRecon_opangle = -1;
		Double_t gam2_ThrRecon_opangle = angle2;
		
		//Thrown and reconstructed indexes are backwards, so switch to match recon index
		////Normal ordering: angle2 would be smaller because it's right
		if(angle1<angle2) {
			// cout << "Flipping..." << endl;
			my_p4_gam1_th = my_p4_gam_second_th;
			my_p4_gam2_th = my_p4_gam_first_th;
			gam1_FOM = gamma_second_FOM;
			gam2_FOM = gamma_first_FOM;
			gam1_index = gam_thr_second_reconindex;
			gam2_index = gam_thr_first_reconindex;
			gam2_ThrRecon_opangle = angle1;
		}
		// cout << "Gamma 1 thrown phi: " << my_p4_gam_first_th.Phi()*180/3.1415 << endl;
		// cout << "Gamma 1 thrown theta: " << my_p4_gam_first_th.Theta()*180/3.1415 << endl;
		// cout << "Gamma 1 thrown E: " << my_p4_gam_first_th.E() << endl;
		// cout << "Gamma 2 thrown phi: " << my_p4_gam_second_th.Phi()*180/3.1415 << endl;
		// cout << "Gamma 2 thrown theta: " << my_p4_gam_second_th.Theta()*180/3.1415 << endl;
		// cout << "Gamma 2 thrown E: " << my_p4_gam_second_th.E() << endl;
		// cout << "Gamma recon 2 phi: " << locPhotonP4.Phi()*180/3.1415 << endl;
		// cout << "Gamma recon 2 theta: " << locPhotonP4.Theta()*180/3.1415 << endl;
		// cout << "Gamma recon 2 E: " << locPhotonP4.E() << endl;
		// if(gam2_index>=0) dNeutralHypoWrapper->Set_ArrayIndex(gam2_index);	
		// cout << "Gamma2 matching 2 phi: " << dNeutralHypoWrapper->Get_P4_Measured().Phi()*180/3.1415 << endl;
		// cout << "Gamma2 matching 2 theta: " << dNeutralHypoWrapper->Get_P4_Measured().Theta()*180/3.1415 << endl;
		// cout << "Gamma2 matching 2 E: " << dNeutralHypoWrapper->Get_P4_Measured().E() << endl;
		// cout << "Angle 1: " << angle1 << endl;
		// cout << "Angle 2: " << angle2 << endl;
		// if(FCAL_counter==1&&BCAL_counter==0) cout << "ONE CANDIDATE FOUND AND ANGLE IS: " << gam1_ThrRecon_opangle << endl;
		// cout << endl;
		
		

		
		//Important note!!! One shower will correspond to Gamma2, which is already used
		//When loop reaches this point, loc_i and candidate counter get one off from each other (and that's fine)
		//Also since I'm saving for up to 15 candidates, I need to loop 16 times to account for the already used photon
		for(UInt_t loc_i = 0; loc_i < jz_NumUnusedCombosSaved+1; ++loc_i)
		{
				
				
			//Clobber arrays with nonsense if beyond number of actual showers
			if(loc_i >= Get_NumNeutralHypos()) {
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Px",-100.,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Py",-100.,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Pz",-100.,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_E" ,-100. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_Angle" ,-100. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_DelPhi" ,-1000. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_DelTheta" ,-1000. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Int_t>("MissingCan_ISFCAL", -100., candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Int_t>("CandidateCounter", candidate_counter, candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("threepi_can_mass" ,-100. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("twogamma_can_mass" ,-100. ,candidate_counter);
				if(jz_get_thrown_info) 	dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_ThrOpangle" ,-100. ,candidate_counter);
				candidate_counter++;
				continue;
			}
			
			//Set branch array indices corresponding to this particle, if applicable
			if(loc_i < Get_NumNeutralHypos()) dNeutralHypoWrapper->Set_ArrayIndex(loc_i);
			
			if( fabs(dNeutralHypoWrapper->Get_P4_Measured().E()-locPhotonP4_Measured.E())<0.00001 ) {
				// cout << "Shower belongs to reconstructed from start one. Skipping." << endl;
				continue;
			}

			Double_t Missing_Thrown_opangle = -1234;
			if(jz_get_thrown_info) Missing_Thrown_opangle = (my_p4_gam1_th.Vect()).Angle((dNeutralHypoWrapper->Get_P4_Measured().Vect()))*180./3.14159;
			
			Double_t threepi_mass = (dNeutralHypoWrapper->Get_P4_Measured()+locPiMinusP4+locPiPlusP4+locPhotonP4).M();
			Double_t twogamma_mass = (dNeutralHypoWrapper->Get_P4_Measured()+locPhotonP4).M();
			
			Int_t shower_system = -1;
			if(dNeutralHypoWrapper->Get_Energy_BCAL()>0.001  ) {
				shower_system=0;
				BCAL_counter++;
			}
			if(dNeutralHypoWrapper->Get_Energy_FCAL()>0.001  ) {
				shower_system=1;
				FCAL_counter++;
			}
			if(dNeutralHypoWrapper->Get_Energy_BCAL()>0.001 && dNeutralHypoWrapper->Get_Energy_FCAL()>0.001  ) {
				cout << "Error!!! multiple detectors have energy deposition for this hypothesis!!!" << endl;
				continue;
			}
			
			if(dNeutralHypoWrapper->Get_Energy_FCAL()>0.001 && dNeutralHypoWrapper->Get_P4_Measured().Theta()*180./3.1415 > 15) {
				cout << "Warning!!! Says it's in the FCAL but theta is > 15 degrees!!!" << endl;
			}
			
			
			Double_t my_DelPhi = (dNeutralHypoWrapper->Get_P4_Measured().Phi()-locMissingPhotonP4.Phi())*180./3.1415;
			if(my_DelPhi<-180) my_DelPhi+=180.;
			if(my_DelPhi>180)  my_DelPhi-=180.;
			
			Double_t my_DelTheta = (dNeutralHypoWrapper->Get_P4_Measured().Theta()-locMissingPhotonP4.Theta())*180./3.1415;
			// if(my_DelTheta<-180) my_DelPhi+=180.;
			// if(my_DelTheta>180)  my_DelPhi-=180.;
			
			//Candidate is reconstructed as different photon. Save, unless studying theta cut.
            if(dNeutralHypoWrapper->Get_P4_Measured().Theta()*180./3.14159>jz_UnusedPhotonThetaCut) {
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Px",dNeutralHypoWrapper->Get_P4_Measured().Px(),candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Py",dNeutralHypoWrapper->Get_P4_Measured().Py(),candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Pz",dNeutralHypoWrapper->Get_P4_Measured().Pz(),candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_E" ,dNeutralHypoWrapper->Get_P4_Measured().E() ,candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_Angle" ,dNeutralHypoWrapper->Get_P4_Measured().Vect().Angle(locMissingPhotonP4.Vect())*180./3.1415   ,candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_DelPhi" ,my_DelPhi   ,candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_DelTheta" ,my_DelTheta   ,candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Int_t>("MissingCan_ISFCAL", shower_system, candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Int_t>("CandidateCounter", candidate_counter, candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("threepi_can_mass" ,threepi_mass ,candidate_counter);
                dFlatTreeInterface->Fill_Fundamental<Double_t>("twogamma_can_mass" ,(dNeutralHypoWrapper->Get_P4_Measured()+locPhotonP4).M() ,candidate_counter);
                if(jz_get_thrown_info) dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_ThrOpangle" ,Missing_Thrown_opangle ,candidate_counter);
			}
			//For studies where we reject candidate showers below theta cut, don't save photon info.
            else {
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Px",-100.,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Py",-100.,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_Pz",-100.,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_E" ,-100. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_Angle" ,-100. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_DelPhi" ,-1000. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingFound_DelTheta" ,-1000. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Int_t>("MissingCan_ISFCAL", -100., candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Int_t>("CandidateCounter", candidate_counter, candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("threepi_can_mass" ,-100. ,candidate_counter);
				dFlatTreeInterface->Fill_Fundamental<Double_t>("twogamma_can_mass" ,-100. ,candidate_counter);
				if(jz_get_thrown_info) 	dFlatTreeInterface->Fill_Fundamental<Double_t>("MissingCan_ThrOpangle" ,-100. ,candidate_counter);
                }
			
			candidate_counter++;
			curr_missing_can_p4 = dNeutralHypoWrapper->Get_P4_Measured();
			
		}
		// cout << endl;
		dFlatTreeInterface->Fill_Fundamental<UInt_t>("NMissingCandidates",FCAL_counter+BCAL_counter);
		dFlatTreeInterface->Fill_Fundamental<UInt_t>("NFCALCandidates",FCAL_counter);
		dFlatTreeInterface->Fill_Fundamental<UInt_t>("NBCALCandidates",BCAL_counter);
		
		if(jz_get_thrown_info) {
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_px_th",my_p4_pipl_th.Px());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_py_th",my_p4_pipl_th.Py());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_pz_th",my_p4_pipl_th.Pz());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiPlus_E_th",my_p4_pipl_th.E());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_px_th",my_p4_pim_th.Px());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_py_th",my_p4_pim_th.Py());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_pz_th",my_p4_pim_th.Pz());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("PiMinus_E_th",my_p4_pim_th.E());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_px_th",my_p4_proton_th.Px());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_py_th",my_p4_proton_th.Py());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_pz_th",my_p4_proton_th.Pz());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Proton_E_th",my_p4_proton_th.E());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_px_th",my_p4_gam1_th.Px());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_py_th",my_p4_gam1_th.Py());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_pz_th",my_p4_gam1_th.Pz());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_E_th",my_p4_gam1_th.E());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_px_th",my_p4_gam2_th.Px());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_py_th",my_p4_gam2_th.Py());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_pz_th",my_p4_gam2_th.Pz());
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_E_th",my_p4_gam2_th.E());
			
			
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma1_FOM",gam1_FOM);
			dFlatTreeInterface->Fill_Fundamental<Double_t>("Gamma2_FOM",gam2_FOM);
			
			Double_t DeltaPhi_rad1 = my_p4_gam1_th.Phi() - dNeutralHypoWrapper->Get_P4_Measured().Phi();
			if(DeltaPhi_rad1<-3.14159) DeltaPhi_rad1+=3.14159;
			if(DeltaPhi_rad1>3.14159) DeltaPhi_rad1-=3.14159;
			Double_t DeltaTheta_rad2 = my_p4_gam2_th.Theta() - dNeutralHypoWrapper->Get_P4_Measured().Theta();
			if(DeltaTheta_rad2<-3.14159) DeltaTheta_rad2+=3.14159;
			if(DeltaTheta_rad2>3.14159) DeltaTheta_rad2-=3.14159;
			
			
			
			dFlatTreeInterface->Fill_Fundamental<Double_t>("two_gamma_opangle_thr",thrown_pi0_opangle);
			dFlatTreeInterface->Fill_Fundamental<Double_t>("gam2_ThrRecon_opangle",gam2_ThrRecon_opangle);
		}		
		
		//FILL FLAT TREE
		Fill_FlatTree(); //for the active combo
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

void DSelector_FCAL_skimmer::Finalize(void)
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
