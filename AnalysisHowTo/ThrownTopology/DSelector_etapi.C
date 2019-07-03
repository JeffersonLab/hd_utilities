#include "DSelector_etapi.h"

void DSelector_etapi::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "etapi.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = ""; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = ""; //if blank, default name will be chosen

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

	// EXAMPLE: Create deque for histogramming particle masses:
	// // For histogramming the phi mass in phi -> K+ K-
	// // Be sure to change this and dAnalyzeCutActions to match reaction
	std::deque<Particle_t> MyPi0;
	MyPi0.push_back(Gamma); MyPi0.push_back(Gamma); 
	std::deque<Particle_t> MyEta;
	MyEta.push_back(Gamma); MyEta.push_back(Gamma);
	std::deque<Particle_t> MyPi0Eta;
	MyPi0Eta.push_back(Pi0); MyPi0Eta.push_back(Eta); 

	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data

	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));

	//MASSES
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 1, MyPi0, 1000, 0.07, 0.2, "Pi0"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 2, MyEta, 1000, 0.3, 0.8, "Eta"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPi0Eta, 1000, 0.5, 2.0, "Pi0Eta"));
	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));
	dAnalysisActions.push_back(new DCutAction_KinFitFOM(dComboWrapper, 0.001));

	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 1, MyPi0, 1000, 0.07, 0.2, "Pi0KinFitCut_Measured"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 2, MyEta, 1000, 0.3, 0.8, "EtaKinFitCut_Measured"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPi0Eta, 1000, 0.5, 2.0, "Pi0EtaKinFitCut_Measured"));

	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 1, MyPi0, 1000, 0.07, 0.2, "Pi0KinFitCut_KinFit"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 2, MyEta, 1000, 0.3, 0.8, "EtaKinFitCut_KinFit"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 0, MyPi0Eta, 1000, 0.5, 2.0, "Pi0EtaKinFitCut_KinFit"));

	dAnalysisActions.push_back(new DCutAction_InvariantMass(dComboWrapper, true, 1, MyPi0, 0.11, 0.16,"Pi0MassCut"));
	dAnalysisActions.push_back(new DCutAction_InvariantMass(dComboWrapper, true, 2, MyEta, 0.45, 0.65,"EtaMassCut"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 0, MyPi0Eta, 1000, 0.5, 2.0, "Pi0EtaMassCut_KinFit"));

	//KINEMATICS
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();

	dHistThrownTopologies = new TH1F("hThrownTopologies","hThrownTopologies", 10, -0.5, 9.5);

	vector<TString> locThrownTopologies;
	locThrownTopologies.push_back("4#gammap[#pi^{0},#eta]");
	locThrownTopologies.push_back("6#gammap[3#pi^{0}]");		
	locThrownTopologies.push_back("5#gammap[2#pi^{0},#omega]");
	locThrownTopologies.push_back("6#gammap[2#pi^{0},#eta]");
	locThrownTopologies.push_back("4#gammap[2#pi^{0}]");
	locThrownTopologies.push_back("8#gammap[4#pi^{0},#eta]");
	locThrownTopologies.push_back("6#gamma#pi^{#plus}#pi^{#minus}p[3#pi^{0}]");		
	locThrownTopologies.push_back("4#gamma#pi^{#plus}#pi^{#minus}p[2#pi^{0}]");
	locThrownTopologies.push_back("4#gamma#pi^{#plus}#pi^{#minus}p[#pi^{0},#eta]");		
	locThrownTopologies.push_back("8#gammap[3#pi^{0},#eta]");
	locThrownTopologies.push_back("3#gammap[#pi^{0},#omega]");
	for(uint i=0; i<locThrownTopologies.size(); i++) {
		dHistInvariantMass_ThrownTopology[locThrownTopologies[i]] = new TH1I(Form("hInvariantMass_ThrownTopology_%d", i),Form("Invariant Mass Topology: %s", locThrownTopologies[i].Data()), 1000, 0.5, 2.0);
	}
}

Bool_t DSelector_etapi::Process(Long64_t locEntry)
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

	/************************************************* PARSE THROWN TOPOLOGY ***************************************/
	TString locThrownTopology = Get_ThrownTopologyString();

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
		Int_t locPhoton1NeutralID = dPhoton1Wrapper->Get_NeutralID();
		Int_t locPhoton2NeutralID = dPhoton2Wrapper->Get_NeutralID();

		//Step 2
		Int_t locPhoton3NeutralID = dPhoton3Wrapper->Get_NeutralID();
		Int_t locPhoton4NeutralID = dPhoton4Wrapper->Get_NeutralID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();
		//Step 1
		TLorentzVector locPhoton1P4 = dPhoton1Wrapper->Get_P4();
		TLorentzVector locPhoton2P4 = dPhoton2Wrapper->Get_P4();
		//Step 2
		TLorentzVector locPhoton3P4 = dPhoton3Wrapper->Get_P4();
		TLorentzVector locPhoton4P4 = dPhoton4Wrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPhoton1P4_Measured = dPhoton1Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton2P4_Measured = dPhoton2Wrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPhoton3P4_Measured = dPhoton3Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton4P4_Measured = dPhoton4Wrapper->Get_P4_Measured();

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locProtonP4_Measured + locPhoton1P4_Measured + locPhoton2P4_Measured + locPhoton3P4_Measured + locPhoton4P4_Measured;

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		// Fill histogram of thrown topologies
		dHistThrownTopologies->Fill(locThrownTopology.Data(),1);

		TLorentzVector locPi0EtaP4 = locPhoton1P4 + locPhoton2P4 + locPhoton3P4 + locPhoton4P4;
		if(dHistInvariantMass_ThrownTopology.find(locThrownTopology) != dHistInvariantMass_ThrownTopology.end())
			dHistInvariantMass_ThrownTopology[locThrownTopology]->Fill(locPi0EtaP4.M());

		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/

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

void DSelector_etapi::Finalize(void)
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
