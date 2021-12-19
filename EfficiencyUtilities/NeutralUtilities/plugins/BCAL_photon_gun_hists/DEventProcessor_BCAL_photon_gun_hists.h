// $Id$
//
//    File: DEventProcessor_BCAL_photon_gun_hists.h
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_BCAL_photon_gun_hists_
#define _DEventProcessor_BCAL_photon_gun_hists_

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

class DEventProcessor_BCAL_photon_gun_hists : public jana::JEventProcessor
{
	public:
		const char* className(void){return "DEventProcessor_BCAL_photon_gun_hists";}

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
		float bcalInnerRadiusR;
		
		
		//Histograms
		// TH1F* h_thrownE_all_dist;
		// TH1F* h_thrownE_foundshow_dist;
		// TH1F* h_reconE_dist;
		
		TH1F* h_NBCALShowers;
		// TH1F* h_NFCALShowers_nonprimary;
		// TH1F* h_NFCALShowers_primary;
		// TH1F* h_NNeutralShowers;
		// TH1F* h_NNeutralShowersPreselect;
		
		TH1F* h_foundE_all_dist;
		TH1F* h_foundE_1show_dist;
		TH1F* h_foundE_DeltaPhiCuts_dist;
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
		
		TH1F* h_PhotonInteractionR;
		TH1F* h_PhotonInteractionZ;
		TH1F* h_PhotonInteractionMech;
		TH2F* h2_PhotonInteractionMechvsInteractionZ;
		// TH1F* h_PhotonInteractionCase;

		TH1F* h_ThrownPhotonE;
		
		TH1F* h_thrownTheta;
		TH1F* h_thrownPhi;
		TH1F* h_thrownZ;
		TH1F* h_DeltaPhi_all;
		TH1F* h_DeltaPhi_1show;
		TH1F* h_DeltaTheta_all;
		TH2F* h2_DeltaThetaDeltaPhi_all;
		TH2F* h2_DeltaThetaDeltaPhi_best;
		TH2F* h2_DeltaThetaDeltaPhi_notbest;
		TH1F* h_thrownZ_AtCentroid_ALL;
		TH1F* h_reconZ_AtCentroid_ALL;
		TH1F* h_thrownZ_AtCentroid_best;
		TH1F* h_reconZ_AtCentroid_best;
		TH1F* h_thrownR_AtCentroid;
		TH1F* h_reconR_AtCentroid_best;
		TH1F* h_reconR_AtCentroid_notbest;
		TH1F* h_reconR_AtCentroid_all;
		
		TH1F* h_reconTheta_AtCentroid;
		TH1F* h_DeltaZ_AtCentroid;
		TH1F* h_DeltaCosTheta_AtCentroid;
		
		TH1F* h_foundE_innerRegionInteract_dist;
		TH1F* h_foundE_innerRegionInteract_dist_DeltaPhiCuts_dist;
		TH1F* h_foundE_BCALRegionInteract_dist;
		TH1F* h_foundE_BCALRegionInteract_dist_DeltaPhiCuts_dist;
		
		
		
		// TH1F* h_nonprimary_FCALTruthShower_opangle;
		// TH1F* h_nonprimary_TOFconv_FCALTruthShower_opangle;
		// TH1F* h_nonprimary_notTOFconv_FCALTruthShower_opangle;
		
		// TH1F* h_thrownE_pairprod;
		
		DVector3 PropogateThrownNeutralToBCAL(const DVector3 x4_init, const DVector3 p4_init, const double RadiusToEvaluateAt=0.);
		
		//Store any histograms as member variables here
		//TTREE
		string photon_gun_tree_fname; 
		// DTreeInterface* dTreeInterface;
		// DTreeFillData dTreeFillData;
		
		Int_t verbosity_counter;
		Int_t verbosity_counter_theta;
		Int_t DMCTrajectoryPoint_checker;
		
		//For making EventStore skims
		int Get_FileNumber(JEventLoop* locEventLoop) const;
		ofstream dEventStoreSkimStream;
};

#endif // _DEventProcessor_BCAL_photon_gun_hists_

