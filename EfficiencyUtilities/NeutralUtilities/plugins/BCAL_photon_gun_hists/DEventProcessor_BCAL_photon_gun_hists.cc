// $Id$
//
//    File: DEventProcessor_BCAL_photon_gun_hists.cc
// Created: Thu Aug 23 11:25:21 EDT 2018
// Creator: jzarling (on Linux stanley.physics.indiana.edu 2.6.32-696.18.7.el6.x86_64 x86_64)
//

#include "DEventProcessor_BCAL_photon_gun_hists.h"

// Routine used to create our DEventProcessor


// FCAL: starts to quickly drop off at 10 degrees. Totally off by 11.5 degrees.



extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_BCAL_photon_gun_hists()); //register this plugin
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_BCAL_photon_gun_hists::init(void)
{
	// This is called once at program startup.

	DELTAPHI_CUTPARM = 2.0; //Degrees, roughly 5 sigma at 15 degrees
	DELTATHETA_CUTPARM = 7; //Degrees, this one is left extremely wide
	EMIN_CUTPARM = -1.; //GeV
	EMAX_CUTPARM = 1000; //GeV
	THETAMIN_CUTPARM = 0; //Degrees
	THETAMAX_CUTPARM = 1000; //Degrees
	
	if(gPARMS) {
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTAPHI_CUTPARM",   DELTAPHI_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:DELTATHETA_CUTPARM", DELTATHETA_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:EMIN_CUTPARM",   EMIN_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:EMAX_CUTPARM", EMAX_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:THETAMIN_CUTPARM",   THETAMIN_CUTPARM);
	  gPARMS->SetDefaultParameter("PHOTONHISTS:THETAMAX_CUTPARM", THETAMAX_CUTPARM);
	}
	
	// Create directory
	TDirectory *dir = (TDirectory*)gROOT->FindObject("BCAL_photon_gun_hists");
	if(!dir) dir = new TDirectoryFile("BCAL_photon_gun_hists","BCAL_photon_gun_hists");
	dir->cd();
	
	
 	//Initialize histograms here
	h_foundE_all_dist = new TH1F("h_foundE_all_dist","Found Photon Energy Distribution;Reconstructed Energy (GeV)",1000,0,10.);
	h_foundE_1show_dist = new TH1F("h_foundE_1show_dist","Found Photon Energy Distribution;Reconstructed Energy (GeV)",1000,0,10.);
	h_foundE_DeltaPhiCuts_dist = new TH1F("h_foundE_DeltaPhiCuts_dist","Found Photon Energy Distribution;Reconstructed Energy (GeV)",1000,0,10.);
	
	h_PhotonInteractionR = new TH1F("h_PhotonInteractionR","Photon Interaction R Position (cm);Primary Photon Interaction R (cm)",10000,0,150.);
	h_PhotonInteractionZ = new TH1F("h_PhotonInteractionZ","Photon Interaction Z Position (cm);Primary Photon Interaction Z (cm)",10000,40,900.);
	h_PhotonInteractionMech = new TH1F("h_PhotonInteractionMech","Photon Interaction mech;Reaction Mechanism",201,0,20.);
	h2_PhotonInteractionMechvsInteractionZ = new TH2F("h2_PhotonInteractionMechvsInteractionZ","Photon Interaction mech vs Interaction Z;Primary Photon Interaction Z (cm);Reaction Mechanism",1000,40,900.,201,0,20.);
	// h_PhotonInteractionCase = new TH1F("h_PhotonInteractionCase","",1000,0,5.);
	
	h_NBCALShowers = new TH1F("h_NBCALShowers","Number of Showers In Event (all cases);Number of FCAL Showers Reconstructed",1000,0,15.);
	
	h_ThrownPhotonE = new TH1F("h_ThrownPhotonE","Thrown Photon Energy;Thrown Photon Energy (GeV)",1000,0,10.);
	h_thrownTheta = new TH1F("h_thrownTheta","Thrown Photon #theta;Thrown Photon #theta (degrees)",10000,0,180.);
	h_thrownPhi = new TH1F("h_thrownPhi","Thrown Photon #phi;Thrown Photon #phi (degrees)",10000,-180.,180.);
	h_thrownZ = new TH1F("h_thrownZ","Thrown Photon Z;Thrown Photon Z (cm)",10000,0,650.);
	h_thrownZ_AtCentroid_ALL = new TH1F("h_thrownZ_AtCentroid_ALL","Thrown Photon Z Propogated to Centroid, all showers;Thrown Photon Z (cm)",10000,0,650.);
	h_reconZ_AtCentroid_ALL = new TH1F("h_reconZ_AtCentroid_ALL","Reconstructed Shower Z at Centroid, all showers;Reconstructed Z (cm)",10000,0,650.);
	h_thrownZ_AtCentroid_best = new TH1F("h_thrownZ_AtCentroid_best","Thrown Photon Z Propogated to Centroid, best matched shower only;Thrown Photon Z (cm)",10000,0,650.);
	h_reconZ_AtCentroid_best = new TH1F("h_reconZ_AtCentroid_best","Reconstructed Shower Z at Centroid, best matched shower only;Reconstructed Z (cm)",10000,0,650.);
	
	h_thrownR_AtCentroid = new TH1F("h_thrownR_AtCentroid","Thrown Shower R at Centroid;Thrown R (cm)",10000,0,150.);
	h_reconR_AtCentroid_best = new TH1F("h_reconR_AtCentroid_best","Recon Shower R at Centroid (closest shower);Reconstructed R (cm)",10000,0,150.);
	h_reconR_AtCentroid_notbest = new TH1F("h_reconR_AtCentroid_notbest","Recon Shower R at Centroid (not closest shower);Reconstructed R (cm)",10000,0,150.);
	h_reconR_AtCentroid_all = new TH1F("h_reconR_AtCentroid_best","Recon Shower R at Centroid (all showers);Reconstructed R (cm)",10000,0,150.);
	
	h_reconTheta_AtCentroid = new TH1F("h_reconTheta_AtCentroid","Reconstructed Shower #theta at Centroid;Reconstructed #theta (degrees)",10000,0,180.);
	h_DeltaZ_AtCentroid = new TH1F("h_DeltaZ_AtCentroid","#Delta Z at Centroid;Z (cm)",10000,-650,650.);
	h_DeltaCosTheta_AtCentroid = new TH1F("h_DeltaCosTheta_AtCentroid","Reconstructed #Delta Cos(#theta) at Centroid;Reconstructed #theta (degrees)",10000,-0.1,0.1);
	
	h2_DeltaThetaDeltaPhi_all = new TH2F("h2_DeltaThetaDeltaPhi_all","#Delta #phi thrown and shower vs. #Delta #theta thrown and shower;#Delta#theta, thrown vs. reconstructed #gamma (degrees);#Delta#phi, thrown vs. reconstructed #gamma (degrees)",1000,-90.,90,1000,-180.,180.);
	h2_DeltaThetaDeltaPhi_best = new TH2F("h2_DeltaThetaDeltaPhi_best","#Delta #phi thrown and shower vs. #Delta #theta thrown and shower;#Delta#theta, thrown vs. reconstructed #gamma (degrees);#Delta#phi, thrown vs. reconstructed #gamma (degrees)",1000,-90.,90,1000,-180.,180.);
	h2_DeltaThetaDeltaPhi_notbest = new TH2F("h2_DeltaThetaDeltaPhi_notbest","#Delta #phi thrown and shower vs. #Delta #theta thrown and shower;#Delta#theta, thrown vs. reconstructed #gamma (degrees);#Delta#phi, thrown vs. reconstructed #gamma (degrees)",1000,-90.,90,1000,-180.,180.);
	h_DeltaPhi_all = new TH1F("h_DeltaPhi_all","#Delta #phi thrown and shower;#Delta#phi, thrown vs. reconstructed #gamma (degrees)",10000,-180.,180.);
	h_DeltaPhi_1show = new TH1F("h_DeltaPhi_1show","#Delta #phi thrown and shower",10000,-180.,180.);
	h_DeltaTheta_all = new TH1F("h_DeltaTheta_all","#Delta #theta thrown and shower;#Delta#theta, thrown vs. reconstructed #gamma (degrees)",10000,-90.,90);
	
	
	
	
	//Histograms separated by where primary photon interacts. No consideration to secondaries here.
	//Barrel region: includes target, SC, CDC, FDC
	//AND upstream until arriving at TOF, hopefully name isn't too misleading
	// h_foundE_BarrelRegionInteract_dist = new TH1F("h_foundE_BarrelRegionInteract_dist","Found Photon Energy Distribution, Interacts In Barrel Region",1000,0,10.);
	// h_foundE_BarrelRegionInteract_DeltaPhiCuts_dist = new TH1F("h_foundE_BarrelRegionInteract_DeltaPhiCuts_dist","Found Photon Energy Distribution, Interacts In Barrel Region",1000,0,10.);
	//TOF region: includes TOF and FCAL darkroom material, hopefully name isn't too misleading
	// h_foundE_TOFRegionInteract_dist = new TH1F("h_foundE_TOFRegionInteract_dist","Found Photon Energy Distribution, Interacts In TOF Region",1000,0,10.);
	// h_foundE_TOFRegionInteract_DeltaPhiCuts_dist = new TH1F("h_foundE_TOFRegionInteract_DeltaPhiCuts_dist","Found Photon Energy Distribution, Interacts In TOF Region",1000,0,10.);
	//FCAL region: only FCAL material proper and a little behind
	// h_foundE_FCALRegionInteract_dist = new TH1F("h_foundE_FCALRegionInteract_dist","Found Photon Energy Distribution, Interacts In FCAL Region",1000,0,10.);
	// h_foundE_FCALRegionInteract_DeltaPhiCuts_dist = new TH1F("h_foundE_FCALRegionInteract_DeltaPhiCuts_dist","Found Photon Energy Distribution, Interacts In FCAL Region",1000,0,10.);
	h_foundE_innerRegionInteract_dist = new TH1F("h_foundE_innerRegionInteract_dist","Found Photon Energy Distribution, Interacts In Inner Radius;Reconstructed Energy (GeV)",1000,0,10.);
	h_foundE_innerRegionInteract_dist_DeltaPhiCuts_dist = new TH1F("h_foundE_innerRegionInteract_dist_DeltaPhiCuts_dist","Found Photon Energy Distribution, Interacts In Inner Radius;Reconstructed Energy (GeV)",1000,0,10.);
	h_foundE_BCALRegionInteract_dist = new TH1F("h_foundE_BCALRegionInteract_dist","Found Photon Energy Distribution, Interacts In BCAL;Reconstructed Energy (GeV)",1000,0,10.);
	h_foundE_BCALRegionInteract_dist_DeltaPhiCuts_dist = new TH1F("h_foundE_BCALRegionInteract_dist_DeltaPhiCuts_dist","Found Photon Energy Distribution, Interacts In BCAL;Reconstructed Energy (GeV)",1000,0,10.);
	
	
	
	//For all non-primary truth showers, plot opening angle between thrown photon and each shower 
  
	
	
	
	
	//REGISTER BRANCHES
	// dTreeInterface->Create_Branches(locBranchRegister);
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_BCAL_photon_gun_hists::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
{
	// This is called whenever the run number changes

	GX_Geom = NULL;
	DApplication *dapp = dynamic_cast< DApplication* >( locEventLoop->GetJApplication() );
	if( dapp ) GX_Geom = dapp->GetDGeometry( locRunNumber );	
	GX_Geom->GetBCALRmin( bcalInnerRadiusR );

	verbosity_counter = 0;
	verbosity_counter_theta = 0;
	DMCTrajectoryPoint_checker = 0;
	
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_BCAL_photon_gun_hists::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
{
	
	//Retrieve MCThrown stuff
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);


	//Check MCThrown info. Must not be empty, first entry must be a photon
	if(locMCThrowns.size()==0) {
		cout << "ERROR! Thrown MC info empty! Skipping event..." << endl;
		return NOERROR;
	}
	if(locMCThrowns.size()==1&&locMCThrowns[0]->PID()!=Gamma) {
		cout << "Error: Correct number of MCThrown objects, but wrong PID" << endl;
		return NOERROR;
	}	
	if(locMCThrowns.size()==2&&locMCThrowns[0]->PID()!=Gamma&&locMCThrowns[1]->PID()!=Gamma) {
		cout << "Error: MCThrown object is not photon PID" << endl;
		return NOERROR;
	}	
	if(locMCThrowns.size()==3&&locMCThrowns[0]->PID()!=Gamma&&locMCThrowns[1]->PID()!=Gamma&&locMCThrowns[2]->PID()!=Gamma) {
		cout << "Error: MCThrown object is not photon PID" << endl;
		return NOERROR;
	}	
	if(locMCThrowns.size()==4&&locMCThrowns[0]->PID()!=Gamma&&locMCThrowns[1]->PID()!=Gamma&&locMCThrowns[2]->PID()!=Gamma&&locMCThrowns[3]->PID()!=Gamma) {
		cout << "Error: MCThrown object is not photon PID" << endl;
		return NOERROR;
	}	
	if(locMCThrowns.size()>4&&locMCThrowns[0]->PID()!=Gamma&&locMCThrowns[1]->PID()!=Gamma&&locMCThrowns[2]->PID()!=Gamma&&locMCThrowns[3]->PID()!=Gamma&&locMCThrowns[4]->PID()!=Gamma) {
		cout << "Haven't found a photon in the first five indices, I don't know what to do with this event... Skipping..." << endl;
		return NOERROR;
	}	
	
	const DMCThrown* locThrownPhoton = locMCThrowns[0];
	
	if(locMCThrowns.size()>=2) {
		if(verbosity_counter<=2) cout << "Warning: multiple final state photons found in list of thrown products. Hists only use first photon in MCThrown list!! " << endl;
		

		for(int loc_i=0; loc_i<locMCThrowns.size(); ++loc_i) {
			verbosity_counter++;
			if(locMCThrowns[loc_i]->PID()==Gamma) {
				locThrownPhoton = locMCThrowns[loc_i];
				break;
				}
		}
	}
	
	auto locThrownPhotonP4 = locThrownPhoton->lorentzMomentum();
	
	
	//Cuts on sample, before entering any hists
	if(locThrownPhotonP4.E() < EMIN_CUTPARM || locThrownPhotonP4.E() > EMAX_CUTPARM) return NOERROR;
	if(locThrownPhotonP4.Theta()*180/3.14159 < THETAMIN_CUTPARM || locThrownPhotonP4.Theta()*180/3.14159 > THETAMAX_CUTPARM) return NOERROR;
	
	
	
	//Retrieve DMCTrajectoryPoint stuff
	vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints;
	locEventLoop->Get(locDMCTrajectoryPoints);
	bool valid_DMCTrajectoryInfo = false;
	
	// cout << "Size of trajectory points: " << locDMCTrajectoryPoints.size() << endl;
	if(locDMCTrajectoryPoints.size() == 0) {
		DMCTrajectoryPoint_checker++;
		if(DMCTrajectoryPoint_checker <=15) cout << "Warning: DMCTrajectoryPoint data is empty. Histograms that rely on this information will be empty. Check that your control.in file used for generation has flag TRAJECTORIES set to 1 or more" <<endl;
	}
	
	//If DMCThrown.size() is not 1, then I don't know what indexing should be.
	if(locDMCTrajectoryPoints.size() >= 1 && locMCThrowns.size() == 1) valid_DMCTrajectoryInfo = true;
	
	if(valid_DMCTrajectoryInfo) {
		japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
		double locPrimaryZ = locDMCTrajectoryPoints[1]->z;
		double locPrimaryR = sqrt(locDMCTrajectoryPoints[1]->x*locDMCTrajectoryPoints[1]->x+locDMCTrajectoryPoints[1]->y*locDMCTrajectoryPoints[1]->y);
		h_PhotonInteractionR->Fill(locPrimaryR);
		h_PhotonInteractionZ->Fill(locPrimaryZ);
		h_PhotonInteractionMech->Fill(locDMCTrajectoryPoints[1]->mech);
		h2_PhotonInteractionMechvsInteractionZ->Fill(locPrimaryZ,locDMCTrajectoryPoints[1]->mech);
		japp->RootUnLock(); //RELEASE ROOT LOCK!!	
	}
	
	
	
	//Retrieve FCAL/BCAL showers
	vector<const DFCALShower*> DFCALShowers;
	locEventLoop->Get(DFCALShowers);
	vector<const DBCALShower*> DBCALShowers;
	locEventLoop->Get(DBCALShowers);

	double FCAL_energy = 0;
	for(int loc_i=0; loc_i<DFCALShowers.size(); ++loc_i) FCAL_energy+=DFCALShowers[loc_i]->getEnergy();
	double BCAL_energy = 0;
	for(int loc_i=0; loc_i<DBCALShowers.size(); ++loc_i) BCAL_energy+=DBCALShowers[loc_i]->E;
	
	
	
	DVector3 ThrownPhotonAtBCAL;
	if(locMCThrowns.size()>=1) {
		ThrownPhotonAtBCAL = PropogateThrownNeutralToBCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum());
		h_ThrownPhotonE->Fill(locThrownPhotonP4.E());
		h_thrownTheta->Fill( (locThrownPhotonP4.Theta())*180/3.1415) ;
		h_thrownPhi->Fill( (locThrownPhotonP4.Phi())*180/3.1415) ;
		h_thrownZ->Fill( locMCThrowns[0]->position().Z()) ;
	}
	else {
		cout << "Error, no stuff found, skipping... " << endl;
		return NOERROR;
	}
	
	
	Double_t max_E = 0.;
	Double_t min_angle = 10000.;
	for(int loc_i=0; loc_i<DBCALShowers.size(); ++loc_i) {
		
		double bcal_show_E = DBCALShowers[loc_i]->E;
		
		if(bcal_show_E > max_E) max_E = bcal_show_E;
		bool passes_shower_req = false;
		// +/- 3.35 degree default cut in phi: about 5 sigma cut (with 1.2 GeV test photon at 6 degrees, sigma = 0.67 degrees)
		// +/- 0.35 degree default cut in theta: about 5 sigma cut (with 1.2 GeV test photon at 6 degrees, sigma = 0.0682 degrees)
		DVector3 BCAL_show_position = DVector3(DBCALShowers[loc_i]->x,DBCALShowers[loc_i]->y,DBCALShowers[loc_i]->z);
		// double shower_theta = (BCAL_show_position-locMCThrowns[0]->position()).Theta()*180/3.14159;
		//WRONG!!!! // double shower_theta = BCAL_show_position.Angle(locMCThrowns[0]->position())*180/3.14159;
		double shower_phi   = BCAL_show_position.Phi()*180/3.14159;
		double shower_radius   = sqrt(DBCALShowers[loc_i]->x*DBCALShowers[loc_i]->x+DBCALShowers[loc_i]->y*DBCALShowers[loc_i]->y);
		double DeltaZ_recon = (BCAL_show_position.Z()-locMCThrowns[0]->position().Z());
		double shower_theta = TMath::ATan(shower_radius/DeltaZ_recon)*180/3.14159;
		if(shower_theta<0) shower_theta+= 180.;
		
		double delta_phi = shower_phi-locThrownPhotonP4.Phi()*180/3.1415;
		double delta_theta = shower_theta-locThrownPhotonP4.Theta()*180/3.1415;
		
		if( fabs(delta_phi) < DELTAPHI_CUTPARM && fabs(delta_theta)< DELTATHETA_CUTPARM ) passes_shower_req = true;
		
		japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
		h_foundE_all_dist->Fill(bcal_show_E) ;
		if(DBCALShowers.size()==1 && passes_shower_req) h_foundE_1show_dist->Fill(bcal_show_E) ;
		if(passes_shower_req) h_foundE_DeltaPhiCuts_dist->Fill(bcal_show_E);
		h_DeltaPhi_all->Fill( shower_phi-locThrownPhotonP4.Phi()*180/3.1415) ;
		if(DBCALShowers.size()==1) h_DeltaPhi_1show->Fill( shower_phi-locThrownPhotonP4.Phi()*180/3.1415) ;
		h_DeltaTheta_all->Fill( shower_theta-locThrownPhotonP4.Theta()*180/3.1415) ;
		h2_DeltaThetaDeltaPhi_all->Fill( shower_theta-locThrownPhotonP4.Theta()*180/3.1415, shower_phi-ThrownPhotonAtBCAL.Phi()*180/3.1415) ;
		
		h_reconTheta_AtCentroid->Fill(shower_theta);
		h_DeltaZ_AtCentroid->Fill(BCAL_show_position.Z()-locMCThrowns[0]->position().Z());
		double CosThetaThrown = TMath::Cos(locThrownPhotonP4.Theta());
		double CosThetaRecon = TMath::Cos(shower_theta*3.14159/180.);
		h_DeltaCosTheta_AtCentroid->Fill(CosThetaThrown-CosThetaRecon);
		
		double angle = BCAL_show_position.Angle(ThrownPhotonAtBCAL);
		if(angle < min_angle) min_angle = angle;
		
		if(valid_DMCTrajectoryInfo) {
			// double locPrimaryZ = locDMCTrajectoryPoints[1]->z;
			double locPrimaryR = sqrt(locDMCTrajectoryPoints[1]->x*locDMCTrajectoryPoints[1]->x*+locDMCTrajectoryPoints[1]->y*locDMCTrajectoryPoints[1]->y);
			// if(40.<locPrimaryZ&&locPrimaryZ<605.4) h_foundE_BarrelRegionInteract_dist->Fill(bcal_show_E);
			// if(605.4<locPrimaryZ&&locPrimaryZ<624.7) h_foundE_TOFRegionInteract_dist->Fill(bcal_show_E);
			// if(624.7<locPrimaryZ&&locPrimaryZ<690.) h_foundE_FCALRegionInteract_dist->Fill(bcal_show_E);
			if(locPrimaryR<64.) h_foundE_innerRegionInteract_dist->Fill(bcal_show_E);
			if(locPrimaryR>64.) h_foundE_BCALRegionInteract_dist->Fill(bcal_show_E);
		
			if(passes_shower_req)
				// if(40.<locPrimaryZ&&locPrimaryZ<605.4) h_foundE_BarrelRegionInteract_DeltaPhiCuts_dist->Fill(bcal_show_E);
				// if(605.4<locPrimaryZ&&locPrimaryZ<624.7) h_foundE_TOFRegionInteract_DeltaPhiCuts_dist->Fill(bcal_show_E);
				// if(624.7<locPrimaryZ&&locPrimaryZ<690.) h_foundE_FCALRegionInteract_DeltaPhiCuts_dist->Fill(bcal_show_E);
			if(locPrimaryR<64.) h_foundE_innerRegionInteract_dist_DeltaPhiCuts_dist->Fill(bcal_show_E);
			if(locPrimaryR>64. && passes_shower_req) h_foundE_BCALRegionInteract_dist_DeltaPhiCuts_dist->Fill(bcal_show_E);
			}
			
		DVector3 h_thrown_atCentroid = PropogateThrownNeutralToBCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),shower_radius);
		h_thrownZ_AtCentroid_ALL->Fill(h_thrown_atCentroid.Z());
		h_reconZ_AtCentroid_ALL->Fill(BCAL_show_position.Z());
		
			
		japp->RootUnLock(); //RELEASE ROOT LOCK!!
	}
	
	//Loop a second time for best/not best showers
	for(int loc_i=0; loc_i<DBCALShowers.size(); ++loc_i) {
		DVector3 BCAL_show_position = DVector3(DBCALShowers[loc_i]->x,DBCALShowers[loc_i]->y,DBCALShowers[loc_i]->z);
		double angle = BCAL_show_position.Angle(ThrownPhotonAtBCAL);
		//WRONG!! // double shower_theta = (BCAL_show_position-locMCThrowns[0]->position()).Theta()*180/3.14159;
		double shower_phi   = BCAL_show_position.Phi()*180/3.14159;
		double shower_radius   = sqrt(DBCALShowers[loc_i]->x*DBCALShowers[loc_i]->x+DBCALShowers[loc_i]->y*DBCALShowers[loc_i]->y);
		double DeltaZ_recon = (BCAL_show_position.Z()-locMCThrowns[0]->position().Z());
		double shower_theta = TMath::ATan(shower_radius/DeltaZ_recon)*180/3.14159;
		if(shower_theta<0) shower_theta+= 180.;
		
		// cout << "BCAL shower R: " << sqrt(BCAL_show_position.X()*BCAL_show_position.X()+BCAL_show_position.Y()*BCAL_show_position.Y()) << endl;
		// cout << "BCAL inner radius R: " << bcalInnerRadiusR << endl;
		
		japp->RootWriteLock(); //RELEASE ROOT LOCK!!
		if(fabs(min_angle-angle) < 0.0001) h2_DeltaThetaDeltaPhi_best->Fill( shower_theta-locThrownPhotonP4.Theta()*180/3.1415, shower_phi-ThrownPhotonAtBCAL.Phi()*180/3.1415) ;
		else h2_DeltaThetaDeltaPhi_notbest->Fill( shower_theta-locThrownPhotonP4.Theta()*180/3.1415, shower_phi-ThrownPhotonAtBCAL.Phi()*180/3.1415) ;
		DVector3 h_thrown_atCentroid = PropogateThrownNeutralToBCAL(locMCThrowns[0]->position(),locMCThrowns[0]->momentum(),shower_radius);
		h_thrownZ_AtCentroid_best->Fill(h_thrown_atCentroid.Z());
		h_reconZ_AtCentroid_best->Fill(BCAL_show_position.Z());
		
		h_thrownR_AtCentroid->Fill(h_thrown_atCentroid.Perp());
		if(fabs(min_angle-angle) < 0.0001) h_reconR_AtCentroid_best->Fill(BCAL_show_position.Perp());
		if(fabs(min_angle-angle) > 0.0001) h_reconR_AtCentroid_notbest->Fill(BCAL_show_position.Perp());
		h_reconR_AtCentroid_all->Fill(BCAL_show_position.Perp());
		
		
		
		japp->RootUnLock(); //RELEASE ROOT LOCK!!

		
	}
	
	
	
	
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	h_NBCALShowers->Fill(DBCALShowers.size());
	japp->RootUnLock(); //RELEASE ROOT LOCK!!
	

 
	
 
	return NOERROR;
}

int DEventProcessor_BCAL_photon_gun_hists::Get_FileNumber(JEventLoop* locEventLoop) const
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
jerror_t DEventProcessor_BCAL_photon_gun_hists::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_BCAL_photon_gun_hists::fini(void)
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
	
	// delete dTreeInterface; //saves trees to file, closes file
	
	return NOERROR;
}

//Convert thrown photon coordinates at primary vertex in target to coordinates at FCAL face 
DVector3 DEventProcessor_BCAL_photon_gun_hists::PropogateThrownNeutralToBCAL(const DVector3 x4_init, const DVector3 p4_init, const double RadiusToEvaluateAt) {
	
	//tan(theta1) = delta X / delta Z = Px / Pz
	//tan(theta1) = delta X / delta Z = Py / Pz
	
	double radius = (RadiusToEvaluateAt==0) ? bcalInnerRadiusR : RadiusToEvaluateAt;
	
	double tan_theta = TMath::Tan(p4_init.Theta());
	double DeltaZ = radius/tan_theta;
	
	double x_pos_AtBCAL = x4_init.X() + DeltaZ*(p4_init.Px()/p4_init.Pz());
	double y_pos_AtBCAL = x4_init.Y() + DeltaZ*(p4_init.Py()/p4_init.Pz());
	
	return DVector3(x_pos_AtBCAL,y_pos_AtBCAL,DeltaZ+x4_init.Z());
}

