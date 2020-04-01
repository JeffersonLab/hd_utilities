// $Id$
//
//    File: JEventProcessor_bcal_photon_covariance.cc
// Created: Mon Feb  8 15:12:19 EST 2016
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

//#include "JEventProcessor_bcal_photon_covariance.h"
//using namespace jana;


#include <stdint.h>
#include <vector>

#include "JEventProcessor_bcal_photon_covariance.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include "TRACKING/DMCThrown.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"
#include "BCAL/DBCALCluster.h"
#include "BCAL/DBCALShower.h"

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
static TH1I *cluster_distributions[nump];
static TH2I *cluster_correlations[nump][nump];
static TH1I *shower_distributions[nump];
static TH2I *shower_correlations[nump][nump];
static TH1I *shower_target_time;
int ecut, rcut, t0cut, phicut;

//static TTree *covariance_tree;
//static float covariance_matrix[nump][nump];

const float myPI=3.14159265358979;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
	void InitPlugin(JApplication *app){
		InitJANAPlugin(app);
		app->AddProcessor(new JEventProcessor_bcal_photon_covariance());
	}
} // "C"


//------------------
// JEventProcessor_bcal_photon_covariance (Constructor)
//------------------
JEventProcessor_bcal_photon_covariance::JEventProcessor_bcal_photon_covariance()
{
	VERBOSE = 0; // >0 once off info ; >2 event by event ; >3 everything
	ENERGY = 0;
	THETA = 0;

	if (gPARMS){
		gPARMS->SetDefaultParameter("bcal_photon_covariance:VERBOSE", VERBOSE, "bcal_photon_covariance verbosity level");
		gPARMS->SetDefaultParameter("plugin:ENERGY", ENERGY, "bcal_photon_covariance thrown energy");
		gPARMS->SetDefaultParameter("plugin:THETA", THETA, "bcal_photon_covariance thrown angle");
	}
}

//------------------
// ~JEventProcessor_bcal_photon_covariance (Destructor)
//------------------
JEventProcessor_bcal_photon_covariance::~JEventProcessor_bcal_photon_covariance()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_bcal_photon_covariance::init(void)
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
	
	// create root folder for bcal and cd to it, store main dir
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

	char cluster_titles[nump][20] = {"energy","rho","theta","phi","time"};
	for (int i=0; i<nump; i++) {
		sprintf(name,"cluster_distribution_%i",i);
		sprintf(title,"%s distribution (%.2f GeV, %.1f deg);%s",cluster_titles[i],ENERGY,THETA,cluster_titles[i]);
        if (VERBOSE>=2) printf("creating %s   %s\n",name,title);
		cluster_distributions[i] = new TH1I(name,title,100,0,0);
	}
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			if (j>i) 
				{
					sprintf(name,"cluster_correlation_%i_%i",i,j);
					sprintf(title,"%s %s correlation (%.2f GeV, %.1f deg);%s;%s",
							cluster_titles[i],cluster_titles[j],ENERGY,THETA,cluster_titles[i],cluster_titles[j]);
                    if (VERBOSE>=2) printf("creating %s   %s\n",name,title);
					cluster_correlations[i][j] = new TH2I(name,title,100,0,0,100,0,0);
				}
		}
	}

	char shower_titles[nump][20] = {"E","r","d#hat{#phi}","z","t"};
	char shower_units[nump][20] = {"GeV","cm","cm","cm","ns"};
	for (int i=0; i<nump; i++) {
		sprintf(name,"shower_distribution_%i",i);
		sprintf(title,"%s (%.2f GeV, %.1f deg);%s  (%s)",shower_titles[i],ENERGY,THETA,shower_titles[i],shower_units[i]);
        if (VERBOSE>=2) printf("creating %s   %s\n",name,title);
		shower_distributions[i] = new TH1I(name,title,100,0,0);
	}
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			if (j>i) 
				{
					sprintf(name,"shower_correlation_%i_%i",i,j);
					sprintf(title,"%s-%s (%.2f GeV, %.1f deg);%s  (%s);%s  (%s)",
							shower_titles[i],shower_titles[j],ENERGY,THETA,
							shower_titles[i],shower_units[i],shower_titles[j],shower_units[j]);
                    if (VERBOSE>=2) printf("creating %s   %s\n",name,title);
					shower_correlations[i][j] = new TH2I(name,title,100,0,0,100,0,0);
				}
		}
	}
	sprintf(name,"shower_distribution_%i",5);
	sprintf(title,"%s (%.2f GeV, %.1f deg);%s  (%s)","t0",ENERGY,THETA,"ns","ns");
	shower_target_time = new TH1I(name,title,100,0,0);
    if (VERBOSE>=2) printf("creating %s   %s\n",name,title);

    ecut=0; rcut=0; t0cut=0; phicut=0;

	// back to main dir
	main->cd();
	
	// unlock
	japp->RootUnLock();
	
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_bcal_photon_covariance::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes

	// load BCAL geometry
  	vector<const DBCALGeometry *> BCALGeomVec;
  	eventLoop->Get(BCALGeomVec);
  	if(BCALGeomVec.size() == 0)
		throw JException("Could not load DBCALGeometry object!");
	dBCALGeom = BCALGeomVec[0];
    //printf("%x\n",dBCALGeom);

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_bcal_photon_covariance::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	vector<const DMCThrown*> mythrown;
	vector<const DBCALCluster*> dbcalclusters;
	vector<const DBCALShower*> dbcalshowers;

	loop->Get(mythrown);
	loop->Get(dbcalclusters);
	loop->Get(dbcalshowers);
	
	japp->RootWriteLock();

	Double_t sh[nump];

    if (VERBOSE>=3) printf("event %5lu, thrown %lu, clusters %lu, showers %lu\n",
                           eventnumber,mythrown.size(),dbcalclusters.size(),dbcalshowers.size());

	if ( mythrown.size() == 1 ) {
		float thrown_px = mythrown[0]->px();
		float thrown_py = mythrown[0]->py();
		float thrown_E = mythrown[0]->energy();
		// CLUSTERS
		for(unsigned int i=0; i<dbcalclusters.size(); i++) {
			const DBCALCluster *cluster = dbcalclusters[i];
			// cuts to keep things reasonable
			//if (((thrown_E>=3.5 && cluster->E() > 0.7 * thrown_E) ||
			// (thrown_E<3.5 && thrown_E>=2.0 && cluster->E() > thrown_E-1) ||
			// (thrown_E<2.0 && cluster->E() > 1.25/2.0*(thrown_E-0.25))) &&
			//cluster->t() < 100 &&
			//abs(cluster->phi() - 3.1415) < 0.03)
				{
					float cl[nump];
					cl[0] = cluster->E();
					cl[1] = cluster->rho();
					cl[2] = cluster->theta();
					cl[3] = cluster->phi();
					cl[4] = cluster->t();
					//cl[5] = thrown_E;
					for (int i=0; i<nump; i++) {
						cluster_distributions[i]->Fill(cl[i]);
						for (int j=0; j<nump; j++) {
							if (j>i) 
								{
									cluster_correlations[i][j]->Fill(cl[i],cl[j]);
								}
						}
					}
				}
		}
		// SHOWERS
		for(unsigned int i=0; i<dbcalshowers.size(); i++) {
			const DBCALShower *shower = dbcalshowers[i];
			float shower_E = shower->E;
			float shower_x = shower->x;
			float shower_y = shower->y;
			float shower_z = shower->z;
			float shower_t = shower->t;
			float shower_r = sqrt(shower_x*shower_x+shower_y*shower_y);
			//float shower_rho = sqrt(shower_r*shower_r+shower_z*shower_z);
			float thrown_phi = atan2(thrown_py, thrown_px);
			float shower_phi = atan2(shower_y,shower_x)-thrown_phi;
			while (abs(shower_phi)>myPI/4) {
				if (shower_phi>myPI/4) shower_phi-=myPI/2;
				if (shower_phi<-myPI/4) shower_phi+=myPI/2;
			}
			float shower_phihat = shower_phi*shower_r;

            // // Original projection Cannot remember the logic here 
			// float showerRhoToBCAL = shower_rho*(dBCALGeom->GetBCAL_inner_rad()/shower_r);
			// //float showerRhoToBCAL = shower_rho*(65.0/shower_r);
			// float showerTimeToBCAL = showerRhoToBCAL* 0.0348177 + 0.3;
			// float shower_t0 = shower_t - showerTimeToBCAL;  // Target time: corrected for particle propagation

            // // Project to target time using MEASURED position
            // float shower_z_fromtarg = shower_z-65;
			// float shower_rho_fromtarg = sqrt(shower_r*shower_r+shower_z_fromtarg*shower_z_fromtarg);
            // float flight_time = shower_rho_fromtarg/30;
            // float shower_t0 = shower_t - flight_time;

            // Project to target time using EXPECTED position since showers can end up anywhere
            float shower_rho_EXPECTED = shower_r/sin(THETA/180*myPI);
            float flight_time = shower_rho_EXPECTED/30;
            float shower_t0 = shower_t - flight_time;

			// cuts to keep things reasonable
			//float E_sigma = 0.024 + thrown_E * 0.064; // at 90 degrees
			//float E_sigma = 0.021 + thrown_E * 0.049;
			if (shower_E < thrown_E/2) { // - 3*E_sigma) {
				ecut++;
				if (VERBOSE>=2) printf(" energy cut: ");
            } else {
				if (shower_r > 80) {
					rcut++;
					if (VERBOSE>=2) printf("      r cut: ");
                } else {
                    // huge time outliers can ruin everything, put a "loose" cut on shower_t 
                    // (loose  for some angles and tighter for others, since it is not constant.)
					if (shower_t>12 || shower_t<1.5 || shower_t0 > 0.8) {
						t0cut++;
						if (VERBOSE>=2) printf("      t cut: ");
                    } else {
						if (abs(shower_phi) > 0.025) { 
							phicut++;
							if (VERBOSE>=2) printf("    phi cut: ");
						} else {
							if (VERBOSE>=2) printf("keep shower: ");
							sh[0] = shower_E;
							sh[1] = shower_r;
							sh[2] = shower_phihat;
							sh[3] = shower_z;
							sh[4] = shower_t;
							principal->AddRow( sh );
							shower_target_time->Fill(shower_t0);
							for (int i=0; i<nump; i++) {
								shower_distributions[i]->Fill(sh[i]);
								for (int j=0; j<nump; j++) {
									if (j>i) 
										{
											shower_correlations[i][j]->Fill(sh[i],sh[j]);
										}
                                }
                            }
                        }
                    }
                }
            }
			if (VERBOSE>=2) printf("E=%8.4f, (x,y,z)=(%8.4f,%8.4f,%8.4f) r=%8.4f  phi=%8.4f  t=%8.4f\n",
				   shower_E,shower_x,shower_y,shower_z,shower_r,shower_phi,shower_t);
		}
	}
	japp->RootUnLock();

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_bcal_photon_covariance::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_bcal_photon_covariance::fini(void)
{
	// Called before program exit after event processing is finished.
	

	//creat and initialise variables
	double rms[nump];
	double mean[nump];
	double correlation[nump][nump];
	double covariance[nump][nump];
    double entries = shower_distributions[0]->GetEntries();
    double mean_t0=0, rms_t0=0;

	printf("%.0f showers kept\n",entries);
    printf("ecut=%i, rcut=%i, t0cut=%i, phicut=%i\n",ecut, rcut, t0cut, phicut);


	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			covariance[i][j] = 0;
			correlation[i][j] = 0;
		}
	}

	// fit histograms
	TF1 *f_gaus = new TF1("f_gaus","gaus");
	f_gaus->SetLineWidth(1);
	for (int i=0; i<nump; i++) {
		shower_distributions[i]->Fit("f_gaus","Q");
		rms[i] = shower_distributions[i]->GetRMS();
		mean[i] = shower_distributions[i]->GetMean();
		//covariance[i][i] = rms[i]*rms[i];
	}
    rms_t0 = shower_target_time->GetRMS();
    mean_t0 = shower_target_time->GetMean();

	TF1 *f_pol1 = new TF1("f_pol1","pol1");
	f_pol1->SetLineWidth(1);
	for (int i=0; i<nump; i++) {
		for (int j=0; j<nump; j++) {
			if (j>i) 
				{
					printf("i=%i, j=%i\n",i,j);
					shower_correlations[i][j]->Fit("f_pol1","Q");
					//correlation[i][j] = f_pol1->GetParameter(1);
					//covariance[i][j] = rms[i]*rms[j]*correlation[i][j];
				}
		}
	}

	// print results
	printf("bcal_photon_covariance:\n mean ");
	for (int i=0; i<nump; i++) {				  
		printf("%12.6f ",mean[i]); 
	}
	printf("\n  rms ");
	for (int i=0; i<nump; i++) {				  
		printf("%12.6f ",rms[i]); 
	}
	printf("\n\n");

	japp->RootWriteLock();

	// create file and tree to store variance and covariance
	char name[255], title[255];
	sprintf(name,"bcal_covariance_%.2f_%.1f.root",ENERGY,THETA);
	printf("Writing to %s\n",name);
	TFile covfile(name,"RECREATE");
	TTree *covariance_tree = new TTree("covariance_tree","tree storing covariance matrix");
	covariance_tree->Branch("energy",&ENERGY);
	covariance_tree->Branch("angle",&THETA);
	covariance_tree->Branch("entries",&entries);
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
    covariance_tree->Branch("rms_t0",&rms_t0,"rms_t0/D");
    covariance_tree->Branch("mean_t0",&mean_t0,"mean_t0/D");	

	// put correlations in tree
	for (int i=0; i<nump; i++) {
		for (int j=0; j<=i; j++) {
			sprintf(name,"cor%i%i",i,j);
			sprintf(title,"cor%i%i/D",i,j);
			if (i!=j) covariance_tree->Branch(name,&correlation[i][j],title);
		}
	}

	// get covariance matrix from built in methods
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

	japp->RootUnLock();

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

