#define b1_pi_cxx
// The class definition in b1_pi.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("b1_pi.C")
// Root > T->Process("b1_pi.C","some options")
// Root > T->Process("b1_pi.C+")
//

#include "b1_pi.h"
#include <TH2.h>
#include <TStyle.h>


void b1_pi::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

	//-------------------------------------
	// Instantiate my histograms
	mass_X = new TH1D("mass_X", "Mass of Exotic Hybrid", 100, 0.0, 4.0);
	mass_b1 = new TH1D("mass_b1", "Mass of b1 meson", 200, 0.0, 2.0);
	mass_omega = new TH1D("mass_omega", "Mass of #omega meson", 100, 0.0, 2.0);
	mass_rho = new TH1D("mass_rho", "Mass of #rho meson", 200, 0.0, 2.0);
	mass_pi0 = new TH1D("mass_pi0", "Mass of #pi^{o} meson", 500, 0.0, 0.5);
	
	Ngood = 0;
	mass_X_good = (TH1D*)mass_X->Clone("mass_X_good");
	mass_b1_good = (TH1D*)mass_b1->Clone("mass_b1_good");
	mass_omega_good = (TH1D*)mass_omega->Clone("mass_omega_good");
	mass_rho_good = (TH1D*)mass_rho->Clone("mass_rho_good");
	mass_pi0_good = (TH1D*)mass_pi0->Clone("mass_pi0_good");

	stats = new TH1D("stats", "Misc. statistics", 10, 0.5, 10.5);
	//-------------------------------------

}

void b1_pi::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

Bool_t b1_pi::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either b1_pi::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

	//-------------------------------------
	// Read the event into memory
	GetEntry(entry);

	//-------------------------------------
	// For some reason, ROOT no longer makes the objects available as objects (???)
	// so we have to explicitly re-create out TLorentzVector objects


	// (Not sure why I'm forced to do this!!)
	if(Nphoton>kMaxphoton)Nphoton = kMaxphoton;
	if(Nproton>kMaxproton)Nproton = kMaxproton;
	if(Npip>kMaxpip)Npip = kMaxpip;
	if(Npim>kMaxpim)Npim = kMaxpim;
	
	TLorentzVector *photon = new TLorentzVector[Nphoton];
	TLorentzVector *pip = new TLorentzVector[Npip];
	TLorentzVector *pim = new TLorentzVector[Npim];
	TLorentzVector *proton = new TLorentzVector[Nproton];
	for(Int_t i=0; i<Nphoton; i++)photon[i].SetPxPyPzE(photon_p_fP_fX[i], photon_p_fP_fY[i], photon_p_fP_fZ[i], photon_p_fE[i]);
	for(Int_t i=0; i<Npip; i++)pip[i].SetPxPyPzE(pip_p_fP_fX[i], pip_p_fP_fY[i], pip_p_fP_fZ[i], pip_p_fE[i]);
	for(Int_t i=0; i<Npim; i++)pim[i].SetPxPyPzE(pim_p_fP_fX[i], pim_p_fP_fY[i], pim_p_fP_fZ[i], pim_p_fE[i]);
	for(Int_t i=0; i<Nproton; i++)proton[i].SetPxPyPzE(proton_p_fP_fX[i], proton_p_fP_fY[i], proton_p_fP_fZ[i], proton_p_fE[i]);

	TLorentzVector target(target_fP_fX, target_fP_fY, target_fP_fZ, target_fE);
	TLorentzVector beam(beam_fP_fX, beam_fP_fY, beam_fP_fZ, beam_fE);
	TVector3 vertex(vertex_fX, vertex_fY, vertex_fZ);
	TLorentzVector W(W_fP_fX, W_fP_fY, W_fP_fZ, W_fE);
		
	//-------------------------------------
	// Fill histograms
	
	// Note: Below we loop over all combinations but keep only the 
	// one closest the relevant meson mass.

	// Find the 2 gammas that best reconstruct a pi0
	TLorentzVector pi0(0,0,0,0);
	if(Nphoton>0){
		for(Int_t i=0; i<Nphoton-1; i++){
			if(photon[i].E()<0.05)continue;
			for(Int_t j=i+1; j<Nphoton; j++){
				if(photon[j].E()<0.05)continue;
				TLorentzVector my_pi0 = photon[i]+photon[j];
				if(fabs(my_pi0.M()-0.135) < fabs(pi0.M()-0.135))pi0 = my_pi0;
			}
		}
	}

	// Flag to indicate all of the thrown particles are in a fiducial
	// region of the detector and that the photons were properly reconstructed
	bool good = all_fiducial && Nphoton==2 && fabs(pi0.M()-0.134)<0.200;
	if(good)Ngood++;

	// Uncomment the following to force the pi0 to correct mass
	//pi0.SetE(sqrt(pow(pi0.P(),2) + pow(0.135,2)));

	// Find pi+pi+pi-pi-pi0 particles that give us the best X(2000) mass
	TLorentzVector X(0,0,0,0);
	Int_t ipip1=-1, ipip2=-1;
	Int_t ipim1=-1, ipim2=-1;
	if(pi0.P()!=0.0 && Npip>0 && Npim>0){
		for(Int_t k=0; k<Npip-1; k++){
			for(Int_t m=k+1; m<Npip; m++){
				for(Int_t n=0; n<Npim-1; n++){
					for(Int_t p=n+1; p<Npim; p++){
						TLorentzVector my_X = pi0+pip[k]+pip[m]+pim[n]+pim[p];
						if(fabs(my_X.M()-2.000) < fabs(X.M()-2.000)){
							X = my_X;
							ipip1 = k;
							ipip2 = m;
							ipim1 = n;
							ipim2 = p;
						}
					}
				}
			}
		}
	}
	
	// Fill all histograms if an "X" was found
	if(X.P()!=0.0){
	
		mass_pi0->Fill(pi0.M());
		mass_X->Fill(X.M());
		if(good){
			mass_pi0_good->Fill(pi0.M());
			mass_X_good->Fill(X.M());
		}
	
		// b1
		// (find the pi- that did NOT come from the X decay directly)
		TLorentzVector b1;
		TLorentzVector b1_1 = X - pim[ipim1];
		TLorentzVector b1_2 = X - pim[ipim2];
		Int_t index_rho_pim; // which ever is not from the X must be from the rho
		if(fabs(b1_1.M()-1.235) < fabs(b1_2.M()-1.235)){
			b1 = b1_1;
			index_rho_pim = ipim1;
		}else{
			b1 = b1_2;
			index_rho_pim = ipim2;
		}
		mass_b1->Fill(b1.M());
		if(good)mass_b1_good->Fill(b1.M());

		// omega
		// (find the pi+ that did NOT come from the b1 decay directly)
		TLorentzVector omega;
		TLorentzVector omega_1 = b1 - pip[ipip1];
		TLorentzVector omega_2 = b1 - pip[ipip2];
		Int_t index_rho_pip; // which ever is not from the b1 must be from the rho
		if(fabs(omega_1.M()-0.783) < fabs(omega_2.M()-0.783)){
			omega = omega_1;
			index_rho_pip = ipip1;
		}else{
			omega = omega_2;
			index_rho_pip = ipip2;
		}
		mass_omega->Fill(omega.M());
		if(good)mass_omega_good->Fill(omega.M());

		// rho
		//TLorentzVector rho = pip[index_rho_pip] + pim[index_rho_pim];
		TLorentzVector rho = omega - pi0;
		mass_rho->Fill(rho.M());
		if(good)mass_rho_good->Fill(rho.M());

	} // if(X.P()!=0.0)

	//-------------------------------------
	// Delete our TLorentzVector objects
	if(Nphoton>0)delete[] photon;
	if(Npip>0)delete[] pip;
	if(Npim>0)delete[] pim;
	if(Nproton>0)delete[] proton;

   return kTRUE;
}

void b1_pi::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void b1_pi::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
	
	int binlo = mass_X_good->FindBin(1.0);
	int binhi = mass_X_good->FindBin(3.0);
	double integral = mass_X_good->Integral(binlo, binhi);

	cout<<"  Number of \"good\" events: "<<Ngood<<endl;
	cout<<"       Integral of X mass: "<<integral<<endl;
	cout<<"               Efficiency: "<<integral/(double)Ngood<<endl;
	
	stats->SetBinContent(1, Ngood);
	stats->SetBinContent(2, integral);
	stats->SetBinContent(3, integral/(double)Ngood);
}
