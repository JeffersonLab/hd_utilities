// $Id$
//
//    File: DEventProcessor_CCAL_photon_gun_hists.cc
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#include "DEventProcessor_CCAL_photon_gun_hists.h"

// Routine used to create our DEventProcessor


// CCAL: starts to quickly drop off at 10 degrees. Totally off by 11.5 degrees.



extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_CCAL_photon_gun_hists()); //register this plugin
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_CCAL_photon_gun_hists::init(void)
{
	// This is called once at program startup.

	DELTAPHI_CUTPARM = 3.35; //Degrees
	DELTATHETA_CUTPARM = 0.35; //Degrees
	DELTAENERGY_CUTPARM = 0.35; //GeV (resolution of CCAL at 1GeV is about 10.9 percent, this is a bit over 3 sigma)
	DISTANCE_CUTPARM = 4.0; //cm
	
	if(gPARMS) {
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTAPHI_CUTPARM",   DELTAPHI_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTATHETA_CUTPARM", DELTATHETA_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTAENERGY_CUTPARM", DELTAENERGY_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DISTANCE_CUTPARM", DISTANCE_CUTPARM);
	}
	
 	//Initialize histograms here

	h_NCCALShowers = new TH1F("h_NCCALShowers","Number of Showers In Event;Reconstructed CCAL Showers",45,0,15.);
	h_NMatched = new TH1F("h_NMATCHED","Number of Throwns Matched to Reconstructed Showers;Matched Throwns",6,0,2.);
	h_deltaPosition = new TH1F("h_deltaPosition","Distance Between Thrown and Reconstructed;Distance (cm)",1000,0.0,10.0);
	h_deltaPosition_logd = new TH1F("h_deltaPosition_logd","Distance Between Thrown and Reconstructed (Log Distance);Distance (cm)",1000,0.0,10.0);
	h_deltax = new TH1F("h_deltax","x_{Thrown} - x_{Recon};x_{Thrown} - x_{Recon} (cm)",1000,-5.0,5.0);
	h_deltay = new TH1F("h_deltay","y_{Thrown} - y_{Recon};y_{Thrown} - y_{Recon} (cm)",1000,-5.0,5.0);
	h_deltax_logd = new TH1F("h_deltax_logd","x1_{Thrown} - x1_{Recon} (Log Distance);x1_{Thrown} - x1_{Recon} (cm)",1000,-5.0,5.0);
	h_deltay_logd = new TH1F("h_deltay_logd","y1_{Thrown} - y1_{Recon} (Log Distance);y1_{Thrown} - y1_{Recon} (cm)",1000,-5.0,5.0);
	h_deltax_thrownx = new TH2F("h_deltax_thrownx","#Deltax vs. Thrown x-Position in Block (x, y);(x_{Thrown} - x_{Block}) / d;x_{Thrown} - x_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltax_matchedx = new TH2F("h_deltax_matchedx","#Deltax vs. Matched x-Position in Block (x, y);(x_{Recon} - x_{Block}) / d;x_{Thrown} - x_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_throwny = new TH2F("h_deltay_throwny","#Deltay vs. Thrown y-Position in Block (x, y);(y_{Thrown} - y_{Block}) / d;y_{Thrown} - y_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_matchedy = new TH2F("h_deltay_matchedy","#Deltay vs. Matched y-Position in Block (x, y);(y_{Recon} - y_{Block}) / d;y_{Thrown} - y_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltax_thrownx_logd = new TH2F("h_deltax_thrownx_logd","#Deltax vs. Thrown x-Position in Block (x1, y1);(x_{Thrown} - x_{Block}) / d;x_{Thrown} - x1_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltax_matchedx_logd = new TH2F("h_deltax_matchedx_logd","#Deltax vs. Matched x-Position in Block (x1, y1);(x1_{Recon} - x_{Block}) / d;x_{Thrown} - x1_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_throwny_logd = new TH2F("h_deltay_throwny_logd","#Deltay vs. Thrown y-Position in Block (x1, y1);(y_{Thrown} - y_{Block}) / d;y_{Thrown} - y1_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);
	h_deltay_matchedy_logd = new TH2F("h_deltay_matchedy_logd","#Deltay vs. Matched y-Position in Block (x1, y1);(y1_{Recon} - y_{Block}) / d;y_{Thrown} - y1_{Recon} (cm)",1200,-1.2,1.2,2000,-8.0,8.0);

	//REGISTER BRANCHES
	// dTreeInterface->Create_Branches(locBranchRegister);
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_CCAL_photon_gun_hists::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
{
	// This is called whenever the run number changes

	// GX_Geom = NULL;
	// DApplication *dapp = dynamic_cast< DApplication* >( locEventLoop->GetJApplication() );
	// if( dapp ) GX_Geom = dapp->GetDGeometry( locRunNumber );	
	// GX_Geom->GetCCALZ( CCALFrontFaceZ );

	ccalGeom = new DCCALGeometry();

	CCALFrontFaceZ = 1279.376;

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_CCAL_photon_gun_hists::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
{

	DVector3 vertex(0.0,0.0,800.0); // Custom vertex

	//Retrieve MCThrown stuff
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);

	//Check MCThrown info. Must not be empty, first entry must be a photon
	if(locMCThrowns.size()!=1) {
		cout << "ERROR! Event does not have exactly one thrown! Skipping event..." << endl;
		return NOERROR;
	}

	//Retrieve CCAL/BCAL showers
	vector<const DCCALShower*> DCCALShowers;
	locEventLoop->Get(DCCALShowers);

	double CCAL_energy = 0;
	for(int loc_i=0; loc_i<(int)DCCALShowers.size(); ++loc_i) CCAL_energy+=DCCALShowers[loc_i]->E;	

	DVector3 ThrownPhotonAtCCAL, ShowerPosition, ShowerPosition_logd;
	DVector3 ThrownPhotonAtCCALFace, ThrownPhotonAtCCALAlt;
	DVector2 BlockPosition, BlockPosition_logd, BlockPosition_thrown;
	int row, column, row_logd, column_logd, row_thrown, column_thrown;
	ThrownPhotonAtCCAL = PropogateThrownNeutralToCCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),CCALFrontFaceZ+8.0) - vertex;
	ThrownPhotonAtCCALAlt = PropogateThrownNeutralToCCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),CCALFrontFaceZ+15.0) - vertex;
	ThrownPhotonAtCCALFace = PropogateThrownNeutralToCCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),CCALFrontFaceZ) - vertex;

	bool passes_shower_req = false;
	int matched_showers = -1;
	int used_shower;
	Double_t max_E = 0.;
	for (int loc_j=0; loc_j<1; ++loc_j) { // loop over throwns
		double min_separation = 100.0; // Initialize to nonsense numbers
		double distance = 100.0;
		used_shower = -1;
		for(int loc_i=0; loc_i<(int)DCCALShowers.size(); ++loc_i) {
			ShowerPosition.SetX(DCCALShowers[loc_i]->x - vertex.X());
			ShowerPosition.SetY(DCCALShowers[loc_i]->y - vertex.Y());
			ShowerPosition.SetZ(DCCALShowers[loc_i]->z - vertex.Z());
			ShowerPosition_logd.SetX(DCCALShowers[loc_i]->x1 - vertex.X());
			ShowerPosition_logd.SetY(DCCALShowers[loc_i]->y1 - vertex.Y());
			ShowerPosition_logd.SetZ(DCCALShowers[loc_i]->z - vertex.Z());
cout.precision(4);
if (loc_j == 0 && loc_i == 0) {
	cout << endl << "Throwns:" << endl;
	cout << "0	" << ThrownPhotonAtCCALFace.X() << "	" << ThrownPhotonAtCCALFace.Y() << "	" << ThrownPhotonAtCCALFace.Z() + vertex.Z() << "	" << ThrownPhotonAtCCALFace.Phi()*180/3.1415 << "	" << ThrownPhotonAtCCALFace.Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << "	Face" << endl;
	cout << "	" << ThrownPhotonAtCCAL.X() << "	" << ThrownPhotonAtCCAL.Y() << "	" << ThrownPhotonAtCCAL.Z() + vertex.Z() << "	" << ThrownPhotonAtCCAL.Phi()*180/3.1415 << "	" << ThrownPhotonAtCCAL.Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << "	Face + 8 cm" << endl;
	cout << "	" << ThrownPhotonAtCCALAlt.X() << "	" << ThrownPhotonAtCCALAlt.Y() << "	" << ThrownPhotonAtCCALAlt.Z() + vertex.Z() << "	" << ThrownPhotonAtCCALAlt.Phi()*180/3.1415 << "	" << ThrownPhotonAtCCALAlt.Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << "	Face + 15 cm" << endl;
	cout << "vertex	" << vertex.X() << "	" << vertex.Y() << "	" << vertex.Z() << endl;
	cout << "x_init	" << locMCThrowns[0]->position().X() << "	" << locMCThrowns[0]->position().Y() << "	" << locMCThrowns[0]->position().Z() << endl;
	cout << "p_init	" << locMCThrowns[0]->momentum().X() << "	" << locMCThrowns[0]->momentum().Y() << "	" << locMCThrowns[0]->momentum().Z() << "	" << locMCThrowns[0]->momentum().Phi()*180/3.1415 << "	" << locMCThrowns[0]->momentum().Theta()*180/3.1415 << "	" << locMCThrowns[0]->momentum().Mag() << endl;
	cout << "Showers:" << endl;
}
			if(DCCALShowers[loc_i]->E > max_E) max_E = DCCALShowers[loc_i]->E;
			distance = (ShowerPosition - ThrownPhotonAtCCALFace).Perp();
if (loc_j == 0) cout << loc_i << "	" << DCCALShowers[loc_i]->x << "	" << DCCALShowers[loc_i]->y << "	" << DCCALShowers[loc_i]->z << "	" << ShowerPosition.Phi()*180/3.1415 << "	" << ShowerPosition.Theta()*180/3.1415 << "	" << DCCALShowers[loc_i]->E << "	Nominal" << endl;
if (loc_j == 0) cout << loc_i << "	" << DCCALShowers[loc_i]->x1 << "	" << DCCALShowers[loc_i]->y1 << "	" << DCCALShowers[loc_i]->z << "	" << ShowerPosition_logd.Phi()*180/3.1415 << "	" << ShowerPosition_logd.Theta()*180/3.1415 << "	" << DCCALShowers[loc_i]->E << "	Log-Weighted" << endl;
			if (/*distance < DISTANCE_CUTPARM && */fabs(DCCALShowers[loc_i]->E-locMCThrowns[loc_j]->energy()) < DELTAENERGY_CUTPARM) {
				passes_shower_req = true;
				if (distance < min_separation) {
					min_separation = distance;
					used_shower = loc_i;
				}
			}
		}
		matched_showers = used_shower;
cout << "Used Shower:	" << matched_showers << endl;
	}
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	if(passes_shower_req) {
		h_NMatched->Fill(1);
		ShowerPosition.SetX(DCCALShowers[matched_showers]->x - vertex.X());
		ShowerPosition.SetY(DCCALShowers[matched_showers]->y - vertex.Y());
		ShowerPosition.SetZ(DCCALShowers[matched_showers]->z - vertex.Z());
		ShowerPosition_logd.SetX(DCCALShowers[matched_showers]->x1 - vertex.X());
		ShowerPosition_logd.SetY(DCCALShowers[matched_showers]->y1 - vertex.Y());
		ShowerPosition_logd.SetZ(DCCALShowers[matched_showers]->z - vertex.Z());
		h_deltaPosition->Fill((ShowerPosition - ThrownPhotonAtCCALFace).Perp());
		h_deltaPosition_logd->Fill((ShowerPosition_logd - ThrownPhotonAtCCALFace).Perp());
		h_deltax->Fill(ThrownPhotonAtCCALFace.X()-ShowerPosition.X());
		h_deltay->Fill(ThrownPhotonAtCCALFace.Y()-ShowerPosition.Y());
		h_deltax_logd->Fill(ThrownPhotonAtCCALFace.X()-ShowerPosition_logd.X());
		h_deltay_logd->Fill(ThrownPhotonAtCCALFace.Y()-ShowerPosition_logd.Y());

// Should be able to check if insert exists to set in_insert accordingly... Not sure how to do that, though
		row_thrown = ccalGeom->row((float)ThrownPhotonAtCCALFace.Y());
		column_thrown = ccalGeom->column((float)ThrownPhotonAtCCALFace.X());
		BlockPosition_thrown = ccalGeom->positionOnFace(row_thrown,column_thrown);
		row = ccalGeom->row((float)ShowerPosition.Y());
		column = ccalGeom->column((float)ShowerPosition.X());
		BlockPosition = ccalGeom->positionOnFace(row,column);
		row_logd = ccalGeom->row((float)ShowerPosition_logd.Y());
		column_logd = ccalGeom->column((float)ShowerPosition_logd.X());
		BlockPosition_logd = ccalGeom->positionOnFace(row_logd,column_logd);

		h_deltax_thrownx->Fill((ThrownPhotonAtCCALFace.X()-BlockPosition_thrown.X())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.X()-ShowerPosition.X());
		h_deltax_matchedx->Fill((ShowerPosition.X()-BlockPosition.X())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.X()-ShowerPosition.X());
		h_deltax_thrownx_logd->Fill((ThrownPhotonAtCCALFace.X()-BlockPosition_thrown.X())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.X()-ShowerPosition_logd.X());
		h_deltax_matchedx_logd->Fill((ShowerPosition_logd.X()-BlockPosition_logd.X())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.X()-ShowerPosition_logd.X());

		h_deltay_throwny->Fill((ThrownPhotonAtCCALFace.Y()-BlockPosition_thrown.Y())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.Y()-ShowerPosition.Y());
		h_deltay_matchedy->Fill((ShowerPosition.Y()-BlockPosition.Y())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.Y()-ShowerPosition.Y());
		h_deltay_throwny_logd->Fill((ThrownPhotonAtCCALFace.Y()-BlockPosition_thrown.Y())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.Y()-ShowerPosition_logd.Y());
		h_deltay_matchedy_logd->Fill((ShowerPosition_logd.Y()-BlockPosition_logd.Y())/ccalGeom->blockSize(),ThrownPhotonAtCCALFace.Y()-ShowerPosition_logd.Y());
	}
	h_NCCALShowers->Fill(DCCALShowers.size());
	japp->RootUnLock(); //RELEASE ROOT LOCK!!

	return NOERROR;
}

int DEventProcessor_CCAL_photon_gun_hists::Get_FileNumber(JEventLoop* locEventLoop) const
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
jerror_t DEventProcessor_CCAL_photon_gun_hists::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_CCAL_photon_gun_hists::fini(void)
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
	
	// delete dTreeInterface; //saves trees to file, closes file

	return NOERROR;
}

//Convert thrown photon coordinates at primary vertex in target to coordinates at CCAL face 
DVector3 DEventProcessor_CCAL_photon_gun_hists::PropogateThrownNeutralToCCAL(const DVector3 x4_init, const DVector3 p4_init, double shower_z) {
	
	//tan(theta1) = delta X / delta Z = Px / Pz
	//tan(theta1) = delta X / delta Z = Py / Pz
	
	double DeltaZ = shower_z - x4_init.Z();
	
	double x_pos_AtCCAL = x4_init.X() + DeltaZ*(p4_init.Px()/p4_init.Pz());
	double y_pos_AtCCAL = x4_init.Y() + DeltaZ*(p4_init.Py()/p4_init.Pz());
	
	return DVector3(x_pos_AtCCAL,y_pos_AtCCAL,shower_z);
}

