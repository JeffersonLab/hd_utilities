#include "DSelector_cdc_dedx.h"

//#include "DCustomAction_New_CDC_dEdx_cuts.cc"
//#include "DCustomAction_New_CDC_dEdx_cuts.h"


void DSelector_cdc_dedx::Init(TTree *locTree)
{

	dOutputFileName = "hist_cdc_dedx.root"; //"" for none
	dOutputTreeFileName = "tree_cdc_dedx.root"; //"" for none
	dFlatTreeFileName = ""; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = ""; //if blank, default name will be chosen

	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree

	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	Get_ComboWrappers();

        IsMC = (dTreeInterface->Get_Branch("MCWeight") != NULL);

	cout << "Is MC: " << IsMC << endl;

        if (!IsMC) {
  	    int checkerr = Read_CDC_dEdx_amp_CorrectionFile();
            if (checkerr) exit(0);
	    checkerr = Read_CDC_dEdx_int_CorrectionFile();
            if (checkerr) exit(0);
        }

	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, true, ""));
	Initialize_Actions();


	dHist_NewDedxProton = new TH2I("New_CDC_dEdxVsP_Proton", ";Momentum (GeV/c);CDC dE/dx (keV/cm)",250,0.0,10.0, 250, 0.0, 25.0);

	dHist_NewDedxKPlus = new TH2I("New_CDC_dEdxVsP_K+", ";Momentum (GeV/c);CDC dE/dx (keV/cm)",250,0.0,10.0, 250, 0.0, 25.0);

	dHist_NewDedxKMinus = new TH2I("New_CDC_dEdxVsP_K-", ";Momentum (GeV/c);CDC dE/dx (keV/cm)",250,0.0,10.0, 250, 0.0, 25.0);

	dHist_NewDedxIntProton = new TH2I("New_CDC_dEdxintVsP_Proton", ";Momentum (GeV/c);CDC dE/dx (keV/cm)",250,0.0,10.0, 250, 0.0, 25.0);

	dHist_NewDedxIntKPlus = new TH2I("New_CDC_dEdxintVsP_K+", ";Momentum (GeV/c);CDC dE/dx (keV/cm)",250,0.0,10.0, 250, 0.0, 25.0);

	dHist_NewDedxIntKMinus = new TH2I("New_CDC_dEdxintVsP_K-", ";Momentum (GeV/c);CDC dE/dx (keV/cm)",250,0.0,10.0, 250, 0.0, 25.0);


	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("newdedx_p", "NumCombos");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("newdedx_kp", "NumCombos");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("newdedx_km", "NumCombos");

	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("newdedxint_p", "NumCombos");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("newdedxint_kp", "NumCombos");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("newdedxint_km", "NumCombos");
	

}

Bool_t DSelector_cdc_dedx::Process(Long64_t locEntry)
{
	DSelector::Process(locEntry); //Gets the data from the tree for the entry

	TLorentzVector locProductionX4 = Get_X4_Production();

	Reset_Actions_NewEvent();

        Float_t newdedx_p;
        Float_t newdedx_kp;
        Float_t newdedx_km;

        Float_t newdedxint_p;
        Float_t newdedxint_kp;
        Float_t newdedxint_km;


	/************************************************* LOOP OVER COMBOS *************************************************/

        Int_t NCombos = (Int_t)Get_NumCombos();

	//Loop over combos
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{

                newdedx_p=0;
                newdedx_kp=0;
                newdedx_km=0;

                newdedxint_p=0;
                newdedxint_kp=0;
                newdedxint_km=0;


		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);

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


		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

                // proton momentum cut
                if (locProtonP4_Measured.P() <= 0.35) dComboWrapper->Set_IsComboCut(true);  
		if(dComboWrapper->Get_IsComboCut()) continue;

                //vertex cut
                Float_t vertexX = locProductionX4.X();
                Float_t vertexY = locProductionX4.Y();
                Float_t vertexZ = locProductionX4.Z();

                Bool_t badv=0;
                if (vertexZ<51.0 || vertexZ>79.1) dComboWrapper->Set_IsComboCut(true);  
		if(dComboWrapper->Get_IsComboCut()) continue;
		
		// ------------------ NEW CDC dE/dx histos ------------------

                // protons

                Int_t loc_protonhypo = Get_Hypo_Num(Proton,locProtonTrackID); //,dChargedHypoWrapper);

                if (loc_protonhypo > -1) {

                  newdedx_p = Correct_CDC_dEdx_amp(loc_protonhypo,locProtonP4.Theta());
		  newdedxint_p = Correct_CDC_dEdx_int(loc_protonhypo,locProtonP4.Theta());

		  if (newdedx_p > 0) dHist_NewDedxProton->Fill(locProtonP4.P(),newdedx_p);

                  if (newdedxint_p > 0) dHist_NewDedxIntProton->Fill(locProtonP4.P(),newdedxint_p);
                }


		// K+

                Int_t loc_kplushypo = Get_Hypo_Num(KPlus,locKPlusTrackID); //,dChargedHypoWrapper);
		
                if (loc_kplushypo > -1) {

                  newdedx_kp = Correct_CDC_dEdx_amp(loc_kplushypo,locKPlusP4.Theta());
		  newdedxint_kp = Correct_CDC_dEdx_int(loc_kplushypo,locKPlusP4.Theta());

		  if (newdedx_kp > 0) dHist_NewDedxKPlus->Fill(locKPlusP4.P(),newdedx_kp);
                  if (newdedxint_kp > 0) dHist_NewDedxIntKPlus->Fill(locKPlusP4.P(),newdedxint_kp);
                }


		// K-

                Int_t loc_kminushypo = Get_Hypo_Num(KMinus,locKMinusTrackID); //,dChargedHypoWrapper);

                if (loc_kminushypo > -1) {

                  newdedx_km = Correct_CDC_dEdx_amp(loc_kminushypo,locKMinusP4.Theta());
		  newdedxint_km = Correct_CDC_dEdx_int(loc_kminushypo,locKMinusP4.Theta());

		  if (newdedx_km > 0) dHist_NewDedxKMinus->Fill(locKMinusP4.P(),newdedx_km);
                  if (newdedxint_km > 0) dHist_NewDedxIntKMinus->Fill(locKMinusP4.P(),newdedxint_km);
                }



		dTreeInterface->Fill_Fundamental<Float_t>("newdedx_p", (Float_t)newdedx_p,loc_i);
		dTreeInterface->Fill_Fundamental<Float_t>("newdedx_kp", (Float_t)newdedx_kp,loc_i);
		dTreeInterface->Fill_Fundamental<Float_t>("newdedx_km", (Float_t)newdedx_km,loc_i);

		dTreeInterface->Fill_Fundamental<Float_t>("newdedxint_p", (Float_t)newdedxint_p,loc_i);
		dTreeInterface->Fill_Fundamental<Float_t>("newdedxint_kp", (Float_t)newdedxint_kp,loc_i);
		dTreeInterface->Fill_Fundamental<Float_t>("newdedxint_km", (Float_t)newdedxint_km,loc_i);
		
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


	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/

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


	return kTRUE;
}

void DSelector_cdc_dedx::Finalize(void)
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
