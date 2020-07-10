//
//
// this code takes the mean-time offsets and time-difference offsets of all
// double ended readout paddles and calculates the offsets for the individual
// PMTs. Then the mean of all these offsets is determined and subracted from
// all the individual offsets. In that way the timing offsets for the PMTs will
// be neutral with respect to global timing calibrations between detectors.
// The offsets for the single ended paddles are set to zero.
//
// 3 input files:
// mean time both planes: mtparameters_plane0_ref18.dat
// time difference plane 0: tof_timediffoffsets_plane0.dat
// time difference plane 1: tof_timediffoffsets_plane1.dat
// changed to:
// time difference plane 0: timediff_offsets_plane%d_run%d.dat
// time difference plane 1: timediff_offsets_plane%d_run%d.dat
//
// 2 output file:
//               tofpmt_tdc_offsets_all_runXXXXX.DB
//               tofpaddles_propagation_speed_runXXXXX.DB
//


#include "TROOT.h"
#include "TStyle.h"
#include "TMath.h"
#include "TPad.h"
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TF1.h>
#include <TProfile.h>
#include <fstream>
#include <iostream>

using namespace std;

#define NumPMTMax 200
int NPMTS = 0;
int BARS_PER_PLANE = 0; // including 2 short padeles being one
int PMTS_PER_PLANE = 0; 
int NSHORTS = 0;       // number of short paddles per plane


int REFPADDLE = 16; // was 18

void consolidate(int RunNumber){
  // 3 input files
  // mean time both planes: mtparameters_plane0_ref18.dat
  // time difference plane 0: tof_timediffoffsets_plane0.dat
  // time difference plane 1: tof_timediffoffsets_plane1.dat

  NPMTS = 176;            // TOF 1 geometry
  NSHORTS = 4;
  if (RunNumber>69999){
    NPMTS = 184;          // TOF 2 geometry
    NSHORTS = 8;
  }
  BARS_PER_PLANE = NPMTS/4;
  PMTS_PER_PLANE = NPMTS/2;

  double MeanTimes[2][100];
  double DeltaTimes[2][100];

  char inf[128];
  sprintf(inf,"calibration%d/mtparameters_plane0_ref%d.dat",RunNumber, REFPADDLE);
  cout<<"read data from: "<<inf<<endl;

  ifstream INF;
  INF.open(inf);
  int k,plane;
  double t;
  while(!INF.eof()) {
    INF>>k>>plane>>t;
    MeanTimes[plane][k] = t;
  }
  INF.close();

  sprintf(inf,"calibration%d/tof_timediffoffsets_plane0.dat",RunNumber);
  //sprintf(inf,"calibration%d/timediff_offsets_plane0_run%d.dat",RunNumber,RunNumber);

  cout<<"read data from: "<<inf<<endl;
  INF.open(inf);
  double dummy;
  while(!INF.eof()) {
    INF>>k>>t>>dummy;
    DeltaTimes[0][k] = t;
  }
  INF.close();
 
  sprintf(inf,"calibration%d/tof_timediffoffsets_plane1.dat",RunNumber);
  //sprintf(inf,"calibration%d/timediff_offsets_plane1_run%d.dat",RunNumber,RunNumber);
  cout<<"read data from: "<<inf<<endl;
  INF.open(inf);
  while(!INF.eof()) {
    INF>>k>>t>>dummy;
    DeltaTimes[1][k] = t;
  }
  INF.close();
 

  double OFFSETS[2][100][2];

  char outf[128];
  sprintf(outf,"calibration%d/tofpmt_tdc_offsets_all.dat",RunNumber);
  ofstream OUTF;
  OUTF.open(outf);
  OUTF<<"paddle plane  offsetL   offsetR"<<endl;
  OUTF<<"----------------------------"<<endl;
  TH1F *histoffsets = new TH1F("histoffsets","PMT offsets distribution",100,-5.,5.);
  
  for (int plane=0;plane<2;plane++) {    
    for (int k=0;k<BARS_PER_PLANE;k++){
      // MT = (cR+cL)/2  DT = (cR-cL)/2
      // cL = (MT-DT)  and cR = (MT+DT)
      
      OFFSETS[plane][k][0] = (MeanTimes[plane][k] - DeltaTimes[plane][k]) ;
      OFFSETS[plane][k][1] = (MeanTimes[plane][k] + DeltaTimes[plane][k]) ;      
      
      OUTF<<k<<"  "<<plane<<"  "<<OFFSETS[plane][k][0]<<"   "<<OFFSETS[plane][k][1]<<endl;
      //if ((k<21) || (k>22)){
      if (TMath::Abs(k+1 - BARS_PER_PLANE/2 - 0.5)>(double)NSHORTS/4.) {
	histoffsets->Fill(OFFSETS[plane][k][0]);
	histoffsets->Fill(OFFSETS[plane][k][1]);	
      }
    }
  }
  OUTF.close();
  histoffsets->Draw();
  histoffsets->Fit("gaus");
  gPad->SetGrid();
  gPad->Update();
  sprintf(outf,"plots/distribution_of_offsets_run%d.pdf",RunNumber);
  gPad->SaveAs(outf);
  //TF1 *f1 = histoffsets->GetFunction("gaus");
  //float MeanOff = f1->GetParameter(1);
  float MeanOff = histoffsets->GetMean(); // use mean instead of gaussian fit

  sprintf(outf,"calibration%d/tofpmt_tdc_offsets_all_run%d.DB",RunNumber,RunNumber);
  OUTF.open(outf);
  for (int plane=0;plane<2;plane++) {    
    for (int end=0;end<2;end++){
      for (int k=0;k<BARS_PER_PLANE;k++){
	//if ((k<21) || (k>22) ){
	if (TMath::Abs(k+1 - BARS_PER_PLANE/2 - 0.5)>(double)NSHORTS/4.) {
	  OUTF<<OFFSETS[plane][k][end]-MeanOff<<endl; 
	} else {
	  OUTF<<0<<endl;
	}
      }
    }
  }
  OUTF.close();
  
  char fnam[128];
  sprintf(fnam,"calibration%d/tof_speeds_plane0_run%d.dat",RunNumber,RunNumber);
  cout<<"open file: "<<fnam<<endl;
  INF.open(fnam);
  if (!INF){
    cout<<"Error open file: "<<fnam<<endl;
    cout<<"Major problem, stop right here!"<<endl;
    return;
  }
  int dum1;
  float Speeds[NumPMTMax],dum2;
  float SSum = 0;
  int Scnt = 0;
  for (int k=0;k<BARS_PER_PLANE;k++){
    INF>>dum1>>Speeds[k]>>dum2;
    //cout<<dum1<<"  "<<Speeds[k]<<endl;
    if (TMath::Abs(Speeds[k]-15.5)<2.){
      SSum += Speeds[k];
      Scnt++;
    }
  }
  INF.close();
  sprintf(fnam,"calibration%d/tof_speeds_plane1_run%d.dat",RunNumber,RunNumber);
  cout<<"open file: "<<fnam<<endl;
  INF.open(fnam);
  if (!INF){
    cout<<"Error open file: "<<fnam<<endl;
    cout<<"Major problem, stop right here!"<<endl;
    return;
  }
  for (int k=0;k<BARS_PER_PLANE;k++){
    INF>>dum1>>Speeds[k+BARS_PER_PLANE]>>dum2;
    if (TMath::Abs(Speeds[k+BARS_PER_PLANE]-15.5)<2.){
      SSum += Speeds[k];
      Scnt++;
    }
  }
  INF.close();
  double Mspeed = SSum/(double)Scnt;
  cout<<"Mean propagation speed is: "<<Mspeed<<endl;

  sprintf(outf,"calibration%d/tofpaddles_propagation_speed_run%d.DB",RunNumber,RunNumber);
  OUTF.open(outf);

  for (int k=0;k<PMTS_PER_PLANE;k++){
    //if ((k==21) || (k==22) || (k==65) || (k==66)) {
    if( (TMath::Abs(k+1 - BARS_PER_PLANE/2 - 0.5)<(double)NSHORTS/4.)  ||
	(TMath::Abs(k+1 - BARS_PER_PLANE - BARS_PER_PLANE/2 - 0.5)<(double)NSHORTS/4.)) {
      // for the single ended paddles use the mean propagation speed of 
      // all the other paddles.
      Speeds[k] = Mspeed;
    }
    OUTF<<Speeds[k]<<endl;
  }
  OUTF.close();

}
