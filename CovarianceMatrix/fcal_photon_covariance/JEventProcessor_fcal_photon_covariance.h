// $Id$
//
//    File: JEventProcessor_fcal_photon_covariance.h
// Created: Mon Feb  8 15:12:19 EST 2016
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_fcal_photon_covariance_
#define _JEventProcessor_fcal_photon_covariance_

#include <JANA/JEventProcessor.h>

class JEventProcessor_fcal_photon_covariance:public jana::JEventProcessor{
	public:
		JEventProcessor_fcal_photon_covariance();
		~JEventProcessor_fcal_photon_covariance();
		const char* className(void){return "JEventProcessor_fcal_photon_covariance";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		int VERBOSE;
		float ENERGY;
		float THETA;
};

#endif // _JEventProcessor_fcal_photon_covariance_

