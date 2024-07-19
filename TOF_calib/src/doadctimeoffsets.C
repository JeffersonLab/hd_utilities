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
int RunNumber = 0;

#define NumPMTMax 200
int NPMTS = 0;
int BARS_PER_PLANE = 0; // including 2 short padeles being one
int PMTS_PER_PLANE = 0; 

void doadctimeoffsets(int Run){

  TH2D *TDiff;
  TH2D *TDiffF;


  RunNumber = Run;

  NPMTS = 176;            // TOF 1 geometry
  if (RunNumber>69999){
    NPMTS = 184;          // TOF 2 geometry
  }
  BARS_PER_PLANE = NPMTS/4;
  PMTS_PER_PLANE = NPMTS/2;
 
  double LOLIM = -20.;
  double HILIM = 20.;

  TH1D *TDiffSingles[NPMTS];
  
  TDiff  = new TH2D("TDiff",  "TDC_T - ADC_T", 
		    NumPMTMax, 0., (double)NumPMTMax, 
		    400, LOLIM, HILIM);
  TDiffF = new TH2D("TDiffF","TDC_T - ADC_T", 
		    NumPMTMax, 0., (double)NumPMTMax, 
		    400, LOLIM*5., HILIM*5.);
  
  char fnam[128];
  char ROOTFILE[128];
  if (RunNumber == 99999){
    sprintf(ROOTFILE,"localdir/big%d.root",RunNumber);
  } else {
    sprintf(ROOTFILE,"localdir/tofdata_run%d.root",RunNumber);
  }

  // get TDC timing offsets
  sprintf(fnam,"calibration%d/tofpmt_tdc_offsets_all_FULL_run%d.DB",RunNumber,RunNumber);
  //cout<<"read TDC timing offsetfile: "<<fnam<<endl;
  ifstream INF;
  INF.open(fnam);
  double PMTOffsets[NumPMTMax];
  for (int k=0;k<NPMTS;k++){
    INF>>PMTOffsets[k];
    //cout<<k<<"  "<<PMTOffsets[k]<<endl;
  }
  INF.close();
 

  // get global ADC and TDC time offsets from ccdb database;
  sprintf(fnam,"src/base_times.dat");
  char cmd[256];
  if (RunNumber < 70000){
    sprintf(cmd,"ccdb dump /TOF/base_time_offset:%d:: > %s", RunNumber, fnam);
  }else {
    sprintf(cmd,"ccdb dump /TOF2/base_time_offset:%d:: > %s", RunNumber, fnam);
  }
  system(cmd);
  INF.open(fnam);
  char dummy[128];
  INF.getline(dummy,120);
  double ADC_T_OFFSET, TDC_T_OFFSET;
  INF>>ADC_T_OFFSET>>TDC_T_OFFSET;
  INF.close();


  // get walk correction parameters
  double WalkPar[NumPMTMax][17];
  double dum;
  int idx;
  sprintf(fnam,"calibration%d/tof_walk_parameters_run%d.dat",RunNumber,RunNumber);
  INF.open(fnam);
  for (int k=0;k<NPMTS;k++){
    INF >> idx;
    for (int s=0;s<15;s++) {
      INF >> WalkPar[k][s];
    }
    INF>>dummy; // this is the chi2 of both fits
  }
  INF.close();
  double ReferenceLoc = 1500.;  // this is the reference ADC value to calibrate to. any walk correction is zero to this point

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
  float PEAKL[100];
  float PEAKR[100];

  int NsinglesA;
  int PlaneSA[100];
  int PaddleSA[100];
  int LRA[100];
  float ADCS[100];
  float TADCS[100];
  int OF[100];
  float PEAK[100];

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
  t3->SetBranchAddress("PEAKL",PEAKL);
  t3->SetBranchAddress("PEAKR",PEAKR);
 
  t3->SetBranchAddress("NsinglesA",&NsinglesA);
  t3->SetBranchAddress("PlaneSA",PlaneSA);
  t3->SetBranchAddress("PaddleSA",PaddleSA);
  t3->SetBranchAddress("LRA",LRA);
  t3->SetBranchAddress("ADCS",ADCS);
  t3->SetBranchAddress("TADCS",TADCS);
  t3->SetBranchAddress("OF",OF);
  t3->SetBranchAddress("PEAK",PEAK);

  t3->SetBranchAddress("NsinglesT",&NsinglesT);
  t3->SetBranchAddress("PlaneST",PlaneST);
  t3->SetBranchAddress("PaddleST",PaddleST);
  t3->SetBranchAddress("LRT",LRT);
  t3->SetBranchAddress("TDCST",TDCST);

  unsigned int nentries = (unsigned int) t3->GetEntries();
  cout<<"Number of Entries "<<nentries<<endl;
  for (unsigned int kk=0; kk<nentries; kk++){
    t3->GetEntry(kk);
    //cout<<Event<<" "<<Nhits<<" "<<NhitsA<<" "<<NsinglesA<<" "<<NsinglesT<<endl;
    if (!(kk%10000000)){
      TDatime a;
      cout<<"Current time is "<<a.Get()<<endl;
    }

    double THESHIFT = (double)TShift;

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
          int idx1 = plane*PMTS_PER_PLANE + paddle - 1;    // north/top
          int idx2 = plane*PMTS_PER_PLANE + BARS_PER_PLANE + paddle - 1;   //south/bottom

	  if (0) {
	    if ( (idx2 == 63) && (PEAKR[n]<300))
	      continue;
	  }

	  if ( (OFR[n]) || (OFL[n]) ){
	    continue;
	  }

          // apply TDC offsets
          tL -= PMTOffsets[idx1];
          tR -= PMTOffsets[idx2];

          // apply walk correciton
	  double ADCval = PEAKL[n];
	  if (ADCval>4090){
	    ADCval = 4090;
	  }
	  double a1 = WalkPar[idx1][0]
	    +WalkPar[idx1][2]/ADCval
	    +WalkPar[idx1][4]/ADCval/ADCval
	    +WalkPar[idx1][6]/ADCval/ADCval/ADCval/ADCval
	    +WalkPar[idx1][8]/TMath::Sqrt(ADCval);
	  
	  // ReferenceLoc is chosen to be 1500. ADC counts
	  double a2 = WalkPar[idx1][0]
	    +WalkPar[idx1][2]/ReferenceLoc
	    +WalkPar[idx1][4]/ReferenceLoc/ReferenceLoc
	    +WalkPar[idx1][6]/ReferenceLoc/ReferenceLoc/ReferenceLoc/ReferenceLoc
	    +WalkPar[idx1][8]/TMath::Sqrt(ReferenceLoc);
	  
	  float tcL = a1 - a2;
	  
	  ADCval = PEAKR[n];
	  if (ADCval>4090){
	    ADCval = 4090;
	  }
	  a1 = WalkPar[idx2][0]
	    +WalkPar[idx2][2]/ADCval
	    +WalkPar[idx2][4]/ADCval/ADCval
	    +WalkPar[idx2][6]/ADCval/ADCval/ADCval/ADCval
	    +WalkPar[idx2][8]/TMath::Sqrt(ADCval);

	  // ReferenceLoc is chosen to be 1500. ADC counts
	  a2 = WalkPar[idx2][0]
	    +WalkPar[idx2][2]/ReferenceLoc
	    +WalkPar[idx2][4]/ReferenceLoc/ReferenceLoc
	    +WalkPar[idx2][6]/ReferenceLoc/ReferenceLoc/ReferenceLoc/ReferenceLoc
	    +WalkPar[idx2][8]/TMath::Sqrt(ReferenceLoc);

          double tcR = a1 - a2;
	  
	  tL -= (float)tcL;
          tR -= (float)tcR;

	  tL += TDC_T_OFFSET;
	  tR += TDC_T_OFFSET;
 
          double tLADC = (double)MeanTimeA[n] - (double)TimeDiffA[n];
          double tRADC = (double)MeanTimeA[n] + (double)TimeDiffA[n];

	  tLADC += ADC_T_OFFSET;
	  tRADC += ADC_T_OFFSET;

	  /*
	  if (idx2 == 157){
	    cout<<"idx2="<<idx2<<endl;
	    cout<<tR<<"    "<<PMTOffsets[idx2]<<endl;
	  }
	  */

	  TDiff->Fill(idx1, tL - tLADC + THESHIFT);
	  TDiff->Fill(idx2, tR - tRADC + THESHIFT);
	  TDiffF->Fill(idx1, tL - tLADC + THESHIFT);
	  TDiffF->Fill(idx2, tR - tRADC + THESHIFT);

	}
      }
    }


    for (int i=0; i<NsinglesA; i++) { // loop over singles hits
      int plane = PlaneSA[i];
      int paddle = PaddleSA[i];
      int side = LRA[i];
      int idx = plane*PMTS_PER_PLANE + side*BARS_PER_PLANE + paddle - 1;
      double tADC = (double)TADCS[i];

      for (int j=0; j<NsinglesT; j++){ // loop over TDC single hits
	
	if ((PlaneST[j] == plane ) && (PaddleST[j] == paddle)){
	  // found matching TDC hit
	  // now apply walk correction to hit
	  double ADCval = PEAK[i];
	  if (ADCval>4090){
	    ADCval = 4090;
	  }

	  double a1 = WalkPar[idx][0]
	    +WalkPar[idx][2]/ADCval
	    +WalkPar[idx][4]/ADCval/ADCval
	    +WalkPar[idx][6]/ADCval/ADCval/ADCval/ADCval
	    +WalkPar[idx][8]/TMath::Sqrt(ADCval);
	  
	  // ReferenceLoc is chosen to be 1500. ADC counts
	  double a2 = WalkPar[idx][0]
	    +WalkPar[idx][2]/ReferenceLoc
	    +WalkPar[idx][4]/ReferenceLoc/ReferenceLoc
	    +WalkPar[idx][6]/ReferenceLoc/ReferenceLoc/ReferenceLoc/ReferenceLoc
	    +WalkPar[idx][8]/TMath::Sqrt(ReferenceLoc);
	  
	  double walk = a1 - a2;

	  double T = (double)TDCST[j] - walk;  // apply walk correction 
	  T -= PMTOffsets[idx];       // apply pmt timing offset
	  T += TDC_T_OFFSET;          // apply global TDC time offset

	  tADC += ADC_T_OFFSET;       // apply global ADC time offset

 	  TDiff->Fill(idx,  T - tADC + THESHIFT);
 	  TDiffF->Fill(idx, T - tADC + THESHIFT);

	}
      }
    }

  }

  if (DEBUG){
    TDiff->Draw("colz");
    gPad->SetLogz(1);
    gPad->Update();
    getchar();
    TDiffF->Draw("colz");
    gPad->SetLogz(1);
    gPad->Update();
    getchar();
  }

  float ADC_T_offsets[NumPMTMax];

  for (int k=0;k<NPMTS;k++) {

    ADC_T_offsets[k] = 0.;
    //cout<<"bin "<<k+1<<endl;
    string hnamek = "histDt"+to_string(k);
      TDiffSingles[k] = (TH1D*)TDiffF->ProjectionY(hnamek.c_str(),k+1,k+1)->Clone();
    //    TH1D *h = TDiffF->ProjectionY("h",k+1,k+1);
    TH1D *h = TDiffSingles[k];

    if (h->GetEntries()>100){

      char hnam1[128];
      sprintf(hnam1,"TDCtime - ADCtime (all corrected) PMT #%d ",k+1);
      h->SetTitle(hnam1);
      h->GetXaxis()->SetTitle("time difference [ns]");

      double pos = 0;
      double sig = 0;
      int maxbin = h->GetMaximumBin();
      float maxloc = h->GetBinCenter(maxbin);

      if (h->GetBinContent(maxbin)<1){
	cout<<"Error histogram projection empty!"<<endl;
	continue;
      }

      float lowlim = maxloc - 1.;
      float higlim = maxloc + 1.;
      if (lowlim < LOLIM){
	lowlim = LOLIM;
      }
      if (higlim > HILIM) {
	higlim = HILIM;
      }

      if (DEBUG) {
	cout<<lowlim<<" / "<<higlim<<endl;
	h->Draw();
	gPad->Update();
	getchar();
      }
      
      h->Fit("gaus","Q","R", lowlim, higlim);
      TF1 *func = h->GetFunction("gaus");
      pos = func->GetParameter(1);
      sig = func->GetParameter(2);

      lowlim = pos - 1.7*sig;
      higlim = pos + 1.7*sig;
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
  cout<<"write ADC timing offsetfile: "<<fnam<<endl;
  ofstream OUTF;
  OUTF.open(fnam);
  for (int k=0;k<NPMTS;k++) {
    OUTF << ADC_T_offsets[k]<<endl;
  }
 
  OUTF.close();

  sprintf(fnam,"calibration%d/tdc_adc_times_run%d.root",RunNumber,RunNumber);
  cout<<"write root file with histograms and fit for ADC timing offsets: "<<fnam<<endl;
  TFile *RootF = new TFile(fnam, "RECREATE");
  RootF->cd();
  TDiff->Write();
  TDiffF->Write();
  for (int k=0; k<NPMTS; k++){
    TDiffSingles[k]->Write();
  }

  RootF->Close();

}
