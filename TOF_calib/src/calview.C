#include <fstream>
#include <iostream>

using namespace std;

#include <TFile.h>
#include <TH1.h>
#include <TGraphErrors.h>

double DATA[176][1000];
double X[1000];

int DEBUG = 1;

double Speed[88];

void calview(int PAR){


  TCanvas *c1 = new TCanvas("c1", "new canvas",1000,800);

  char inf[128];
  RANGE =0;
  if (PAR == 1){
    sprintf(inf,"tofcalib_timing_parms.dat");
    RANGE = 176;
  } else if (PAR == 2){
    sprintf(inf,"tofcalib_speed_parms.dat");
    RANGE = 88;
  } else {
    cout<<"Error not such data! PAR="<<PAR<<endl;
    return;
  }

  ifstream INF(inf);
  int runnum;
  int cnt = 0;
  while (!INF.eof()){
    
    INF>>runnum;

    X[cnt] = runnum;
    for (int k=0;k<RANGE;k++){
      INF>>DATA[k][cnt];
    }
    cnt++;
    
  }
  INF.close();
  cnt--;
  
  TH1D *hmean[176];            // distribution of parameters over all runs
  TGraphErrors *graphs[176];   // parameters as function of runs

  for (int PMT=0; PMT<RANGE; PMT++) {
    
    char htit[128];
    if  (PAR==1){
      sprintf(htit,"PMT #%d timing offset",PMT+1);
    } else {
      sprintf(htit,"Paddle #%d propagation speed",PMT+1);
    }
    char hnam[128];
    sprintf(hnam,"hmean%d",PMT+1);
    if (PAR==1){
      hmean[PMT] = new TH1D(hnam, htit, 800, -2., 2.);
      hmean[PMT]->GetXaxis()->SetTitle("timing offset [ns]");
    } else {
      hmean[PMT] = new TH1D(hnam, htit, 500, 15., 17.0);
    }
    
    for (int k=0;k<cnt;k++){
      hmean[PMT]->Fill(DATA[PMT][k]);
    }
    
    
    graphs[PMT] = new TGraphErrors(cnt,X,DATA[PMT],NULL,NULL);
    if (PAR==1){
      graphs[PMT]->SetTitle("timing offset as function of run number"); 
      graphs[PMT]->GetXaxis()->SetTitle("run number [#]");
      graphs[PMT]->GetYaxis()->SetTitle("timing offset [ns]");
    } else {
      graphs[PMT]->SetTitle("effective velocity as function of run number"); 
      graphs[PMT]->GetXaxis()->SetTitle("run number [#]");
      graphs[PMT]->GetYaxis()->SetTitle("velocity [cm/ns]");
    }

  }

  TFile *RF;

  if (PAR == 1) { 
    RF = new TFile("tofcalib_timeres.root","RECREATE");
  } else {
    RF = new TFile("tofcalib_velores.root","RECREATE");
  }

  for (int PMT=0; PMT<RANGE; PMT++) {
    
    hmean[PMT]->Write();
    graphs[PMT]->Write();
    
  }
  RF->Close();


  double Velocities[88];

  for (int k=0; k<44; k++) {
    
    int PMT1 = k;
    int PMT2 = k+44;
    
    c1->Clear();
    c1->Divide(2,2);
    
    c1->cd(1);
    if (PAR==2){
      if (hmean[PMT1]->GetEntries()>10) {
	double max = hmean[PMT1]->GetBinCenter(hmean[PMT1]->GetMaximumBin());
	hmean[PMT1]->Fit("gaus","","R",max-0.03,max+0.03);
	TF1 *f1 = hmean[PMT1]->GetFunction("gaus");
	double sig = f1->GetParameter(2);
	if (sig>0.05) {
	  hmean[PMT1]->Fit("gaus","","R",max-0.05,max+0.05);
	}
	hmean[PMT1]->GetXaxis()->SetRangeUser(max-0.2,max+0.2);
	gStyle->SetOptFit(1);
	f1 = hmean[PMT1]->GetFunction("gaus");
	Velocities[PMT1] = f1->GetParameter(1); 
      }
      hmean[PMT1]->Draw();
      Speed[PMT1] = hmean[PMT1]->GetMean();
    } else {
      double max = hmean[PMT1]->GetBinCenter(hmean[PMT1]->GetMaximumBin());
      hmean[PMT1]->GetXaxis()->SetRangeUser(max-0.5,max+0.5);
      hmean[PMT1]->Draw();
    }
    gPad->SetGrid();
    c1->cd(2);
    graphs[PMT1]->SetMarkerColor(4);
    graphs[PMT1]->SetMarkerStyle(21); 
    graphs[PMT1]->Draw("AP");
    if (PAR==1){
      graphs[PMT1]->GetYaxis()->SetRangeUser(-2.,2.);
    } else {
      graphs[PMT1]->GetYaxis()->SetRangeUser(15.,17.);
    }
    gPad->SetGrid();
    gPad->Update();
    
    c1->cd(3);
    if (PAR==2){
      if (hmean[PMT2]->GetEntries()>10) {
	double max = hmean[PMT2]->GetBinCenter(hmean[PMT2]->GetMaximumBin());
	hmean[PMT2]->Fit("gaus","","R",max-0.03,max+0.03);
	TF1 *f1 = hmean[PMT2]->GetFunction("gaus");
	double sig = f1->GetParameter(2);
	if (sig>0.05) {
	  hmean[PMT2]->Fit("gaus","","R",max-0.05,max+0.05);
	}
	hmean[PMT2]->GetXaxis()->SetRangeUser(max-0.2,max+0.2);
	gStyle->SetOptFit(1);
	f1 = hmean[PMT2]->GetFunction("gaus");
	Velocities[PMT2] = f1->GetParameter(1); 
      }
      hmean[PMT2]->Draw(); 
      Speed[PMT2] = hmean[PMT1]->GetMean();

    } else {
      double max = hmean[PMT2]->GetBinCenter(hmean[PMT2]->GetMaximumBin());
      hmean[PMT2]->GetXaxis()->SetRangeUser(max-0.5,max+0.5);
      hmean[PMT2]->Draw();
    }
    gPad->SetGrid();
    c1->cd(4);
    graphs[PMT2]->SetMarkerColor(4);
    graphs[PMT2]->SetMarkerStyle(21); 
    graphs[PMT2]->Draw("AP");
    if (PAR==1){
      graphs[PMT2]->GetYaxis()->SetRangeUser(-2.,2.);
    } else {
      graphs[PMT2]->GetYaxis()->SetRangeUser(15.,17.);
    }
    gPad->SetGrid();
    gPad->Update();

    char outfp[128];
    if (PAR==1){
      sprintf(outfp, "figures/pmt%03d_pmt%03d_offsets.pdf",PMT1,PMT2);
    } else {
      sprintf(outfp, "figures/paddle%03d_paddle%03d_velocities.pdf",PMT1,PMT2);
    }
    c1->SaveAs(outfp);
    
    if (DEBUG>1)
      getchar();
    
    if (PAR == 1) {
      PMT1 += 88;
      PMT2 += 88;
      
      sprintf(outfp, "figures/pmt%03d_pmt%03d_offsets.pdf",PMT1,PMT2);

      c1->Clear();
      c1->Divide(2,2);
      
      c1->cd(1);
      double max = hmean[PMT1]->GetBinCenter(hmean[PMT1]->GetMaximumBin());
      hmean[PMT1]->GetXaxis()->SetRangeUser(max-0.5,max+0.5);
      hmean[PMT1]->Draw();
      gPad->SetGrid();
      c1->cd(2);
      graphs[PMT1]->SetMarkerColor(4);
      graphs[PMT1]->SetMarkerStyle(21); 
      graphs[PMT1]->Draw("AP");
      if (PAR==1){
	graphs[PMT1]->GetYaxis()->SetRangeUser(-2.,2.);
      } else {
	graphs[PMT1]->GetYaxis()->SetRangeUser(15.,17.);
      }
      gPad->SetGrid();
      gPad->Update();
      
      c1->cd(3);
      max = hmean[PMT2]->GetBinCenter(hmean[PMT2]->GetMaximumBin());
      hmean[PMT2]->GetXaxis()->SetRangeUser(max-0.5,max+0.5);
      hmean[PMT2]->Draw();
      gPad->SetGrid();
      c1->cd(4);
      graphs[PMT2]->SetMarkerColor(4);
      graphs[PMT2]->SetMarkerStyle(21); 
      graphs[PMT2]->Draw("AP");
      graphs[PMT2]->GetYaxis()->SetRangeUser(-2.,2.);
      
      gPad->SetGrid();
      gPad->Update();
      
      if (DEBUG>1)
	getchar();
    }
    c1->SaveAs(outfp);

  }
  
  if (PAR == 2){
    ofstream OF("paddle_velocities.dat");
    for (int k=0;k<88;k++){
      OF<<Velocities[k]<<"        "<<Speed[k]<<endl;
    }
    OF.close();
  }
  
  
}
