#include "DSelector_gg.h"

void DSelector_gg::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "hist_gg.root"; //"" for none
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

	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	// timing distribution to see prompt and accidental peaks in DeltaT distribution
	dHist_BeamDeltaT = new TH1F("BeamDeltaT", "; t_{Tagger} - t_{RF} (ns)", 200, -20., 20.);

	// IMPORTANT: For weighted histograms DON'T USE TH1I!  Instead use TH1F or TH1D to store non-integer weights
	dHist_MissingMassSquared_weighted = new TH1F("MissingMassSquared_weighted", "; Missing Mass Squared (GeV/c^{2})^{2}", 300, -0.1, 0.1);
	dHist_MissingMassSquared_prompt = new TH1F("MissingMassSquared_prompt", "; Missing Mass Squared (GeV/c^{2})^{2}", 300, -0.1, 0.1);
	dHist_MissingMassSquared_accid = new TH1F("MissingMassSquared_accid", "; Missing Mass Squared (GeV/c^{2})^{2}", 300, -0.1, 0.1);
	
	// IMPORTANT: Need to set Sumw2 to use weight squared in histogram for subtraction later.  Can either do this here at histogram definition, or in macro for plotting.
	dHist_MissingMassSquared_prompt->Sumw2();
	dHist_MissingMassSquared_accid->Sumw2();
}

Bool_t DSelector_gg::Process(Long64_t locEntry)
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

	/******************************************** GET POLARIZATION ORIENTATION ******************************************/


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

	//EXAMPLE: Combo-specific info:
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMassSquared;

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
		Int_t locPhoton1NeutralID = dPhoton1Wrapper->Get_NeutralID();
		Int_t locPhoton2NeutralID = dPhoton2Wrapper->Get_NeutralID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locPhoton1P4 = dPhoton1Wrapper->Get_P4();
		TLorentzVector locPhoton2P4 = dPhoton2Wrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locPhoton1P4_Measured = dPhoton1Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton2P4_Measured = dPhoton2Wrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/
		TLorentzVector loc_MissingP4_Measured = locBeamP4_Measured + dTargetP4;
		loc_MissingP4_Measured -= (locProtonP4_Measured + locPhoton1P4_Measured + locPhoton2P4_Measured);

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		/******************************************** ACCIDENTAL SUBRACTION INFO *******************************************/
		
		// measured tagger time for combo
		TLorentzVector locBeam_X4_Measured = dComboBeamWrapper->Get_X4_Measured(); 

		// measured RF time for combo
		double locRFTime = dComboWrapper->Get_RFTime_Measured(); 

		// time difference between tagger and RF (corrected for production vertex position relative to target center)
		double locBeamDeltaT = locBeam_X4_Measured.T() - (locRFTime + (locBeam_X4_Measured.Z() - dTargetCenter.Z())/29.9792458); 
		dHist_BeamDeltaT->Fill(locBeamDeltaT);

		// calculate accidental subtraction weight based on time difference 
		double locWeight = 0.; // weight to accidentally subtracted histgorams
		bool locAccid = false; // flag to fill separate prompt and accidental histograms for later subtraction

		if(fabs(locBeamDeltaT) < 0.5*4.008) { // prompt signal recieves a weight of 1
			locWeight = 1.;
			locAccid = false;
		}
                else { // accidentals recieve a weight of 1/# RF bunches included in TTree (6 in this case)
			locWeight = -1./6.;
			locAccid = true;
		}

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS ************************************/

		//Missing Mass Squared
		double locMissingMassSquared = loc_MissingP4_Measured.M2();

		//Motivation for uniqueness tracking
		//  * Since all 4 measured particles enter in the calculation of the MM^2 they should be tracked uniquely
		//  * All beam photons are uniquely tracked, so both prompt and accidental photons are used to fill the histograms below (as described in Richard's slides).
		
		//Uniqueness tracking: Build the map of particles used for the missing mass
			//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMassSquared;
		locUsedThisCombo_MissingMassSquared[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMassSquared[Gamma].insert(locPhoton1NeutralID);
		locUsedThisCombo_MissingMassSquared[Gamma].insert(locPhoton2NeutralID);
		locUsedThisCombo_MissingMassSquared[Proton].insert(locProtonTrackID);
		
		//compare to what's been used so far
		if(locUsedSoFar_MissingMassSquared.find(locUsedThisCombo_MissingMassSquared) == locUsedSoFar_MissingMassSquared.end())
		{
			// fill weighted histogram with ALL beam photons! (resulting histogram is accidental subtracted)
			dHist_MissingMassSquared_weighted->Fill(locMissingMassSquared, locWeight);

			// fill separate histograms for subtracting later
			if(locAccid) dHist_MissingMassSquared_accid->Fill(locMissingMassSquared);
			else dHist_MissingMassSquared_prompt->Fill(locMissingMassSquared);
			
			// register unique combo
			locUsedSoFar_MissingMassSquared.insert(locUsedThisCombo_MissingMassSquared);
		}
	} // end of combo loop

	//FILL HISTOGRAMS: Num combos / events surviving actions
	Fill_NumCombosSurvivedHists();

	return kTRUE;
}

void DSelector_gg::Finalize(void)
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
