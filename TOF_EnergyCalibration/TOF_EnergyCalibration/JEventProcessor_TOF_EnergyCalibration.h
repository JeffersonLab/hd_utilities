// $Id$
//
//    File: JEventProcessor_TOF_EnergyCalibration.h
// Created: Fri Jun 23 15:39:45 EDT 2017
// Creator: bcannon (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TOF_EnergyCalibration_
#define _JEventProcessor_TOF_EnergyCalibration_

#include <JANA/JEventProcessor.h>
#include <TOF/DTOFPoint.h>
#include <TFile.h>
#include <TH1F.h>


class JEventProcessor_TOF_EnergyCalibration:public jana::JEventProcessor{
	public:
		JEventProcessor_TOF_EnergyCalibration();
		~JEventProcessor_TOF_EnergyCalibration();
		const char* className(void){return "JEventProcessor_TOF_EnergyCalibration";}

		// Histograms
		map<int,TH1I*> dHist_ADCEnergy;

		// Output root file
		TFile * ROOTFile;

	private:
		set <int> find_pmt_location(DVector3 loc_pos, int loc_HorizontalBar, int loc_VerticalBar, int loc_HorizontalBarStatus, int loc_VerticalBarStatus);
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TOF_EnergyCalibration_

