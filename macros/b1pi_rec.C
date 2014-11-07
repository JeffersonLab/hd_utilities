#define b1pi_rec_cxx
// The class definition in b1pi_rec.h has been generated automatically
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
// Root > T->Process("b1pi_rec.C")
// Root > T->Process("b1pi_rec.C","some options")
// Root > T->Process("b1pi_rec.C+")
//

#include "b1pi_rec.h"
#include <TH2.h>
#include <TStyle.h>


void b1pi_rec::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
   //-------------------------------------
   // Instantiate my histograms
   mass_X     = new TH1D("mass_X", "Mass of Exotic Hybrid", 500, 0.0, 4.0);
   mass_b1    = new TH1D("mass_b1", "Mass of b1 meson", 200, 0.0, 2.0);
   mass_omega = new TH1D("mass_omega", "Mass of #omega meson", 500, 0.0, 2.0);
   mass_rho   = new TH1D("mass_rho", "Mass of #rho meson", 200, 0.0, 2.0);
   mass_pi0   = new TH1D("mass_pi0", "Mass of #pi^{o} meson", 500, 0.0, 0.5);
   vertex_z    = new TH1D("vertex_z", "z-Vertex position", 160, 45.0, 85.);

   mass_X->GetXaxis()->SetTitle("Mass [GeV]");     
   mass_b1->GetXaxis()->SetTitle("Mass [GeV]");       
   mass_omega->GetXaxis()->SetTitle("Mass [GeV]");     
   mass_rho->GetXaxis()->SetTitle("Mass [GeV]");       
   mass_pi0->GetXaxis()->SetTitle("Mass [GeV]");       
   vertex_z->GetXaxis()->SetTitle("z-vertex position [cm]");       
    
   theta_gamma      = new TH1D("theta_gamma", "photon angles", 720, 0.0, 180.0);
   theta_pip        = new TH1D("theta_pip", "pi+ angles", 720, 0.0, 180.0);
   theta_pim        = new TH1D("theta_pim", "pi- angles", 720, 0.0, 180.0);
   theta_p          = new TH1D("theta_p", "proton angles", 720, 0.0, 180.0);  
 
   theta_gamma->GetXaxis()->SetTitle("Polar Angle [deg]");   
   theta_pip->GetXaxis()->SetTitle("Polar Angle [deg]");      
   theta_pim->GetXaxis()->SetTitle("Polar Angle [deg]");      
   theta_p->GetXaxis()->SetTitle("Polar Angle [deg]");        

   
   phi_gamma      = new TH1D("phi_gamma", "photon azimuthal angles", 360, -180.0, 180.0);
   phi_pip        = new TH1D("phi_pip", "pi+ azimuthal angles",360, -180.0, 180.0);
   phi_pim        = new TH1D("phi_pim", "pi- azimuthal angles", 360, -180.0, 180.0);
   phi_p          = new TH1D("phi_p", "proton azimuthal angles", 360, -180.0, 180.0);  
   
   phi_gamma->GetXaxis()->SetTitle("Azimuthal Angle [deg]");   
   phi_pip->GetXaxis()->SetTitle("Azimuthal Angle [deg]");      
   phi_pim->GetXaxis()->SetTitle("Azimuthal Angle [deg]");      
   phi_p->GetXaxis()->SetTitle("Azimuthal Angle [deg]");        

   
   Eg_vs_tg         = new TH2D("Eg_vs_tg","Photon Energy vs. theta", 360, 0.0, 90.0, 100, 0., 1.);
   Epip_vs_tpip     = new TH2D("Epip_vs_tpip","Pi+ Energy vs. theta", 360, 0.0, 90.0, 30, 0., 6.);
   Epim_vs_tpim     = new TH2D("Epim_vs_tpim","Pi- Energy vs. theta", 360, 0.0, 90.0, 30, 0., 6.);
   Ep_vs_tp         = new TH2D("Ep_vs_tp","Proton Energy vs. theta", 360, 0.0, 90.0, 30, 0., 3.);

   Eg_vs_tg->GetXaxis()->SetTitle("Polar Angle [deg]");     
   Epip_vs_tpip->GetXaxis()->SetTitle("Polar Angle [deg]"); 
   Epim_vs_tpim->GetXaxis()->SetTitle("Polar Angle [deg]"); 
   Ep_vs_tp->GetXaxis()->SetTitle("Polar Angle [deg]");     

   Eg_vs_tg->GetYaxis()->SetTitle("Momentum [GeV/c]");     
   Epip_vs_tpip->GetYaxis()->SetTitle("Momentum [GeV/c]"); 
   Epim_vs_tpim->GetYaxis()->SetTitle("Momentum [GeV/c]"); 
   Ep_vs_tp->GetYaxis()->SetTitle("Momentum [GeV/c]");     
   
   //-------------------------------------

}

void b1pi_rec::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

Bool_t b1pi_rec::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either b1pi_rec::GetEntry() or TBranch::GetEntry()
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
  
  // Fill histograms

  //cout<<"z-Vertex = "<<vertex.Z()<<endl;
  vertex_z->Fill(vertex.Z());
  
  // Note: Below we loop over all combinations but keep only the 
  // one closest the relevant meson mass.
  
  // Find the 2 gammas that best reconstruct a pi0
  TLorentzVector pi0(0,0,0,0);
  if(Nphoton>0){
    for(Int_t i=0; i<Nphoton-1; i++){
      for(Int_t j=i+1; j<Nphoton; j++){
        TLorentzVector my_pi0 = photon[i]+photon[j];
        if(fabs(my_pi0.M()-0.135) < fabs(pi0.M()-0.135))pi0 = my_pi0;
      }
    }
  }
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
    
    // rho
    //TLorentzVector rho = pip[index_rho_pip] + pim[index_rho_pim];
    TLorentzVector rho = omega - pi0;
    mass_rho->Fill(rho.M());
    
  } // if(X.P()!=0.0)
  
  // fill histos for individual particles:

  for (Int_t i=0;i<Nphoton;i++){
    Double_t l = photon[i].X()*photon[i].X()+photon[i].Y()*photon[i].Y()+photon[i].Z()*photon[i].Z();
    if (l>0){
      Double_t r = photon[i].Z()/sqrt(l);
      if ((r<-1.)||(r>1.)){
	r = r/fabs(r);
      }
      Double_t theta = acos(r)*180./3.1415926; 
      Double_t phi = (180./3.1415926)*atan2(photon[i].Y(),photon[i].X());

      phi_gamma->Fill(phi);
      theta_gamma->Fill(theta);
      Eg_vs_tg->Fill(theta,photon[i].P());
    }
  }
  for (Int_t i=0;i<Npip;i++){
    Double_t l = pip[i].X()*pip[i].X()+pip[i].Y()*pip[i].Y()+pip[i].Z()*pip[i].Z();
    if (l>0){
      Double_t r = pip[i].Z()/sqrt(l);
      if ((r<-1.)||(r>1.)){
	r = r/fabs(r);
      }
      Double_t theta = acos(r)*180./3.1415926;
      Double_t phi = (180./3.1415926)*atan2(pip[i].Y(),pip[i].X());

      phi_pip->Fill(phi);
      theta_pip->Fill(theta);
      Epip_vs_tpip->Fill(theta,pip[i].P());
    }
  }
  for (Int_t i=0;i<Npim;i++){
    Double_t l = pim[i].X()*pim[i].X()+pim[i].Y()*pim[i].Y()+pim[i].Z()*pim[i].Z();
    if (l>0){
      Double_t r = pim[i].Z()/sqrt(l);
      if ((r<-1.)||(r>1.)){
	r = r/fabs(r);
      }
      Double_t theta = acos(r)*180./3.1415926;
      Double_t phi = (180./3.1415926)*atan2(pim[i].Y(),pim[i].X());

      phi_pim->Fill(phi);
      theta_pim->Fill(theta); 
      Epim_vs_tpim->Fill(theta,pim[i].P());
      }
  }
  for (Int_t i=0;i<Nproton;i++){
    Double_t l = proton[i].X()*proton[i].X()+proton[i].Y()*proton[i].Y()+proton[i].Z()*proton[i].Z();
    if (l>0){
      Double_t r = proton[i].Z()/sqrt(l);
      if ((r<-1.)||(r>1.)){
	r = r/fabs(r);
      }
      Double_t theta = acos(r)*180./3.1415926;
      Double_t phi = (180./3.1415926)*atan2(proton[i].Y(),proton[i].X());

      phi_p->Fill(phi);
      theta_p->Fill(theta);
      Ep_vs_tp->Fill(theta,proton[i].P());
    }
  }
  

  //-------------------------------------
  // Delete our TLorentzVector objects
  if(Nphoton>0)delete[] photon;
  if(Npip>0)delete[] pip;
  if(Npim>0)delete[] pim;
  if(Nproton>0)delete[] proton;

   return kTRUE;
}

void b1pi_rec::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void b1pi_rec::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

  TFile *OF = new TFile("b1_pi_plots_rec.root","RECREATE");
  mass_pi0->Write();
  mass_rho->Write();
  mass_omega->Write();
  mass_b1->Write();
  mass_X->Write();
  vertex_z->Write();

  theta_gamma->Write();
  theta_pip->Write();
  theta_pim->Write();
  theta_p->Write();
  
  phi_gamma->Write();
  phi_pip->Write();
  phi_pim->Write();
  phi_p->Write();
  
  Eg_vs_tg->Write();
  Epip_vs_tpip->Write();
  Epim_vs_tpim->Write();
  Ep_vs_tp->Write();



  OF->Close();

}
