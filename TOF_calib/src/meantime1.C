//
// main function: meantime1(int Run, int REF, int RefPlane)
//
// Determine meantime-difference between ref-paddle and all
// other 90deg. rotated paddles behind/in-front-of it.
// Fit the meantime-difference peak for each paddle w.r.t. the
// reference paddle.
// To get better results apply the walk corrections and make a cut
// the time difference to match the geometric position of the reference
// paddle the cut is +/- 30cm The cut can not be too tight because the
// times are not yet calibrated ;-)
// output file: "calibration/mtpos_refpad%dplane%d.dat",REFPAD,REFPLANE
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
#include <TDatime.h>

using namespace std;

TH2F *dMT;
int DEBUG = 2; // 1: show plots, 2: save plots, 99: save plots and interupt 

int NOWALK = 0;
int REFPAD = 18;
int REFPLANE = 0;
int RunNumber;

void findpeak(double*, double*);

void meantime1(int Run, int REF, int RefPlane){
  
  RunNumber = Run;
  char ROOTFileName[128];
  sprintf(ROOTFileName,"localdir/tofdata_run%d.root",RunNumber);
  if (RunNumber == 99999)
    sprintf(ROOTFileName,"localdir/big%d.root",RunNumber);

  REFPAD = REF;
  REFPLANE = RefPlane;
  int THEPLANE = 1;
  if (REFPLANE){
    THEPLANE = 0;
  }

  // create dMT thefirst time
  if (dMT==NULL){
    dMT = new TH2F("dMT","Mean Time Difference",300, -10.,10.,44,0.,44.);
  }

  dMT->Reset();
  char htit1[128];
  sprintf(htit1,"Mean Time Difference Ref-Pad%d Plane %d",REFPAD,REFPLANE);
  dMT->SetTitle(htit1);

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

  // define xpos of the reference paddle by geometry
  int REFPADi = REFPAD-1;
  float xpos= 999.;
  if (REFPADi<19){
    xpos = -15. - 6.0*(18. - REFPADi);
  } else if (REFPADi>24) {
    xpos = 15. + 6.0*(REFPADi-24.);
  } else if (REFPADi<21){
    xpos = -7.5 - 3.0*(20. - REFPADi);
  } else if (REFPADi>22){
    xpos = 7.5 + 3.0*(REFPADi-23.);
  }

  // prepare root file and tree to read from 
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
  float PEAKR[100];
  float PEAKL[100];
  
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
  cout<<"Number of Entries "<<nentries;
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

    // select events that have a hit in the reference paddle
    int idRef;
    for (idRef=0; idRef<Nhits;idRef++){
      if ((Plane[idRef] == REFPLANE) && (Paddle[idRef] == REFPAD)){
	//cout<<idRef<<endl;
	break;
      }
    }

    // in case of hit in reference paddle find ADC data and apply walk correction
    // to get corrected mean time.
    float MT_Ref=999.;
    if (idRef<Nhits){
      // find ADC data for Ref Paddle
      for (int n=0; n<NhitsA; n++){
	if ( (PlaneA[n] == REFPLANE) && 
	     (PaddleA[n] == REFPAD) ) {

	  int idxL = REFPLANE*88 + PaddleA[n]-1;
	  int idxR = idxL + 44;

	  // calculate walk correction for TDC times
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

	  float tcorr = tcR + tcL;
	  //cout<<endl;
	  //cout<<tcorr<<"  "<<ADCL[n]<<"  "<<ADCR[n]<<endl;
	  if (!NOWALK) {
	    MT_Ref = MeanTime[idRef] - tcorr/2.;
	  }
	  break;
	}
      }

      // loop over paddles to find hit in other paddles and apply
      // the walk correction. Then subract the reference mean time from the paddle mean time
      // and fill it in a 2-d histogram.
      float MT_Pad = 0;
      float TD_Pad = 999.;
      for (int n=0; n<Nhits;n++){
	if (Plane[n] == THEPLANE){
	  for (int i=0; i<NhitsA; i++){
	    if ((PlaneA[i] == THEPLANE) && (PaddleA[i] == Paddle[n])){
	      int idxL = 88 * THEPLANE + PaddleA[i]-1;
	      int idxR =  idxL + 44;

	      // calculate walk correction for TDC times
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
	      
	      float tcorr = tcR + tcL;

	      //cout<<tcorr<<"  "<<ADCL[i]<<"  "<<ADCR[i]<<endl;
	      if (!NOWALK) {
		MT_Pad = MeanTime[n] - tcorr/2.;
		TD_Pad = TimeDiff[n] - (tcR-tcL)/2.;
	      }
	      break;
	    }
	  }
	  float thepos = TD_Pad*15.5;  // rought time to distance
	  //cout<<xpos<<"   "<<thepos<<endl;
	  if (TMath::Abs(thepos-xpos)<30.) {
	    float MT = MT_Pad - MT_Ref;
	    dMT->Fill(MT,Paddle[n]-1);
	  }
	}  
      }
    }

  }

  ROOTFile->Close();

  double ppos[44];
  double psig[44];
  // find the peaks in all the 1-d projections of the 2-d histogram
  findpeak(ppos,psig);
  
  char of[128];
  sprintf(of,"calibration%d/mtpos_refpad%dplane%d.dat",RunNumber,REFPAD,REFPLANE);
  ofstream OF;
  OF.open(of);
  if (OF){
    for (int n=0;n<44;n++){
      OF<<n<<" "<<ppos[n]<<" "<<psig[n]<<endl;
    }
  }
  OF.close();
} 

void findpeak(double *MTPosition, double *MTSigma){


  // loop over all 44 bins of the vertical axis of the 2-d histograms
  // these are the paddles orthogonal to the reference paddle.
  // find the peak in these 1-d distributions using Gaussian fits.
  
  if (DEBUG==99){
    dMT->Draw("colz");
    gPad->Update();
    getchar();
  }


  for (int k=1;k<45;k++ ){

    TH1D *h = dMT->ProjectionX("h",k,k);

    double pos = 0;
    double sig = 0;

    if (h->GetEntries()>100){
      
      //cout<<"Paddle: "<<k<<endl;
      TSpectrum *speaks = new TSpectrum(4);
      Int_t nfound = 0;
      if (DEBUG) {
	nfound = speaks->Search(h,2,"",0.10);
      } else {
 	nfound = speaks->Search(h,2,"nodraw",0.10);
     }
      
      //printf("Found %d candidate peaks to fit\n",nfound);
      double *xpeaks = speaks->GetPositionX();
      double max = 0.;
      double MaxPeak = 0.;
      int MaxPN;
      for (int pp=0;pp<nfound;pp++) {
	double xp = xpeaks[pp];
	cout<<"Peak "<<pp<<": x = "<<xp<<endl;

	Int_t bin = h->GetXaxis()->FindBin(xp);
	double yp = h->GetBinContent(bin);
	if (yp>MaxPeak){
	  MaxPeak = yp;
	  MaxPN = pp;
	  max = xp;
	}
      }

      double maxc = max;
      double bw = h->GetBinWidth(1);
      double loli = maxc - 4.*bw;
      double hili = maxc + 4.*bw;
      if (loli<-10){
	loli = -10.;
      }
      if (hili>10.){
	hili = 10.;
      }

      h->Fit("gaus","QR","",loli,hili);
      TF1 *f1 = h->GetFunction("gaus");
      pos = f1->GetParameter(1);
      sig = f1->GetParameter(2);
      hili = pos + 1*sig;
      loli = pos - 1*sig;
      h->Fit("gaus","QR","",loli,hili);
      f1 = h->GetFunction("gaus");
      pos = f1->GetParameter(1);
      sig = f1->GetParameter(2);
      hili = pos + 1.*sig;
      loli = pos - 1.*sig;
      h->Fit("gaus","QR","",loli,hili);
      f1 = h->GetFunction("gaus");
      pos = f1->GetParameter(1);      

      if (DEBUG){
	if (DEBUG>1){
	  char exnam[128];
	  if (k==3){
	    dMT->GetYaxis()->SetTitle("Paddle Number [#]");
	    sprintf(exnam,"MeanTime Difference MT_{i}-MT_{ref} [ns]");
	    dMT->GetXaxis()->SetTitle(exnam);
	    dMT->Draw("colz");
	    gPad->SetGrid();
	    gPad->SetLogz(1);
	    gPad->Update();
	    sprintf(exnam,"plots/mtdiff_vs_padnum_RefPad%d_RefPlane%d_run%d.pdf",REFPAD,REFPLANE,RunNumber);
	    if (NOWALK){
	      sprintf(exnam,"plots/mtdiff_vs_padnum_RefPad%d_run%d_nowalk.pdf",REFPAD,RunNumber);
	    }
	    gPad->SaveAs(exnam);
	  }
	  if (DEBUG>1){
	    if (!(k%10)){
	      gPad->SetLogz(0);  
	      h->Draw();
	      sprintf(exnam,"Mean-Time-Difference Pad%d to Ref-Pad%d",k,REFPAD);
	      h->SetTitle(exnam);
	      h->GetXaxis()->SetTitle("MeanTime Difference MT_{i}-MT_{ref} [ns]");
	      h->GetYaxis()->SetTitle("Counts [#]");
	      gPad->SetGrid();
	      gPad->Update();
	      sprintf(exnam,"plots/mtdiff_vs_padnum_proj_%d_ref%d_RefPlane%d_run%d.pdf",k,REFPAD,REFPLANE,RunNumber);
	      if (NOWALK){
		sprintf(exnam,"plots/mtdiff_vs_padnum_proj_%d_ref%d_run%d_nowalk.pdf",k,REFPAD,RunNumber);
	      }
	      gPad->SaveAs(exnam);
	    }
	  }
	}
	h->Draw();
	gPad->Update();
	if (DEBUG == 99)
	  getchar();
      }
    }
    MTPosition[k-1] = pos;
    MTSigma[k-1] = sig;
  }

  return;
}
