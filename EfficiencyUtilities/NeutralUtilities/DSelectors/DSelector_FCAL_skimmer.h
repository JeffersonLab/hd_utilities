#ifndef DSelector_FCAL_skimmer_h
#define DSelector_FCAL_skimmer_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_FCAL_skimmer : public DSelector
{
	public:

		DSelector_FCAL_skimmer(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_FCAL_skimmer(){}

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
		DChargedTrackHypothesis* dProtonWrapper;

		//Step 1
		DParticleComboStep* dStep1Wrapper;
		DChargedTrackHypothesis* dPiPlusWrapper;
		DChargedTrackHypothesis* dPiMinusWrapper;

		//Step 2
		DParticleComboStep* dStep2Wrapper;
		DKinematicData* dMissingPhotonWrapper;
		DNeutralParticleHypothesis* dPhotonWrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		TH1I* dHist_MissingMassSquared;
		TH1I* dHist_BeamEnergy;

		Bool_t jz_get_thrown_info;
		Long64_t jz_counter;
		Int_t jz_PolDirection;
		Int_t jz_NumUnusedCombosSaved;
        Float_t jz_UnusedPhotonThetaCut;
		
	ClassDef(DSelector_FCAL_skimmer, 0);
};

void DSelector_FCAL_skimmer::Get_ComboWrappers(void)
{
	
	//OLD!!!!
	//Step 0
	// dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	// dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	// dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));

	//Step 1
	// dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	// dPiPlusWrapper = static_cast<DChargedTrackHypothesis*>(dStep1Wrapper->Get_FinalParticle(1));
	// dPiMinusWrapper = static_cast<DChargedTrackHypothesis*>(dStep1Wrapper->Get_FinalParticle(2));

	//Step 2
	// dStep2Wrapper = dComboWrapper->Get_ParticleComboStep(2);
	// dMissingPhotonWrapper = dStep2Wrapper->Get_FinalParticle(0);
	// dPhotonWrapper = static_cast<DNeutralParticleHypothesis*>(dStep2Wrapper->Get_FinalParticle(1));

	//New!!! (ReactionFilter)
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dPiPlusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));
	dPiMinusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(2));
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(3));

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dPhotonWrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));
	dMissingPhotonWrapper = dStep1Wrapper->Get_FinalParticle(1);
	
	}

#endif // DSelector_FCAL_skimmer_h
