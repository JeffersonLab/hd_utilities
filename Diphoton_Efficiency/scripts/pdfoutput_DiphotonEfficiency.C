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

void pdfoutput_DiphotonEfficiency(string tag = "TEMP") {

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

  TH1D *heffSeparation_FCAL1_island, *hthrownSeparation_FCAL1_island, *hmatchedSeparation_FCAL1_island;
  TH1D *heffSeparation_FCAL1_nominal, *hthrownSeparation_FCAL1_nominal, *hmatchedSeparation_FCAL1_nominal;
  TH1D *heffSeparation_FCAL1_island_logd, *hthrownSeparation_FCAL1_island_logd, *hmatchedSeparation_FCAL1_island_logd;
  TH1D *heffSeparation_FCAL1_nominal_logd, *hthrownSeparation_FCAL1_nominal_logd, *hmatchedSeparation_FCAL1_nominal_logd;
  TH1D *heffSeparation_FCAL2_island, *hthrownSeparation_FCAL2_island, *hmatchedSeparation_FCAL2_island;
  TH1D *heffSeparation_FCAL2_nominal, *hthrownSeparation_FCAL2_nominal, *hmatchedSeparation_FCAL2_nominal;
  TH1D *heffSeparation_FCAL2_island_logd, *hthrownSeparation_FCAL2_island_logd, *hmatchedSeparation_FCAL2_island_logd;
  TH1D *heffSeparation_FCAL2_nominal_logd, *hthrownSeparation_FCAL2_nominal_logd, *hmatchedSeparation_FCAL2_nominal_logd;
  TH1D *heffSeparation_CCAL, *hthrownSeparation_CCAL, *hmatchedSeparation_CCAL;
  TH1D *htemp;
  TCanvas *c1;
  TLegend *legend1, *legend2, *legend3;

  TFile *file_Separation_FCAL1_island = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_61936_island/root/merged.root");
  TFile *file_Separation_FCAL1_nominal = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_61936_nominal/root/merged.root");
  TFile *file_Separation_FCAL2_island = TFile::Open("/work/halld/beattite/island/diphoton_FCAL2_100000_island/root/merged.root");
  TFile *file_Separation_FCAL2_nominal = TFile::Open("/work/halld/beattite/island/diphoton_FCAL2_100000_nominal/root/merged.root");
  TFile *file_Separation_CCAL = TFile::Open("/work/halld/beattite/island/diphoton_CCAL_igal2/root/merged.root");

  hthrownSeparation_FCAL1_island = (TH1D*)file_Separation_FCAL1_island->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL1_island = (TH1D*)file_Separation_FCAL1_island->Get("h_matchedSeparation");
  hthrownSeparation_FCAL1_nominal = (TH1D*)file_Separation_FCAL1_nominal->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL1_nominal = (TH1D*)file_Separation_FCAL1_nominal->Get("h_matchedSeparation");
  hthrownSeparation_FCAL1_island_logd = (TH1D*)file_Separation_FCAL1_island->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL1_island_logd = (TH1D*)file_Separation_FCAL1_island->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL1_nominal_logd = (TH1D*)file_Separation_FCAL1_nominal->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL1_nominal_logd = (TH1D*)file_Separation_FCAL1_nominal->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL2_island = (TH1D*)file_Separation_FCAL2_island->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL2_island = (TH1D*)file_Separation_FCAL2_island->Get("h_matchedSeparation");
  hthrownSeparation_FCAL2_nominal = (TH1D*)file_Separation_FCAL2_nominal->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL2_nominal = (TH1D*)file_Separation_FCAL2_nominal->Get("h_matchedSeparation");
  hthrownSeparation_FCAL2_island_logd = (TH1D*)file_Separation_FCAL2_island->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL2_island_logd = (TH1D*)file_Separation_FCAL2_island->Get("h_matchedSeparation_logd");
  hthrownSeparation_FCAL2_nominal_logd = (TH1D*)file_Separation_FCAL2_nominal->Get("h_thrownSeparation");
  hmatchedSeparation_FCAL2_nominal_logd = (TH1D*)file_Separation_FCAL2_nominal->Get("h_matchedSeparation_logd");
  hthrownSeparation_CCAL = (TH1D*)file_Separation_CCAL->Get("h_thrownSeparation");
  hmatchedSeparation_CCAL = (TH1D*)file_Separation_CCAL->Get("h_matchedSeparation");
  hthrownSeparation_FCAL1_island->Rebin(20);
  hmatchedSeparation_FCAL1_island->Rebin(20);
  hthrownSeparation_FCAL1_nominal->Rebin(20);
  hmatchedSeparation_FCAL1_nominal->Rebin(20);
//  hthrownSeparation_FCAL1_island_logd->Rebin(20);
  hmatchedSeparation_FCAL1_island_logd->Rebin(20);
//  hthrownSeparation_FCAL1_nominal_logd->Rebin(20);
  hmatchedSeparation_FCAL1_nominal_logd->Rebin(20);
  hthrownSeparation_FCAL2_island->Rebin(20);
  hmatchedSeparation_FCAL2_island->Rebin(20);
  hthrownSeparation_FCAL2_nominal->Rebin(20);
  hmatchedSeparation_FCAL2_nominal->Rebin(20);
//  hthrownSeparation_FCAL2_island_logd->Rebin(20);
  hmatchedSeparation_FCAL2_island_logd->Rebin(20);
//  hthrownSeparation_FCAL2_nominal_logd->Rebin(20);
  hmatchedSeparation_FCAL2_nominal_logd->Rebin(20);
  hthrownSeparation_CCAL->Rebin(20);
  hmatchedSeparation_CCAL->Rebin(20);

  c1 = new TCanvas();
  c1->cd();
    heffSeparation_FCAL1_island = (TH1D*)hmatchedSeparation_FCAL1_island->Clone("heffSeparation_FCAL1_island"); heffSeparation_FCAL1_island->Divide(hthrownSeparation_FCAL1_island);
    heffSeparation_FCAL1_island->SetLineColorAlpha(kBlue,0.9);
    heffSeparation_FCAL1_island->SetMarkerColorAlpha(kBlue,0.9);
    heffSeparation_FCAL1_island->SetLineStyle(2);
    heffSeparation_FCAL1_island->SetLineWidth(4);
    heffSeparation_FCAL1_island->SetTitle("Diphoton Gun Efficiency vs. FCAL Separation");
    heffSeparation_FCAL1_island->SetMaximum(1.05);
    heffSeparation_FCAL1_island->Draw("C");
    heffSeparation_FCAL1_nominal = (TH1D*)hmatchedSeparation_FCAL1_nominal->Clone("heffSeparation_FCAL1_nominal"); heffSeparation_FCAL1_nominal->Divide(hthrownSeparation_FCAL1_nominal);
    heffSeparation_FCAL1_nominal->SetLineColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL1_nominal->SetMarkerColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL1_nominal->SetLineStyle(2);
    heffSeparation_FCAL1_nominal->SetLineWidth(4);
    heffSeparation_FCAL1_nominal->Draw("SAME C");
    heffSeparation_FCAL1_island_logd = (TH1D*)hmatchedSeparation_FCAL1_island_logd->Clone("heffSeparation_FCAL1_island_logd"); heffSeparation_FCAL1_island_logd->Divide(hthrownSeparation_FCAL1_island_logd);
    heffSeparation_FCAL1_island_logd->SetLineColorAlpha(kViolet,0.9);
    heffSeparation_FCAL1_island_logd->SetMarkerColorAlpha(kViolet,0.9);
    heffSeparation_FCAL1_island_logd->SetLineStyle(2);
    heffSeparation_FCAL1_island_logd->SetLineWidth(4);
    heffSeparation_FCAL1_island_logd->Draw("SAME C");
    heffSeparation_FCAL1_nominal_logd = (TH1D*)hmatchedSeparation_FCAL1_nominal_logd->Clone("heffSeparation_FCAL1_nominal_logd"); heffSeparation_FCAL1_nominal_logd->Divide(hthrownSeparation_FCAL1_nominal_logd);
    heffSeparation_FCAL1_nominal_logd->SetLineColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL1_nominal_logd->SetMarkerColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL1_nominal_logd->SetLineStyle(2);
    heffSeparation_FCAL1_nominal_logd->SetLineWidth(4);
    heffSeparation_FCAL1_nominal_logd->Draw("SAME C");

    heffSeparation_FCAL2_island = (TH1D*)hmatchedSeparation_FCAL2_island->Clone("heffSeparation_FCAL2_island"); heffSeparation_FCAL2_island->Divide(hthrownSeparation_FCAL2_island);
    heffSeparation_FCAL2_island->SetLineColorAlpha(kBlue,0.9);
    heffSeparation_FCAL2_island->SetMarkerColorAlpha(kBlue,0.9);
    heffSeparation_FCAL2_island->SetLineStyle(1);
    heffSeparation_FCAL2_island->SetLineWidth(4);
    heffSeparation_FCAL2_island->Draw("SAME C");
    heffSeparation_FCAL2_nominal = (TH1D*)hmatchedSeparation_FCAL2_nominal->Clone("heffSeparation_FCAL2_nominal"); heffSeparation_FCAL2_nominal->Divide(hthrownSeparation_FCAL2_nominal);
    heffSeparation_FCAL2_nominal->SetLineColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL2_nominal->SetMarkerColorAlpha(kRed+1,0.9);
    heffSeparation_FCAL2_nominal->SetLineStyle(1);
    heffSeparation_FCAL2_nominal->SetLineWidth(4);
    heffSeparation_FCAL2_nominal->Draw("SAME C");
    heffSeparation_FCAL2_island_logd = (TH1D*)hmatchedSeparation_FCAL2_island_logd->Clone("heffSeparation_FCAL2_island_logd"); heffSeparation_FCAL2_island_logd->Divide(hthrownSeparation_FCAL2_island_logd);
    heffSeparation_FCAL2_island_logd->SetLineColorAlpha(kViolet,0.9);
    heffSeparation_FCAL2_island_logd->SetMarkerColorAlpha(kViolet,0.9);
    heffSeparation_FCAL2_island_logd->SetLineStyle(1);
    heffSeparation_FCAL2_island_logd->SetLineWidth(4);
    heffSeparation_FCAL2_island_logd->Draw("SAME C");
    heffSeparation_FCAL2_nominal_logd = (TH1D*)hmatchedSeparation_FCAL2_nominal_logd->Clone("heffSeparation_FCAL2_nominal_logd"); heffSeparation_FCAL2_nominal_logd->Divide(hthrownSeparation_FCAL2_nominal_logd);
    heffSeparation_FCAL2_nominal_logd->SetLineColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL2_nominal_logd->SetMarkerColorAlpha(kOrange+1,0.9);
    heffSeparation_FCAL2_nominal_logd->SetLineStyle(1);
    heffSeparation_FCAL2_nominal_logd->SetLineWidth(4);
    heffSeparation_FCAL2_nominal_logd->Draw("SAME C");

    heffSeparation_CCAL = (TH1D*)hmatchedSeparation_CCAL->Clone("heffSeparation_CCAL"); heffSeparation_CCAL->Divide(hthrownSeparation_CCAL);
    heffSeparation_CCAL->SetLineColorAlpha(kGreen+1,0.9);
    heffSeparation_CCAL->SetMarkerColorAlpha(kGreen+1,0.9);
    heffSeparation_CCAL->SetLineStyle(1);
    heffSeparation_CCAL->SetLineWidth(4);
    heffSeparation_CCAL->Draw("SAME C");

    legend1 = new TLegend(0.63,0.2,0.9,0.4);
    legend1->AddEntry(heffSeparation_FCAL2_island,"Island","l");
    legend1->AddEntry(heffSeparation_FCAL2_island_logd,"Island (Log)","l");
    legend1->AddEntry(heffSeparation_FCAL2_nominal,"Nominal","l");
    legend1->AddEntry(heffSeparation_FCAL2_nominal_logd,"Nominal (Log)","l");
    legend1->SetFillStyle(3001);
    legend1->Draw("SAME");

    legend2 = new TLegend(0.45,0.2,0.62,0.35);
    legend2->AddEntry(heffSeparation_FCAL1_island,"FCAL1","l");
    legend2->AddEntry(heffSeparation_FCAL2_island,"FCAL2","l");
    legend2->AddEntry(heffSeparation_CCAL,"CCAL","l");
    legend2->SetFillStyle(3001);
    legend2->Draw("SAME");

  c1->Update(); c1->Print(Form("/work/halld/beattite/island/%s_eff.pdf",tag.data()),"Title:2g Efficiency in photon separation"); c1->Close();
}
