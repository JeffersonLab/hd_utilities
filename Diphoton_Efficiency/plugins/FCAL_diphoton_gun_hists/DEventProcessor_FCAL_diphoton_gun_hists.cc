// $Id$
//
//    File: DEventProcessor_FCAL_diphoton_gun_hists.cc
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#include "DEventProcessor_FCAL_diphoton_gun_hists.h"

// Routine used to create our DEventProcessor


// FCAL: starts to quickly drop off at 10 degrees. Totally off by 11.5 degrees.



extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_FCAL_diphoton_gun_hists()); //register this plugin
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_FCAL_diphoton_gun_hists::init(void)
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
	h_thrownSeparation = new TH1F("h_thrownSeparation","Thrown Photon Separation;Separation (cm)",2000,0.0,20.0);
	h_matchedSeparation = new TH1F("h_matchedSeparation","Matched Photon Separation;Separation (cm)",2000,0.0,20.0);
	h_matchedSeparation_2shower = new TH1F("h_matchedSeparation_2shower","Matched Photon Separation (2 Reconstructed Showers);#Separation (cm)",2000,0.0,20.0);
	h_matchedSeparation_logd = new TH1F("h_matchedSeparation_logd","Matched Photon Separation (Log Distance);Separation (cm)",2000,0.0,20.0);
	h_matchedSeparation_2shower_logd = new TH1F("h_matchedSeparation_2shower_logd","Matched Photon Separation (Log Distance, 2 Reconstructed Showers);#Separation (cm)",2000,0.0,20.0);
 
	//REGISTER BRANCHES
	// dTreeInterface->Create_Branches(locBranchRegister);
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_FCAL_diphoton_gun_hists::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
{
	// This is called whenever the run number changes

	// GX_Geom = NULL;
	// DApplication *dapp = dynamic_cast< DApplication* >( locEventLoop->GetJApplication() );
	// if( dapp ) GX_Geom = dapp->GetDGeometry( locRunNumber );	
	// GX_Geom->GetFCALZ( fcalFrontFaceZ );

	fcalFrontFaceZ = 625.;
//	fcalFrontFaceZ = 638.; // Empirically, this is the average z-position of reconstructed showers

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_FCAL_diphoton_gun_hists::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
{

	DVector3 vertex(0.0,0.0,65.0);

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

	//Retrieve FCAL/BCAL showers
	vector<const DFCALShower*> DFCALShowers;
	locEventLoop->Get(DFCALShowers);

	double FCAL_energy = 0;
	double separation = 0;
	for(int loc_i=0; loc_i<(int)DFCALShowers.size(); ++loc_i) FCAL_energy+=DFCALShowers[loc_i]->getEnergy();	

	DVector3 ThrownPhotonAtFCAL[2], Difference, ShowerPosition, ShowerPosition_logd;
	ThrownPhotonAtFCAL[0] = PropogateThrownNeutralToFCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),638.0) - vertex; // Get approximate positions for cout purposes
	ThrownPhotonAtFCAL[1] = PropogateThrownNeutralToFCAL(locMCThrowns[1]->position(),locMCThrowns[1]->momentum(),638.0) - vertex;
	Difference = ThrownPhotonAtFCAL[0] - ThrownPhotonAtFCAL[1];
	separation = Difference.Perp();
	h_thrownSeparation->Fill(separation) ;

	bool passes_shower_req[2] = {false, false};
	bool passes_shower_req_logd[2] = {false, false};
	int matched_showers[2] = {-1, -1};
	int matched_showers_logd[2] = {-1, -1};
	int used_shower, used_shower_logd;
	Double_t max_E = 0.;
	for (int loc_j=0; loc_j<2; ++loc_j) { // loop over throwns
		double min_separation = 100.0; // Initialize to nonsense numbers
		double min_separation_logd = 100.0;
		double distance = 100.0;
		double distance_logd = 100.0;
		used_shower = -1;
		used_shower_logd = -1;
		for(int loc_i=0; loc_i<(int)DFCALShowers.size(); ++loc_i) {
			ShowerPosition = DFCALShowers[loc_i]->getPosition() - vertex;
			ShowerPosition_logd = DFCALShowers[loc_i]->getPosition_log() - vertex;
cout.precision(4);
if (loc_j == 0 && loc_i == 0) {
	cout << endl << "Throwns:" << endl;
	cout << "0	" << ThrownPhotonAtFCAL[0].X() << "	" << ThrownPhotonAtFCAL[0].Y() << "	" << ThrownPhotonAtFCAL[0].Z() + vertex.Z()  << "	" << ThrownPhotonAtFCAL[0].Phi()*180/3.1415 << "	" << ThrownPhotonAtFCAL[0].Theta()*180/3.1415 << "	" << locMCThrowns[0]->energy() << endl;
	cout << "1	" << ThrownPhotonAtFCAL[1].X() << "	" << ThrownPhotonAtFCAL[1].Y() << "	" << ThrownPhotonAtFCAL[1].Z() + vertex.Z()  << "	" << ThrownPhotonAtFCAL[1].Phi()*180/3.1415 << "	" << ThrownPhotonAtFCAL[1].Theta()*180/3.1415 << "	" << locMCThrowns[1]->energy() << endl;
	cout << "Separation:" << separation << endl;
	cout << "Showers:" << endl;
}
if (loc_j == 0) cout << loc_i << "	" << ShowerPosition.X() << "	" << ShowerPosition.Y() << "	" << ShowerPosition.Z() + vertex.Z() << "	" << ShowerPosition.Phi()*180/3.1415 << "	" << ShowerPosition.Theta()*180/3.1415 << "	" << DFCALShowers[loc_i]->getEnergy() << endl << "	" << ShowerPosition_logd.X() << "	" << ShowerPosition_logd.Y() << "	" << ShowerPosition_logd.Z() + vertex.Z() << "	" << ShowerPosition_logd.Phi()*180/3.1415 << "	" << ShowerPosition_logd.Theta()*180/3.1415 << endl;

			if (loc_i == matched_showers[0] && loc_i == matched_showers_logd[0]) continue; // Skip this shower if it was matched to the first thrown in both cases
			if(DFCALShowers[loc_i]->getEnergy() > max_E) max_E = DFCALShowers[loc_i]->getEnergy();
		// +/- 3.35 degree default cut in phi: about 5 sigma cut (with 1.2 GeV test photon at 6 degrees, sigma = 0.67 degrees)
		// +/- 0.35 degree default cut in theta: about 5 sigma cut (with 1.2 GeV test photon at 6 degrees, sigma = 0.0682 degrees)
			ThrownPhotonAtFCAL[loc_j] = PropogateThrownNeutralToFCAL(locMCThrowns[loc_j]->position(),locMCThrowns[loc_j]->momentum(),ShowerPosition.Z()) - vertex; // Get position based on reconstructed shower z
			distance = (ShowerPosition - ThrownPhotonAtFCAL[loc_j]).Perp();
			distance_logd = (ShowerPosition_logd - ThrownPhotonAtFCAL[loc_j]).Perp();
			if (loc_i != matched_showers[0] && distance < DISTANCE_CUTPARM && fabs(DFCALShowers[loc_i]->getEnergy()-locMCThrowns[loc_j]->energy()) < DELTAENERGY_CUTPARM) {
				passes_shower_req[loc_j] = true;
				if (distance < min_separation) {
					min_separation = distance;
					used_shower = loc_i;
				}
			}
			if (loc_i != matched_showers_logd[0] && distance_logd < DISTANCE_CUTPARM && fabs(DFCALShowers[loc_i]->getEnergy()-locMCThrowns[loc_j]->energy()) < DELTAENERGY_CUTPARM) {
				passes_shower_req_logd[loc_j] = true;
				if (distance_logd < min_separation_logd) {
					min_separation_logd = distance_logd;
					used_shower_logd = loc_i;
				}
			}
		}
		matched_showers[loc_j] = used_shower;
		matched_showers_logd[loc_j] = used_shower_logd;
cout << "Used Shower:	" << matched_showers[loc_j] << "	" << matched_showers_logd[loc_j] << endl;
//if (used_shower_logd != -1) cout << "Energy difference = " << fabs(DFCALShowers[used_shower_logd]->getEnergy()-locMCThrowns[loc_j]->energy()) << endl;
	}
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	if(passes_shower_req[0] && passes_shower_req[1]) {
		if (DFCALShowers.size() == 2) h_matchedSeparation_2shower->Fill(separation);
		h_matchedSeparation->Fill(separation);
	}
	if(passes_shower_req_logd[0] && passes_shower_req_logd[1]) {
		if (DFCALShowers.size() == 2) h_matchedSeparation_2shower_logd->Fill(separation);
		h_matchedSeparation_logd->Fill(separation);
	}
	h_NFCALShowers->Fill(DFCALShowers.size());
	japp->RootUnLock(); //RELEASE ROOT LOCK!!

	return NOERROR;
}

int DEventProcessor_FCAL_diphoton_gun_hists::Get_FileNumber(JEventLoop* locEventLoop) const
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
jerror_t DEventProcessor_FCAL_diphoton_gun_hists::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_FCAL_diphoton_gun_hists::fini(void)
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

	h_effSeparation_logd = (TH1F*)h_matchedSeparation_logd->Clone("h_effSeparation_logd");
	h_effSeparation_logd->Divide(h_thrownSeparation);
	h_effSeparation_logd->SetTitle("Efficiency (Matched/Thrown) by Separation (Log Distance);Separation (cm)");

	h_effSeparation_2shower_logd = (TH1F*)h_matchedSeparation_2shower_logd->Clone("h_effSeparation_2shower_logd");
	h_effSeparation_2shower_logd->Divide(h_thrownSeparation);
	h_effSeparation_2shower_logd->SetTitle("Efficiency (Matched/Thrown) by Separation (Log Distance, 2 Reconstructed Shower);Separation (cm)");

	return NOERROR;
}

//Convert thrown photon coordinates at primary vertex in target to coordinates at FCAL face 
DVector3 DEventProcessor_FCAL_diphoton_gun_hists::PropogateThrownNeutralToFCAL(const DVector3 x4_init, const DVector3 p4_init, double shower_z) {
	
	//tan(theta1) = delta X / delta Z = Px / Pz
	//tan(theta1) = delta X / delta Z = Py / Pz
	
	double DeltaZ = shower_z - x4_init.Z();
	
	double x_pos_AtFCAL = x4_init.X() + DeltaZ*(p4_init.Px()/p4_init.Pz());
	double y_pos_AtFCAL = x4_init.Y() + DeltaZ*(p4_init.Py()/p4_init.Pz());
	
	return DVector3(x_pos_AtFCAL,y_pos_AtFCAL,shower_z);
}

