// $Id$
//
//    File: DEventProcessor_photoneff_hists.cc
// Created: Thu Feb 12 09:43:13 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#include <iostream>
#include <cmath>
using namespace std;

#include <TThread.h>

#include <TROOT.h>

#include "DEventProcessor_photoneff_hists.h"

#include <JANA/JApplication.h>
#include <JANA/JEventLoop.h>

#include <DANA/DApplication.h>
#include <DVector2.h>
#include <particleType.h>
#include <FCAL/DFCALHit.h>
#include <BCAL/DBCALMCResponse.h>
//#include <FCAL/DFCALMCResponse.h>

// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new DEventProcessor_photoneff_hists());
}
} // "C"


//------------------
// DEventProcessor_photoneff_hists
//------------------
DEventProcessor_photoneff_hists::DEventProcessor_photoneff_hists()
{
	phtn_ptr = &phtn;


	pthread_mutex_init(&mutex, NULL);
	
}

//------------------
// ~DEventProcessor_photoneff_hists
//------------------
DEventProcessor_photoneff_hists::~DEventProcessor_photoneff_hists()
{

}

//------------------
// init
//------------------
jerror_t DEventProcessor_photoneff_hists::init(void)
{
	// Create TRACKING directory
	TDirectory *dir = (TDirectory*)gROOT->FindObject("CALORIMETRY");
	if(!dir)dir = new TDirectoryFile("CALORIMETRY","CALORIMETRY");
	dir->cd();

	// Create Trees
	phtneff = new TTree("phtneff","Photon Reconstruction Efficiency");
	phtneff->Branch("E","photon",&phtn_ptr);

	dir->cd("../");
	
        // Create ROOT file
        RootFile = new TFile("photoneff.root","RECREATE");
 
	JParameterManager *parms = app->GetJParameterManager();

	DEBUG = 1;
	
	parms->SetDefaultParameter("TRKEFF:DEBUG", DEBUG);

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_photoneff_hists::brun(JEventLoop *loop, int32_t runnumber)
{

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_photoneff_hists::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_photoneff_hists::fini(void)
{
        return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_photoneff_hists::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	vector<const DFCALHit*> fcalhits;
	vector<const DBCALMCResponse*> bcalhits;
	vector<const DPhoton*> photons;
	vector<const DMCThrown*> mcthrowns;
	vector<const DMCTrajectoryPoint*> mctrajpoints;
	
	loop->Get(fcalhits);
	loop->Get(bcalhits);
	loop->Get(photons);
	loop->Get(mcthrowns);
	loop->Get(mctrajpoints);

	// Get hit list for all throwns
	for(unsigned int i=0; i<mcthrowns.size(); i++){
		const DMCThrown *mcthrown = mcthrowns[i];
		
		// if this is a charged track, then skip it
		if(fabs(mcthrowns[i]->charge())!=0.0)continue;

		// Momentum of thrown particle
		DVector3 pthrown = mcthrown->momentum();

		bool locIsRecon = isReconstructable(mcthrown, loop);

		// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
		japp->RootWriteLock(); //ACQUIRE ROOT LOCK
	
		phtn.pthrown = pthrown;
		// Initialize with the "not found" values
		phtn.pfit.SetXYZ(0,0,0);
		phtn.chisq=1.0E20;
		phtn.Ndof=-1;
		phtn.delta_E_over_E=1.0E20;
		phtn.delta_theta=1.0E20;
		phtn.delta_phi=1.0E20;
		phtn.isreconstructable = locIsRecon;
		phtn.Nbcal = 0;
		phtn.Nfcal = 0;
		phtn.event = eventnumber;
		
		double fom_best = 1.0E8;

		// Loop over found/fit photons
		for(unsigned int j=0; j<photons.size(); j++){
			const DPhoton *photon = photons[j];
			
			// Copy momentum vector to convenient local variables
			DVector3 pfit  = photon->momentum();
			
			// Calculate residuals from momentum parameters from DParticle
			double delta_E_over_E = (pfit.Mag()-pthrown.Mag())/pthrown.Mag();
			double delta_theta = (pfit.Theta() - pthrown.Theta())*1000.0;
			double delta_phi = (pfit.Phi() - pthrown.Phi())*1000.0;

			// Formulate a figure of merit to decide if this fit photon is closer to
			// the thrown photon than the best one we found so far.
			double fom = fabs(delta_E_over_E);
			if(fom<fom_best){
				fom = fom_best;
				
				phtn.pfit = pfit;
				//phtn.chisq = photon->chisq;
				//phtn.Ndof = photon->Ndof;
				phtn.delta_E_over_E = delta_E_over_E;
				phtn.delta_theta = delta_theta;
				phtn.delta_phi = delta_phi;
				phtn.Nbcal = bcalhits.size(); // Need hits actually used in DPhoton!
				phtn.Nfcal = fcalhits.size(); // Need hits actually used in DPhoton!

			}
		}		
		
		phtneff->Fill();

		japp->RootUnLock(); //RELEASE ROOT LOCK
	}
	
	return NOERROR;
}

//------------------
// isReconstructable
//------------------
bool DEventProcessor_photoneff_hists::isReconstructable(const DMCThrown *mcthrown, JEventLoop *loop)
{
	/// For photon reconstruction, we just check if the total energy
	/// in ether the BCAL or FCAL is greater than 50% of the thrown
	/// value to determine if it is reconstructible.
	///
	/// This is not entirely accurate since a photon could convert
	/// upstream and spray that much energy into a calorimeter
	/// and the photon would then not be reconstructible.
	///
	/// On the other hand, if you are really looking for detection
	/// efficiency, then you'll want that to be included in the
	/// calculation so maybe this is a good thing ....?
	
	vector<const DBCALMCResponse*> bcalhits;
	vector<const DFCALHit*> fcalhits;
	
	loop->Get(bcalhits);
	loop->Get(fcalhits);
	
	double Ebcal = 0.0;
	for(unsigned int i=0; i<bcalhits.size(); i++)Ebcal += (bcalhits[i])->E;
	if(Ebcal>0.5*mcthrown->energy())return true;

	double Efcal = 0.0;
	for(unsigned int i=0; i<fcalhits.size(); i++)Efcal += (fcalhits[i])->E;
	if(Efcal>0.5*mcthrown->energy())return true;
	
	return false;
}

