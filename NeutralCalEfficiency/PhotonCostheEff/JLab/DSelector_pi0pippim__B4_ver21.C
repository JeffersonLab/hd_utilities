#include "DSelector_pi0pippim__B4_ver21.h"

void DSelector_pi0pippim__B4_ver21::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).


         cout << "Init: Begin initialization" << endl;

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "DSelector_pi0pippim__B4.root"; //"" for none
	dOutputTreeFileName = "tree_DSelector_pi0pippim__B4.root"; //"" for none
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



        cout << "Init: User Initialization" << endl;
	// EXAMPLE: Create deque for histogramming particle masses:
	// // For histogramming the phi mass in phi -> K+ K-
	// // Be sure to change this and dAnalyzeCutActions to match reaction
	// std::deque<Particle_t> MyPhi;
	// MyPhi.push_back(KPlus); MyPhi.push_back(KMinus);

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

	// ANALYZE CUT ACTIONS
	// // Change MyPhi to match reaction
	// dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 0, MyPhi, 1000, 0.9, 2.4, "CutActionEffect" );

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();
	// dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

        cout << "Init: Define Histograms" << endl;
	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 200, -0.1, 0.1);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 600, 0.0, 12.0);

	dH1_CL = new TH1I("CL", ";Combo CL", 100, 0.0, 1.0);
	dH1_Chi2NDF = new TH1I("Chi2NDF", ";Combo Chi2/NDF", 100, 0.0, 20.0);
	dH2_Chi2_vs_Pi0Theta = new TH2I("Chi2_vs_Pi0Theta", ";Chi2_vs_Pi0Theta", 120, 0.0, 40.0, 100, 0.0, 20.0);

	dH1_ProtonP = new TH1I("ProtonP", ";Proton KIN P (GeV)", 100, 0.0, 10.0);
	dH1_PiPlusP = new TH1I("PiPlusP", ";PiPlus KIN  P (GeV)", 100, 0.0, 10.0);
	dH1_PiMinusP = new TH1I("PiMinusP", ";PiMinus KIN  P (GeV)", 100, 0.0, 10.0);
	dH1_Pi0P = new TH1I("Pi0P", ";Pi0 P (GeV)", 100, 0.0, 10.0);
	dH1_Photon1P = new TH1I("Photon1P", ";Photon 1 KIN P  (GeV)", 100, 0.0, 10.0);
	dH1_Photon2P = new TH1I("Photon2P", ";Photon 2 KIN  P (GeV)", 100, 0.0, 10.0);
	dH1_Pi0MeasuredP = new TH1I("Pi0MeasuredP", ";Pi0 MEAS P (GeV)", 100, 0.0, 10.0);
	dH1_Pi0MeasuredMass = new TH1I("Pi0MeasuredMass", ";Pi0 MEAS Mass (GeV)", 100, 0.0, 0.4);
	dH1_Mass3pi = new TH1I("Mass3pi", ";3pi Mass (GeV)", 100, 0.5, 1.5);

	dH1_CostheStarB_BCAL = new TH1I("CostheStarB_BCAL", ";Costhe^{*B} BCAL", 150, -1.5,1.5);
	dH1_CostheStarB_FCAL = new TH1I("CostheStarB_FCAL", ";Costhe^{*B} FCAL", 150, -1.5,1.5);
	dH1_CostheStarB_FCALBCAL = new TH1I("CostheStarB_FCALBCAL", ";Costhe^{*B} BCAL/FCAL", 150, -1.5,1.5);
	dH1_CostheStarA_BCAL = new TH1I("CostheStarA_BCAL", ";Costhe^{*A} BCAL", 150, -1.5,1.5);
	dH1_CostheStarA_FCAL = new TH1I("CostheStarA_FCAL", ";Costhe^{*A} FCAL", 150, -1.5,1.5);
	dH1_CostheStarA_FCALBCAL = new TH1I("CostheStarA_FCALBCAL", ";Costhe^{*A} BCAL/FCAL", 150, -1.5,1.5);
	dH1_CostheStarDiff = new TH1I("CostheStarDiff", ";Costhe^{*A}-Costhe^{*B}", 100, -0.5,0.5);

	// Note: The three histograms below should have the same binning as dH1_CostheStarB_* histograms
	dH1_CostheStarB_BCAL_eff = new TH1F("CostheStarB_BCAL_eff", ";Costhe^{*B} BCAL Eff", 150, -1.5,1.5);
	dH1_CostheStarB_FCAL_eff = new TH1F("CostheStarB_FCAL_eff", ";Costhe^{*B} FCAL Eff", 150, -1.5,1.5);
	dH1_CostheStarB_FCALBCAL_eff = new TH1F("CostheStarB_FCALBCAL_eff", ";Costhe^{*B} BCAL/FCAL Eff", 150, -1.5,1.5);

	dH1_E2_w1_FCAL = new TH1F("E2_w1_FCAL", ";E2 FCAL w=1", 50, 0,2);
	dH1_E2_w1_BCAL = new TH1F("E2_w1_BCAL", ";E2 BCAL w=1", 50, 0,2);
	dH1_E2_w1_FCALBCAL = new TH1F("E2_w1_FCALBCAL", ";E2 FCAL/BCAL w=1", 50, 0,2);

	dH1_E2_weff_FCAL = new TH1F("E2_weff_FCAL", ";E2 FCAL weighted", 50, 0,2);
	dH1_E2_weff_BCAL = new TH1F("E2_weff_BCAL", ";E2 BCAL weighted", 50, 0,2);
	dH1_E2_weff_FCALBCAL = new TH1F("E2_weff_FCALBCAL", ";E2 FCAL/BCAL weighted", 50, 0,2);

	dH1_E2_eff_FCAL = new TH1F("E2_eff_FCAL", ";E2 FCAL Eff", 50, 0,2);
	dH1_E2_eff_FCAL->Sumw2();
	dH1_E2_eff_BCAL = new TH1F("E2_eff_BCAL", ";E2 BCAL Eff", 50, 0,2);
	dH1_E2_eff_BCAL->Sumw2();
	dH1_E2_eff_FCALBCAL = new TH1F("E2_eff_FCALBCAL", ";E2 FCAL/BCAL Eff", 50, 0,2);
	dH1_E2_eff_FCALBCAL->Sumw2();


	dH1_Pi0Theta_BCAL = new TH1I("Pi0Theta_BCAL", ";Pi0Theta BCAL (deg)", 120, 0, 40);
	dH1_Pi0Theta_FCAL = new TH1I("Pi0Theta_FCAL", ";Pi0Theta FCAL (deg)", 120, 0, 40);
	dH1_Pi0Theta_FCALBCAL = new TH1I("Pi0Theta_FCALBCAL", ";Pi0Theta FCAL/BCAL (deg)", 120, 0, 40);

	dH1_PhotonTheta_BCAL = new TH1I("PhotonTheta_BCAL", ";Photon Theta BCAL (deg)", 120, 0, 40);
	dH1_PhotonTheta_FCAL = new TH1I("PhotonTheta_FCAL", ";Photon Theta FCAL (deg)", 120, 0, 40);
	dH1_PhotonTheta_FCALBCAL = new TH1I("PhotonTheta_FCALBCAL", ";Photon Theta FCAL/BCAL (deg)", 120, 0, 40);

	dH2_ProtonP_MeasVsKin = new TH2I("ProtonP_MeasVsKin", ";Proton P MEAS Vs KIN (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_PiPlusP_MeasVsKin = new TH2I("PiPlusP_MeasVsKin", ";PiPlus P MEAS Vs KIN  (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_PiMinusP_MeasVsKin = new TH2I("PiMinusP_MeasVsKin", ";PiMinus P MEAS Vs KIN  (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_Pi0P_MeasVsKin = new TH2I("Pi0P_MeasVsKin", ";Pi0 P (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_Photon1P_MeasVsKin = new TH2I("Photon1P_MeasVsKin", ";Photon 1 P MEAS Vs KIN   (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_Photon2P_MeasVsKin = new TH2I("Photon2P_MeasVsKin", ";Photon 2 P MEAS Vs KIN  (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_Photon2P_vs_Photon1P = new TH2I("Photon2P_vs_Photon1P", ";Photon2 P vs Photon1 P  (GeV)", 100, 0.0, 10.0, 100, 0.0, 10.0);
	dH2_ProtondEdX_vs_P = new TH2I("ProtondEdX_vs_P", ";Proton dEdX vs P ", 100, 0.0, 4.0, 100, 0.0, 10.0);
	dH2_PiPlusdEdX_vs_P = new TH2I("PiPlusdEdX_vs_P", ";Pi+ EdX vs P ", 100, 0.0, 4.0, 100, 0.0, 10.0);
	dH2_PiMinusdEdX_vs_P = new TH2I("PiMinusdEdX_vs_P", ";Pi- EdX vs P ", 100, 0.0, 4.0, 100, 0.0, 10.0);

	// EXAMPLE CUT PARAMETERS:
	fMinProton_dEdx = new TF1("fMinProton_dEdx", "exp(-1.*[0]*x + [1]) + [2]", 0., 10.);
	fMinProton_dEdx->SetParameters(4.0, 2.5, 1.25);
	// fMaxPion_dEdx = new TF1("fMaxPion_dEdx", "exp(-1.*[0]*x + [1]) + [2]", 0., 10.);
	// fMaxPion_dEdx->SetParameters(4.0, 2.0, 2.5);
	fMaxPion_dEdx = new TF1("fMaxPion_dEdx", "[0]*(1-exp(-1.*[1]*x))", 0., 10.);
	fMaxPion_dEdx->SetParameters(4.0, 4.0);

	// dMinKinFitCL = 5.73303e-7; //5.73303e-7;
	dMinKinFitCL = 0.01;
	// dMaxKinFitChiSq = 5.0;
	dMaxKinFitChiSq = 40.0;
	dMinBeamEnergy = 5.5;
	dMaxBeamEnergy = 6.0;
	dMin2piMass = 0.2;
	dMax2piMass = 0.5;
	dMin3piMass = 0.7;
	dMax3piMass = 3.0;
	// dMax3piMass = 0.85;
	dMinMissingMassSquared = -0.02;
	dMaxMissingMassSquared = 0.02;
	Epi0_min=3;
	Epi0_max=5;
        dMinmisE = -1;    // initial values from Qiao
        dMaxmisE = 1;    // initial values from Qiao
        dMaxmisTM = 0.25;    // initial values from Qiao
	dMinGapTheta = 8.;
	dMaxGapTheta = 12.;

	// initialize vector with efficiency parameters
	Veffinfo.clear();     // clear vector to begin
	

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

Bool_t DSelector_pi0pippim__B4_ver21::Process(Long64_t locEntry)
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

  if (locEntry > 1e7) {
    return 0;    // limit number of events to process for debugging.
  }
    else {
      if (locEntry%10000==0) cout << "Processed " << locEntry << " events" << endl;
    }

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
	// dAnalyzeCutActions->Reset_NewEvent(); // manual action, must call Reset_NewEvent()

	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING
		//Sometimes, some content is the exact same between one combo and the next
			//e.g. maybe two combos have different beam particles, but the same data for the final-state
		//When histogramming, you don't want to double-count when this happens: artificially inflates your signal (or background)
		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)
		//Then for each combo, just compare to what you used before, and make sure it's unique

	//EXAMPLE 1: Particle-specific info:
	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search
	set<Int_t> locUsedSoFar_ProtonP;
	set<Int_t> locUsedSoFar_PiPlusP;
	set<Int_t> locUsedSoFar_PiMinusP;
	set<Int_t> locUsedSoFar_Photon1P;
	set<Int_t> locUsedSoFar_Photon2P;
        set<map<Particle_t, set<Int_t> > > locUsedSoFar_Pi0;
        set<map<Particle_t, set<Int_t> > > locUsedSoFar_Pi0Measured;
        set<map<Particle_t, set<Int_t> > > locUsedSoFar_Mass3pi;

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
		Int_t locPiPlusTrackID = dPiPlusWrapper->Get_TrackID();
		Int_t locPiMinusTrackID = dPiMinusWrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		//Step 1
		Int_t locPhoton1NeutralID = dPhoton1Wrapper->Get_NeutralID();
		Int_t locPhoton2NeutralID = dPhoton2Wrapper->Get_NeutralID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured

                double kinfitCL = dComboWrapper->Get_ConfidenceLevel_KinFit("");
                double kinfitChi2NDF = dComboWrapper->Get_ChiSq_KinFit("")/dComboWrapper->Get_NDF_KinFit("");

		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locPiPlusP4 = dPiPlusWrapper->Get_P4();
		TLorentzVector locPiMinusP4 = dPiMinusWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();
		//Step 1
		TLorentzVector locDecayingPi0P4 = dDecayingPi0Wrapper->Get_P4();
		TLorentzVector locPhoton1P4 = dPhoton1Wrapper->Get_P4();
		TLorentzVector locPhoton2P4 = dPhoton2Wrapper->Get_P4();

		TLorentzVector locMissingP4 = locBeamP4 + dTargetP4 -  locPiPlusP4 - locPiMinusP4 - locDecayingPi0P4 - locProtonP4; 
		TLorentzVector loc3piP4 = locPiPlusP4 + locPiMinusP4 + locDecayingPi0P4;
 
		double ProtonP = sqrt(locProtonP4.E()*locProtonP4.E()-locProtonP4.M2());
		double PiPlusP = sqrt(locPiPlusP4.E()*locPiPlusP4.E()-locPiPlusP4.M2());
		double PiMinusP = sqrt(locPiMinusP4.E()*locPiMinusP4.E()-locPiMinusP4.M2());
		double Pi0P = sqrt(locDecayingPi0P4.E()*locDecayingPi0P4.E()-locDecayingPi0P4.M2());

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locPiPlusP4_Measured = dPiPlusWrapper->Get_P4_Measured();
		TLorentzVector locPiMinusP4_Measured = dPiMinusWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPhoton1P4_Measured = dPhoton1Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton2P4_Measured = dPhoton2Wrapper->Get_P4_Measured();
		TLorentzVector locDecayingPi0P4_Measured = locPhoton1P4_Measured + locPhoton2P4_Measured;

                double Ephoton1_BCAL = dPhoton1Wrapper->Get_Energy_BCAL();
                double Ephoton1_FCAL = dPhoton1Wrapper->Get_Energy_FCAL();
                double Ephoton2_BCAL = dPhoton2Wrapper->Get_Energy_BCAL();
                double Ephoton2_FCAL = dPhoton2Wrapper->Get_Energy_FCAL();

		double ProtonP_Measured = sqrt(locProtonP4_Measured.E()*locProtonP4_Measured.E()-locProtonP4_Measured.M2());
		double PiPlusP_Measured = sqrt(locPiPlusP4_Measured.E()*locPiPlusP4_Measured.E()-locPiPlusP4_Measured.M2());
		double PiMinusP_Measured = sqrt(locPiMinusP4_Measured.E()*locPiMinusP4_Measured.E()-locPiMinusP4_Measured.M2());
		double Pi0P_Measured = sqrt(locDecayingPi0P4_Measured.E()*locDecayingPi0P4_Measured.E()-locDecayingPi0P4_Measured.M2());



		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/
		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		TLorentzVector loc3piP4_Measured = locPiPlusP4_Measured + locPiMinusP4_Measured + locPhoton1P4_Measured + locPhoton2P4_Measured;
		locMissingP4_Measured -= loc3piP4_Measured + locProtonP4_Measured ;
		TLorentzVector loc2gammaP4_Measured = locPhoton1P4_Measured + locPhoton2P4_Measured;



		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE



		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		// dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
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

		Double_t CostheStarB, CostheStarA;
		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();
                //Missing Energy
                double misE = locMissingP4_Measured.E();
                //Missing transverse Momentum
                double misTM = locMissingP4_Measured.Perp();
		Double_t Pi0Theta = locDecayingPi0P4.Vect().Theta()*180./3.14159;
		Double_t Photon1Theta = locPhoton1P4.Vect().Theta()*180./3.14159;
		Double_t Photon2Theta = locPhoton2P4.Vect().Theta()*180./3.14159;

		dH1_CL->Fill(kinfitCL);
		dH1_Chi2NDF->Fill(kinfitChi2NDF);
		dH2_Chi2_vs_Pi0Theta->Fill(Pi0Theta,kinfitChi2NDF);

		// Event selection cuts.
		if (kinfitCL < dMinKinFitCL) continue;
		if (kinfitChi2NDF > dMaxKinFitChiSq) continue;       // Chi2 cut
		if (loc3piP4.M()<dMin3piMass || loc3piP4.M()>dMax3piMass) continue; 
		if (locMissingMassSquared<dMinMissingMassSquared || locMissingMassSquared>dMaxMissingMassSquared) continue; 
		if (misE<dMinmisE || misE>dMaxmisE) continue; 
		if (misTM>dMaxmisTM) continue;
		if (locDecayingPi0P4.E() < Epi0_min || locDecayingPi0P4.E() > Epi0_max) continue;  
		// if (Pi0Theta > dMinGapTheta && Pi0Theta < dMaxGapTheta) continue; // angle cut on pi0
		 if ((Photon1Theta > dMinGapTheta && Photon1Theta < dMaxGapTheta) || 
		     (Photon2Theta > dMinGapTheta && Photon2Theta < dMaxGapTheta) ) continue;           // angle cuts on photons


		double locPiPlus_dEdx_CDC = dPiPlusWrapper->Get_dEdx_CDC()*1e6;
		if(locPiPlus_dEdx_CDC > fMaxPion_dEdx->Eval(locPiPlusP4.P())) {
			dComboWrapper->Set_IsComboCut(true);
			continue;
			}
		double locPiMinus_dEdx_CDC = dPiMinusWrapper->Get_dEdx_CDC()*1e6;
		if(locPiMinus_dEdx_CDC > fMaxPion_dEdx->Eval(locPiMinusP4.P())) {
			dComboWrapper->Set_IsComboCut(true);
			continue;
			}

		// Proton CDC dE/dx histogram and cut 
		double locProton_dEdx_CDC = dProtonWrapper->Get_dEdx_CDC()*1e6;
		if(locProton_dEdx_CDC < fMinProton_dEdx->Eval(locProtonP4.P())) {
			dComboWrapper->Set_IsComboCut(true);
			continue;
			}
		dH2_PiPlusdEdX_vs_P->Fill(PiPlusP,locPiPlus_dEdx_CDC); 
		dH2_PiMinusdEdX_vs_P->Fill(PiMinusP,locPiMinus_dEdx_CDC);
		dH2_ProtondEdX_vs_P->Fill(ProtonP,locProton_dEdx_CDC); 

 
	        Double_t Ebar=3.88;      // Average energy of pi0 for range of 3-5 GeV.
		Double_t MPI=locDecayingPi0P4.M();
		Double_t Pbar = sqrt(Ebar*Ebar - MPI*MPI);

		//Histogram beam energy (if haven't already)
		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
		{
			dHist_BeamEnergy->Fill(locBeamP4.E());
			locUsedSoFar_BeamEnergy.insert(locBeamID);
		}
		if(locUsedSoFar_ProtonP.find(locProtonTrackID) == locUsedSoFar_ProtonP.end())
		{
		        dH1_ProtonP->Fill(ProtonP);
		        dH2_ProtonP_MeasVsKin->Fill(ProtonP,ProtonP_Measured);
			locUsedSoFar_ProtonP.insert(locProtonTrackID);
		}
		if(locUsedSoFar_PiPlusP.find(locPiPlusTrackID) == locUsedSoFar_PiPlusP.end())
		{
		        dH1_PiPlusP->Fill(PiPlusP);
		        dH2_PiPlusP_MeasVsKin->Fill(PiPlusP,PiPlusP_Measured);
			locUsedSoFar_PiPlusP.insert(locPiPlusTrackID);
		}
		if(locUsedSoFar_PiMinusP.find(locPiMinusTrackID) == locUsedSoFar_PiMinusP.end())
		{
			dH1_PiMinusP->Fill(PiMinusP);
		        dH2_PiMinusP_MeasVsKin->Fill(PiMinusP,PiMinusP_Measured);
			locUsedSoFar_PiMinusP.insert(locPiMinusTrackID);
		}
		if(locUsedSoFar_Photon1P.find(locPhoton1NeutralID) == locUsedSoFar_Photon1P.end())
		{
			dH1_Photon1P->Fill(locPhoton1P4.E());
		        dH2_Photon1P_MeasVsKin->Fill(locPhoton1P4.E(),locPhoton1P4_Measured.E());
			locUsedSoFar_Photon1P.insert(locPhoton1NeutralID);
		}
		if(locUsedSoFar_Photon2P.find(locPhoton2NeutralID) == locUsedSoFar_Photon2P.end())
		{
			dH1_Photon2P->Fill(locPhoton2P4.E());
		        dH2_Photon2P_MeasVsKin->Fill(locPhoton2P4.E(),locPhoton2P4_Measured.E());
			locUsedSoFar_Photon2P.insert(locPhoton2NeutralID);
		}

                map<Particle_t, set<Int_t> > locUsedThisCombo_Mass3pi;
		locUsedThisCombo_Mass3pi[PiPlus].insert(locPiPlusTrackID);
		locUsedThisCombo_Mass3pi[PiMinus].insert(locPiMinusTrackID);
		locUsedThisCombo_Mass3pi[Gamma].insert(locPhoton1NeutralID);
		locUsedThisCombo_Mass3pi[Gamma].insert(locPhoton2NeutralID);
		if(locUsedSoFar_Mass3pi.find(locUsedThisCombo_Mass3pi) == locUsedSoFar_Mass3pi.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
		        dH1_Mass3pi->Fill(loc3piP4.M());
			locUsedSoFar_Mass3pi.insert(locUsedThisCombo_Mass3pi);
		}

                map<Particle_t, set<Int_t> > locUsedThisCombo_Pi0;
		locUsedThisCombo_Pi0[Gamma].insert(locPhoton1NeutralID);
		locUsedThisCombo_Pi0[Gamma].insert(locPhoton2NeutralID);
		if(locUsedSoFar_Pi0.find(locUsedThisCombo_Pi0) == locUsedSoFar_Pi0.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
		        dH1_Pi0P->Fill(Pi0P);
		        dH2_Pi0P_MeasVsKin->Fill(Pi0P,Pi0P_Measured);
		        dH2_Photon2P_vs_Photon1P->Fill(locPhoton1P4.E(),locPhoton2P4.E());
			locUsedSoFar_Pi0.insert(locUsedThisCombo_Pi0);
		}

                map<Particle_t, set<Int_t> > locUsedThisCombo_Pi0Measured;
		locUsedThisCombo_Pi0Measured[Gamma].insert(locPhoton1NeutralID);  // assume that measured and kin fit are tied together 
		locUsedThisCombo_Pi0Measured[Gamma].insert(locPhoton2NeutralID);  
		if(locUsedSoFar_Pi0Measured.find(locUsedThisCombo_Pi0Measured) == locUsedSoFar_Pi0Measured.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
		        dH1_Pi0MeasuredP->Fill(Pi0P_Measured);
		        dH1_Pi0MeasuredMass->Fill(locDecayingPi0P4_Measured.M());
			locUsedSoFar_Pi0Measured.insert(locUsedThisCombo_Pi0Measured);
			Int_t tempCal=0;
			if (Ephoton1_FCAL>0 && Ephoton2_FCAL>0) {
			  CostheStarA = Ephoton1_FCAL>Ephoton2_FCAL? (Ebar-2*Ephoton2_FCAL)/Pbar :  (Ebar-2*Ephoton1_FCAL)/Pbar;
			  CostheStarB = (Ephoton1_FCAL-Ephoton2_FCAL)/sqrt((Ephoton1_FCAL+Ephoton2_FCAL)*(Ephoton1_FCAL+Ephoton2_FCAL) - MPI*MPI);
			  CostheStarB = CostheStarB>0? CostheStarB : -CostheStarB;   //make positive
			  dH1_CostheStarB_FCAL->Fill(CostheStarB);
			  dH1_CostheStarA_FCAL->Fill(CostheStarA);
	                  dH1_Pi0Theta_FCAL->Fill(Pi0Theta);
	                  dH1_PhotonTheta_FCAL->Fill(Photon1Theta);
	                  dH1_PhotonTheta_FCAL->Fill(Photon2Theta);
			  tempCal = 1;
			}
			if (Ephoton1_BCAL>0 && Ephoton2_BCAL>0) {
			  CostheStarA = Ephoton1_BCAL>Ephoton2_BCAL? (Ebar-2*Ephoton2_BCAL)/Pbar :  (Ebar-2*Ephoton1_BCAL)/Pbar;
			  CostheStarB = (Ephoton1_BCAL-Ephoton2_BCAL)/sqrt((Ephoton1_BCAL+Ephoton2_BCAL)*(Ephoton1_BCAL+Ephoton2_BCAL) - MPI*MPI);
			  CostheStarB = CostheStarB>0? CostheStarB : -CostheStarB;   //make positive
			  dH1_CostheStarB_BCAL->Fill(CostheStarB);
			  dH1_CostheStarA_BCAL->Fill(CostheStarA);
	                  dH1_Pi0Theta_BCAL->Fill(Pi0Theta);
	                  dH1_PhotonTheta_BCAL->Fill(Photon1Theta);
	                  dH1_PhotonTheta_BCAL->Fill(Photon2Theta);
			  tempCal = 2;
			}
			if (Ephoton1_BCAL>0 && Ephoton2_FCAL>0) {
			  CostheStarA = Ephoton1_BCAL>Ephoton2_FCAL? (Ebar-2*Ephoton2_FCAL)/Pbar :  (Ebar-2*Ephoton1_BCAL)/Pbar;
			  CostheStarB = (Ephoton1_BCAL-Ephoton2_FCAL)/sqrt((Ephoton1_BCAL+Ephoton2_FCAL)*(Ephoton1_BCAL+Ephoton2_FCAL) - MPI*MPI);
			  CostheStarB = CostheStarB>0? CostheStarB : -CostheStarB;   //make positive
			  dH1_CostheStarB_FCALBCAL->Fill(CostheStarB);
			  dH1_CostheStarA_FCALBCAL->Fill(CostheStarA);
	                  dH1_Pi0Theta_FCALBCAL->Fill(Pi0Theta);
	                  dH1_PhotonTheta_FCALBCAL->Fill(Photon1Theta);
	                  dH1_PhotonTheta_FCALBCAL->Fill(Photon2Theta);
			  tempCal = 3;
			}
			if (Ephoton1_FCAL>0 && Ephoton2_BCAL>0) {
			  CostheStarA = Ephoton1_FCAL>Ephoton2_BCAL? (Ebar-2*Ephoton2_BCAL)/Pbar :  (Ebar-2*Ephoton1_FCAL)/Pbar;
			  CostheStarB = (Ephoton1_FCAL-Ephoton2_BCAL)/sqrt((Ephoton1_FCAL+Ephoton2_BCAL)*(Ephoton1_FCAL+Ephoton2_BCAL) - MPI*MPI);
			  CostheStarB = CostheStarB>0? CostheStarB : -CostheStarB;   //make positive
			  dH1_CostheStarB_FCALBCAL->Fill(CostheStarB);
			  dH1_CostheStarA_FCALBCAL->Fill(CostheStarA);
	                  dH1_Pi0Theta_FCALBCAL->Fill(Pi0Theta);
	                  dH1_PhotonTheta_FCALBCAL->Fill(Photon1Theta);
	                  dH1_PhotonTheta_FCALBCAL->Fill(Photon2Theta);
			  tempCal = 3;
			}
			effinfo.cosB = CostheStarB;
			effinfo.cosA = CostheStarA;
			effinfo.locCal = tempCal;
			if (locPhoton1P4.E() > locPhoton2P4.E()) {
			  effinfo.P4gamma1 = locPhoton1P4;
			  effinfo.P4gamma2 = locPhoton2P4;
			}
			else {
			  effinfo.P4gamma1 = locPhoton2P4;
			  effinfo.P4gamma2 = locPhoton1P4;
			}
			Veffinfo.push_back(effinfo);

			dH1_CostheStarDiff->Fill(CostheStarA-CostheStarB);
	      
		}


		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		//Uniqueness tracking: Build the map of particles used for the missing mass
			//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[PiPlus].insert(locPiPlusTrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinusTrackID);
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhoton1NeutralID);
		locUsedThisCombo_MissingMass[Gamma].insert(locPhoton2NeutralID);

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

		// selective printout for surviving events

		if (CostheStarB > 1.5) {
		cout << endl << "Kin Fit jcombo=" << loc_i << endl; 
		cout << " locBeamP4="; locBeamP4.Print();
		cout << " Mass=" << locProtonP4.M() << " locProtonP4="; locProtonP4.Print();
		cout << " Mass=" << locPiPlusP4.M() << " locPiPlusP4="; locPiPlusP4.Print();
		cout << " Mass=" << locPiMinusP4.M() << " locPiMinusP4="; locPiMinusP4.Print();
		cout << " Mass=" << locDecayingPi0P4.M() << " locDecayingPi0P4="; locDecayingPi0P4.Print();
		cout << " Mass=" << locPhoton1P4.M() << " locPhoton1P4="; locPhoton1P4.Print();
		cout << " Mass=" << locPhoton2P4.M() << " locPhoton2P4="; locPhoton2P4.Print();
		cout << " Mass=" << locMissingP4.M() << " locMissingP4="; locMissingP4.Print();
		cout << " Mass=" << loc3piP4.M() << " loc3piP4="; loc3piP4.Print();

		cout << "Measured" << endl; 
		cout << " locBeamP4_Measured="; locBeamP4_Measured.Print();
		cout << " Mass=" << locProtonP4_Measured.M() << " locProtonP4="; locProtonP4_Measured.Print();
		cout << " Mass=" << locPiPlusP4_Measured.M() << " locPiPlusP4="; locPiPlusP4_Measured.Print();
		cout << " Mass=" << locPiMinusP4_Measured.M() << " locPiMinusP4="; locPiMinusP4_Measured.Print();
		cout << " Mass=" << locPhoton1P4_Measured.M() << " locPhoton1P4_Measured="; locPhoton1P4_Measured.Print();
		cout << " Mass=" << locPhoton2P4_Measured.M() << " locPhoton2P4_Measured="; locPhoton2P4_Measured.Print();
		cout << " Mass=" << loc2gammaP4_Measured.M() << " loc2gammaP4_Measured="; loc2gammaP4_Measured.Print();
		cout << " Mass=" << locMissingP4_Measured.M() << " locMissingP4_Measured="; locMissingP4_Measured.Print();
		cout << " Mass=" << loc3piP4_Measured.M() << " loc3piP4_Measured="; loc3piP4_Measured.Print();
		cout << " Photon1E=" << locPhoton1P4_Measured.E() << " Photon2E=" << locPhoton2P4_Measured.E() 
		     << " Ephoton1_BCAL=" << Ephoton1_BCAL <<  " Ephoton2_BCAL=" << Ephoton2_BCAL 
		     << " Ephoton1_FCAL=" << Ephoton1_FCAL <<  " Ephoton2_FCAL=" << Ephoton2_FCAL << endl;
		cout << "Measured: MM2 =" << locMissingP4_Measured.M2() << " GenDeltaE=" << locBeamP4.E()-locBeamP4_Measured.E() 
                     << " CostheStarB=" << CostheStarB  << " CostheStarA=" << CostheStarA << endl;

		}



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

void DSelector_pi0pippim__B4_ver21::Finalize(void)
{
	//Save anything to output here that you do not want to be in the default DSelector output ROOT file.

	//Otherwise, don't do anything else (especially if you are using PROOF).
		//If you are using PROOF, this function is called on each thread,
		//so anything you do will not have the combined information from the various threads.
		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem.

	//DO YOUR STUFF HERE

    
    //
    // unpack costhestar distribution and intepret it as an efficiency distribution as a function of E2meas
    
    const Int_t ndim = dH1_CostheStarB_FCAL->GetNbinsX();
    Double_t xlo = dH1_CostheStarB_FCAL->GetBinLowEdge(1);
    Double_t width = dH1_CostheStarB_FCAL->GetBinWidth(1);

    int max_bin;
    max_bin = dH1_CostheStarB_BCAL->GetMaximumBin();
    Double_t max_contentB_BCAL = dH1_CostheStarB_BCAL->GetBinContent(max_bin);     // use as normalization for efficiency
    max_bin = dH1_CostheStarB_FCAL->GetMaximumBin();
    Double_t max_contentB_FCAL = dH1_CostheStarB_FCAL->GetBinContent(max_bin);     // use as normalization for efficiency
    max_bin = dH1_CostheStarB_FCALBCAL->GetMaximumBin();
    Double_t max_contentB_FCALBCAL = dH1_CostheStarB_FCALBCAL->GetBinContent(max_bin);     // use as normalization for efficiency

    Double_t cosmin = 0;
    Double_t cosmax = 0.25;
    Double_t eff0 = 1.0;
    Double_t ave_contentB_BCAL = 0;
    Int_t npts_contentB_BCAL = 0;
    for (Int_t j=0; j<ndim; j++) {
      Float_t cos = xlo + (j+0.5)*width;
      Float_t content = dH1_CostheStarB_BCAL->GetBinContent(j+1);
      if (cos > cosmin && cos < cosmax) {
	ave_contentB_BCAL += content;
	npts_contentB_BCAL++;
      }
    }
    ave_contentB_BCAL = npts_contentB_BCAL>0? ave_contentB_BCAL*eff0/npts_contentB_BCAL: 0;
    cout << " ave_contentB_BCAL=" << ave_contentB_BCAL << endl;

    Double_t ave_contentB_FCAL = 0;
    Int_t npts_contentB_FCAL = 0;
    for (Int_t j=0; j<ndim; j++) {
      Float_t cos = xlo + (j+0.5)*width;
      Float_t content = dH1_CostheStarB_FCAL->GetBinContent(j+1);
      if (cos > cosmin && cos < cosmax) {
	ave_contentB_FCAL += content;
	npts_contentB_FCAL++;
      }
    }
    ave_contentB_FCAL = npts_contentB_FCAL>0? ave_contentB_FCAL*eff0/npts_contentB_FCAL: 0;
    cout << " ave_contentB_FCAL=" << ave_contentB_FCAL << endl;

    Double_t ave_contentB_FCALBCAL = 0;
    Int_t npts_contentB_FCALBCAL = 0;
    for (Int_t j=0; j<ndim; j++) {
      Float_t cos = xlo + (j+0.5)*width;
      Float_t content = dH1_CostheStarB_FCALBCAL->GetBinContent(j+1);
      if (cos > cosmin && cos < cosmax) {
	ave_contentB_FCALBCAL += content;
	npts_contentB_FCALBCAL++;
      }
    }
    ave_contentB_FCALBCAL = npts_contentB_FCALBCAL>0? ave_contentB_FCALBCAL*eff0/npts_contentB_FCALBCAL: 0;
    cout << " ave_contentB_FCALBCAL=" << ave_contentB_FCALBCAL << endl;
    
    
    for(Int_t j=0;j<ndim;j++) {
        Float_t costhestar_bin;
        Float_t content;
        Float_t efficiency;
        Float_t sigma;
         
        costhestar_bin = dH1_CostheStarB_BCAL->GetBinCenter(j+1);
        content = dH1_CostheStarB_BCAL->GetBinContent(j+1);
        efficiency = ave_contentB_BCAL > 0? content/ave_contentB_BCAL: 0;
        dH1_CostheStarB_BCAL_eff->SetBinContent(j+1,efficiency);
        sigma = ave_contentB_BCAL > 0? sqrt (content)/ave_contentB_BCAL: 0;   //  just take uncertainty in point, ignore fluctuation in maximum
        dH1_CostheStarB_BCAL_eff->SetBinError(j+1,sigma);

        costhestar_bin = dH1_CostheStarB_FCAL->GetBinCenter(j+1);
        content = dH1_CostheStarB_FCAL->GetBinContent(j+1);
        efficiency = ave_contentB_FCAL > 0? content/ave_contentB_FCAL: 0;
        dH1_CostheStarB_FCAL_eff->SetBinContent(j+1,efficiency);
        sigma = ave_contentB_FCAL > 0? sqrt (content)/ave_contentB_FCAL: 0;   //  just take uncertainty in point, ignore fluctuation in maximum
        dH1_CostheStarB_FCAL_eff->SetBinError(j+1,sigma);

        costhestar_bin = dH1_CostheStarB_FCALBCAL->GetBinCenter(j+1);
        content = dH1_CostheStarB_FCALBCAL->GetBinContent(j+1);
        efficiency = ave_contentB_FCALBCAL > 0? content/ave_contentB_FCALBCAL: 0;
        dH1_CostheStarB_FCALBCAL_eff->SetBinContent(j+1,efficiency);
        sigma =  ave_contentB_FCALBCAL > 0? sqrt (content)/ave_contentB_FCALBCAL: 0;   //  just take uncertainty in point, ignore fluctuation in maximum
        dH1_CostheStarB_FCALBCAL_eff->SetBinError(j+1,sigma);
    }

	// loop over information stored in vector
	for (Int_t kk=0; kk<Veffinfo.size(); kk++) {
	  Double_t CostheA = Veffinfo[kk].cosA;
	  Double_t CostheB = Veffinfo[kk].cosB;
	  Double_t E1 = Veffinfo[kk].P4gamma1.E();
	  Double_t E2 = Veffinfo[kk].P4gamma2.E();
	  Int_t locCal = Veffinfo[kk].locCal;

	  cout << endl << " Vector loop=" << Veffinfo.size() << " CostheA=" << CostheA << " CostheB=" << CostheB << " locCal=" << locCal << endl;  
          cout << " P4gamma1: E1=" << Veffinfo[kk].P4gamma1.E() ; Veffinfo[kk].P4gamma1.Print();  
          cout << " P4gamma2: E2=" << Veffinfo[kk].P4gamma2.E() ; Veffinfo[kk].P4gamma2.Print();

	  Int_t bin = (CostheB-xlo)/width + 1.5;

	  if (locCal == 1) {
	    Double_t eff=dH1_CostheStarB_FCAL_eff->GetBinContent(bin);
	    dH1_E2_w1_FCAL->Fill(E2,1.);
	    dH1_E2_weff_FCAL->Fill(E2,eff);
	  }
	  else if (locCal == 2) {
	    Double_t eff=dH1_CostheStarB_BCAL_eff->GetBinContent(bin);
	    dH1_E2_w1_BCAL->Fill(E2,1.);
	    dH1_E2_weff_BCAL->Fill(E2,eff);
	  }
	  else if (locCal == 3) {
	    Double_t eff=dH1_CostheStarB_FCALBCAL_eff->GetBinContent(bin);
	    dH1_E2_w1_FCALBCAL->Fill(E2,1.);
	    dH1_E2_weff_FCALBCAL->Fill(E2,eff);
	  }
	  else {
	    cout << " *** DSelector illegal locCal=" << locCal << endl;
	  }

	}

	// now divide histograms to produce an efficiency
	dH1_E2_eff_FCAL->Add(dH1_E2_weff_FCAL);
	dH1_E2_eff_FCAL->Divide(dH1_E2_w1_FCAL);
	dH1_E2_eff_BCAL->Add(dH1_E2_weff_BCAL);
	dH1_E2_eff_BCAL->Divide(dH1_E2_w1_BCAL);
	dH1_E2_eff_FCALBCAL->Add(dH1_E2_weff_FCALBCAL);
	dH1_E2_eff_FCALBCAL->Divide(dH1_E2_w1_FCALBCAL);
 

	//CALL THIS LAST
	DSelector::Finalize(); //Saves results to the output file
}
