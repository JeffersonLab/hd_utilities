// $Id$
//
//    File: JEventProcessor_fcal_photon_covariance.cc
// Created: Mon Feb  8 15:12:19 EST 2016
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

//#include "JEventProcessor_fcal_photon_covariance.h"
//using namespace jana;


#include <stdint.h>
#include <vector>

#include "JEventProcessor_fcal_photon_covariance.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include "TRACKING/DMCThrown.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"
//#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALShower.h"

#include <TDirectory.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TProfile2D.h>
#include <TStyle.h>
#include <TPrincipal.h>
#include <TTree.h>
#include <TFile.h>

// root hist pointers

const int nump = 5;
static TPrincipal *principal;
//static TH1I *cluster_distributions[nump];
//static TH2I *cluster_correlations[nump][nump];
static TH1I *shower_distributions[nump];
static TH2I *shower_correlations[nump][nump];
int cutshowers_phi=0;
int cutshowers_t=0;
int cutshowers_r=0;
int cutshowers_E=0;
int cutshowers_num=0;
int keepshowers=0;
int numevents;

//static TTree *covariance_tree;
//static float covariance_matrix[nump][nump];

const float myPI=3.14159265358979;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
	void InitPlugin(JApplication *app){
		InitJANAPlugin(app);
		app->AddProcessor(new JEventProcessor_fcal_photon_covariance());
	}
} // "C"


//------------------
// JEventProcessor_fcal_photon_covariance (Constructor)
//------------------
JEventProcessor_fcal_photon_covariance::JEventProcessor_fcal_photon_covariance()
{
	VERBOSE = 0; // >0 once off info ; >2 event by event ; >3 everything
	ENERGY = 0;
	THETA = 0;

	if (gPARMS){
		gPARMS->SetDefaultParameter("fcal_photon_covariance:VERBOSE", VERBOSE, "fcal_photon_covariance verbosity level");
		gPARMS->SetDefaultParameter("fcal_photon_covariance:ENERGY", ENERGY, "fcal_photon_covariance thrown energy");
		gPARMS->SetDefaultParameter("fcal_photon_covariance:THETA", THETA, "fcal_photon_covariance thrown angle");
	}
}

//------------------
// ~JEventProcessor_fcal_photon_covariance (Destructor)
//------------------
JEventProcessor_fcal_photon_covariance::~JEventProcessor_fcal_photon_covariance()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_fcal_photon_covariance::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();
	//
	// First thread to get here makes all histograms. If one pointer is
	// already not NULL, assume all histograms are defined and return now


	// lock all root operations
	japp->RootWriteLock();
	
	// create root folder for fcal and cd to it, store main dir
	TDirectory *main = gDirectory;
	gDirectory->mkdir("singlephoton")->cd();
	gStyle->SetOptStat(1110);

	// set style
	gStyle->SetTitleOffset(1, "Y");
  	gStyle->SetTitleSize(0.05,"xyz");
	gStyle->SetTitleSize(0.08,"h");
	gStyle->SetLabelSize(0.05,"xyz");
	gStyle->SetTitleX(0);
	gStyle->SetTitleAlign(13);
	gStyle->SetNdivisions(505,"xy");

	// ***** create histograms for neutral particles

	char name[255], title[255];

	principal = new TPrincipal(nump, "ND");

	//covariance_tree = new TTree("covariance_tree","tree storing covariance matrix");



	// char cluster_titles[nump][20] = {"energy","rho","theta","phi","time"};
	// //char cluster_titles[nump][20] = {"energy","rho","theta","phi","time","thrown energy"};
	// for (int i=0; i<nump; i++) {
	// 	sprintf(name,"cluster_distribution_%i",i);
	// 	sprintf(title,"%s distribution;%s",cluster_titles[i],cluster_titles[i]);
	// 	cluster_distributions[i] = new TH1I(name,title,100,0,0);
	// }
	// for (int i=0; i<nump; i++) {
	// 	for (int j=0; j<nump; j++) {
	// 		if (j>i) 
	// 			{
	// 				sprintf(name,"cluster_correlation_%i_%i",i,j);
	// 				sprintf(title,"%s %s correlation;%s;%s",cluster_titles[i],cluster_titles[j],cluster_titles[i],cluster_titles[j]);
	// 				cluster_correlations[i][j] = new TH2I(name,title,100,0,0,100,0,0);
	// 			}
	// 	}
	// }

	char shower_titles[nump][20] = {"energy","r","phihat","z","time"};
	char shower_units[nump][20] = {"GeV","cm","cm","cm","ns"};
	for (int i=0; i<nump; i++) {
		sprintf(name,"shower_distribution_%i",i);
		sprintf(title,"%s distribution (%.1f GeV, %.0f deg);%s  (%s)",shower_titles[i],ENERGY,THETA,shower_titles[i],shower_units[i]);
		shower_distributions[i] = new TH1I(name,title,100,0,0);
	}
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			if (j>i) 
				{
					sprintf(name,"shower_correlation_%i_%i",i,j);
					sprintf(title,"%s-%s correlation (%.1f GeV, %.0f deg);%s  (%s);%s  (%s)",
							shower_titles[i],shower_titles[j],ENERGY,THETA,
							shower_titles[i],shower_units[i],shower_titles[j],shower_units[j]);
					shower_correlations[i][j] = new TH2I(name,title,100,0,0,100,0,0);
				}
		}
	}


	cutshowers_phi=0;
	cutshowers_t=0;
	cutshowers_r=0;
	cutshowers_E=0;
	cutshowers_num=0;
	keepshowers=0;
	numevents=0;

	// back to main dir
	main->cd();
	
	// unlock
	japp->RootUnLock();
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_fcal_photon_covariance::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_fcal_photon_covariance::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	vector<const DMCThrown*> mythrown;
	//vector<const DFCALCluster*> dfcalclusters;
	vector<const DFCALShower*> dfcalshowers;

	loop->Get(mythrown);
	//loop->Get(dfcalclusters);
	loop->Get(dfcalshowers);
	
	japp->RootWriteLock();

	Double_t sh[nump];
	numevents++;

	if (VERBOSE>2) printf("event %5lu,  %lu thrown, %lu fcal showers\n",eventnumber,mythrown.size(),dfcalshowers.size());
	if ( mythrown.size() == 1 ) {
		float thrown_px = mythrown[0]->px();
		float thrown_py = mythrown[0]->py();
		float thrown_pz = mythrown[0]->pz();
		float thrown_E = mythrown[0]->energy();
		
		// SHOWERS
		for(unsigned int i=0; i<dfcalshowers.size(); i++) {
			const DFCALShower *shower = dfcalshowers[i];
			float shower_E = shower->getEnergy();
			float shower_x = shower->getPosition().X();
			float shower_y = shower->getPosition().Y();
			float shower_z = shower->getPosition().Z();
			float shower_t = shower->getTime();
			float shower_r = sqrt(shower_x*shower_x+shower_y*shower_y);
			//float shower_rho = sqrt(shower_r*shower_r+shower_z*shower_z);
			//shower_t -= shower_rho/29.9792; // what is this for?  Can't remember

			float thrown_phi = atan2(thrown_py, thrown_px);
			float thrown_r = sqrt(thrown_px*thrown_px + thrown_py*thrown_py)/thrown_pz*shower_z;
			float shower_phi = atan2(shower_y,shower_x)-thrown_phi;
			while (abs(shower_phi)>myPI/4) {
				if (shower_phi>myPI/4) shower_phi-=myPI/2;
				if (shower_phi<-myPI/4) shower_phi+=myPI/2;
			}
			float shower_phihat = shower_phi*shower_r;
			
			float E_sigma = 0.04 + thrown_E * 0.025;
			if (shower_E > thrown_E - 3.5*E_sigma) {
				if (shower_r > 0.75*thrown_r) {
					if (shower_t < 21) {
						if (abs(shower_phihat) < 2.5) { 
							keepshowers++;
							if (VERBOSE>2) printf("keep shower: ");
							sh[0] = shower_E;
							sh[1] = shower_r;
							sh[2] = shower_phihat;
							sh[3] = shower_z;
							sh[4] = shower_t;
							//sh[3] = shower_t;
							principal->AddRow( sh );
							for (int i=0; i<nump; i++) {
								shower_distributions[i]->Fill(sh[i]);
								for (int j=0; j<nump; j++) {
									if (j>i) 
										{
											shower_correlations[i][j]->Fill(sh[i],sh[j]);
										}
								}
							}
						} else {
							cutshowers_phi++;
							if (VERBOSE>2) printf("%5lu       phi cut: ",eventnumber);
						}
					} else {
						cutshowers_t++;
						if (VERBOSE>2) printf("%5lu         t cut: ",eventnumber);
					}
				} else {
					cutshowers_r++;
					if (VERBOSE>2) printf("%5lu         r cut: ",eventnumber);
				}
			} else {
				cutshowers_E++;
				if (VERBOSE>2) printf("%5lu    energy cut: ",eventnumber);
			}
			if (VERBOSE>2) printf("E=%8.4f, (x,y,z)=(%8.4f,%8.4f,%8.4f) r=%8.4f  phi=%8.4f  t=%8.4f\n",
				   shower_E,shower_x,shower_y,shower_z,shower_r,shower_phi,shower_t);
		}
	} else {
		cutshowers_num++;
		if (VERBOSE>2) printf("%5lu num thrown cut: ",eventnumber);
	}

	japp->RootUnLock();

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_fcal_photon_covariance::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_fcal_photon_covariance::fini(void)
{
	// Called before program exit after event processing is finished.
	printf("%i events\n",numevents);	
	printf("%i showers cut for num thrown\n",cutshowers_num);
	printf("%i showers cut for E\n",cutshowers_E);
	printf("%i showers cut for r\n",cutshowers_r);
	printf("%i showers cut for time\n",cutshowers_t);
	printf("%i showers cut for phi\n",cutshowers_phi);
	printf("%i showers kept\n",keepshowers);

	//creat and initialise variables
	double rms[nump];
	double mean[nump];
	double correlation[nump][nump];
	double covariance[nump][nump];

	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			covariance[i][j] = 0;
			correlation[i][j] = 0;
		}
	}

	// fit histograms
	//	TF1 *f_gaus = new TF1("f_gaus","gaus");
	for (int i=0; i<nump; i++) {
		shower_distributions[i]->Fit("gaus");
		rms[i] = shower_distributions[i]->GetRMS();
		mean[i] = shower_distributions[i]->GetMean();
		//covariance[i][i] = rms[i]*rms[i];
	}
	//TF1 *f_pol1 = new TF1("f_pol1","pol1");
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			if (j>i) 
				{
					printf("i=%i, j=%i\n",i,j);
					shower_correlations[i][j]->Fit("pol1");
					//correlation[i][j] = f_pol1->GetParameter(1);
					//covariance[i][j] = rms[i]*rms[j]*correlation[i][j];
				}
		}
	}

	// print results
	printf("fcal_photon_covariance:\n mean ");
	for (int i=0; i<nump; i++) {				  
		printf("%12.6f ",mean[i]); 
	}
	printf("\n  rms ");
	for (int i=0; i<nump; i++) {				  
		printf("%12.6f ",rms[i]); 
	}
	printf("\n\n");


	// create file and tree to store variance and covariance
	char name[255], title[255];
	sprintf(name,"fcal_covariance_%.2f_%.1f.root",ENERGY,THETA);
	printf("Writing to %s\n",name);
	TFile covfile(name,"RECREATE");
	TTree *covariance_tree = new TTree("covariance_tree","tree storing covariance matrix");
	covariance_tree->Branch("energy",&ENERGY);
	covariance_tree->Branch("angle",&THETA);
	for (int i=0; i<nump; i++) {
		sprintf(name,"rms%i",i);
		sprintf(title,"rms%i/D",i);
		covariance_tree->Branch(name,&rms[i],title);	
	}
	for (int i=0; i<nump; i++) {
		sprintf(name,"mean%i",i);
		sprintf(title,"mean%i/D",i);
		covariance_tree->Branch(name,&mean[i],title);	
	}
	// put correlations in tree
	for (int i=0; i<nump; i++) {
		for (int j=0; j<=i; j++) {
			sprintf(name,"cor%i%i",i,j);
			sprintf(title,"cor%i%i/D",i,j);
			if (i!=j) covariance_tree->Branch(name,&correlation[i][j],title);
		}
	}

	// get covraince matrix from built in methods
	const TMatrixD* pmat = principal->GetCovarianceMatrix();
	pmat->Print();
	TMatrixD mat = *pmat; // nonpointer allows access to elements

	// write matrix to tree
	for (int i=0; i<nump; i++) {
		for (int j=0; j<=i; j++) {
			sprintf(name,"cov%i%i",i,j);
			sprintf(title,"cov%i%i/D",i,j);
			covariance_tree->Branch(name,&covariance[i][j],title);
			covariance[i][j] = mat(i,j);
			correlation[i][j] = mat(i,j)/sqrt(mat(i,i))/sqrt(mat(j,j));
		}
	}
	// fill the tree
	covariance_tree->Fill();
	covariance_tree->Write();
	covfile.Close();

	// Check that the values were faithfully reproduced
	printf("covariance\n");
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {				  
			printf("%12.6f ",covariance[i][j]); 
		}
		printf("\n");
	}
	printf("\n");


	printf("correlation\n");
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {				  
			printf("%12.6f ",correlation[i][j]); 
		}
		printf("\n");
	}
	printf("\n");

	return NOERROR;
}

