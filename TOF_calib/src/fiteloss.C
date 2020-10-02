//-----------------------------------------------------------------------
// Determine timing offsets for single ended paddles
// create root file adchists_runXXXX.root with root tree and histograms
// create full DB file for timing offsets
// if in debug mode 10 quite at this point in the code otherwise continue
// to fit the adc spectra.
// histograms: TimingN    TDC offsets of single ended pmt M
//             xTvsxEN    xpos from TDC vs xpos from E for paddle N
//             ADCHistsN  ADC Integral for PMT N for central hit
//             ADCPeakHistsN  ADC Signal Amplitude for PMT N for central hit
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

#define NumPMTMax 200
int NPMTS = 184;

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
   int start = hist->FindBin(200.);
   int stop = hist->FindBin(1600.);
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
     } else{
       break;
     }
   }
   if (bin<0){
     return NULL;
   }
   max = hist->GetBinCenter(bin);
   FitRange[0] = max - 50.;//0.5*hist->GetMean();
   FitRange[1] = 140.;
   FitRange[0] = 500.;
   FitRange[1] = 1500.;
   cout<<"Fit RANGE: "<< FitRange[0] <<" "<< FitRange[1] <<endl;

   LimitHigh[3]=5.0;
   StartValues[0]=1000.; //Width
   StartValues[1]=800; //MP
   StartValues[2]=hist->GetEntries(); //Area
   StartValues[3]=10.0; //GSigma


   LimitHigh[0] = 5000.0;
   LimitLow[0]  = 0.5;
   LimitHigh[1] = 1300.0;
   LimitLow[1]  = 200.0;
   LimitHigh[2] = hist->GetEntries()*500.;
   LimitLow[2]  = hist->GetEntries()*0.01;
   LimitHigh[3] = 3000.0;
   LimitLow[3]  = 0.001;

   Double_t chisqr;
   Int_t    ndf;
   TF1 *fitfunc = langaufit(hist,FitRange,StartValues,
			    LimitLow,LimitHigh,fp,fpe,&chisqr,&ndf);
   
   langaupro(fp,Peak,FWHM);

   return fitfunc;
}

void fiteloss(int R){

  NPMTS = 176;            // TOF 1 geometry
  if (R>69999){
    NPMTS = 184;          // TOF 2 geometry
  }

  char hnam[128];
  char histname[128];
  char rf[128];
  sprintf(rf, "./calibration%d/walk_histos_run%d.root",R,R);
  sprintf(histname, "Twalk") ;
  
 
  TFile *RF = new TFile(rf, "READ");

  float PEAKPOS[200];
  
  for (int k=0; k<NPMTS; k++) {
    
    sprintf(hnam, "%s%d", histname, k);

    TH2F *h = (TH2F*)RF->Get(hnam);

    if (h->GetEntries()<100){
      cout<<k<<" Histogram insufficient statistics"<<endl;
      continue;
    }

    TH1D *hist = h->ProjectionX("hist", 1, h->GetYaxis()->GetNbins());
    if (hist->GetEntries()<100){
      continue;
    }

    // now find minimum ionizing peak!
    double peak, fwhm;
    TF1*fitres =  langaus(hist, peak, fwhm);
    hist->GetXaxis()->SetRangeUser(0., 3000.);
    hist->Draw();
    fitres->Draw("same");
    gPad->SetGrid();
    gPad->Update();
    gPad->Update();
    getchar();
    //break;  
    
  }
  

}
