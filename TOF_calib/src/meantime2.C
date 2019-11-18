//
// function: meantime2()
//
// read mtpos_..... data from all reference paddles in the first plane and  
// use paddle 18 of the first plane as THE reference paddle to calculate for each
// of the other reference paddles the differences of the meantimedifferences and 
// plot these data in a histogram
// fit the location of this histogram which is the relative time offset in the
// mean time difference between paddle 18 and these other reference paddles in the 
// same plane.
// now we have a full list of relative mean time differences of all paddles with
// respect to the reference paddle 18 in the first plane.
// the single ended paddles are of course not use here.
// output file:   "calibration/mtparameters_plane0_ref18.dat"
//
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
int NSHORTS = 0;
int BARS_PER_PLANE = 0; // including 2 short padeles being one
int PMTS_PER_PLANE = 0; 

TH1F *histdt;
int DEBUG = 2;  // 0: not print outs, 2: generate plots in pdf, 99: user interupt after draw on screen
double getmt(int , int , double* ,int );

void meantime2(int RunNumber){

  NPMTS = 176;                 // TOF1 geometry 
  NSHORTS = 4;
  if (RunNumber>69999){
    NPMTS = 184;               // TOF2 geometry
    NSHORTS = 8;
  }
  BARS_PER_PLANE = NPMTS/4;
  PMTS_PER_PLANE = NPMTS/2;

  double MTPlane0[100];
  double MTPlane1[100];


  for (int k=(BARS_PER_PLANE/2) - NSHORTS/4; k<(BARS_PER_PLANE/2) + NSHORTS/4; k++){  
    MTPlane0[k] = 0.;
    MTPlane0[k] = 0.;
    MTPlane1[k] = 0.;
    MTPlane1[k] = 0.;
  }
  // determine average mean time differences between the reference paddle 18 and paddle k
  // <s> = 1/N Sum(dMT_k - dMT_18)_i  sum over all i
  // this average number is the meatime offsets between paddle k and paddle 18 in plane 0
  for (int k=1;k<BARS_PER_PLANE/2 - NSHORTS/4 + 1;k++){    
    MTPlane0[k-1] = getmt(18,k,MTPlane1,RunNumber);
  }
  for (int k=BARS_PER_PLANE/2 + NSHORTS/4 + 1;k<BARS_PER_PLANE+1;k++){    
    MTPlane0[k-1] = getmt(18,k,MTPlane1,RunNumber);
  }
  MTPlane0[18-1] = 0.;  // this is the reference paddle!

  ofstream OUTF;
  char of[128];
  sprintf(of,"calibration%d/mtparameters_plane0_ref18.dat",RunNumber);
  OUTF.open(of);
  if (OUTF){
    for (int k=0; k<BARS_PER_PLANE; k++){
      OUTF<<k<<"  "<<" 0   "<<MTPlane0[k]<<endl;
    }
    for (int k=0; k<BARS_PER_PLANE; k++){
      OUTF<<k<<"  "<<" 1   "<<MTPlane1[k]<<endl;
    }
  }
  OUTF.close();
  cout<<"End of meantime2()"<<endl;
}

//
// helper function to determine mean difference between two paddle o the same plane
// normaly this would be plane 0
//

double getmt(int p1, int p2, double *mtref,int RunNumber) {

  double PPos[2][100];
  int REFPAD[2] = {p1,p2};
  int REFPLANE = 0;

  for (int k=0;k<2;k++){
    char inf[128];
    sprintf(inf,"calibration%d/mtpos_refpad%dplane%d.dat",RunNumber,REFPAD[k],REFPLANE);
    ifstream IF;
    IF.open(inf);
    int dummy;
    double dummy1;
    if (IF){
      for (int n=0;n<BARS_PER_PLANE;n++){
	IF>>dummy>>PPos[k][n]>>dummy1;
      }
    }
    IF.close();
  }
  for (int n=0;n<BARS_PER_PLANE;n++){
    mtref[n] = PPos[0][n];
  }
  
  // create histogram for the first tme
  if (histdt == NULL){
    histdt = new TH1F("histdt","MTREF diff", 200,-4.,4.);
  }
  histdt->Reset();

  // fill histogram with differences

  for (int n=0;n<BARS_PER_PLANE;n++){
    float tdiff = PPos[0][n] -  PPos[1][n];
    histdt->Fill(tdiff);
  }

  // fit histogram to find mean
  int mb = histdt->GetMaximumBin();
  double max = histdt->GetBinCenter(mb);
  double loli = max - .1;
  double hili = max + .1;
  histdt->Fit("gaus","RQ","",loli,hili);
  TF1 *f1 = histdt->GetFunction("gaus");
  double pos = f1->GetParameter(1);
  double sig = f1->GetParameter(2);
  if (sig>0.1){
    pos = histdt->GetMean();
  }
  //double pos = max;
  if (DEBUG) {
    char fnam[128];
    if (DEBUG>1){
      if (!(REFPAD[1]%5)){
	sprintf(fnam,"Average MT-differences Paddle %d to Ref Paddle %d [ns]",REFPAD[1],REFPAD[0]);
	histdt->GetXaxis()->SetTitle(fnam);
	gStyle->SetOptFit(1);
	histdt->Draw();
	gPad->SetGrid();
	gPad->Update();
	sprintf(fnam,"plots/meantime_average_to_refpaddle%d.pdf",REFPAD[1]);
	gPad->SaveAs(fnam);
      }
    }
    //histdt->Draw();
    //gPad->SetGrid();
    //gPad->Update();
    if (DEBUG == 99){
      getchar();
    }
  }

  return pos;

}
