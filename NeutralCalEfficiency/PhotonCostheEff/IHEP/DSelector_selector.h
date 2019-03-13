#ifndef DSelector_selector_h
#define DSelector_selector_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_selector : public DSelector
{
	public:

		DSelector_selector(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_selector(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

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
                TH1F* h_m_pi0;
                TH1F* h_m_omega;
                TH1F* h_m_pi0_M;
                TH1F* h_m_omega_M;
                TH1F* h_KinFitChiSq;
                TH1F* h_KinFitChiSq_best;
                TH1F* h_KinFitCL;
                TH1F* h_KinVertex_z_pre;
                TH1F* h_KinVertex_z_aft;
                TH2F* h_KinVertex_r_pre;
                TH2F* h_KinVertex_r_aft;
                TH2F* h_r_FCAL;
                TH1F* h_z_BCAL;
                TH1F* h_MissingMassSquared;
                TH1F* h_MissingE;
                TH1F* h_MissingTM;
                TH1F* h_PBE;
                TH1F* h_Ncombo_pre;
                TH1F* h_Ncombo_aft;
                TH1F* h_the_proton;
                TH1F* h_the_pip;
                TH1F* h_the_pim;
                TH1F* h_beamdeltaT;
                TH1F* h_costhe_gam;
                TH1F* h_Egam;
		
		TH1I* dHist_MissingMassSquared;
		TH1I* dHist_BeamEnergy;

	ClassDef(DSelector_selector, 0);
};

void DSelector_selector::Get_ComboWrappers(void)
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

#endif // DSelector_selector_h
