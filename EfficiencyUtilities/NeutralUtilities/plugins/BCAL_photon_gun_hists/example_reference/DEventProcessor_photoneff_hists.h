// $Id$
//
//    File: DEventProcessor_photoneff_hists.h
// Created: Thu Feb 12 09:43:13 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DEventProcessor_photoneff_hists_
#define _DEventProcessor_photoneff_hists_

#include <pthread.h>
#include <map>
using std::map;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <JANA/JFactory.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEventLoop.h>

#include <PID/DKinematicData.h>
#include <PID/DPhoton.h>
#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrajectoryPoint.h>

#include "photon.h"


class DEventProcessor_photoneff_hists:public JEventProcessor{

	public:
		DEventProcessor_photoneff_hists();
		~DEventProcessor_photoneff_hists();

		TTree *phtneff;
		photon phtn;
		photon *phtn_ptr;


	private:
		jerror_t init(void);	///< Invoked via DEventProcessor virtual method
		jerror_t brun(JEventLoop *loop, int32_t runnumber);
		jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);	///< Invoked via DEventProcessor virtual method
		jerror_t erun(void);					///< Invoked via DEventProcessor virtual method
		jerror_t fini(void);					///< Invoked via DEventProcessor virtual method

		
		//Histograms
		TH1F* 
		
		
		bool isReconstructable(const DMCThrown *mcthrown, JEventLoop *loop);

		pthread_mutex_t mutex;
		
		int DEBUG;
		
};

#endif // _DEventProcessor_photoneff_hists_

