//-----------------------------------------------------------------------
// Determine timing offsets for single ended paddles
// crate root file adchists_runXXXX.root with root tree and histograms
// create full DB file for timing offsets
// if in debug mode 10 quite at this point in the code otherwise continue
// to fit the adc spectra.
// histograms: TimingN    TDC offsets of single ended pmt M
//             xTvsxEN    xpos from TDC vs xpos from E for paddle N
//             ADCHistsN  ADC Integral for PMT N for central hit
//             xTvsEPMTN  xpos from TDC vs ADC Integral for pmt N
//-----------------------------------------------------------------------

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

TH1D *ADCHists[179];
TH2D *xTvsxE[88];
TH2D *E0vsxE[88];
TH1F *Timings[8];
TH2D *TDiff_TDCvsADC[88];

TH2D *xTvsEPMT[179];
TH2D *xTvsEPMTcorr[179];

int DEBUG = 10;
int use_current_calibration = 1;

TH2D *h2d;
void saverootfile(int R){

  char fnam[128];
  sprintf(fnam,"calibration%d/adchists_run%d.root",R,R);

  TFile *RF = new TFile(fnam,"RECREATE");
  RF->cd();

  for (int k=0;k<8;k++){
    Timings[k]->Write();
  }
  for (int k=0;k<88;k++){
    xTvsxE[k]->Write();
    E0vsxE[k]->Write();
    TDiff_TDCvsADC[k]->Write();
  }
  for (int k=0;k<176;k++){
    ADCHists[k]->Write();
    xTvsEPMT[k]->Write();
    xTvsEPMTcorr[k]->Write();
  }
    
  RF->Close();

}

Double_t langaufun(Double_t *x, Double_t *par) {

   //Fit parameters:
   //par[0]=Width (scale) parameter of Landau density
   //par[1]=Most Probable (MP, location) parameter of Landau density
   //par[2]=Total area (integral -inf to inf, normalization constant)
   //par[3]=Width (sigma) of convoluted Gaussian function
   //
   //In the Landau distribution (represented by the CERNLIB approximation), 
   //the maximum is located at x=-0.22278298 with the location parameter=0.
   //This shift is corrected within this function, so that the actual
   //maximum is identical to the MP parameter.

      // Numeric constants
      Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
      Double_t mpshift  = -0.22278298;       // Landau maximum location

      // Control constants
      Double_t np = 100.0;      // number of convolution steps
      Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

      // Variables
      Double_t xx;
      Double_t mpc;
      Double_t fland;
      Double_t sum = 0.0;
      Double_t xlow,xupp;
      Double_t step;
      Double_t i;


      // MP shift correction
      mpc = par[1] - mpshift * par[0]; 

      // Range of convolution integral
      xlow = x[0] - sc * par[3];
      xupp = x[0] + sc * par[3];

      step = (xupp-xlow) / np;

      // Convolution integral of Landau and Gaussian by sum
      for(i=1.0; i<=np/2; i++) {
         xx = xlow + (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);

         xx = xupp - (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);
      }

      return (par[2] * step * sum * invsq2pi / par[3]);
}



TF1 *langaufit(TH1D *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
{
   // Once again, here are the Landau * Gaussian parameters:
   //   par[0]=Width (scale) parameter of Landau density
   //   par[1]=Most Probable (MP, location) parameter of Landau density
   //   par[2]=Total area (integral -inf to inf, normalization constant)
   //   par[3]=Width (sigma) of convoluted Gaussian function
   //
   // Variables for langaufit call:
   //   his             histogram to fit
   //   fitrange[2]     lo and hi boundaries of fit range
   //   startvalues[4]  reasonable start values for the fit
   //   parlimitslo[4]  lower parameter limits
   //   parlimitshi[4]  upper parameter limits
   //   fitparams[4]    returns the final fit parameters
   //   fiterrors[4]    returns the final fit errors
   //   ChiSqr          returns the chi square
   //   NDF             returns ndf

   Int_t i;
   Char_t FunName[100];

   sprintf(FunName,"Fitfcn_%s",his->GetName());

   TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
   if (ffitold) delete ffitold;

   TF1 *ffit = new TF1(FunName,langaufun,fitrange[0],fitrange[1],4);
   //TF1 *ffit = new TF1(FunName,langaufun,4000.,13000.,4);
   ffit->SetParameters(startvalues);
   ffit->SetParNames("Width","MP","Area","GSigma");
   
   for (i=0; i<4; i++) {
      ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
   }

   his->Fit(FunName,"RQ0","",fitrange[0],fitrange[1]);   // fit within specified range, use ParLimits, do not plot
   //his->Fit(FunName,"R0" ,"",5000.,13000.);   // fit within specified range, use ParLimits, do not plot

   ffit->GetParameters(fitparams);    // obtain fit parameters
   for (i=0; i<4; i++) {
      fiterrors[i] = ffit->GetParError(i);     // obtain fit parameter errors
   }
   ChiSqr[0] = ffit->GetChisquare();  // obtain chi^2
   NDF[0] = ffit->GetNDF();           // obtain ndf

   return (ffit);              // return fit function

}


Int_t langaupro(Double_t *params, Double_t &maxx, Double_t &FWHM) {

   // Seaches for the location (x value) at the maximum of the 
   // Landau-Gaussian convolute and its full width at half-maximum.
   //
   // The search is probably not very efficient, but it's a first try.

   Double_t p,x,fy,fxr,fxl;
   Double_t step;
   Double_t l,lold;
   Int_t i = 0;
   Int_t MAXCALLS = 10000;


   // Search for maximum

   p = params[1] - 0.1 * params[0];
   step = 0.05 * params[0];
   lold = -2.0;
   l    = -1.0;


   while ( (l != lold) && (i < MAXCALLS) ) {
      i++;

      lold = l;
      x = p + step;
      l = langaufun(&x,params);
 
      if (l < lold)
         step = -step/10;
 
      p += step;
   }

   if (i == MAXCALLS)
      return (-1);

   maxx = x;

   fy = l/2;


   // Search for right x location of fy

   p = maxx + params[0];
   step = params[0];
   lold = -2.0;
   l    = -1e300;
   i    = 0;


   while ( (l != lold) && (i < MAXCALLS) ) {
      i++;

      lold = l;
      x = p + step;
      l = TMath::Abs(langaufun(&x,params) - fy);
 
      if (l > lold)
         step = -step/10;
 
      p += step;
   }

   if (i == MAXCALLS)
      return (-2);

   fxr = x;


   // Search for left x location of fy

   p = maxx - 0.5 * params[0];
   step = -params[0];
   lold = -2.0;
   l    = -1e300;
   i    = 0;

   while ( (l != lold) && (i < MAXCALLS) ) {
      i++;

      lold = l;
      x = p + step;
      l = TMath::Abs(langaufun(&x,params) - fy);
 
      if (l > lold)
         step = -step/10;
 
      p += step;
   }

   if (i == MAXCALLS)
      return (-3);


   fxl = x;

   FWHM = fxr - fxl;
   return (0);
}

TF1*  langaus(TH1D *hist, Double_t &Peak, Double_t &FWHM) {

   // Fitting histogram
   printf("Fitting...\n");

   // Setting fit range and start values
   Double_t FitRange[2];
   Double_t StartValues[4], LimitLow[4], LimitHigh[4], fp[4], fpe[4];
   //hist->GetXaxis()->SetRangeUser(500.,15000.);

   double w = hist->GetBinWidth(1);
   int start = hist->FindBin(2000.);
   int stop = hist->FindBin(14000.);
   double max = 0;
   int bin = -1;
   for (int n=start;n<stop;n+=5){
     double sum = 0;
     for (int i=0;i<5;i++){
       sum += hist->GetBinContent(n+i);
     }
     if (sum>max){
       max = sum;
       bin = n;
     }
   }
   if (bin<0){
     return NULL;
   }
   max = hist->GetBinCenter(bin);
   FitRange[0] = max - 2500.;//0.5*hist->GetMean();
   FitRange[1] = 14000.;
   cout<<"Fit RANGE: "<< FitRange[0] <<" "<< FitRange[1] <<endl;

   LimitHigh[3]=5.0;
   StartValues[0]=1000.; //Width
   StartValues[1]=max; //MP
   StartValues[2]=hist->GetEntries(); //Area
   StartValues[3]=10.0; //GSigma


   LimitHigh[0] = 5000.0;
   LimitLow[0]  = 0.5;
   LimitHigh[1] = 13000.0;
   LimitLow[1]  = 4000.0;
   LimitHigh[2] = hist->GetEntries()*500.;
   LimitLow[2]  = hist->GetEntries()*0.01;
   LimitHigh[3] = 3000.0;
   LimitLow[3]  = 0.001;

   Double_t chisqr;
   Int_t    ndf;
   TF1 *fitfunc = langaufit(hist,FitRange,StartValues,
			    LimitLow,LimitHigh,fp,fpe,&chisqr,&ndf);
   
   // fit second time with adjusted fit range
   //FitRange[0] = fp[1]-fp[0]*3.;
   //FitRange[0] = fp[1]-2000.;
   //fitfunc = langaufit(hist,FitRange,StartValues,
   //		       LimitLow,LimitHigh,fp,fpe,&chisqr,&ndf);

   langaupro(fp,Peak,FWHM);

   return fitfunc;
}

void dofitall(int R , int dbgmode){

  DEBUG = dbgmode;

  h2d = new TH2D("h2d","of vs. e", 200, 0., 24000., 10, 0., 10.);
  int RunNumber = R;
  char fnam[128];
  if (RunNumber == 99999){
    sprintf(fnam,"localdir/big%d.root",RunNumber);
  } else {
    sprintf(fnam,"localdir/tofdata_run%d.root",RunNumber);
  }

  for (int k=0;k<176;k++){
    char hnam[128];
    sprintf(hnam,"ADCHists%d",k);
    char htit[128];
    sprintf(htit,"ADC Integral PMT%d",k);
    ADCHists[k] = new TH1D(hnam,htit,200, 0.,24000.);

    sprintf(hnam,"xTvsEPMT%d",k);
    sprintf(htit,"XPos vs PMTIntegral %d",k);
    if ( (k==21) || (k==22) || (k==21+44) || (k==22+44) || (k==21+88) || (k==22+88) || (k==21+44+88) || (k==22+44+88)){
      xTvsEPMT[k] = new TH2D(hnam,htit,200, 0., 24000., 44, 0., 44.);
    } else {
      xTvsEPMT[k] = new TH2D(hnam,htit,200, 0., 24000., 40, 0., 260.);
    }
    sprintf(hnam,"xTvsEPMTcorr%d",k);
    sprintf(htit,"XPos vs PMTIntegral-corr %d",k);
    xTvsEPMTcorr[k] = new TH2D(hnam,htit,200, 0., 24000., 40, 0., 260.);
  }

  for (int k=0;k<88;k++){
    char hnam[128];
    sprintf(hnam,"xTvsxE%d",k);
    char htit[128];
    sprintf(htit,"xposT vs xposE %d",k);
    xTvsxE[k] = new TH2D(hnam,htit,400, -150.,150., 100, -150.,150);    

    sprintf(hnam,"E0vsxE%d",k);
    sprintf(htit,"E0 vs xposE paddleID%d",k);
    E0vsxE[k] = new TH2D(hnam, htit, 400, -150.,150., 1000, 0., 65000.);    


    sprintf(hnam,"TDiff_TDCvsADC%d",k);
    sprintf(htit,"TimeDifference TDC vs ADC paddleID %d",k);
    TDiff_TDCvsADC[k] = new TH2D(hnam, htit, 240, -10., 10. , 240, -10., 10.);
    TDiff_TDCvsADC[k]->GetXaxis()->SetTitle("#Delta T ADC [ns]");
    TDiff_TDCvsADC[k]->GetYaxis()->SetTitle("#Delta T TDC calibrated [ns]");
  }


  for (int k=0;k<8;k++){
    char hnam[128];
    sprintf(hnam,"Timing%d",k);
    char htit[128];
    int pl = k/4;
    int u = (k-pl*4)/2;
    int pad = 22 + (k%2);
    sprintf(htit,"PMT_time - MT_REF plane %d Paddle %d pmtside %d",pl,pad,u);
    Timings[k] = new TH1F(hnam,htit,500,-10.,10.);
    //Timings[k] = new TH1F(hnam,htit,200,-6.,0.);
    Timings[k]->GetXaxis()->SetTitle("T-MTref [ns]");
  }

  TFile *f = new TFile(fnam,"READ");
  //f->ls();
  TTree *t3 = (TTree*)f->Get("TOFcalib/t3");

  sprintf(fnam,"calibration%d/tofpmt_tdc_offsets_all_run%d.DB",RunNumber,RunNumber);
  cout<<"offsetfile: "<<fnam<<endl;
  ifstream INF;
  INF.open(fnam);
  float PMTOffsets[176];
  for (int k=0;k<176;k++){
    INF>>PMTOffsets[k];
    //cout<<k<<"  "<<PMTOffsets[k]<<endl;
  }
  INF.close();

  double Speeds[88];
  if (use_current_calibration) {
    sprintf(fnam,"calibration%d/tofpaddles_propagation_speed_run%d.DB",RunNumber,RunNumber);
    INF.open(fnam);
    if (!INF){
      cout<<"Error open file: "<<fnam<<endl;
      cout<<"Stop right here....!"<<endl;
      return;
    }
    cout<<"Use propacation speeds from current calibration data!!!!!"<<endl;
    int dum1;
    double dum2;
    for (int k=0;k<88;k++){
      INF>>dum1>>Speeds[k]>>dum2;
    }
    INF.close();
  } else {
    // get paddle effective light velocity for file or data base
    sprintf(fnam,"src/Paddle_velocities.dat");
    struct stat buffer;   
    int fromDB = 0;
    if ( stat(fnam, &buffer) == 0) {
      cout<<fnam<<" NOT FOUND! Try to read from data base.."<<endl;
      fromDB = 1;
      system("ccdb dump TOF/propagation_speed > src/Paddle_velocities.dat");
      INF.open(fnam);
      char dummy[128];
      INF.getline(dummy,120);
    } else {
      INF.open(fnam);
    }

    for (int k=0;k<88;k++){
      INF>>Speeds[k];
    }
  
    INF.close();
    
    // create proper format for src/Paddle_velocities.dat file if needed
    if (fromDB) {
      ofstream OUTF("src/Paddle_velocities.dat");
      if (OUTF) {
	for (int k=0;k<88;k++){
	  OUTF<<Speeds[k];
	}
	OUTF.close();
      }
    }

  }



  // get walk correction parameters
  double timewalk_parameters_AMP[176][6];
  double dummy;
  sprintf(fnam,"calibration%d/tof_walk_parameters_run%d.DB",RunNumber,RunNumber);
  INF.open(fnam);
  for (int k=0;k<176;k++){
    INF >> timewalk_parameters_AMP[n][0] >> timewalk_parameters_AMP[n][1] >> timewalk_parameters_AMP[n][2] 
        >> timewalk_parameters_AMP[n][3] >> timewalk_parameters_AMP[n][4] >> timewalk_parameters_AMP[n][5];
  }
  INF.close();

  // create histograms!!!!!! here!!!!!
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
  int OFL[100];
  int OFR[100];

  int NsinglesA;
  int PlaneSA[100];
  int PaddleSA[100];
  int LRA[100];
  float ADCS[100];
  float PEAK[100];
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
  t3->SetBranchAddress("PEAKL",PEAKL);
  t3->SetBranchAddress("PEAKR",PEAKR);
  t3->SetBranchAddress("OFL",OFL);
  t3->SetBranchAddress("OFR",OFR);
 
  t3->SetBranchAddress("NsinglesA",&NsinglesA);
  t3->SetBranchAddress("PlaneSA",PlaneSA);
  t3->SetBranchAddress("PaddleSA",PaddleSA);
  t3->SetBranchAddress("LRA",LRA);
  t3->SetBranchAddress("ADCS",ADCS);
  t3->SetBranchAddress("PEAK",PEAK);
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
    
    int CenterHits[2][2];
    memset(CenterHits,0,16);
    int CenterHitsALL[2][44][2];
    memset(CenterHitsALL,0,2*44*2*4);
    float MeanTimeRef[2][2] = {0., 0., 0., 0.};

    for (int i=0; i<Nhits;i++){
      int paddle = Paddle[i];
      int plane = Plane[i];
      
      for (int n=0; n<NhitsA;n++){      
        if ( (paddle == PaddleA[n]) &&
             (plane == PlaneA[n])) {      
          float pmtL = ADCL[n];
          float pmtR = ADCR[n];
          float tL = MeanTime[i]-TimeDiff[i];
          float tR = MeanTime[i]+TimeDiff[i];
          int hid1 = plane*88 + paddle - 1;
          int hid2 = plane*88 + 44 + paddle - 1;

	  // apply TDC offsets
	  tL -= PMTOffsets[hid1];
	  tR -= PMTOffsets[hid2];

          int IDX[2];
          IDX[0] = idxL;
          IDX[1] = idxR;
          double AMP[2];
          AMP[0] = PEAKL[n]; 
          AMP[1] = PEAKR[n]; 
          float corr[2];
          
          //apply walk correction
          for (int s=0; s<2;s++) {
            double C0 = timewalk_parameters_AMP[IDX[s]][0];
            double C1 = timewalk_parameters_AMP[IDX[s]][1];
            double C2 = timewalk_parameters_AMP[IDX[s]][2];
            double C3 = timewalk_parameters_AMP[IDX[s]][3];
            double hookx = timewalk_parameters_AMP[IDX[s]][4]; 
            double refx = timewalk_parameters_AMP[IDX[s]][5];
            double val_at_ref = C0 + C1*pow(refx,C2); 
            double val_at_hook = C0 + C1*pow(hookx,C2); 
            double slope = (val_at_hook - C3)/hookx;
            if (refx>hookx){
              val_at_ref  = slope * refx + C3; 
            }
            double val_at_A = C0 + C1*pow(AMP[s],C2);
            if (AMP[s]>hookx){
              val_at_A = slope * AMP[s] + C3; 
            }            
            corr[i] = val_at_A - val_at_ref;
	    //cout<<i<<":  "<<hookx<<"  "<<refx<<"   "<<AMP[i]<<" "<<slope<<"  "<<val_at_A<<"  "<<val_at_ref<<endl; 
            //cout<<"        "<<C0<<"  "<<C1<<"  "<<C2<<endl;

          }

	  tL -= corr[0];
	  tR -= corr[1];

	  float DT = (tR-tL)/2.;

	  //cout<<kk<<" "<<DT<<endl;

	  int idx = 44*plane+paddle-1;
	  TDiff_TDCvsADC[idx]->Fill(TimeDiffA[n],DT); 

	  double xT = (tR-tL)/2.*Speeds[idx];
	  double xE = TMath::Log(pmtL/pmtR)/2.*100.;
	  double E0 = TMath::Sqrt(pmtL*pmtR)*TMath::Exp(126./100.);
	  if (E0>350.)
	    xTvsxE[idx]->Fill(xE,xT);

	  E0vsxE[idx]->Fill(xE,E0);

	  xTvsEPMT[hid1]->Fill(pmtL,126.-xT);
	  xTvsEPMT[hid2]->Fill(pmtR,126.+xT);
	  
	  // remove attenuation and correct energy deposition to
	  // the center of the paddle.
	  // Short attenuation length is 76 cm
	  // Long attenuation legnth is 485 cm 
	  double dist = 126.-xT; 
	  double corr = TMath::Exp(-dist/76.) +  TMath::Exp(-dist/485.); 
	  xTvsEPMTcorr[hid1]->Fill(pmtL/corr,126.-xT);
	  dist = 126.+xT; 
	  corr = TMath::Exp(-dist/76.) +  TMath::Exp(-dist/485.); 
	  xTvsEPMTcorr[hid2]->Fill(pmtR/corr,126.+xT);
	  
	  if (TMath::Abs(DT)<0.25){
	    if (paddle < 22){ // overlaps with R/D
	      CenterHits[plane][1]++;
	      MeanTimeRef[plane][1] = (tR+tL)/2.;; 
	      CenterHitsALL[plane][paddle-1][1]++;
	    }
	    if (paddle > 23){ // overlaps with L/U
	      CenterHits[plane][0]++;
	      MeanTimeRef[plane][0] = (tR+tL)/2.;; 
	      CenterHitsALL[plane][paddle-1][0]++;
	    }
	    if (hid1==18){
	      h2d->Fill(pmtL/2.,(double)OFL[n]);
	    }
	    if (OFL[n]<1) {
		ADCHists[hid1]->Fill(pmtL);
	    }
	    if (OFR[n]<1){
		ADCHists[hid2]->Fill(pmtR);
	    }
	  }
        }
      }
    } //loop over all full paddles

    for (int i=0; i<NsinglesA; i++) {
      int plane = PlaneSA[i];
      int paddle = PaddleSA[i];
      int side = LRA[i];
      int idx = plane*88 + side*44 + paddle -1;
      int p = 1;
      int s = side;

      float T = TDCST[i];

      // calculate walk correction
      double C0 = timewalk_parameters_AMP[idx][0];
      double C1 = timewalk_parameters_AMP[idx][1];
      double C2 = timewalk_parameters_AMP[idx][2];
      double C3 = timewalk_parameters_AMP[idx][3];
      double hookx = timewalk_parameters_AMP[idx][4]; 
      double refx = timewalk_parameters_AMP[idx][5];
      double val_at_ref = C0 + C1*pow(refx,C2); 
      double val_at_hook = C0 + C1*pow(hookx,C2); 
      double slope = (val_at_hook - C3)/hookx;
      if (refx>hookx){
	val_at_ref  = slope * refx + C3; 
      }
      double val_at_A = C0 + C1*pow(PEAK[i],C2);
      if (AMP[i]>hookx){
	val_at_A = slope * PEAK[i] + C3; 
      }
      
      float walk = val_at_A - val_at_ref;

      T -= walk;

      if (plane){
	p = 0;
      }
      if (CenterHits[p][s]){
	if (OF[i]<1){
	  if (side){ 
	    if (CenterHitsALL[p][20][side]){
	      ADCHists[idx]->Fill(ADCS[i]); 
	    }
	  } else {
	    if (CenterHitsALL[p][23][side]){
	      ADCHists[idx]->Fill(ADCS[i]); 
	    }
	  }
	}
	int idxt = plane*4 + (paddle-22) + side*2;
	Timings[idxt]->Fill(T - MeanTimeRef[p][s] );
      }

      for (int n=0; n<44; n++) {
	if (CenterHitsALL[p][n][s]){
	  float x = (float)n;
	  xTvsEPMT[idx]->Fill(ADCS[i],x);
	}
      }
    }
  }
  cout<<"Finished reading root file. closing file"<<endl;
  //f->Close();

  saverootfile(RunNumber);
  if (DEBUG == 99){
    return;
  }


  // Fit the Timings[i] i=0,..,7 histograms to get offsets for the single ended paddle pmts

  float OffsetPMT[8];
  float OffsetPMTsig[8];

  char of1[128];
  sprintf(of1,"calibration%d/tdc_pmtsingles_offsets_run%d.dat",RunNumber,RunNumber);
  ofstream OUTF1;
  OUTF1.open(of1);
  
  for (int n=0; n<8;n++){
    int maxloc = Timings[n]->GetMaximumBin();
    float max = Timings[n]->GetBinCenter(maxloc);
    float hili = max+0.5;
    float loli = max-0.5;
    Timings[n]->Fit("gaus","RQ0","",loli,hili);
    TF1 *f1 = Timings[n]->GetFunction("gaus");
    max = f1->GetParameter(1);
    float sig =  f1->GetParameter(2);
    hili = max+sig;
    loli = max-sig;
    Timings[n]->Fit("gaus","RQ0","",loli,hili);
    f1 = Timings[n]->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig =  f1->GetParameter(2);	
    OffsetPMT[n] = max;
    OffsetPMTsig[n] = sig;
    int plane = n/4;
    int u = (n-plane*4)/2;
    int pad = 22 + (n%2);
    OUTF1<<plane<<"  "<<pad<<" "<<"  "<<u<<"  "<<max<<"  "<<sig<<endl;
    if (DEBUG == 77){
      Timings[n]->Draw();
      gPad->Update();
      getchar();
    }

    int indx = plane*88 + u*44 + pad-1;
    PMTOffsets[indx] = max;

  }
  OUTF1.close();

  // update DB file for pmt timing offsets
  sprintf(fnam,"calibration%d/tofpmt_tdc_offsets_all_FULL_run%d.DB",RunNumber,RunNumber);
  ofstream OUTF2;
  OUTF2.open(fnam);
  for (int k=0;k<176;k++){
    OUTF2<<PMTOffsets[k]<<endl;
  }
  OUTF2.close();

  // Now we should have ADC histograms with hits from the center of the paddles
  // or in the case of short paddle hits from the far end.

  if (DEBUG == 10){
    return;
  }

  float W[2][2][44];
  float dW[2][2][44];
  float M[2][2][44];
  float dM[2][2][44];
  float A[2][2][44];
  float dA[2][2][44];
  float G[2][2][44];
  float dG[2][2][44];
  float chisq[2][2][44];
  float peak[2][2][44];
  float fwhm[2][2][44];
  TCanvas *c1 = new TCanvas("c1","TOF MPVs Fit Results",700.,500.);

  double Peak;
  double Fwhm;
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(111);
  gStyle->SetLabelSize(0.03,"x");
  gStyle->SetLabelSize(0.03,"y");
  
  char htit[128];
  for (int k=0;k<176;k++){
    //ADCHists[k]->Draw();
    //gPad->Update();
    //getchar();
    if (ADCHists[k]->GetEntries()>1000){
      int plane = k/88;
      int side = (k - plane*88)/44;
      int paddle = k - plane*88 - side*44;
      
      sprintf(htit,"TOF ADC Signal Plane %d, Paddle %d, Side %d",plane,paddle,side);
      ADCHists[k]->SetTitle(htit);
      
      TF1* TheFitFunction = langaus(ADCHists[k],Peak,Fwhm);
      if (DEBUG){
	ADCHists[k]->Draw();
	TheFitFunction->Draw("lsame");
	//	h2d->Draw();
	gPad->Update();
	if (DEBUG>1){
	  char sf[128];
	  sprintf(sf,"tofadcresponsefitp%dp%ds%d.pdf",plane,paddle,side);
	  gPad->SaveAs(sf);
	}
	getchar();
      }
      W[plane][side][paddle]      = TheFitFunction->GetParameter(0);
      dW[plane][side][paddle]     = TheFitFunction->GetParError(0);
      M[plane][side][paddle]      = TheFitFunction->GetParameter(1);
      dM[plane][side][paddle]     = TheFitFunction->GetParError(1);
      A[plane][side][paddle]      = TheFitFunction->GetParameter(2);
      dA[plane][side][paddle]     = TheFitFunction->GetParError(2);
      G[plane][side][paddle]      = TheFitFunction->GetParameter(3);
      dG[plane][side][paddle]     = TheFitFunction->GetParError(3);
      chisq[plane][side][paddle]  = TheFitFunction->GetChisquare();
      peak[plane][side][paddle]   = Peak;
      fwhm[plane][side][paddle]   = Fwhm;
    }
  }
  f->Close();

  sprintf(fnam,"landaufit_run%d.dat",RunNumber);
  ofstream OUTF;
  OUTF.open(fnam);

  OUTF<<"Plane#  Paddle#   PMT#  pos  sigma"<<endl;
  OUTF<<"----------------------------------"<<endl;
  for (int k=0;k<2;k++){
    for (int j=0;j<2;j++){
      for (int n=0;n<44;n++){
	OUTF<<k<<" "<<j<<" "<<n<<"      ";
	OUTF<<W[k][j][n]<<"  "   
	    <<dW[k][j][n] <<"  "    
	    <<M[k][j][n] <<"  "     
	    <<dM[k][j][n]<<"  "     
	    <<A[k][j][n]<<"  "      
	    <<dA[k][j][n] <<"  "    
	    <<G[k][j][n] <<"  "     
	    <<dG[k][j][n] <<"   "<< chisq[k][j][n]  <<"  "
	    <<peak[k][j][n] <<"   "<< fwhm[k][j][n]<<endl; 
      }
    }
  }
  OUTF.close();

  float X[44];
  for (int k=0;k<44;k++){
    X[k] = k;
  }

  TGraphErrors *gr[4];
  gr[0] = new TGraphErrors(44, X, M[0][0], NULL, dM[0][0]);
  gr[1] = new TGraphErrors(44, X, M[0][1], NULL, dM[0][1]);
  gr[2] = new TGraphErrors(44, X, M[1][0], NULL, dM[1][0]);
  gr[3] = new TGraphErrors(44, X, M[1][1], NULL, dM[1][1]);

  gr[0]->SetTitle("Plane 0 PMT end 0");
  gr[1]->SetTitle("Plane 0 PMT end 1");
  gr[2]->SetTitle("Plane 1 PMT end 0");
  gr[3]->SetTitle("Plane 1 PMT end 1");
  for (int k=0;k<4;k++){
    gr[k]->SetMarkerStyle(21);
    gr[k]->SetMarkerSize(.5);
  }

  //TCanvas *c1 = new TCanvas("c1","TOF MPVs",700.,500.);
  c1->Clear();
  c1->Divide(2,2);

  c1->cd(1);
  gPad->SetGrid();
  gr[0]->GetXaxis()->SetTitle("PMT #");
  gr[0]->GetYaxis()->SetTitle("MPV [arb.]");
  gr[0]->Draw("ap");
  gr[0]->Draw("ap");
  c1->cd(2);
  gPad->SetGrid();
  gr[1]->GetXaxis()->SetTitle("PMT #");
  gr[1]->GetYaxis()->SetTitle("MPV [arb.]");
  gr[1]->Draw("ap");
  gr[1]->Draw("ap");
  c1->cd(3);
  gPad->SetGrid();
  gr[2]->GetXaxis()->SetTitle("PMT #");
  gr[2]->GetYaxis()->SetTitle("MPV [arb.]");
  gr[2]->Draw("ap");
  gr[2]->Draw("ap");
  c1->cd(4);
  gPad->SetGrid();
  gr[3]->GetXaxis()->SetTitle("PMT #");
  gr[3]->GetYaxis()->SetTitle("MPV [arb.]");
  gr[3]->Draw("ap");
  gr[3]->Draw("ap");

  sprintf(fnam,"mpvs_run%d.pdf",RunNumber);
  c1->SaveAs(fnam);
  c1->Clear();

  TGraphErrors *grPeak[4];
  grPeak[0] = new TGraphErrors(44, X, peak[0][0], NULL,  NULL);
  grPeak[1] = new TGraphErrors(44, X, peak[0][1], NULL,  NULL);
  grPeak[2] = new TGraphErrors(44, X, peak[1][0], NULL,  NULL);
  grPeak[3] = new TGraphErrors(44, X, peak[1][1], NULL,  NULL);

  grPeak[0]->SetTitle("Plane 0 PMT end 0");
  grPeak[1]->SetTitle("Plane 0 PMT end 1");
  grPeak[2]->SetTitle("Plane 1 PMT end 0");
  grPeak[3]->SetTitle("Plane 1 PMT end 1");
  for (int k=0;k<4;k++){
    grPeak[k]->SetMarkerStyle(21);
    grPeak[k]->SetMarkerSize(.5);
  }

  //TCanvas *c1 = new TCanvas("c1","TOF MPVs",700.,500.);
  c1->Clear();
  c1->Divide(2,2);
  c1->cd(1);
  gPad->SetGrid();
  grPeak[0]->GetXaxis()->SetTitle("PMT #");
  grPeak[0]->GetYaxis()->SetTitle("MPV [arb.]");
  grPeak[0]->Draw("ap");
  grPeak[0]->Draw("ap");
  c1->cd(2);
  gPad->SetGrid();
  grPeak[1]->GetXaxis()->SetTitle("PMT #");
  grPeak[1]->GetYaxis()->SetTitle("MPV [arb.]");
  grPeak[1]->Draw("ap");
  grPeak[1]->Draw("ap");
  c1->cd(3);
  gPad->SetGrid();
  grPeak[2]->GetXaxis()->SetTitle("PMT #");
  grPeak[2]->GetYaxis()->SetTitle("MPV [arb.]");
  grPeak[2]->Draw("ap");
  grPeak[2]->Draw("ap");
  c1->cd(4);
  gPad->SetGrid();
  grPeak[3]->GetXaxis()->SetTitle("PMT #");
  grPeak[3]->GetYaxis()->SetTitle("MPV [arb.]");
  grPeak[3]->Draw("ap");
  grPeak[3]->Draw("ap");

  sprintf(fnam,"peak_run%d.pdf",RunNumber);
  c1->SaveAs(fnam);
  c1->Clear();


  TGraphErrors *grFwhm[4];
  grFwhm[0] = new TGraphErrors(44, X, fwhm[0][0], NULL,  NULL);
  grFwhm[1] = new TGraphErrors(44, X, fwhm[0][1], NULL,  NULL);
  grFwhm[2] = new TGraphErrors(44, X, fwhm[1][0], NULL,  NULL);
  grFwhm[3] = new TGraphErrors(44, X, fwhm[1][1], NULL,  NULL);

  grFwhm[0]->SetTitle("Plane 0 PMT end 0");
  grFwhm[1]->SetTitle("Plane 0 PMT end 1");
  grFwhm[2]->SetTitle("Plane 1 PMT end 0");
  grFwhm[3]->SetTitle("Plane 1 PMT end 1");
  for (int k=0;k<4;k++){
    grFwhm[k]->SetMarkerStyle(21);
    grFwhm[k]->SetMarkerSize(.5);
  }

  //TCanvas *c1 = new TCanvas("c1","TOF MPVs",700.,500.);
  c1->Clear();
  c1->Divide(2,2);
  c1->cd(1);
  gPad->SetGrid();
  grFwhm[0]->GetXaxis()->SetTitle("PMT #");
  grFwhm[0]->GetYaxis()->SetTitle("FWHM [arb.]");
  grFwhm[0]->Draw("ap");
  grFwhm[0]->Draw("ap");
  c1->cd(2);
  gPad->SetGrid();
  grFwhm[1]->GetXaxis()->SetTitle("PMT #");
  grFwhm[1]->GetYaxis()->SetTitle("FWHM [arb.]");
  grFwhm[1]->Draw("ap");
  grFwhm[1]->Draw("ap");
  c1->cd(3);
  gPad->SetGrid();
  grFwhm[2]->GetXaxis()->SetTitle("PMT #");
  grFwhm[2]->GetYaxis()->SetTitle("FWHM [arb.]");
  grFwhm[2]->Draw("ap");
  grFwhm[2]->Draw("ap");
  c1->cd(4);
  gPad->SetGrid();
  grFwhm[3]->GetXaxis()->SetTitle("PMT #");
  grFwhm[3]->GetYaxis()->SetTitle("FWHM [arb.]");
  grFwhm[3]->Draw("ap");
  grFwhm[3]->Draw("ap");

  sprintf(fnam,"fwhm_run%d.pdf",RunNumber);
  c1->SaveAs(fnam);
  c1->Clear();
  
}




