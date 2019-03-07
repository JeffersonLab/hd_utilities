#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include "TLatex.h"
#include "TPaveStats.h"
#include "TGraphPainter.h"
#include "TString.h"
#include "TCollection.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2I.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMinuit.h"
#include "TKey.h"
#include "TCut.h"
#include "TDatime.h"
#include "TAxis.h"
#include "TLine.h"
#include "TTree.h"
#include "TBranch.h"
#include "TStyle.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TDirectory.h"
#include "TString.h"
#include "TLorentzVector.h"

Bool_t FCAL_STUDY=true; //true: look for one candidates in FCAL; false: look for one candidate in BCAL

Int_t NBeamBunchesOutoftime = 2;

// Double_t m1_pi0cut_lo = 0.08;
// Double_t m1_pi0cut_hi = 0.19;
Double_t m1_pi0cut_lo = 0.00;
Double_t m1_pi0cut_hi = 0.25;
Double_t m1_DeltaPhiCut = 180; // +/- this many degrees, how well candidate shower + spectator shower + pi+pi- are opposite the proton

Double_t pi0_MMcut_lo = 0.11;
Double_t pi0_MMcut_hi = 0.16;

Double_t MISS_THETA_LOCUT = 0; // FCAL
Double_t MISS_THETA_HICUT = 20.5; //FCAL
// Double_t MISS_THETA_LOCUT = 8; // BCAL
// Double_t MISS_THETA_HICUT = 38; //BCAL

// Double_t MISS_E_MIN = 0.6; // Good for FCAL
Double_t MISS_E_MIN = 0.0; // Try for FCAL

//Pick a number high enough to suppress rho -> pi+ pi- + fake photon or else missing spectrum (no second candidate) will get screwy!
Double_t FOUND_E_MIN = 0.5; // Other pi0 photon must have at least this much energy. 0.8 in FCAL would be nice, statistics willing.

Int_t N_E_BINS = 14; // FCAL
Int_t N_THETA_BINS = 40; //FCAL
// Int_t N_E_BINS = 20; // BCAL
// Int_t N_THETA_BINS = 11; //BCAL

Double_t gamma1_miss_lotheta = 3.5; //For projecting into E distribution, FCAL
Double_t gamma1_miss_hitheta = 9.5; //For projecting into E distribution, FCAL
Double_t gamma1_miss_Emin = 0.8; //For projecting into theta distribution, FCAL
// Double_t gamma1_miss_lotheta = 15; //For projecting into E distribution, BCAL
// Double_t gamma1_miss_hitheta = 100; //For projecting into E distribution, BCAL
// Double_t gamma1_miss_Emin = 0.0; //For projecting into theta distribution, BCAL

Bool_t USE_THROWN_4VEC=false; //true: replace all 4-vectors with thrown (but not other stuff like pi0 missing mass)
// Bool_t USE_THROWN_4VEC=true; //true: replace all 4-vectors with thrown (but not other stuff like pi0 missing mass)

Bool_t Redefine_FOUND_E_MIN_cut(Double_t new_val) {
	FOUND_E_MIN = new_val;
	return true;
}


Int_t GetEBin14_FCAL(Double_t my_E) {
	
	Int_t my_bin = -1;

	if(0.00<my_E&&my_E<0.33)  my_bin = 0;
	if(0.33<my_E&&my_E<0.67)  my_bin = 1;
	if(0.67<my_E&&my_E<1.00)  my_bin = 2;
	if(1.00<my_E&&my_E<1.33)  my_bin = 3;
	if(1.33<my_E&&my_E<1.67)  my_bin = 4;
	if(1.67<my_E&&my_E<2.00)  my_bin = 5;
	if(2.00<my_E&&my_E<2.33)  my_bin = 6;
	if(2.33<my_E&&my_E<2.67)  my_bin = 7;
	if(2.67<my_E&&my_E<3.00)  my_bin = 8;
	if(3.00<my_E&&my_E<3.33)  my_bin = 9;
	if(3.33<my_E&&my_E<3.67)  my_bin = 10;
	if(3.67<my_E&&my_E<4.00)  my_bin = 11;
	if(4.00<my_E&&my_E<4.33)  my_bin = 12;
	if(4.33<my_E&&my_E<4.67)  my_bin = 13;

	return my_bin;
	
}

Int_t GetEBin_BCAL(Double_t my_E) {
	
	Int_t my_bin = -1;

	if(0.0<my_E&&my_E<0.1)  my_bin = 0;
	if(0.1<my_E&&my_E<0.2)  my_bin = 1;
	if(0.2<my_E&&my_E<0.3)  my_bin = 2;
	if(0.3<my_E&&my_E<0.4)  my_bin = 3;
	if(0.4<my_E&&my_E<0.5)  my_bin = 4;
	if(0.5<my_E&&my_E<0.6)  my_bin = 5;
	if(0.6<my_E&&my_E<0.7)  my_bin = 6;
	if(0.7<my_E&&my_E<0.8)  my_bin = 7;
	if(0.8<my_E&&my_E<0.9)  my_bin = 8;
	if(0.9<my_E&&my_E<1.0)  my_bin = 9;
	if(1.0<my_E&&my_E<1.1)  my_bin = 10;
	if(1.1<my_E&&my_E<1.2)  my_bin = 11;
	if(1.2<my_E&&my_E<1.3)  my_bin = 12;
	if(1.3<my_E&&my_E<1.4)  my_bin = 13;
	if(1.4<my_E&&my_E<1.5)  my_bin = 14;
	if(1.5<my_E&&my_E<1.6)  my_bin = 15;
	if(1.6<my_E&&my_E<1.7)  my_bin = 16;
	if(1.7<my_E&&my_E<1.8)  my_bin = 17;
	if(1.8<my_E&&my_E<1.9)  my_bin = 18;
	if(1.9<my_E&&my_E<2.0)  my_bin = 19;


	return my_bin;
	
}


Int_t GetThetaBin25FCAL(Double_t my_theta) {
	
	Int_t my_bin = -1;

	if(0.0<my_theta&&my_theta<0.5)    my_bin = 0;
	if(0.5<my_theta&&my_theta<1.0)    my_bin = 1;
	if(1.0<my_theta&&my_theta<1.5)    my_bin = 2;
	if(1.5<my_theta&&my_theta<2.0)    my_bin = 3;
	if(2.0<my_theta&&my_theta<2.5)    my_bin = 4;
	if(2.5<my_theta&&my_theta<3.0)    my_bin = 5;
	if(3.0<my_theta&&my_theta<3.5)    my_bin = 6;
	if(3.5<my_theta&&my_theta<4.0)    my_bin = 7;
	if(4.0<my_theta&&my_theta<4.5)    my_bin = 8;
	if(4.5<my_theta&&my_theta<5.0)    my_bin = 9;
	if(5.0<my_theta&&my_theta<5.5)    my_bin = 10;
	if(5.5<my_theta&&my_theta<6.0)    my_bin = 11;
	if(6.0<my_theta&&my_theta<6.5)    my_bin = 12;
	if(6.5<my_theta&&my_theta<7.0)    my_bin = 13;
	if(7.0<my_theta&&my_theta<7.5)    my_bin = 14;
	if(7.5<my_theta&&my_theta<8.0)    my_bin = 15;
	if(8.0<my_theta&&my_theta<8.5)    my_bin = 16;
	if(8.5<my_theta&&my_theta<9.0)    my_bin = 17;
	if(9.0<my_theta&&my_theta<9.5)    my_bin = 18;
	if(9.5<my_theta&&my_theta<10.0)   my_bin = 19;
	if(10.0<my_theta&&my_theta<10.5)  my_bin = 20;
	if(10.5<my_theta&&my_theta<11.0)  my_bin = 21;
	if(11.0<my_theta&&my_theta<11.5)  my_bin = 22;
	if(11.5<my_theta&&my_theta<12.0)  my_bin = 23;
	if(12.0<my_theta&&my_theta<12.5)  my_bin = 24;
	if(12.5<my_theta&&my_theta<13.0)  my_bin = 25;
	if(13.0<my_theta&&my_theta<13.5)  my_bin = 26;
	if(13.5<my_theta&&my_theta<14.0)  my_bin = 27;
	if(14.0<my_theta&&my_theta<14.5)  my_bin = 28;
	if(14.5<my_theta&&my_theta<15.0)  my_bin = 29;
	if(15.0<my_theta&&my_theta<15.5)  my_bin = 30;
	if(15.5<my_theta&&my_theta<16.0)  my_bin = 31;
	if(16.0<my_theta&&my_theta<16.5)  my_bin = 32;
	if(16.5<my_theta&&my_theta<17.0)  my_bin = 33;
	if(17.0<my_theta&&my_theta<17.5)  my_bin = 34;
	if(17.5<my_theta&&my_theta<18.0)  my_bin = 35;
	if(18.0<my_theta&&my_theta<18.5)  my_bin = 36;
	if(18.5<my_theta&&my_theta<19.0)  my_bin = 37;
	if(19.0<my_theta&&my_theta<19.5)  my_bin = 38;
	if(19.5<my_theta&&my_theta<20.0)  my_bin = 39;

	return my_bin;
	
}

Int_t GetThetaBinBCAL(Double_t my_theta) {
	
	Int_t my_bin = -1;

	if(8.0< my_theta&&my_theta<11.)    my_bin = 0;
	if(11.0<my_theta&&my_theta<14.)    my_bin = 1;
	if(14.0<my_theta&&my_theta<17.)    my_bin = 2;
	if(17.0<my_theta&&my_theta<20.)    my_bin = 3;
	if(20.0<my_theta&&my_theta<23.)    my_bin = 4;
	if(23.0<my_theta&&my_theta<26.)    my_bin = 5;
	if(26.0<my_theta&&my_theta<29.)    my_bin = 6;
	if(29.0<my_theta&&my_theta<32.)    my_bin = 7;
	if(32.0<my_theta&&my_theta<35.)    my_bin = 8;
	if(35.0<my_theta&&my_theta<38.)    my_bin = 9;
	if(38.0<my_theta&&my_theta<41.)    my_bin = 10;

	
	return my_bin;
}


Int_t MakeOmegaHists(TString str_infile, TString str_outfile, Bool_t make_thrown_info_hists = false, Long64_t max_events = 1000000000000) {
	
	if(USE_THROWN_4VEC&&!make_thrown_info_hists) {
		cout << "Error! Turn make_thrown_info_hists option on (and verify you're using MC)" << endl;
		return 0;
	}
	
	Int_t nbins_combined_hists = 1000;
	
	TFile* f_in = new TFile(str_infile);
	TTree* t_in = (TTree*)f_in->FindObjectAny("p3pi_FCALStudy");
	

	Double_t RecM_Proton;    t_in->SetBranchAddress("RecM_Proton",&RecM_Proton);
	Double_t RecM_Proton_pkf;    t_in->SetBranchAddress("RecM_Proton_pkf",&RecM_Proton_pkf);
	Double_t threepi_mass;    t_in->SetBranchAddress("threepi_mass",&threepi_mass);
	Double_t threepi_mass_pkf;    t_in->SetBranchAddress("threepi_mass_pkf",&threepi_mass_pkf);
	Double_t twogamma_mass;    t_in->SetBranchAddress("twogamma_mass",&twogamma_mass);
	Double_t twogamma_mass_pkf;    t_in->SetBranchAddress("twogamma_mass_pkf",&twogamma_mass_pkf);
	
	Double_t PiPlus_px;       t_in->SetBranchAddress("PiPlus_px",&PiPlus_px);
	Double_t PiPlus_py;       t_in->SetBranchAddress("PiPlus_py",&PiPlus_py);
	Double_t PiPlus_pz;       t_in->SetBranchAddress("PiPlus_pz",&PiPlus_pz);
	Double_t PiPlus_E;        t_in->SetBranchAddress("PiPlus_E",&PiPlus_E);
	Double_t PiMinus_px;      t_in->SetBranchAddress("PiMinus_px",&PiMinus_px);
	Double_t PiMinus_py;      t_in->SetBranchAddress("PiMinus_py",&PiMinus_py);
	Double_t PiMinus_pz;      t_in->SetBranchAddress("PiMinus_pz",&PiMinus_pz);
	Double_t PiMinus_E;       t_in->SetBranchAddress("PiMinus_E",&PiMinus_E);
	Double_t Proton_px;       t_in->SetBranchAddress("Proton_px",&Proton_px);
	Double_t Proton_py;       t_in->SetBranchAddress("Proton_py",&Proton_py);
	Double_t Proton_pz;       t_in->SetBranchAddress("Proton_pz",&Proton_pz);
	Double_t Proton_E;        t_in->SetBranchAddress("Proton_E",&Proton_E);
	Double_t Gamma1_px;       t_in->SetBranchAddress("Gamma1_px",&Gamma1_px);
	Double_t Gamma1_py;       t_in->SetBranchAddress("Gamma1_py",&Gamma1_py);
	Double_t Gamma1_pz;       t_in->SetBranchAddress("Gamma1_pz",&Gamma1_pz);
	Double_t Gamma1_E;        t_in->SetBranchAddress("Gamma1_E",&Gamma1_E);
	Double_t Gamma2_px;       t_in->SetBranchAddress("Gamma2_px",&Gamma2_px);
	Double_t Gamma2_py;       t_in->SetBranchAddress("Gamma2_py",&Gamma2_py);
	Double_t Gamma2_pz;       t_in->SetBranchAddress("Gamma2_pz",&Gamma2_pz);
	Double_t Gamma2_E;        t_in->SetBranchAddress("Gamma2_E",&Gamma2_E);
	Int_t   PiPlus_PIDhit;   t_in->SetBranchAddress("PiPlus_PIDhit",&PiPlus_PIDhit);
	Double_t   PiPlus_TrackingChi2_ndf;   t_in->SetBranchAddress("PiPlus_TrackingChi2_ndf",&PiPlus_TrackingChi2_ndf);
	Double_t   PiPlus_TimingChi2_ndf;   t_in->SetBranchAddress("PiPlus_TimingChi2_ndf",&PiPlus_TimingChi2_ndf);
	Double_t   PiPlus_dEdxChi2_ndf;   t_in->SetBranchAddress("PiPlus_dEdxChi2_ndf",&PiPlus_dEdxChi2_ndf);
	Double_t   PiPlus_beta_meas;   t_in->SetBranchAddress("PiPlus_beta_meas",&PiPlus_beta_meas);
	Int_t   PiMinus_PIDhit;  t_in->SetBranchAddress("PiMinus_PIDhit",&PiMinus_PIDhit);
	Double_t   PiMinus_TrackingChi2_ndf;   t_in->SetBranchAddress("PiMinus_TrackingChi2_ndf",&PiMinus_TrackingChi2_ndf);
	Double_t   PiMinus_TimingChi2_ndf;   t_in->SetBranchAddress("PiMinus_TimingChi2_ndf",&PiMinus_TimingChi2_ndf);
	Double_t   PiMinus_dEdxChi2_ndf;   t_in->SetBranchAddress("PiMinus_dEdxChi2_ndf",&PiMinus_dEdxChi2_ndf);
	Double_t   PiMinus_beta_meas;   t_in->SetBranchAddress("PiMinus_beta_meas",&PiMinus_beta_meas);
	Int_t   Proton_PIDhit;   t_in->SetBranchAddress("Proton_PIDhit",&Proton_PIDhit);
	Double_t   Proton_TrackingChi2_ndf;   t_in->SetBranchAddress("Proton_TrackingChi2_ndf",&Proton_TrackingChi2_ndf);
	Double_t   Proton_TimingChi2_ndf;   t_in->SetBranchAddress("Proton_TimingChi2_ndf",&Proton_TimingChi2_ndf);
	Double_t   Proton_dEdxChi2_ndf;   t_in->SetBranchAddress("Proton_dEdxChi2_ndf",&Proton_dEdxChi2_ndf);
	Double_t   Proton_beta_meas;   t_in->SetBranchAddress("Proton_beta_meas",&Proton_beta_meas);

	Double_t MM2;    t_in->SetBranchAddress("MM2",&MM2);

	Double_t rf_deltaT;    t_in->SetBranchAddress("rf_deltaT",&rf_deltaT);
	Double_t beamE;    t_in->SetBranchAddress("beamE",&beamE);
	Double_t vertex_z;    t_in->SetBranchAddress("vertex_z",&vertex_z);
	Double_t vertex_r;    t_in->SetBranchAddress("vertex_r",&vertex_r);
	Int_t NExtraTracks;    t_in->SetBranchAddress("NExtraTracks",&NExtraTracks);
	Double_t ExtraShowerE;    t_in->SetBranchAddress("ExtraShowerE",&ExtraShowerE);
	
	Double_t kinfit_CL;    t_in->SetBranchAddress("kinfit_CL",&kinfit_CL);
	Double_t kinfit_chi2_ndf;    t_in->SetBranchAddress("kinfit_chi2_ndf",&kinfit_chi2_ndf);
	
	Int_t Run;    t_in->SetBranchAddress("Run",&Run);
	Int_t Event;    t_in->SetBranchAddress("Event",&Event);

	// Int_t NumNeutralCombosSaved = 15;
	Int_t NumNeutralCombosSaved = 1;
	
	Double_t MissingCan_Px[NumNeutralCombosSaved]; t_in->SetBranchAddress("MissingCan_Px",MissingCan_Px);
	Double_t MissingCan_Py[NumNeutralCombosSaved]; t_in->SetBranchAddress("MissingCan_Py",MissingCan_Py);
	Double_t MissingCan_Pz[NumNeutralCombosSaved]; t_in->SetBranchAddress("MissingCan_Pz",MissingCan_Pz);
	Double_t MissingCan_E[NumNeutralCombosSaved];  t_in->SetBranchAddress("MissingCan_E" ,MissingCan_E);
	
	Int_t MissingCan_ISFCAL[NumNeutralCombosSaved];  t_in->SetBranchAddress("MissingCan_ISFCAL" ,MissingCan_ISFCAL);
	Double_t threepi_can_mass[NumNeutralCombosSaved];  t_in->SetBranchAddress("threepi_can_mass" ,threepi_can_mass);
	Double_t twogamma_can_mass[NumNeutralCombosSaved];  t_in->SetBranchAddress("twogamma_can_mass" ,twogamma_can_mass);
	Int_t NMissingCandidates;  t_in->SetBranchAddress("NMissingCandidates" ,&NMissingCandidates);
	Int_t NFCALCandidates;     t_in->SetBranchAddress("NFCALCandidates" ,&NFCALCandidates);
	Int_t NBCALCandidates;     t_in->SetBranchAddress("NBCALCandidates" ,&NBCALCandidates);

	Double_t PiPlus_px_th; if(make_thrown_info_hists) t_in->SetBranchAddress("PiPlus_px_th",&PiPlus_px_th);
	Double_t PiPlus_py_th; if(make_thrown_info_hists) t_in->SetBranchAddress("PiPlus_py_th",&PiPlus_py_th);
	Double_t PiPlus_pz_th; if(make_thrown_info_hists) t_in->SetBranchAddress("PiPlus_pz_th",&PiPlus_pz_th);
	Double_t PiPlus_E_th;  if(make_thrown_info_hists) t_in->SetBranchAddress("PiPlus_E_th",&  PiPlus_E_th);
	Double_t PiMinus_px_th; if(make_thrown_info_hists) t_in->SetBranchAddress("PiMinus_px_th",&PiMinus_px_th);
	Double_t PiMinus_py_th; if(make_thrown_info_hists) t_in->SetBranchAddress("PiMinus_py_th",&PiMinus_py_th);
	Double_t PiMinus_pz_th; if(make_thrown_info_hists) t_in->SetBranchAddress("PiMinus_pz_th",&PiMinus_pz_th);
	Double_t PiMinus_E_th;  if(make_thrown_info_hists) t_in->SetBranchAddress("PiMinus_E_th",&  PiMinus_E_th);
	Double_t Proton_px_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Proton_px_th",&Proton_px_th);
	Double_t Proton_py_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Proton_py_th",&Proton_py_th);
	Double_t Proton_pz_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Proton_pz_th",&Proton_pz_th);
	Double_t Proton_E_th;  if(make_thrown_info_hists) t_in->SetBranchAddress("Proton_E_th",&  Proton_E_th);
	Double_t Gamma1_px_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma1_px_th",&Gamma1_px_th);
	Double_t Gamma1_py_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma1_py_th",&Gamma1_py_th);
	Double_t Gamma1_pz_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma1_pz_th",&Gamma1_pz_th);
	Double_t Gamma1_E_th;  if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma1_E_th",&  Gamma1_E_th);
	Double_t Gamma2_px_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma2_px_th",&Gamma2_px_th);
	Double_t Gamma2_py_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma2_py_th",&Gamma2_py_th);
	Double_t Gamma2_pz_th; if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma2_pz_th",&Gamma2_pz_th);
	Double_t Gamma2_E_th;  if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma2_E_th",&  Gamma2_E_th);
	
	Double_t Gamma1_FOM;  if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma1_FOM",  &Gamma1_FOM);
	Double_t Gamma2_FOM;  if(make_thrown_info_hists) t_in->SetBranchAddress("Gamma2_FOM",  &Gamma2_FOM);
	Double_t two_gamma_opangle_thr;  if(make_thrown_info_hists) t_in->SetBranchAddress("two_gamma_opangle_thr",  &two_gamma_opangle_thr);
	Double_t gam2_ThrRecon_opangle;  if(make_thrown_info_hists) t_in->SetBranchAddress("gam2_ThrRecon_opangle",  &gam2_ThrRecon_opangle);
	
	
	Long64_t nentries = t_in->GetEntries();
	cout << "There are " << nentries <<  " events to get through" << endl;
	
	//Define histograms here
	TH1F* h_all_missphoton_E_intime = new TH1F("h_all_missphoton_E_intime","Mass Distribution",nbins_combined_hists,0.0,10.);		
	TH1F* h_all_missphoton_E_outoftime = new TH1F("h_all_missphoton_E_outoftime","Mass Distribution",nbins_combined_hists,0.0,10.);		
	TH1F* h_all_missphoton_E_accidsub = new TH1F("h_all_missphoton_E_accidsub","Mass Distribution",nbins_combined_hists,0.0,10.);		
	TH1F* h_all_missphoton_theta_intime = new TH1F("h_all_missphoton_theta_intime","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_all_missphoton_theta_outoftime = new TH1F("h_all_missphoton_theta_outoftime","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_all_missphoton_theta_accidsub = new TH1F("h_all_missphoton_theta_accidsub","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_accepted_missphoton_E_intime = new TH1F("h_accepted_missphoton_E_intime","Mass Distribution",nbins_combined_hists,0.0,10.);		
	TH1F* h_accepted_missphoton_E_outoftime = new TH1F("h_accepted_missphoton_E_outoftime","Mass Distribution",nbins_combined_hists,0.0,10.);		
	TH1F* h_accepted_missphoton_E_accidsub = new TH1F("h_accepted_missphoton_E_accidsub","Mass Distribution",nbins_combined_hists,0.0,10.);		
	TH1F* h_accepted_missphoton_theta_intime = new TH1F("h_accepted_missphoton_theta_intime","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_accepted_missphoton_theta_outoftime = new TH1F("h_accepted_missphoton_theta_outoftime","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_accepted_missphoton_theta_accidsub = new TH1F("h_accepted_missphoton_theta_accidsub","Mass Distribution",nbins_combined_hists,0.0,45.);		
	
	
	TH1F* h_rf_deltaT_all = new TH1F("h_rf_deltaT_all","Mass Distribution",nbins_combined_hists,-20,20);		
	TH1F* h_rf_deltaT_outoftime = new TH1F("h_rf_deltaT_outoftime","Mass Distribution",nbins_combined_hists,-20,20);		
	TH1F* h_rf_deltaT_accidsub = new TH1F("h_rf_deltaT_accidsub","Mass Distribution",nbins_combined_hists,-20,20);		
	
	TH1F* h_deltaphi_all_intime = new TH1F("h_deltaphi_all_intime","Mass Distribution",nbins_combined_hists,0.,370);		
	TH1F* h_deltaphi_all_outoftime = new TH1F("h_deltaphi_all_outoftime","Mass Distribution",nbins_combined_hists,0.,370);		
	TH1F* h_deltaphi_all_accidsub = new TH1F("h_deltaphi_all_accidsub","Mass Distribution",nbins_combined_hists,0.,370);	
	
	TH1F* h_m1num_miss_omega_intime = new TH1F("h_m1num_miss_omega_intime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m1num_miss_omega_outoftime = new TH1F("h_m1num_miss_omega_outoftime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m1num_miss_omega_accidsub = new TH1F("h_m1num_miss_omega_accidsub","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m1den_miss_omega_intime = new TH1F("h_m1den_miss_omega_intime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m1den_miss_omega_outoftime = new TH1F("h_m1den_miss_omega_outoftime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m1den_miss_omega_accidsub = new TH1F("h_m1den_miss_omega_accidsub","Mass Distribution",nbins_combined_hists,0.0,2.);		
	
	TH1F* h_m2num_inv_omega_intime = new TH1F("h_m2num_inv_omega_intime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m2num_inv_omega_outoftime = new TH1F("h_m2num_inv_omega_outoftime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m2num_inv_omega_accidsub = new TH1F("h_m2num_inv_omega_accidsub","Mass Distribution",nbins_combined_hists,0.0,2.);		
	//"Inefficiency", or when there are 0 candidates reconstructed
	TH1F* h_m2denineff_miss_omega_intime = new TH1F("h_m2denineff_miss_omega_intime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m2denineff_miss_omega_outoftime = new TH1F("h_m2denineff_miss_omega_outoftime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_m2denineff_miss_omega_accidsub = new TH1F("h_m2denineff_miss_omega_accidsub","Mass Distribution",nbins_combined_hists,0.0,2.);		
	
	//Match cuts from h_m2num_inv_omega hists, used to determine scale factor
	TH1F* h_missing_omega_4scale_intime = new TH1F("h_missing_omega_4scale_intime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_missing_omega_4scale_outoftime = new TH1F("h_missing_omega_4scale_outoftime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_missing_omega_4scale_accidsub = new TH1F("h_missing_omega_4scale_accidsub","Mass Distribution",nbins_combined_hists,0.0,2.);		
	
	//Might be useful eventually in determining a shape for bad combos that enter gamma gamma pi+ pi- inv. mass
	TH1F* h_inv_omega_badpi0_intime = new TH1F("h_inv_omega_badpi0_intime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_inv_omega_badpi0_outoftime = new TH1F("h_inv_omega_badpi0_outoftime","Mass Distribution",nbins_combined_hists,0.0,2.);		
	TH1F* h_inv_omega_badpi0_accidsub = new TH1F("h_inv_omega_badpi0_accidsub","Mass Distribution",nbins_combined_hists,0.0,2.);		
	
	//Two dimensional histograms
	TH2F* h2_MM_vs_invm_loose_intime = new TH2F("h2_MM_vs_invm_loose_intime","MM vs. invariant mass",nbins_combined_hists,0.0,2.,nbins_combined_hists,0.0,2.);
	TH2F* h2_MM_vs_invm_loose_outoftime = new TH2F("h2_MM_vs_invm_loose_outoftime","MM vs. invariant mass",nbins_combined_hists,0.0,2.,nbins_combined_hists,0.0,2.);
	TH2F* h2_MM_vs_invm_loose_accidsub = new TH2F("h2_MM_vs_invm_loose_accidsub","MM vs. invariant mass",nbins_combined_hists,0.0,2.,nbins_combined_hists,0.0,2.);
	TH2F* h2_MM_vs_invm_pi0cuts_intime = new TH2F("h2_MM_vs_invm_pi0cuts_intime","MM vs. invariant mass",nbins_combined_hists,0.0,2.,nbins_combined_hists,0.0,2.);
	TH2F* h2_MM_vs_invm_pi0cuts_outoftime = new TH2F("h2_MM_vs_invm_pi0cuts_outoftime","MM vs. invariant mass",nbins_combined_hists,0.0,2.,nbins_combined_hists,0.0,2.);
	TH2F* h2_MM_vs_invm_pi0cuts_accidsub = new TH2F("h2_MM_vs_invm_pi0cuts_accidsub","MM vs. invariant mass",nbins_combined_hists,0.0,2.,nbins_combined_hists,0.0,2.);
	
	//Pi0 distributions of candidates
	TH1F* h_inv_pi0_all_intime = new TH1F("h_inv_pi0_all_intime","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_inv_pi0_all_outoftime = new TH1F("h_inv_pi0_all_outoftime","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_inv_pi0_all_accidsub = new TH1F("h_inv_pi0_all_accidsub","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	
	//Found photon stuff
	TH1F* h_gamma2_theta_all_intime = new TH1F("h_gamma2_theta_all_intime","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_gamma2_theta_all_outoftime = new TH1F("h_gamma2_theta_all_outoftime","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_gamma2_theta_all_accidsub = new TH1F("h_gamma2_theta_all_accidsub","Mass Distribution",nbins_combined_hists,0.0,45.);		
	TH1F* h_gamma2_E_all_intime = new TH1F("h_gamma2_E_all_intime","Mass Distribution",nbins_combined_hists,0.0,7.);		
	TH1F* h_gamma2_E_all_outoftime = new TH1F("h_gamma2_E_all_outoftime","Mass Distribution",nbins_combined_hists,0.0,7.);		
	TH1F* h_gamma2_E_all_accidsub = new TH1F("h_gamma2_E_all_accidsub","Mass Distribution",nbins_combined_hists,0.0,7.);		
	
	TH2F* h_gamma2_E_vs_theta_all_intime = new TH2F("h_gamma2_E_vs_theta_all_intime","Mass Distribution",nbins_combined_hists,0.0,45.,nbins_combined_hists,0.0,7.);		
	TH2F* h_gamma2_E_vs_theta_all_outoftime = new TH2F("h_gamma2_E_vs_theta_all_outoftime","Mass Distribution",nbins_combined_hists,0.0,45.,nbins_combined_hists,0.0,7.);		
	TH2F* h_gamma2_E_vs_theta_all_accidsub = new TH2F("h_gamma2_E_vs_theta_all_accidsub","Mass Distribution",nbins_combined_hists,0.0,45.,nbins_combined_hists,0.0,7.);		
	
	TH1F* h_m1num_miss_omega_Ebins_intime[N_E_BINS];
	TH1F* h_m1num_miss_omega_Ebins_outoftime[N_E_BINS];
	TH1F* h_m1num_miss_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m1den_miss_omega_Ebins_intime[N_E_BINS];
	TH1F* h_m1den_miss_omega_Ebins_outoftime[N_E_BINS];
	TH1F* h_m1den_miss_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m1num_miss_omega_Thetabins_intime[N_THETA_BINS];
	TH1F* h_m1num_miss_omega_Thetabins_outoftime[N_THETA_BINS];
	TH1F* h_m1num_miss_omega_Thetabins_accidsub[N_THETA_BINS];
	TH1F* h_m1den_miss_omega_Thetabins_intime[N_THETA_BINS];
	TH1F* h_m1den_miss_omega_Thetabins_outoftime[N_THETA_BINS];
	TH1F* h_m1den_miss_omega_Thetabins_accidsub[N_THETA_BINS];
	
	TH1F* h_m2num_inv_omega_Ebins_intime[N_E_BINS];
	TH1F* h_m2num_inv_omega_Ebins_outoftime[N_E_BINS];
	TH1F* h_m2num_inv_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m2denineff_miss_omega_Ebins_intime[N_E_BINS];
	TH1F* h_m2denineff_miss_omega_Ebins_outoftime[N_E_BINS];
	TH1F* h_m2denineff_miss_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m2num_inv_omega_Thetabins_intime[N_THETA_BINS];
	TH1F* h_m2num_inv_omega_Thetabins_outoftime[N_THETA_BINS];
	TH1F* h_m2num_inv_omega_Thetabins_accidsub[N_THETA_BINS];
	TH1F* h_m2denineff_miss_omega_Thetabins_intime[N_THETA_BINS];
	TH1F* h_m2denineff_miss_omega_Thetabins_outoftime[N_THETA_BINS];
	TH1F* h_m2denineff_miss_omega_Thetabins_accidsub[N_THETA_BINS];
	
 	for(int i =0; i<N_E_BINS; ++i) {
		char asdfasdf[20];
		sprintf(asdfasdf,"%d",i);
		TString index = asdfasdf;

		h_m1num_miss_omega_Ebins_intime[i] = new TH1F("h_m1num_miss_omega_Ebins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1num_miss_omega_Ebins_outoftime[i] = new TH1F("h_m1num_miss_omega_Ebins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1num_miss_omega_Ebins_accidsub[i] = new TH1F("h_m1num_miss_omega_Ebins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1den_miss_omega_Ebins_intime[i] = new TH1F("h_m1den_miss_omega_Ebins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1den_miss_omega_Ebins_outoftime[i] = new TH1F("h_m1den_miss_omega_Ebins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1den_miss_omega_Ebins_accidsub[i] = new TH1F("h_m1den_miss_omega_Ebins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		
		h_m2num_inv_omega_Ebins_intime[i] = new TH1F("h_m2num_inv_omega_Ebins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2num_inv_omega_Ebins_outoftime[i] = new TH1F("h_m2num_inv_omega_Ebins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2num_inv_omega_Ebins_accidsub[i] = new TH1F("h_m2num_inv_omega_Ebins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2denineff_miss_omega_Ebins_intime[i] = new TH1F("h_m2denineff_miss_omega_Ebins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2denineff_miss_omega_Ebins_outoftime[i] = new TH1F("h_m2denineff_miss_omega_Ebins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2denineff_miss_omega_Ebins_accidsub[i] = new TH1F("h_m2denineff_miss_omega_Ebins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
	}		
 	for(int i =0; i<N_THETA_BINS; ++i) {
		char asdfasdf[20];
		sprintf(asdfasdf,"%d",i);
		TString index = asdfasdf;

		h_m1num_miss_omega_Thetabins_intime[i] = new TH1F("h_m1num_miss_omega_Thetabins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1num_miss_omega_Thetabins_outoftime[i] = new TH1F("h_m1num_miss_omega_Thetabins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1num_miss_omega_Thetabins_accidsub[i] = new TH1F("h_m1num_miss_omega_Thetabins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1den_miss_omega_Thetabins_intime[i] = new TH1F("h_m1den_miss_omega_Thetabins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1den_miss_omega_Thetabins_outoftime[i] = new TH1F("h_m1den_miss_omega_Thetabins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m1den_miss_omega_Thetabins_accidsub[i] = new TH1F("h_m1den_miss_omega_Thetabins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		
		h_m2num_inv_omega_Thetabins_intime[i] = new TH1F("h_m2num_inv_omega_Thetabins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2num_inv_omega_Thetabins_outoftime[i] = new TH1F("h_m2num_inv_omega_Thetabins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2num_inv_omega_Thetabins_accidsub[i] = new TH1F("h_m2num_inv_omega_Thetabins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2denineff_miss_omega_Thetabins_intime[i] = new TH1F("h_m2denineff_miss_omega_Thetabins_intime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2denineff_miss_omega_Thetabins_outoftime[i] = new TH1F("h_m2denineff_miss_omega_Thetabins_outoftime"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
		h_m2denineff_miss_omega_Thetabins_accidsub[i] = new TH1F("h_m2denineff_miss_omega_Thetabins_accidsub"+index,"Mass Distribution",nbins_combined_hists,0.0,2.);		
	}		

	//Show cuts around missing pi0, which helps exclusivity
	TH1F* h_miss_pi0_all_intime = new TH1F("h_miss_pi0_all_intime","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_miss_pi0_all_outoftime = new TH1F("h_miss_pi0_all_outoftime","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_miss_pi0_all_accidsub = new TH1F("h_miss_pi0_all_accidsub","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_miss_pi0_postcuts_intime = new TH1F("h_miss_pi0_postcuts_intime","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_miss_pi0_postcuts_outoftime = new TH1F("h_miss_pi0_postcuts_outoftime","Mass Distribution",nbins_combined_hists,0.0,0.3);		
	TH1F* h_miss_pi0_postcuts_accidsub = new TH1F("h_miss_pi0_postcuts_accidsub","Mass Distribution",nbins_combined_hists,0.0,0.3);		

	TH1F* h_gam1_thrown_theta_inrange = NULL;
	TH1F* h_gam1_thrown_theta_inrange_nofound = NULL;
	TH1F* h_gam1_thrown_theta_inrange_FCALedge = NULL;
	TH1F* h_gam1_thrown_theta_inrange_FCALedge_nofound = NULL;
	TH1F* h_gam1_thrown_E_inrange = NULL;
	TH1F* h_gam1_thrown_E_inrange_nofound = NULL;
	TH1F* h_gam1_FOM_inrange = NULL;
	TH1F* h_gam2_FOM_inrange = NULL;
	TH1F* h_TwoGammaOpangle_inrange = NULL;
	
	TH1F* h_gam1_DeltaPhi = NULL;
	TH1F* h_gam1_DeltaTheta = NULL;
	TH1F* h_gam2_DeltaPhi = NULL;
	TH1F* h_gam2_DeltaTheta = NULL;
	
	if(make_thrown_info_hists) {
		h_gam1_thrown_theta_inrange = new TH1F("h_gam1_thrown_theta_inrange","Thrown Theta",nbins_combined_hists,0.0,15.);	
		h_gam1_thrown_theta_inrange_nofound = new TH1F("h_gam1_thrown_theta_inrange_nofound","Thrown Theta",nbins_combined_hists,0.0,15.);	
		h_gam1_thrown_theta_inrange_FCALedge = new TH1F("h_gam1_thrown_theta_inrange_FCALedge","Thrown Theta",nbins_combined_hists,0.0,15.);	
		h_gam1_thrown_theta_inrange_FCALedge_nofound = new TH1F("h_gam1_thrown_theta_inrange_FCALedge_nofound","Thrown Theta",nbins_combined_hists,0.0,15.);	
		h_gam1_thrown_E_inrange = new TH1F("h_gam1_thrown_E_inrange","Thrown E",nbins_combined_hists,0.0,7.);	
		h_gam1_thrown_E_inrange_nofound = new TH1F("h_gam1_thrown_E_inrange_nofound","Thrown E",nbins_combined_hists,0.0,7.);	
		h_gam1_FOM_inrange = new TH1F("h_gam1_FOM_inrange","FOM",nbins_combined_hists,0.0,1.);	
		h_gam2_FOM_inrange = new TH1F("h_gam2_FOM_inrange","FOM",nbins_combined_hists,0.0,1.);	
		h_TwoGammaOpangle_inrange = new TH1F("h_TwoGammaOpangle_inrange","Thrown Opening Angle",nbins_combined_hists,0.0,70.);	
		
		h_gam1_DeltaPhi = new TH1F("h_gam1_DeltaPhi",    "#Delta#phi thrown vs found, gamma1",nbins_combined_hists*10,-180.,180.);	
		h_gam1_DeltaTheta = new TH1F("h_gam1_DeltaTheta","#Delta#phi thrown vs found, gamma1",nbins_combined_hists*10,-180.,180.);	
		h_gam2_DeltaPhi = new TH1F("h_gam2_DeltaPhi",    "#Delta#phi thrown vs found, gamma2",nbins_combined_hists*10,-180.,180.);	
		h_gam2_DeltaTheta = new TH1F("h_gam2_DeltaTheta","#Delta#phi thrown vs found, gamma2",nbins_combined_hists*10,-180.,180.);	
	}
	
	for(size_t i =0; i<nentries; ++i) {
		t_in->GetEntry(i);
		if(i%100000==0) cout << "Entry: "  << i << endl;
		if(i>=max_events) {
			cout << "Max events reached. Exiting..." << endl;
			break;			
		}
		

		TLorentzVector p4_pipl = TLorentzVector(PiPlus_px,PiPlus_py,PiPlus_pz,PiPlus_E);
		TLorentzVector p4_pim = TLorentzVector(PiMinus_px,PiMinus_py,PiMinus_pz,PiMinus_E);
		TLorentzVector p4_proton = TLorentzVector(Proton_px,Proton_py,Proton_pz,Proton_E);
		TLorentzVector p4_gam1 = TLorentzVector(Gamma1_px,Gamma1_py,Gamma1_pz,Gamma1_E);
		TLorentzVector p4_gam2 = TLorentzVector(Gamma2_px,Gamma2_py,Gamma2_pz,Gamma2_E);
				
		TLorentzVector p4_pipl_thr;
		TLorentzVector p4_pim_thr;
		TLorentzVector p4_proton_thr;
		TLorentzVector p4_gam1_thr;
		TLorentzVector p4_gam2_thr;
				
		if(make_thrown_info_hists) {
			p4_pipl_thr = TLorentzVector(PiPlus_px_th,PiPlus_py_th,PiPlus_pz_th,PiPlus_E_th);
			p4_pim_thr = TLorentzVector(PiMinus_px_th,PiMinus_py_th,PiMinus_pz_th,PiMinus_E_th);
			p4_proton_thr = TLorentzVector(Proton_px_th,Proton_py_th,Proton_pz_th,Proton_E_th);
			p4_gam1_thr = TLorentzVector(Gamma1_px_th,Gamma1_py_th,Gamma1_pz_th,Gamma1_E_th);
			p4_gam2_thr = TLorentzVector(Gamma2_px_th,Gamma2_py_th,Gamma2_pz_th,Gamma2_E_th);
		}	
				
		if(USE_THROWN_4VEC) {
			p4_pipl = p4_pipl_thr;
			p4_pim = p4_pim_thr;
			p4_proton = p4_proton_thr;
			p4_gam1 =p4_gam1_thr;
			p4_gam2 =p4_gam2_thr;
		}
				
		TLorentzVector p4_show = TLorentzVector(MissingCan_Px[0],MissingCan_Py[0],MissingCan_Pz[0],MissingCan_E[0]);
				
		// Cuts on whole sample here
		if(Gamma2_E<FOUND_E_MIN) continue;
		//Extra effective PID
		if(PiPlus_beta_meas<0.95||PiPlus_beta_meas>1.05) continue;
		if(PiMinus_beta_meas<0.95||PiMinus_beta_meas>1.05) continue;
		if(Proton_beta_meas<0.15||Proton_beta_meas>0.9) continue;
		if(PiPlus_PIDhit==0||PiMinus_PIDhit==0||Proton_PIDhit==0) continue;
		
		Int_t my_E_bin = -1; 
		if(FCAL_STUDY) my_E_bin = GetEBin14_FCAL(p4_gam1.E());
		else my_E_bin = GetEBin_BCAL(p4_gam1.E());
		Int_t my_theta_bin = -1;
		if(FCAL_STUDY) my_theta_bin= GetThetaBin25FCAL(p4_gam1.Theta()*180./3.14159);
		else my_theta_bin = GetThetaBinBCAL(p4_gam1.Theta()*180./3.14159);
		
		if(fabs(rf_deltaT)<2) {
			h_all_missphoton_E_intime->Fill(p4_gam1.E());
			h_all_missphoton_theta_intime->Fill(p4_gam1.Theta()*180./3.14159);
			h_miss_pi0_all_intime->Fill(twogamma_mass_pkf);
			h_gamma2_theta_all_intime->Fill(p4_gam2.Theta()*180./3.14159);
			h_gamma2_E_all_intime->Fill(p4_gam2.E());
			h_gamma2_E_vs_theta_all_intime->Fill(p4_gam2.Theta()*180./3.14159,p4_gam2.E());
		}
		if(fabs(rf_deltaT)>2) {
			h_all_missphoton_E_outoftime->Fill(p4_gam1.E());
			h_all_missphoton_theta_outoftime->Fill(p4_gam1.Theta()*180./3.14159);
			h_miss_pi0_all_outoftime->Fill(twogamma_mass_pkf);
			h_gamma2_theta_all_outoftime->Fill(p4_gam2.Theta()*180./3.14159);
			h_gamma2_E_all_outoftime->Fill(p4_gam2.E());
			h_gamma2_E_vs_theta_all_outoftime->Fill(p4_gam2.Theta()*180./3.14159,p4_gam2.E());
		}
				
		//Don't seem to clean up sample...
		// if(PiPlus_TrackingChi2_ndf>2) continue;
		// if(PiMinus_TrackingChi2_ndf>2) continue;
		// if(Proton_TrackingChi2_ndf>2) continue;
		if(kinfit_chi2_ndf>30) continue;
		
		h_rf_deltaT_all->Fill(rf_deltaT);
		if(fabs(rf_deltaT)>2) h_rf_deltaT_outoftime->Fill(rf_deltaT);
				
		if(twogamma_mass_pkf<pi0_MMcut_lo||twogamma_mass_pkf>pi0_MMcut_hi) continue;
		if(FCAL_STUDY && (NFCALCandidates>1 || NBCALCandidates > 0) ) continue; //Match old selection and shorten.
		if(!FCAL_STUDY && (NFCALCandidates>0 || NBCALCandidates > 1) ) continue; //Match old selection and shorten.
		
		if(p4_gam1.Theta()*180./3.14159 < MISS_THETA_LOCUT || p4_gam1.Theta()*180./3.14159 > MISS_THETA_HICUT) continue;
		if(p4_gam1.E() < MISS_E_MIN) continue;
		
		// if(FCAL_STUDY && p4_gam1.Theta()*180./3.14159 > 13) continue; //Skip photons that point way outside FCAL
		// if(!FCAL_STUDY && p4_gam1.Theta()*180./3.14159 < 10) continue; //Skip photons that point way outside BCAL
				
		if(fabs(rf_deltaT)<2) {
			h_miss_pi0_postcuts_intime->Fill(twogamma_mass_pkf);
			h_accepted_missphoton_E_intime->Fill(p4_gam1.E());
			h_accepted_missphoton_theta_intime->Fill(p4_gam1.Theta()*180./3.14159);
			h_m1den_miss_omega_intime->Fill(RecM_Proton); //One good shower required to get here
			if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m1den_miss_omega_Ebins_intime[my_E_bin]->Fill(RecM_Proton);
			if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m1den_miss_omega_Thetabins_intime[my_theta_bin]->Fill(RecM_Proton);
			if(NFCALCandidates==0&&NBCALCandidates==0) {
				h_m2denineff_miss_omega_intime->Fill(RecM_Proton);
				if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m2denineff_miss_omega_Ebins_intime[my_E_bin]->Fill(RecM_Proton);
				if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m2denineff_miss_omega_Thetabins_intime[my_theta_bin]->Fill(RecM_Proton);
			}
			if(make_thrown_info_hists) {
				h_gam1_thrown_theta_inrange->Fill(p4_gam1_thr.Theta()*180./3.14159);
				if(NFCALCandidates==0&&NBCALCandidates==0) h_gam1_thrown_theta_inrange_nofound->Fill(p4_gam1_thr.Theta()*180./3.14159);
				if(p4_gam1_thr.Theta()*180./3.14159<1.3 || p4_gam1_thr.Theta()*180./3.14159>9.8) {
					h_gam1_thrown_theta_inrange_FCALedge->Fill(p4_gam1_thr.Theta()*180./3.14159);
					if(NFCALCandidates==0&&NBCALCandidates==0) h_gam1_thrown_theta_inrange_FCALedge_nofound->Fill(p4_gam1_thr.Theta()*180./3.14159);
				}
				h_gam1_thrown_E_inrange->Fill(p4_gam1_thr.E());
				if(NFCALCandidates==0&&NBCALCandidates==0) h_gam1_thrown_E_inrange_nofound->Fill(p4_gam1_thr.E());
				h_gam1_FOM_inrange->Fill(Gamma1_FOM);
				h_gam2_FOM_inrange->Fill(Gamma2_FOM);
				h_TwoGammaOpangle_inrange->Fill(two_gamma_opangle_thr);
				
				if(NFCALCandidates==1 || NBCALCandidates ==1) {
					Double_t gam1_DeltaPhi = p4_gam1_thr.Phi()-p4_show.Phi();
					if(gam1_DeltaPhi<-3.14159) gam1_DeltaPhi+=3.14159; //keep within 180 degrees
					if(gam1_DeltaPhi> 3.14159) gam1_DeltaPhi-=3.14159; //keep within 180 degrees
					h_gam1_DeltaPhi->Fill(gam1_DeltaPhi*180./3.14159);
					Double_t gam1_DeltaTheta = p4_gam1_thr.Theta()-p4_show.Theta();
					if(gam1_DeltaTheta<-3.14159) gam1_DeltaTheta+=3.14159; //keep within 180 degrees
					if(gam1_DeltaTheta> 3.14159) gam1_DeltaTheta-=3.14159; //keep within 180 degrees
					h_gam1_DeltaTheta->Fill(gam1_DeltaTheta*180./3.14159);
				}
				
				Double_t gam2_DeltaPhi = p4_gam2_thr.Phi()-p4_gam2.Phi();
				if(gam2_DeltaPhi<-3.14159) gam2_DeltaPhi+=3.14159; //keep within 180 degrees
				if(gam2_DeltaPhi> 3.14159) gam2_DeltaPhi-=3.14159; //keep within 180 degrees
				h_gam2_DeltaPhi->Fill(gam2_DeltaPhi*180./3.14159);
				Double_t gam2_DeltaTheta = p4_gam2_thr.Theta()-p4_gam2.Theta();
				if(gam2_DeltaTheta<-3.14159) gam2_DeltaTheta+=3.14159; //keep within 180 degrees
				if(gam2_DeltaTheta> 3.14159) gam2_DeltaTheta-=3.14159; //keep within 180 degrees
				h_gam2_DeltaTheta->Fill(gam2_DeltaTheta*180./3.14159);
				
			}
		}
		if(fabs(rf_deltaT)>2) {
			h_miss_pi0_postcuts_outoftime->Fill(twogamma_mass_pkf);
			h_accepted_missphoton_E_outoftime->Fill(p4_gam1.E());
			h_accepted_missphoton_theta_outoftime->Fill(p4_gam1.Theta()*180./3.14159);
			h_m1den_miss_omega_outoftime->Fill(RecM_Proton); //One good shower required to get here
			if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m1den_miss_omega_Ebins_outoftime[my_E_bin]->Fill(RecM_Proton);
			if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m1den_miss_omega_Thetabins_outoftime[my_theta_bin]->Fill(RecM_Proton);
			if(NFCALCandidates==0&&NBCALCandidates==0) {
				h_m2denineff_miss_omega_outoftime->Fill(RecM_Proton);
				if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m2denineff_miss_omega_Ebins_outoftime[my_E_bin]->Fill(RecM_Proton);
				if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m2denineff_miss_omega_Thetabins_outoftime[my_theta_bin]->Fill(RecM_Proton);
			}
		}
		
		//Loop over shower candidates
 		for (size_t j =0; j<NumNeutralCombosSaved; ++j) {
						
						
			if(MissingCan_E[j] < 0.0001) break; // No filled entries past this point
						
			TLorentzVector p4_show = TLorentzVector(MissingCan_Px[j],MissingCan_Py[j],MissingCan_Pz[j],MissingCan_E[j]);
			// Old Deltaphi betwen proton and everything else
			Double_t DeltaPhi_wshow = fabs( p4_proton.Phi() - (p4_show+p4_gam2+p4_pipl+p4_pim).Phi())*180./3.14159  ;
			// New Deltaphi betwen shower candidate and everything else
			// Double_t DeltaPhi_wshow = fabs( p4_show.Phi() - (p4_gam2+p4_pipl+p4_pim+p4_proton).Phi())*180./3.14159  ;
			
			
			//Quality cuts for shower candidates
			// if(twogamma_can_mass[j]<m1_pi0cut_lo||twogamma_can_mass[j]>m1_pi0cut_hi) continue;
			// if(fabs(DeltaPhi_wshow-180.)>15) continue; //Doesn't really cut much
		
		
			if(fabs(rf_deltaT)<2) {
				h_deltaphi_all_intime->Fill(DeltaPhi_wshow);
				h_inv_pi0_all_intime->Fill(twogamma_can_mass[j]); //One good shower required to get here
				if(twogamma_can_mass[j]>m1_pi0cut_lo&&twogamma_can_mass[j]<m1_pi0cut_hi&&fabs(DeltaPhi_wshow-180.)<m1_DeltaPhiCut) {
					h_m1num_miss_omega_intime->Fill(RecM_Proton); //One good shower required to get here
					if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m1num_miss_omega_Ebins_intime[my_E_bin]->Fill(RecM_Proton);
					if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m1num_miss_omega_Thetabins_intime[my_theta_bin]->Fill(RecM_Proton);
				}
				if(twogamma_can_mass[j]<m1_pi0cut_lo) h_inv_omega_badpi0_intime->Fill(threepi_can_mass[j]); //One good shower required to get here
				h_m2num_inv_omega_intime->Fill(threepi_can_mass[j]); //One good shower required to get here
				if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m2num_inv_omega_Ebins_intime[my_E_bin]->Fill(RecM_Proton);
				if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m2num_inv_omega_Thetabins_intime[my_theta_bin]->Fill(RecM_Proton);
				h_missing_omega_4scale_intime->Fill(RecM_Proton); //One good shower required to get here
				if(twogamma_can_mass[j]>m1_pi0cut_lo&&twogamma_can_mass[j]<m1_pi0cut_hi&&fabs(DeltaPhi_wshow-180.)<m1_DeltaPhiCut) h2_MM_vs_invm_pi0cuts_intime->Fill(RecM_Proton,threepi_can_mass[j]); //One good shower required to get here
				h2_MM_vs_invm_loose_intime->Fill(RecM_Proton,threepi_can_mass[j]); //One good shower required to get here
			}
			if(fabs(rf_deltaT)>2) {
				h_deltaphi_all_outoftime->Fill(DeltaPhi_wshow);
				h_inv_pi0_all_outoftime->Fill(twogamma_can_mass[j]); //One good shower required to get here
				if(twogamma_can_mass[j]>m1_pi0cut_lo&&twogamma_can_mass[j]<m1_pi0cut_hi&&fabs(DeltaPhi_wshow-180.)<m1_DeltaPhiCut) {
					h_m1num_miss_omega_outoftime->Fill(RecM_Proton); //One good shower required to get here
					if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m1num_miss_omega_Ebins_outoftime[my_E_bin]->Fill(RecM_Proton);
					if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m1num_miss_omega_Thetabins_outoftime[my_theta_bin]->Fill(RecM_Proton);
				}
				if(twogamma_can_mass[j]<m1_pi0cut_lo) h_inv_omega_badpi0_outoftime->Fill(threepi_can_mass[j]); //One good shower required to get here
				h_m2num_inv_omega_outoftime->Fill(threepi_can_mass[j]); //One good shower required to get here
				if(my_E_bin>=0&&my_E_bin<N_E_BINS&&p4_gam1.Theta()*180./3.14159>gamma1_miss_lotheta&&p4_gam1.Theta()*180./3.14159<gamma1_miss_hitheta) h_m2num_inv_omega_Ebins_outoftime[my_E_bin]->Fill(RecM_Proton);
				if(my_theta_bin>=0&&my_theta_bin<N_THETA_BINS&&p4_gam1.E()>gamma1_miss_Emin) h_m2num_inv_omega_Thetabins_outoftime[my_theta_bin]->Fill(RecM_Proton);
				h_missing_omega_4scale_outoftime->Fill(RecM_Proton); //One good shower required to get here
				if(twogamma_can_mass[j]>m1_pi0cut_lo&&twogamma_can_mass[j]<m1_pi0cut_hi&&fabs(DeltaPhi_wshow-180.)<m1_DeltaPhiCut) h2_MM_vs_invm_pi0cuts_outoftime->Fill(RecM_Proton,threepi_can_mass[j]); //One good shower required to get here
				h2_MM_vs_invm_loose_outoftime->Fill(RecM_Proton,threepi_can_mass[j]); //One good shower required to get here
			}
		}
		
	}
	
	cout << "Finished on run " << Run << endl;

	h_all_missphoton_E_intime->Sumw2();
	h_all_missphoton_E_outoftime->Sumw2();
	h_all_missphoton_theta_intime->Sumw2();
	h_all_missphoton_theta_outoftime->Sumw2();
	h_accepted_missphoton_E_intime->Sumw2();
	h_accepted_missphoton_E_outoftime->Sumw2();
	h_accepted_missphoton_theta_intime->Sumw2();
	h_accepted_missphoton_theta_outoftime->Sumw2();

	h_m1num_miss_omega_intime->Sumw2();
	h_m1num_miss_omega_outoftime->Sumw2();
	h_m1den_miss_omega_intime->Sumw2();
	h_m1den_miss_omega_outoftime->Sumw2();
	
	h_m2num_inv_omega_intime->Sumw2();
	h_m2num_inv_omega_outoftime->Sumw2();
	h_m2denineff_miss_omega_intime->Sumw2();
	h_m2denineff_miss_omega_outoftime->Sumw2();


	h_missing_omega_4scale_intime->Sumw2();
	h_missing_omega_4scale_outoftime->Sumw2();

	h_inv_omega_badpi0_intime->Sumw2();
	h_inv_omega_badpi0_outoftime->Sumw2();

	h2_MM_vs_invm_loose_intime->Sumw2();
	h2_MM_vs_invm_loose_outoftime->Sumw2();
	h2_MM_vs_invm_pi0cuts_intime->Sumw2();
	h2_MM_vs_invm_pi0cuts_outoftime->Sumw2();

	h_inv_pi0_all_intime->Sumw2();
	h_inv_pi0_all_outoftime->Sumw2();

	h_miss_pi0_all_intime->Sumw2();
	h_miss_pi0_all_outoftime->Sumw2();
	h_miss_pi0_postcuts_intime->Sumw2();
	h_miss_pi0_postcuts_outoftime->Sumw2();
	
	
	Double_t Delta_t_scalefactor = 1. / (2*NBeamBunchesOutoftime);
	
	
	h_all_missphoton_E_accidsub->Add(h_all_missphoton_E_intime,h_all_missphoton_E_outoftime,1.,-1*Delta_t_scalefactor);
	h_all_missphoton_theta_accidsub->Add(h_all_missphoton_theta_intime,h_all_missphoton_theta_outoftime,1.,-1*Delta_t_scalefactor);
	h_accepted_missphoton_E_accidsub->Add(h_accepted_missphoton_E_intime,h_accepted_missphoton_E_outoftime,1.,-1*Delta_t_scalefactor);
	h_accepted_missphoton_theta_accidsub->Add(h_accepted_missphoton_theta_intime,h_accepted_missphoton_theta_outoftime,1.,-1*Delta_t_scalefactor);
		
	h_rf_deltaT_accidsub->Add(h_rf_deltaT_all,h_rf_deltaT_outoftime,1.,-1);

	h_deltaphi_all_accidsub->Add(h_deltaphi_all_intime,h_deltaphi_all_outoftime,1.,-1*Delta_t_scalefactor);

	h_m1num_miss_omega_accidsub->Add(h_m1num_miss_omega_intime,h_m1num_miss_omega_outoftime,1.,-1*Delta_t_scalefactor);
	h_m1den_miss_omega_accidsub->Add(h_m1den_miss_omega_intime,h_m1den_miss_omega_outoftime,1.,-1*Delta_t_scalefactor);

	h_m2num_inv_omega_accidsub->Add(h_m2num_inv_omega_intime,h_m2num_inv_omega_outoftime,1.,-1*Delta_t_scalefactor);
	h_m2denineff_miss_omega_accidsub->Add(h_m2denineff_miss_omega_intime,h_m2denineff_miss_omega_outoftime,1.,-1*Delta_t_scalefactor);

	h_missing_omega_4scale_accidsub->Add(h_missing_omega_4scale_intime,h_missing_omega_4scale_outoftime,1.,-1*Delta_t_scalefactor);

	h_inv_omega_badpi0_accidsub->Add(h_inv_omega_badpi0_intime,h_inv_omega_badpi0_outoftime,1.,-1*Delta_t_scalefactor);

	h2_MM_vs_invm_loose_accidsub->Add(h2_MM_vs_invm_loose_intime,h2_MM_vs_invm_loose_outoftime,1.,-1*Delta_t_scalefactor);
	h2_MM_vs_invm_pi0cuts_accidsub->Add(h2_MM_vs_invm_pi0cuts_intime,h2_MM_vs_invm_pi0cuts_outoftime,1.,-1*Delta_t_scalefactor);

	h_inv_pi0_all_accidsub->Add(h_inv_pi0_all_intime,h_inv_pi0_all_outoftime,1.,-1*Delta_t_scalefactor);

	h_miss_pi0_all_accidsub->Add(h_miss_pi0_all_intime,h_miss_pi0_all_outoftime,1.,-1*Delta_t_scalefactor);
	h_miss_pi0_postcuts_accidsub->Add(h_miss_pi0_postcuts_intime,h_miss_pi0_postcuts_outoftime,1.,-1*Delta_t_scalefactor);
	
	h_gamma2_theta_all_accidsub->Add(h_gamma2_theta_all_intime,h_gamma2_theta_all_outoftime,1.,-1*Delta_t_scalefactor);
	h_gamma2_E_all_accidsub->Add(h_gamma2_E_all_intime,h_gamma2_E_all_outoftime,1.,-1*Delta_t_scalefactor);
	h_gamma2_E_vs_theta_all_accidsub->Add(h_gamma2_E_vs_theta_all_intime,h_gamma2_E_vs_theta_all_outoftime,1.,-1*Delta_t_scalefactor);
	
 	for(int i =0; i<N_E_BINS; ++i) {
		h_m1num_miss_omega_Ebins_intime[i]->Sumw2();
		h_m1num_miss_omega_Ebins_outoftime[i]->Sumw2();
		h_m1den_miss_omega_Ebins_intime[i]->Sumw2();
		h_m1den_miss_omega_Ebins_outoftime[i]->Sumw2();
		h_m2num_inv_omega_Ebins_intime[i]->Sumw2();
		h_m2num_inv_omega_Ebins_outoftime[i]->Sumw2();
		h_m2denineff_miss_omega_Ebins_intime[i]->Sumw2();
		h_m2denineff_miss_omega_Ebins_outoftime[i]->Sumw2();
		
		h_m1num_miss_omega_Ebins_accidsub[i]->Add(h_m1num_miss_omega_Ebins_intime[i],h_m1num_miss_omega_Ebins_outoftime[i],1.,-1*Delta_t_scalefactor);
		h_m1den_miss_omega_Ebins_accidsub[i]->Add(h_m1den_miss_omega_Ebins_intime[i],h_m1den_miss_omega_Ebins_outoftime[i],1.,-1*Delta_t_scalefactor);
		h_m2num_inv_omega_Ebins_accidsub[i]->Add(h_m2num_inv_omega_Ebins_intime[i],h_m2num_inv_omega_Ebins_outoftime[i],1.,-1*Delta_t_scalefactor);
		h_m2denineff_miss_omega_Ebins_accidsub[i]->Add(h_m2denineff_miss_omega_Ebins_intime[i],h_m2denineff_miss_omega_Ebins_outoftime[i],1.,-1*Delta_t_scalefactor);
	}	
	
 	for(int i =0; i<N_THETA_BINS; ++i) {
		h_m1num_miss_omega_Thetabins_intime[i]->Sumw2();
		h_m1num_miss_omega_Thetabins_outoftime[i]->Sumw2();
		h_m1den_miss_omega_Thetabins_intime[i]->Sumw2();
		h_m1den_miss_omega_Thetabins_outoftime[i]->Sumw2();
		h_m2num_inv_omega_Thetabins_intime[i]->Sumw2();
		h_m2num_inv_omega_Thetabins_outoftime[i]->Sumw2();
		h_m2denineff_miss_omega_Thetabins_intime[i]->Sumw2();
		h_m2denineff_miss_omega_Thetabins_outoftime[i]->Sumw2();
		
		h_m1num_miss_omega_Thetabins_accidsub[i]->Add(h_m1num_miss_omega_Thetabins_intime[i],h_m1num_miss_omega_Thetabins_outoftime[i],1.,-1*Delta_t_scalefactor);
		h_m1den_miss_omega_Thetabins_accidsub[i]->Add(h_m1den_miss_omega_Thetabins_intime[i],h_m1den_miss_omega_Thetabins_outoftime[i],1.,-1*Delta_t_scalefactor);
		h_m2num_inv_omega_Thetabins_accidsub[i]->Add(h_m2num_inv_omega_Thetabins_intime[i],h_m2num_inv_omega_Thetabins_outoftime[i],1.,-1*Delta_t_scalefactor);
		h_m2denineff_miss_omega_Thetabins_accidsub[i]->Add(h_m2denineff_miss_omega_Thetabins_intime[i],h_m2denineff_miss_omega_Thetabins_outoftime[i],1.,-1*Delta_t_scalefactor);
	}	
	
	TFile* outfile = new TFile( str_outfile, "RECREATE" );
	outfile->cd();
	
	h_all_missphoton_E_accidsub->Write();
	h_all_missphoton_theta_accidsub->Write();
	h_accepted_missphoton_E_accidsub->Write();
	h_accepted_missphoton_theta_accidsub->Write();
	
	h_rf_deltaT_all->Write();
	h_rf_deltaT_outoftime->Write();
	h_rf_deltaT_accidsub->Write();

	h_deltaphi_all_accidsub->Write();

	h_m1num_miss_omega_intime->Write();
	h_m1num_miss_omega_outoftime->Write();
	h_m1num_miss_omega_accidsub->Write();
	h_m1den_miss_omega_accidsub->Write();

	h_m2num_inv_omega_accidsub->Write();
	h_m2denineff_miss_omega_accidsub->Write();

	h_missing_omega_4scale_accidsub->Write();

	h_inv_omega_badpi0_accidsub->Write();

	h2_MM_vs_invm_loose_accidsub->Write();
	h2_MM_vs_invm_pi0cuts_accidsub->Write();

	h_inv_pi0_all_accidsub->Write();

	h_miss_pi0_all_accidsub->Write();
	h_miss_pi0_postcuts_accidsub->Write();
	
	h_gamma2_theta_all_accidsub->Write();
	h_gamma2_E_all_accidsub->Write();
	h_gamma2_E_vs_theta_all_accidsub->Write();

	for(int i =0; i<N_E_BINS; ++i) h_m1num_miss_omega_Ebins_accidsub[i]->Write();
	for(int i =0; i<N_E_BINS; ++i) h_m1den_miss_omega_Ebins_accidsub[i]->Write();
	for(int i =0; i<N_E_BINS; ++i) h_m2num_inv_omega_Ebins_accidsub[i]->Write();
	for(int i =0; i<N_E_BINS; ++i) h_m2denineff_miss_omega_Ebins_accidsub[i]->Write();
	for(int i =0; i<N_THETA_BINS; ++i) h_m1num_miss_omega_Thetabins_accidsub[i]->Write();
	for(int i =0; i<N_THETA_BINS; ++i) h_m1den_miss_omega_Thetabins_accidsub[i]->Write();
	for(int i =0; i<N_THETA_BINS; ++i) h_m2num_inv_omega_Thetabins_accidsub[i]->Write();
	for(int i =0; i<N_THETA_BINS; ++i) h_m2denineff_miss_omega_Thetabins_accidsub[i]->Write();
	
	if(make_thrown_info_hists) {
		h_gam1_thrown_theta_inrange->Write();
		h_gam1_thrown_theta_inrange_nofound->Write();
		h_gam1_thrown_theta_inrange_FCALedge->Write();
		h_gam1_thrown_theta_inrange_FCALedge_nofound->Write();
		h_gam1_thrown_E_inrange->Write();
		h_gam1_thrown_E_inrange_nofound->Write();
		h_gam1_FOM_inrange->Write();
		h_gam2_FOM_inrange->Write();
		h_TwoGammaOpangle_inrange->Write();
		
		h_gam1_DeltaPhi->Write();
		h_gam1_DeltaTheta->Write();
		h_gam2_DeltaPhi->Write();
		h_gam2_DeltaTheta->Write();
	}
	
	
	outfile->Close();


	

	cout << "Done" << endl;
	return 0;
	
}
