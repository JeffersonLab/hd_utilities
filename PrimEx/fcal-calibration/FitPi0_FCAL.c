#include <iostream>
#include <fstream>
#include <sstream> 
using namespace std;
#include <TFile.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <TGraph2D.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TStyle.h>
#include <TText.h>
#include <TCanvas.h>
//#include <TF2.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TLegend.h>
#include <TFitResult.h>
#include <TRatioPlot.h>
#include <TProfile2D.h>
#include <TLatex.h>
#include <TLine.h>
#include <TGaxis.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TLorentzRotation.h"
#include <TGenPhaseSpace.h>

#include "/work/halld/home/gxproj2/calibration/ilib/Nicer2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nicer1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice3D2D.c"
/*
#include "/home/igal/root/ilib/Nicer2D.c"
#include "/home/igal/root/ilib/Nicer1D.c"
#include "/home/igal/root/ilib/Nice1D.c"
#include "/home/igal/root/ilib/Nice3D2D.c"
*/
//#define ITERATION 7


void FitPi0_FCAL(int ITERATION = 0, TString path = "") {
  //Begin by Defining iteration number.
  //ITERATION is used to get gains from previous iterations
  const int kBlocksWide = 59;
  const int kBlocksTall = 59;
  int kMaxChannels = kBlocksWide * kBlocksTall;
  
  
  // Script used for fitting the output of the FCAL_Pi0TOF plugin
  // Invoke using root -l -b <rootfile> FitGains.C
  // Force Batch Mode
  //gROOT->SetBatch();
  
  gROOT->Reset();
  
  gROOT->SetStyle("Bold");
  gStyle->SetCanvasColor(0);
  gStyle->SetLabelColor(1);
  gStyle->SetLabelColor(1,"Y");
  gStyle->SetHistLineColor(1); 
  gStyle->SetHistLineWidth(1); 
  gStyle->SetNdivisions(505);
  gStyle->SetNdivisions(505,"Y");
  //gROOT->Macro("setcolor2.c");
  //gStyle->SetHistFillColor(999);
  gROOT->SetStyle("Plain");  // white as bg
  gStyle->SetOptStat("111111");
  gStyle->SetFrameLineWidth(1);
  gStyle->SetTitleFontSize(0.1);
  gStyle->SetTitleBorderSize(0);
    
  //Definitions
  Double_t smallBetween1 = .1;
  Double_t smallBetween2 = .1;
  Double_t smallBetween3 = .1;
  Double_t smallBetween4 = .1;
  
  Double_t small = .00001;
  
  TLine TLine;
  //TLine * fLine;
  TLatex *t = new TLatex();
  t->SetTextSize(0.05);
  t->SetTextFont(42);
  t->SetTextAlign(12);
  t->SetTextColor(1);
  
  TCanvas *C1;
  
  TPad *SmallC;
  TGaxis *XAxis,*YAxis;
  TLatex XTitle,YTitle,XtopTitle;
  
  TLegend *legend;

  ifstream in;
  ofstream out;
  TString Line = ""; 
  TString cleg = "";
  
  TFile * ifile = new TFile(path + "runpi0.root"); 

  // Define histograms
  TProfile  *hCurrentGainConstants = (TProfile *) gDirectory->Get("FCAL_Pi0TOF/CurrentGainConstants");
  TH1I * hPi0Mass = (TH1I *) gDirectory->Get("FCAL_Pi0log/Pi0Mass");
  TH2F * hPi0MassVsChNum = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNum");
  TH2F * hPi0MassVsChNumWeighted = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNumWeighted");
  TH2F * hPi0MassVsChNumWeightedSquared = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNumWeightedSquared_Fid");

  TH2F * hPi0MassVsChNum_frac = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNum");
  TH2F * hPi0MassVsChNumWeighted_frac = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNumWeighted");
  TH2F * hPi0MassVsChNumWeightedSquared_frac = (TH2F *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsChNumWeightedSquared_Fid");

  TH2I * hPi0MassVsE = (TH2I *) gDirectory->Get("FCAL_Pi0log/Pi0MassVsE");
  
  TFile * outFile = new TFile(path + Form("FCALPi0FitResults_%d.root", ITERATION),"RECREATE");

  // Make an output file

  TH1F * hFitPi0Mass = new TH1F("FitPi0Mass", "#pi^{0} Mass fit result", 2800, -0.5, 2799.5);
  TH1F * hFitGain = new TH1F("FitGain", "new gain result", 2800, -0.5, 2799.5);
  TH1F * h_gg_im[2800];
  TH1F * h_gg_im_w[2800];
  TH1F * h_gg_im_wsq[2800];
  TH1F * h_gg_im_f[2800];
  TH1F * h_gg_im_w_f[2800];
  TH1F * h_gg_im_wsq_f[2800];

  // 2D histograms of FCAL gains
  TH2F * XYGains = new TH2F("XYGains", "", 59, -30, 30, 59, -30, 30);
  TProfile2D * XYGainsb = new TProfile2D("XYGainsb", " Gains", 59, -30, 30, 59, -30, 30, -150, 150);
  TH2F * XYMean = new TH2F("XYMean", "", 59, -30, 30, 59, -30, 30);
  TH2F * XYWidth = new TH2F("XYWidth", "", 59, -30, 30, 59, -30, 30);
  TH1F * h_Chi2 = new TH1F("Chi2", ";#font[42]{#chi^{2}/ndf};#font[42]{Count [a.u.]}", 100, 0., 20.);
  TH1F * h_width = new TH1F("width", ";#font[42]{#sigma [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 100, 5., 25.);
  TH1F * h_mean = new TH1F("mean", ";#font[42]{#mu [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 300, 50., 200.);
  TH1F * h_width_err = new TH1F("width_err", ";#font[42]{Error on #sigma [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 100, 0., 2.);
  TH1F * h_mean_err = new TH1F("mean_err", ";#font[42]{Error on #mu [MeV/#it{c}^{2}]};#font[42]{Count [a.u.]}", 100, 0., 2.);
  TH1F * h_gain = new TH1F("gain", ";#font[42]{Gain};#font[42]{Count [a.u.]}", 100, 0., 2.);
  
  TH2F * XYGeo[24];
  for (int i = 0; i < 24; i ++)
    XYGeo[i] = new TH2F(Form("XYGeo_%d", i),"", 59, -30, 30, 59, -30, 30);
  
  // TGraph
  TGraphErrors * grNewGain = new TGraphErrors();
  TGraphErrors * grchiSquare = new TGraphErrors();
  TGraphErrors * grWidth = new TGraphErrors();
  TGraphErrors * grGains[100];
  TGraphErrors * grGains_vs_tc[2800];
  TGraphErrors * grFitPars[2];
  
  double fitPi0Mean[2800], fitMax[2800];
  double fitPi0Mean_err[2800], fitMax_err[2800];
  double NormalizedChi2[2800];
  double maxval[2800];
  double maxval_zoom[2800];
  double width[2800];
  double width_err[2800];
  double mean[2800];
  double mean_err[2800];
  double pdgPi0Mass = 0.1349766;
  double NewGains[2800];
  double NewGainsChi2[2800];
  double OldGains[2800];
  double NewGains_err[2800];
  double fPi0Pos;
  
  // Fit function
  TF1 * fFitFunc;
  TF1 * fgau;
  TF1 * fsig;
  TF1 * fbkg;
  TF1 * bkg_eta_fc;
  TF1 * sig_eta_fc;
  TF1 * tot_eta_fc;
  TF1 * bkg_pi0_fc;
  TF1 * sig_pi0_fc;
  TF1 * tot_pi0_fc;
    
  TFitResultPtr r;
  TFitResultPtr r1;
  TFitResultPtr r2;
  
  ostringstream fname;
  double parfsig[2800][3], chiSquare[2800];
  int countLow = 0;
  int ctr = 0;
  //int old_ch[2800];
  
  int sq_vs_ch[2800];
  vector <int> square_tab[24];
  vector <int> bad_det_tab;
  vector <int> bad_rg_tab;

  // Initialization
  for (int i = 0; i < 2800; i ++) {
    //old_ch[i] = 0;
    sq_vs_ch[i] = -1;
    fitMax[i] = 0;
    NewGains[i] = 1;
    OldGains[i] = 1;
    NewGains_err[i] = 1;
    mean[i] = 0;
    width[i] = 0;
    mean_err[i] = 0;
    width_err[i] = 0;
  }
  
  for (int i = 0; i < 24; i ++) {
    //TString ring_file = Form("/work/halld/home/gxproj2/calibration/fcal/squares/FCALinnersquare%d.txt", i + 1);
    TString ring_file = Form("rings/FCAL_rings_%d.txt", i + 1);
    in.open(ring_file);
    int k = 0;
    while ( in.good() ) {
      int ch = -1;
      in>>ch;
      if (ch >= 0) {
	sq_vs_ch[ch] = i;
	square_tab[i].push_back(ch);
	//old_ch[ch] ++;
	ctr ++;
	k ++;
      }
    }
    in.close();
    cout <<"ring " << i << " #ch " << k << endl;
  }
  cout <<"Total ch # "<< ctr << " truth " << hPi0MassVsChNum->GetNbinsX() << endl;
  
  int max_ctr = 50;

  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  int good_ch_ctr = 0;
  for (int i = 0; i < 24; i ++) {
    
    int canvas_nb = square_tab[i].size() / 16;
    
    for (int j = 0; j <= canvas_nb; j ++) {
      
      cleg = path + Form("aa_im_weighted_square_%d_canvas_nb_%d", i, j);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      C1->Divide(4,4);
      
      for (int k = 0; k < 16; k ++) {
	
	int size = k + j * 16;
	
	if (size < (int) square_tab[i].size()) {
	  
	  int l = square_tab[i][size];
	  
	  OldGains[l] = hCurrentGainConstants->GetBinContent(l + 1);
	  
	  //histogramming
	  h_gg_im[l] = (TH1F *) hPi0MassVsChNum->ProjectionY(Form("h_gg_im_%d", l), l + 1, l + 1);
	  h_gg_im_w[l] = (TH1F *) hPi0MassVsChNumWeighted->ProjectionY(Form("h_gg_im_w_%d", l), l + 1, l + 1);
	  h_gg_im_wsq[l] = (TH1F *) hPi0MassVsChNumWeightedSquared->ProjectionY(Form("h_gg_im_wsq_%d", l), l + 1, l + 1);
	  h_gg_im_f[l] = (TH1F *) hPi0MassVsChNum_frac->ProjectionY(Form("h_gg_im_f_%d", l), l + 1, l + 1);
	  h_gg_im_w_f[l] = (TH1F *) hPi0MassVsChNumWeighted_frac->ProjectionY(Form("h_gg_im_w_f_%d", l), l + 1, l + 1);
	  h_gg_im_wsq_f[l] = (TH1F *) hPi0MassVsChNumWeightedSquared_frac->ProjectionY(Form("h_gg_im_wsq_f_%d", l), l + 1, l + 1);
	  //h_gg_im_raw[l] = (TH1F *) hPi0MassVsChNum->ProjectionY(Form("h_gg_im_raw_%d", l), l + 1, l + 1);
	  mean[l] = h_gg_im[l]->GetMean();
	  
	  if (path.Contains("simu"))
	    max_ctr > 1;
	  
	  if (h_gg_im[l]->GetEntries() > max_ctr) {
	    Nice1D(h_gg_im[l], 0.05, 0.05, 42, 505, 1.1, 1.3, 
		   "", 
		   "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}", "#font[41]{Counts}");
	    
	    //booking fct
	    fgau = new TF1(Form("fgau_%d", l), "gaus", 0.12, 0.16);
	    fsig = new TF1(Form("fsig_%d", l), "crystalball", 0.12, 0.16);
	    fbkg = new TF1(Form("fbkg_%d", l), "exp(pol5)");
	    if (path.Contains("simu"))
	      fFitFunc = new TF1(Form("fFitFunc_%d", l), "gaus");
	    else
	      fFitFunc = new TF1(Form("fFitFunc_%d", l), "crystalball+exp(pol5(5))");
	    fFitFunc->SetLineColor(2);
	    int fBinPi0Pos = h_gg_im[l]->FindBin(pdgPi0Mass);
	    fPi0Pos = h_gg_im[l]->GetBinCenter(h_gg_im[l]->GetMaximumBin());
	    if (fPi0Pos < 0.100 || fPi0Pos > 0.160) fPi0Pos = 0.135;
	    //double min_mass = fPi0Pos - 0.050;
	    //double max_mass = fPi0Pos + 0.050;
	    double min_mass = 0.08;
	    double max_mass = 0.220;
	    //h_gg_im[l]->GetXaxis()->SetRangeUser(fPi0Pos - 0.050, fPi0Pos + 0.050);
	    if (i < 5) {
	      //h_gg_im[l]->GetXaxis()->SetRangeUser(0.090, 0.180);
	      min_mass = 0.09;
	      max_mass = 0.18;
	    }
	    fPi0Pos = h_gg_im[l]->GetBinCenter(h_gg_im[l]->GetMaximumBin());
	    maxval_zoom[l] = fPi0Pos;
	    if (fPi0Pos < 0.100 || fPi0Pos > 0.160) fPi0Pos = 0.135;
	    double mass_low = h_gg_im[l]->GetXaxis()->GetBinCenter(1);
	    if (mass_low < 0.01) mass_low = 0.02;
	    if (i > 21) {//h_gg_im[l]->GetXaxis()->SetRangeUser(mass_low, 0.20);
	      min_mass = mass_low;
	      max_mass = 0.2;
	    }
	    h_gg_im[l]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	    h_gg_im_w[l]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	    h_gg_im_wsq[l]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	    h_gg_im_f[l]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	    h_gg_im_w_f[l]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	    h_gg_im_wsq_f[l]->GetXaxis()->SetRangeUser(min_mass, max_mass);
	    
	    fPi0Pos = h_gg_im[l]->GetBinCenter(h_gg_im[l]->GetMaximumBin());
	    maxval[l] = fPi0Pos;
	    Double_t par_tot_fct[13];
	    Double_t height = h_gg_im[l]->GetBinCenter(fBinPi0Pos);
	    if (i > 21)
	      height = h_gg_im[l]->GetMaximum() / 2;
	    //Double_t par_sig_fct[5] = {height, fPi0Pos, 0.01, 0.1, 1.0}; 
	    Double_t par_sig_fct[5] = {height, fPi0Pos, 0.01, 4000, 1000}; 
	    Double_t par_bkg_fct[8] = {1., 1., 1., 1., 1., 1.};
	    
	    fFitFunc->SetRange(min_mass, max_mass);
	    fbkg->SetRange(min_mass, max_mass);
	    fgau->SetRange(min_mass, max_mass);
	    
	    fsig->SetParameters(par_sig_fct);
	    h_gg_im[l]->Fit(fsig, "RBQ0");
	    fsig->GetParameters(&par_tot_fct[0]);
	    h_gg_im[l]->Fit(fbkg, "RBQ0");
	    fbkg->GetParameters(&par_tot_fct[5]);
	    fFitFunc->SetParameters(par_tot_fct);
	    fFitFunc->SetParLimits(0, h_gg_im[l]->GetMaximum() * 0.05, h_gg_im[l]->GetMaximum() * 1.2);
	    if (i < 22) {
	      fFitFunc->SetParLimits(1, 0.12, 0.15);
	      fFitFunc->SetParLimits(2, 0.005, 0.020);
	    } else {
	      fFitFunc->SetParLimits(1, mass_low, 0.15);
	      fFitFunc->SetParLimits(2, 0.005, 0.10);
	    }
	    h_gg_im[l]->Fit(fFitFunc, "RMBQ0");
	    fFitFunc->SetParameters(par_tot_fct);
	    r = h_gg_im[l]->Fit(fFitFunc, "RMBQS0");
	    double Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) { 
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, 0, h_gg_im[l]->GetMaximum() * 1.2);
	      r = h_gg_im[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) { 
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_w[l]->GetMaximum() * 1.2);
	      r = h_gg_im_w[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) { 
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_wsq[l]->GetMaximum() * 1.2);
	      r = h_gg_im_wsq[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) { 
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_f[l]->GetMaximum() * 1.2);
	      r = h_gg_im_f[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) { 
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_w_f[l]->GetMaximum() * 1.2);
	      r = h_gg_im_w_f[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) { 
	      fFitFunc->SetParameters(par_tot_fct);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_wsq_f[l]->GetMaximum() * 1.2);
	      r = h_gg_im_wsq_f[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) {
	      fFitFunc = new TF1(Form("fFitFunc_ReFit1_%d", l), "gaus+pol3(3)",0.1,0.18);
	      fFitFunc->SetParameters(h_gg_im[l]->GetMaximum() * 1.2, 0.135, 0.008, 1, 1, 1, 1);
	      fFitFunc->SetLineColor(3);
	      fFitFunc->SetParLimits(0, 0, h_gg_im[l]->GetMaximum() * 1.2);
	      fFitFunc->SetParLimits(1, 0.12, 0.14);
	      fFitFunc->SetParLimits(2, 0.001, 0.015);
	      r = h_gg_im[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) {
	      fFitFunc = new TF1(Form("fFitFunc_ReFit2_%d", l), "gaus+pol3(3)",0.1,0.18);
	      fFitFunc->SetParameters(h_gg_im_w[l]->GetMaximum() * 1.2, 0.135, 0.008, 1, 1, 1, 1);
	      fFitFunc->SetLineColor(3);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_w[l]->GetMaximum() * 1.2);
	      fFitFunc->SetParLimits(1, 0.12, 0.14);
	      fFitFunc->SetParLimits(2, 0.001, 0.015);
	      r = h_gg_im_w[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) {
	      fFitFunc = new TF1(Form("fFitFunc_ReFit3_%d", l), "gaus+pol3(3)",0.1,0.18);
	      fFitFunc->SetParameters(h_gg_im_wsq[l]->GetMaximum() * 1.2, 0.135, 0.008, 1, 1, 1, 1);
	      fFitFunc->SetLineColor(3);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_wsq[l]->GetMaximum() * 1.2);
	      fFitFunc->SetParLimits(1, 0.12, 0.14);
	      fFitFunc->SetParLimits(2, 0.001, 0.015);
	      r = h_gg_im_wsq[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) {
	      fFitFunc = new TF1(Form("fFitFunc_ReFit4_%d", l), "gaus",0.12,0.15);
	      fFitFunc->SetLineColor(6);
	      fFitFunc->SetParLimits(0, 0, h_gg_im[l]->GetMaximum() * 1.2);
	      fFitFunc->SetParLimits(1, 0.12, 0.14);
	      fFitFunc->SetParLimits(2, 0.001, 0.015);
	      r = h_gg_im[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) {
	      fFitFunc = new TF1(Form("fFitFunc_ReFit5_%d", l), "gaus",0.12,0.15);
	      fFitFunc->SetLineColor(6);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_w[l]->GetMaximum() * 1.2);
	      fFitFunc->SetParLimits(1, 0.12, 0.14);
	      fFitFunc->SetParLimits(2, 0.001, 0.015);
	      r = h_gg_im_w[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    if (Chi2 > 8.0) {
	      fFitFunc = new TF1(Form("fFitFunc_ReFit6_%d", l), "gaus",0.12,0.15);
	      fFitFunc->SetLineColor(6);
	      fFitFunc->SetParLimits(0, 0, h_gg_im_wsq[l]->GetMaximum() * 1.2);
	      fFitFunc->SetParLimits(1, 0.12, 0.14);
	      fFitFunc->SetParLimits(2, 0.001, 0.015);
	      r = h_gg_im_wsq[l]->Fit(fFitFunc, "RMBQS0");
	    }
	    Chi2 = r->Chi2() / (double) r->Ndf();
	    NormalizedChi2[l] = Chi2;
	    width[l] = r->Parameter(2);
	    fitPi0Mean[l]  = r->Parameter(1);
	    width_err[l] = r->ParError(2);
	    fitPi0Mean_err[l]  = r->ParError(1);
	    h_Chi2->Fill(Chi2);
	    h_mean->Fill(r->Parameter(1) * 1e3);
	    h_width->Fill(r->Parameter(2) * 1e3);
	    h_mean_err->Fill(r->ParError(1) * 1e3);
	    h_width_err->Fill(r->ParError(2) * 1e3);
	    //if (Chi2 < 10.0)
	    if (Chi2 < 8.0) {
	      NewGains[l] = hCurrentGainConstants->GetBinContent(l + 1) * (pdgPi0Mass / fitPi0Mean[l]); //calculate a new gain
	      fitMax[l] = fitPi0Mean[l];
	    } else { 
	      NewGains[l] = hCurrentGainConstants->GetBinContent(l + 1) * (pdgPi0Mass / maxval_zoom[l]);
	      fitMax[l] = maxval_zoom[l];
	    }
	    //if (i > 24 && Chi2 < 10.0)
	    if (i > 21 && Chi2 < 8.0) {
	      NewGains[l] = hCurrentGainConstants->GetBinContent(l + 1) * (0.101 / fitPi0Mean[l]);
	      fitMax[l] = 0.101;
	    } else if (i > 21 && Chi2 >= 8.0) {
	      NewGains[l] = hCurrentGainConstants->GetBinContent(l + 1) * (0.101 / maxval[l]);
	      fitMax[l] = 0.101;
	    }
	    NewGainsChi2[l] = Chi2;
	    if (NewGains[l] > 2.0) NewGains[l] = 1;
	    double QA = NewGains[l] / OldGains[l];
	    //if (0.99 <= QA && QA <= 1.01) NewGains[l] = OldGains[l];
	    h_gain->Fill(NewGains[l]);
	    // final results
	    C1->cd(k + 1);
	    gPad->SetLeftMargin(smallBetween1);
	    gPad->SetRightMargin(smallBetween2);
	    gPad->SetTopMargin(smallBetween3);
	    gPad->SetBottomMargin(smallBetween4);
	    
	    h_gg_im_wsq[l]->SetMarkerStyle(20);
	    h_gg_im_wsq[l]->SetMarkerSize(0.5);
	    h_gg_im_wsq[l]->SetMarkerColor(1);
	    h_gg_im_wsq[l]->SetLineColor(1);
	    h_gg_im_w[l]->SetMarkerStyle(21);
	    h_gg_im_w[l]->SetMarkerSize(0.5);
	    h_gg_im_w[l]->SetMarkerColor(1);
	    h_gg_im_w[l]->SetLineColor(1);
	    h_gg_im[l]->SetMarkerStyle(22);
	    h_gg_im[l]->SetMarkerSize(0.5);
	    h_gg_im[l]->SetMarkerColor(1);
	    h_gg_im[l]->SetLineColor(1);
	    h_gg_im_wsq_f[l]->SetMarkerStyle(24);
	    h_gg_im_wsq_f[l]->SetMarkerSize(0.5);
	    h_gg_im_wsq_f[l]->SetMarkerColor(1);
	    h_gg_im_wsq_f[l]->SetLineColor(3);
	    h_gg_im_w_f[l]->SetMarkerStyle(25);
	    h_gg_im_w_f[l]->SetMarkerSize(0.5);
	    h_gg_im_w_f[l]->SetMarkerColor(3);
	    h_gg_im_w_f[l]->SetLineColor(3);
	    h_gg_im_f[l]->SetMarkerStyle(25);
	    h_gg_im_f[l]->SetMarkerSize(0.5);
	    h_gg_im_f[l]->SetMarkerColor(3);
	    h_gg_im_f[l]->SetLineColor(3);
	    
	    h_gg_im[l]->SetMinimum(0);
	    h_gg_im[l]->Draw();
	    h_gg_im_w[l]->Draw("psame");
	    h_gg_im_wsq[l]->Draw("psame");
            h_gg_im_f[l]->Draw("psame");
	    h_gg_im_w_f[l]->Draw("psame");
	    h_gg_im_wsq_f[l]->Draw("psame");
	    
	    // set indicator line
	    TLine.SetLineWidth(2);
	    TLine.SetLineStyle(2);
	    TLine.SetLineColor(4);
	    TLine.DrawLine(fPi0Pos, 0, fPi0Pos, h_gg_im[l]->GetMaximum() + 20);
	    
	    // set indicator line
	    fPi0Pos = fFitFunc->GetParameter(1); 
	    TLine.SetLineColor(3);
	    TLine.DrawLine(fPi0Pos, 0, fPi0Pos, h_gg_im[l]->GetMaximum() + 20);
	    
	    // check if mass is in normal range
	    if (fPi0Pos < 0.80 || fPi0Pos > 0.200) fPi0Pos = 0.135;
	    // set indicator line
	    TLine.SetLineColor(2);
	    TLine.DrawLine(fPi0Pos, 0, fPi0Pos, h_gg_im[l]->GetMaximum() + 20);
	    if (i > 24) {
	      TLine.SetLineColor(6);
	      TLine.DrawLine(maxval[l], 0, maxval[l], h_gg_im[l]->GetMaximum() + 20);
	    }
	    // draw fitting function
	    if (fFitFunc) {
	      //if (Chi2 >= 10.0) {
	      if (Chi2 >= 8.0) {
		fFitFunc->SetLineColor(4);
		bad_det_tab.push_back(l);
		bad_rg_tab.push_back(i);
	      } else {
		good_ch_ctr ++;
	      }
	      fFitFunc->Draw("same");
	    }
	    t->DrawLatex(0.15, ((double) h_gg_im[l]->GetMaximum()), Form("#font[42]{# %d, ring # %d}", l, i));
	  } else {
	    bad_det_tab.push_back(l);
	    bad_rg_tab.push_back(i);
	    NewGainsChi2[l] = -1;
	  }
	}
      }
      C1->Print(cleg + ".pdf");
    }
  }
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Chi2_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_Chi2, 0.05, 42, 504, 1.1, 1.5);
  h_Chi2->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_width, 0.05, 42, 504, 1.1, 1.5);
  h_width->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Mean_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_mean, 0.05, 42, 504, 1.1, 1.5);
  h_mean->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_err_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_width_err, 0.05, 42, 504, 1.1, 1.5);
  h_width_err->Draw();
  
  C1->Print(cleg + ".pdf");

  cleg = path + "Mean_err_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_mean_err, 0.05, 42, 504, 1.1, 1.5);
  h_mean_err->Draw();
  
  C1->Print(cleg + ".pdf");
    
  TF1 * fsigGains = new TF1("fsigGains", "gaus", 0.5, 1.5);
  fsigGains->SetLineColor(2);
  h_gain->Fit(fsigGains,"R");

  cleg = path + "Gain_distribution";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);  
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Nicer1D(h_gain, 0.05, 42, 504, 1.1, 1.5);
  h_gain->Draw();
  
  C1->Print(cleg + ".pdf");

  cout << "Number of bad fit " << bad_det_tab.size() << " number of good fit " << good_ch_ctr << endl;
  int canvas_nb = bad_det_tab.size() / 16;

  for (int j = 0; j <= canvas_nb; j ++) {
    
    cleg = path + Form("CHECK_aa_im_weighted_canvas_nb_%d", j);
    C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
    C1->Divide(4,4);

    for (int k = 0; k < 16; k ++) {
      
      int size = k + j * 16;
    
      if (size < (int) bad_det_tab.size()) {
      
	int l = bad_det_tab[size];
	int rg = bad_rg_tab[size];
	C1->cd(k + 1);
	gPad->SetLeftMargin(smallBetween1);
	gPad->SetRightMargin(smallBetween2);
	gPad->SetTopMargin(smallBetween3);
	gPad->SetBottomMargin(smallBetween4);

	h_gg_im[l]->SetMaximum(99.99);
	h_gg_im[l]->SetMinimum(0.0);
	h_gg_im[l]->Draw();
	h_gg_im_w[l]->Draw("psame");
	h_gg_im_wsq[l]->Draw("psame");
	h_gg_im_f[l]->Draw("psame");
	h_gg_im_w_f[l]->Draw("psame");
	h_gg_im_wsq_f[l]->Draw("psame");

	double max = 50;
	// set indicator line
	TLine.SetLineColor(2);
	TLine.DrawLine(fitMax[l], 0, fitMax[l], max);
	fPi0Pos = 0.135;
	TLine.SetLineColor(4);
	TLine.DrawLine(fPi0Pos, 0, fPi0Pos, max);
	// write numbering
	t->DrawLatex(0.135, max, Form("#font[42]{# %d, ring # %d}", l, rg));
      }
      
    }
    
    C1->Print(cleg + ".pdf");

  }

  const int sq_nb = 25;
  TH1F * h1D[sq_nb];
  TGraphErrors * grFitPars_rg_eta[2];
  TGraphErrors * grFitPars_rg_pi0[2];
  
  for (int i = 0; i < 2; i ++) {
    grFitPars_rg_eta[i] = new TGraphErrors();
    grFitPars_rg_pi0[i] = new TGraphErrors();
    grFitPars_rg_eta[i]->SetName(Form("grFitPars_rg_eta_%d", i));
    grFitPars_rg_pi0[i]->SetName(Form("grFitPars_rg_pi0_%d", i));
  }
  
  for (int i = 0; i < sq_nb; i ++) {
    if (i != 0) {
      TString histo = Form("FCAL_Pi0log/Pi0Mass_ring_%d", i);
      if (i > 21)
	histo = Form("FCAL_Pi0log/Pi0Mass2g_ring_%d", i);
      cout << histo << endl;
      h1D[i] = (TH1F *) ifile->Get(histo);
      if (i > 21)
	h1D[i]->Rebin(5);
      
      h1D[i]->SetMarkerStyle(20);
      h1D[i]->SetMarkerColor(1);
      h1D[i]->SetLineColor(1);
      h1D[i]->SetMarkerSize(1.0);
      Nice1D(h1D[i], 0.05, 0.05, 42,505,1.,1.8,"","#font[42]{#it{m}_{#gamma#gamma} [GeV/#it{c}^{2}]}","#font[41]{Counts [a.u.]}");

      bkg_eta_fc  = new TF1(Form("bkg_eta_fc_%d", i),"pol3",0.440,0.700);
      sig_eta_fc  = new TF1(Form("sig_eta_fc_%d", i),"gaus",0.500,0.600);
      tot_eta_fc  = new TF1(Form("tot_eta_fc_%d", i),"gaus + pol3(3)",0.440,0.700);
            
      tot_eta_fc->SetLineColor(2);
      tot_eta_fc->SetLineWidth(2);
      bkg_eta_fc->SetLineColor(3);
      bkg_eta_fc->SetLineWidth(1);
      double peak_init = 0.536;
      double width_init = 0.01;
      Double_t par_tot_eta_fc[7], par_sig_eta_fc[3] = {0.,peak_init,width_init}, par_bkg_eta_fc[4] = {1.,1.,1.,1.};
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      sig_eta_fc->SetParameters(par_sig_eta_fc);
      bkg_eta_fc->SetParameters(par_bkg_eta_fc);
      
      h1D[i]->Fit(bkg_eta_fc, "RMBQS0");
      bkg_eta_fc->GetParameters(&par_tot_eta_fc[3]);
      h1D[i]->Fit(sig_eta_fc, "RMBQS0");
      sig_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      tot_eta_fc->SetParLimits(1,0.510,0.550);
      tot_eta_fc->SetParLimits(2,0.001,0.03);
      
      h1D[i]->Fit(tot_eta_fc,"RMBQS0");
      tot_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      bkg_eta_fc->SetParameters(&par_tot_eta_fc[3]);
      /*
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"pol4",0.096,0.18);
      sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol4(5)",0.096,0.18);
      */
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"exp(pol3)",0.09,0.225);
      //bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"pol3",0.1,0.205);
      if (i == 23)
	sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.10,0.13);
      else
	sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + exp(pol3(5))",0.09,0.225);
      //tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol3(5)",0.1,0.205);
      tot_pi0_fc->SetLineColor(2);
      tot_pi0_fc->SetLineWidth(2);
      bkg_pi0_fc->SetLineColor(3);
      bkg_pi0_fc->SetLineWidth(1);
      peak_init = 0.135;
      width_init = 0.008;
      /*
      if (i == 23) {
	peak_init = 0.105;
	width_init = 0.028;
      }
      */
      Double_t par_tot_pi0_fc[11], par_sig_pi0_fc[5] = {0., peak_init, width_init,3.58653e+02,1.53665e+03}, par_bkg_pi0_fc[6] = {1., 1., 1., 1., 1., 1.};
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      sig_pi0_fc->SetParameters(par_sig_pi0_fc);
      bkg_pi0_fc->SetParameters(par_bkg_pi0_fc);
      
      h1D[i]->Fit(bkg_pi0_fc, "RMBQS0");
      bkg_pi0_fc->GetParameters(&par_tot_pi0_fc[5]);
      h1D[i]->Fit(sig_pi0_fc, "RMBQS0");
      sig_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      if (i == 23)
	tot_pi0_fc->SetParLimits(1,0.100,0.140);
      else
	tot_pi0_fc->SetParLimits(1,0.130,0.140);
      if (i == 23)
	tot_pi0_fc->SetParLimits(2,0.005,0.030);
      else
	tot_pi0_fc->SetParLimits(2,0.005,0.015);
      
      h1D[i]->Fit(tot_pi0_fc,"RMBQS0");
      tot_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      bkg_pi0_fc->SetParameters(&par_tot_pi0_fc[5]);

      grFitPars_rg_pi0[0]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(1) * 1e3);
      grFitPars_rg_pi0[0]->SetPointError(i, 0, tot_pi0_fc->GetParError(1) * 1e3);
      grFitPars_rg_eta[0]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(1) * 1e3);
      grFitPars_rg_eta[0]->SetPointError(i, 0, tot_eta_fc->GetParError(1) * 1e3);
      grFitPars_rg_pi0[1]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(2) * 1e3);
      grFitPars_rg_pi0[1]->SetPointError(i, 0, tot_pi0_fc->GetParError(2) * 1e3);
      grFitPars_rg_eta[1]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(2) * 1e3);
      grFitPars_rg_eta[1]->SetPointError(i, 0, tot_eta_fc->GetParError(2) * 1e3);
      
      if (i >= 21) {
	double fittedPi0Mean = tot_pi0_fc->GetParameter(1);
	for (int j = 0; j < (int) square_tab[i].size(); j ++) {
	  NewGains[square_tab[i][j]] = hCurrentGainConstants->GetBinContent(square_tab[i][j] + 1) * (pdgPi0Mass / fittedPi0Mean);
	  if (i >= 22) NewGains[square_tab[i][j]] = 1;
	}
      }
      cleg = Form("im_vs_rg_%d", i);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      gPad->SetLeftMargin(smallBetween1);
      gPad->SetRightMargin(smallBetween2);
      gPad->SetTopMargin(smallBetween3);
      gPad->SetBottomMargin(smallBetween4);
      //if (i == 24) h1D[i]->SetMaximum(1e2);
      //if (i == 24) h1D[i]->SetMaximum(1e4);
      h1D[i]->SetMinimum(1);
      h1D[i]->Draw();

      tot_pi0_fc->Draw("same");
      bkg_pi0_fc->Draw("same");
      tot_eta_fc->Draw("same");
      bkg_eta_fc->Draw("same");

      gPad->SetLogy();
      double max = h1D[i]->GetMaximum();
      t->SetTextSize(0.03);
      t->DrawLatex(0.35, max/1.0, Form("#font[42]{#mu_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(1) * 1e3, tot_pi0_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/2.0, Form("#font[42]{#sigma_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(2) * 1e3, tot_pi0_fc->GetParError(2) * 1e3));
      t->DrawLatex(0.35, max/4.0, Form("#font[42]{#mu_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(1) * 1e3, tot_eta_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/8.0, Form("#font[42]{#sigma_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(2) * 1e3, tot_eta_fc->GetParError(2) * 1e3));
      
      TLine.SetLineColor(2);
      TLine.DrawLine(fPi0Pos, 0, fPi0Pos, h1D[i]->GetMaximum() * 1.2);
      C1->Print(path + cleg + ".pdf");
    }
  }
  
  TH2F * DrawM_rg_pi0 = new TH2F("DrawM_rg_pi0", ";#font[42]{Ring #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 128, 135.99);
  TH2F * DrawW_rg_pi0 = new TH2F("DrawW_rg_pi0", ";#font[42]{Ring #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 7.0, 8.99);
  Nicer2D(DrawM_rg_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_rg_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);

  TH2F * DrawM_rg_eta = new TH2F("DrawM_rg_eta", ";#font[42]{Ring #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 530, 549.99);
  TH2F * DrawW_rg_eta = new TH2F("DrawW_rg_eta", ";#font[42]{Ring #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, 24.99, 100, 10.0, 29.99);
  Nicer2D(DrawM_rg_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_rg_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Mean_vs_ring_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_rg_pi0->Draw();
  grFitPars_rg_pi0[0]->SetMarkerStyle(20);
  grFitPars_rg_pi0[0]->SetMarkerSize(1.2);
  grFitPars_rg_pi0[0]->SetMarkerColor(1);
  grFitPars_rg_pi0[0]->SetLineColor(1);
  grFitPars_rg_pi0[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 134.97, 24.99, 134.97);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_ring_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_rg_pi0->Draw();
  grFitPars_rg_pi0[1]->SetMarkerStyle(20);
  grFitPars_rg_pi0[1]->SetMarkerSize(1.2);
  grFitPars_rg_pi0[1]->SetMarkerColor(1);
  grFitPars_rg_pi0[1]->SetLineColor(1);
  grFitPars_rg_pi0[1]->Draw("PSZ");
  
  C1->Print(cleg + ".pdf");
    
  cleg = path + "Mean_vs_ring_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_rg_eta->Draw();
  grFitPars_rg_eta[0]->SetMarkerStyle(20);
  grFitPars_rg_eta[0]->SetMarkerSize(1.2);
  grFitPars_rg_eta[0]->SetMarkerColor(1);
  grFitPars_rg_eta[0]->SetLineColor(1);
  grFitPars_rg_eta[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 547.43, 24.99, 547.43);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_ring_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_rg_eta->Draw();
  grFitPars_rg_eta[1]->SetMarkerStyle(20);
  grFitPars_rg_eta[1]->SetMarkerSize(1.2);
  grFitPars_rg_eta[1]->SetMarkerColor(1);
  grFitPars_rg_eta[1]->SetLineColor(1);
  grFitPars_rg_eta[1]->Draw("PSZ");
  C1->Print(cleg + ".pdf");

  outFile->cd();
  
  for (int i = 0; i < 2; i ++) {
    grFitPars_rg_eta[i]->Write();
    grFitPars_rg_pi0[i]->Write();
  }
  
  //Define FCAL constants
  double kMidBlock = ( kBlocksWide - 1 ) / 2;
  double kBeamHoleSize = 3;
  
  double blockSize=4.0157;
  double radius=1.20471*100.;
  double blockLength=45.;
  
  // inflate the innner radius by 1% to for "safe" comparison
  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize * sqrt(2.)*1.01;
  
  bool   m_activeBlock[kBlocksTall][kBlocksWide];
  TVector2 m_positionOnFace[kBlocksTall][kBlocksWide];
  double BlockGain[kBlocksTall][kBlocksWide],BlockWidth[kBlocksTall][kBlocksWide],BlockMean[kBlocksTall][kBlocksWide];
  int m_numActiveBlocks = 0;
  int m_numDeadBlocks = 0;
  double BlockGainActive[2800];
  int OuterChannelNum[2800];
  int nOuter = 0;

  double channelNum[2800];
  int m_channelNumber[kBlocksTall][kBlocksWide];
  int m_row[kMaxChannels];
  int m_column[kMaxChannels];

  for (int i = 0; i < 2800; i ++) channelNum[i] = i + 1;

  for (int row = 0; row < kBlocksTall; row ++) {
    
    for (int col = 0; col < kBlocksWide; col ++) {
      
      // transform to beam axis
      m_positionOnFace[row][col] =TVector2(  ( col - kMidBlock ) * blockSize,( row - kMidBlock ) * blockSize );
      
      double thisRadius = m_positionOnFace[row][col].Mod();
      
      if( (thisRadius < radius ) && (thisRadius > innerRadius) ) {
	
	// build the "channel map"
	m_row[m_numActiveBlocks] = row;

	BlockGain[row][col] = NewGains[m_numActiveBlocks];
	BlockWidth[row][col] = width[m_numActiveBlocks];
	BlockMean[row][col] = fitMax[m_numActiveBlocks];
	BlockGainActive[m_numActiveBlocks] = NewGains[m_numActiveBlocks];
	
	XYGains->Fill((row - 29), (col - 29), BlockGainActive[m_numActiveBlocks]);
	XYMean->Fill((row - 29), (col - 29), BlockMean[row][col]);
	XYWidth->Fill((row - 29), (col - 29), BlockWidth[row][col]);

	for (int sq = 0; sq < 24; sq ++) {
	  if (sq_vs_ch[m_numActiveBlocks] == sq) {
	    XYGeo[sq]->Fill((row - 29), (col - 29), 1);
	  }
	}
	m_numActiveBlocks++;
      }
    }
  }
  
  out.open(path + "bad_gains.txt");
  for (int i = 0; i < (int) bad_det_tab.size(); i ++) {
    out << bad_det_tab[i] << endl;
  }
  out.close();

  // Dump the gains
  ofstream textOut1;
  textOut1.open(path + "new_gains.txt");
  int counter = 0;
  for (auto i: BlockGainActive){
    textOut1 << i << endl;
    counter ++;
  }
  textOut1.close();
  
  ofstream textOut2;
  textOut2.open(path + "old_gains.txt");
  counter = 0;
  for (auto i: OldGains){
    textOut2 << i << endl;
    counter ++;
  }
  textOut2.close();

  ofstream textOut3;
  textOut3.open(path + "chi2_values.txt");
  counter = 0;
  for (auto i: NewGainsChi2){
    textOut3 << i << endl;
    counter ++;
  }
  textOut3.close();
  
  //Graph of new gains, Chi^2, and width
  grNewGain = new TGraphErrors(2800, channelNum, BlockGainActive, 0, 0);
  grNewGain->SetName("grNewGain");
  grNewGain->Write();
  grchiSquare = new TGraphErrors(2800, channelNum, NormalizedChi2, 0, 0);
  grchiSquare->SetName("grchiSquare");
  grchiSquare->Write();
  grWidth = new TGraphErrors(2800, channelNum, width, 0, width_err);
  grWidth->SetName("grWidth");
  grWidth->Write();

  Nice3D2D(XYMean ,0.05,0.05,42,505,1.25,1.55,1.75,"",
           "#font[42]{Column #}",
           "#font[42]{Row #}",
           "#font[42]{Mean [GeV/#it{c}^{2}]}");
  
  Nice3D2D(XYWidth ,0.05,0.05,42,505,1.25,1.55,1.75,"",
	   "#font[42]{Column #}",
	   "#font[42]{Row #}",
	   "#font[42]{Width [GeV/#it{c}^{2}]}");
  
  Nice3D2D(XYGains ,0.05,0.05,42,505,1.25,1.55,1.75,"",
	   "#font[42]{Column #}",
	   "#font[42]{Row #}",
	   "#font[42]{Gain}");

  smallBetween1 = .15;
  smallBetween2 = .25;
  smallBetween3 = .05;
  smallBetween4 = .15;
  
  cleg = path + "XYMean";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  XYMean->Draw("colz");
  
  C1->Print(cleg + ".pdf");

  cleg = path + "XYWidth";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  XYWidth->Draw("colz");

  C1->Print(cleg + ".pdf");

  cleg = path + "XYGains";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  XYGains->Draw("colz");

  C1->Print(cleg + ".pdf");
  
  for (int i = 0; i < 24; i ++) {
    cleg = path + Form("XYGeo_%d", i);
    C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    Nice3D2D(XYGeo[i],0.05,0.05,42,505,1.25,1.55,1.75,"",
	     "#font[42]{Column #}",
	     "#font[42]{Row #}",
	     "#font[42]{Entry}");
    XYGeo[i]->Draw("colz");
    
    C1->Print(cleg + ".pdf");
  }

  TGraphErrors * grFitPars_eta[2];
  TGraphErrors * grFitPars_pi0[2];
  
  for (int i = 0; i < 2; i ++) {
    grFitPars_eta[i] = new TGraphErrors();
    grFitPars_pi0[i] = new TGraphErrors();
    grFitPars_eta[i]->SetName(Form("grFitPars_eta_%d", i));
    grFitPars_pi0[i]->SetName(Form("grFitPars_pi0_%d", i));
  }
  
  TFile * f_aaMass[100];
  TH1F * h_aaMass[100];

  for (int i = 0; i < ITERATION; i ++) {
    
    if (i < ITERATION) {
      f_aaMass[i] = new TFile (Form(path + "../g%d/runpi0.root", i + 1));
      h_aaMass[i] = (TH1F *) f_aaMass[i]->Get("FCAL_Pi0log/Pi0Mass");
      h_aaMass[i]->SetName(Form("Pi0Mass_%d", i));
      Nice1D(h_aaMass[i], 0.05, 0.05, 42, 505, 1.1, 1.3, "", "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}", "#font[41]{Counts}");
      
      bkg_eta_fc  = new TF1(Form("bkg_eta_fc_%d", i),"pol3",0.440,0.700);
      sig_eta_fc  = new TF1(Form("sig_eta_fc_%d", i),"gaus",0.500,0.600);
      tot_eta_fc  = new TF1(Form("tot_eta_fc_%d", i),"gaus + pol3(3)",0.440,0.700);
            
      tot_eta_fc->SetLineColor(2);
      tot_eta_fc->SetLineWidth(2);
      bkg_eta_fc->SetLineColor(3);
      bkg_eta_fc->SetLineWidth(1);
      double peak_init = 0.536;
      double width_init = 0.01;
      Double_t par_tot_eta_fc[7], par_sig_eta_fc[3] = {0.,peak_init,width_init}, par_bkg_eta_fc[4] = {1.,1.,1.,1.};
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      sig_eta_fc->SetParameters(par_sig_eta_fc);
      bkg_eta_fc->SetParameters(par_bkg_eta_fc);
      
      h_aaMass[i]->Fit(bkg_eta_fc, "RMBQS0");
      bkg_eta_fc->GetParameters(&par_tot_eta_fc[3]);
      h_aaMass[i]->Fit(sig_eta_fc, "RMBQS0");
      sig_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      tot_eta_fc->SetParameters(par_tot_eta_fc);
      tot_eta_fc->SetParLimits(1,0.510,0.550);
      tot_eta_fc->SetParLimits(2,0.001,0.03);
      
      h_aaMass[i]->Fit(tot_eta_fc,"RMBQS0");
      tot_eta_fc->GetParameters(&par_tot_eta_fc[0]);
      bkg_eta_fc->SetParameters(&par_tot_eta_fc[3]);
      /*
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"pol4",0.096,0.18);
      sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol4(5)",0.096,0.18);
      */
      bkg_pi0_fc  = new TF1(Form("bkg_pi0_fc_%d", i),"exp(pol3)",0.09,0.225);
      sig_pi0_fc  = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc  = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + exp(pol3(5))",0.09,0.225);
      tot_pi0_fc->SetLineColor(2);
      tot_pi0_fc->SetLineWidth(2);
      bkg_pi0_fc->SetLineColor(3);
      bkg_pi0_fc->SetLineWidth(1);
      peak_init = 0.135;
      width_init = 0.008;
      Double_t par_tot_pi0_fc[11], par_sig_pi0_fc[5] = {0., peak_init, width_init,3.58653e+02,1.53665e+03}, par_bkg_pi0_fc[6] = {1., 1., 1., 1., 1., 1.};
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      sig_pi0_fc->SetParameters(par_sig_pi0_fc);
      bkg_pi0_fc->SetParameters(par_bkg_pi0_fc);
      
      h_aaMass[i]->Fit(bkg_pi0_fc, "RMBQS0");
      bkg_pi0_fc->GetParameters(&par_tot_pi0_fc[5]);
      h_aaMass[i]->Fit(sig_pi0_fc, "RMBQS0");
      sig_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      tot_pi0_fc->SetParameters(par_tot_pi0_fc);
      tot_pi0_fc->SetParLimits(1,0.130,0.140);
      tot_pi0_fc->SetParLimits(2,0.005,0.015);
      
      h_aaMass[i]->Fit(tot_pi0_fc,"RMBQS0");
      tot_pi0_fc->GetParameters(&par_tot_pi0_fc[0]);
      bkg_pi0_fc->SetParameters(&par_tot_pi0_fc[5]);

      grFitPars_pi0[0]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(1) * 1e3);
      grFitPars_pi0[0]->SetPointError(i, 0, tot_pi0_fc->GetParError(1) * 1e3);
      grFitPars_eta[0]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(1) * 1e3);
      grFitPars_eta[0]->SetPointError(i, 0, tot_eta_fc->GetParError(1) * 1e3);
      grFitPars_pi0[1]->SetPoint(i, (double) i, tot_pi0_fc->GetParameter(2) * 1e3);
      grFitPars_pi0[1]->SetPointError(i, 0, tot_pi0_fc->GetParError(2) * 1e3);
      grFitPars_eta[1]->SetPoint(i, (double) i, tot_eta_fc->GetParameter(2) * 1e3);
      grFitPars_eta[1]->SetPointError(i, 0, tot_eta_fc->GetParError(2) * 1e3);

      smallBetween1 = .15;
      smallBetween2 = .05;
      smallBetween3 = .05;
      smallBetween4 = .15;
      cleg = path + Form("aaMass_iter_%d", i);
      C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
      gPad->SetLeftMargin(smallBetween1);
      gPad->SetRightMargin(smallBetween2);
      gPad->SetTopMargin(smallBetween3);
      gPad->SetBottomMargin(smallBetween4);

      h_aaMass[i]->SetLineColor(1);
      h_aaMass[i]->SetLineWidth(2);
      h_aaMass[i]->SetMinimum(1e2);
      h_aaMass[i]->Draw();
      tot_pi0_fc->Draw("same");
      bkg_pi0_fc->Draw("same");
      tot_eta_fc->Draw("same");
      bkg_eta_fc->Draw("same");
      gPad->SetLogy();
      double max = h_aaMass[i]->GetMaximum();
      t->SetTextSize(0.03);
      t->DrawLatex(0.35, max/1.0, Form("#font[42]{#mu_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(1) * 1e3, tot_pi0_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/2.0, Form("#font[42]{#sigma_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(2) * 1e3, tot_pi0_fc->GetParError(2) * 1e3));
      t->DrawLatex(0.35, max/4.0, Form("#font[42]{#mu_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(1) * 1e3, tot_eta_fc->GetParError(1) * 1e3));
      t->DrawLatex(0.35, max/8.0, Form("#font[42]{#sigma_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(2) * 1e3, tot_eta_fc->GetParError(2) * 1e3));
      C1->Print(cleg + ".pdf");
    }
  }

  outFile->cd();

  for (int i = 0; i < 2; i ++) {
    grFitPars_eta[i]->Write();
    grFitPars_pi0[i]->Write();
  }
  
  TH2F * DrawM_pi0 = new TH2F("DrawM_pi0", ";#font[42]{Iteration #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, (double) (ITERATION + 1), 100, 128, 135.99);
  TH2F * DrawW_pi0 = new TH2F("DrawW_pi0", ";#font[42]{Iteration #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, (double) (ITERATION + 1), 100, 7.0, 9.99);
  Nicer2D(DrawM_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_pi0, 0.05, 42, 505, 1.2, 1.4, 1.2);

  TH2F * DrawM_eta = new TH2F("DrawM_eta", ";#font[42]{Iteration #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, 0.0, (double) (ITERATION + 1), 100, 530, 549.99);
  TH2F * DrawW_eta = new TH2F("DrawW_eta", ";#font[42]{Iteration #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, 0.0, (double) (ITERATION + 1), 100, 10.0, 29.99);
  Nicer2D(DrawM_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  Nicer2D(DrawW_eta, 0.05, 42, 505, 1.2, 1.4, 1.2);
  
  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  cleg = path + "Mean_vs_iteration_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_pi0->Draw();
  grFitPars_pi0[0]->SetMarkerStyle(20);
  grFitPars_pi0[0]->SetMarkerSize(1.2);
  grFitPars_pi0[0]->SetMarkerColor(1);
  grFitPars_pi0[0]->SetLineColor(1);
  grFitPars_pi0[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 134.97, (double) (ITERATION + 1), 134.97);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_iteration_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_pi0->Draw();
  grFitPars_pi0[1]->SetMarkerStyle(20);
  grFitPars_pi0[1]->SetMarkerSize(1.2);
  grFitPars_pi0[1]->SetMarkerColor(1);
  grFitPars_pi0[1]->SetLineColor(1);
  grFitPars_pi0[1]->Draw("PSZ");
  
  C1->Print(cleg + ".pdf");
    
  cleg = path + "Mean_vs_iteration_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawM_eta->Draw();
  grFitPars_eta[0]->SetMarkerStyle(20);
  grFitPars_eta[0]->SetMarkerSize(1.2);
  grFitPars_eta[0]->SetMarkerColor(1);
  grFitPars_eta[0]->SetLineColor(1);
  grFitPars_eta[0]->Draw("PSZ");
  TLine.SetLineColor(2);
  TLine.SetLineWidth(2);
  TLine.SetLineStyle(2);
  TLine.DrawLine(0, 547.43, (double) (ITERATION + 1), 547.43);
  C1->Print(cleg + ".pdf");

  cleg = path + "Width_vs_iteration_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  DrawW_eta->Draw();
  grFitPars_eta[1]->SetMarkerStyle(20);
  grFitPars_eta[1]->SetMarkerSize(1.2);
  grFitPars_eta[1]->SetMarkerColor(1);
  grFitPars_eta[1]->SetLineColor(1);
  grFitPars_eta[1]->Draw("PSZ");
  C1->Print(cleg + ".pdf");
  
  outFile->Write();
  outFile->Close();
  
}
