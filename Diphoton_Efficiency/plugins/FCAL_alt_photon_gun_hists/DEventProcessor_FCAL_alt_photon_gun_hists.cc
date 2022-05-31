// $Id$
//
//    File: DEventProcessor_FCAL_alt_photon_gun_hists.cc
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#include "DEventProcessor_FCAL_alt_photon_gun_hists.h"

// Routine used to create our DEventProcessor


// FCAL: starts to quickly drop off at 10 degrees. Totally off by 11.5 degrees.



extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_FCAL_alt_photon_gun_hists()); //register this plugin
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_FCAL_alt_photon_gun_hists::init(void)
{
	// This is called once at program startup.

	DELTAPHI_CUTPARM = 3.35; //Degrees
	DELTATHETA_CUTPARM = 0.35; //Degrees
	DELTAENERGY_CUTPARM = 0.35; //GeV (resolution of FCAL at 1GeV is about 10.9 percent, this is a bit over 3 sigma)
	DISTANCE_CUTPARM = 2.0; //cm

	if(gPARMS) {
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTAPHI_CUTPARM",   DELTAPHI_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTATHETA_CUTPARM", DELTATHETA_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTAENERGY_CUTPARM", DELTAENERGY_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DISTANCE_CUTPARM", DISTANCE_CUTPARM);
	}

 	//Initialize histograms here

	h_NFCALShowers = new TH1F("h_NFCALShowers","Number of Showers In Event;Reconstructed FCAL Showers",45,0,15.);
	h_NMatched = new TH1F("h_NMATCHED","Number of Throwns Matched to Reconstructed Showers;Matched Throwns",6,0,2.);

	h_deltax_thrownx = new TH2F("h_deltax_thrownx","#Deltax vs. Thrown x-Position in Block;(x_{Thrown} - x_{Block}) / d;x_{Thrown} - x_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltax_matchedx = new TH2F("h_deltax_matchedx","#Deltax vs. Matched x-Position in Block;(x_{Recon} - x_{Block}) / d;x_{Thrown} - x_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_throwny = new TH2F("h_deltay_throwny","#Deltay vs. Thrown y-Position in Block;(y_{Thrown} - y_{Block}) / d;y_{Thrown} - y_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_matchedy = new TH2F("h_deltay_matchedy","#Deltay vs. Matched y-Position in Block;(y_{Recon} - y_{Block}) / d;y_{Thrown} - y_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltax_thrownx_logd = new TH2F("h_deltax_thrownx_logd","#Deltax vs. Thrown x-Position in Block (Log Distance);(x_{Thrown} - x_{Block}) / d;x_{Thrown} - x_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltax_matchedx_logd = new TH2F("h_deltax_matchedx_logd","#Deltax vs. Matched x-Position in Block (Log Distance);(x_{Recon} - x_{Block}) / d;x_{Thrown} - x_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_throwny_logd = new TH2F("h_deltay_throwny_logd","#Deltay vs. Thrown y-Position in Block (Log Distance);(y_{Thrown} - y_{Block}) / d;y_{Thrown} - y_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_matchedy_logd = new TH2F("h_deltay_matchedy_logd","#Deltay vs. Matched y-Position in Block (Log Distance);(y_{Recon} - y_{Block}) / d;y_{Thrown} - y_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltaPosition = new TH1F("h_deltaPosition","Distance Between Thrown and Reconstructed;Distance (cm)",1000,0.0,10.0);
	h_deltaPosition_logd = new TH1F("h_deltaPosition_logd","Distance Between Thrown and Reconstructed (Log Distance);Distance (cm)",1000,0.0,10.0);
	h_deltax = new TH1F("h_deltax","x_{Thrown} - x_{Recon};x_{Thrown} - x_{Recon} (cm)",1000,-5.0,5.0);
	h_deltay = new TH1F("h_deltay","y_{Thrown} - y_{Recon};y_{Thrown} - y_{Recon} (cm)",1000,-5.0,5.0);
	h_deltax_logd = new TH1F("h_deltax_logd","x1_{Thrown} - x1_{Recon} (Log Distance);x1_{Thrown} - x1_{Recon} (cm)",1000,-5.0,5.0);
	h_deltay_logd = new TH1F("h_deltay_logd","y1_{Thrown} - y1_{Recon} (Log Distance);y1_{Thrown} - y1_{Recon} (cm)",1000,-5.0,5.0);

	//REGISTER BRANCHES
	// dTreeInterface->Create_Branches(locBranchRegister);

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_FCAL_alt_photon_gun_hists::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
{
	// This is called whenever the run number changes

	 GX_Geom = NULL;
	 DApplication *dapp = dynamic_cast< DApplication* >( locEventLoop->GetJApplication() );
	 if( dapp ) GX_Geom = dapp->GetDGeometry( locRunNumber );	
	// GX_Geom->GetFCALZ( fcalFrontFaceZ );

	fcalGeom = new DFCALGeometry(GX_Geom);

	fcalFrontFaceZ = 625.;
//	fcalFrontFaceZ = 638.; // Empirically, this is the average z-position of reconstructed showers

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_FCAL_alt_photon_gun_hists::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
{

	DVector3 vertex(0.0,0.0,65.0);

	//Retrieve MCThrown stuff
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);

	//Check MCThrown info. Must not be empty, first entry must be a photon
	if(locMCThrowns.size()!=1) {
		cout << "ERROR! Event does not have exactly one thrown! Skipping event..." << endl;
		return NOERROR;
	}

	//Retrieve FCAL/BCAL showers
	vector<const DFCALShower*> DFCALShowers;
	locEventLoop->Get(DFCALShowers);

	double FCAL_energy = 0;
	for(int loc_i=0; loc_i<(int)DFCALShowers.size(); ++loc_i) FCAL_energy+=DFCALShowers[loc_i]->getEnergy();	

	DVector3 ThrownPhotonAtFCAL, ThrownPhotonAtFCAL_logd, ShowerPosition, ShowerPosition_logd;
	DVector2 BlockPosition, BlockPosition_logd, BlockPosition_thrown, BlockPosition_thrown_logd;
	int row, column, row_logd, column_logd, row_thrown, column_thrown, row_thrown_logd, column_thrown_logd;

	bool passes_shower_req = false;
	int matched_showers = -1;
	int used_shower;

	for (int loc_j=0; loc_j<1; ++loc_j) { // loop over throwns
		used_shower = -1;
		for(int loc_i=0; loc_i<(int)DFCALShowers.size(); ++loc_i) {
			if (fabs(DFCALShowers[loc_i]->getEnergy()-locMCThrowns[loc_j]->energy()) < DELTAENERGY_CUTPARM) {
				passes_shower_req = true;
				used_shower = loc_i;
			}
		}
		matched_showers = used_shower;
	}
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	if(passes_shower_req) {
		h_NMatched->Fill(1);
		ShowerPosition = DFCALShowers[matched_showers]->getPosition();
		ShowerPosition_logd = DFCALShowers[matched_showers]->getPosition_log();
		ThrownPhotonAtFCAL = PropogateThrownNeutralToFCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),ShowerPosition.Z());
		ThrownPhotonAtFCAL_logd = PropogateThrownNeutralToFCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),ShowerPosition_logd.Z());
		h_deltaPosition->Fill((ShowerPosition - ThrownPhotonAtFCAL).Perp());
		h_deltaPosition_logd->Fill((ShowerPosition_logd - ThrownPhotonAtFCAL_logd).Perp());
		h_deltax->Fill(ThrownPhotonAtFCAL.X()-ShowerPosition.X());
		h_deltay->Fill(ThrownPhotonAtFCAL.Y()-ShowerPosition.Y());
		h_deltax_logd->Fill(ThrownPhotonAtFCAL_logd.X()-ShowerPosition_logd.X());
		h_deltay_logd->Fill(ThrownPhotonAtFCAL_logd.Y()-ShowerPosition_logd.Y());

// Should be able to check if insert exists to set in_insert accordingly... Not sure how to do that, though
		row_thrown = fcalGeom->row(ThrownPhotonAtFCAL.Y(),false);
		column_thrown = fcalGeom->column(ThrownPhotonAtFCAL.X(),false);
		BlockPosition_thrown = fcalGeom->positionOnFace(row_thrown,column_thrown);
		row_thrown_logd = fcalGeom->row(ThrownPhotonAtFCAL_logd.Y(),false);
		column_thrown_logd = fcalGeom->column(ThrownPhotonAtFCAL_logd.X(),false);
		BlockPosition_thrown_logd = fcalGeom->positionOnFace(row_thrown_logd,column_thrown_logd);
		row = fcalGeom->row(ShowerPosition.Y(),false);
		column = fcalGeom->column(ShowerPosition.X(),false);
		BlockPosition = fcalGeom->positionOnFace(row,column);
		row_logd = fcalGeom->row(ShowerPosition_logd.Y(),false);
		column_logd = fcalGeom->column(ShowerPosition_logd.X(),false);
		BlockPosition_logd = fcalGeom->positionOnFace(row_logd,column_logd);

		h_deltax_thrownx->Fill((ThrownPhotonAtFCAL.X()-BlockPosition_thrown.X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL.X()-ShowerPosition.X());
		h_deltax_matchedx->Fill((ShowerPosition.X()-BlockPosition.X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL.X()-ShowerPosition.X());
		h_deltax_thrownx_logd->Fill((ThrownPhotonAtFCAL_logd.X()-BlockPosition_thrown_logd.X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL_logd.X()-ShowerPosition_logd.X());
		h_deltax_matchedx_logd->Fill((ShowerPosition_logd.X()-BlockPosition_logd.X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL_logd.X()-ShowerPosition_logd.X());

		h_deltay_throwny->Fill((ThrownPhotonAtFCAL.Y()-BlockPosition_thrown.Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL.Y()-ShowerPosition.Y());
		h_deltay_matchedy->Fill((ShowerPosition.Y()-BlockPosition.Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL.Y()-ShowerPosition.Y());
		h_deltay_throwny_logd->Fill((ThrownPhotonAtFCAL_logd.Y()-BlockPosition_thrown_logd.Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL_logd.Y()-ShowerPosition_logd.Y());
		h_deltay_matchedy_logd->Fill((ShowerPosition_logd.Y()-BlockPosition_logd.Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL_logd.Y()-ShowerPosition_logd.Y());



/*	cout.precision(4);
	cout << endl << "Geometry Tests:" << endl;
	cout << "0	" << ThrownPhotonAtFCAL[0].X() << "	" << ThrownPhotonAtFCAL[0].Y() << endl;
BlockPosition = fcalGeom->positionOnFace(fcalGeom->row(ThrownPhotonAtFCAL[0].Y(),false),fcalGeom->column(ThrownPhotonAtFCAL[0].X(),false));
	cout << "  nom	" << fcalGeom->column(ThrownPhotonAtFCAL[0].X(),false) << "	" << fcalGeom->row(ThrownPhotonAtFCAL[0].Y(),false) << "	" << BlockPosition.X() << "	" << BlockPosition.Y() << endl;
BlockPosition = fcalGeom->positionOnFace(fcalGeom->row(ThrownPhotonAtFCAL[0].Y(),true),fcalGeom->column(ThrownPhotonAtFCAL[0].X(),true));
	cout << "  ins	" << fcalGeom->column(ThrownPhotonAtFCAL[0].X(),true) << "	" << fcalGeom->row(ThrownPhotonAtFCAL[0].Y(),true) << "	" << BlockPosition.X() << "	" << BlockPosition.Y() << endl;
	cout << "1	" << ThrownPhotonAtFCAL[1].X() << "	" << ThrownPhotonAtFCAL[1].Y() << endl;
BlockPosition = fcalGeom->positionOnFace(fcalGeom->row(ThrownPhotonAtFCAL[1].Y(),false),fcalGeom->column(ThrownPhotonAtFCAL[1].X(),false));
	cout << "  nom	" << fcalGeom->column(ThrownPhotonAtFCAL[1].X(),false) << "	" << fcalGeom->row(ThrownPhotonAtFCAL[1].Y(),false) << "	" << BlockPosition.X() << "	" << BlockPosition.Y() << endl;
BlockPosition = fcalGeom->positionOnFace(fcalGeom->row(ThrownPhotonAtFCAL[1].Y(),true),fcalGeom->column(ThrownPhotonAtFCAL[1].X(),true));
	cout << "  ins	" << fcalGeom->column(ThrownPhotonAtFCAL[1].X(),true) << "	" << fcalGeom->row(ThrownPhotonAtFCAL[1].Y(),true) << "	" << BlockPosition.X() << "	" << BlockPosition.Y() << endl;
	cout << endl;
*/


	}
	else h_NMatched->Fill(0);
	h_NFCALShowers->Fill(DFCALShowers.size());
	japp->RootUnLock(); //RELEASE ROOT LOCK!!

	return NOERROR;
}

int DEventProcessor_FCAL_alt_photon_gun_hists::Get_FileNumber(JEventLoop* locEventLoop) const
{
	//Assume that the file name is in the format: *_X.ext, where:
		//X is the file number (a string of numbers of any length)
		//ext is the file extension (probably .evio or .hddm)

	//get the event source
	JEventSource* locEventSource = locEventLoop->GetJEvent().GetJEventSource();
	if(locEventSource == NULL)
		return -1;

	//get the source file name (strip the path)
	string locSourceFileName = locEventSource->GetSourceName();

	//find the last "_" & "." indices
	size_t locUnderscoreIndex = locSourceFileName.rfind("_");
	size_t locDotIndex = locSourceFileName.rfind(".");
	if((locUnderscoreIndex == string::npos) || (locDotIndex == string::npos))
		return -1;

	size_t locNumberLength = locDotIndex - locUnderscoreIndex - 1;
	string locFileNumberString = locSourceFileName.substr(locUnderscoreIndex + 1, locNumberLength);

	int locFileNumber = -1;
	istringstream locFileNumberStream(locFileNumberString);
	locFileNumberStream >> locFileNumber;

	return locFileNumber;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_FCAL_alt_photon_gun_hists::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_FCAL_alt_photon_gun_hists::fini(void)
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
	
	// delete dTreeInterface; //saves trees to file, closes file

	return NOERROR;
}

//Convert thrown photon coordinates at primary vertex in target to coordinates at FCAL face 
DVector3 DEventProcessor_FCAL_alt_photon_gun_hists::PropogateThrownNeutralToFCAL(const DVector3 x4_init, const DVector3 p4_init, double shower_z) {
	
	//tan(theta1) = delta X / delta Z = Px / Pz
	//tan(theta1) = delta X / delta Z = Py / Pz
	
	double DeltaZ = shower_z - x4_init.Z();
	
	double x_pos_AtFCAL = x4_init.X() + DeltaZ*(p4_init.Px()/p4_init.Pz());
	double y_pos_AtFCAL = x4_init.Y() + DeltaZ*(p4_init.Py()/p4_init.Pz());
	
	return DVector3(x_pos_AtFCAL,y_pos_AtFCAL,shower_z);
}

