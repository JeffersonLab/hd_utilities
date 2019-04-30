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

Long64_t max_events = 5000000000;

Bool_t cut_2D_tail = false;
Double_t pi0_MMcut_lo = 0.10;
Double_t pi0_MMcut_hi = 0.17;


Int_t flatten_output(TString str_infile, TString str_outfile, Bool_t has_thrown_info = false, Bool_t cut_pi0MM=false) {
	
	TFile* f_in = new TFile(str_infile);
	TTree* t_in = (TTree*)f_in->FindObjectAny("p3pi_FCALStudy");
	
	TFile* f_out = new TFile(str_outfile,"RECREATE");
	TTree* t_out = new TTree("p3pi_FCALStudy","");

	
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
	Int_t   PiPlus_Tracking_ndf;   t_in->SetBranchAddress("PiPlus_Tracking_ndf",&PiPlus_Tracking_ndf);
	Double_t   PiPlus_TimingChi2_ndf;   t_in->SetBranchAddress("PiPlus_TimingChi2_ndf",&PiPlus_TimingChi2_ndf);
	Double_t   PiPlus_dEdxChi2_ndf;   t_in->SetBranchAddress("PiPlus_dEdxChi2_ndf",&PiPlus_dEdxChi2_ndf);
	Double_t   PiPlus_beta_meas;   t_in->SetBranchAddress("PiPlus_beta_meas",&PiPlus_beta_meas);
	Int_t   PiMinus_PIDhit;  t_in->SetBranchAddress("PiMinus_PIDhit",&PiMinus_PIDhit);
	Double_t   PiMinus_TrackingChi2_ndf;   t_in->SetBranchAddress("PiMinus_TrackingChi2_ndf",&PiMinus_TrackingChi2_ndf);
	Int_t   PiMinus_Tracking_ndf;   t_in->SetBranchAddress("PiMinus_Tracking_ndf",&PiMinus_Tracking_ndf);
	Double_t   PiMinus_TimingChi2_ndf;   t_in->SetBranchAddress("PiMinus_TimingChi2_ndf",&PiMinus_TimingChi2_ndf);
	Double_t   PiMinus_dEdxChi2_ndf;   t_in->SetBranchAddress("PiMinus_dEdxChi2_ndf",&PiMinus_dEdxChi2_ndf);
	Double_t   PiMinus_beta_meas;   t_in->SetBranchAddress("PiMinus_beta_meas",&PiMinus_beta_meas);
	Int_t   Proton_PIDhit;   t_in->SetBranchAddress("Proton_PIDhit",&Proton_PIDhit);
	Double_t   Proton_TrackingChi2_ndf;   t_in->SetBranchAddress("Proton_TrackingChi2_ndf",&Proton_TrackingChi2_ndf);
	Int_t   Proton_Tracking_ndf;   t_in->SetBranchAddress("Proton_Tracking_ndf",&Proton_Tracking_ndf);
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
	
	Double_t PiPlus_px_th; if(has_thrown_info) t_in->SetBranchAddress("PiPlus_px_th",&PiPlus_px_th);
	Double_t PiPlus_py_th; if(has_thrown_info) t_in->SetBranchAddress("PiPlus_py_th",&PiPlus_py_th);
	Double_t PiPlus_pz_th; if(has_thrown_info) t_in->SetBranchAddress("PiPlus_pz_th",&PiPlus_pz_th);
	Double_t PiPlus_E_th;  if(has_thrown_info) t_in->SetBranchAddress("PiPlus_E_th",&  PiPlus_E_th);
	Double_t PiMinus_px_th; if(has_thrown_info) t_in->SetBranchAddress("PiMinus_px_th",&PiMinus_px_th);
	Double_t PiMinus_py_th; if(has_thrown_info) t_in->SetBranchAddress("PiMinus_py_th",&PiMinus_py_th);
	Double_t PiMinus_pz_th; if(has_thrown_info) t_in->SetBranchAddress("PiMinus_pz_th",&PiMinus_pz_th);
	Double_t PiMinus_E_th;  if(has_thrown_info) t_in->SetBranchAddress("PiMinus_E_th",&  PiMinus_E_th);
	Double_t Proton_px_th; if(has_thrown_info) t_in->SetBranchAddress("Proton_px_th",&Proton_px_th);
	Double_t Proton_py_th; if(has_thrown_info) t_in->SetBranchAddress("Proton_py_th",&Proton_py_th);
	Double_t Proton_pz_th; if(has_thrown_info) t_in->SetBranchAddress("Proton_pz_th",&Proton_pz_th);
	Double_t Proton_E_th;  if(has_thrown_info) t_in->SetBranchAddress("Proton_E_th",&  Proton_E_th);
	Double_t Gamma1_px_th; if(has_thrown_info) t_in->SetBranchAddress("Gamma1_px_th",&Gamma1_px_th);
	Double_t Gamma1_py_th; if(has_thrown_info) t_in->SetBranchAddress("Gamma1_py_th",&Gamma1_py_th);
	Double_t Gamma1_pz_th; if(has_thrown_info) t_in->SetBranchAddress("Gamma1_pz_th",&Gamma1_pz_th);
	Double_t Gamma1_E_th;  if(has_thrown_info) t_in->SetBranchAddress("Gamma1_E_th",&  Gamma1_E_th);
	Double_t Gamma2_px_th; if(has_thrown_info) t_in->SetBranchAddress("Gamma2_px_th",&Gamma2_px_th);
	Double_t Gamma2_py_th; if(has_thrown_info) t_in->SetBranchAddress("Gamma2_py_th",&Gamma2_py_th);
	Double_t Gamma2_pz_th; if(has_thrown_info) t_in->SetBranchAddress("Gamma2_pz_th",&Gamma2_pz_th);
	Double_t Gamma2_E_th;  if(has_thrown_info) t_in->SetBranchAddress("Gamma2_E_th",&  Gamma2_E_th);
	
	Double_t Gamma1_FOM;  if(has_thrown_info) t_in->SetBranchAddress("Gamma1_FOM",  &Gamma1_FOM);
	Double_t Gamma2_FOM;  if(has_thrown_info) t_in->SetBranchAddress("Gamma2_FOM",  &Gamma2_FOM);
	Double_t two_gamma_opangle_thr;  if(has_thrown_info) t_in->SetBranchAddress("two_gamma_opangle_thr",  &two_gamma_opangle_thr);
	Double_t gam2_ThrRecon_opangle;  if(has_thrown_info) t_in->SetBranchAddress("gam2_ThrRecon_opangle",  &gam2_ThrRecon_opangle);
	
	
	
	///////////////
	
	Double_t RecM_Proton_out;    t_out->Branch("RecM_Proton",&RecM_Proton_out);
	Double_t RecM_Proton_pkf_out;    t_out->Branch("RecM_Proton_pkf",&RecM_Proton_pkf_out);
	Double_t threepi_mass_out;    t_out->Branch("threepi_mass",&threepi_mass_out);
	Double_t threepi_mass_pkf_out;    t_out->Branch("threepi_mass_pkf",&threepi_mass_pkf_out);
	Double_t twogamma_mass_out;    t_out->Branch("twogamma_mass",&twogamma_mass_out);
	Double_t twogamma_mass_pkf_out;    t_out->Branch("twogamma_mass_pkf",&twogamma_mass_pkf_out);
	
	Double_t PiPlus_px_out;       t_out->Branch("PiPlus_px",&PiPlus_px_out);
	Double_t PiPlus_py_out;       t_out->Branch("PiPlus_py",&PiPlus_py_out);
	Double_t PiPlus_pz_out;       t_out->Branch("PiPlus_pz",&PiPlus_pz_out);
	Double_t PiPlus_E_out;        t_out->Branch("PiPlus_E",&PiPlus_E_out);
	Double_t PiMinus_px_out;      t_out->Branch("PiMinus_px",&PiMinus_px_out);
	Double_t PiMinus_py_out;      t_out->Branch("PiMinus_py",&PiMinus_py_out);
	Double_t PiMinus_pz_out;      t_out->Branch("PiMinus_pz",&PiMinus_pz_out);
	Double_t PiMinus_E_out;       t_out->Branch("PiMinus_E",&PiMinus_E_out);
	Double_t Proton_px_out;       t_out->Branch("Proton_px",&Proton_px_out);
	Double_t Proton_py_out;       t_out->Branch("Proton_py",&Proton_py_out);
	Double_t Proton_pz_out;       t_out->Branch("Proton_pz",&Proton_pz_out);
	Double_t Proton_E_out;        t_out->Branch("Proton_E",&Proton_E_out);
	Double_t Gamma1_px_out;       t_out->Branch("Gamma1_px",&Gamma1_px_out);
	Double_t Gamma1_py_out;       t_out->Branch("Gamma1_py",&Gamma1_py_out);
	Double_t Gamma1_pz_out;       t_out->Branch("Gamma1_pz",&Gamma1_pz_out);
	Double_t Gamma1_E_out;        t_out->Branch("Gamma1_E",&Gamma1_E_out);
	Double_t Gamma2_px_out;       t_out->Branch("Gamma2_px",&Gamma2_px_out);
	Double_t Gamma2_py_out;       t_out->Branch("Gamma2_py",&Gamma2_py_out);
	Double_t Gamma2_pz_out;       t_out->Branch("Gamma2_pz",&Gamma2_pz_out);
	Double_t Gamma2_E_out;        t_out->Branch("Gamma2_E",&Gamma2_E_out);
	
	Int_t   PiPlus_PIDhit_out;    t_out->Branch("PiPlus_PIDhit",&PiPlus_PIDhit_out);
	Double_t   PiPlus_TrackingChi2_ndf_out;    t_out->Branch("PiPlus_TrackingChi2_ndf",&PiPlus_TrackingChi2_ndf_out);
	Int_t   PiPlus_Tracking_ndf_out;    t_out->Branch("PiPlus_Tracking_ndf",&PiPlus_Tracking_ndf_out);
	Double_t   PiPlus_TimingChi2_ndf_out;    t_out->Branch("PiPlus_TimingChi2_ndf",&PiPlus_TimingChi2_ndf_out);
	Double_t   PiPlus_dEdxChi2_ndf_out;    t_out->Branch("PiPlus_dEdxChi2_ndf",&PiPlus_dEdxChi2_ndf_out);
	Double_t   PiPlus_beta_meas_out;    t_out->Branch("PiPlus_beta_meas",&PiPlus_beta_meas_out);
	Int_t   PiMinus_PIDhit_out;   t_out->Branch("PiMinus_PIDhit",&PiMinus_PIDhit_out);
	Double_t   PiMinus_TrackingChi2_ndf_out;    t_out->Branch("PiMinus_TrackingChi2_ndf",&PiMinus_TrackingChi2_ndf_out);
	Int_t   PiMinus_Tracking_ndf_out;    t_out->Branch("PiMinus_Tracking_ndf",&PiMinus_Tracking_ndf_out);
	Double_t   PiMinus_TimingChi2_ndf_out;    t_out->Branch("PiMinus_TimingChi2_ndf",&PiMinus_TimingChi2_ndf_out);
	Double_t   PiMinus_dEdxChi2_ndf_out;    t_out->Branch("PiMinus_dEdxChi2_ndf",&PiMinus_dEdxChi2_ndf_out);
	Double_t   PiMinus_beta_meas_out;    t_out->Branch("PiMinus_beta_meas",&PiMinus_beta_meas_out);
	Int_t   Proton_PIDhit_out;    t_out->Branch("Proton_PIDhit",&Proton_PIDhit_out);
	Double_t   Proton_TrackingChi2_ndf_out;    t_out->Branch("Proton_TrackingChi2_ndf",&Proton_TrackingChi2_ndf_out);
	Int_t   Proton_Tracking_ndf_out;    t_out->Branch("Proton_Tracking_ndf",&Proton_Tracking_ndf_out);
	Double_t   Proton_TimingChi2_ndf_out;    t_out->Branch("Proton_TimingChi2_ndf",&Proton_TimingChi2_ndf_out);
	Double_t   Proton_dEdxChi2_ndf_out;    t_out->Branch("Proton_dEdxChi2_ndf",&Proton_dEdxChi2_ndf_out);
	Double_t   Proton_beta_meas_out;    t_out->Branch("Proton_beta_meas",&Proton_beta_meas_out);

	Double_t MM2_out;    t_out->Branch("MM2",&MM2_out);

	Double_t rf_deltaT_out;    t_out->Branch("rf_deltaT",&rf_deltaT_out);
	Double_t beamE_out;    t_out->Branch("beamE",&beamE_out);
	Double_t vertex_z_out;    t_out->Branch("vertex_z",&vertex_z_out);
	Double_t vertex_r_out;    t_out->Branch("vertex_r",&vertex_r_out);
	Int_t NExtraTracks_out;    t_out->Branch("NExtraTracks",&NExtraTracks_out);
	Double_t ExtraShowerE_out;    t_out->Branch("ExtraShowerE",&ExtraShowerE_out);
	
	Double_t kinfit_CL_out;    t_out->Branch("kinfit_CL",&kinfit_CL_out);
	Double_t kinfit_chi2_ndf_out;    t_out->Branch("kinfit_chi2_ndf",&kinfit_chi2_ndf_out);
	
	Int_t Run_out;    t_out->Branch("Run",&Run_out);
	Int_t Event_out;    t_out->Branch("Event",&Event_out);

	t_out->Branch("NumNeutralCombosSaved",&NumNeutralCombosSaved,"NumNeutralCombosSaved/I");
	Double_t MissingCan_Px_out[NumNeutralCombosSaved]; t_out->Branch("MissingCan_Px",MissingCan_Px_out,"MissingCan_Px[NumNeutralCombosSaved]/D");
	Double_t MissingCan_Py_out[NumNeutralCombosSaved]; t_out->Branch("MissingCan_Py",MissingCan_Py_out,"MissingCan_Py[NumNeutralCombosSaved]/D");
	Double_t MissingCan_Pz_out[NumNeutralCombosSaved]; t_out->Branch("MissingCan_Pz",MissingCan_Pz_out,"MissingCan_Pz[NumNeutralCombosSaved]/D");
	Double_t MissingCan_E_out[NumNeutralCombosSaved];  t_out->Branch("MissingCan_E" ,MissingCan_E_out,"MissingCan_E[NumNeutralCombosSaved]/D");
	
	Int_t MissingCan_ISFCAL_out[NumNeutralCombosSaved];  t_out->Branch("MissingCan_ISFCAL" ,MissingCan_ISFCAL_out,"MissingCan_ISFCAL[NumNeutralCombosSaved]/I");
	Double_t threepi_can_mass_out[NumNeutralCombosSaved];  t_out->Branch("threepi_can_mass" ,threepi_can_mass_out,"threepi_can_mass[NumNeutralCombosSaved]/D");
	Double_t twogamma_can_mass_out[NumNeutralCombosSaved];  t_out->Branch("twogamma_can_mass" ,twogamma_can_mass_out,"twogamma_can_mass[NumNeutralCombosSaved]/D");
	
	Int_t NMissingCandidates_out;  t_out->Branch("NMissingCandidates" ,&NMissingCandidates_out);
	Int_t NFCALCandidates_out;  t_out->Branch("NFCALCandidates" ,&NFCALCandidates_out);
	Int_t NBCALCandidates_out;  t_out->Branch("NBCALCandidates" ,&NBCALCandidates_out);
	
	Double_t PiPlus_px_th_out; if(has_thrown_info) t_out->Branch("PiPlus_px_th",&PiPlus_px_th_out);
	Double_t PiPlus_py_th_out; if(has_thrown_info) t_out->Branch("PiPlus_py_th",&PiPlus_py_th_out);
	Double_t PiPlus_pz_th_out; if(has_thrown_info) t_out->Branch("PiPlus_pz_th",&PiPlus_pz_th_out);
	Double_t PiPlus_E_th_out;  if(has_thrown_info) t_out->Branch("PiPlus_E_th",&  PiPlus_E_th_out);
	Double_t PiMinus_px_th_out; if(has_thrown_info) t_out->Branch("PiMinus_px_th",&PiMinus_px_th_out);
	Double_t PiMinus_py_th_out; if(has_thrown_info) t_out->Branch("PiMinus_py_th",&PiMinus_py_th_out);
	Double_t PiMinus_pz_th_out; if(has_thrown_info) t_out->Branch("PiMinus_pz_th",&PiMinus_pz_th_out);
	Double_t PiMinus_E_th_out;  if(has_thrown_info) t_out->Branch("PiMinus_E_th",&  PiMinus_E_th_out);
	Double_t Proton_px_th_out; if(has_thrown_info) t_out->Branch("Proton_px_th",&Proton_px_th_out);
	Double_t Proton_py_th_out; if(has_thrown_info) t_out->Branch("Proton_py_th",&Proton_py_th_out);
	Double_t Proton_pz_th_out; if(has_thrown_info) t_out->Branch("Proton_pz_th",&Proton_pz_th_out);
	Double_t Proton_E_th_out;  if(has_thrown_info) t_out->Branch("Proton_E_th",&  Proton_E_th_out);
	Double_t Gamma1_px_th_out; if(has_thrown_info) t_out->Branch("Gamma1_px_th",&Gamma1_px_th_out);
	Double_t Gamma1_py_th_out; if(has_thrown_info) t_out->Branch("Gamma1_py_th",&Gamma1_py_th_out);
	Double_t Gamma1_pz_th_out; if(has_thrown_info) t_out->Branch("Gamma1_pz_th",&Gamma1_pz_th_out);
	Double_t Gamma1_E_th_out;  if(has_thrown_info) t_out->Branch("Gamma1_E_th",&  Gamma1_E_th_out);
	Double_t Gamma2_px_th_out; if(has_thrown_info) t_out->Branch("Gamma2_px_th",&Gamma2_px_th_out);
	Double_t Gamma2_py_th_out; if(has_thrown_info) t_out->Branch("Gamma2_py_th",&Gamma2_py_th_out);
	Double_t Gamma2_pz_th_out; if(has_thrown_info) t_out->Branch("Gamma2_pz_th",&Gamma2_pz_th_out);
	Double_t Gamma2_E_th_out;  if(has_thrown_info) t_out->Branch("Gamma2_E_th",&  Gamma2_E_th_out);

	Double_t Gamma1_FOM_out;  if(has_thrown_info) t_out->Branch("Gamma1_FOM",&  Gamma1_FOM_out);
	Double_t Gamma2_FOM_out;  if(has_thrown_info) t_out->Branch("Gamma2_FOM",&  Gamma2_FOM_out);
	Double_t two_gamma_opangle_thr_out;  if(has_thrown_info) t_out->Branch("two_gamma_opangle_thr",&  two_gamma_opangle_thr_out);
	Double_t gam2_ThrRecon_opangle_out;  if(has_thrown_info) t_out->Branch("gam2_ThrRecon_opangle",&  gam2_ThrRecon_opangle_out);
	
	
	Long64_t nentries = t_in->GetEntries();

	cout << "There are " << nentries << " to get through" << endl;
	
	for(size_t i =0; i<nentries; ++i) {
		t_in->GetEntry(i);
		if(i%100000==0) cout << "Entry: "  << i << endl;
		// cout << "Entry: "  << i << endl;
		if(i>=max_events) {
			cout << "Max events reached. Exiting..." << endl;
			break;			
		}
		
		
		
		RecM_Proton_out = RecM_Proton;
		RecM_Proton_pkf_out = RecM_Proton_pkf;
		threepi_mass_out = threepi_mass;
		threepi_mass_pkf_out = threepi_mass_pkf;
		twogamma_mass_out = twogamma_mass;
		twogamma_mass_pkf_out = twogamma_mass_pkf;
		
		if( (twogamma_mass_pkf<pi0_MMcut_lo || twogamma_mass_pkf>pi0_MMcut_hi) && cut_pi0MM) continue;
		
		PiPlus_px_out = PiPlus_px;
		PiPlus_py_out = PiPlus_py;
		PiPlus_pz_out = PiPlus_pz;
		PiPlus_E_out  = PiPlus_E;
		PiMinus_px_out = PiMinus_px;
		PiMinus_py_out = PiMinus_py;
		PiMinus_pz_out = PiMinus_pz;
		PiMinus_E_out  = PiMinus_E;
		Proton_px_out = Proton_px;
		Proton_py_out = Proton_py;
		Proton_pz_out = Proton_pz;
		Proton_E_out  = Proton_E;
		Gamma1_px_out = Gamma1_px;
		Gamma1_py_out = Gamma1_py;
		Gamma1_pz_out = Gamma1_pz;
		Gamma1_E_out  = Gamma1_E;
		Gamma2_px_out = Gamma2_px;
		Gamma2_py_out = Gamma2_py;
		Gamma2_pz_out = Gamma2_pz;
		Gamma2_E_out  = Gamma2_E;
		
		MM2_out  = MM2;
		
		PiPlus_PIDhit_out = PiPlus_PIDhit;
		PiPlus_TrackingChi2_ndf_out = PiPlus_TrackingChi2_ndf;
		PiPlus_Tracking_ndf_out = PiPlus_Tracking_ndf;
		PiPlus_TimingChi2_ndf_out = PiPlus_TimingChi2_ndf;
		PiPlus_dEdxChi2_ndf_out = PiPlus_dEdxChi2_ndf;
		PiPlus_beta_meas_out = PiPlus_beta_meas;
		PiMinus_PIDhit_out = PiMinus_PIDhit;
		PiMinus_TrackingChi2_ndf_out = PiMinus_TrackingChi2_ndf;
		PiMinus_Tracking_ndf_out = PiMinus_Tracking_ndf;
		PiMinus_TimingChi2_ndf_out = PiMinus_TimingChi2_ndf;
		PiMinus_dEdxChi2_ndf_out = PiMinus_dEdxChi2_ndf;
		PiMinus_beta_meas_out = PiMinus_beta_meas;
		Proton_PIDhit_out = Proton_PIDhit;
		Proton_TrackingChi2_ndf_out = Proton_TrackingChi2_ndf;
		Proton_Tracking_ndf_out = Proton_Tracking_ndf;
		Proton_TimingChi2_ndf_out = Proton_TimingChi2_ndf;
		Proton_dEdxChi2_ndf_out = Proton_dEdxChi2_ndf;
		Proton_beta_meas_out = Proton_beta_meas;

		rf_deltaT_out  = rf_deltaT;
		beamE_out  = beamE;
		vertex_z_out  = vertex_z;
		vertex_r_out  = vertex_r;
		NExtraTracks_out  = NExtraTracks;
		ExtraShowerE_out  = ExtraShowerE;

		kinfit_CL_out  = kinfit_CL;
		kinfit_chi2_ndf_out  = kinfit_chi2_ndf;

		Run_out  = Run;
		Event_out  = Event;
		
		NMissingCandidates_out  = NMissingCandidates;
		NFCALCandidates_out  = NFCALCandidates;
		NBCALCandidates_out  = NBCALCandidates;
		
		// cout << "recm proton: " << RecM_Proton << endl;
		
		// cout << "run: " << Run << endl;
		// cout << "event: " << Event << endl;
		
		Bool_t save_event = true;
		if(cut_2D_tail) save_event = false;
		
		for (size_t j =0; j<NumNeutralCombosSaved; ++j) {
			
			// if(Event==5607) {
			// cout << "MissingCan_Px " << MissingCan_Px[j] << endl;
			// cout << "MissingCan_Py " << MissingCan_Py[j] << endl;
			// cout << "MissingCan_Pz " << MissingCan_Pz[j] << endl;
			// cout << "MissingCan_E "  << MissingCan_E[j]  << endl;
				// cout << "Shower mass: " << TLorentzVector(MissingCan_Px[j],MissingCan_Py[j],MissingCan_Pz[j],MissingCan_E[j]).M() << endl;
				// cout << endl;
			// }
			
			MissingCan_Px_out[j] = MissingCan_Px[j];
			MissingCan_Py_out[j] = MissingCan_Py[j];
			MissingCan_Pz_out[j] = MissingCan_Pz[j];
			MissingCan_E_out[j]  = MissingCan_E[j];
			
			MissingCan_ISFCAL_out[j]  = MissingCan_ISFCAL[j];
			threepi_can_mass_out[j]  = threepi_can_mass[j];
			twogamma_can_mass_out[j]  = twogamma_can_mass[j];
			
			
			if(cut_2D_tail) {
				if(threepi_can_mass[j]>0.76&&threepi_can_mass[j]<0.81&&RecM_Proton>0.85) save_event = true;
			}	
			
		}
		
		// if(Gamma2_E<0.5) continue;
		
		
		if(cut_2D_tail&&!save_event) continue;
		
		PiPlus_px_th_out = PiPlus_px_th;
		PiPlus_py_th_out = PiPlus_py_th;
		PiPlus_pz_th_out = PiPlus_pz_th;
		PiPlus_E_th_out = PiPlus_E_th;
		PiMinus_px_th_out = PiMinus_px_th;
		PiMinus_py_th_out = PiMinus_py_th;
		PiMinus_pz_th_out = PiMinus_pz_th;
		PiMinus_E_th_out = PiMinus_E_th;
		Proton_px_th_out = Proton_px_th;
		Proton_py_th_out = Proton_py_th;
		Proton_pz_th_out = Proton_pz_th;
		Proton_E_th_out = Proton_E_th;
		Gamma1_px_th_out = Gamma1_px_th;
		Gamma1_py_th_out = Gamma1_py_th;
		Gamma1_pz_th_out = Gamma1_pz_th;
		Gamma1_E_th_out = Gamma1_E_th;
		Gamma2_px_th_out = Gamma2_px_th;
		Gamma2_py_th_out = Gamma2_py_th;
		Gamma2_pz_th_out = Gamma2_pz_th;
		Gamma2_E_th_out = Gamma2_E_th;
		
		Gamma1_FOM_out = Gamma1_FOM;
		Gamma2_FOM_out = Gamma2_FOM;
		two_gamma_opangle_thr_out = two_gamma_opangle_thr;
		gam2_ThrRecon_opangle_out = gam2_ThrRecon_opangle;
		
		t_out->Fill();
	}
	
	f_out->Write("",4); //Option for int 4: write/delete (avoids issue of second ttree showing up)
	f_out->Close();


	cout << "Done" << endl;
	return 0;
	
}
