// $Id$
//
//    File: DEventProcessor_pi_pm_gun_hists.h
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_pi_pm_gun_hists_
#define _DEventProcessor_pi_pm_gun_hists_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DTreeInterface.h"


#include <TOF/DTOFHitMC.h>
#include <TRACKING/DTrackCandidate_factory_THROWN.h>

#include "HDGEOMETRY/DGeometry.h"

#include <TRACKING/DMCTrajectoryPoint.h>

using namespace jana;
using namespace std;

class DEventProcessor_pi_pm_gun_hists : public jana::JEventProcessor
{
	public:
		const char* className(void){return "DEventProcessor_pi_pm_gun_hists";}

		double DELTAPHI_CUTPARM;
		double DELTATHETA_CUTPARM;
		
		double EMIN_CUTPARM; //Minimum thrown photon energy allowed to enter *any* histograms
		double EMAX_CUTPARM; //Maximum thrown photon energy allowed to enter *any* histograms
		double THETAMIN_CUTPARM; //Minimum thrown photon theta (degrees) allowed to enter *any* histograms
		double THETAMAX_CUTPARM; //Maximum thrown photon theta (degrees) allowed to enter *any* histograms
		
		
		
	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber);	///< Called every time a new run number is detected.
		jerror_t evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber);	///< Called every event.
		jerror_t erun(void);						///< Called every time run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

		DGeometry* GX_Geom;
		double fcalFrontFaceZ;
		
		
		//Histograms
		TH1F* h_ThrownPionCharge;
		TH1F* h_ThrownPionP;
		TH1F* h_ThrownPionTheta;
		TH1F* h_ThrownPionPhi;
		
		TH1F* h_PionInteractionZ;
		TH1F* h_PionInteractionMech;
		TH2F* h2_PionInteractionMechvsInteractionZ;
		TH1F* h_PionInteractionCase;
		
		TH1F* h_ChargedTrackHypothesis_NumTracks;
		TH1F* h_ChargedTrackHypothesis_P;
		TH1F* h_ChargedTrackHypothesis_Theta;
		TH1F* h_ChargedTrackHypothesis_Phi;
		TH1F* h_TrackCandidate_NumTracks;
		TH1F* h_TrackCandidate_P;
		TH1F* h_TrackCandidate_Theta;
		TH1F* h_TrackCandidate_Phi;
		TH1F* h_TrackTimeBased_NumTracks;
		TH1F* h_TrackTimeBased_P;
		TH1F* h_TrackTimeBased_Theta;
		TH1F* h_TrackTimeBased_Phi;
		TH1F* h_TrackWireBaseds_NumTracks;
		TH1F* h_TrackWireBaseds_P;
		TH1F* h_TrackWireBaseds_Theta;
		TH1F* h_TrackWireBaseds_Phi;
		TH1F* h_TrackCandidate_FDCCathodes_NumTracks;
		TH1F* h_TrackCandidate_FDCCathodes_P;
		TH1F* h_TrackCandidate_FDCCathodes_Theta;
		TH1F* h_TrackCandidate_FDCCathodes_Phi;
		
		
		
		
		// TH1F* h_thrownE_all_dist;
		// TH1F* h_thrownE_foundshow_dist;
		// TH1F* h_reconE_dist;
		
		// TH1F* h_NFCALShowers;
		// TH1F* h_NFCALShowers_nonprimary;
		// TH1F* h_NFCALShowers_primary;
		// TH1F* h_NNeutralShowers;
		// TH1F* h_NNeutralShowersPreselect;
		
		// TH1F* h_foundE_all_dist;
		// TH1F* h_foundE_DeltaPhiCuts_dist;
		// TH1F* h_foundE_NeutralShower_DeltaPhiCuts_dist;
		// TH1F* h_foundE_NeutralShowerPreselect_DeltaPhiCuts_dist;
		// TH1F* h_foundE_NeutralShowerHypothesis_DeltaPhiCuts_dist;
		// TH1F* h_foundE_NeutralParticle_DeltaPhiCuts_dist;
		// TH1F* h_foundE_highestE_dist;
		// TH1F* h_foundE_primary_dist;
		// TH1F* h_foundE_nonprimary_dist;
		// TH1F* h_foundE_1orless_all_dist;
		// TH1F* h_foundE__1orlessprimary_dist;
		// TH1F* h_foundE_1orless_nonprimary_dist;
		
		// TH1F* h_PhotonInteractionZ;
		// TH1F* h_PhotonInteractionMech;
		// TH2F* h2_PhotonInteractionMechvsInteractionZ;
		// TH1F* h_PhotonInteractionCase;

		// TH1F* h_ThrownPhotonE;
		
		// TH1F* h_thrownTheta;
		// TH1F* h_DeltaPhi_all;
		// TH1F* h_DeltaTheta_all;
		// TH2F* h2_DeltaThetaDeltaPhi_all;
		
		// TH1F* h_foundE_BarrelRegionInteract_dist;
		// TH1F* h_foundE_BarrelRegionInteract_DeltaPhiCuts_dist;
		// TH1F* h_foundE_TOFRegionInteract_dist;
		// TH1F* h_foundE_TOFRegionInteract_DeltaPhiCuts_dist;
		// TH1F* h_foundE_FCALRegionInteract_dist;
		// TH1F* h_foundE_FCALRegionInteract_DeltaPhiCuts_dist;
		
		// TH1F* h_nonprimary_FCALTruthShower_opangle;
		// TH1F* h_nonprimary_TOFconv_FCALTruthShower_opangle;
		// TH1F* h_nonprimary_notTOFconv_FCALTruthShower_opangle;
		
		// TH1F* h_thrownE_pairprod;
		
		// DVector3 PropogateThrownNeutralToFCAL(DVector3 x4_init, DVector3 p4_init);
		
		//Store any histograms as member variables here
		//TTREE
		string photon_gun_tree_fname; 
		// DTreeInterface* dTreeInterface;
		// DTreeFillData dTreeFillData;
		
		Int_t verbosity_counter;
		Int_t DMCTrajectoryPoint_checker;
		
		//For making EventStore skims
		int Get_FileNumber(JEventLoop* locEventLoop) const;
		ofstream dEventStoreSkimStream;
};

#endif // _DEventProcessor_pi_pm_gun_hists_

