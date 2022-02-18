#include "DSelector_kpkm.h"

void DSelector_kpkm::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "kpkm.root"; //"" for none
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
	std::deque<Particle_t> MyPhi;
	MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);

	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data

	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));
	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.2, KPlus, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.2, KMinus, SYS_TOF));

	//MASSES
	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));
	dAnalysisActions.push_back(new DCutAction_KinFitFOM(dComboWrapper, 0.001));

	//CUT MISSING MASS
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.02, 0.02));

	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPhi, 500, 0.3, 1.5, "Phi"));

	//BEAM ENERGY
	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));

	//KINEMATICS
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 600, -0.06, 0.06);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 600, 0.0, 12.0);

	//DIRC HISTOGRAMS
	dHist_KPlusDIRCXY = new TH2F("KPlusDIRCXY", "; X (cm); Y (cm)", 300, -150, 150, 300, -150, 150);
	dHist_KMinusDIRCXY = new TH2F("KMinusDIRCXY", "; X (cm); Y (cm)", 300, -150, 150, 300, -150, 150);
	dHist_KPlusDIRCThetaCVsP = new TH2F("KPlusDIRCThetaCVsP", "; P (GeV); #theta_{C}", 300, 0., 10., 300, 0., 60.);
	dHist_KMinusDIRCThetaCVsP = new TH2F("KMinusDIRCThetaCVsP", "; P (GeV); #theta_{C}", 300, 0., 10., 300, 0., 60.);
	dHist_Ldiff = new TH2F("Ldiff", "; Minus L_{#pi} - L_{K} ; Plus L_{#pi} - L_{K}", 200, -200, 200, 200, -200, 200);
	
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

		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/
		
		dHist_BeamEnergy->Fill(locBeamP4.E());

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();
		dHist_MissingMassSquared->Fill(locMissingMassSquared);

		/************************************ EXAMPLE: DIRC HISTOGRAMS ************************************/
		// location on DIRC XY plane
		double locKPlusDIRCX = dKPlusWrapper->Get_Track_ExtrapolatedX_DIRC();
		double locKPlusDIRCY = dKPlusWrapper->Get_Track_ExtrapolatedY_DIRC();
		double locKMinusDIRCX = dKMinusWrapper->Get_Track_ExtrapolatedX_DIRC();
		double locKMinusDIRCY = dKMinusWrapper->Get_Track_ExtrapolatedY_DIRC();
		dHist_KPlusDIRCXY->Fill(locKPlusDIRCX, locKPlusDIRCY);
		dHist_KMinusDIRCXY->Fill(locKMinusDIRCX, locKMinusDIRCY);

		// DIRC performance variables (likelihood, theta_C), see code for more
		// gluex_root_analysis/libraries/DSelector/DChargedTrackHypothesis.h
		int locKPlusNumPhotons_DIRC = dKPlusWrapper->Get_Track_NumPhotons_DIRC();
		int locKMinusNumPhotons_DIRC = dKMinusWrapper->Get_Track_NumPhotons_DIRC();

		// require a minimum number of detected photons
		if(locKPlusNumPhotons_DIRC < 5 || locKMinusNumPhotons_DIRC < 5) 
			continue;

		double locKPlusThetaC_DIRC = dKPlusWrapper->Get_Track_ThetaC_DIRC()*TMath::RadToDeg();
		double locKMinusThetaC_DIRC = dKMinusWrapper->Get_Track_ThetaC_DIRC()*TMath::RadToDeg();
		double locKPlusP = locKPlusP4.Vect().Mag();
		double locKMinusP = locKMinusP4.Vect().Mag();
		dHist_KPlusDIRCThetaCVsP->Fill(locKPlusP, locKPlusThetaC_DIRC);
		dHist_KMinusDIRCThetaCVsP->Fill(locKMinusP, locKMinusThetaC_DIRC);

		double locKPlusLdiff_DIRC = dKPlusWrapper->Get_Track_Lpi_DIRC() - dKPlusWrapper->Get_Track_Lk_DIRC();
		double locKMinusLdiff_DIRC = dKMinusWrapper->Get_Track_Lpi_DIRC() - dKMinusWrapper->Get_Track_Lk_DIRC();
		
		dHist_Ldiff->Fill(locKPlusLdiff_DIRC, locKMinusLdiff_DIRC);

	} // end of combo loop

	//FILL HISTOGRAMS: Num combos / events surviving actions
	Fill_NumCombosSurvivedHists();

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
