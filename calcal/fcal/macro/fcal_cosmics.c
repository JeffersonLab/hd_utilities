#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
#include <TFile.h>
#include <TChain.h>
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
#include <TLatex.h>
#include <TLine.h>
#include <TGaxis.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TCut.h"
#include "TArrow.h"
#include "TLorentzRotation.h"
#include <TGenPhaseSpace.h>
#include <TTimeStamp.h>
#include <TRandom3.h>
#include <TFitResult.h>
#include "TMinuit.h"


#include "/work/halld/home/gxproj2/calibration/ilib/Nicer2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nicer1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1Db.c"
#include "/work/halld/home/gxproj2/calibration/ilib/NiceTH1.c"

void fcal_cosmics (TString s_ifile = "", TString s_old_gain = "", TString s_new_gain = "") {
  
  cout <<  "FCAL cosmics" << endl;
  
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
  gStyle->SetHistFillColor(999);
  gROOT->SetStyle("Plain");  // white as bg
  gStyle->SetOptStat("111111");
  gStyle->SetFrameLineWidth(2);
  gStyle->SetTitleFontSize(0.1);
  gStyle->SetTitleBorderSize(0);
  
  TH1::SetDefaultSumw2();
  TH2::SetDefaultSumw2();
    
  //Definitions
  Double_t smallBetween1 = .1;
  Double_t smallBetween2 = .1;
  Double_t smallBetween3 = .1;
  Double_t smallBetween4 = .1;
  
  Double_t small = .00001;
  
  TLine TLine;
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
  TString cleg = "";
  TString line = "";
  double max = 0;
    
  double cosmic_peak = 0.035; // GeV
  int x_bin = 1000;
  double x_min = 0.0;
  double x_max = 1.0;
  double s_x = (x_max - x_min) / ((double) x_bin);
  TH2F * h_E_v_blk = new TH2F("E_v_blk", ";#font[42]{Block #};#font[42]{Energy deposited [GeV]};#font[42]{Events #}", 2800, -0.5, 2799.5, x_bin, x_min, x_max);
  Nicer2D(h_E_v_blk, 0.05, 42, 505, 1.2, 1.5, 1.2);

  TH2F * h_y_v_x= new TH2F("y_v_x", ";#font[42]{x [cm]};#font[42]{y [cm]};#font[42]{Events #}", 52, -130, 130, 52, -130, 130);
  Nicer2D(h_y_v_x, 0.05, 42, 505, 1.2, 1.5, 1.2);
  
  TH1F * h_E[2800];
  
  TChain * ichain = new TChain("fcalBlockHits");
  ichain->Add(s_ifile);

  ichain->Draw("E:chan >> E_v_blk", "", "");
  ichain->Draw("y:x >> y_v_x", "", "");

  vector <int> blk_list;
  vector <int> bad_blk_list;
  vector <int> good_blk_list;
  vector <double> peak_list;
  vector <double> old_gain_list;
  vector <double> new_gain_list;

  TH1F * h_new_gain = new TH1F("new_gain", ";#font[42]{Software (new) gain};#font[42]{Entries [a.u.]}", 100, 0., 9.88);
  TH1F * h_old_gain = new TH1F("old_gain", ";#font[42]{Software (old) gain};#font[42]{Entries [a.u.]}", 100, 0., 9.88);
  Nicer1D(h_old_gain, 0.05, 42, 505, 1.2, 1.5);
  Nicer1D(h_new_gain, 0.05, 42, 505, 1.2, 1.5);
  
  in.open(s_old_gain);
  int ctr = 0;
  while (ctr < 2800) {
    double old_gain = 0;
    in >> old_gain;
    old_gain_list.push_back(old_gain);
    h_old_gain->Fill(old_gain);
    ctr ++;
  }
  in.close();
  
  for (int i = 0; i < 2800; i ++) {
    h_E[i] = (TH1F *) h_E_v_blk->ProjectionY(Form("E_%d", i), i + 1, i + 1);
    //Nice1D(h_E[i], 0.05, 0.05, 42, 505, 1., 1.8, "", "#font[42]{Energy deposited [GeV]}", Form("#font[41]{Events / %0.3f [GeV]}", s_x));
    Nice1Db(h_E[i], 0., 0.099, 0.05, 0.05, 42, 505, 1., 1.8, "", "#font[42]{Energy deposited [GeV]}", Form("#font[41]{Events / %0.3f [GeV]}", s_x));
    if (h_E[i]->GetEntries() > 0) blk_list.push_back(i);
  }
  cout << "Number of block with entries " << blk_list.size() << endl;
  
  line = s_old_gain.ReplaceAll(".txt", "");
  
  cleg = "plot-cosmics-E_v_blk-" + line;
  smallBetween1 = .175;
  smallBetween2 = .2;
  smallBetween3 = .05;
  smallBetween4 = .2;
  C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_E_v_blk->SetMinimum(1);
  h_E_v_blk->Draw("colz");
  gPad->SetLogz();
  C1->Print(cleg + ".pdf");

  cleg = "plot-cosmics-y_v_x-" + line;
  smallBetween1 = .175;
  smallBetween2 = .2;
  smallBetween3 = .05;
  smallBetween4 = .2;
  C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_y_v_x->SetMinimum(1);
  h_y_v_x->Draw("colz");
  gPad->SetLogz();
  C1->Print(cleg + ".pdf");
    
  int cdv = 25;
  int cnb = blk_list.size() / cdv + 1;
  cout << "Number of Canvas " << cnb << endl;

  TF1 * fct_cos;

  TH1F * h_chi2 = new TH1F("chi2", ";#font[42]{#chi^{2}/ndf};#font[42]{Entries [a.u.]}", 100, 0., 9.99);
  TH1F * h_peak = new TH1F("peak", ";#font[42]{Landau peak [GeV]};#font[42]{Entries [a.u.]}", 200, 0., 0.099);
  Nicer1D(h_peak, 0.05, 42, 505, 1.2, 1.5);
  Nicer1D(h_chi2, 0.05, 42, 505, 1.2, 1.5);
  for (int i = 0; i < cnb; i ++) {
    //for (int i = 0; i < 1; i ++) {
    cleg = Form("plot-cosmics-E-canvas-%d-", i) + line;
    smallBetween1 = .175;
    smallBetween2 = .05;
    smallBetween3 = .05;
    smallBetween4 = .175;
    C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
    C1->Divide(5, 5);
    for (int j = 0; j < cdv; j ++) {
      int k = i * cdv + j;
      if (k <  blk_list.size()) {
	//fct_cos = new TF1(Form("fct_%d", k), "landau(0)+landau(3)", 0.01, 0.059);
	fct_cos = new TF1(Form("fct_%d", k), "landau(0)+pol2(3)", 0.01, 0.079);
	//fct_cos = new TF1(Form("fct_%d", k), "landau", 0.01, 0.059);
	fct_cos->SetParameters(h_E[blk_list[k]]->GetMaximum(), cosmic_peak, 0.004, 1, 1, 1);
	fct_cos->SetLineColor(2);
	fct_cos->SetLineWidth(2);
	h_E[blk_list[k]]->Fit(fct_cos,"RB0+");
	double lp = fct_cos->GetParameter(1);
	double chi2 = fct_cos->GetChisquare();
	double ndf = fct_cos->GetNDF();
	C1->cd(j + 1);
	gPad->SetLeftMargin(smallBetween1);
	gPad->SetRightMargin(smallBetween2);
	gPad->SetTopMargin(smallBetween3);
	gPad->SetBottomMargin(smallBetween4);
	h_E[blk_list[k]]->SetMinimum(0);
	//h_E[blk_list[k]]->SetMarkerStyle(20);
	//h_E[blk_list[k]]->SetMarkerSize(1.2);
	//h_E[blk_list[k]]->Draw("APZ");
	NiceTH1(h_E[blk_list[k]], 20, 1.2, 1, 1, 3, 0);
	t->DrawLatex(0.05, h_E[blk_list[k]]->GetMaximum(), Form("#font[42]{blk# %d}", blk_list[k]));
	//t->DrawLatex(0.025, h_E[blk_list[k]]->GetMaximum() * 0.9, Form("#font[42]{#mu = %0.3f GeV #pm %0.3f GeV}", fct_cos->GetParameter(1), fct_cos->GetParError(1)));
	t->DrawLatex(0.05, h_E[blk_list[k]]->GetMaximum() * 0.9, Form("#font[42]{#mu = %0.3fGeV}", lp));
	t->DrawLatex(0.05, h_E[blk_list[k]]->GetMaximum() * 0.8, Form("#font[42]{#chi^{2}/ndf = %0.3f}", chi2/ndf));
	t->DrawLatex(0.05, h_E[blk_list[k]]->GetMaximum() * 0.7, Form("#font[42]{g^{old} = %0.4f}", old_gain_list[blk_list[k]]));
	h_peak->Fill(lp);
	h_chi2->Fill(chi2 / ndf);
	fct_cos->Draw("csame");
	if ((lp < 0.01 || lp > 0.08) || ((chi2 / ndf) > 5 || (chi2 / ndf) < 0.1))
	  bad_blk_list.push_back(blk_list[k]);
	else {
	  good_blk_list.push_back(blk_list[k]);
	  peak_list.push_back(lp);
	}
      }
    }
    C1->Print(cleg + ".pdf");
  }

  cleg = "plot-landau-peak-" + line;
  smallBetween1 = .175;
  smallBetween2 = .2;
  smallBetween3 = .05;
  smallBetween4 = .2;
  C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_peak->SetMinimum(0);
  NiceTH1(h_peak, 20, 1.2, 1, 1, 3, 3);
  TLine.SetLineColor(2);
  TLine.SetLineStyle(2);
  TLine.SetLineWidth(3);
  TLine.DrawLine(cosmic_peak, 0, cosmic_peak, h_peak->GetMaximum());
  TLine.DrawLine(0.01, 0, 0.01, h_peak->GetMaximum());
  TLine.DrawLine(0.08, 0, 0.08, h_peak->GetMaximum());
  C1->Print(cleg + ".pdf");
  
  cleg = "plot-fit-chi2-per-ndf-" + line;
  smallBetween1 = .175;
  smallBetween2 = .2;
  smallBetween3 = .05;
  smallBetween4 = .2;
  C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_chi2->SetMinimum(0);
  NiceTH1(h_chi2, 20, 1.2, 1, 1, 3, 3);
  TLine.SetLineColor(2);
  TLine.SetLineStyle(2);
  TLine.SetLineWidth(3);
  TLine.DrawLine(0.1, 0, 0.1, h_chi2->GetMaximum());
  TLine.DrawLine(5, 0, 5, h_chi2->GetMaximum());
  C1->Print(cleg + ".pdf");
  
  cdv = 25;
  cnb = bad_blk_list.size() / cdv + 1;
  cout << "Number of Canvas with bad blocks " << cnb << endl;
  for (int i = 0; i < cnb; i ++) {
    cleg = Form("plot-cosmics-bad-E-canvas-%d-", i) + line;
    smallBetween1 = .175;
    smallBetween2 = .05;
    smallBetween3 = .05;
    smallBetween4 = .175;
    C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
    C1->Divide(5, 5);
    for (int j = 0; j < cdv; j ++) {
      int k = i * cdv + j;
      if (k <  bad_blk_list.size()) {
	C1->cd(j + 1);
	gPad->SetLeftMargin(smallBetween1);
	gPad->SetRightMargin(smallBetween2);
	gPad->SetTopMargin(smallBetween3);
	gPad->SetBottomMargin(smallBetween4);
	h_E[bad_blk_list[k]]->SetMinimum(0);
	double mean = h_E[bad_blk_list[k]]->GetMean();
	double rms = h_E[bad_blk_list[k]]->GetRMS();
	Nice1Db(h_E[bad_blk_list[k]], mean - rms, mean + rms, 0.05, 0.05, 42, 505, 1., 1.8, "", "#font[42]{Energy deposited [GeV]}", Form("#font[41]{Events / %0.3f [GeV]}", s_x));
	NiceTH1(h_E[bad_blk_list[k]], 20, 1.2, 1, 1, 3, 0);
	t->DrawLatex(0.05, h_E[bad_blk_list[k]]->GetMaximum(), Form("#font[42]{blk# %d}", bad_blk_list[k]));
      }
    }
    C1->Print(cleg + ".pdf");
  }
    
  for (int i = 0; i < good_blk_list.size(); i ++) {
    double fitted_peak = peak_list[i];
    double old_gain = old_gain_list[good_blk_list[i]];
    double new_gain = old_gain * cosmic_peak / fitted_peak;
    //cout << new_gain << endl;
    new_gain_list.push_back(new_gain);
    h_new_gain->Fill(new_gain);
  }
  cout << "Number of old gain " << old_gain_list.size() << endl;
  cout << "===================" << endl;
  out.open(s_new_gain);
  for (int i = 0; i < 2800; i ++) {
    bool good_blk = false;
    for (int j = 0; j < good_blk_list.size(); j ++) {
      if (i == good_blk_list[j]) {
	out << new_gain_list[j] << endl;
	good_blk = true;
      } 
    }
    if (!good_blk)
      out << old_gain_list[i] << endl;
  }
  out.close();
  cout << "Bad block " << bad_blk_list.size() << endl;

  out.open("bad-" + s_new_gain);
  for (int i = 0; i < bad_blk_list.size(); i ++) {
    out << bad_blk_list[i] << endl;
  }
  out.close();

  cleg = "plot-gain-" + line;
  smallBetween1 = .175;
  smallBetween2 = .2;
  smallBetween3 = .05;
  smallBetween4 = .2;
  C1 = new TCanvas(cleg, cleg, 10, 10, 900, 900);
  gPad->SetLeftMargin(smallBetween1);
  gPad->SetRightMargin(smallBetween2);
  gPad->SetTopMargin(smallBetween3);
  gPad->SetBottomMargin(smallBetween4);
  h_new_gain->SetMinimum(0);
  NiceTH1(h_new_gain, 20, 1.2, 2, 1, 3, 2);
  NiceTH1(h_old_gain, 20, 1.2, 1, 1, 3, 3);
  TLine.SetLineColor(2);
  TLine.SetLineStyle(2);
  TLine.SetLineWidth(3);
  TLine.DrawLine(1, 0, 1, h_new_gain->GetMaximum());
  C1->Print(cleg + ".pdf");
}
