//-----------------------------------------------------------------------
// Fit ADC spectra with landau convoluted with Gaussian as a function of
// the distance from the pmt for both left and right side
// then fit these data points do determine attenuation length
// use adchists_run%d.root to get the 2d histograms x-position(TDC) vs. ADCIntegral
//
//
//-----------------------------------------------------------------------
#include "TTree.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TText.h"

#include <iostream>
#include <fstream>

using namespace std;

int DEBUG = 0;
Double_t SNRPeak, SNRFWHM;
int RunNumber;

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
   ffit->SetParameters(startvalues);
   ffit->SetParNames("Width","MP","Area","GSigma");
   
   for (i=0; i<4; i++) {
      ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
   }

   his->Fit(FunName,"RBQ0");   // fit within specified range, use ParLimits, do not plot

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

TF1*  langaus(TH1D *hist, int k) {

   // Fitting histogram
   printf("Fitting...\n");

   // Setting fit range and start values
   Double_t FitRange[2];
   Double_t StartValues[4], LimitLow[4], LimitHigh[4], fp[4], fpe[4];
   int ok =1;
   int bins = 10;

   FitRange[0] = hist->GetBinCenter(bins-1);
   FitRange[1] =  FitRange[0] + 5000.;
   if (FitRange[1]>24000.){
     FitRange[1] = 24000.;
   }
   
   if (k<15) {
     hist->GetXaxis()->SetRangeUser(5000.,23000.);
   } else {
     hist->GetXaxis()->SetRangeUser(2500.,23000.);
   }
   int maxb = hist->GetMaximumBin();
   double max = hist->GetBinCenter(maxb);
   FitRange[0] = max - 1000.;
   FitRange[1] =  FitRange[0] + 5000.;
   if (FitRange[1]>24000.){
     FitRange[1] = 24000.;
   }

   hist->GetXaxis()->SetRangeUser(3000.,23000.);

   cout<<"Fitrange:  "<<FitRange[0]<<"  "<<FitRange[1]<<endl;

   StartValues[0] = 1000; //Width
   StartValues[1] = FitRange[0]+1000.; //MP
   StartValues[2] = hist->GetEntries(); //Area
   StartValues[3] = 10.0; //GSigma

   LimitHigh[0] = 1500.0;
   LimitLow[0]  = 200.5;
   LimitHigh[1] = FitRange[1];
   LimitLow[1]  = FitRange[0];
   LimitHigh[2] = hist->GetEntries()*500.;
   LimitLow[2]  = hist->GetEntries()*0.01;
   LimitHigh[3] = 3000.0;
   LimitLow[3]  = 0.00001;

   Double_t chisqr;
   Int_t    ndf;
   TF1 *fitfunc = langaufit(hist,FitRange,StartValues,
			    LimitLow,LimitHigh,fp,fpe,&chisqr,&ndf);
   
   double w = fitfunc->GetParameter(0);
   double mp = fitfunc->GetParameter(1);
   if (FitRange[0]>(mp-w)){
     FitRange[0] = mp-w*1.5;
     if (FitRange[0]<0.){
       FitRange[0] = 10.;
     }
     fitfunc = langaufit(hist,FitRange,StartValues,
			 LimitLow,LimitHigh,fp,fpe,&chisqr,&ndf);
   } 
   
   langaupro(fp,SNRPeak,SNRFWHM);

   return fitfunc;
}

void dofit(int R){

  RunNumber = R;
  char fnam[128];

  sprintf(fnam,"calibration%d/adchists_run%d.root", RunNumber, RunNumber);
  TFile *f = new TFile(fnam,"READ");
  //f->ls();

  TCanvas *c2 = new TCanvas("c2","Attenuation length",800.,600.);
  float a1[88]   ;
  float d1[88]   ;
  float a2[88]   ;
  float d2[88]   ;
  float Norm[88] ; 
  float SingleAtten[88];
  float PaddleGainRatio[88];
  for (int k=0;k<88;k++){
    PaddleGainRatio[k] = 1.;
  }

  double GainVal[88][5];

  for (int Plane=0;Plane<2;Plane++){
    
    for (int Paddle=0; Paddle<44;Paddle++) {

      if ((Paddle == 21) || (Paddle == 22))
	continue;
      
      int idx[2];
      idx[0] = Paddle + Plane*88;
      idx[1] = 44 + Paddle + Plane*88;
      
      
      double XPos[2][100];
      double YPos[2][100];
      double dXPos[2][100];
      double dYPos[2][100];
      int mbins;
      for (int n=0;n<2;n++){
	
	char hnam[128];
	sprintf(hnam,"xTvsEPMT%d",idx[n]);
	
	TH2D *h = (TH2D*)f->Get(hnam);
	if (h == NULL){
	  cout<<"ERROR not histogram:"<<hnam<<endl;
	  return;
	}
	int NBins = h->GetNbinsY();
	mbins = NBins;
	for ( int k=1; k<NBins-2; k++) {
	  
	  TH1D *hp = h->ProjectionX("hp",k,k);
	  TH1D *toh;
	  toh = hp;

	  int rebin = 4;
	  if (k>20){
	    rebin = 2;
	  }
	  toh = (TH1D*) hp->Rebin(rebin);
	  
	  TF1* TheFitFunction = langaus(toh,k);
	  
	  XPos[n][k-1] = h->GetYaxis()->GetBinCenter(k);
	  dXPos[n][k-1] = 0.;
	  YPos[n][k-1] = SNRPeak;
	  dYPos[n][k-1] = SNRFWHM/12.;
	  
	  if (DEBUG>1) {
	    hp->Draw();
	    TheFitFunction->Draw("lsame");
	    gPad->Update();
	    //char tn[128];
	    //sprintf(tn,"plots/landau_fit_%d_%d_run%d.pdf",idx[n],k,RunNumber);
	    //gPad->SaveAs(tn);
	    if (DEBUG==99){
	      getchar();
	    }
	  }
	}
	
      }
      
      cout<<"Now get attenuation lengths"<<endl;
      
      TGraphErrors *grf1 = new TGraphErrors(mbins-3,XPos[0],YPos[0],dXPos[0],dYPos[0]);
      TGraphErrors *grf2 = new TGraphErrors(mbins-3,XPos[1],YPos[1],dXPos[1],dYPos[1]);
      
      grf1->SetMarkerColor(4);
      grf1->SetMarkerStyle(21);
      
      grf2->SetMarkerColor(6);
      grf2->SetMarkerStyle(21);
      
      char grftit[128];
      sprintf(grftit,"Paddle %d, Plane %d",Paddle+1, Plane);
      TMultiGraph *mgr = new TMultiGraph("mgr",grftit);
      mgr->Add(grf1);
      mgr->Add(grf2);
      
      c2->Clear();
      c2->SetGrid();
      mgr->Draw("AP");
      gPad->SetGrid();
      //fitf->Draw();
      gPad->Update();
      //getchar();
      
      // calculate mean between bin 7 and 30
      float cnt = 0.;
      float sum1 = 0.;
      float sum2 = 0.;
      for (int k=7;k<30;k++){
	sum1 += YPos[0][k];
	sum2 += YPos[1][k];
	cnt += 1.;
      }
      sum1 /= cnt;
      sum2 /= cnt;
      
      float Ratio = sum1/sum2;
      PaddleGainRatio[Plane*44 + Paddle] = Ratio; // left/right

      int index = 0;
      double X[100];
      double Y[100];
      double dY[100];

      for (int n=0;n<mbins-3;n++){
	if (TMath::Abs(YPos[0][n]-YPos[1][n]*Ratio)<5000) {
	  if (((n<6) && (YPos[0][n]>9000.)) || (n>5)){
	    X[index] = XPos[0][n];
	    Y[index] = YPos[0][n];
	    dY[index] = dYPos[0][n];
	    if (dY[index]<200.){
	      dY[index] = 200.;
	    }
	    index++;
	  }

	  if (((n<6) && (YPos[1][n]>9000.)) || (n>5)){
	    X[index] = XPos[1][n];
	    Y[index] = YPos[1][n]*Ratio;
	    dY[index] = dYPos[1][n];
	    if (dY[index]<200.){
	      dY[index] = 200.;
	    }
	    index++;
	  }
	  if (n==20){
	    GainVal[Plane*44+Paddle][0] = XPos[0][n];
	    GainVal[Plane*44+Paddle][1] = YPos[0][n];
	    GainVal[Plane*44+Paddle][2] = dY[index-2];
	    GainVal[Plane*44+Paddle][3] = YPos[1][n];
	    GainVal[Plane*44+Paddle][4] = dY[index-1];
	  }
	}    
	else {
	  float val = YPos[0][n];
	  float dval = dYPos[0][n];
	  if (val<YPos[1][n]*Ratio){
	    val = YPos[1][n]*Ratio;
	    dval = dYPos[1][n];
	  }
	  X[index] = XPos[0][n];
	  Y[index] = val;
	  dY[index] = dval;
	  if (dY[index]<500.){
	    dY[index] = 500.;
	  }
	  index++;
	}
      }
      
      TGraphErrors *Grf = new TGraphErrors(index,X,Y,NULL,dY);

      int WhatFunction = 0;
      TF1 *f1,*f2,*f3;
      if (WhatFunction){
	f1 = new TF1("f1","[4]*([0]*exp(-x/[1]) + [2]*exp(-x/[3]))", 0., 250.);
	f1->SetParameter(0,5000.);
	f1->SetParameter(1,50.);
	f1->SetParameter(2,5000.);
	f1->SetParameter(3,300.);
	//f1->SetParameter(4,1.);
	f1->FixParameter(4,1.);
	f1->SetParLimits(0, 1000., 50000.);
	f1->SetParLimits(2, 1000., 20000.);
	f1->SetParLimits(1, 30., 150.);
	f1->SetParLimits(3, 150., 1000.);
      } else {
	f1 = new TF1("f1","([0]*exp(-x/[1]) + [0]*exp(-x/[2]))", 0., 250.);
	f1->SetParameter(0,5000.);
	f1->SetParameter(1,50.);
	f1->SetParameter(2,500.);
	//f1->SetParameter(4,1.);
	f1->SetParLimits(0, 1000., 50000.);
	f1->SetParLimits(1, 30., 150.);
	f1->SetParLimits(2, 150., 1000.);

	f2 = new TF1("f2","[0]*exp(-x/[1])", 85., 170.);
	f2->SetParameter(0,5000.);
	f2->SetParameter(1,500.);

	f3 = new TF1("f3","([0]*exp(-x/75.) + [0]*exp(-x/470.))", 0., 250.);
	f3->SetParameter(0,5000.);
	f3->SetParLimits(0, 1000., 50000.);
      }

      
      Grf->Fit("f1", "QR", "",0.,250. );
      f1->Draw("same");
      Grf->Fit("f3", "QR", "",0.,250. );
      f3->SetLineColor(5);
      f3->Draw("same");

      mgr->GetXaxis()->SetTitle("Distance from PMT [cm]");
      mgr->GetYaxis()->SetTitle("Landau Peak [arb.]");
      gPad->Update();
      
      float xloc = 150.;
      float yloc = X[2];
      float att;
      if (WhatFunction){
	a1[Paddle+44*Plane] = f1->GetParameter(0);
	d1[Paddle+44*Plane] = f1->GetParameter(1);
	a2[Paddle+44*Plane] = f1->GetParameter(2);
	d2[Paddle+44*Plane] = f1->GetParameter(3);
	Norm[Paddle+44*Plane] = f1->GetParameter(4); 
      } else {
	a1[Paddle+44*Plane] = f1->GetParameter(0);
	d1[Paddle+44*Plane] = f1->GetParameter(1);
	d2[Paddle+44*Plane] = f1->GetParameter(2);	
	Norm[Paddle+44*Plane] = 0.0; 
	a2[Paddle+44*Plane] = 0.0; 
	Grf->Fit("f2", "QR", "",85.,170. );
	f2->SetLineColor(7);
	f2->Draw("same");
	att = f2->GetParameter(1);
	SingleAtten[Paddle+44*Plane] = att;
      }



      TText *t0 ;
      if (WhatFunction){
	t0 = new TText(0.4,0.85,"Fit: Norm*(a1*exp(-x/d1)+a2*exp(-x/d2))");
      } else {
	t0 = new TText(0.4,0.85,"Fit: (a1*exp(-x/d1)+a1*exp(-x/d2))");
      }
      char lin[128];
      sprintf(lin,"Norm:   %.2f fixed",Norm[Paddle+44*Plane]);
      TText *t1 = new TText(0.6,0.78,lin);
      sprintf(lin,"a1:       %.1f",a1[Paddle+44*Plane]);
      TText *t2 = new TText(0.6,0.75,lin);
      sprintf(lin,"d1:       %.1f",d1[Paddle+44*Plane]);
      TText *t3 = new TText(0.6,0.72,lin);
      sprintf(lin,"a2:       %.1f",a2[Paddle+44*Plane]);
      TText *t4 = new TText(0.6,0.69,lin);
      sprintf(lin,"d2:       %.1f",d2[Paddle+44*Plane]);
      TText *t5 = new TText(0.6,0.66,lin);
      
      sprintf(lin,"Ratio blue/pink:   %.2f",Ratio);
      TText *t6 = new TText(0.6, 0.5,lin);

      sprintf(lin,"Single Atten.:   %.2f",att);
      TText *t7 = new TText(0.6, 0.6,lin);
      
      t0->SetNDC();
      t0->SetTextSize(0.038);
      t0->Draw();
      if (WhatFunction){
	t1->SetNDC();
	t1->SetTextSize(0.03);
	t1->Draw();
      }
      t2->SetNDC();
      t2->SetTextSize(0.03);
      t2->Draw();
      t3->SetNDC();
      t3->SetTextSize(0.03);
      t3->Draw();
      if (WhatFunction){
	t4->SetNDC();
	t4->SetTextSize(0.03);
	t4->Draw();
      }
      t5->SetNDC();
      t5->SetTextSize(0.03);
      t5->Draw();
      t6->SetNDC();
      t6->SetTextSize(0.03);
      t6->Draw();
      t7->SetNDC();
      t7->SetTextSize(0.03);
      t7->Draw();
      
      gPad->Update();
      c2->Update();
      sprintf(grftit,"plots/paddle%d_plane%d_atten_run%d.pdf",Paddle+1, Plane, RunNumber);
      c2->SaveAs(grftit);
      if (DEBUG == 99){
	getchar();
      }
    }
  }
  f->Close();

  TH1F *ParA1 = new TH1F("ParA1"," Parameter a1 in attenuation fit", 20., 4000.,10000.);
  TH1F *ParA2 = new TH1F("ParA2"," MPV at x=150", 178, -0.5, 177.5);
  TH1F *ParD1 = new TH1F("ParD1"," Parameter d1 in attenuation fit", 20., 0.,200.);
  TH1F *ParD2 = new TH1F("ParD2"," Parameter d2 in attenuation fit", 20., 0.,1000.);
  ParA2->GetXaxis()->SetTitle("PMT #");
  ParA2->GetYaxis()->SetTitle("MPV in ADC units");

  TH1F *ParSingleD = new TH1F("ParSingleD"," Single attenuation parameter fit", 30., 0.,1000.);

  char of[128];
  sprintf(of,"calibration%d/pmtgain_centerlocation_run%d.dat",RunNumber,RunNumber);
  ofstream OUTF1;
  OUTF1.open(of);

  sprintf(of,"calibration%d/attenuation_values_full_run%d.dat",RunNumber,RunNumber);
  ofstream OUTF2;
  OUTF2.open(of);

  sprintf(of,"calibration%d/attenuation_values_single_run%d.dat",RunNumber,RunNumber);
  ofstream OUTF3;
  OUTF3.open(of);

  // use Paddle 11 plane 0 as reference!!!!!!
  sprintf(of,"calibration%d/gain_and_attenuations_full_run%d.DB",RunNumber,RunNumber);
  ofstream OUTF4;
  OUTF4.open(of);
  

  for (int k=0;k<88;k++){

    if ((k == 21) || (k == 22) || (k == 21+44) || (k == 22+44) ) {
      OUTF4<<k<<"  1. 1. 1. 80. 500."<<endl;
    } else {

      ParA1->Fill(a1[k]);
      ParA2->Fill((float)k,GainVal[k][1]);
      ParA2->Fill((float)k+88,GainVal[k][3]);
      ParD1->Fill(d1[k]);
      ParD2->Fill(d2[k]);
      ParSingleD->Fill(SingleAtten[k]);
      
      OUTF2<<k<<"   "<<a1[k]<<"   "<<d1[k]<<"   "<<d2[k]<<"   " << PaddleGainRatio[k] <<endl; 
      OUTF3<<k<<"   "<<SingleAtten[k]<<endl; 
      
      for (int n=0;n<5;n++){
	OUTF1<<n<<"  "<<GainVal[k][0]<<"  "<<GainVal[k][1]<<"  "<<GainVal[k][2]
	     <<"  "<<GainVal[k][3]<<"  "<<GainVal[k][4]<<endl;
      }
      float gainLeft = 1.;
      float gainRight = 1.;
      if (a1[k]>0){
	gainLeft  = a1[10]/a1[k];
	gainRight = a1[10]/a1[k]*PaddleGainRatio[k];
      }
      
      OUTF4<<k  << "  "<< gainLeft << "   "<< gainRight << "   " << a1[k]<<"   "<<d1[k]<<"   "<<d2[k]<<endl;

    }

  }

  OUTF1.close();
  OUTF2.close();
  OUTF3.close();
  OUTF4.close();

  c2->Clear();
  c2->Divide(2,2);
  c2->cd(1);
  ParA1->Draw();
  c2->cd(2);
  ParA2->Draw();
  c2->cd(3);
  ParD1->Draw();
  c2->cd(4);
  ParD2->Draw();
  gPad->Update();
  sprintf(fnam,"calibration%d/attenuation_run%d.pdf", RunNumber, RunNumber);
  c2->SaveAs(fnam);

  sprintf(fnam,"calibration%d/attenuation_run%d.root", RunNumber, RunNumber);
  TFile *fnew = new TFile(fnam,"RECREATE");
  fnew->cd();
  ParA1->Write();
  ParA2->Write();
  ParD1->Write();
  ParD2->Write();
  ParSingleD->Write();
  fnew->Close();

}
