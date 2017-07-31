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

using namespace std;

TH2F *Twalk[176];
TF1 *AllFits[2][176];
TGraphErrors *onedplots[176];


int DEBUG = 91; // 2 = make plots, 99 = interupt

double  fithist(TH2F*, double*, int , int, int, int);
TGraphErrors *mkprof(TH2F*, int);
int RunNumber ;

void walk1(int Run){
  
  RunNumber = Run;
  char ROOTFileName[128];
  sprintf(ROOTFileName,"localdir/run%d/hd_root_tofcalib.root",RunNumber);
  sprintf(ROOTFileName,"localdir/tofdata_run%d.root",RunNumber);
  if (RunNumber == 99999)
    sprintf(ROOTFileName,"localdir/big%d.root",RunNumber);

  // create a 2-d histogram for each PMT (176) with horizontal axis ADC
  // and vertical axis timedifference 
  for (int n=0; n<176; n++){
    char hnam[128];
    sprintf(hnam,"Twalk%d",n);
    char htit[128];
    sprintf(htit,"E vs. T %d",n);
    Twalk[n] = new TH2F(hnam,htit,200, 0.,4096., 500, 250.,310.);
    //Twalk[n] = new TH2F(hnam,htit,200, 0.,24000., 500, 250.,350.);
    Twalk[n]->GetXaxis()->SetTitle("Signal Amplitude [adc]");
  }

  // prepare root file and tree for reading
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
	  float tL = MeanTime[i]-TimeDiff[i];
	  float tR = MeanTime[i]+TimeDiff[i];
	  float adcTL = MeanTimeA[n]-TimeDiffA[n] ;
	  float adcTR = MeanTimeA[n]+TimeDiffA[n] ;
	  int hid1 = plane*88 + paddle - 1;
	  int hid2 = plane*88 + 44 + paddle - 1;
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
	  float adct = TADCS[n]; 
	  float tdct = TDCST[i];
	  int idx = 88*plane + s*44 + paddle -1;
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

  // now do the walk determintion for all 176 PMTs
  double FitPar[176][17];
  double allp[17];
  double CHI2[176];
  for (int n=0; n<176; n++){
    if (Twalk[n]->GetEntries()>100){
      // fit 2-D histogram using profile 
      int plane  = n/88;
      int side = (n - 88*plane)/44;
      int paddle = n - 88*plane - side*44;
      cout<<"Paddle "<<n<< ": do walk fit"<<endl; 
      CHI2[n] = fithist(Twalk[n], allp, plane, paddle, side, n);

      for (int i=0;i<17;i++){
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
  TH1D *MeanOffset = new TH1D("MeanOffsetTDC","Mean TDC Time Offset w.r.t. ADCs",500, 220., 320.);
  ofstream OUTF1;
  sprintf(outf, "calibration%d/tof_TDC_ADC_timediff_run%d.dat",RunNumber,RunNumber);
  OUTF1.open(outf);
  double TheOffsets[176];
  for (int n=0; n<176; n++){
    OUTF<<n;
    for (int s=0;s<17;s++){
      OUTF<<"   "<<FitPar[n][s];
    }
    OUTF<<"    "<<CHI2[n]<<endl;

    TheOffsets[n] = FitPar[n][8]
      +FitPar[n][10]*TMath::Power(1500.,-0.5) 
      +FitPar[n][12]*TMath::Power(1500.,-0.33) 
      +FitPar[n][14]*TMath::Power(1500.,-0.2);

    MeanOffset->Fill(TheOffsets[n]);
    OUTF1<<n<<"  "<<TheOffsets[n]<<"       "<<CHI2[n]<<endl;
  }
  OUTF.close();
  OUTF1.close();

  // create DB file to loaded to ccdb data base
  sprintf(outf, "calibration%d/tof_walk_parameters_run%d.DB",RunNumber,RunNumber);
  OUTF.open(outf);
  for (int n=0; n<176; n++){
    OUTF<< FitPar[n][0]<<"   " << FitPar[n][2] 
	<<"   "<< FitPar[n][4]<<"   "<< FitPar[n][6]
	<<"   "<< FitPar[n][8]<<"   " << FitPar[n][10] 
	<<"   "<< FitPar[n][12]<<"   "<< FitPar[n][14]
	<<"   "<< FitPar[n][16]
	<<"  1500."<<endl ;
  }
  OUTF.close();
  
  MeanOffset->Draw();
  sprintf(outf,"plots/meanoffset_tADCminustTDC_run%d.pdf",RunNumber);
  gPad->SaveAs(outf);
  
  // calculate offsets for TDC vs. ADC and mean global offset
  // between TDC and ADC
  double mean = MeanOffset->GetMean();
  double rms = MeanOffset->GetRMS();
  double loli = mean-3.*rms;
  double hili = mean+3.*rms;
  MeanOffset->Fit("gaus","RQ","",loli,hili);
  MeanOffset->Draw();
  double CenterOffset = MeanOffset->GetFunction("gaus")->GetParameter(1);
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
  for (int n=0; n<176; n++){
    OUTF<<CenterOffset-TheOffsets[n]<<endl;
  }
  OUTF.close();

  char rfile[128];
  sprintf(rfile,"calibration%d/walk_results_run%d.root",RunNumber,RunNumber);
  TFile *Rout = new TFile(rfile,"RECREATE");
  Rout->cd();
  for (unsigned int k=0;k<176;k++){ 
    Twalk[k]->Write();    
    onedplots[k]->Write();
  }  
  for (unsigned int k=0;k<176;k++){ 
    if (AllFits[0][k]){
      AllFits[0][k]->Write();
      if (AllFits[1][k]){
	AllFits[1][k]->Write();
      }
    }
 }  
  MeanOffset->Write();
  Rout->Close();

}

double fithist(TH2F *hist, double *allp, int plane, int paddle, int side, int idx){

  // make profile plot from hist
  TGraphErrors *grnew = mkprof(hist, idx);
  onedplots[idx] = grnew;
  
  //two independend fit functions for two regions
  TF1 *f1 = new TF1("f1", "[0]+ [1]*pow(x,-0.5) + [2]*pow(x,-0.33)+ [3]*pow(x,-0.2)",150.,500.);
  TF1 *f2 = new TF1("f2", "[0]+ [1]*pow(x,-0.5) + [2]*pow(x,-0.33)+ [3]*pow(x,-0.2)",200.,3950.);

  // determine the connection point between the two regions of fit
  TH1D *response = hist->ProjectionX("response", 1,hist->GetYaxis()->GetNbins()-1);
  double max = response->GetBinContent(6);
  int k = 16;
  while (response->GetBinContent(k)<max){
    max = response->GetBinContent(k);
    cout<<k<<"  "<<max<<endl;
    k++;
  }
  double ConectPoint = response->GetBinCenter(k-1);
  if (ConectPoint<300){
    ConectPoint = 300.;
  }

  f1->SetParameter(0, 286.);
  f1->SetParameter(1, 100.);
  f1->SetParameter(2, -11.);
  f1->SetParameter(3, -1.);

  f1->SetParLimits(0, 270., 300.);

  f2->SetParameter(0, 286.);
  f2->SetParameter(1, 100.);
  f2->SetParameter(2, -11.);
  f2->SetParameter(3, -1.);

  f2->SetParLimits(0, 200., 300.);
  

  TFitResultPtr r = grnew->Fit(f1, "S", "R", 150., ConectPoint);
  grnew->GetYaxis()->SetRangeUser(270.,300.);
  TF1 *thefit = grnew->GetFunction("f1");
  char fitnam[128];
  sprintf(fitnam,"fit1hist%d",idx);
  thefit->SetName(fitnam);
  AllFits[0][idx] = (TF1*)thefit->Clone();
  AllFits[0][idx]->SetLineColor(4);
  double chi2 =  r->Chi2() / r->Ndf(); 
  for (int k=0;k<4;k++){
    allp[2*k] = thefit->GetParameter(k);
    allp[2*k+1] = thefit->GetParError(k);
    //cout<< allp[2*k]<<"  "<<allp[2*k+1]<<endl;
  }

  r = grnew->Fit(f2, "S", "R", ConectPoint, 3900.);
  thefit = grnew->GetFunction("f2");
  sprintf(fitnam,"fit2hist%d",idx);
  thefit->SetName(fitnam);
  AllFits[1][idx] = (TF1*)thefit->Clone();
  chi2 +=  r->Chi2() / r->Ndf(); 
  chi2 /=2;
  int OFFS = 8;
  for (int k=0;k<4;k++){
    allp[2*k+OFFS] = thefit->GetParameter(k);
    allp[2*k+1+OFFS] = thefit->GetParError(k);
    //cout<< allp[2*k]<<"  "<<allp[2*k+1]<<endl;
  }
  allp[16] = ConectPoint;

  if (DEBUG) {
    hist->Draw("colz");
    char hnam[128];
    sprintf(hnam,"Time vs Energy with Walk Correction Fit PMT %d", idx);
    hist->SetTitle(hnam);
    hist->GetXaxis()->SetTitle("ADC Amplitude");
    hist->GetYaxis()->SetTitle("Time Difference T_{TDC}-T_{ADC} [ns]");
    grnew->Draw("same AP");
    gPad->SetGrid();
    gPad->Update();
    AllFits[0][idx]->Draw("same");
    AllFits[1][idx]->Draw("same");

    if (DEBUG>90){
	sprintf(hnam,"plots/walk_correction_pl%d_pad%d_side%d_run%d.pdf",plane, paddle,side,RunNumber);
	gPad->SaveAs(hnam);
    }

    if (DEBUG==99){
      getchar();
    }

  }

  return chi2;
}

TGraphErrors *mkprof(TH2F *h, int id){

  int nbins = h->GetXaxis()->GetNbins();

  double X[600];
  double Y[600];
  double dY[600];

  int cnt = 0;
  for (int k=7;k<nbins;k++){

    X[cnt] = h->GetXaxis()->GetBinCenter(k);
    TH1D *p = (TH1D*)h->ProjectionY("p",k,k);

    if (p->GetEntries()<10){
      continue;
    }

    int mb = p->GetMaximumBin();
    double max = p->GetBinCenter(mb);
    double maxold = max;
    double hili = max + 4.*p->GetBinWidth(mb);
    double loli = max - 4.*p->GetBinWidth(mb);
    p->Fit("gaus","","R",loli,hili);
    TF1 *f = p->GetFunction("gaus");
    max = f->GetParameter(1);
    double sig = f->GetParameter(2);
    loli = max - 1.5*sig;
    hili = max + 1.5*sig;
    p->Fit("gaus","","R",loli,hili);
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


