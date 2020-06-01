#include "DSelector_p2gamma.h"

void DSelector_p2gamma::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "p2gamma.root"; //"" for none
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

	//KINEMATICS (INITIAL)
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Initial"));

	//PID  (INITIAL)
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Initial"));

	//MASSES
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPi0, 110, 0.08, 0.19, "Pi0"));
	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.25, 0.25));
	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));	

	// ************* CROSS SECTION SYSTEMATIC CUTS ****************** //

	//CUT MISSING MASS
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.01, 0.01));

	//NOMINAL PID TIMING CUTS (https://halldweb.jlab.org/wiki/index.php/PID_study_proposal)
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.5, Gamma, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.5, Gamma, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, Proton, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.6, Proton, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, Proton, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, Proton, SYS_START));

	//MASSES (FINAL)
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 0, MyPi0, 110, 0.08, 0.19, "Pi0Final"));

	//KINEMATICS (FINAL)
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Final"));

	//PID (FINAL)
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Final"));

	// ANALYZE CUT ACTIONS
        // Change MyPi0 to match reaction
        dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 0, MyPi0, 110, 0.08, 0.19, "CutActionEffect" );

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();
	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 600, -0.06, 0.06);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 600, 0.0, 12.0);
}

Bool_t DSelector_p2gamma::Process(Long64_t locEntry)
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
	dAnalyzeCutActions->Reset_NewEvent(); // manual action, must call Reset_NewEvent()

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

		/********************************************* GET COMBO RF TIMING INFO *****************************************/

                TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
                Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
                Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
                Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
                Int_t locNumOutOfTimeBunchesInTree = 4; //YOU need to specify this number
                        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches) 

                Bool_t locSkipNearestOutOfTimeBunch = true; // True: skip events from nearest out-of-time bunch on either side (recommended).
                Int_t locNumOutOfTimeBunchesToUse = locSkipNearestOutOfTimeBunch ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree;
                Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed: need added factor.
                Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
                Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
                if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1) continue; // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
		
		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locPhoton1P4_Measured + locPhoton2P4_Measured + locProtonP4_Measured;

		TLorentzVector locPhotonPairP4_Measured = locPhoton1P4_Measured + locPhoton2P4_Measured;
		
		// Combine 4-vectors for Missing momentum for gamma p -> Xp
                TLorentzVector locMissingP4_OffProton = locBeamP4_Measured + dTargetP4;
                locMissingP4_OffProton -= locProtonP4_Measured;

		/******************************************** EXECUTE YOUR NON-PID SELECTION CUTS HERE **********************************************/

		/**************************************** PreSelection ******************************************/
		//The space-time information when the photons hit the detector                
                TLorentzVector locPhoton1_X4_Measured = dPhoton1Wrapper->Get_X4_Shower();
                TLorentzVector locPhoton2_X4_Measured = dPhoton2Wrapper->Get_X4_Shower();

                //The space-time information when the photons hit the detector                
                TLorentzVector X4_Photon1 = locPhoton1_X4_Measured;
                double x_g1 = X4_Photon1.X();
                double y_g1 = X4_Photon1.Y();
                double z_g1 = X4_Photon1.Z();
                double r_g1 = X4_Photon1.Perp();
                double t_g1 = X4_Photon1.T();

                TLorentzVector X4_Photon2 = locPhoton2_X4_Measured;
                double x_g2 = X4_Photon2.X();
                double y_g2 = X4_Photon2.Y();
                double z_g2 = X4_Photon2.Z();
                double r_g2 = X4_Photon2.Perp();
                double t_g2 = X4_Photon2.T();

                //the distant between two photons in XY plane, used for splits cut in FCAL
                double dis_g1_g2 = sqrt((x_g1-x_g2)*(x_g1-x_g2)+(y_g1-y_g2)*(y_g1-y_g2));

                //define the detector photon angle
                TVector3 X3_Photon1 = X4_Photon1.Vect();
                TVector3 X3_Photon2 = X4_Photon2.Vect();
                TVector3 X3_target(0.0, 0.0, 65.0);

		double theta_det_g1 = (X3_Photon1-X3_target).Theta();
                double theta_det_g2 = (X3_Photon2-X3_target).Theta();
                double theta_det_gmin = theta_det_g1;
                if(theta_det_g1>theta_det_g2)
			theta_det_gmin = theta_det_g2;
                double theta_det_g_min = theta_det_gmin*180/TMath::Pi();

		//vertex information
		TLorentzVector locProtonX4_Measured = dProtonWrapper->Get_X4_Measured();
                double z = 0.0;
                double r = 0.0;
                z = locProtonX4_Measured.Z();
                r = locProtonX4_Measured.Perp();

                if(locProtonP4_Measured.Vect().Mag() < 0.25) //cut in momentum of proton
			continue;  

                if(z<=51.0 || z>=76.0 || r>=1.0) //cut in vertex
			continue;   

                if(dis_g1_g2<=12.5) //cut for FCAL splits
			continue;   

		// replace with Mark's cuts...
                if((180.0/TMath::Pi())*theta_det_g1<=2.5 || (180.0/TMath::Pi())*theta_det_g2<=2.5) continue; //cut for beam hole
		
                if(((180.0/TMath::Pi())*theta_det_g1<=11.5 && (180.0/TMath::Pi())*theta_det_g1>=10.3) || ((180.0/TMath::Pi())*theta_det_g2<=11.5 && (180.0/TMath::Pi())*theta_det_g2>=10.3)) continue; //cut for FCAL-BCAL gap

		//Missing mass off proton
                double mmo = locMissingP4_OffProton.M();

                //Missing Mass squared
                double mmsq = locMissingP4_Measured.M2();

		//Missing Energy
                double me = locMissingP4_Measured.E();

                //Delta_phi
		double phi_2g = (180.0/TMath::Pi())*(locPhotonPairP4_Measured.Phi());
                double phi_p = (180/TMath::Pi())*(locProtonP4_Measured.Phi());
                double delta_phi = phi_2g - phi_p;
                if(delta_phi>360.) delta_phi += -360.;
                if(delta_phi<0.)   delta_phi +=  360.;

		// Nominal non-PID analysis cuts
		if(locBeamP4.E()<6.5 || locBeamP4.E()>11.5 || fabs(delta_phi-180.0)>5.0 || me<-0.50 || me>0.70 || mmo>0.5) 
			continue;

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		dAnalyzeCutActions->Perform_ActionWeight(locHistAccidWeightFactor); 

		// Note: Using accidental subtraction in AnalyzeCutAction requires version > 1.10.0 of gluex_root_analysis. To use without accidental subtraction and a previous version of the software, comment out the line above and uncomment next line
		//dAnalyzeCutActions->Perform_Action();

		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		//if you manually execute any actions, and it fails a cut, be sure to call:
			//dComboWrapper->Set_IsComboCut(true);

		// Fill histograms and execute analysis

	} // end of combo loop

	//FILL HISTOGRAMS: Num combos / events surviving actions
	Fill_NumCombosSurvivedHists();

	return kTRUE;
}

void DSelector_p2gamma::Finalize(void)
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
