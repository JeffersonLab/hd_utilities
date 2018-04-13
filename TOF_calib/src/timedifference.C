//
// function: timedifference(int Run, int REF, int REFPLANE)
//
// USE time difference of TOF;
// calculate time difference of reference paddle 
// apply walk correction to the timing
// find paddle hits in the other plane and plot them
// in 2d histogram: paddle-number-other-plane vs time difference
// the find the timing peak in each projection of the paddle number
// and fit the peak location. This is the central position in time
// defined by the geomtric location of the paddle behind/infrontof 
// the reference paddle.
// output file: "calibration/deltat_tpos_refpad%dplane%d.dat",REF,REFPLANE
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
#include "TSpectrum.h"
#include <fstream>
#include <iostream>

using namespace std;

TH2D *histTD;
int DEBUG = 2; //1: plot histogram, 2: plot and save, 99: interupt
int RunNumber; 

void timedifference(int Run, int REF, int REFPLANE){
  
  RunNumber = Run;
  char ROOTFileName[128];
  sprintf(ROOTFileName,"localdir/tofdata_run%d.root",RunNumber);
  if (RunNumber == 99999)
    sprintf(ROOTFileName,"localdir/big%d.root",RunNumber);

  int REFPAD = REF;
  int THEPLANE = 1;
  if ( REFPLANE ){
    THEPLANE = 0;
  }
  if (histTD==NULL){
    histTD = new TH2D("histTD","Time Difference",47, -1.5, 45.5 , 1600, -12., 12.);
  }

  histTD->Reset();

  //read walk correction factors
  char inf[128];
  sprintf(inf,"calibration%d/tof_walk_parameters_run%d.dat",RunNumber,RunNumber);
  ifstream INF;
  INF.open(inf);
  int idx;
  double FitPar[176][17];
  double dummy;
  for (int n=0; n<176; n++){
    INF >> idx;
    for (int s=0;s<17;s++) {
      INF >> FitPar[n][s];
    }
    INF>>dummy; // this is the chi2 of both fits
  }
  INF.close();

  TFile *ROOTFile = new TFile(ROOTFileName);
  TTree *t3 = (TTree*)ROOTFile->Get("TOFcalib/t3");
  int MaxHits = 100;
  int Event;
  int Nhits;
  float TShift;
  int Plane[100];
  int Paddle[100];
  float MeanTime[100];
  float TimeDiff[100];
  int NhitsA;
  int PlaneA[100];
  int PaddleA[100];
  float MeanTimeA[100];
  float TimeDiffA[100];
  float ADCL[100];
  float ADCR[100];
  float PEAKL[100];
  float PEAKR[100];
   
  t3->SetBranchAddress("Event",&Event);
  t3->SetBranchAddress("Nhits",&Nhits);
  t3->SetBranchAddress("TShift",&TShift);
  t3->SetBranchAddress("Plane",Plane);
  t3->SetBranchAddress("Paddle",Paddle);
  t3->SetBranchAddress("MeanTime",MeanTime);
  t3->SetBranchAddress("TimeDiff",TimeDiff);

  t3->SetBranchAddress("NhitsA",&NhitsA);
  t3->SetBranchAddress("PlaneA",PlaneA);
  t3->SetBranchAddress("PaddleA",PaddleA);
  t3->SetBranchAddress("MeanTimeA",MeanTimeA);
  t3->SetBranchAddress("TimeDiffA",TimeDiffA);
  t3->SetBranchAddress("ADCL",ADCL);
  t3->SetBranchAddress("ADCR",ADCR);
  t3->SetBranchAddress("PEAKL",PEAKL);
  t3->SetBranchAddress("PEAKR",PEAKR);
  
  unsigned int nentries = (unsigned int) t3->GetEntries();
  cout<<"Number of Entries "<<nentries<<endl;
  /*
  if (nentries>100000000){
    nentries = 100000000;
    cout<<"  use only 100000000";
  }
  cout<<endl;
  */
  for (unsigned int k=0; k<nentries; k++){
    t3->GetEntry(k);
    //cout<<Event<<" "<<Nhits<<endl;
    if (!(k%10000000)){
      TDatime a;
      cout<<"Current time is "<<a.Get()<<endl;
    }

    int idRef;
    for (idRef=0; idRef<Nhits;idRef++){
      if ((Plane[idRef] == REFPLANE) && (Paddle[idRef] == REFPAD)){
	//cout<<idRef<<endl;
	break;
      }
    }

    float TD_REF = 999.;
    if (idRef<Nhits){

      // find ADC data for Ref Paddle
      for (int n=0; n<NhitsA; n++){
	if ( (PlaneA[n] == REFPLANE) && 
	     (PaddleA[n] == REFPAD) ) {

	  int idxL = REFPLANE*88 + REFPAD-1;
	  int idxR = idxL + 44;

	  // apply walk correction
	  int DOFF = 0;
	  if (PEAKL[n]>FitPar[idxL][16]){
	    DOFF = 8;
	  }
	  double a1 = FitPar[idxL][0+DOFF]
	    +FitPar[idxL][2+DOFF]*TMath::Power(PEAKL[n],-0.5) 
	    +FitPar[idxL][4+DOFF]*TMath::Power(PEAKL[n],-0.33) 
	    +FitPar[idxL][6+DOFF]*TMath::Power(PEAKL[n],-0.2);
	  if (PEAKL[n]>4095){
	    a1 += 0.55;
	  }

	  DOFF = 8;
	  double a2 = FitPar[idxL][0+DOFF]
	    +FitPar[idxL][2+DOFF]*TMath::Power(1500.,-0.5) 
	    +FitPar[idxL][4+DOFF]*TMath::Power(1500.,-0.33) 
	    +FitPar[idxL][6+DOFF]*TMath::Power(1500.,-0.2);

	  float tcL = a1 - a2;

	  DOFF = 0;
	  if (PEAKR[n]>FitPar[idxR][16]){
	    DOFF = 8;
	  }	  
	  a1 = FitPar[idxR][0+DOFF]
	    +FitPar[idxR][2+DOFF]*TMath::Power(PEAKR[n],-0.5) 
	    +FitPar[idxR][4+DOFF]*TMath::Power(PEAKR[n],-0.33) 
	    +FitPar[idxR][6+DOFF]*TMath::Power(PEAKR[n],-0.2);
	  if (PEAKR[n]>4095){
	    a1 += 0.55;
	  }

	  DOFF = 8;
	  a2 = FitPar[idxR][0+DOFF]
	    +FitPar[idxR][2+DOFF]*TMath::Power(1500.,-0.5) 
	    +FitPar[idxR][4+DOFF]*TMath::Power(1500.,-0.33) 
	    +FitPar[idxR][6+DOFF]*TMath::Power(1500.,-0.2);

	  float tcR = a1 - a2; 

	  float tcorr = (tcR - tcL)/2.;
	  //cout<<endl;
	  //cout<<tcorr<<"  "<<ADCL[n]<<"  "<<ADCR[n]<<endl;
	  // apply walk correction:
	  TD_REF = TimeDiff[idRef] - tcorr;
	  break;
	}
      }

      for (int n=0; n<Nhits;n++){
	if (Plane[n] == THEPLANE){ 
	  histTD->Fill((double)Paddle[n],TD_REF);
	}  
      }
    }
    
  }

  ROOTFile->Close();

  double tpos[44];
  double dtpos[44];
  for (int j=3; j<3+44; j++){
    double p = 0.;
    double dp = 0.;
    tpos[j-3] = p;
    dtpos[j-3] = dp;
    TH1D *hp = histTD->ProjectionY("hp",j,j);
    char hnamx[128];
    sprintf(hnamx,"Projection #Deltat to paddel %d REFPAD%d plane%d",j-2,REFPAD,REFPLANE);
    hp->SetTitle(hnamx);

    if (hp->Integral(1,hp->GetNbinsX()-2)>100){


      if (j<6){
	hp->GetXaxis()->SetRangeUser(-12.,-3.);
      }
      if (j>38){
	hp->GetXaxis()->SetRangeUser(3.,12.);
      }


      TSpectrum *speaks = new TSpectrum(6);
      Int_t nfound=0;
      if (DEBUG){
	nfound = speaks->Search(hp,2,"",0.10);
      } else {
	nfound = speaks->Search(hp,2,"nodraw",0.10);
      }
      //printf("Found %d candidate peaks to fit\n",nfound);
      double *xpeaks = speaks->GetPositionX();

      // for paddles on the left of the center take left most peak
      // for paddles on the right of the center take right most peak
      double max = 100.;
      if (j>24){
	max = -100.;
      }
      for (int pp=0;pp<nfound;pp++) {
	double xp = xpeaks[pp];
	if (j<24) {
	  if (xp<max){
	    max = xp;
	  }	
	} else {
	  if (xp>max){
	    max = xp;
	  }	
	}
      }

      double bw = hp->GetBinWidth(1);
      double hili = max + 10.*bw;
      double loli = max - 10.*bw;

      hp->Fit("gaus","RQ0","",loli,hili);
      TF1 *f1 = hp->GetFunction("gaus");
      p = f1->GetParameter(1);
      double s = f1->GetParameter(2);
      hili = p+s*1.2;
      loli = p-s*1.2;
      if ((j>19) && (j<24))
	hili = p+s*0.7;
      if ((j>23) && (j<30))
	loli = p-s*0.7;
      


      hp->Fit("gaus","RQ","",loli,hili);
      f1 = hp->GetFunction("gaus");
      p = f1->GetParameter(1);
      dp = f1->GetParError(1);

      if (DEBUG>2) {
	hp->Draw();
	gPad->Update();
	getchar();
      }

      if (DEBUG) {
	char fnam[128];
	if (DEBUG>1){
	  if (j==3) {
	    sprintf(fnam,"Paddle number other plane vs. #Deltat Paddle REFPAD%d",REFPAD);
	    histTD->SetTitle(fnam);
	    histTD->GetYaxis()->SetTitle("#Deltat [ns]");
	    histTD->GetXaxis()->SetTitle("Paddle number [#]");
	    histTD->Draw("colz");
	    gPad->SetGrid();
	    gPad->SetLogz(1);
	    gPad->Update();
	    sprintf(fnam,"plots/paddleNumber_vs_deltatRefPad%d_plane%d_run%d.pdf",REFPAD,REFPLANE,RunNumber);
	    gPad->SaveAs(fnam);
	  }
	  if (!((j-3)%5) || (DEBUG>2)){
	    hp->GetXaxis()->SetTitle("#Deltat [ns]");
	    char hnam1[128];
	    sprintf(hnam1,"Projection #Deltat to paddel %d REFPAD%d plane%d",j-2,REFPAD,REFPLANE);
	    hp->SetTitle(hnam1);
	    gStyle->SetOptFit(1); 
	    hp->GetXaxis()->SetRangeUser(p-2.,p+2);
	    hp->Draw();
	    //	    f1->Draw("same");
	    gPad->SetLogz(0);
	    gPad->Update();
	    sprintf(hnam1,"plots/deltat_fitposition_p%d_repad%d_plane%d_run%d.pdf",j-2,REFPAD,REFPLANE,RunNumber);
	    gPad->SaveAs(hnam1);
	  }
	}
	if (DEBUG == 99){
	  hp->Draw();
	  gPad->SetGrid();
	  gPad->Update();
	  getchar();
	}
      }
    } else {
      cout<<"Error histogram is empty!!!! Paddle "<<j-2<<endl;
      p = 0;
      dp = 0;
    }
    tpos[j-3] = p;
    dtpos[j-3] = dp;
  }

  char of[128];
  sprintf(of,"calibration%d/deltat_tpos_refpad%dplane%d.dat",RunNumber,REF,REFPLANE);
  ofstream OF(of, ofstream::out);

  if (OF){
    for (int n=0; n<44; n++){ 
      OF<<n<<"  "<<tpos[n]<<"  "<<dtpos[n]<<endl;
    }
  } else {
    cout<<"Error open file "<<of<<endl;
  }
  OF.close();
  if (!(REF%10)){
    sprintf(of,"calibration%d/deltat_tpos_refpad%dplane%d.root",RunNumber,REF,REFPLANE);
    TFile *f = new TFile(of,"RECREATE");
    histTD->Write();
    f->Close();
  }

} 

