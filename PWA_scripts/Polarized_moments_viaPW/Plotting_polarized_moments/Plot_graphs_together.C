#include <iostream>
#include <TF1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TLine.h>
#include <TGaxis.h>
#include <TMultiGraph.h>



void Plot_graphs_together(){


  TFile *file_in_orig,*file_in_one;

  file_in_orig=new TFile("Drawing_moments_origpar.root");
  file_in_one=new TFile("Drawing_moments.root");

  TGraphErrors *gr_H1_00_orig= (TGraphErrors *)file_in_orig->Get("H1_00");
  TGraphErrors *gr_H0_00_orig= (TGraphErrors *)file_in_orig->Get("H0_00");
  TGraphErrors *gr_H1_10_orig= (TGraphErrors *)file_in_orig->Get("H1_10");
  TGraphErrors *gr_H0_10_orig= (TGraphErrors *)file_in_orig->Get("H0_10");
  TGraphErrors *gr_H1_11_orig= (TGraphErrors *)file_in_orig->Get("H1_11");
  TGraphErrors *gr_H0_11_orig= (TGraphErrors *)file_in_orig->Get("H0_11");
  TGraphErrors *gr_H1_20_orig= (TGraphErrors *)file_in_orig->Get("H1_20");
  TGraphErrors *gr_H0_20_orig= (TGraphErrors *)file_in_orig->Get("H0_20");
  TGraphErrors *gr_H1_21_orig= (TGraphErrors *)file_in_orig->Get("H1_21");
  TGraphErrors *gr_H0_21_orig= (TGraphErrors *)file_in_orig->Get("H0_21");
  TGraphErrors *gr_H0_22_orig= (TGraphErrors *)file_in_orig->Get("H0_22");
  TGraphErrors *gr_H1_22_orig= (TGraphErrors *)file_in_orig->Get("H1_22");
 


  TGraphErrors *gr_H1_00_one= (TGraphErrors *)file_in_one->Get("H1_00");
  TGraphErrors *gr_H0_00_one= (TGraphErrors *)file_in_one->Get("H0_00");
  TGraphErrors *gr_H1_10_one= (TGraphErrors *)file_in_one->Get("H1_10");
  TGraphErrors *gr_H0_10_one= (TGraphErrors *)file_in_one->Get("H0_10");
  TGraphErrors *gr_H1_11_one= (TGraphErrors *)file_in_one->Get("H1_11");
  TGraphErrors *gr_H0_11_one= (TGraphErrors *)file_in_one->Get("H0_11");
  TGraphErrors *gr_H1_20_one= (TGraphErrors *)file_in_one->Get("H1_20");
  TGraphErrors *gr_H0_20_one= (TGraphErrors *)file_in_one->Get("H0_20");
  TGraphErrors *gr_H1_21_one= (TGraphErrors *)file_in_one->Get("H1_21");
  TGraphErrors *gr_H0_21_one= (TGraphErrors *)file_in_one->Get("H0_21");
  TGraphErrors *gr_H0_22_one= (TGraphErrors *)file_in_one->Get("H0_22");
  TGraphErrors *gr_H1_22_one= (TGraphErrors *)file_in_one->Get("H1_22");


  //gr_H1_22_one->SetLineColor(4);
  //gr_H1_22_one->SetMarkerStyle(23);



  TMultiGraph *mg_H1_00=new TMultiGraph();
  gr_H1_00_orig->SetMarkerColor(2);
  mg_H1_00->Add(gr_H1_00_orig,"1p"); //points
  mg_H1_00->Add(gr_H1_00_one,"c");   //line

  TMultiGraph *mg_H0_00=new TMultiGraph();
  gr_H0_00_orig->SetMarkerColor(2);
  mg_H0_00->Add(gr_H0_00_orig,"1p"); //points
  mg_H0_00->Add(gr_H0_00_one,"c");   //line

  TMultiGraph *mg_H1_10=new TMultiGraph();
  gr_H1_10_orig->SetMarkerColor(2);
  mg_H1_10->Add(gr_H1_10_orig,"1p"); //points
  mg_H1_10->Add(gr_H1_10_one,"c");   //line

  TMultiGraph *mg_H0_10=new TMultiGraph();
  gr_H0_10_orig->SetMarkerColor(2);
  mg_H0_10->Add(gr_H0_10_orig,"1p"); //points
  mg_H0_10->Add(gr_H0_10_one,"c");   //line

  TMultiGraph *mg_H1_11=new TMultiGraph();
  gr_H1_11_orig->SetMarkerColor(2);
  mg_H1_11->Add(gr_H1_11_orig,"1p"); //points
  mg_H1_11->Add(gr_H1_11_one,"c");   //line

  TMultiGraph *mg_H0_11=new TMultiGraph();
  gr_H0_11_orig->SetMarkerColor(2);
  mg_H0_11->Add(gr_H0_11_orig,"1p"); //points
  mg_H0_11->Add(gr_H0_11_one,"c");   //line

  TMultiGraph *mg_H1_20=new TMultiGraph();
  gr_H1_20_orig->SetMarkerColor(2);
  mg_H1_20->Add(gr_H1_20_orig,"1p"); //points
  mg_H1_20->Add(gr_H1_20_one,"c");   //line

  TMultiGraph *mg_H0_20=new TMultiGraph();
  gr_H0_20_orig->SetMarkerColor(2);
  mg_H0_20->Add(gr_H0_20_orig,"1p"); //points
  mg_H0_20->Add(gr_H0_20_one,"c");   //line

  TMultiGraph *mg_H1_21=new TMultiGraph();
  gr_H1_21_orig->SetMarkerColor(2);
  mg_H1_21->Add(gr_H1_21_orig,"1p"); //points
  mg_H1_21->Add(gr_H1_21_one,"c");   //line

  TMultiGraph *mg_H0_21=new TMultiGraph();
  gr_H0_21_orig->SetMarkerColor(2);
  mg_H0_21->Add(gr_H0_21_orig,"1p"); //points
  mg_H0_21->Add(gr_H0_21_one,"c");   //line

  TMultiGraph *mg_H0_22=new TMultiGraph();
  gr_H0_22_orig->SetMarkerColor(2);
  mg_H0_22->Add(gr_H0_22_orig,"1p"); //points
  mg_H0_22->Add(gr_H0_22_one,"c");   //line

  TMultiGraph *mg_H1_22=new TMultiGraph();
  gr_H1_22_orig->SetMarkerColor(2);
  mg_H1_22->Add(gr_H1_22_orig,"1p"); //points
  mg_H1_22->Add(gr_H1_22_one,"c");   //line

  TCanvas *c1=new TCanvas();

  mg_H1_00->Draw("a");
  c1->Print("Plots/H1_00_comp.pdf");

  mg_H0_00->Draw("a");
  c1->Print("Plots/H0_00_comp.pdf");

  mg_H1_10->Draw("a");
  c1->Print("Plots/H1_10_comp.pdf");

  mg_H0_10->Draw("a");
  c1->Print("Plots/H0_10_comp.pdf");

  mg_H1_11->Draw("a");
  c1->Print("Plots/H1_11_comp.pdf");

  mg_H0_11->Draw("a");
  c1->Print("Plots/H0_11_comp.pdf");

  mg_H1_20->Draw("a");
  c1->Print("Plots/H1_20_comp.pdf");

  mg_H0_20->Draw("a");
  c1->Print("Plots/H0_20_comp.pdf");

  mg_H1_21->Draw("a");
  c1->Print("Plots/H1_21_comp.pdf");

  mg_H0_21->Draw("a");
  c1->Print("Plots/H0_21_comp.pdf");

  mg_H0_22->Draw("a");
  c1->Print("Plots/H0_22_comp.pdf");

  mg_H1_22->Draw("a");
  c1->Print("Plots/H1_22_comp.pdf");


}
