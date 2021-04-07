#include "DSelector_selectTopology.h"

void DSelector_selectTopology::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "selectTopology.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = ""; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = ""; //if blank, default name will be chosen
	//dSaveDefaultFlatBranches = true; // False: don't save default branches, reduce disk footprint.

	//Because this function gets called for each TTree in the TChain, we must be careful:
		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms
	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree
	//gDirectory now points to the output file with name dOutputFileName (if any)
	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	Get_ComboWrappers();
	dPreviousRunNumber = 0;

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHistMMOP_phi = new TH1F("MMOP_phi", "; Missing Mass off Proton (GeV)", 100, 0.5, 1.5);
	dHistMMOP_nonphi = new TH1F("MMOP_nonphi", "; Missing Mass off Proton (GeV)", 100, 0.5, 1.5);
	dHistMass2K_phi = new TH1F("Mass2K_phi", "; KK Mass (GeV)", 100, 0.5, 1.5);
        dHistMass2K_nonphi = new TH1F("Mass2K_nonphi", "; KK Mass (GeV)", 100, 0.5, 1.5);

	dIsMC = (dTreeInterface->Get_Branch("MCWeight") != NULL);

}

Bool_t DSelector_selectTopology::Process(Long64_t locEntry)
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

	/************************************************* PARSE THROWN TOPOLOGY ***************************************/
	TString locThrownTopology = Get_ThrownTopologyString();
  	//cout<<locThrownTopology.Data()<<endl;

  	// skip exclusive phi -> K+K- events from bggen events
  	bool isThrownTopology = false;
  	if(locThrownTopology.Contains("K^{#plus}K^{#minus}p[#phi]"))
		isThrownTopology = true;
		//return kFALSE;

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
		Int_t locKMinusTrackID = dKMinusWrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locKMinusP4 = dKMinusWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();
		TLorentzVector locMissingKPlusP4 = dMissingKPlusWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKMinusP4_Measured = dKMinusWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locKMinusP4_Measured + locProtonP4_Measured;

		TLorentzVector locP4_mmop = locBeamP4_Measured + dTargetP4;
	        locP4_mmop -= locProtonP4_Measured;
      		double loc_mmop = locP4_mmop.M();

		if(isThrownTopology) dHistMMOP_phi->Fill(loc_mmop);
                else dHistMMOP_nonphi->Fill(loc_mmop);

		Particle_t KMissID = KPlus;
		for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)
		{
	  		//Set branch array indices corresponding to this particle
	  		dChargedHypoWrapper->Set_ArrayIndex(loc_i);
	
			if (dChargedHypoWrapper->Get_PID() != KMissID)
			    	continue;
	  		if (dChargedHypoWrapper->Get_TrackID() == locKMinusTrackID)
	    			continue;
			if (dChargedHypoWrapper->Get_TrackID() == locProtonTrackID)
	    			continue;

			TLorentzVector locKPlusMissP4_reco = dChargedHypoWrapper->Get_P4_Measured();
			TLorentzVector loc2KP4_reco = locKMinusP4 + locKPlusMissP4_reco;
			double loc2KMass_reco = loc2KP4_reco.M();

			if(isThrownTopology) dHistMass2K_phi->Fill(loc2KMass_reco);
	                else dHistMass2K_nonphi->Fill(loc2KMass_reco);
		}

	} // end of combo loop

	return kTRUE;
}

void DSelector_selectTopology::Finalize(void)
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
