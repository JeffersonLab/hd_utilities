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

TH2F *histTD; 
int DEBUG = 2; //1: plot histogram, 2: plot and save, 99: interupt
int RunNumber; 

void timedifferenceAMP(int Run, int REF, int REFPLANE){
  
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
    histTD = new TH2F("histTD","Time Difference",47, -1.5, 45.5 , 400, -12., 12.);
  }

  histTD->Reset();

  //read walk correction factors
  char inf[128];
  sprintf(inf,"calibration%d/tof_walk_parameters_AMP_run%d.DB",RunNumber,RunNumber);
  ifstream INF;
  INF.open(inf);
  int idx;
  double timewalk_parameters_AMP[176][6];
  for (int n=0; n<176; n++){
    INF >> timewalk_parameters_AMP[n][0] >> timewalk_parameters_AMP[n][1] >> timewalk_parameters_AMP[n][2] 
        >> timewalk_parameters_AMP[n][3] >> timewalk_parameters_AMP[n][4] >> timewalk_parameters_AMP[n][5];
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
  for (unsigned int k=0; k<nentries; k++){
    t3->GetEntry(k);
    //cout<<Event<<" "<<Nhits<<endl;

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

          int IDX[2];
          IDX[0] = idxL;
          IDX[1] = idxR;
          double AMP[2];
          AMP[0] = PEAKL[n]; 
          AMP[1] = PEAKR[n]; 
          float corr[2];
          
          //apply walk correction
          for (int i=0; i<2;i++) {
            double C0 = timewalk_parameters_AMP[IDX[i]][0];
            double C1 = timewalk_parameters_AMP[IDX[i]][1];
            double C2 = timewalk_parameters_AMP[IDX[i]][2];
            double C3 = timewalk_parameters_AMP[IDX[i]][3];
            double hookx = timewalk_parameters_AMP[IDX[i]][4]; 
            double refx = timewalk_parameters_AMP[IDX[i]][5];
            double val_at_ref = C0 + C1*pow(refx,C2); 
            double val_at_hook = C0 + C1*pow(hookx,C2); 
            double slope = (val_at_hook - C3)/hookx;
            if (refx>hookx){
              val_at_ref  = slope * refx + C3; 
            }
            double val_at_A = C0 + C1*pow(AMP[i],C2);
            if (AMP[i]>hookx){
              val_at_A = slope * AMP[i] + C3; 
            }
            
            corr[i] = val_at_A - val_at_ref;

	    //cout<<i<<":  "<<hookx<<"  "<<refx<<"   "<<AMP[i]<<" "<<slope<<"  "<<val_at_A<<"  "<<val_at_ref<<endl; 
            //cout<<"        "<<C0<<"  "<<C1<<"  "<<C2<<endl;

          }
	 
	  

          TD_REF = TimeDiff[idRef] - (corr[1]-corr[0])/2.;

	  break;
	}
      }

      for (int j=0; j<Nhits;j++){
	if (Plane[j] == THEPLANE){ 
	  histTD->Fill((float)Paddle[j],TD_REF);
	}  
      }
    }
    
  }

  ROOTFile->Close();

  if (DEBUG==99){
    histTD->Draw("colz");
    gPad->Update();
    getchar();
  }
  


  double tpos[44];
  double dtpos[44];
  for (int j=3; j<3+44; j++){
    double p = 0.;
    double dp = 0.;
    tpos[j-3] = p;
    dtpos[j-3] = dp;
    TH1D *hp = histTD->ProjectionY("hp",j,j);

    if (hp->Integral(1,hp->GetNbinsX()-2)>100){

      TSpectrum *speaks = new TSpectrum(6);
      Int_t nfound=0;
      if (DEBUG){
	nfound = speaks->Search(hp,2,"",0.20);
      } else {
	nfound = speaks->Search(hp,2,"nodraw",0.20);
      }
      //printf("Found %d candidate peaks to fit\n",nfound);
      double *xpeaks = speaks->GetPositionX();

      // for paddles on the left of the center take left most peak
      // for paddles on the right of the cneter take right most peak
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
      double hili = max + 5.*bw;
      double loli = max - 5.*bw;

      hp->Fit("gaus","RQ0","",loli,hili);
      TF1 *f1 = hp->GetFunction("gaus");
      p = f1->GetParameter(1);
      double s = f1->GetParameter(2);
      hili = p+s;
      loli = p-s;
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
	  if (!(j%2)){
	    hp->GetXaxis()->SetTitle("#Deltat [ns]");
	    char hnam1[128];
	    sprintf(hnam1,"Projection #Deltat to paddel %d REFPAD%d plane%d",j-2,REFPAD,REFPLANE);
	    hp->SetTitle(hnam1);
	    gStyle->SetOptFit(1); 
	    hp->Draw();
	    //	    f1->Draw("same");
	    gPad->SetLogz(0);
	    gPad->Update();
	    sprintf(hnam1,"plots/deltat_fitposition_p%d_repad%d_plane%d.pdf",j-2,REFPAD,REFPLANE);
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

  sprintf(of,"calibration%d/deltat_tpos_refpad%dplane%d.root",RunNumber,REF,REFPLANE);
  TFile *f = new TFile(of,"RECREATE");
  histTD->Write();
  f->Close();

} 

