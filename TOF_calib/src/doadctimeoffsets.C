// apply all timing offsets to TDC time and overall timing correction for ADC time
// then calculate time difference between adc and tdc time and deterine offsets
// T_tdc - T_adc = offset_t_adc
// the sign of the parameters will be flipped to be in accordance with the analysis code


#include "TTree.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TCanvas.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int DEBUG = 0;

TH2F *TDiff;
int FIRST = 0;
void saverootfile(int R){

  char fnam[128];
  sprintf(fnam,"calibration%d/adc_time_offsets_run%d.root",R,R);

  TFile *RF = new TFile(fnam,"RECREATE");
  RF->cd();

  TDiff->Write();
    
  RF->Close();

}


void doadctimeoffsets(int RunNumber){

  float LOLIM = -10.;
  float HILIM = 10.;

  if (!FIRST){
    TDiff = new TH2F("TDiff","TDC_T - ADC_T",176, 0,176, 200, LOLIM, HILIM);
  } else {
    TDiff->Reset();
  }
  FIRST++;

  char fnam[128];
  char ROOTFILE[128];
  if (RunNumber == 99999){
    sprintf(ROOTFILE,"localdir/big%d.root",RunNumber);
  } else {
    sprintf(ROOTFILE,"localdir/tofdata_run%d.root",RunNumber);
  }

  // get TDC timing offsets
  sprintf(fnam,"calibration%d/tofpmt_tdc_offsets_all_FULL_run%d.DB",RunNumber,RunNumber);
  cout<<"offsetfile: "<<fnam<<endl;
  ifstream INF;
  INF.open(fnam);
  float PMTOffsets[176];
  for (int k=0;k<176;k++){
    INF>>PMTOffsets[k];
    //cout<<k<<"  "<<PMTOffsets[k]<<endl;
  }
  INF.close();
 

  // get global ADC and TDC time offsets from ccdb database;
  sprintf(fnam,"src/base_times.dat");
  char cmd[256];
  sprintf(cmd,"ccdb dump /TOF/base_time_offset:%d:: > %s", RunNumber, fnam);
  system(cmd);
  INF.open(fnam);
  char dummy[128];
  INF.getline(dummy,120);
  float ADC_T_OFFSET, TDC_T_OFFSET;
  INF>>ADC_T_OFFSET>>TDC_T_OFFSET;
  INF.close();


  // get walk correction parameters
  double WalkPar[176][3];
  double dum;
  sprintf(fnam,"calibration%d/tof_walk_parameters_run%d.DB",RunNumber,RunNumber);
  INF.open(fnam);
  for (int k=0;k<176;k++){
    INF>>dum>>WalkPar[k][0]>>WalkPar[k][1]>>WalkPar[k][2];
  }
  INF.close();

  TFile *f = new TFile(ROOTFILE,"READ");
  //f->ls();
  TTree *t3 = (TTree*)f->Get("TOFcalib/t3");
  
  
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
  int OFL[100];
  int OFR[100];

  int NsinglesA;
  int PlaneSA[100];
  int PaddleSA[100];
  int LRA[100];
  float ADCS[100];
  float TADCS[100];
  int OF[100];

  int NsinglesT;
  int PlaneST[100];
  int PaddleST[100];
  int LRT[100];
  float TDCST[100];

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
  t3->SetBranchAddress("OFL",OFL);
  t3->SetBranchAddress("OFR",OFR);
 
  t3->SetBranchAddress("NsinglesA",&NsinglesA);
  t3->SetBranchAddress("PlaneSA",PlaneSA);
  t3->SetBranchAddress("PaddleSA",PaddleSA);
  t3->SetBranchAddress("LRA",LRA);
  t3->SetBranchAddress("ADCS",ADCS);
  t3->SetBranchAddress("TADCS",TADCS);
  t3->SetBranchAddress("OF",OF);

  t3->SetBranchAddress("NsinglesT",&NsinglesT);
  t3->SetBranchAddress("PlaneST",PlaneST);
  t3->SetBranchAddress("PaddleST",PaddleST);
  t3->SetBranchAddress("LRT",LRT);
  t3->SetBranchAddress("TDCST",TDCST);

  unsigned int nentries = (unsigned int) t3->GetEntries();
  cout<<"Start reading root file with "<<nentries<<" events ..."<<endl;
  for (unsigned int kk=0; kk<nentries; kk++){
    t3->GetEntry(kk);
    //cout<<Event<<" "<<Nhits<<" "<<NhitsA<<" "<<NsinglesA<<" "<<NsinglesT<<endl;

    float THESHIFT = TShift;
    
    if (1) { // make offset 0: TEMPORARY FIX
      int S = (int)TShift / (int)4;
      S -= 1;
      if (S<1)
        S = 6;
      
      THESHIFT = (float)S * 4.;
    }
    

    for (int i=0; i<Nhits;i++){ // loop over double ended paddles TDC hits
      int paddle = Paddle[i];
      int plane = Plane[i];
      
      for (int n=0; n<NhitsA;n++){      
        if ( (paddle == PaddleA[n]) &&
             (plane == PlaneA[n])) {   // find matching ADC hit
          float pmtL = ADCL[n];
          float pmtR = ADCR[n];
          float tL = MeanTime[i]-TimeDiff[i];
          float tR = MeanTime[i]+TimeDiff[i];
          int hid1 = plane*88 + paddle - 1;
          int hid2 = plane*88 + 44 + paddle - 1;

          // apply TDC offsets
          tL -= PMTOffsets[hid1];
          tR -= PMTOffsets[hid2];

          // apply walk correciton
          float tcL = WalkPar[hid1][0] * ( pow(ADCL[n],WalkPar[hid1][1]) -  
                                           pow(WalkPar[hid1][2],WalkPar[hid1][1]));
          float tcR = WalkPar[hid2][0] * ( pow(ADCR[n],WalkPar[hid2][1]) -  
                                           pow(WalkPar[hid2][2],WalkPar[hid2][1]));
          tL -= tcL;
          tR -= tcR;

	  tL += TDC_T_OFFSET;
	  tR += TDC_T_OFFSET;
 
          float tLADC = MeanTimeA[i]-TimeDiffA[i];
          float tRADC = MeanTimeA[i]+TimeDiffA[i];

	  tLADC += ADC_T_OFFSET;
	  tRADC += ADC_T_OFFSET;

	  TDiff->Fill(hid1, tL - tLADC + THESHIFT);
	  TDiff->Fill(hid2, tR - tRADC + THESHIFT);

	}
      }
    }


    for (int i=0; i<NsinglesA; i++) { // loop over singles hits
      int plane = PlaneSA[i];
      int paddle = PaddleSA[i];
      int side = LRA[i];
      int idx = plane*88 + side*44 + paddle - 1;
      float tADC = TADCS[i];

      for (int j=0; j<NsinglesT; j++){ // loop over TDC single hits
	
	if ((PlaneST[j] == plane ) && (PaddleST[j] == paddle)){
	  // found matching TDC hit
	  float walk = WalkPar[idx][0] * ( pow(ADCS[i],WalkPar[idx][1]) -  
                                           pow(WalkPar[idx][2],WalkPar[idx][1]));
	  float T = TDCST[j] - walk;  // apply walk correction 
	  T -= PMTOffsets[idx];       // apply pmt timing offset
	  T += TDC_T_OFFSET;          // apply global TDC time offset

	  tADC += ADC_T_OFFSET;       // apply global ADC time offset

 	  TDiff->Fill(idx, T - tADC + THESHIFT);

	}
      }
    }

  }

  if (DEBUG){
    TDiff->Draw("colz");
    gPad->Update();
    getchar();
  }

  float ADC_T_offsets[176];

  for (int k=0;k<176;k++) {

    ADC_T_offsets[k] = 0.;
    //cout<<"bin "<<k+1<<endl;
    TH1D *h = TDiff->ProjectionY("h",k+1,k+1);
    if (h->GetEntries()>100){
      double pos = 0;
      double sig = 0;
      int maxbin = h->GetMaximumBin();
      float maxloc = h->GetBinCenter(maxbin);

      float lowlim = maxloc - 1.;
      float higlim = maxloc + 1.;
      if (lowlim < LOLIM){
	lowlim = LOLIM;
      }
      if (higlim > HILIM) {
	higlim = HILIM;
      }
      
      h->Fit("gaus","Q","R", lowlim, higlim);
      TF1 *func = h->GetFunction("gaus");
      pos = func->GetParameter(1);
      sig = func->GetParameter(2);

      lowlim = pos - 2.*sig;
      higlim = pos + 2.*sig;
      if (lowlim < LOLIM){
	lowlim = LOLIM;
      }
      if (higlim > HILIM) {
	higlim = HILIM;
      }
      
      h->Fit("gaus","Q","R", lowlim, higlim);
      func = h->GetFunction("gaus");
      pos = func->GetParameter(1);
      sig = func->GetParameter(2);

      ADC_T_offsets[k] = -pos; //change sign

      if (DEBUG){
	h->Draw();
	gPad->Update();
	getchar();
      }

    }  

  }


  sprintf(fnam,"calibration%d/tdc_adc_time_offsets_run%d.DB",RunNumber,RunNumber);
  ofstream OUTF;
  OUTF.open(fnam);
  for (int k=0;k<176;k++) {
    OUTF << ADC_T_offsets[k]<<endl;
  }
 
  OUTF.close();

  sprintf(fnam,"calibration%d/tdc_adc_times_run%d.root",RunNumber,RunNumber);
  TFile *RootF = new TFile(fnam, "RECREATE");
  RootF->cd();
  TDiff->Write();
  RootF->Close();

  
  
}
