// 
// generate 2-D histogram of the time-difference (T_TDC-TADC) vs. 
// energy (ADC Amplitude)
// and fit the profile to a functional form to determine the TDC walk.
// The time difference between TDC and ADC time needs to be corrected for
// the 6 fold ambiguity between the 24ns and 4ns clocks of the TDC and the ADC.
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
#include <TGraphErrors.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <fstream>
#include <iostream>
#include <TLine.h>

using namespace std;

#define NumPMTMax 200

int NPMTS = 0;
TH2F *Twalk[NumPMTMax];
TF1 *AllFits[2][NumPMTMax];
TGraphErrors *onedplots[NumPMTMax];

int DEBUG = 2; // 2 = make plots, 99 = interupt

double  fithist(TH2F*, double*, int , int, int, int);
TGraphErrors *mkprof(TH2F*, int);
int RunNumber ;

int BARS_PER_PLANE = 0; // including 2 short padeles being one
int PMTS_PER_PLANE = 0; 


float CenterTime = 69.;
//
// main start function walk1()
//
void walk1(int Run){
  RunNumber = Run;
  NPMTS = 176;            // TOF 1 geometry

  char TOFDIRcmd[128];
  sprintf(TOFDIRcmd, "ccdb dump TOF/base_time_offset:%d > calib.log", RunNumber);

  if (RunNumber>69999){
    NPMTS = 184;          // TOF 2 geometry
    sprintf(TOFDIRcmd, "ccdb dump TOF2/base_time_offset:%d > calib.log", RunNumber);
  }
  BARS_PER_PLANE = NPMTS/4;
  PMTS_PER_PLANE = NPMTS/2;

  // first get base time offset from ccdb data base
  system(TOFDIRcmd);
  ifstream INF1("calib.log");
  if (INF1 == NULL){
    cout<<"Error no calibration file found! BAIL NOW!"<<endl;
    return;
  }
  string dummy1;
  getline( INF1, dummy1);
  float ADCoffset = 0.;
  float TDCoffset = 0.;
  INF1>>ADCoffset>>TDCoffset;
  INF1.close();

  cout<<"ADC and TDC global time offsets(from ccdb): "<<ADCoffset<< "  " << TDCoffset<<endl;
  //  return;


  char ROOTFileName[128];
  //sprintf(ROOTFileName,"localdir/run%d/hd_root_tofcalib.root",RunNumber);
  sprintf(ROOTFileName,"localdir/tofdata_run%d.root",RunNumber);
  if (RunNumber == 99999)
    sprintf(ROOTFileName,"localdir/big%d.root",RunNumber);

  TFile *ROOTFile = new TFile(ROOTFileName);
  TH2D *TDCtime2d = (TH2D*) ROOTFile->Get("TOFcalib/TOFTDCtime");
  TH2D *ADCtime2d = (TH2D*) ROOTFile->Get("TOFcalib/TOFADCtime");
  TH1D *TDCpj = (TH1D*)TDCtime2d->ProjectionX("TDCpj", 1, TDCtime2d->GetYaxis()->GetNbins());
  TH1D *ADCpj = (TH1D*)ADCtime2d->ProjectionX("ADCpj", 1, ADCtime2d->GetYaxis()->GetNbins());

  int mbt = TDCpj->GetMaximumBin();
  int mba = ADCpj->GetMaximumBin();
  float c = TDCpj->GetBinCenter(mbt) - ADCpj->GetBinCenter(mba);
  float l1 = (float)((int)c - 50);
  float l2 = (float)((int)c + 50);

  CenterTime = c+15;

  ROOTFile->Close();

  // create a 2-d histogram for each PMT  with horizontal axis ADC
  // and vertical axis timedifference 
  for (int n=0; n<NPMTS; n++){
    char hnam[128];
    sprintf(hnam,"Twalk%d",n);
    char htit[128];
    sprintf(htit,"E vs. T %d",n);
    //Twalk[n] = new TH2F(hnam,htit,200, 0.,4096., 500, 250.,310.);
    //Twalk[n] = new TH2F(hnam,htit,200, 0., 4096., 1000, l1, l2);
    Twalk[n] = new TH2F(hnam,htit,200, 0., 4096., 400, -10., 10.);
    Twalk[n]->GetXaxis()->SetTitle("Signal Amplitude [adc]");
  }

  // prepare root tree for reading
  ROOTFile = new TFile(ROOTFileName);
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


  // loop over events in the tree
  cout<<"Start reading root file ..."<<endl;
  unsigned int nentries = (unsigned int) t3->GetEntries();
  cout<<"Number of Entries "<<nentries;
  if (nentries>100000000){
    nentries = 100000000;
    cout<<"  use only 100000000";
  }
  cout<<endl;
  //nentries = 1000000;

  for (unsigned int k=0; k<nentries; k++){
    t3->GetEntry(k);
    //cout<<Event<<" "<<Nhits<<endl;
    if (!(k%10000000)){
      TDatime a;
      cout<<"Current time is "<<a.Get()<<endl;
    }


    // loop over TDC hits and find matching ADC hits
    // then determine time difference between the time of the TDC and
    // the time of the ADC and fill 2-d histogram with (ADC,dt)


    float THESHIFT = TShift;


    for (int i=0; i<Nhits;i++){
      int paddle = Paddle[i];
      int plane = Plane[i];
      
      for (int n=0; n<NhitsA;n++){	
	if ( (paddle == PaddleA[n]) &&
	     (plane == PlaneA[n])) {	  

	  // only use those matching paddles that have more or less
	  // also goto matching time difference 

	  float pmtL = PEAKL[n];
	  float pmtR = PEAKR[n];
	  float tL = MeanTime[i]-TimeDiff[i] + TDCoffset;
	  float tR = MeanTime[i]+TimeDiff[i] + TDCoffset;
	  float adcTL = MeanTimeA[n]-TimeDiffA[n] + ADCoffset ;
	  float adcTR = MeanTimeA[n]+TimeDiffA[n] + ADCoffset ;
	  int hid1 = plane*PMTS_PER_PLANE + paddle - 1;
	  int hid2 = plane*PMTS_PER_PLANE + BARS_PER_PLANE + paddle - 1;
	  //cout<<tL-adcTL<<endl;
	  if ( (!OFR[n]) && (!OFL[n]) ){
	    //bool c = (((tL-adcTL+THESHIFT)>248.) && (pmtL>3000)) || ((tL-adcTL+THESHIFT)<240.);
	    //c = false;
	    //if (!c)
	    Twalk[hid1]->Fill(pmtL, tL-adcTL+THESHIFT);
	    //cout<< tL-adcTL+THESHIFT<<endl;
	    //c = (((tR-adcTR+THESHIFT)>248) && (pmtR>3000)) || ((tR-adcTR+THESHIFT)<240.);
	    //if (!c)

	    Twalk[hid2]->Fill(pmtR, tR-adcTR+THESHIFT);
	    
	  }
	  
	}
      }
    }

    //loop over single ended paddles and do the same
    for (int i=0; i<NsinglesT;i++){
      int paddle = PaddleST[i];
      int plane = PlaneST[i];
      int s = LRT[i];
      for (int n=0; n<NsinglesA;n++){	
	if ( (paddle == PaddleSA[n]) &&
	     (plane == PlaneSA[n]) && 
	     (s == LRA[n])) {	  
	  
	  float pmt = PEAK[n];
	  float adct = TADCS[n] + ADCoffset; 
	  float tdct = TDCST[i] + TDCoffset;
	  int idx = plane*PMTS_PER_PLANE + s*BARS_PER_PLANE + paddle -1;
	  if (!OF[n]){
	    //bool c = (((tdct-adct+THESHIFT)>248.) && (pmt>2000)) || ((tdct-adct+THESHIFT)<240.);
	    //if (!c)
	    Twalk[idx]->Fill(pmt, tdct-adct+THESHIFT);
	  }
	}
      }
    }
  }

  ROOTFile->Close();
  cout<<".... done reading"<<endl;

  if (1) {
    char rfile[128];
    sprintf(rfile,"calibration%d/walk_histos_run%d.root",RunNumber,RunNumber);
    TFile *Rout = new TFile(rfile,"RECREATE");
    Rout->cd();
    for (int k=0;k<NPMTS;k++){ 
      Twalk[k]->Write(); 
    }  

    Rout->Close();
  }

  // now do the walk determintion for all PMTs
  double FitPar[NumPMTMax][17];
  double allp[17];
  double CHI2[NumPMTMax];

  //
  // loop over PMTs and apply fit to data with fit function defined in fithist()
  //
  for (int n=0; n<NPMTS; n++){
    TH1F *hpjy = (TH1F*)Twalk[n]->ProjectionY("hpjy", 195, 195);
    if (((Twalk[n]->GetEntries()>100) || hpjy->GetEntries()>10) ){
      // fit 2-D histogram using profile 
      int plane  = n/PMTS_PER_PLANE;
      int side = (n - PMTS_PER_PLANE*plane)/BARS_PER_PLANE;
      int paddle = n - PMTS_PER_PLANE*plane - side*BARS_PER_PLANE;
      cout<<"Paddle "<<n<< ": do walk fit"<<endl; 
      CHI2[n] = fithist(Twalk[n], allp, plane, paddle, side, n);

      for (int i=0;i<16;i++){
	FitPar[n][i] = allp[i];
      }
    } else {
      for (int s=0;s<17;s++){
	FitPar[n][s] = 0.;
      }
    }
  }
  
  char outf[128];
  sprintf(outf, "calibration%d/tof_walk_parameters_run%d.dat",RunNumber,RunNumber);
  ofstream OUTF;
  OUTF.open(outf);
  TH1D *MeanOffsetTDC = new TH1D("MeanOffsetTDC","Walk Correction Value at ADC = 1500",
			      500, -500., 500);
  MeanOffsetTDC->GetXaxis()->SetTitle("walk corr. at ADC=1500 [ns]");
  ofstream OUTF1;
  sprintf(outf, "calibration%d/tof_TDC_ADC_timediff_run%d.dat",RunNumber,RunNumber);
  OUTF1.open(outf);
  double TheOffsets[NumPMTMax];
  for (int n=0; n<NPMTS; n++){
    OUTF<<n;
    for (int s=0;s<16;s++){
      OUTF<<"   "<<FitPar[n][s];
    }
    OUTF<<endl;

    TheOffsets[n] = FitPar[n][0]
      +FitPar[n][2]/1500. 
      +FitPar[n][4]/1500./1500.
      +FitPar[n][6]/1500./1500./1500./1500.
      +FitPar[n][8]/TMath::Sqrt(1500.);

    MeanOffsetTDC->Fill(TheOffsets[n]);
    OUTF1<<n<<"  "<<TheOffsets[n]<<"       "<<CHI2[n]<<endl;
  }

  OUTF.close();
  OUTF1.close();

  // create DB file to loaded to ccdb data base
  sprintf(outf, "calibration%d/tof_walk_parameters_run%d.DB",RunNumber,RunNumber);
  OUTF.open(outf);
  for (int n=0; n<NPMTS; n++){
    for (int j=0;j<7; j++){
      OUTF<< FitPar[n][j*2]<<"   ";
    }
    OUTF<<FitPar[n][14]<<"  "<<FitPar[n][15]; // chi2 and start if fit of F1    
    OUTF<<"  1500."<<endl ;    // chose as reference.
  }

  OUTF.close();

  // make marker to specify the vesion of walk correction used:
  sprintf(outf, "calibration%d/walkcortype.log", RunNumber);
  OUTF.open(outf);
  OUTF<<"4"<<endl;
  OUTF.close();
  
  MeanOffsetTDC->Draw();
  sprintf(outf,"plots/meanoffset_tADCminustTDC_run%d.pdf",RunNumber);
  gPad->SaveAs(outf);
  
  // calculate offsets for TDC vs. ADC and mean global offset
  // between TDC and ADC
  double mean = MeanOffsetTDC->GetMean();
  double rms = MeanOffsetTDC->GetRMS();
  double loli = mean-3.*rms;
  double hili = mean+3.*rms;
  MeanOffsetTDC->Fit("gaus","RQ","",loli,hili);
  MeanOffsetTDC->Draw();
  double CenterOffset = MeanOffsetTDC->GetFunction("gaus")->GetParameter(1);
  //cout<<CenterOffset<<endl;

  // the ADC global time offset minus this value give the TDC global time offset
  sprintf(outf, "calibration%d/adc_vs_tdc_relative_global_time_run%d.DB",RunNumber,RunNumber);
  OUTF.open(outf);
  OUTF<<CenterOffset<<endl;
  OUTF.close();

  sprintf(outf, "calibration%d/adc_vs_tdc_relative_time_run%d.DB",RunNumber,RunNumber);
  OUTF.open(outf);
  // CenterOffset is mean time offset between ADC and TDC
  // TheOffsets[n] is nth pmt offset w.r.t. CenterOffset
  for (int n=0; n<NPMTS; n++){
    OUTF<<CenterOffset-TheOffsets[n]<<endl;
  }
  OUTF.close();

  char rfile[128];
  sprintf(rfile,"calibration%d/walk_results_run%d.root",RunNumber,RunNumber);
  TFile *Rout = new TFile(rfile,"RECREATE");
  Rout->cd();
  for (int k=0;k<NPMTS;k++){ 
    Twalk[k]->Write(); 
    if (onedplots[k]){
      onedplots[k]->Write();
    }
  }
  for (int k=0;k<NPMTS;k++){ 
    if (AllFits[0][k]){
      AllFits[0][k]->Write();
    }
    if (AllFits[1][k]){
      AllFits[1][k]->Write();
    }
  }  
  MeanOffsetTDC->Write();
  Rout->Close();

}

double fithist(TH2F *hist, double *allp, int plane, int paddle, int side, int idx){
  
  //
  // make profile plot from histogram "hist"
  //
  TGraphErrors *graph = mkprof(hist, idx);
  onedplots[idx] = graph;
  
  // new fit functions with total number of fit parameters 5+2 = 7
  TF1 *f1 = new TF1("f1", "[0] + [1]/x + [2]/(x*x) + [3]/(x*x)/(x*x) + [4]/sqrt(x)",150.,4000.);
  TF1 *F1 = new TF1("F1", "[0] + [1]/x + [2]/(x*x) + [3]/(x*x)/(x*x) + [4]/sqrt(x)",150.,4000.);
  TF1 *f2 = new TF1("f2", "[0] +  [1]*x", 1500., 4000.);
  TF1 *F2 = new TF1("F2", "[0] +  [1]*x", 1500., 4000.);
  
  f1->SetParameter(0, 0.);
  f1->SetParameter(1, -100.);
  f1->SetParameter(2, 1000.);
  f1->SetParameter(3, -1.e8);
  f1->SetParameter(4, 10.);

  cout<<"Fit f1 for PMT: "<<idx<<endl;
  // get 3rd bin in X of the graph
  double ax, ay;
  graph->GetPoint(2, ax, ay);
  f1->SetParLimits(0,-100, ay+(TMath::Abs(ay)*0.5));

  // set range limits for fit parameters [1] to [4]
  f1->SetParLimits(1, -10000000., 0.);
  f1->SetParLimits(2, 0., 10000000.);
  f1->SetParLimits(3, -10e10, 0.);
  f1->SetParLimits(4, 0., 10000. );
  
  float f1limit = ax-1.;
  cout<<"start fit at "<<ax<<endl;
  TFitResultPtr res = (TFitResultPtr)graph->Fit(f1, "SQ", "R", f1limit, 3900.);
  double Chi2 = res->Chi2() / res->Ndf(); 
  cout<<"chi2 = "<<Chi2<<endl;
  if (Chi2>500){
    f1->SetParameter(0, -2.7);
    f1->SetParameter(1, -100.);
    f1->SetParameter(2, 1000.);
    f1->SetParameter(3, -1.e8);
    f1->SetParameter(4, 10.);
    f1->SetParLimits(1, -10000000., 0.);
    f1->SetParLimits(2, 0., 10000000.);
    f1->SetParLimits(3, -10e10, 0.);
    f1->SetParLimits(4, 0., 10000. );

    //res = (TFitResultPtr)graph->Fit(f1, "SQ", "R", f1limit*.95, 3000.);
    res = (TFitResultPtr)hist->Fit(f1,"SQ", "R", f1limit, 3000);
    hist->Draw();
    gPad->Update();
    //getchar();
    Chi2 = res->Chi2() / res->Ndf(); 
    cout<<"SECOND ROUND FIT: chi2 = "<<Chi2<<endl;
  }
  if (Chi2>1000){
    f1->SetParameter(0, -2.7);
    f1->SetParLimits(1, -10000000., 0.);
    f1->SetParLimits(2, 0., 10000000.);
    f1->SetParLimits(3, -10e10, 0.);
    f1->SetParLimits(4, 0., 10000. );

    //res = (TFitResultPtr)graph->Fit(f1, "SQ", "R", f1limit*0.95, 2000.);
    res = (TFitResultPtr)hist->Fit(f1, "SQ", "R", f1limit, 2000.);
    hist->Draw();
    gPad->Update();
    //getchar();
    Chi2 = res->Chi2() / res->Ndf(); 
    cout<<"THIRD ROUND FIT: chi2 = "<<Chi2<<endl;
  }

  TF1 *fres = (TF1*)graph->GetFunction("f1");
  for (int k=0;k<5;k++){
    F1->SetParameter(k, fres->GetParameter(k));
    allp[2*k] = fres->GetParameter(k);
    allp[2*k+1] = fres->GetParError(k);
  }
  F1->SetLineColor(4);

  if (DEBUG>98){
    graph->Draw("AP");
    F1->Draw("same");
    gPad->SetGrid();
    gPad->Update();
    getchar();
  }


  if (0) {
    f2->SetParameter(0, 1.);
    f2->SetParameter(1, -0.001);
    int nbins = graph->GetN();
    graph->GetPoint(nbins-1, ax, ay);
    if (ax<2000.){
      for (int k=0;k<2;k++){
	allp[2*k+10] = 0.;
	allp[2*k+1+10] = 0.;
	AllFits[1][idx] = (TF1*)f2->Clone();
	allp[15] = f1limit;
	allp[14] = Chi2;
      }
      return Chi2;
    }
  }

  return Chi2; // never go passt here anymore

  cout<<"Fit f2 for PMT: "<<idx<<endl;
  TFitResultPtr res1 = graph->Fit(f2, "SQ", "R", 2000, 3900.);
  Chi2 += res->Chi2() / res->Ndf(); 
  fres = (TF1*)graph->GetFunction("f2");
  int OFFS = 10;
  for (int k=0;k<2;k++){
    F2->SetParameter(k, fres->GetParameter(k));
    allp[2*k+OFFS] = fres->GetParameter(k);
    allp[2*k+1+OFFS] = fres->GetParError(k);
  }
  F2->SetLineColor(2);

  if (DEBUG>98){
    graph->Draw("AP");
    F2->Draw("same");
    gPad->SetGrid();
    gPad->Update();
    getchar();
  }

  char fitnam[128];
  sprintf(fitnam,"fit1hist%d",idx);
  F1->SetName(fitnam);
  AllFits[0][idx] = (TF1*)F1->Clone();
  AllFits[0][idx]->SetLineColor(4);

  sprintf(fitnam,"fit2hist%d",idx);
  F2->SetName(fitnam);
  AllFits[1][idx] = (TF1*)F2->Clone();
  allp[15] = f1limit;
  allp[14] = Chi2;

  if (DEBUG) {
    hist->Draw("colz");
    gPad->SetLogz(1);
    char hnam[128];
    sprintf(hnam,"Time vs Energy with Walk Correction Fit PMT %d", idx);
    hist->SetTitle(hnam);
    hist->GetXaxis()->SetTitle("ADC Amplitude");
    hist->GetYaxis()->SetTitle("Time Difference T_{TDC}-T_{ADC} [ns]");
    graph->Draw("same AP");
    graph->GetXaxis()->SetRangeUser(20., 4000.);
    gPad->SetGrid();
    gPad->Update();

    F1->SetLineColor(2);
    F1->SetLineWidth(2);
    F1->Draw("same");
    F2->SetLineColor(4);
    F2->SetLineWidth(4);
    F2->Draw("same");

    gPad->Update();

    if (DEBUG>90){
      if (!(idx%5)||(DEBUG>98))
      sprintf(hnam,"plots/walk_correction_pl%d_pad%d_side%d_run%d.pdf",plane, paddle,side,RunNumber);
      gPad->SaveAs(hnam);
    }

    if (DEBUG==99){
      getchar();
    }

  }

  return Chi2;
}

TGraphErrors *mkprof(TH2F *h, int id){

  int nbins = h->GetXaxis()->GetNbins();

  double X[600];
  double Y[600];
  double dY[600];

  int cnt = 0;
  cout<<"Create profile histogram for PMT: "<<id<<endl;
  for (int k=5;k<nbins;k++){

    X[cnt] = h->GetXaxis()->GetBinCenter(k);
    TH1D *p = (TH1D*)h->ProjectionY("p",k,k);

    if (p->GetEntries()<50){
      continue;
    }

    if (p->GetMaximum()<1){
      continue;
    }
    
    int mb = p->GetMaximumBin();
    double max = p->GetBinCenter(mb);
    double maxold = max;
    double hili = max + 4.*p->GetBinWidth(mb);
    double loli = max - 4.*p->GetBinWidth(mb);

    // quit if limits are off
    if ( (hili > p->GetBinCenter(p->GetXaxis()->GetNbins()) ) || 
	 (loli < p->GetBinCenter(1)) ){
      continue;
    }

    // quit if no data in range
    double SUM = 0;
    for (int j=p->FindBin(loli); j<p->FindBin(hili)+1; j++){
      SUM += p->GetBinContent(j);
    }
    if (SUM<10){
      continue;
    }

    p->Fit("gaus","Q","R",loli,hili);

    TF1 *f = p->GetFunction("gaus");
    max = f->GetParameter(1);
    double sig = f->GetParameter(2);
    loli = max - 1.5*sig;
    hili = max + 1.5*sig;

    // quit if limits are off
    if ( (hili > p->GetBinCenter(p->GetXaxis()->GetNbins()) ) || 
	 (loli < p->GetBinCenter(1)) ){
      continue;
    }

    //quit if no data in range
    SUM = 0;
    for (int j=p->FindBin(loli); j<p->FindBin(hili)+1; j++){
      SUM += p->GetBinContent(j);
    }
    if (SUM<10){
      continue;
    }

    p->Fit("gaus","Q","R",loli,hili);
    f = p->GetFunction("gaus");

    double fact =1.;
    if (k<25){
      fact = 1.;
    }

    Y[cnt] = f->GetParameter(1); 
    dY[cnt] = f->GetParError(1)*fact; 
    cnt++;
    

  }

  TGraphErrors *gr = new TGraphErrors(cnt, X, Y, NULL, dY);
  char nam[128];
  sprintf(nam,"gr_%d",id);
  gr->SetName(nam);
  sprintf(nam,"Projection PMT %d",id);
  gr->SetTitle(nam);


  return gr;

}


