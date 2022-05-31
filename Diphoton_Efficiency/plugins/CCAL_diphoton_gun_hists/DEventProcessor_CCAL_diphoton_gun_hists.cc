// $Id$
//
//    File: DEventProcessor_CCAL_diphoton_gun_hists.cc
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#include "DEventProcessor_CCAL_diphoton_gun_hists.h"

// Routine used to create our DEventProcessor


// CCAL: starts to quickly drop off at 10 degrees. Totally off by 11.5 degrees.



extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_CCAL_diphoton_gun_hists()); //register this plugin
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_CCAL_diphoton_gun_hists::init(void)
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
	h_thrownSeparation = new TH1F("h_thrownSeparation","Thrown Photon Separation;Separation (cm)",2000,0.0,20.0);
	h_matchedSeparation = new TH1F("h_matchedSeparation","Matched Photon Separation;Separation (cm)",2000,0.0,20.0);
	h_matchedSeparation_2shower = new TH1F("h_matchedSeparation_2shower","Matched Photon Separation (2 Reconstructed Showers);#Separation (cm)",2000,0.0,20.0);
	h_NMatched = new TH1F("h_NMATCHED","Number of Throwns Matched to Reconstructed Showers;Matched Throwns",6,0,2.);
	h_deltaPosition = new TH1F("h_deltaPosition","Distance Between Thrown and Reconstructed (x1, y1);Distance (cm)",1000,0.0,10.0);

	//REGISTER BRANCHES
	// dTreeInterface->Create_Branches(locBranchRegister);
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_CCAL_diphoton_gun_hists::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
{
	// This is called whenever the run number changes

	// GX_Geom = NULL;
	// DApplication *dapp = dynamic_cast< DApplication* >( locEventLoop->GetJApplication() );
	// if( dapp ) GX_Geom = dapp->GetDGeometry( locRunNumber );	
	// GX_Geom->GetCCALZ( CCALFrontFaceZ );

	CCALFrontFaceZ = 1279.376;

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_CCAL_diphoton_gun_hists::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
{

	DVector3 vertex(0.0,0.0,800.0); // Custom vertex

	//Retrieve MCThrown stuff
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);

	//Check MCThrown info. Must not be empty, first entry must be a photon
	if(locMCThrowns.size()!=2) {
		cout << "ERROR! Event does not have exactly two thrown! Skipping event..." << endl;
		return NOERROR;
	}

	auto locThrownPhoton1P4 = locMCThrowns[0]->lorentzMomentum();
	auto locThrownPhoton2P4 = locMCThrowns[1]->lorentzMomentum();	

	//Retrieve CCAL/BCAL showers
	vector<const DCCALShower*> DCCALShowers;
	locEventLoop->Get(DCCALShowers);

	double CCAL_energy = 0;
	double separation = 0;
	for(int loc_i=0; loc_i<(int)DCCALShowers.size(); ++loc_i) CCAL_energy+=DCCALShowers[loc_i]->E;	

	DVector3 ThrownPhotonAtCCAL[2], Difference, ShowerPosition[2], ShowerPositionTemp;
	DVector3 ThrownPhotonAtCCALFace[2], ThrownPhotonAtCCALAlt[2];
	ThrownPhotonAtCCAL[0] = PropogateThrownNeutralToCCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),CCALFrontFaceZ+8.0) - vertex;
	ThrownPhotonAtCCAL[1] = PropogateThrownNeutralToCCAL(locMCThrowns[1]->position(),locMCThrowns[1]->momentum(),CCALFrontFaceZ+8.0) - vertex;
	ThrownPhotonAtCCALAlt[0] = PropogateThrownNeutralToCCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),CCALFrontFaceZ+15.0) - vertex;
	ThrownPhotonAtCCALAlt[1] = PropogateThrownNeutralToCCAL(locMCThrowns[1]->position(),locMCThrowns[1]->momentum(),CCALFrontFaceZ+15.0) - vertex;
	ThrownPhotonAtCCALFace[0] = PropogateThrownNeutralToCCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),CCALFrontFaceZ) - vertex;
	ThrownPhotonAtCCALFace[1] = PropogateThrownNeutralToCCAL(locMCThrowns[1]->position(),locMCThrowns[1]->momentum(),CCALFrontFaceZ) - vertex;
	Difference = ThrownPhotonAtCCAL[0] - ThrownPhotonAtCCAL[1];
	separation = Difference.Perp();
	h_thrownSeparation->Fill(separation) ;

	bool passes_shower_req[2] = {false, false};
	int matched_showers[2] = {-1, -1};
	int used_shower;
	Double_t max_E = 0.;
	for (int loc_j=0; loc_j<2; ++loc_j) { // loop over throwns
		double min_separation = 100.0; // Initialize to nonsense numbers
		double distance = 100.0;
		used_shower = -1;
		for(int loc_i=0; loc_i<(int)DCCALShowers.size(); ++loc_i) {
			ShowerPositionTemp.SetX(DCCALShowers[loc_i]->x1 - vertex.X());
			ShowerPositionTemp.SetY(DCCALShowers[loc_i]->y1 - vertex.Y());
			ShowerPositionTemp.SetZ(DCCALShowers[loc_i]->z - vertex.Z());
cout.precision(4);
if (loc_j == 0 && loc_i == 0) {
	cout << endl << "Throwns:" << endl;
	cout << "0	" << ThrownPhotonAtCCALFace[0].X() << "	" << ThrownPhotonAtCCALFace[0].Y() << "	" << ThrownPhotonAtCCALFace[0].Z() + vertex.Z() << "	" << ThrownPhotonAtCCALFace[0].Phi()*180/3.1415 << "	" << ThrownPhotonAtCCALFace[0].Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << "	Face" << endl;
	cout << "	" << ThrownPhotonAtCCAL[0].X() << "	" << ThrownPhotonAtCCAL[0].Y() << "	" << ThrownPhotonAtCCAL[0].Z() + vertex.Z() << "	" << ThrownPhotonAtCCAL[0].Phi()*180/3.1415 << "	" << ThrownPhotonAtCCAL[0].Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << "	Face + 8 cm" << endl;
	cout << "	" << ThrownPhotonAtCCALAlt[0].X() << "	" << ThrownPhotonAtCCALAlt[0].Y() << "	" << ThrownPhotonAtCCALAlt[0].Z() + vertex.Z() << "	" << ThrownPhotonAtCCALAlt[0].Phi()*180/3.1415 << "	" << ThrownPhotonAtCCALAlt[0].Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << "	Face + 15 cm" << endl;
	cout << "1	" << ThrownPhotonAtCCALFace[1].X() << "	" << ThrownPhotonAtCCALFace[1].Y() << "	" << ThrownPhotonAtCCALFace[1].Z() + vertex.Z() << "	" <<ThrownPhotonAtCCALFace[1].Phi()*180/3.1415 << "	" << ThrownPhotonAtCCALFace[1].Theta()*180/3.1415 << "	" << locMCThrowns[1]->energy() << "	Face" << endl;
	cout << "	" << ThrownPhotonAtCCAL[1].X() << "	" << ThrownPhotonAtCCAL[1].Y() << "	" << ThrownPhotonAtCCAL[1].Z() + vertex.Z() << "	" <<ThrownPhotonAtCCAL[1].Phi()*180/3.1415 << "	" << ThrownPhotonAtCCAL[1].Theta()*180/3.1415 << "	" << locMCThrowns[1]->energy() << "	Face + 8 cm" << endl;
	cout << "	" << ThrownPhotonAtCCALAlt[1].X() << "	" << ThrownPhotonAtCCALAlt[1].Y() << "	" << ThrownPhotonAtCCALAlt[1].Z() + vertex.Z() << "	" <<ThrownPhotonAtCCALAlt[1].Phi()*180/3.1415 << "	" << ThrownPhotonAtCCALAlt[1].Theta()*180/3.1415 << "	" << locMCThrowns[1]->energy() << "	Face + 15 cm" << endl;
	cout << "Separation:" << separation << endl;
	cout << "Showers:" << endl;
}
			if (loc_i == matched_showers[0]) continue; // Skip this shower if it was matched to the first thrown
			if(DCCALShowers[loc_i]->E > max_E) max_E = DCCALShowers[loc_i]->E;
			distance = (ShowerPositionTemp - ThrownPhotonAtCCALFace[loc_j]).Perp();
if (loc_j == 0) cout << loc_i << "	" << DCCALShowers[loc_i]->x << "	" << DCCALShowers[loc_i]->y << "	" << DCCALShowers[loc_i]->z << "	" << ShowerPositionTemp.Phi()*180/3.1415 << "	" << ShowerPositionTemp.Theta()*180/3.1415 << "	" << DCCALShowers[loc_i]->E << endl;
if (loc_j == 0) cout << " x1,y1	" << DCCALShowers[loc_i]->x1 << "	" << DCCALShowers[loc_i]->y1 << endl;
			if (distance < DISTANCE_CUTPARM && fabs(DCCALShowers[loc_i]->E-locMCThrowns[loc_j]->energy()) < DELTAENERGY_CUTPARM) {
				passes_shower_req[loc_j] = true;
				if (distance < min_separation) {
					min_separation = distance;
					used_shower = loc_i;
				}
			}
		}
		matched_showers[loc_j] = used_shower;
cout << "Used Shower:	" << matched_showers[loc_j] << endl;
	}
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	if((passes_shower_req[0] && !passes_shower_req[1]) || (passes_shower_req[1] && !passes_shower_req[0])) h_NMatched->Fill(1);
	if(passes_shower_req[0] && passes_shower_req[1]) {
		if (DCCALShowers.size() == 2) h_matchedSeparation_2shower->Fill(separation);
		h_matchedSeparation->Fill(separation);
		ShowerPosition[0].SetX(DCCALShowers[matched_showers[0]]->x1 - vertex.X());
		ShowerPosition[0].SetY(DCCALShowers[matched_showers[0]]->y1 - vertex.Y());
		ShowerPosition[0].SetZ(DCCALShowers[matched_showers[0]]->z - vertex.Z());
		ShowerPosition[1].SetX(DCCALShowers[matched_showers[1]]->x1 - vertex.X());
		ShowerPosition[1].SetY(DCCALShowers[matched_showers[1]]->y1 - vertex.Y());
		ShowerPosition[1].SetZ(DCCALShowers[matched_showers[1]]->z - vertex.Z());
		h_deltaPosition->Fill((ShowerPosition[0] - ThrownPhotonAtCCALFace[0]).Perp());
		h_deltaPosition->Fill((ShowerPosition[1] - ThrownPhotonAtCCALFace[1]).Perp());
		h_NMatched->Fill(2);
	}
	h_NCCALShowers->Fill(DCCALShowers.size());
	japp->RootUnLock(); //RELEASE ROOT LOCK!!

	return NOERROR;
}

int DEventProcessor_CCAL_diphoton_gun_hists::Get_FileNumber(JEventLoop* locEventLoop) const
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
jerror_t DEventProcessor_CCAL_diphoton_gun_hists::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_CCAL_diphoton_gun_hists::fini(void)
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
	
	// delete dTreeInterface; //saves trees to file, closes file

	h_effSeparation = (TH1F*)h_matchedSeparation->Clone("h_effSeparation");
	h_effSeparation->Divide(h_thrownSeparation);
	h_effSeparation->SetTitle("Efficiency (Matched/Thrown) by Separation;Separation (cm)");

	h_effSeparation_2shower = (TH1F*)h_matchedSeparation_2shower->Clone("h_effSeparation_2shower");
	h_effSeparation_2shower->Divide(h_thrownSeparation);
	h_effSeparation_2shower->SetTitle("Efficiency (Matched/Thrown) by Separation (2 Reconstructed Shower);Separation (cm)");

	return NOERROR;
}

//Convert thrown photon coordinates at primary vertex in target to coordinates at CCAL face 
DVector3 DEventProcessor_CCAL_diphoton_gun_hists::PropogateThrownNeutralToCCAL(const DVector3 x4_init, const DVector3 p4_init, double shower_z) {
	
	//tan(theta1) = delta X / delta Z = Px / Pz
	//tan(theta1) = delta X / delta Z = Py / Pz
	
	double DeltaZ = shower_z - x4_init.Z();
	
	double x_pos_AtCCAL = x4_init.X() + DeltaZ*(p4_init.Px()/p4_init.Pz());
	double y_pos_AtCCAL = x4_init.Y() + DeltaZ*(p4_init.Py()/p4_init.Pz());
	
	return DVector3(x_pos_AtCCAL,y_pos_AtCCAL,shower_z);
}

