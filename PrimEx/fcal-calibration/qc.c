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

void qc() {
    
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
  Char_t tmp0[256];

  TF1 *bkg_eta_fc;
  TF1 *sig_eta_fc;
  TF1 *tot_eta_fc;
  TF1 *bkg_pi0_fc;
  TF1 *sig_pi0_fc;
  TF1 *tot_pi0_fc;
  TString str_meson[] = {"#pi^{0}", "#eta"};
  //TString str_rn[] = {"Be", "He 1", "He 2", "He 3"};
  //TString str_rn[] = {"Be", "He 1", "He 2", "He 3", "He 4", "He 5", "He 6"};
  //TString str_rn[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14"};
  TString str_rn[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", 
		      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
		      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		      "30", "31", "32", "33", "34", "35", "36", "37", "38", "29",
		      "40", "41", "42", "43", "44", "45", "46", "47", "48"};
  TGraphErrors * grFitPars_pi0[2];
  TGraphErrors * grFitPars_eta[2];
  TGraphErrors * grMeson_ctr[2];
  for (int j = 0; j < 2; j ++) {
    grMeson_ctr[j] = new TGraphErrors();
    grFitPars_eta[j] = new TGraphErrors();
    grFitPars_pi0[j] = new TGraphErrors();
  }
  TString path = "PDF/";
  int n_period = 48;
  int i = 0; 
  in.open("primex.txt");
  //in.open("testing.txt");
  while ( in.good() ) {
    TString LogLine = "";
    LogLine.ReadLine(in);
    Line = LogLine; 
    Line = Line.ReplaceAll("primex_runs_qc-", "");
    Line = Line.ReplaceAll("added", " ");
    Line = Line.ReplaceAll("/", " ");
    Line = Line.ReplaceAll("_", " ");
    Line = Line.ReplaceAll(".root", "");
    sscanf(Line.Data(),"%*s %*s %s",tmp0);
    int runnb = atof(tmp0);
    cout << runnb << endl;
    if (runnb == 0 || LogLine == "") continue;
    
    TFile * ifile = new TFile(LogLine);
    //TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0HFA/Pi0Mass2g");
    //TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0HFA/Pi0Mass2g_ring_4");
    //TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0TOF/Pi0Mass2g");
    //TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0TOF/Pi0Mass");
    TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0log/Pi0Mass");
    //TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0HFA/Pi0Mass");
    //TH1F * h1D = (TH1F *) ifile->Get("FCAL_Pi0HFA/Pi0Mass2g_sq_0");
    //h1D->SetName(Form("aa_im_runnb_%d", runnb));
    Nice1D(h1D, 0.05, 0.05, 42, 505, 1.1, 1.3, "", "#font[42]{#gamma#gamma invariant mass [GeV/#it{c}^{2}]}", "#font[41]{Counts}");

    bkg_eta_fc = new TF1(Form("bkg_eta_fc_%d", i),"pol3",0.440,0.700);
    sig_eta_fc = new TF1(Form("sig_eta_fc_%d", i),"gaus",0.500,0.600);
    tot_eta_fc = new TF1(Form("tot_eta_fc_%d", i),"gaus + pol3(3)",0.440,0.700);

    tot_eta_fc->SetLineColor(2);
    tot_eta_fc->SetLineWidth(2);
    bkg_eta_fc->SetLineColor(3);
    bkg_eta_fc->SetLineWidth(1);
    double peak_init = 0.536;
    double width_init = 0.01;
    Double_t partot_eta_fc[7],parsig_eta_fc[3] = {0.,peak_init,width_init},parbkg_eta_fc[4] = {1.,1.,1.,1.};
    tot_eta_fc->SetParameters(partot_eta_fc);
    sig_eta_fc->SetParameters(parsig_eta_fc);
    bkg_eta_fc->SetParameters(parbkg_eta_fc);

    h1D->Fit(bkg_eta_fc,"RMBQS0");
    bkg_eta_fc->GetParameters(&partot_eta_fc[3]);
    h1D->Fit(sig_eta_fc,"RMBQS0");
    sig_eta_fc->GetParameters(&partot_eta_fc[0]);
    tot_eta_fc->SetParameters(partot_eta_fc);
    tot_eta_fc->SetParLimits(1,0.510,0.570);
    tot_eta_fc->SetParLimits(2,0.001,0.1);

    h1D->Fit(tot_eta_fc,"RMBQS0");
    tot_eta_fc->GetParameters(&partot_eta_fc[0]);
    bkg_eta_fc->SetParameters(&partot_eta_fc[3]);
    sig_eta_fc->SetParameters(&partot_eta_fc[0]);
    
    //,0.09,0.225
    double min_mass = 0.09;
    double max_mass = 0.225;
    bkg_pi0_fc = new TF1(Form("bkg_pi0_fc_%d", i),"exp(pol3)",min_mass, max_mass);
    sig_pi0_fc = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
    tot_pi0_fc = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + exp(pol3(5))",min_mass,max_mass);
    /*
    //TString str_fc = "pol3";
    if (runnb > 61354) {
      min_mass = 0.07;
      max_mass = 0.22;
      //str_fc = "pol5"
    }
    if (runnb > 61900) {
      min_mass = 0.09;
      max_mass = 0.19;
    }
    if (runnb <= 61354) {
      bkg_pi0_fc = new TF1(Form("bkg_pi0_fc_%d", i),"pol3",min_mass, max_mass);
      sig_pi0_fc = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + pol3(5)",min_mass,max_mass);
    } else {
      bkg_pi0_fc = new TF1(Form("bkg_pi0_fc_%d", i),"exp(pol3)",min_mass, max_mass);
      sig_pi0_fc = new TF1(Form("sig_pi0_fc_%d", i),"crystalball",0.12,0.16);
      tot_pi0_fc = new TF1(Form("tot_pi0_fc_%d", i),"crystalball + exp(pol3(5))",min_mass,max_mass);
    }
			    */
    tot_pi0_fc->SetLineColor(2);
    tot_pi0_fc->SetLineWidth(2);
    bkg_pi0_fc->SetLineColor(3);
    bkg_pi0_fc->SetLineWidth(1);
    peak_init = 0.135;
    width_init = 0.007;
    Double_t partot_pi0_fc[11],parsig_pi0_fc[5] = {0,peak_init,width_init,50,250},parbkg_pi0_fc[6] = {1.,1.,1.,1.,1.,1.};
    tot_pi0_fc->SetParameters(partot_pi0_fc);
    sig_pi0_fc->SetParameters(parsig_pi0_fc);
    bkg_pi0_fc->SetParameters(parbkg_pi0_fc);

    h1D->Fit(bkg_pi0_fc,"RMBQS0");
    bkg_pi0_fc->GetParameters(&partot_pi0_fc[5]);
    h1D->Fit(sig_pi0_fc,"RMBQS0");
    sig_pi0_fc->GetParameters(&partot_pi0_fc[0]);
    tot_pi0_fc->SetParameters(partot_pi0_fc);
    tot_pi0_fc->SetParLimits(1,0.120,0.150);
    tot_pi0_fc->SetParLimits(2,0.005,0.015);
    tot_pi0_fc->SetParLimits(3,0,100);
    tot_pi0_fc->SetParLimits(4,0,500);
    h1D->Fit(tot_pi0_fc,"RMBQS0");
    tot_pi0_fc->GetParameters(&partot_pi0_fc[0]);
    bkg_pi0_fc->SetParameters(&partot_pi0_fc[5]);
    sig_pi0_fc->SetParameters(&partot_pi0_fc[0]);
    double n_pi0 = (sig_pi0_fc->Integral(tot_pi0_fc->GetParameter(1) - 5.0 * fabs(tot_pi0_fc->GetParameter(2)), tot_pi0_fc->GetParameter(1) + 5.0 * fabs(tot_pi0_fc->GetParameter(2)))) / h1D->GetBinWidth(1);
    double n_pi0_err = (sig_pi0_fc->GetParError(0) / sig_pi0_fc->GetParameter(0)) * n_pi0;
    double n_eta = (sig_eta_fc->Integral(tot_eta_fc->GetParameter(1) - 5.0 * fabs(tot_eta_fc->GetParameter(2)), tot_eta_fc->GetParameter(1) + 5.0 * fabs(tot_eta_fc->GetParameter(2)))) / h1D->GetBinWidth(1);
    double n_eta_err = (sig_eta_fc->GetParError(0) / sig_eta_fc->GetParameter(0)) * n_eta;

    grMeson_ctr[0]->SetPoint(i, (double) runnb, n_pi0);
    grMeson_ctr[0]->SetPointError(i, 0, n_pi0_err);
    grMeson_ctr[1]->SetPoint(i, (double) runnb, n_eta);
    grMeson_ctr[1]->SetPointError(i, 0, n_eta_err);
    
    grFitPars_eta[0]->SetPoint(i, (double) runnb, tot_eta_fc->GetParameter(1) * 1e3);
    grFitPars_eta[0]->SetPointError(i, 0, tot_eta_fc->GetParError(1) * 1e3);
    grFitPars_eta[1]->SetPoint(i, (double) runnb, fabs(tot_eta_fc->GetParameter(2)) * 1e3);
    grFitPars_eta[1]->SetPointError(i, 0, tot_eta_fc->GetParError(2) * 1e3);

    grFitPars_pi0[0]->SetPoint(i, (double) runnb, tot_pi0_fc->GetParameter(1) * 1e3);
    grFitPars_pi0[0]->SetPointError(i, 0, tot_pi0_fc->GetParError(1) * 1e3);
    grFitPars_pi0[1]->SetPoint(i, (double) runnb, fabs(tot_pi0_fc->GetParameter(2)) * 1e3);
    grFitPars_pi0[1]->SetPointError(i, 0, tot_pi0_fc->GetParError(2) * 1e3);
   
    smallBetween1 = .15;                                                                                                                                                                                  
    smallBetween2 = .05; 
    smallBetween3 = .05; 
    smallBetween4 = .15; 
    
    cleg = path + Form("aaMass_%d", runnb);
    C1 = new TCanvas(cleg, cleg, 10, 10, 800, 800);
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);

    h1D->SetLineColor(1);
    h1D->SetLineWidth(2);
    h1D->SetMinimum(1);
    h1D->Draw();
    tot_pi0_fc->Draw("same");
    tot_eta_fc->Draw("same");
    bkg_pi0_fc->Draw("same");
    bkg_eta_fc->Draw("same");
    gPad->SetLogy();
    double max = h1D->GetMaximum();
    t->SetTextSize(0.03);
    t->DrawLatex(0.35, max/1.0, Form("#font[42]{#mu_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(1) * 1e3, tot_pi0_fc->GetParError(1) * 1e3));
    t->DrawLatex(0.35, max/2.0, Form("#font[42]{#sigma_{#pi^{0}} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_pi0_fc->GetParameter(2) * 1e3, tot_pi0_fc->GetParError(2) * 1e3));
    t->DrawLatex(0.35, max/4.0, Form("#font[42]{#mu_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(1) * 1e3, tot_eta_fc->GetParError(1) * 1e3));
    t->DrawLatex(0.35, max/8.0, Form("#font[42]{#sigma_{#eta} = %0.2f #pm %0.2f MeV/#it{c}^{2}}", tot_eta_fc->GetParameter(2) * 1e3, tot_eta_fc->GetParError(2) * 1e3));
    C1->Print(cleg + ".pdf");

    i ++;
    ifile->Close();
  }
  in.close();
  //double min_range[] = {61321, 61355, 61623, 61788};
  //double max_range[] = {61354, 61622, 61788, 61956};
  //min_run=(61355 61482 61623 61789 61911)
  //max_run=(61481 61622 61788 61910 61956)
  ////double min_range[] = {61321, 61355, 61482, 61623, 61789, 61911, 61945};
  ////double max_range[] = {61354, 61481, 61622, 61788, 61910, 61944, 61956};
  /*
    set min_run=(61321 61340 61378 61434 61470 61590 61510 61621 61700 61750 61810 61874 61930 61950)
    set max_run=(61339 61377 61433 61469 61489 61509 61620 61699 61749 61809 61873 61929 61949 61956)
  */
  //double min_range[] = {61321, 61340, 61378, 61434, 61470, 61590, 61510, 61621, 61700, 61750, 61810, 61874, 61930, 61950};
  //double max_range[] = {61339, 61377, 61433, 61469, 61489, 61509, 61620, 61699, 61749, 61809, 61873, 61929, 61949, 61956};
  
  double min_range[] = {61321, 61331, 61342, 61351, 61355, 61360, 61372, 61390, 61419, 61427, 61433, 61438, 61466, 61479, 61490, 61502, 61516, 61530, 61548, 61561, 61580, 61592, 61609, 61620, 61630, 61644, 61656, 61670, 61682, 61697, 61707, 61720, 61734, 61746, 61758, 61766, 61780, 61794, 61807, 61820, 61833, 61847, 61857, 61875, 61889, 61908, 61929, 61952};
  double max_range[] = {61330, 61341, 61350, 61354, 61359, 61371, 61389, 61418, 61426, 61431, 61437, 61465, 61478, 61489, 61501, 61515, 61529, 61547, 61560, 61579, 61591, 61608, 61619, 61629, 61643, 61655, 61669, 61681, 61695, 61706, 61719, 61732, 61745, 61757, 61765, 61779, 61793, 61806, 61818, 61832, 61846, 61855, 61874, 61888, 61906, 61928, 61951, 61956};

  double min_run = 61310;
  double max_run = 61999;
  double pi0Mass = 134.97;
  double etaMass = 547.43;
  double min_pi0Mass = pi0Mass-0.025*pi0Mass;
  double max_pi0Mass = pi0Mass+0.025*pi0Mass;
  double min_etaMass = etaMass-0.025*etaMass;
  double max_etaMass = etaMass+0.025*etaMass;
  double min_pi0Sigm = 6.8;
  double max_pi0Sigm = 7.99;
  double min_etaSigm = 15.0;
  double max_etaSigm = 29.99;
  double max_ctr = 1e8;
  TH2F * Draw_ctr = new TH2F("Draw_ctr", ";#font[42]{Run #};#font[42]{Meson number per run};", 100, min_run, max_run, 100, 1, max_ctr);
  TH2F * DrawM_pi0 = new TH2F("DrawM_pi0", ";#font[42]{Run #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, min_run, max_run, 100, min_pi0Mass, max_pi0Mass);
  TH2F * DrawW_pi0 = new TH2F("DrawW_pi0", ";#font[42]{Run #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, min_run, max_run, 100, min_pi0Sigm, max_pi0Sigm);
  Nicer2D(DrawM_pi0, 0.05, 42, 505, 1.2, 0.5, 1.2);
  Nicer2D(DrawW_pi0, 0.05, 42, 505, 1.2, 0.5, 1.2);
  Nicer2D(Draw_ctr, 0.05, 42, 505, 1.2, 0.5, 1.2);

  TH2F * DrawM_eta = new TH2F("DrawM_eta", ";#font[42]{Run #};#font[42]{Mean [MeV/#it{c}^{2}]};", 100, min_run, max_run, 100, min_etaMass, max_etaMass);
  TH2F * DrawW_eta = new TH2F("DrawW_eta", ";#font[42]{Run #};#font[42]{Width [MeV/#it{c}^{2}]};", 100, min_run, max_run, 100, min_etaSigm, max_etaSigm);
  Nicer2D(DrawM_eta, 0.05, 42, 505, 1.2, 0.5, 1.2);
  Nicer2D(DrawW_eta, 0.05, 42, 505, 1.2, 0.5, 1.2);
  
  smallBetween1 = .065;
  smallBetween2 = .015;
  smallBetween3 = .025;
  smallBetween4 = .15;
  
  cleg = path + "MesonNumber_vs_run_number";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 600);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  
  Draw_ctr->Draw();
  for (int i = 0; i < 2; i ++) {
    grMeson_ctr[i]->SetMarkerStyle(20 + i);
    grMeson_ctr[i]->SetMarkerSize(1.2);
    grMeson_ctr[i]->SetMarkerColor(1 + i);
    grMeson_ctr[i]->SetLineColor(1 + i);
    grMeson_ctr[i]->Draw("PSZ");
  }
  TLine.SetLineWidth(1);
  for (int k = 0; k < n_period; k ++) {
    TLine.SetLineColor(4);
    TLine.DrawLine(min_range[k], 1, min_range[k], max_ctr);
    TLine.DrawLine(max_range[k], 1, max_range[k], max_ctr);
    //if (k > 0) t->DrawLatex((min_range[k]+max_range[k]) / 2.0, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else  t->DrawLatex(min_range[k]+4, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else
    t->DrawLatex(min_range[k]+1, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //cout << "x " << (min_range[k]+max_range[k]) / 2.0 << " y " << min_pi0Mass*1.01 << endl;
  }
  legend=new TLegend(0.2,0.2,0.4,0.3);
  for (int i = 0; i < 2; i ++)
    legend->AddEntry(grMeson_ctr[i],TString::Format("#font[42]{%s}", str_meson[i].Data()),"p");
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  legend->SetTextFont(22);
  legend->SetTextSize(.03);
  legend->SetLineColor(0);
  legend->Draw("same");
  gPad->SetLogy();
  C1->Print(cleg + ".pdf");
  C1->Print(cleg + ".C");


  cleg = path + "Mean_vs_run_number_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 600);
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
  TLine.DrawLine(min_run, pi0Mass, max_run, pi0Mass);
  TLine.SetLineStyle(1);
  TLine.DrawLine(min_run, pi0Mass-0.01*pi0Mass, max_run, pi0Mass-0.01*pi0Mass);
  TLine.DrawLine(min_run, pi0Mass+0.01*pi0Mass, max_run, pi0Mass+0.01*pi0Mass);
  TLine.SetLineWidth(1);
  for (int k = 0; k < n_period; k ++) {
    TLine.SetLineColor(4);
    TLine.DrawLine(min_range[k], min_pi0Mass, min_range[k], max_pi0Mass);
    TLine.DrawLine(max_range[k], min_pi0Mass, max_range[k], max_pi0Mass);
    //if (k > 0) t->DrawLatex((min_range[k]+max_range[k]) / 2.0, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else  t->DrawLatex(min_range[k]+4, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else  t->DrawLatex(min_range[k]+1, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    t->DrawLatex(min_range[k]+1, min_pi0Mass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //cout << "x " << (min_range[k]+max_range[k]) / 2.0 << " y " << min_pi0Mass*1.01 << endl;
  }
  
  C1->Print(cleg + ".pdf");
  C1->Print(cleg + ".C");

  cleg = path + "Width_vs_run_number_pi0";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 600);
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
  TLine.SetLineWidth(1);
  for (int k = 0; k < n_period; k ++) {
    TLine.SetLineColor(4);
    TLine.DrawLine(min_range[k], min_pi0Sigm, min_range[k], max_pi0Sigm);
    TLine.DrawLine(max_range[k], min_pi0Sigm, max_range[k], max_pi0Sigm);
    //if (k > 0) t->DrawLatex((min_range[k]+max_range[k]) / 2.0, min_pi0Sigm*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else  t->DrawLatex(min_range[k]+4, min_pi0Sigm*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //t->DrawLatex(min_range[k]+1, min_pi0Sigm*1.025, Form("#font[42]{%s}", str_rn[k].Data()));
    t->DrawLatex(min_range[k]+1, min_pi0Sigm*1.015, Form("#font[42]{%s}", str_rn[k].Data()));
    //cout << "x " << (min_range[k]+max_range[k]) / 2.0 << " y " << min_pi0Mass*1.01 << endl;
  }
  C1->Print(cleg + ".pdf");
  C1->Print(cleg + ".C");
  
  cleg = path + "Mean_vs_run_number_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 600);
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
  TLine.DrawLine(min_run, etaMass, max_run, etaMass);
  TLine.SetLineStyle(1);
  TLine.DrawLine(min_run, etaMass-0.01*etaMass, max_run, etaMass-0.01*etaMass);
  TLine.DrawLine(min_run, etaMass+0.01*etaMass, max_run, etaMass+0.01*etaMass);
  TLine.SetLineWidth(1);
  for (int k = 0; k < n_period; k ++) {
    TLine.SetLineColor(4);
    TLine.DrawLine(min_range[k], min_etaMass, min_range[k], max_etaMass);
    TLine.DrawLine(max_range[k], min_etaMass, max_range[k], max_etaMass);
    //if (k > 0) t->DrawLatex((min_range[k]+max_range[k]) / 2.0, min_etaMass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else  t->DrawLatex(min_range[k]+4, min_etaMass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    t->DrawLatex(min_range[k]+1, min_etaMass*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //cout << "x " << (min_range[k]+max_range[k]) / 2.0 << " y " << min_etaMass*1.01 << endl;
  }
  C1->Print(cleg + ".pdf");
  C1->Print(cleg + ".C");

  cleg = path + "Width_vs_run_number_eta";
  C1 = new TCanvas(cleg, cleg, 10, 10, 1800, 600);
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
  TLine.SetLineWidth(1);
  for (int k = 0; k < n_period; k ++) {
    TLine.SetLineColor(4);
    TLine.DrawLine(min_range[k], min_etaSigm, min_range[k], max_etaSigm);
    TLine.DrawLine(max_range[k], min_etaSigm, max_range[k], max_etaSigm);
    //if (k > 0) t->DrawLatex((min_range[k]+max_range[k]) / 2.0, min_etaSigm*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //else  t->DrawLatex(min_range[k]+4, min_etaSigm*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    t->DrawLatex(min_range[k]+1, min_etaSigm*1.05, Form("#font[42]{%s}", str_rn[k].Data()));
    //cout << "x " << (min_range[k]+max_range[k]) / 2.0 << " y " << min_etaSigm*1.01 << endl;
  }
  C1->Print(cleg + ".pdf");
  C1->Print(cleg + ".C");
}
