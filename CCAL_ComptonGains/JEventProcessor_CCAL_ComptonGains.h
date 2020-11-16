// $Id$
//
//    File: JEventProcessor_CCAL_ComptonGains.h
// Created: Mon Dec 03 16:04:16 EST 2019
// Creator: andrsmit (on Linux ifarm1402)
//

#ifndef _JEventProcessor_CCAL_ComptonGains_
#define _JEventProcessor_CCAL_ComptonGains_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"

#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>

#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>
#include <TOF/DTOFPoint.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRIGGER/DTrigger.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>

#include <HDGEOMETRY/DGeometry.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace jana;
using namespace std;


class JEventProcessor_CCAL_ComptonGains:public JEventProcessor {
 	
	public:
		
		JEventProcessor_CCAL_ComptonGains();
  		~JEventProcessor_CCAL_ComptonGains(){};
  		const char* className(void){return "JEventProcessor_CCAL_ComptonGains";}
		
	
 	private:
		
		jerror_t init(void);
  		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
  		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
  		jerror_t erun(void);
  		jerror_t fini(void);
		
		double bar2x( int bar );
		
		int check_TOF_matches( DVector3 pos, double t1, double rfTime, 
			DVector3 vertex, vector< const DTOFPoint* > tof_points );
		
		
		
		//-----      Cut Parameters      -----//
		
		
		const double RF_TIME_CUT = 2.004;
		
		int n_bunches_acc = 1;
		
		bool BYPASS_TRIG;
		
		double MIN_FCAL_ENERGY_CUT;
		double MIN_CCAL_ENERGY_CUT;
		double MIN_BEAM_ENERGY_CUT;
		
		double COPL_CUT;
		double DELTA_K_CUT;
		
		
		const double fcal_tof_xmu          =  0.5582;
		const double fcal_tof_xsig         =  1.8;
		const double fcal_tof_ymu          = -0.1844;
		const double fcal_tof_ysig         =  1.8;
		const double fcal_tof_x1           =  fcal_tof_xmu  -  3.*fcal_tof_xsig;
		const double fcal_tof_x2           =  fcal_tof_xmu  +  3.*fcal_tof_xsig;
		const double fcal_tof_y1           =  fcal_tof_ymu  -  3.*fcal_tof_ysig;
		const double fcal_tof_y2           =  fcal_tof_ymu  +  3.*fcal_tof_ysig;
		
		
		
		//-----    Physical Constants    -----//
		
		
		const double c    =  29.9792458;
		const double m_e  =  0.510998928e-3;
		
		
		double m_fcalX, m_fcalY, m_fcalZ;
		double m_ccalX, m_ccalY, m_ccalZ;
		double m_beamX, m_beamY, m_beamZ;
		
		
		//-----        Histograms        -----//
		
		
		TH1F *h_fcal_tof_dt;
		TH1F *h_fcal_tof_dx,     *h_fcal_tof_dy;
		TH1F *h_fcal_tof_dx_bar, *h_fcal_tof_dy_bar;
		
		TH1F *h_fcal_rf_dt, *h_ccal_rf_dt,   *h_tof_rf_dt;
		TH1F *h_beam_rf_dt, *h_beam_ccal_dt, *h_beam_fcal_dt;
		
		TH1F *h_deltaPhi, *h_deltaT;
		
		TH1F *h_deltaE,    *h_deltaK;
		
		TH2F *h_CompRatio, *h_CompRatio_g, *h_CompRatio_e;
		TH2F *h_CompDiff,  *h_CompDiff_g,  *h_CompDiff_e;
		
		TH2F *h_CompRatio_v_E, *h_CompRatio_v_E_e, *h_CompRatio_v_E_g;
		
		TH2F *h_xy_fcal,     *h_xy_ccal;
		
		TH1F *h_tof_match;
};

#endif // _JEventProcessor_CCAL_ComptonGains_

