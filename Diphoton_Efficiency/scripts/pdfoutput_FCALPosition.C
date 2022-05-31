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

void pdfoutput_FCALPosition(string tag = "TEMP") {

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

  TH2D *hdeltax_thrownx_nominal, *hdeltax_matchedx_nominal, *hdeltay_throwny_nominal, *hdeltay_matchedy_nominal;
  TH2D *hdeltax_thrownx_nominal_logd, *hdeltax_matchedx_nominal_logd, *hdeltay_throwny_nominal_logd, *hdeltay_matchedy_nominal_logd;
  TH2D *hdeltax_thrownx_island, *hdeltax_matchedx_island, *hdeltay_throwny_island, *hdeltay_matchedy_island;
  TH2D *hdeltax_thrownx_island_logd, *hdeltax_matchedx_island_logd, *hdeltay_throwny_island_logd, *hdeltay_matchedy_island_logd;
  TH1D *hdeltax_nominal, *hdeltay_nominal;
  TH1D *hdeltax_nominal_logd, *hdeltay_nominal_logd;
  TH1D *hdeltax_island, *hdeltay_island;
  TH1D *hdeltax_island_logd, *hdeltay_island_logd;
  TH1D *hdeltaPosition_nominal, *hdeltaPosition_nominal_logd;
  TH1D *hdeltaPosition_island, *hdeltaPosition_island_logd;
  TCanvas *c1;
  TLatex *label_nominal = new TLatex(0.5,0.5,"Nominal");
  TLatex *label_island = new TLatex(0.5,0.5,"Island");

  TFile *file_Separation_FCAL_island = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111113_island_single/root/merged.root");
  TFile *file_Separation_FCAL_nominal = TFile::Open("/work/halld/beattite/island/diphoton_FCAL1_111113_nominal_single/root/merged.root");

  hdeltax_thrownx_nominal = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltax_thrownx");
  hdeltax_matchedx_nominal = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltax_matchedx");
  hdeltay_throwny_nominal = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltay_throwny");
  hdeltay_matchedy_nominal = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltay_matchedy");
  hdeltax_thrownx_nominal_logd = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltax_thrownx_logd");
  hdeltax_matchedx_nominal_logd = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltax_matchedx_logd");
  hdeltay_throwny_nominal_logd = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltay_throwny_logd");
  hdeltay_matchedy_nominal_logd = (TH2D*)file_Separation_FCAL_nominal->Get("h_deltay_matchedy_logd");
  hdeltax_thrownx_island = (TH2D*)file_Separation_FCAL_island->Get("h_deltax_thrownx");
  hdeltax_matchedx_island = (TH2D*)file_Separation_FCAL_island->Get("h_deltax_matchedx");
  hdeltay_throwny_island = (TH2D*)file_Separation_FCAL_island->Get("h_deltay_throwny");
  hdeltay_matchedy_island = (TH2D*)file_Separation_FCAL_island->Get("h_deltay_matchedy");
  hdeltax_thrownx_island_logd = (TH2D*)file_Separation_FCAL_island->Get("h_deltax_thrownx_logd");
  hdeltax_matchedx_island_logd = (TH2D*)file_Separation_FCAL_island->Get("h_deltax_matchedx_logd");
  hdeltay_throwny_island_logd = (TH2D*)file_Separation_FCAL_island->Get("h_deltay_throwny_logd");
  hdeltay_matchedy_island_logd = (TH2D*)file_Separation_FCAL_island->Get("h_deltay_matchedy_logd");
  hdeltaPosition_nominal = (TH1D*)file_Separation_FCAL_nominal->Get("h_deltaPosition");
  hdeltaPosition_nominal_logd = (TH1D*)file_Separation_FCAL_nominal->Get("h_deltaPosition_logd");
  hdeltaPosition_island = (TH1D*)file_Separation_FCAL_island->Get("h_deltaPosition");
  hdeltaPosition_island_logd = (TH1D*)file_Separation_FCAL_island->Get("h_deltaPosition_logd");
  hdeltax_nominal = (TH1D*)file_Separation_FCAL_nominal->Get("h_deltax");
  hdeltay_nominal = (TH1D*)file_Separation_FCAL_nominal->Get("h_deltay");
  hdeltax_nominal_logd = (TH1D*)file_Separation_FCAL_nominal->Get("h_deltax_logd");
  hdeltay_nominal_logd = (TH1D*)file_Separation_FCAL_nominal->Get("h_deltay_logd");
  hdeltax_island = (TH1D*)file_Separation_FCAL_island->Get("h_deltax");
  hdeltay_island = (TH1D*)file_Separation_FCAL_island->Get("h_deltay");
  hdeltax_island_logd = (TH1D*)file_Separation_FCAL_island->Get("h_deltax_logd");
  hdeltay_island_logd = (TH1D*)file_Separation_FCAL_island->Get("h_deltay_logd");

  c1 = new TCanvas();
  c1->Divide(1,2);
  c1->cd(1);
    hdeltaPosition_nominal->Draw("P");
  c1->cd(2);
    hdeltaPosition_nominal_logd->Draw("P");
  c1->cd(0);
    label_nominal->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf(",tag.data()),"Title:Thrown to Reconstructed Distance (Nominal)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(1,2);
  c1->cd(1);
    hdeltaPosition_island->Draw("P");
  c1->cd(2);
    hdeltaPosition_island_logd->Draw("P");
  c1->cd(0);
    label_island->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf",tag.data()),"Title:Thrown to Reconstructed Distance (Island)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
    hdeltax_nominal->Draw("P");
  c1->cd(2);
    hdeltay_nominal->Draw("P");
  c1->cd(3);
    hdeltax_nominal_logd->Draw("P");
  c1->cd(4);
    hdeltay_nominal_logd->Draw("P");
  c1->cd(0);
    label_nominal->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf",tag.data()),"Title:X- and Y-Positions (Nominal)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
    hdeltax_island->Draw("P");
  c1->cd(2);
    hdeltay_island->Draw("P");
  c1->cd(3);
    hdeltax_island_logd->Draw("P");
  c1->cd(4);
    hdeltay_island_logd->Draw("P");
  c1->cd(0);
    label_island->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf",tag.data()),"Title:X- and Y-Positions (Island)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
    hdeltax_thrownx_nominal->Draw("COLZ");
  c1->cd(2);
    hdeltax_matchedx_nominal->Draw("COLZ");
  c1->cd(3);
    hdeltax_thrownx_nominal_logd->Draw("COLZ");
  c1->cd(4);
    hdeltax_matchedx_nominal_logd->Draw("COLZ");
  c1->cd(0);
    label_nominal->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf",tag.data()),"Title:X-Position on Block (Nominal)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
    hdeltax_thrownx_island->Draw("COLZ");
  c1->cd(2);
    hdeltax_matchedx_island->Draw("COLZ");
  c1->cd(3);
    hdeltax_thrownx_island_logd->Draw("COLZ");
  c1->cd(4);
    hdeltax_matchedx_island_logd->Draw("COLZ");
  c1->cd(0);
    label_island->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf",tag.data()),"Title:X-Position on Block (Island)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
    hdeltay_throwny_nominal->Draw("COLZ");
  c1->cd(2);
    hdeltay_matchedy_nominal->Draw("COLZ");
  c1->cd(3);
    hdeltay_throwny_nominal_logd->Draw("COLZ");
  c1->cd(4);
    hdeltay_matchedy_nominal_logd->Draw("COLZ");
  c1->cd(0);
    label_nominal->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf",tag.data()),"Title:Y-Position on Block (Nominal)"); c1->Close();

  c1 = new TCanvas();
  c1->Divide(2,2);
  c1->cd(1);
    hdeltay_throwny_island->Draw("COLZ");
  c1->cd(2);
    hdeltay_matchedy_island->Draw("COLZ");
  c1->cd(3);
    hdeltay_throwny_island_logd->Draw("COLZ");
  c1->cd(4);
    hdeltay_matchedy_island_logd->Draw("COLZ");
  c1->cd(0);
    label_island->Draw("SAME");
  c1->Print(Form("/work/halld/beattite/island/%s_position.pdf)",tag.data()),"Title:Y-Position on Block (Island)"); c1->Close();
}
