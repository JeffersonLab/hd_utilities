#ifndef DSelector_pi0pippim__B4_ver21_h
#define DSelector_pi0pippim__B4_ver21_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include <TRandom1.h>



		// store information from first loop in order to process second combo loop
		struct infostruct{
		  double cosB;
		  double cosA;
		  Int_t locCal;
		  TLorentzVector P4gamma1;
		  TLorentzVector P4gamma2;
		}; 
		struct infostruct effinfo;
		vector<struct infostruct> Veffinfo;   // vector of efficincy information



class DSelector_pi0pippim__B4_ver21 : public DSelector
{
	public:

		DSelector_pi0pippim__B4_ver21(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_pi0pippim__B4_ver21(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

		// ANALYZE CUT ACTIONS
		// // Automatically makes mass histograms where one cut is missing
		DHistogramAction_AnalyzeCutActions* dAnalyzeCutActions;

		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS

		//Step 0
		DParticleComboStep* dStep0Wrapper;
		DBeamParticle* dComboBeamWrapper;
		DChargedTrackHypothesis* dPiPlusWrapper;
		DChargedTrackHypothesis* dPiMinusWrapper;
		DChargedTrackHypothesis* dProtonWrapper;

		//Step 1
		DParticleComboStep* dStep1Wrapper;
		DKinematicData* dDecayingPi0Wrapper;
		DNeutralParticleHypothesis* dPhoton1Wrapper;
		DNeutralParticleHypothesis* dPhoton2Wrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		TH1D* dHist_MissingMassSquared;
		TH1D* dHist_BeamEnergy;

		TH1D* dH1_CutSelection;
		TH1D* dH1_CL;
		TH1D* dH1_Chi2NDF;
		TH1D* dH1_Chi2NDF_BCAL;
		TH1D* dH1_Chi2NDF_FCAL;
		TH1D* dH1_Chi2NDF_FCALBCAL;

		TH1D* dH1_ProtonP;
		TH1D* dH1_PiPlusP;
		TH1D* dH1_PiMinusP;
		TH1D* dH1_Pi0P;
		TH1D* dH1_Photon1P;
		TH1D* dH1_Photon2P;
		TH1D* dH1_Pi0MeasuredP;
		TH1D* dH1_Pi0MeasuredMass;
		TH1D* dH1_Mass3pi;
		TH1D* dH1_MassPiPlusP;
		TH1D* dH1_MassPiMinusP;
		TH1D* dH1_MassPiPlusPiMinus;
		TH1D* dH1_MassPiPlusPi0;
		TH1D* dH1_MassPiMinusPi0;
		TH1D* dH1_t_BCAL;
		TH1D* dH1_t_FCAL;
		TH1D* dH1_t_FCALBCAL;

		TH1D* dH1_CostheStarB_BCAL;
		TH1D* dH1_CostheStarB_FCAL;
		TH1D* dH1_CostheStarB_FCALBCAL;
		TH1D* dH1_CostheStarA_BCAL;
		TH1D* dH1_CostheStarA_FCAL;
		TH1D* dH1_CostheStarA_FCALBCAL;
		TH1D* dH1_CostheStarDiff;

		TH1D* dH1_CostheStarB_BCAL_eff;
		TH1D* dH1_CostheStarB_FCAL_eff;
		TH1D* dH1_CostheStarB_FCALBCAL_eff;

		TH1D* dH1_Pi0Theta_FCAL;
		TH1D* dH1_Pi0Theta_BCAL;
		TH1D* dH1_Pi0Theta_FCALBCAL;

		TH1D* dH1_PhotonTheta_FCAL;
		TH1D* dH1_PhotonTheta_BCAL;
		TH1D* dH1_PhotonTheta_FCALBCAL;

		TH2I* dH2_Chi2_vs_Pi0Theta;
		TH2I* dH2_ProtonP_MeasVsKin;
		TH2I* dH2_PiPlusP_MeasVsKin;
		TH2I* dH2_PiMinusP_MeasVsKin;
		TH2I* dH2_Pi0P_MeasVsKin;
		TH2I* dH2_Photon1P_MeasVsKin;
		TH2I* dH2_Photon2P_MeasVsKin;
		TH2I* dH2_Photon2P_vs_Photon1P;
		TH2I* dH2_ProtondEdX_vs_P;
		TH2I* dH2_PiPlusdEdX_vs_P;
		TH2I* dH2_PiMinusdEdX_vs_P;
		TH2I* dH2_PiPlusTheta_vs_P;
		TH2I* dH2_PiMinusTheta_vs_P;
		TH2I* dH2_Photon1Theta_vs_E_BCAL;
		TH2I* dH2_Photon2Theta_vs_E_BCAL;
		TH2I* dH2_Photon1Theta_vs_E_FCAL;
		TH2I* dH2_Photon2Theta_vs_E_FCAL;
		TH2I* dH2_Photon1Theta_vs_E_FCALBCAL;
		TH2I* dH2_Photon2Theta_vs_E_FCALBCAL;

		TH1D* dH1_E2_w1_FCAL;
		TH1D* dH1_E2_w1_BCAL;
		TH1D* dH1_E2_w1_FCALBCAL;

		TH1D* dH1_E2_weff_FCAL;
		TH1D* dH1_E2_weff_BCAL;
		TH1D* dH1_E2_weff_FCALBCAL;

		TH1D* dH1_E2_eff_FCAL;
		TH1D* dH1_E2_eff_BCAL;
		TH1D* dH1_E2_eff_FCALBCAL;

		TH1D* dH1_Shower_Quality_FCAL1;
		TH1D* dH1_Shower_Quality_FCAL2;
		TH1D* dHist_TaggerAccidentals;
		TH1D* dHist_TaggerAccidentalsWeight;

		TProfile* dP1_E2_eff_FCAL;
		TProfile* dP1_E2_eff_BCAL;
		TProfile* dP1_E2_eff_FCALBCAL;

		TH2D* dH2_E2_w1_FCAL;
		TH2D* dH2_E2_w1_BCAL;
		TH2D* dH2_E2_w1_FCALBCAL;

		TH2D* dH2_E2_weff_FCAL;
		TH2D* dH2_E2_weff_BCAL;
		TH2D* dH2_E2_weff_FCALBCAL;

		TH2D* dH2_E2_eff_FCAL;
		TH2D* dH2_E2_eff_BCAL;
		TH2D* dH2_E2_eff_FCALBCAL;

		TProfile2D* dP2_E2_eff_FCAL;
		TProfile2D* dP2_E2_eff_BCAL;
		TProfile2D* dP2_E2_eff_FCALBCAL;

		// Cut parameters
		TF1* fMinProton_dEdx;
		TF1* fMaxPion_dEdx;
		Double_t dMinKinFitCL;
		Double_t dMaxKinFitChiSq;
		Double_t dMinBeamEnergy;
		Double_t dMaxBeamEnergy;
		Double_t dMinFCALShape;
		Double_t dMin2piMass;
		Double_t dMax2piMass;
		Double_t dMin3piMass;
		Double_t dMax3piMass;
		Double_t dMax3piMass_FCAL;
		Double_t dMinMissingMassSquared;
		Double_t dMaxMissingMassSquared;
		Double_t Epi0_min;
		Double_t Epi0_max;
		Double_t dMinmisE;
		Double_t dMaxmisE;
		Double_t dMaxmisTM;
		Double_t dMinGapTheta;
		Double_t dMaxGapTheta;
		Double_t dMinPiPlusPMass;
		Double_t dMinPiMinusPMass;

	ClassDef(DSelector_pi0pippim__B4_ver21, 0);
};

void DSelector_pi0pippim__B4_ver21::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dPiPlusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));
	dPiMinusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(2));
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(3));

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dDecayingPi0Wrapper = dStep1Wrapper->Get_InitialParticle();
	dPhoton1Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));
	dPhoton2Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(1));
}

#endif // DSelector_pi0pippim__B4_ver21_h
