// $Id$
//
//    File: DEventProcessor_FCAL_alt_diphoton_gun_hists.cc
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#include "DEventProcessor_FCAL_alt_diphoton_gun_hists.h"

// Routine used to create our DEventProcessor


// FCAL: starts to quickly drop off at 10 degrees. Totally off by 11.5 degrees.



extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_FCAL_alt_diphoton_gun_hists()); //register this plugin
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_FCAL_alt_diphoton_gun_hists::init(void)
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
	h_rawSeparation_2shower = new TH1F("h_rawSeparation_2shower","Photon Separation (2 Reconstructed Showers);#Separation (cm)",2000,0.0,20.0);

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

	//REGISTER BRANCHES
	// dTreeInterface->Create_Branches(locBranchRegister);

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_FCAL_alt_diphoton_gun_hists::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
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
jerror_t DEventProcessor_FCAL_alt_diphoton_gun_hists::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
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

	DVector3 ThrownPhotonAtFCAL[2], Difference, ShowerPosition[2], ShowerPosition_logd[2];
	DVector2 BlockPosition[2], BlockPosition_logd[2], BlockPosition_thrown[2];
	int row[2], column[2], row_logd[2], column_logd[2], row_thrown[2], column_thrown[2];
	ThrownPhotonAtFCAL[0] = PropogateThrownNeutralToFCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),638.0) - vertex; // Get approximate positions for cout purposes
	ThrownPhotonAtFCAL[1] = PropogateThrownNeutralToFCAL(locMCThrowns[1]->position(),locMCThrowns[1]->momentum(),638.0) - vertex;
	Difference = ThrownPhotonAtFCAL[0] - ThrownPhotonAtFCAL[1];
	separation = Difference.Perp();
	h_thrownSeparation->Fill(separation) ;

	bool passes_shower_req[2] = {false, false};
	int matched_showers[2] = {-1, -1};
	int used_shower;
	Double_t max_E = 0.;
	for (int loc_j=0; loc_j<2; ++loc_j) { // loop over throwns
		used_shower = -1;
		for(int loc_i=0; loc_i<(int)DFCALShowers.size(); ++loc_i) {
			if (loc_i == matched_showers[0]) continue; // Skip this shower if it was matched to the first thrown
			if(DFCALShowers[loc_i]->getEnergy() > max_E) max_E = DFCALShowers[loc_i]->getEnergy();
		// +/- 3.35 degree default cut in phi: about 5 sigma cut (with 1.2 GeV test photon at 6 degrees, sigma = 0.67 degrees)
		// +/- 0.35 degree default cut in theta: about 5 sigma cut (with 1.2 GeV test photon at 6 degrees, sigma = 0.0682 degrees)
			if (fabs(DFCALShowers[loc_i]->getEnergy()-locMCThrowns[loc_j]->energy()) < DELTAENERGY_CUTPARM) {
				passes_shower_req[loc_j] = true;
				used_shower = loc_i;
			}
		}
		matched_showers[loc_j] = used_shower;
	}
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	if(passes_shower_req[0] && passes_shower_req[1]) {
		if (DFCALShowers.size() == 2) h_matchedSeparation_2shower->Fill(separation);
		h_matchedSeparation->Fill(separation);

		ShowerPosition[0] = DFCALShowers[matched_showers[0]]->getPosition();
		ShowerPosition_logd[0] = DFCALShowers[matched_showers[0]]->getPosition_log();
		ShowerPosition[1] = DFCALShowers[matched_showers[1]]->getPosition();
		ShowerPosition_logd[1] = DFCALShowers[matched_showers[1]]->getPosition_log();
		ThrownPhotonAtFCAL[0] = PropogateThrownNeutralToFCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),ShowerPosition[0].Z());
		ThrownPhotonAtFCAL[1] = PropogateThrownNeutralToFCAL(locMCThrowns[1]->position(),locMCThrowns[1]->momentum(),ShowerPosition[1].Z());
		h_deltaPosition->Fill((ShowerPosition[0] - ThrownPhotonAtFCAL[0]).Perp());
		h_deltaPosition->Fill((ShowerPosition[1] - ThrownPhotonAtFCAL[1]).Perp());
		h_deltaPosition_logd->Fill((ShowerPosition_logd[0] - ThrownPhotonAtFCAL[0]).Perp());
		h_deltaPosition_logd->Fill((ShowerPosition_logd[1] - ThrownPhotonAtFCAL[1]).Perp());
// Should be able to check if insert exists to set in_insert accordingly... Not sure how to do that, though
		row_thrown[0] = fcalGeom->row(ThrownPhotonAtFCAL[0].Y(),false);
		column_thrown[0] = fcalGeom->column(ThrownPhotonAtFCAL[0].X(),false);
		BlockPosition_thrown[0] = fcalGeom->positionOnFace(row_thrown[0],column_thrown[0]);
		row_thrown[1] = fcalGeom->row(ThrownPhotonAtFCAL[1].Y(),false);
		column_thrown[1] = fcalGeom->column(ThrownPhotonAtFCAL[1].X(),false);
		BlockPosition_thrown[1] = fcalGeom->positionOnFace(row_thrown[1],column_thrown[1]);
		row[0] = fcalGeom->row(ShowerPosition[0].Y(),false);
		column[0] = fcalGeom->column(ShowerPosition[0].X(),false);
		BlockPosition[0] = fcalGeom->positionOnFace(row[0],column[0]);
		row[1] = fcalGeom->row(ShowerPosition[1].Y(),false);
		column[1] = fcalGeom->column(ShowerPosition[1].X(),false);
		BlockPosition[1] = fcalGeom->positionOnFace(row[1],column[1]);
		row_logd[0] = fcalGeom->row(ShowerPosition_logd[0].Y(),false);
		column_logd[0] = fcalGeom->column(ShowerPosition_logd[0].X(),false);
		BlockPosition_logd[0] = fcalGeom->positionOnFace(row_logd[0],column_logd[0]);
		row_logd[1] = fcalGeom->row(ShowerPosition_logd[1].Y(),false);
		column_logd[1] = fcalGeom->column(ShowerPosition_logd[1].X(),false);
		BlockPosition_logd[1] = fcalGeom->positionOnFace(row_logd[1],column_logd[1]);

		h_deltax_thrownx->Fill((ThrownPhotonAtFCAL[0].X()-BlockPosition_thrown[0].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].X()-ShowerPosition[0].X());
		h_deltax_matchedx->Fill((ShowerPosition[0].X()-BlockPosition[0].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].X()-ShowerPosition[0].X());
		h_deltax_thrownx_logd->Fill((ThrownPhotonAtFCAL[0].X()-BlockPosition_thrown[0].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].X()-ShowerPosition_logd[0].X());
		h_deltax_matchedx_logd->Fill((ShowerPosition_logd[0].X()-BlockPosition_logd[0].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].X()-ShowerPosition_logd[0].X());

		h_deltay_throwny->Fill((ThrownPhotonAtFCAL[0].Y()-BlockPosition_thrown[0].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].Y()-ShowerPosition[0].Y());
		h_deltay_matchedy->Fill((ShowerPosition[0].Y()-BlockPosition[0].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].Y()-ShowerPosition[0].Y());
		h_deltay_throwny_logd->Fill((ThrownPhotonAtFCAL[0].Y()-BlockPosition_thrown[0].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].Y()-ShowerPosition_logd[0].Y());
		h_deltay_matchedy_logd->Fill((ShowerPosition_logd[0].Y()-BlockPosition_logd[0].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[0].Y()-ShowerPosition_logd[0].Y());

		h_deltax_thrownx->Fill((ThrownPhotonAtFCAL[1].X()-BlockPosition_thrown[1].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].X()-ShowerPosition[1].X());
		h_deltax_matchedx->Fill((ShowerPosition[1].X()-BlockPosition[1].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].X()-ShowerPosition[1].X());
		h_deltax_thrownx_logd->Fill((ThrownPhotonAtFCAL[1].X()-BlockPosition_thrown[1].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].X()-ShowerPosition_logd[1].X());
		h_deltax_matchedx_logd->Fill((ShowerPosition_logd[1].X()-BlockPosition_logd[1].X())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].X()-ShowerPosition_logd[1].X());

		h_deltay_throwny->Fill((ThrownPhotonAtFCAL[1].Y()-BlockPosition_thrown[1].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].Y()-ShowerPosition[1].Y());
		h_deltay_matchedy->Fill((ShowerPosition[1].Y()-BlockPosition[1].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].Y()-ShowerPosition[1].Y());
		h_deltay_throwny_logd->Fill((ThrownPhotonAtFCAL[1].Y()-BlockPosition_thrown[1].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].Y()-ShowerPosition_logd[1].Y());
		h_deltay_matchedy_logd->Fill((ShowerPosition_logd[1].Y()-BlockPosition_logd[1].Y())/fcalGeom->blockSize(),ThrownPhotonAtFCAL[1].Y()-ShowerPosition_logd[1].Y());



	cout.precision(4);
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



	}
	if(DFCALShowers.size() == 2) {
		h_rawSeparation_2shower->Fill(separation);
	}
	h_NFCALShowers->Fill(DFCALShowers.size());
	japp->RootUnLock(); //RELEASE ROOT LOCK!!

	return NOERROR;
}

int DEventProcessor_FCAL_alt_diphoton_gun_hists::Get_FileNumber(JEventLoop* locEventLoop) const
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
jerror_t DEventProcessor_FCAL_alt_diphoton_gun_hists::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_FCAL_alt_diphoton_gun_hists::fini(void)
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

	h_effSeparation_raw2shower = (TH1F*)h_rawSeparation_2shower->Clone("h_effSeparation_raw2shower");
	h_effSeparation_raw2shower->Divide(h_thrownSeparation);
	h_effSeparation_raw2shower->SetTitle("Efficiency (2 Showers/Thrown) by Separation;Separation (cm)");

	return NOERROR;
}

//Convert thrown photon coordinates at primary vertex in target to coordinates at FCAL face 
DVector3 DEventProcessor_FCAL_alt_diphoton_gun_hists::PropogateThrownNeutralToFCAL(const DVector3 x4_init, const DVector3 p4_init, double shower_z) {
	
	//tan(theta1) = delta X / delta Z = Px / Pz
	//tan(theta1) = delta X / delta Z = Py / Pz
	
	double DeltaZ = shower_z - x4_init.Z();
	
	double x_pos_AtFCAL = x4_init.X() + DeltaZ*(p4_init.Px()/p4_init.Pz());
	double y_pos_AtFCAL = x4_init.Y() + DeltaZ*(p4_init.Py()/p4_init.Pz());
	
	return DVector3(x_pos_AtFCAL,y_pos_AtFCAL,shower_z);
}

