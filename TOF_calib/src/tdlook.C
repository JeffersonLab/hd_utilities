//
// tdlook.C: use time difference data from fits and determine 
//           effective velocity in scintillators for all paddles
//
//
//
//
//


#include "TROOT.h"
#include "TStyle.h"
#include "TMath.h"
#include "TPad.h"
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TProfile.h>
#include <fstream>
#include <iostream>

using namespace std;

int DEBUG = 2;
int RunNumber;

void gettime(int , int , double &, double &, double &, double &);

void tdlook(int R, int PLID){

  double Speeds[44];
  double DSpeeds[44];
  double v,dv;

  double TimeDiffOffset[44];
  double TimeDiffSlope[44];
  double offs,slop;

  RunNumber = R;

  Speeds[21] = 0;
  DSpeeds[21] = 0;
  Speeds[22] = 0;
  DSpeeds[22] = 0;

  TimeDiffOffset[21] = 0.;
  TimeDiffSlope[21] = 0.;
  TimeDiffOffset[22] = 0.;
  TimeDiffSlope[22] = 0.;

  for (int k=0;k<21;k++) {
    int REFID = k+1;
    gettime(REFID, PLID, v, dv, offs, slop);
    Speeds[k] = v;
    DSpeeds[k] = dv;
    TimeDiffOffset[k] = offs;
    TimeDiffSlope[k] = slop;
  }
  for (int k=23;k<44;k++) {
    int REFID = k+1;
    gettime(REFID, PLID, v, dv, offs, slop);
    Speeds[k] = v;
    DSpeeds[k] = dv;
    TimeDiffOffset[k] = offs;
    TimeDiffSlope[k] = slop;
 }

  char outf[128];
  sprintf(outf,"calibration%d/tof_speeds_plane%d_run%d.dat",RunNumber, PLID, RunNumber);
  ofstream OUTF;
  OUTF.open(outf,ios::out);
  for (int k=0;k<44;k++){
    OUTF << k << "  " << Speeds[k] << "  " << DSpeeds[k] << endl;
  }
  OUTF.close();

  sprintf(outf,"calibration%d/tof_timediffoffsets_plane%d.dat",RunNumber, PLID);
  OUTF.open(outf,ios::out);
  for (int k=0;k<44;k++){
    OUTF << k << "  " << TimeDiffOffset[k] << "  " << TimeDiffSlope[k] << endl;
  }
  OUTF.close();

}


void gettime(int REFID, int PLANEID, 
	     double &speed, double &dspeed, 
	     double &DToffset, double &DTslope){



  double XP[44];
  XP[21] = -3.;
  XP[22] = 3.;
  XP[20] = -7.5;
  XP[23] = 7.5;
  XP[19] = -10.5;
  XP[24] = 10.5;
  XP[18] = -15.;
  XP[25] = 15.;
  for (int k=1;k<19;k++){
    XP[18-k] = -15. - k*6.;
    XP[25+k] =  15. + k*6.;
  }

  char inf[128];
  sprintf(inf,"calibration%d/deltat_tpos_refpad%dplane%d.dat",RunNumber,REFID,PLANEID);
  ifstream INF;
  INF.open(inf);

  if (!INF){
    cout<<"file "<<inf<<" is not open! exit"<<endl;
    return;
  }
  
  double Times[44];
  double dTimes[44];
  int dummy;
  int idx = 0;
  double xt[44];
  while (!INF.eof()){
    xt[idx] = idx;
    INF>>dummy>>Times[idx]>>dTimes[idx];
    idx++;
  }


  TF1 *p2 = new TF1("p2","[0]+[1]*x",-125.,125.);
  p2->SetLineColor(8);

  //TF1 *SIN = new TF1("SIN","[0]*sin(x/[1])",-125.,125.);
  //SIN->SetLineColor(3);
  //SIN->SetParameter(0,8.);
  //SIN->SetParameter(1,10000.);

  idx=0;
  double XX[44];
  double YY[44];
  double dYY[44];
  int hili = 24;
  int loli = 17;
  if ((REFID>19) && (REFID<26)){
    loli = 16;
    hili = 28;
  }
  for (int k=0;k<44;k++){
    if ((k<loli) || (k>hili)){ // filter out half paddles and narrow paddles
      if (dTimes[k]<0.1){
	XX[idx] = XP[k];
	YY[idx] = Times[k];
	dYY[idx] = dTimes[k];
	idx++;
      }
    }
  }

  TGraphErrors *gr1 = new TGraphErrors(idx,XX,YY,NULL,dYY);
  gr1->SetMarkerStyle(20);
  gr1->SetMarkerSize(0.7);
  gr1->SetMarkerColor(2);
  gr1->Draw("AP");

  TGraphErrors *gr2 = (TGraphErrors*)gr1->Clone();
  gr1->Fit(p2,"RQ","",XX[0],XX[idx-1]);
  //gr1->Draw("ap*");
  p2->Draw("same");

  //gr2->Fit(SIN,"RQ","",XX[0],XX[idx-1]);
  //SIN->Draw("same");

  if (DEBUG>1){
    char tnam[128];
    sprintf(tnam,"Velocity Fit: Paddle %d, Plane %d",REFID,PLANEID);
    gr1->SetTitle(tnam);
    gr1->GetXaxis()->SetTitle("x position [cm]");
    gr1->GetYaxis()->SetTitle("#Deltat from fit [ns]");
    gPad->SetGrid();
    gPad->Update();
    //if ((REFID==5) || (REFID==15) || (REFID==19)) {
    if (1) {
      sprintf(tnam,"plots/velocity_refpad%d_plane%d_run%d.pdf",REFID,PLANEID,RunNumber);	
      gPad->SaveAs(tnam);
      
      double chi2 = p2->GetChisquare();
      double ndeg = (double)p2->GetNumberFitPoints() - (double)p2->GetNumberFreeParameters();
      cout<<"chi2/ndeg = "<<chi2/ndeg<<endl;
    }
    if (DEBUG == 99){
      getchar();
    }
  }
  

  DToffset = p2->GetParameter(0);
  DTslope  = p2->GetParError(0);

  speed  = 1./p2->GetParameter(1);
  dspeed = p2->GetParError(1)/p2->GetParameter(1)/p2->GetParameter(1);

}
