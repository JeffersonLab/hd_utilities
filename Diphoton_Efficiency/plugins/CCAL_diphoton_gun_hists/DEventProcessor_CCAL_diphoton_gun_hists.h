// $Id$
//
//    File: DEventProcessor_CCAL_diphoton_gun_hists.h
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_CCAL_diphoton_gun_hists_
#define _DEventProcessor_CCAL_diphoton_gun_hists_

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

class DEventProcessor_CCAL_diphoton_gun_hists : public jana::JEventProcessor
{
	public:
		const char* className(void){return "DEventProcessor_CCAL_diphoton_gun_hists";}
		double DELTAPHI_CUTPARM;
		double DELTATHETA_CUTPARM;
		double DELTAENERGY_CUTPARM;
		double DISTANCE_CUTPARM;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber);	///< Called every time a new run number is detected.
		jerror_t evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber);	///< Called every event.
		jerror_t erun(void);						///< Called every time run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

		DGeometry* GX_Geom;
		double CCALFrontFaceZ;
				
		TH1F* h_NCCALShowers;
		TH1F* h_thrownSeparation;
		TH1F* h_matchedSeparation;
		TH1F* h_matchedSeparation_2shower;
		TH1F* h_matchedSeparation_logd;
		TH1F* h_matchedSeparation_2shower_logd;
		TH1F* h_effSeparation;
		TH1F* h_effSeparation_2shower;
		TH1F* h_effSeparation_logd;
		TH1F* h_effSeparation_2shower_logd;
		TH1F* h_NMatched;
		TH1F* h_deltaPosition;
		
		DVector3 PropogateThrownNeutralToCCAL(DVector3 x4_init, DVector3 p4_init, double shower_z);
		
		//For making EventStore skims
		int Get_FileNumber(JEventLoop* locEventLoop) const;
		ofstream dEventStoreSkimStream;
};

#endif // _DEventProcessor_CCAL_diphoton_gun_hists_

