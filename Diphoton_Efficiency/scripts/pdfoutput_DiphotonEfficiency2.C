#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TPaveLabel.h>
#include <TLatex.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TLine.h>
#include <TArc.h>
#include <TMath.h>
#include <TGraphErrors.h>
#include <TEllipse.h>
#include <TBox.h>
#include <TArrow.h>
#include <iostream>
#include <fstream>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TAxis.h>
#include <TGaxis.h>

double gaussian(Double_t *x, Double_t *par)
{
    double variance2 = par[2]*par[2]*2.0;
    double term = x[0]-par[1];
    return par[0]*exp(-(term*term)/variance2) + par[3]*x[0]*x[0] + par[4]*x[0] + par[5];
}

void pdfoutput_DiphotonEfficiency2(string tag = "TEMP") {

// set the paper & margin sizes
gStyle->SetPaperSize(20,26);
gStyle->SetPadTopMargin(0.12);
gStyle->SetPadRightMargin(0.05);
gStyle->SetPadBottomMargin(0.16);
gStyle->SetPadLeftMargin(0.12);

// use large Times-Roman fonts
//gStyle->SetTextFont(132);
gStyle->SetTextSize(0.08);
//gStyle->SetLabelFont(132,"x");
//gStyle->SetLabelFont(132,"y");
//gStyle->SetLabelFont(132,"z");
gStyle->SetLabelSize(0.05,"x");
gStyle->SetTitleSize(0.07,"x");
gStyle->SetLabelSize(0.05,"y");
gStyle->SetTitleSize(0.07,"y");
gStyle->SetLabelSize(0.05,"z");
gStyle->SetTitleSize(0.07,"z");
gStyle->SetTitleSize(0.08,"t");
//gStyle->SetTitleSize(0.065,"xy");
gStyle->SetTitleOffset(0.8,"y");

gStyle->SetPadTickX(1);
gStyle->SetPadTickY(1);
gStyle->SetPadGridX(kTRUE);
gStyle->SetPadGridY(kTRUE);
gStyle->SetGridColor(16);
//gStyle->SetFitFormat("2.2e");
gStyle->SetOptFit(0);
gStyle->SetOptStat(0);
gStyle->SetStatY(0.88);
gStyle->SetStatX(0.95);
//gStyle->SetStatW(0.18);
gStyle->SetLegendTextSize(0.04);
gStyle->SetLegendFillColor(18);
gStyle->SetStatStyle(3001);
gROOT->ForceStyle();

  TH1D *heffSeparation_FCAL_island, *hthrownSeparation_FCAL_island, *hmatchedSeparation_FCAL_island;
  TH1D *heffSeparation_FCAL_nominal, *hthrownSeparation_FCAL_nominal, *hmatchedSeparation_FCAL_nominal;
  TH1D *heffSeparation_FCAL_island_logd, *hthrownSeparation_FCAL_island_logd, *hmatchedSeparation_FCAL_island_logd;
  TH1D *heffSeparation_FCAL_nominal_logd, *hthrownSeparation_FCAL_nominal_logd, *hmatchedSeparation_FCAL_nominal_logd;
  TH1D *heffSeparation_FCAL_island2, *hthrownSeparation_FCAL_island2, *hmatchedSeparation_FCAL_island2;
  TH1D *heffSeparation_FCAL_nominal2, *hthrownSeparation_FCAL_nominal2, *hmatchedSeparation_FCAL_nominal2;
  TH1D *heffSeparation_FCAL_island2_logd, *hthrownSeparation_FCAL_island2_logd, *hmatchedSeparation_FCAL_island2_logd;
  TH1D *heffSeparation_FCAL_nominal2_logd, *hthrownSeparation_FCAL_nominal2_logd, *hmatchedSeparation_FCAL_nominal2_logd;
  TH1D *heffSeparation_FCAL_island3, *hthrownSeparation_FCAL_island3, *hmatchedSeparation_FCAL_island3;
  TH1D *heffSeparation_FCAL_nominal3, *hthrownSeparation_FCAL_nominal3, *hmatchedSeparation_FCAL_nominal3;
  TH1D *heffSeparation_FCAL_island3_logd, *hthrownSeparation_FCAL_island3_logd, *hmatchedSeparation_FCAL_island3_logd;
  TH1D *heffSeparation_FCAL_nominal3_logd, *hthrownSeparation_FCAL_nominal3_logd, *hmatchedSeparation_FCAL_nominal3_logd;
  TH1D *heffSeparation_FCAL_island4, *hthrownSeparation_FCAL_island4, *hmatchedSeparation_FCAL_island4;
  TH1D *heffSeparation_FCAL_nominal4, *hthrownSeparation_FCAL_nominal4, *hmatchedSeparation_FCAL_nominal4;
  TH1D *heffSeparation_FCAL_island4_logd, *hthrownSeparation_FCAL_island4_logd, *hmatchedSeparation_FCAL_island4_logd;
  TH1D *heffSeparation_FCAL_nominal4_logd, *hthrownSeparation_FCAL_nominal4_logd, *hmatchedSeparation_FCAL_nominal4_logd;
  TH1D *htemp;
  TCanvas *c1;
  TLegend *legend1, *legend2, *legend3;

  TFile *file_Separation_FCAL_island = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111114_island/root/merged.root");
  TFile *file_Separation_FCAL_nominal = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111114_nominal/root/merged.root");
  TFile *file_Separation_FCAL_island2 = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111113_island/root/merged.root");
  TFile *file_Separation_FCAL_nominal2 = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111113_nominal/root/merged.root");
  TFile *file_Separation_FCAL_island3 = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111112_island/root/merged.root");
  TFile *file_Separation_FCAL_nominal3 = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111112_nominal/root/merged.root");
  TFile *file_Separation_FCAL_island4 = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111111_island/root/merged.root");
  TFile *file_Separation_FCAL_nominal4 = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111111_nominal/root/merged.root");

  hthrownSeparation_FCAL_island = (TH1D*)file_Separation_FCAL_island->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island = (TH1D*)file_Separation_FCAL_island->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_nominal = (TH1D*)file_Separation_FCAL_nominal->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal = (TH1D*)file_Separation_FCAL_nominal->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_island_logd = (TH1D*)file_Separation_FCAL_island->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island_logd = (TH1D*)file_Separation_FCAL_island->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_nominal_logd = (TH1D*)file_Separation_FCAL_nominal->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal_logd = (TH1D*)file_Separation_FCAL_nominal->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_island2 = (TH1D*)file_Separation_FCAL_island2->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island2 = (TH1D*)file_Separation_FCAL_island2->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_nominal2 = (TH1D*)file_Separation_FCAL_nominal2->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal2 = (TH1D*)file_Separation_FCAL_nominal2->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_island2_logd = (TH1D*)file_Separation_FCAL_island2->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island2_logd = (TH1D*)file_Separation_FCAL_island2->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_nominal2_logd = (TH1D*)file_Separation_FCAL_nominal2->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal2_logd = (TH1D*)file_Separation_FCAL_nominal2->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_island3 = (TH1D*)file_Separation_FCAL_island3->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island3 = (TH1D*)file_Separation_FCAL_island3->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_nominal3 = (TH1D*)file_Separation_FCAL_nominal3->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal3 = (TH1D*)file_Separation_FCAL_nominal3->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_island3_logd = (TH1D*)file_Separation_FCAL_island3->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island3_logd = (TH1D*)file_Separation_FCAL_island3->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_nominal3_logd = (TH1D*)file_Separation_FCAL_nominal3->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal3_logd = (TH1D*)file_Separation_FCAL_nominal3->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_island4 = (TH1D*)file_Separation_FCAL_island4->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island4 = (TH1D*)file_Separation_FCAL_island4->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_nominal4 = (TH1D*)file_Separation_FCAL_nominal4->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal4 = (TH1D*)file_Separation_FCAL_nominal4->Get("h_matchedSeparation");
  hthrownSeparation_FCAL_island4_logd = (TH1D*)file_Separation_FCAL_island4->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_island4_logd = (TH1D*)file_Separation_FCAL_island4->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_nominal4_logd = (TH1D*)file_Separation_FCAL_nominal4->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL_nominal4_logd = (TH1D*)file_Separation_FCAL_nominal4->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL_island->Rebin(20);
  hmatchedSeparation_FCAL_island->Rebin(20);
  hthrownSeparation_FCAL_nominal->Rebin(20);
  hmatchedSeparation_FCAL_nominal->Rebin(20);
//  hthrownSeparation_FCAL_island_logd->Rebin(20);
  hmatchedSeparation_FCAL_island_logd->Rebin(20);
//  hthrownSeparation_FCAL_nominal_logd->Rebin(20);
  hmatchedSeparation_FCAL_nominal_logd->Rebin(20);
  hthrownSeparation_FCAL_island2->Rebin(20);
  hmatchedSeparation_FCAL_island2->Rebin(20);
  hthrownSeparation_FCAL_nominal2->Rebin(20);
  hmatchedSeparation_FCAL_nominal2->Rebin(20);
//  hthrownSeparation_FCAL_island2_logd->Rebin(20);
  hmatchedSeparation_FCAL_island2_logd->Rebin(20);
//  hthrownSeparation_FCAL_nominal2_logd->Rebin(20);
  hmatchedSeparation_FCAL_nominal2_logd->Rebin(20);
  hthrownSeparation_FCAL_island3->Rebin(20);
  hmatchedSeparation_FCAL_island3->Rebin(20);
  hthrownSeparation_FCAL_nominal3->Rebin(20);
  hmatchedSeparation_FCAL_nominal3->Rebin(20);
//  hthrownSeparation_FCAL_island3_logd->Rebin(20);
  hmatchedSeparation_FCAL_island3_logd->Rebin(20);
//  hthrownSeparation_FCAL_nominal3_logd->Rebin(20);
  hmatchedSeparation_FCAL_nominal3_logd->Rebin(20);
  hthrownSeparation_FCAL_island4->Rebin(20);
  hmatchedSeparation_FCAL_island4->Rebin(20);
  hthrownSeparation_FCAL_nominal4->Rebin(20);
  hmatchedSeparation_FCAL_nominal4->Rebin(20);
//  hthrownSeparation_FCAL_island4_logd->Rebin(20);
  hmatchedSeparation_FCAL_island4_logd->Rebin(20);
//  hthrownSeparation_FCAL_nominal4_logd->Rebin(20);
  hmatchedSeparation_FCAL_nominal4_logd->Rebin(20);

  c1 = new TCanvas();
  c1->cd();
    heffSeparation_FCAL_nominal = (TH1D*)hmatchedSeparation_FCAL_nominal->Clone("heffSeparation_FCAL_nominal"); heffSeparation_FCAL_nominal->Divide(hthrownSeparation_FCAL_nominal);
    heffSeparation_FCAL_nominal->SetLineColorAlpha(kBlue,0.9);
    heffSeparation_FCAL_nominal->SetMarkerColorAlpha(kBlue,0.9);
    heffSeparation_FCAL_nominal->SetLineStyle(1);
    heffSeparation_FCAL_nominal->SetLineWidth(4);
    heffSeparation_FCAL_nominal->SetTitle("FCAL1 Diphoton Gun Efficiency vs. FCAL Separation (Nominal)");
    heffSeparation_FCAL_nominal->SetMaximum(1.05);
    heffSeparation_FCAL_nominal->Draw("C");
    heffSeparation_FCAL_nominal2 = (TH1D*)hmatchedSeparation_FCAL_nominal2->Clone("heffSeparation_FCAL_nominal2"); heffSeparation_FCAL_nominal2->Divide(hthrownSeparation_FCAL_nominal2);
    heffSeparation_FCAL_nominal2->SetLineColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_nominal2->SetMarkerColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_nominal2->SetLineStyle(1);
    heffSeparation_FCAL_nominal2->SetLineWidth(4);
    heffSeparation_FCAL_nominal2->Draw("SAME C");
    heffSeparation_FCAL_nominal3 = (TH1D*)hmatchedSeparation_FCAL_nominal3->Clone("heffSeparation_FCAL_nominal3"); heffSeparation_FCAL_nominal3->Divide(hthrownSeparation_FCAL_nominal3);
    heffSeparation_FCAL_nominal3->SetLineColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_nominal3->SetMarkerColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_nominal3->SetLineStyle(1);
    heffSeparation_FCAL_nominal3->SetLineWidth(4);
    heffSeparation_FCAL_nominal3->Draw("SAME C");
    heffSeparation_FCAL_nominal4 = (TH1D*)hmatchedSeparation_FCAL_nominal4->Clone("heffSeparation_FCAL_nominal4"); heffSeparation_FCAL_nominal4->Divide(hthrownSeparation_FCAL_nominal4);
    heffSeparation_FCAL_nominal4->SetLineColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_nominal4->SetMarkerColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_nominal4->SetLineStyle(1);
    heffSeparation_FCAL_nominal4->SetLineWidth(4);
    heffSeparation_FCAL_nominal4->Draw("SAME C");

    heffSeparation_FCAL_nominal_logd = (TH1D*)hmatchedSeparation_FCAL_nominal_logd->Clone("heffSeparation_FCAL_nominal_logd"); heffSeparation_FCAL_nominal_logd->Divide(hthrownSeparation_FCAL_nominal_logd);
    heffSeparation_FCAL_nominal_logd->SetLineColorAlpha(kBlue+1,0.9);
    heffSeparation_FCAL_nominal_logd->SetMarkerColorAlpha(kBlue+1,0.9);
    heffSeparation_FCAL_nominal_logd->SetLineStyle(2);
    heffSeparation_FCAL_nominal_logd->SetLineWidth(4);
    heffSeparation_FCAL_nominal_logd->Draw("SAME C");
    heffSeparation_FCAL_nominal2_logd = (TH1D*)hmatchedSeparation_FCAL_nominal2_logd->Clone("heffSeparation_FCAL_nominal2_logd"); heffSeparation_FCAL_nominal2_logd->Divide(hthrownSeparation_FCAL_nominal2_logd);
    heffSeparation_FCAL_nominal2_logd->SetLineColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_nominal2_logd->SetMarkerColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_nominal2_logd->SetLineStyle(2);
    heffSeparation_FCAL_nominal2_logd->SetLineWidth(4);
    heffSeparation_FCAL_nominal2_logd->Draw("SAME C");
    heffSeparation_FCAL_nominal3_logd = (TH1D*)hmatchedSeparation_FCAL_nominal3_logd->Clone("heffSeparation_FCAL_nominal3_logd"); heffSeparation_FCAL_nominal3_logd->Divide(hthrownSeparation_FCAL_nominal3_logd);
    heffSeparation_FCAL_nominal3_logd->SetLineColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_nominal3_logd->SetMarkerColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_nominal3_logd->SetLineStyle(2);
    heffSeparation_FCAL_nominal3_logd->SetLineWidth(4);
    heffSeparation_FCAL_nominal3_logd->Draw("SAME C");
    heffSeparation_FCAL_nominal4_logd = (TH1D*)hmatchedSeparation_FCAL_nominal4_logd->Clone("heffSeparation_FCAL_nominal4_logd"); heffSeparation_FCAL_nominal4_logd->Divide(hthrownSeparation_FCAL_nominal4_logd);
    heffSeparation_FCAL_nominal4_logd->SetLineColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_nominal4_logd->SetMarkerColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_nominal4_logd->SetLineStyle(2);
    heffSeparation_FCAL_nominal4_logd->SetLineWidth(4);
    heffSeparation_FCAL_nominal4_logd->Draw("SAME C");

    legend1 = new TLegend(0.59,0.2,0.9,0.4);
    legend1->AddEntry(heffSeparation_FCAL_nominal,"20 MeV Threshold","l");
    legend1->AddEntry(heffSeparation_FCAL_nominal2,"15 MeV Threshold","l");
    legend1->AddEntry(heffSeparation_FCAL_nominal3,"10 MeV Threshold","l");
    legend1->AddEntry(heffSeparation_FCAL_nominal4,"  5 MeV Threshold","l");
    legend1->SetFillStyle(3001);
    legend1->Draw("SAME");

    legend2 = new TLegend(0.45,0.2,0.58,0.3);
    legend2->AddEntry(heffSeparation_FCAL_nominal,"COG","l");
    legend2->AddEntry(heffSeparation_FCAL_nominal_logd,"LOG","l");
    legend2->SetFillStyle(3001);
    legend2->Draw("SAME");

  c1->Update(); c1->Print(Form("/work/halld/beattite/island/%s_eff.pdf(",tag.data()),"Title:2g Efficiency in photon separation for various FCAL1 thresholds (Nominal)"); c1->Close();

  c1 = new TCanvas();
  c1->cd();
    heffSeparation_FCAL_island = (TH1D*)hmatchedSeparation_FCAL_island->Clone("heffSeparation_FCAL_island"); heffSeparation_FCAL_island->Divide(hthrownSeparation_FCAL_island);
    heffSeparation_FCAL_island->SetLineColorAlpha(kBlue,0.9);
    heffSeparation_FCAL_island->SetMarkerColorAlpha(kBlue,0.9);
    heffSeparation_FCAL_island->SetLineStyle(1);
    heffSeparation_FCAL_island->SetLineWidth(4);
    heffSeparation_FCAL_island->SetTitle("FCAL1 Diphoton Gun Efficiency vs. FCAL Separation (Island)");
    heffSeparation_FCAL_island->SetMaximum(1.05);
    heffSeparation_FCAL_island->Draw("C");
    heffSeparation_FCAL_island2 = (TH1D*)hmatchedSeparation_FCAL_island2->Clone("heffSeparation_FCAL_island2"); heffSeparation_FCAL_island2->Divide(hthrownSeparation_FCAL_island2);
    heffSeparation_FCAL_island2->SetLineColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_island2->SetMarkerColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_island2->SetLineStyle(1);
    heffSeparation_FCAL_island2->SetLineWidth(4);
    heffSeparation_FCAL_island2->Draw("SAME C");
    heffSeparation_FCAL_island3 = (TH1D*)hmatchedSeparation_FCAL_island3->Clone("heffSeparation_FCAL_island3"); heffSeparation_FCAL_island3->Divide(hthrownSeparation_FCAL_island3);
    heffSeparation_FCAL_island3->SetLineColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_island3->SetMarkerColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_island3->SetLineStyle(1);
    heffSeparation_FCAL_island3->SetLineWidth(4);
    heffSeparation_FCAL_island3->Draw("SAME C");
    heffSeparation_FCAL_island4 = (TH1D*)hmatchedSeparation_FCAL_island4->Clone("heffSeparation_FCAL_island4"); heffSeparation_FCAL_island4->Divide(hthrownSeparation_FCAL_island4);
    heffSeparation_FCAL_island4->SetLineColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_island4->SetMarkerColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_island4->SetLineStyle(1);
    heffSeparation_FCAL_island4->SetLineWidth(4);
    heffSeparation_FCAL_island4->Draw("SAME C");

    heffSeparation_FCAL_island_logd = (TH1D*)hmatchedSeparation_FCAL_island_logd->Clone("heffSeparation_FCAL_island_logd"); heffSeparation_FCAL_island_logd->Divide(hthrownSeparation_FCAL_island_logd);
    heffSeparation_FCAL_island_logd->SetLineColorAlpha(kBlue+1,0.9);
    heffSeparation_FCAL_island_logd->SetMarkerColorAlpha(kBlue+1,0.9);
    heffSeparation_FCAL_island_logd->SetLineStyle(2);
    heffSeparation_FCAL_island_logd->SetLineWidth(4);
    heffSeparation_FCAL_island_logd->Draw("SAME C");
    heffSeparation_FCAL_island2_logd = (TH1D*)hmatchedSeparation_FCAL_island2_logd->Clone("heffSeparation_FCAL_island2_logd"); heffSeparation_FCAL_island2_logd->Divide(hthrownSeparation_FCAL_island2_logd);
    heffSeparation_FCAL_island2_logd->SetLineColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_island2_logd->SetMarkerColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL_island2_logd->SetLineStyle(2);
    heffSeparation_FCAL_island2_logd->SetLineWidth(4);
    heffSeparation_FCAL_island2_logd->Draw("SAME C");
    heffSeparation_FCAL_island3_logd = (TH1D*)hmatchedSeparation_FCAL_island3_logd->Clone("heffSeparation_FCAL_island3_logd"); heffSeparation_FCAL_island3_logd->Divide(hthrownSeparation_FCAL_island3_logd);
    heffSeparation_FCAL_island3_logd->SetLineColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_island3_logd->SetMarkerColorAlpha(kViolet+1,0.9);
    heffSeparation_FCAL_island3_logd->SetLineStyle(2);
    heffSeparation_FCAL_island3_logd->SetLineWidth(4);
    heffSeparation_FCAL_island3_logd->Draw("SAME C");
    heffSeparation_FCAL_island4_logd = (TH1D*)hmatchedSeparation_FCAL_island4_logd->Clone("heffSeparation_FCAL_island4_logd"); heffSeparation_FCAL_island4_logd->Divide(hthrownSeparation_FCAL_island4_logd);
    heffSeparation_FCAL_island4_logd->SetLineColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_island4_logd->SetMarkerColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL_island4_logd->SetLineStyle(2);
    heffSeparation_FCAL_island4_logd->SetLineWidth(4);
    heffSeparation_FCAL_island4_logd->Draw("SAME C");

    legend1->Draw("SAME");

    legend2->Draw("SAME");

  c1->Update(); c1->Print(Form("/work/halld/beattite/island/%s_eff.pdf)",tag.data()),"Title:2g Efficiency in photon separation for various FCAL1 thresholds (Island)"); c1->Close();
}
