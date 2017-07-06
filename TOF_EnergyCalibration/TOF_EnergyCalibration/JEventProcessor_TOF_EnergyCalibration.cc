// $Id$
//
//    File: JEventProcessor_TOF_EnergyCalibration.cc
// Created: Fri Jun 23 15:39:45 EDT 2017
// Creator: bcannon (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_TOF_EnergyCalibration.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_TOF_EnergyCalibration());
}
} // "C"


//------------------
// JEventProcessor_TOF_EnergyCalibration (Constructor)
//------------------
JEventProcessor_TOF_EnergyCalibration::JEventProcessor_TOF_EnergyCalibration()
{

}

//------------------
// ~JEventProcessor_TOF_EnergyCalibration (Destructor)
//------------------
JEventProcessor_TOF_EnergyCalibration::~JEventProcessor_TOF_EnergyCalibration()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_TOF_EnergyCalibration::init(void)
{
	// This is called once at program startup. 
	// Get top directory
	TDirectory *top = gDirectory;

  // create root file here so the tree does not show up in hd_root.root
  ROOTFile = new TFile("TOF_EnergyCalibrationMC.root","RECREATE");
  ROOTFile->cd();

	// Create 1D histograms to be used map<int,TH1I*> dHist_ADCEnergy
	char histname[128], histtitle[128];
	for (int pmt = 0; pmt < 176; ++pmt)
	{
		sprintf(histname,"ADCHists%d",pmt);
		sprintf(histtitle,"ADC Energy - PMT %d",pmt);
		dHist_ADCEnergy[pmt] = new TH1I(histname, histtitle, 200, 0.00, 0.005);
	}
	top->cd();
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TOF_EnergyCalibration::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TOF_EnergyCalibration::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);

	// Get TOFPoints
	vector<const DTOFPoint*> dTOFPoints;
	loop->Get(dTOFPoints);

	DVector3 pos;
	float dE;
	int dHorizontalBar, dVerticalBar, dHorizontalBarStatus, dVerticalBarStatus;
	set<int> pmt;
	for (unsigned int loc_i = 0; loc_i < dTOFPoints.size(); ++loc_i)
	{
		// Get measurements
		pos = dTOFPoints[loc_i]->pos;
		dHorizontalBar = dTOFPoints[loc_i]->dHorizontalBar;
		dVerticalBar = dTOFPoints[loc_i]->dVerticalBar;
		dHorizontalBarStatus = dTOFPoints[loc_i]->dHorizontalBarStatus;
		dVerticalBarStatus = dTOFPoints[loc_i]->dVerticalBarStatus;
		dE = dTOFPoints[loc_i]->dE;
		
		// Find PMT locations and fill appropriate histograms
		pmt = find_pmt_location(pos, dHorizontalBar, dVerticalBar, dHorizontalBarStatus, dVerticalBarStatus);
		for (set<int>::iterator loc_j = pmt.begin(); loc_j != pmt.end(); ++loc_j)
		{
			japp->RootFillLock(this);
				dHist_ADCEnergy[*loc_j]->Fill(dE);
			japp->RootFillUnLock(this);
		}
		pmt.clear();
	}


	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_TOF_EnergyCalibration::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TOF_EnergyCalibration::fini(void)
{
	// Called before program exit after event processing is finished.
	// Change to correct directory
	TDirectory * top = gDirectory;
  ROOTFile->cd();

	// Write the histograms
	for (int pmt = 0; pmt < 176; ++pmt)
	{
		dHist_ADCEnergy[pmt]->Write();
	}

	// Close file
	ROOTFile->Close();
	top->cd();


	return NOERROR;
}

set<int> JEventProcessor_TOF_EnergyCalibration::find_pmt_location(DVector3 loc_pos, int loc_HorizontalBar, int loc_VerticalBar, int loc_HorizontalBarStatus, int loc_VerticalBarStatus){

	// Initialize pmt numbers
	set<int> loc_pmt;
	int pmt_num_NORTH = -1;
	int pmt_num_SOUTH = -1;
	int pmt_num_TOP = -1;
	int pmt_num_BOTTOM = -1;
	
	// Set the pmt numbers
	if (fabs(loc_pos.x()) < 4.)
	{
		if ((loc_HorizontalBarStatus == 1) || ((loc_HorizontalBarStatus == 3)))
			pmt_num_NORTH = loc_HorizontalBar - 1 + 88 + 44;
		if ((loc_HorizontalBarStatus == 2) || ((loc_HorizontalBarStatus == 3)))
			pmt_num_SOUTH = loc_HorizontalBar - 1 + 88;
		if ((loc_VerticalBar==22) || (loc_VerticalBar==23)) // vertical single ended paddles
		{
			if ((loc_VerticalBarStatus == 1) || ((loc_VerticalBarStatus == 3)))
				pmt_num_TOP = loc_VerticalBar - 1;
			if ((loc_VerticalBarStatus == 2) || ((loc_VerticalBarStatus == 3)))
				pmt_num_BOTTOM = loc_VerticalBar - 1 + 44;
		}
	}
	if (fabs(loc_pos.y()) < 4.)
	{
		if ((loc_VerticalBarStatus == 1) || ((loc_VerticalBarStatus == 3)))
			pmt_num_TOP = loc_VerticalBar - 1;
		if ((loc_VerticalBarStatus == 2) || ((loc_VerticalBarStatus == 3)))
			pmt_num_BOTTOM = loc_VerticalBar - 1 + 44;
		if ((loc_HorizontalBar==22) || (loc_HorizontalBar==23)) // horizontal single ended paddles
		{
			if ((loc_HorizontalBarStatus == 1) || ((loc_HorizontalBarStatus == 3)))
				pmt_num_NORTH = loc_HorizontalBar - 1 + 88 + 44;
			if ((loc_HorizontalBarStatus == 2) || ((loc_HorizontalBarStatus == 3)))
				pmt_num_SOUTH = loc_HorizontalBar - 1 + 88;
		}
	} 

	// Record pmt numbers
	if (pmt_num_NORTH!=-1)
		loc_pmt.insert(pmt_num_NORTH);
	if (pmt_num_SOUTH!=-1)
		loc_pmt.insert(pmt_num_SOUTH);
	if (pmt_num_TOP!=-1)
		loc_pmt.insert(pmt_num_TOP);
	if (pmt_num_BOTTOM!=-1)
		loc_pmt.insert(pmt_num_BOTTOM);

	return loc_pmt;
}

