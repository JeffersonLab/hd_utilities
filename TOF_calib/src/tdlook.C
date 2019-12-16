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

#define NumPMTMax 200
int NPMTS = 0;
int BARS_PER_PLANE = 0; // including 2 short padeles being one
int PMTS_PER_PLANE = 0; 
int NSHORTS = 0;        // number of short paddles per plane

void gettime(int , int , double &, double &, double &, double &);

void tdlook(int R, int PLID){
  RunNumber = R;

  NPMTS = 176;            // TOF 1 geometry
  NSHORTS = 4;
  if (RunNumber>69999){
    NPMTS = 184;          // TOF 2 geometry
    NSHORTS = 8;
  }
  BARS_PER_PLANE = NPMTS/4;
  PMTS_PER_PLANE = NPMTS/2;

  double Speeds[100];
  double DSpeeds[100];
  double v,dv;

  double TimeDiffOffset[100];
  double TimeDiffSlope[100];
  double offs,slop;

  for (int k=(BARS_PER_PLANE/2)-NSHORTS/4; k<(BARS_PER_PLANE/2)+NSHORTS/4; k++){

    Speeds[k] = 0;
    DSpeeds[k] = 0;

    TimeDiffOffset[k] = 0.;
    TimeDiffSlope[k] = 0.;

  }

  for (int k=0; k<(BARS_PER_PLANE/2) - NSHORTS/4; k++) {
    int REFID = k+1;
    gettime(REFID, PLID, v, dv, offs, slop);
    Speeds[k] = v;
    DSpeeds[k] = dv;
    TimeDiffOffset[k] = offs;
    TimeDiffSlope[k] = slop;
  }
  for (int k=(BARS_PER_PLANE/2) + NSHORTS/4; k<BARS_PER_PLANE; k++) {
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
  for (int k=0;k<BARS_PER_PLANE;k++){
    OUTF << k << "  " << Speeds[k] << "  " << DSpeeds[k] << endl;
  }
  OUTF.close();

  sprintf(outf,"calibration%d/tof_timediffoffsets_plane%d.dat",RunNumber, PLID);
  OUTF.open(outf,ios::out);
  for (int k=0;k<BARS_PER_PLANE;k++){
    OUTF << k << "  " << TimeDiffOffset[k] << "  " << TimeDiffSlope[k] << endl;
  }
  OUTF.close();

}


void gettime(int REFID, int PLANEID, 
	     double &speed, double &dspeed, 
	     double &DToffset, double &DTslope){



  double XP[100];            // nominal geometric location of paddle center in [cm] 
  if (BARS_PER_PLANE<45){    // TOF 1
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
  } else {                 // TOF 2
    XP[22] = -2.25;
    XP[23] = 2.25;
    XP[21] = -6.75;
    XP[24] = 6.75;
    XP[20] = -10.5;
    XP[25] = 10.5;
    XP[19] = -13.5;
    XP[26] = 13.5;
    XP[18] = -17.25;
    XP[27] = 17.25;
    XP[17] = -21.75;
    XP[28] = 21.75;
    for (int k=1;k<18;k++){
      XP[17-k] = -27. - (k-1)*6.;
      XP[28+k] =  27. + (k-1)*6.;
    }
  }

  char inf[128];
  sprintf(inf,"calibration%d/deltat_tpos_refpad%dplane%d.dat",RunNumber,REFID,PLANEID);
  ifstream INF;
  INF.open(inf);

  if (!INF){
    cout<<"file "<<inf<<" is not open! exit"<<endl;
    return;
  }
  
  double Times[100];
  double dTimes[100];
  int dummy;
  int idx = 0;
  double xt[100];
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
  double XX[100];
  double YY[100];
  double dYY[100];
  int hili = 28;
  int loli = 17;
  if ((REFID>19) && (REFID<26)){
    loli = 16;
    hili = 28;
  }
  if (NSHORTS>2){
    loli = 16;
    hili = 29;
  }
  for (int k=0;k<BARS_PER_PLANE;k++){
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
    if (!(REFID%5)|| (DEBUG>98)) {
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
