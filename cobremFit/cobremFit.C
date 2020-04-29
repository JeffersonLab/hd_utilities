// Code to fit an enhancement spectrum
#include <stdio.h>
#include <iostream> 
#include <TCanvas.h>
#include <TF1.h>
#include <TMath.h>
#include <TMinuit.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TLine.h>
#include <TSystem.h>
#include <TStyle.h>
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "TRandom2.h"
#include "TGraphErrors.h"
#include "TError.h"
#include "TVirtualFitter.h"

#include "TFile.h"
using namespace std;

Double_t GausOnBase(Double_t *, Double_t *);
Double_t efit(const Double_t *);
//void cbremFit_accidentals(Double_t beamMeV = 12050.0, Double_t colliDist_m = 75.0, Double_t colliRad_mm = 3.4, Int_t nVec=2, Int_t tagger_ps = 1, Bool_t showPlots = true);
//void parFromHuman(Double_t beamMeV = 12050.0, Double_t edgeMeV = 9000.0, Double_t spreadMeV = 40.0, Double_t colliDist_m = 75.0, Double_t colliRad_mm = 3.4, Int_t nVec = 2, Double_t *par=NULL);
void cobremFit(Double_t beamMeV = 11650.0, Double_t colliDist_m = 75.0, Double_t colliRad_mm = 5.0, Int_t nVec=2, Int_t tagger_ps = 1, Bool_t showPlots = true);
void parFromHuman(Double_t beamMeV = 11650.0, Double_t edgeMeV = 7500.0, Double_t spreadMeV = 50.0, Double_t colliDist_m = 75.0, Double_t colliRad_mm = 5.0, Int_t nVec = 2, Double_t *par=NULL);
void enhFromParams(Double_t *par=NULL);


//Some enumerators and names
enum {      
  THETA,  // [0] theta      main angle responsible for coherent edge cutoffs
  SIGMA,  // [1] sigma      smearing of theta
  THETAR, // [2] thetar     relative angle resonsible for colli cutoffs
  SIGMAR, // [3] sigmar     smearing of colli cutoff angle
  E0MEV,  // [4] beam energy
  NVEC,   // [5] nvec       no of vectors contributing
  IVEC};  // [6] ivec[]     array of intensities of vectors up to nvec.


// Some basic consts etc first
// Consts are all UPPER CASE


//Approx Form factor is F(g^2) = (q^2 + b^(-2)) ^ -2
//Where b= 111 x Z^(-1/3) (x 925 to get into units of crystal lattice)
//const Double_t B = 0.247892436;  //where did I get that ? Timm ?
//const Double_t A=0.03;           //made up for now, need to get the actual no for this later
const Double_t k=26.5601;          //put in formula for k later (my own stonehenge paper)

const Int_t VECTORS[]={2,4,6,8,10};    //list of the vectors to be included (022,044);

Int_t counter=1;

//THESE NEED TO BE CHANGED FOR EACH SETTING (ie comment in/out)

Int_t THETASTEPS = 201;          //no of steps in convoluting with gaussian
Double_t LOWFIT = 1500.0;         //how far below the peak in MeV to start fitting

//All histograms and related are globals
TH1F *histP = NULL;  //pol from calculation
TH1F *histE = NULL;  //enh from calculation
TH1F *histD = NULL;  //enh from data to be fitted
 
//these are really just convenient arrays - they don't ever get plotted.
TH1F *weightHist  = NULL;
TH2F *thetaWeight = NULL;
TH2F *thetaPol    = NULL;
TH2F *thetaTtot   = NULL;
TH2F *thetaItot   = NULL;
 
Double_t energy[1000];
Double_t energy_GeV[1000];
Double_t energyBins[1000];
Double_t energyBins_GeV[1000];
Int_t nBins=0;

Int_t fitMinBin;
Int_t fitMaxBin;

Int_t verbose=0;
Double_t bestPar[10];
Double_t bestChisq;

TF1 *gausFit;

TCanvas *genCanvas=NULL;

//general one off init things
Bool_t isInit=kFALSE;

void init(){
  gStyle->SetOptStat(kFALSE);
  isInit=kTRUE;
  gausFit=new TF1("gausFit",GausOnBase,0,100,4);
  genCanvas = new TCanvas("genCanvas","genCanvas",50,50,800,1000);
  genCanvas->Divide(1,2);   
  genCanvas->GetPad(1)->SetGridx(1);
  genCanvas->GetPad(1)->SetGridy(1);
  genCanvas->GetPad(2)->SetGridx(1);
  genCanvas->GetPad(2)->SetGridy(1);
}

//fit of a gaussian on a baseline, for gausFit,
Double_t GausOnBase(Double_t *x, Double_t *par) {
  Double_t arg = 0;
  if (par[2] != 0) arg = (x[0] - par[1])/par[2];
   Double_t fitval = par[3] + par[0]*TMath::Exp(-0.5*arg*arg);
   return fitval;
}


//The main customized fitting function which gets called by MINUIT
Double_t efit(const Double_t *parms){
  
  Double_t chisq = 1.0;
  Double_t delta;
  Double_t b1,b2;
  Double_t err;
  Double_t *par = (Double_t*)parms;

  histE->Reset("ICE"); //reset the histogram
  //  cout << "par[0]= " << par[0] << endl;

  //call the function to make the enhancement and polarization
  enhFromParams(par);

  chisq = 1.0;
  //loop over all the required bins in the histogram to work out a chisq
  for(int n=fitMinBin+1;n<=fitMaxBin;n++){
    b1=histE->GetBinContent(n);
    b2=histD->GetBinContent(n);
    // skip bins without tagger counters
    if(histD->GetBinContent(n) < 0.1) 
      continue;
    //err=1.0;
    err=histD->GetBinError(n);
    delta=(b1-b2)/err;
    chisq+=(delta*delta);
    //note - not a proper chisq because its an enhancement
    //   return;
  }
   
  fprintf(stderr,"Chisq: \t%6.2f\t\r",chisq);

  if(chisq<bestChisq){
    bestChisq=chisq;
    for(int n=0;n<10;n++){
      bestPar[n]=par[n];
    }
    if(verbose){
      if(10%(counter++)){
	//if verbose, draw this on the canvas for every iteration to see how it's going
	genCanvas->cd(1);
	histD->SetLineColor(4);
	histD->Draw("P e");
	histD->SetMinimum(0.0);
	//histD->SetMaximum(6.0);
	genCanvas->cd(1);
	histE->Draw("HIST same");
	
	genCanvas->cd(2);
	histP->Draw("HIST");
	histP->SetMinimum(0);
	histP->SetMaximum(1);
	
	genCanvas->Draw();   
	
	genCanvas->Update();
	gSystem->ProcessEvents();
	counter=1;
      }
    }
  }
  return chisq;
  
}

void cobremFit(Double_t beamMeV, Double_t colliDist_m, Double_t colliRad_mm, Int_t nVec, Int_t tagger_ps, Bool_t showPlots){ // tagger = 0, ps = 1

  Double_t enhancement[1000];
  Double_t enhancementErr[1000];
  Double_t diff1,diff2,fitedge,scalefac;
  fitedge = 0.;
  Char_t name[30];
  Double_t lowmean=100000000.0;
  Double_t par[10];
  ROOT::Math::Minimizer* min;

  if(!isInit) init(); // init anything if needed
        
  TFile *fdiamond = TFile::Open("hist_diamond.root");
  TFile *famorph = TFile::Open("hist_amorph.root");

  //////////////////
  // Load PS data //
  //////////////////
  
#if 0
  TH2F* locHist_DeltaT_PSEnergy = (TH2F*)fdiamond->Get("PSTAG_timeVsTagE");
  TH2F* locHist_DeltaT_PSEnergy_Amorph = (TH2F*)famorph->Get("PSTAG_timeVsTagE");
  int lowBinDeltaT = locHist_DeltaT_PSEnergy->GetYaxis()->FindBin(-1.999);
  int highBinDeltaT = locHist_DeltaT_PSEnergy->GetYaxis()->FindBin(1.999);
  
  TH1F* locHist_PSEnergy = (TH1F*)locHist_DeltaT_PSEnergy->ProjectionX("PS_TagE",lowBinDeltaT,highBinDeltaT);
  TH1F* locHist_PSEnergy_Accidental = (TH1F*)locHist_DeltaT_PSEnergy->ProjectionX("PS_TagE_Accidental");
  locHist_PSEnergy_Accidental->Add(locHist_PSEnergy, -1);
  //locHist_PSEnergy->Add(locHist_PSEnergy_Accidental, -1/10.);
  
  TH1F* locHist_PSEnergy_Amorph = (TH1F*)locHist_DeltaT_PSEnergy_Amorph->ProjectionX("PS_Amorph_TagE",lowBinDeltaT,highBinDeltaT);
  TH1F* locHist_PSEnergy_Amorph_Accidental = (TH1F*)locHist_DeltaT_PSEnergy_Amorph->ProjectionX("PS_Amorph_TagE_Accidental");
  locHist_PSEnergy_Amorph_Accidental->Add(locHist_PSEnergy_Amorph, -1);
  //locHist_PSEnergy_Amorph->Add(locHist_PSEnergy_Amorph_Accidental, -1/10.);
#endif
    
#if 1
  TH1F* locHist_PSEnergy = (TH1F*)fdiamond->Get("PSPair/PSC_PS/PS_E");
  TH1F* locHist_PSEnergy_Amorph = (TH1F*)famorph->Get("PSPair/PSC_PS/PS_E");
#endif

#if 0
  TH1F* locHist_PSEnergy_temp = (TH1F*)fdiamond->Get("rootspy/highlevel/PSPairEnergy");
  TH1F* locHist_PSEnergy_Amorph_temp = (TH1F*)famorph->Get("rootspy/highlevel/PSPairEnergy");
  TH1F* locHist_PSEnergy = new TH1F("PSEnergy", locHist_PSEnergy_temp->GetTitle(), locHist_PSEnergy_temp->GetNbinsX(), 7.0, 12.0);
  TH1F* locHist_PSEnergy_Amorph = new TH1F("PSEnergy_Amorph", locHist_PSEnergy_temp->GetTitle(), locHist_PSEnergy_temp->GetNbinsX(), 7.0, 12.0);
  for(int i=0; i<locHist_PSEnergy_temp->GetNbinsX()+1; i++) {
	locHist_PSEnergy->SetBinContent(i, locHist_PSEnergy_temp->GetBinContent(i));
	locHist_PSEnergy_Amorph->SetBinContent(i, locHist_PSEnergy_Amorph_temp->GetBinContent(i));
	locHist_PSEnergy->SetBinError(i, locHist_PSEnergy_temp->GetBinError(i));
        locHist_PSEnergy_Amorph->SetBinError(i, locHist_PSEnergy_Amorph_temp->GetBinError(i));
  }
#endif

  locHist_PSEnergy->Sumw2();
  locHist_PSEnergy_Amorph->Sumw2();

  TH1F* locHist_PSEnhancement;

  if(locHist_PSEnergy) {
    locHist_PSEnergy->SetTitle("Diamond; PS Energy (GeV)");
    locHist_PSEnergy_Amorph->SetTitle("Amorphous Reference; PS Energy (GeV)");

    locHist_PSEnhancement = (TH1F*)locHist_PSEnergy->Clone();
    locHist_PSEnhancement->Divide(locHist_PSEnergy_Amorph);

    // set range of beam energy
    locHist_PSEnergy->GetXaxis()->SetRangeUser(6.0, beamMeV/1000.);
    locHist_PSEnergy_Amorph->GetXaxis()->SetRangeUser(6.0, beamMeV/1000.);
    locHist_PSEnhancement->GetXaxis()->SetRangeUser(6.0, beamMeV/1000.);

    Double_t scaleFactorPS = 1.;
    for(int i=locHist_PSEnhancement->GetXaxis()->GetNbins(); i>0; i--){
      if(locHist_PSEnhancement->GetBinContent(i) > 0.0) {
	scaleFactorPS = locHist_PSEnhancement->GetBinContent(i);
	break;
      }
    }
    locHist_PSEnhancement->Scale(1./scaleFactorPS);
    locHist_PSEnhancement->SetTitle("Enhancement: Diamond/Amorphous; PS Energy (GeV)");
    locHist_PSEnhancement->SetMinimum(0);

    if(showPlots) {
      TCanvas *ps = new TCanvas("ps","ps", 1200, 400);
      ps->Divide(3,1);
      ps->cd(1);
      locHist_PSEnergy->Draw();
      ps->cd(2);
      locHist_PSEnergy_Amorph->Draw();
      ps->cd(3);
      locHist_PSEnhancement->Draw();
    }
  }
    
  ///////////////////////////////////////
  // Different options for accidentals //
  ///////////////////////////////////////
  const int maxOptions = 4;
  TString optionName[maxOptions] = {"TAGGER/TaggerEnergy_DeltaTSC", "TAGGER/TaggerEnergyCounter_DeltaTSCTOF", "TAGGER/TaggerEnergyCounter_DeltaTRFCalib", "p2pi_preco/Custom_p2pi_hists_TimingCut_Measured/Egamma_DeltaT_Cut"};
#if 1
  double optionLowTime[maxOptions] = {-6., -6., -2., -2.};
  double optionHighTime[maxOptions] = {-3., -3., 1.99, 1.99};
  double optionLowTimeAcc[maxOptions] = {-38., -38., -6., -6.}; // -38., -38., -6., -6.
  double optionHighTimeAcc[maxOptions] = {29., 29., 5.99, 5.99}; // -7., -7., -1.99, -1.99
  double optionScaleAcc[maxOptions] = {16, 16, 2, 2};
  double optionRebin[maxOptions] = {1, 1, 1, 2};
#endif

  for(int iopt = 3; iopt<4; iopt++) {
    //////////////////////
    // Load tagger data //
    //////////////////////
    TH2D* locHist_TaggerEnergy_DeltaTSC = (TH2D*)fdiamond->Get(optionName[iopt]);
    TH2D* locHist_TaggerEnergy_DeltaTSC_Amorph = (TH2D*)famorph->Get(optionName[iopt]);
    //locHist_TaggerEnergy_DeltaTSC->Sumw2();
    //locHist_TaggerEnergy_DeltaTSC_Amorph->Sumw2();

#if 0
    ///////////////////////////////////
    // Channel dependent time offset //
    ///////////////////////////////////
    TH1D* locHist_TaggerEnergy = (TH1D*)locHist_TaggerEnergy_DeltaTSC->ProjectionY(Form("TaggerEnergyCounter_%d", iopt));
    locHist_TaggerEnergy->Reset();
    locHist_TaggerEnergy->SetTitle("Diamond: Channel Dependent");
    TH1D* locHist_TaggerEnergy_Amorph = (TH1D*)locHist_TaggerEnergy_DeltaTSC_Amorph->ProjectionY(Form("TaggerEnergyCounter_Amorph_%d", iopt));
    locHist_TaggerEnergy_Amorph->Reset();
    locHist_TaggerEnergy_Amorph->SetTitle("Amorphous Reference: Channel Dependent");

    
    //TH1D* locHist_TaggerEnergyAcc = (TH1D*)locHist_TaggerEnergy_DeltaTSC->ProjectionY("test");
    //locHist_TaggerEnergyAcc->Reset();
    //TH1D* locHist_TaggerEnergyAcc_Amorph = (TH1D*)locHist_TaggerEnergy_DeltaTSC->ProjectionY("testAcc");
    //locHist_TaggerEnergyAcc_Amorph->Reset();

    for(int iybin=0; iybin<locHist_TaggerEnergy_DeltaTSC->GetYaxis()->GetNbins(); iybin++) { //locHist_TaggerEnergy_DeltaTSC->GetYaxis()->GetNbins()
      TH1D* locHist_DeltaTSC = (TH1D*)locHist_TaggerEnergy_DeltaTSC->ProjectionX(Form("DeltaT_%d",iybin+1), iybin+1, iybin+1);
      TH1D* locHist_DeltaTSC_Amorph = (TH1D*)locHist_TaggerEnergy_DeltaTSC_Amorph->ProjectionX(Form("DeltaT_Amorph_%d",iybin+1), iybin+1, iybin+1);
      //locHist_DeltaTSC->Draw();

      int maxBin = locHist_DeltaTSC->GetMaximumBin();
      double deltaT_maxBin = locHist_DeltaTSC->GetXaxis()->GetBinCenter(maxBin);
      int deltaT_lowBin = locHist_DeltaTSC->GetXaxis()->FindBin(deltaT_maxBin - 2.0);
      int deltaT_highBin = locHist_DeltaTSC->GetXaxis()->FindBin(deltaT_maxBin + 1.99);
      int deltaT_lowBinAcc = locHist_DeltaTSC->GetXaxis()->FindBin(deltaT_maxBin + 2.0);
      int deltaT_highBinAcc = locHist_DeltaTSC->GetXaxis()->FindBin(deltaT_maxBin + 5.99);
      //cout<<endl;
      //cout<<maxBin<<" "<<deltaT_lowBin<<" "<<deltaT_highBin<<endl;
      //cout<<maxBin<<" "<<deltaT_lowBinAcc<<" "<<deltaT_highBinAcc<<endl;

      int maxBinAmorph = locHist_DeltaTSC_Amorph->GetMaximumBin();
      double deltaT_maxBinAmorph = locHist_DeltaTSC_Amorph->GetXaxis()->GetBinCenter(maxBinAmorph);
      int deltaT_lowBinAmorph = locHist_DeltaTSC_Amorph->GetXaxis()->FindBin(deltaT_maxBinAmorph - 2.0);
      int deltaT_highBinAmorph = locHist_DeltaTSC_Amorph->GetXaxis()->FindBin(deltaT_maxBinAmorph + 1.99);
      int deltaT_lowBinAmorphAcc = locHist_DeltaTSC_Amorph->GetXaxis()->FindBin(deltaT_maxBinAmorph + 2.0);
      int deltaT_highBinAmorphAcc = locHist_DeltaTSC_Amorph->GetXaxis()->FindBin(deltaT_maxBinAmorph + 5.99);

      double yield = locHist_DeltaTSC->Integral(deltaT_lowBin, deltaT_highBin);
      double yieldAcc = locHist_DeltaTSC->Integral(deltaT_lowBinAcc, deltaT_highBinAcc);
      double yieldAmorph = locHist_DeltaTSC_Amorph->Integral(deltaT_lowBinAmorph, deltaT_highBinAmorph);
      double yieldAmorphAcc = locHist_DeltaTSC_Amorph->Integral(deltaT_lowBinAmorphAcc, deltaT_highBinAmorphAcc);
      
      // subtract prompt from full sum including prompt and accidentals
      //yieldAcc -= yield;
      //yieldAmorphAcc -= yieldAmorph;

      //cout<<yield<<" "<<yieldAcc<<endl;
      //cout<<yieldAmorph<<" "<<yieldAmorphAcc<<endl;

      //cout<<endl;
      //cout<<locHist_TaggerEnergy->GetBinCenter(iybin+1)<<" "<<yieldAcc/yield/4<<" "<<yieldAmorphAcc/yieldAmorph/4<<endl;

      locHist_TaggerEnergy->SetBinContent(iybin+1, yield - yieldAcc);
      locHist_TaggerEnergy_Amorph->SetBinContent(iybin+1, yieldAmorph - yieldAmorphAcc);
      
      //locHist_TaggerEnergyAcc->SetBinContent(iybin+1, yieldAcc/4.); // - yieldAcc/4.);
      //locHist_TaggerEnergyAcc_Amorph->SetBinContent(iybin+1, yieldAmorphAcc/4.);
    }
#endif
   
#if 0
    /////////////////////////////////////
    // Channel independent time offset //
    /////////////////////////////////////
    int lowTime = locHist_TaggerEnergy_DeltaTSC->GetXaxis()->FindBin(optionLowTime[iopt]);
    int highTime = locHist_TaggerEnergy_DeltaTSC->GetXaxis()->FindBin(optionHighTime[iopt]);
    int lowTimeAcc = locHist_TaggerEnergy_DeltaTSC->GetXaxis()->FindBin(optionLowTimeAcc[iopt]);
    int highTimeAcc = locHist_TaggerEnergy_DeltaTSC->GetXaxis()->FindBin(optionHighTimeAcc[iopt]);
    cout<<lowTime<<" "<<highTime<<endl;
    cout<<lowTimeAcc<<" "<<highTimeAcc<<endl;

    TH1D* locHist_TaggerEnergy = (TH1D*)locHist_TaggerEnergy_DeltaTSC->ProjectionY(Form("TaggerEnergy_%d", iopt), lowTime, highTime);
    locHist_TaggerEnergy->SetTitle("Diamond");
    TH1D* locHist_TaggerEnergy_Amorph = (TH1D*)locHist_TaggerEnergy_DeltaTSC_Amorph->ProjectionY(Form("TaggerEnergy_Amorph_%d", iopt), lowTime, highTime);
    locHist_TaggerEnergy_Amorph->SetTitle("Amorphous Reference");
    TH1D* locHist_TaggerEnergyAcc = (TH1D*)locHist_TaggerEnergy_DeltaTSC->ProjectionY(Form("TaggerEnergyAcc_%d", iopt), lowTimeAcc, highTimeAcc);
    TH1D* locHist_TaggerEnergyAcc_Amorph = (TH1D*)locHist_TaggerEnergy_DeltaTSC_Amorph->ProjectionY(Form("TaggerEnergyAcc_Amorph_%d", iopt), lowTimeAcc, highTimeAcc);
    int rebin = optionRebin[iopt];
    locHist_TaggerEnergy->Rebin(rebin); locHist_TaggerEnergy_Amorph->Rebin(rebin);
    locHist_TaggerEnergyAcc->Rebin(rebin); locHist_TaggerEnergyAcc_Amorph->Rebin(rebin);
    
    //cout<<locHist_TaggerEnergy->Integral()<<" "<<locHist_TaggerEnergy_Amorph->Integral()<<endl;
    //cout<<locHist_TaggerEnergyAcc->Integral()<<" "<<locHist_TaggerEnergyAcc_Amorph->Integral()<<endl;

    // remove prompt from full integral to get accidental
    locHist_TaggerEnergyAcc->Add(locHist_TaggerEnergy, -1.);
    locHist_TaggerEnergyAcc_Amorph->Add(locHist_TaggerEnergy_Amorph, -1.);
    
    //cout<<locHist_TaggerEnergyAcc->Integral()<<" "<<locHist_TaggerEnergyAcc_Amorph->Integral()<<endl;

    // remove accidental from prompt to get true signal
    locHist_TaggerEnergy->Add(locHist_TaggerEnergyAcc, -1./optionScaleAcc[iopt]);
    locHist_TaggerEnergy_Amorph->Add(locHist_TaggerEnergyAcc_Amorph, -1./optionScaleAcc[iopt]);
#endif

    Double_t scaleFactor = 1.;

#if 0    
    // make enhancment histogram 
    TH1D* locHist_TaggerEnhancement = (TH1D*)locHist_TaggerEnergy->Clone();
    locHist_TaggerEnhancement->Divide(locHist_TaggerEnergy_Amorph);
    
    // scale to lowest energy bin
    Double_t scaleFactor = 1.;
    for(int i=0; i<locHist_TaggerEnhancement->GetXaxis()->GetNbins(); i++){
      if(locHist_TaggerEnhancement->GetBinContent(i)) {
	scaleFactor = locHist_TaggerEnhancement->GetBinContent(i);
	break;
      }
    }
    locHist_TaggerEnhancement->Scale(1./scaleFactor);
    
    locHist_TaggerEnhancement->SetTitle("Enhancement: Diamond/Amorphous");
    locHist_TaggerEnhancement->SetMinimum(0);
    
    if(showPlots) {
      TCanvas *dd = new TCanvas("dd","d", 1200, 800);
      dd->Divide(3,2);
      dd->cd(1);
      locHist_TaggerEnergy->Draw("h");
      dd->cd(2);
      locHist_TaggerEnergy_Amorph->Draw("h");
      dd->cd(3);
      locHist_TaggerEnhancement->Draw("h");
      //dd->cd(4);
      //locHist_TaggerEnergyAcc->Draw("h");
      //dd->cd(5);
      //locHist_TaggerEnergyAcc_Amorph->Draw("h");
    }
    //return;
#endif
    
    // Choose tagger or PS for enhancement fitting
    TH1D* locHist_TaggerEnhancement;
    TH1F* hEnhance;
    if(tagger_ps == 0) hEnhance = (TH1F*)locHist_TaggerEnhancement->Clone();
    else hEnhance = (TH1F*)locHist_PSEnhancement->Clone();
    
    /////////////////////////////////////
    // Pack histograms in Ken's format //
    /////////////////////////////////////

    nBins = hEnhance->GetXaxis()->GetNbins();
    //nBins /= rebin;
    cout<<"nBins="<<nBins<<endl;

    for(int n=0;n<nBins;n++){
      energy[n] = hEnhance->GetXaxis()->GetBinCenter(n+1)*1000.0;
      energy_GeV[n] = hEnhance->GetXaxis()->GetBinCenter(n+1);
      enhancement[n] = hEnhance->GetBinContent(n+1);
      enhancementErr[n] = hEnhance->GetBinError(n+1);
      if(enhancementErr[n]>2.0){
	enhancement[n] = 0.;
	enhancementErr[n] = 0.;
      }
      //if(enhancementErr[n]<=0.) cout<<energy_GeV[n]<<" "<<enhancementErr[n]<<endl;
    }
    
    //make some bin edges half way between the energy values.
    for(int b=0;b<nBins-1;b++){
      energyBins[b+1]=0.5*(energy[b]+energy[b+1]);
      energyBins_GeV[b+1]=0.5*(energy_GeV[b]+energy_GeV[b+1]);
    }
    //and the top and bottom have width of the adjacent bin
    energyBins[0]     = energyBins[1]       - (energyBins[2]      - energyBins[1]);
    energyBins[nBins] = energyBins[nBins-1] + (energyBins[nBins-1]- energyBins[nBins-2]);
    energyBins_GeV[0] = energyBins_GeV[1]   - (energyBins_GeV[2]  - energyBins_GeV[1]);
    energyBins_GeV[nBins] = energyBins_GeV[nBins-1] + (energyBins_GeV[nBins-1]- energyBins_GeV[nBins-2]);  
    
    // Histogram for enhancement data used in fit
    histD        = new TH1F("EnhancementData", "EnhancementData;Energy (MeV); Enhancement: Diamond/Amorph",nBins,energyBins);
    
    // Histograms to display the distributions after the fit
    histE        = new TH1F("Enhancement", "Enhancement;Energy (MeV)",nBins,energyBins);
    histP        = new TH1F("Polarization", "Polarization;Energy (MeV);Degree of Polarization",nBins,energyBins);
    
    histE->SetMinimum(0);
    histD->SetMinimum(0);
    histP->SetMinimum(0);
    histP->SetMaximum(1);
    histD->SetMarkerStyle(20);
    histD->SetMarkerSize(0.7);
    histD->GetXaxis()->SetNdivisions(10);
    histE->GetXaxis()->SetNdivisions(10);
    histP->GetXaxis()->SetNdivisions(10);
    histP->GetYaxis()->SetNdivisions(10);
    
    //fill the histD with the enhancement
    for(int n=0;n<nBins;n++){
      histD->SetBinContent(n+1,enhancement[n]);
      histD->SetBinError(n+1,enhancementErr[n]);
    }
    //Get rid of zeros
    //for(int n=1;n<=nBins-1;n++){
    //  if(histD->GetBinContent(n)<0.1) {
    //    histD->SetBinContent(n,histD->GetBinContent(n+1));
    //    histD->SetBinError(n,0.1);
    //  }
    //}
    //Get rid of zeros 2nd pass
    //for(int n=1;n<=nBins-1;n++){
    //  if(histD->GetBinContent(n)<0.1)histD->SetBinContent(n,histD->GetBinContent(n+1));
    //}
    
    //  Get rid of spikes up and down
    for(int n=2;n<=nBins-1;n++){
      diff1=(histD->GetBinContent(n)-histD->GetBinContent(n-1))/histD->GetBinContent(n-1);
      diff2=(histD->GetBinContent(n)-histD->GetBinContent(n+1))/histD->GetBinContent(n+1);
      //    cout << histD->GetBinCenter(n) << " " << histD->GetBinContent(n) << " " << diff1 << " " << diff2 << endl;
      if (((fabs(diff1)>0.2)&&(fabs(diff2)>0.2))&&(fabs(diff1-diff2)<0.1)){
	//cout << "****" << endl;
	//histD->SetBinContent(n,0.); //0.5*(histD->GetBinContent(n-1)+histD->GetBinContent(n+1)));
      }
    }
    
    // find a reasonable minumum spot to set to 1 for the baseline.
    // the lowest 5 channel mean between 0.2 and 0.95 of the range
    lowmean=1000000.0;
    //    for(int n=(int)(0.05*(float)nBins);n<=(int)(0.95*(float)nBins);n++){
    for(int n=3;n<=(int)(0.95*(float)nBins);n++){
      if((histD->Integral(n-2,n+2)<lowmean)){
	lowmean=histD->Integral(n-2,n+2);
	//cout << lowmean << endl;
      }
    }
    
    // get first guess at coherent peak bin
    int maxBin = 0;
    double maxEnhancement = 0;
    for(int i=0; i<nBins; i++) {
      //cout<<i+1<<" "<<histD->GetBinError(i+1)<<endl;
      if(histD->GetBinContent(i+1)>maxEnhancement && histD->GetBinError(i+1)<0.5) {
	maxBin = i+1;
	maxEnhancement = histD->GetBinContent(i+1);
      }
    }
    cout<<maxBin<<endl;
    
    // scale to minimum bin above coherent peak
    Double_t minEnhancement = 9e9;
    for(int i=0; i<histD->GetXaxis()->GetNbins(); i++){
      if(i+1<maxBin || i>histD->GetXaxis()->GetNbins()-5) continue; // only search above coherent peak
      if(histD->GetBinCenter(i+1) > 10000.0) continue;
      if(histD->GetBinContent(i+1) < minEnhancement && histD->GetBinContent(i+1) > 0.02) {
	minEnhancement = histD->GetBinContent(i+1);
	scaleFactor = histD->GetBinContent(i+1);
      }
    }
    cout<<"scale = "<<scaleFactor<<endl;
    histD->Scale(1./scaleFactor);
    if(minEnhancement > 8e9) return; // failed to find scale factor

    histD->GetXaxis()->SetRangeUser(6.0, beamMeV);
    histE->GetXaxis()->SetRangeUser(6.0, beamMeV);
    histP->GetXaxis()->SetRangeUser(6.0, beamMeV);

    genCanvas->cd(1);
    histD->SetMaximum(1.2*histD->GetMaximum());
    histD->SetMinimum(0.0);
    histD->Draw("P e");

    //////////////////////////////////////////
    // Determine initial parameters for fit //
    //////////////////////////////////////////
    
    // Try to make some guesses at the initial parameters
    gausFit->SetRange(histD->GetBinCenter(maxBin),histD->GetBinCenter(maxBin)+300.0);
    gausFit->SetParameter(1,histD->GetBinCenter(maxBin));
    gausFit->SetParameter(2,10.0);
    gausFit->SetParameter(3,1.0);
    histD->Fit(gausFit,"rN");
    //histD->Fit(gausFit,"r");
    //return;
    lowmean=0.0;
    //Get the edge from the derivative
    for(float d = histD->GetBinCenter(maxBin);d < histD->GetBinCenter(maxBin+90.0);d+=0.1){ 
      if(gausFit->Derivative(d)<lowmean){
	lowmean=gausFit->Derivative(d);
	fitedge=d;
      }
    }
    double fitspread = gausFit->GetParameter(2); //10;
    cout << "edge = " << fitedge << " MeV and spread " << fitspread << endl;
    if(fitedge < 7000.0 || fitedge > 9500. || fitspread < 0.) {
      fitedge = 8850.;
      fitspread = 40;
    }
    //fitedge = 8800.;
    //fitspread = 50;

    //Now we have enough information to set the basic parameters
    parFromHuman(beamMeV,fitedge,fitspread,colliDist_m,colliRad_mm,nVec,par);
    
    //set the intensities
    for(int v=0;v<par[NVEC];v++){                                               //give the vectors intensities
      par[IVEC+v] = histD->GetMaximum()*2.0/((Double_t)VECTORS[v]*(Double_t)VECTORS[v]);      //tailing off as 1/VECTORS[v]^2
      //cout << IVEC+v << "  v   " << par[IVEC+v] << endl; 
    }
    enhFromParams(par);
    
    //Redo the intensities according to a the calc / data ratio
    scalefac=histD->GetMaximum()/histE->GetMaximum();
    for(int v=0;v<par[NVEC];v++){ //give the vectors intensities
      par[IVEC+v]*=scalefac;
    }
    enhFromParams(par);
    histE->SetLineColor(2);
    histE->Draw("HIST same");

    genCanvas->cd(2);
    histP->SetLineColor(2);
    //histP->Print("all");
    histP->Draw("HIST");
    //if(iopt == 3) return;
    
    genCanvas->Update();
    gSystem->ProcessEvents();
    gSystem->Sleep(500);
    
    //////////////////////////////////////
    // Main fitting of enhancement data //
    //////////////////////////////////////
    
    //Set the range of the fit to be some sensible amount below peak and just past the 2nd peak.
    fitMinBin=histE->FindBin(histD->GetBinCenter(histD->GetMaximumBin())-LOWFIT);
    fitMinBin=histE->FindBin(7000.0);

    //fitMaxBin    = histE->FindBin(par[E0MEV]/((((2.0/4.0)*((par[E0MEV]/histD->GetBinCenter(histD->GetMaximumBin()))-1.0))+1.0)));
    fitMaxBin    = 5+histE->FindBin(par[E0MEV]/((((1.0/par[NVEC])*((par[E0MEV]/histD->GetBinCenter(histD->GetMaximumBin()))-1.0))+1.0)));
    fitMaxBin    = histE->FindBin(9800.0);

    cout << "fitMinBin " << histE->GetBinCenter(fitMinBin) << endl;
    cout << "fitMaxBin " << histE->GetBinCenter(fitMaxBin) << endl;
    
    min = ROOT::Math::Factory::CreateMinimizer("Minuit", "Simple");
    min->SetPrintLevel(3);
    
    // set tolerance , etc...
    min->SetMaxFunctionCalls(1000000); // for Minuit/Minuit2 
    min->SetMaxIterations(10000);  // for GSL 
    min->SetTolerance(0.001);
    min->SetPrintLevel(1);
    
    ROOT::Math::Functor ft(&efit,IVEC+nVec);     
    min->SetFunction(ft);
    
    //Now set the variables
    min->SetLimitedVariable(THETA,   "Theta",   par[THETA],      par[THETA]/100.0,  0.95*par[THETA], 1.05*par[THETA]);
    min->SetLimitedVariable(SIGMA,   "Sigma",   2.5*par[SIGMA],  par[SIGMA]/100.0,  par[SIGMA],  5.0*par[SIGMA]);
    min->SetLimitedVariable(THETAR,  "Thetar",  par[THETAR],     par[THETAR]/100.0, 0.95*par[THETAR], 1.05*par[THETAR]);
    min->SetLimitedVariable(SIGMAR,  "Sigmar",  0.5*par[SIGMAR], par[SIGMAR]/100.0, 0.1*par[SIGMAR], 50.0*par[SIGMAR]);
    min->SetFixedVariable(E0MEV,     "E0MeV",   par[E0MEV]);  //no of vectors
    min->SetFixedVariable(NVEC,      "Nvec",    par[NVEC]);  //no of vectors
    for(int n=0;n<nVec;n++){
      sprintf(name,"Vec0%d%d", VECTORS[n],VECTORS[n]);
      min->SetLimitedVariable(n+IVEC, name, par[n+IVEC], par[n+IVEC]/100.0,0.0,10*par[n+IVEC]); 
    }
    // min->FixVariable(THETAR);
    // min->FixVariable(SIGMAR);
    
    verbose=1;             //make it show the fit as it's improving
    bestChisq=100000.00;   //set this high for starters
    
    min->Minimize(); 
    min->Minimize(); 
    min->Minimize(); 

    min->PrintResults();
    for(int i=0; i<10; i++)
      cout<<bestPar[i]<<endl;
    
    
    //////////////////////////////////
    // Display best fit enh and pol //
    //////////////////////////////////
    genCanvas->cd(1);
    enhFromParams(bestPar);
    histD->Draw("P");
    histE->SetLineColor(2);
    histE->Draw("HIST same");
    
    genCanvas->cd(2);
    histP->SetLineColor(2);
    histP->Draw("HIST");
    
    genCanvas->Print("enhancementFit.pdf");

    TFile *outFile = new TFile("enhancementFit.root", "recreate");
    const int nBins = histP->GetNbinsX();
    double x[nBins];
    double y[nBins];
    for(int i=0; i<nBins; i++) {
      x[i] = histP->GetBinCenter(i+1)/1000.;
      y[i] = histP->GetBinContent(i+1);
    }
    TGraph *gr = new TGraph(nBins, x, y);
    //gr->Print();
    gr->Write();
    histD->Write();
    outFile->Close();
    
    /*
    // fill new canvas with GeV x-axis
    TH1F *histD_GeV = new TH1F("EnhancementData_GeV", "EnhancmentData; Energy (GeV)", nBins, energyBins_GeV);
    TH1F *histE_GeV = new TH1F("Enhancement_GeV", "Enhancement; Energy (GeV)", nBins, energyBins_GeV);
    TH1F *histP_GeV = new TH1F("Polarization_GeV", "Polarization; Energy (GeV); Degree of Polarization", nBins, energyBins_GeV); 
    for(int i=0; i<histD->GetXaxis()->GetNbins(); i++) {
    histD_GeV->SetBinContent(i+1, histD->GetBinContent(i+1));
    histE_GeV->SetBinContent(i+1, histE->GetBinContent(i+1));
    histP_GeV->SetBinContent(i+1, histP->GetBinContent(i+1));
    }
    histE_GeV->SetMinimum(0);  histE_GeV->SetMaximum(6.0);
    histP_GeV->SetMinimum(0);  histP_GeV->SetMaximum(1.0);
    histD_GeV->SetMarkerStyle(20); histD_GeV->SetMarkerSize(0.7);
    
    TCanvas *gg = new TCanvas("gg", "gg", 50, 50, 800, 1000);
    gg->Divide(1,2);
    gg->cd(1);
    histD_GeV->Draw("P");
    //histE_GeV->SetLineColor(2);
    //histE_GeV->Draw("hist same");
    
    gg->cd(2);
    histP_GeV->SetLineColor(2);
    histP_GeV->Draw("hist");  
    */


    ////////////////////////////////////
    // Clear previous fit information //
    ////////////////////////////////////
    //histP->Reset();
    //histE->Reset();
    //histD->Reset();
    hEnhance->Reset();
    delete histP;
    delete histE;
    delete histD;
  }

  return;
}
  
void parFromHuman(Double_t beamMeV, Double_t edgeMeV, Double_t spreadMeV, Double_t colliDist_m, Double_t colliRad_mm, Int_t nVec, Double_t *par){

  //takes some physical quantities and makes them into parameters, then calls the 
  //enhFromParams function.
  
  
  //  Double_t par[10];                                                           //array of parameters
  Int_t g = 2;                                                                //variables used in CLAS note
  Double_t E0 = beamMeV;
  Double_t Eg = edgeMeV;
  
  
  par[THETA]  = k/(g*E0*E0*((1/Eg)-(1/E0)));                                  //theta from edge and beam energy
  par[SIGMA]  = (par[THETA]-(k/(g*E0*E0*((1/(Eg-spreadMeV))-(1/E0)))))/3.0;   //spread in theta from spread in edge 
  //par[SIGMA]  = 1.776e-04;
  par[THETAR] = E0*0.001*colliRad_mm/colliDist_m;                             //cut from collimator
  par[SIGMAR] = 10.0*par[THETAR]*par[SIGMA]/par[THETA];                       //smear in above same fractional sigma as above
  par[E0MEV]  = E0;                                                           //beam energy
  par[NVEC]   = (Double_t)nVec;                                               //no of harmonics
  
  for(int v=0;v<par[NVEC];v++){                                               //give the vectors intensities
    par[IVEC+v] = 2.0/(Double_t)VECTORS[v];                                   //tailing off as 1/VECTORS[v]
    //cout << IVEC+v << "  v   " << par[IVEC+v] << endl; 
  }
}  
  
  
void enhFromParams(Double_t *par){
  //make an enhancement and corresponding polarization from some the parameters as defined in the CLAS note.
  //this function can be called stand alone, but will also be called many times from the fitting function

  Double_t xd[10];
  Double_t xc[10];
  Double_t Q[10];
  Double_t cohContrib;
  Double_t cohTotal;
  Double_t phiTotal;
  Double_t etotal;
  Double_t ptotal;
  Double_t x=0.0;
  Int_t    g=0;
  Double_t weight=0.0;
  Double_t weightSum=0.0;
  Double_t polSum=0.0;
  Double_t phi,chi,cd;
  Double_t amo;
  Int_t jbin=0;
 
  //loop over sigma
  // for(int p=0;p<10;p++){
  //  cout << p << ": " << par[p] << ", ";
  //}
  //cout << endl;

  // if needed, make some hists
  if(!histE){
    histE        = new TH1F("Enhancement", "Enhancement",  1000, 0, par[E0MEV]);
    histP        = new TH1F("Polarization", "Polarization",1000, 0, par[E0MEV]);
    histE->SetMinimum(0);
    histP->SetMinimum(0);
    histP->SetMaximum(1);
  }    
  if(!thetaPol){
    weightHist   = new TH1F("weightHist",  "weightHist", THETASTEPS+1, 0, THETASTEPS+1 );
    thetaWeight  = new TH2F("thetaWeight", "thetaWeight",histE->GetNbinsX(), histE->GetXaxis()->GetXbins()->GetArray(), THETASTEPS+1,0, THETASTEPS+1);
    thetaPol     = new TH2F("thetaPol",    "thetaPol",   histE->GetNbinsX(), histE->GetXaxis()->GetXbins()->GetArray(), THETASTEPS+1,0, THETASTEPS+1);
    thetaItot    = new TH2F("thetaItot",   "thetaItot",  histE->GetNbinsX(), histE->GetXaxis()->GetXbins()->GetArray(), THETASTEPS+1,0, THETASTEPS+1);
  }
  if(histE->GetNbinsX() < thetaWeight->GetNbinsX()) {
    int rebin = thetaWeight->GetNbinsX()/histE->GetNbinsX();
    thetaWeight->Rebin2D(rebin,1);
    thetaPol->Rebin2D(rebin,1);
    thetaItot->Rebin2D(rebin,1);
  }

  //reset them all for fresh filling
  histE->Reset("ICE");
  histP->Reset("ICE");
  thetaPol->Reset("ICE");
  thetaItot->Reset("ICE");
  weightHist->Reset("ICE");
  thetaWeight->Reset("ICE");

  for(Double_t j=par[THETA]-3.0*par[SIGMA];j<=par[THETA]+3.001*par[SIGMA];j+=(6.0*par[SIGMA])/THETASTEPS){
    
    weight=TMath::Gaus(j,par[THETA],par[SIGMA]);   //get the weight from the gaussian
    weightSum+=weight;                             //add to sum      
    
    //find the discontinuity for each vector
    for(int v=0;v<par[NVEC];v++){
      g=VECTORS[v];
      xd[v]=1.0/((k/(g*par[E0MEV]*j))+1.0);
      Q[v]=(1.0-xd[v])/xd[v];
      xc[v]=xd[v]/(1+((par[THETAR]*par[THETAR])*(1-xd[v])));
      //   cout << "xc" << xc[v] << "    " << xd[v] << endl;
      //xc[v]=xd[v]/(1+((j*j)*(1-xd[v])));
    }

    //loop over all bins in the histogram
    for(int bin=1;bin<=histE->GetNbinsX();bin++){
      x=histE->GetBinCenter(bin)/par[E0MEV];            //find the value of the bin
      amo=1/x;                                    //assume amo = inc = 1/x over regio of interest
      
      cohTotal=0.0;
      phiTotal=0.0;
      
      //loop over all the vectors
      for(int v=0;v<par[NVEC];v++){
	if(x>xd[v]) continue;           //only do up to x_dg
	 
	//work out chi and phi
	phi=(2*Q[v]*Q[v]*x*x)/((1-x)*(1+((1-x)*(1-x))-((4*Q[v]*Q[v]*x*x/(1-x))*(((1-x)/(Q[v]*x))-1))));
	chi=((Q[v]*Q[v]*x)/(1-x))*(1+((1-x)*(1-x))-((4*Q[v]*Q[v]*x*x/(1-x))*(((1-x)/(Q[v]*x))-1)));
	//	cout << j  << "  " << chi << endl;
	cd=0.5*(1+TMath::Erf((x-xc[v])/(TMath::Sqrt(2)*par[SIGMAR])));

	//get coherent contrib for the vector
	cohContrib=cd*par[IVEC+v]*chi;

	//add to the total and update the phi total
	cohTotal+=cohContrib;
	phiTotal+=cohContrib*phi;

      }
      if(cohTotal>0.0) {
	phiTotal/=cohTotal;   //divide by the cohTotal to get the weighted dmean phi
	//cout << x << " " << phiTotal << " " << cohTotal << " " << weight << endl;	 
      }

      //enhancement = coherent total + inc (or amo).
      etotal=(amo+cohTotal)/amo;
      //and pol like this
      //      ptotal=phiTotal*cohTotal/(cohTotal + amo);
      ptotal=phiTotal*cohTotal;

      //add the weighted contribution to the enhancement
      histE->Fill(x*par[E0MEV],weight*etotal);

      //keep the pol for this x,theta coord
      thetaPol->Fill(x*par[E0MEV],jbin,ptotal);

      //keep the total intensity for this x,theta coord
      thetaItot->Fill(x*par[E0MEV],jbin,cohTotal+amo);
    }
    
    //save the weight for this theta point
    weightHist->Fill(jbin,weight);
    jbin++;

  }
  //normalize the sum of the weighted enhancements
  histE->Scale(1.0/weightSum);
  
  
  //loop over each x bin, adding the weighted contribs from each theta pos
  for(int bin=1; bin<=histP->GetNbinsX(); bin++){
    weightSum=0.0;
    polSum=0.0;
    
    for(int jb=1;jb<=weightHist->GetNbinsX();jb++){
      weight=weightHist->GetBinContent(jb);

      //      polSum+=thetaPol->GetBinContent(bin,jb)*thetaItot->GetBinContent(bin,jb)*weight;
      polSum+=thetaPol->GetBinContent(bin,jb)*weight;
      weightSum+=thetaItot->GetBinContent(bin,jb)*weight;
      //polSum+=thetaPol->GetBinContent(bin,jb)*weight;
      //weightSum+=weight;
    }
    polSum/=weightSum;
    histP->SetBinContent(bin,polSum);
  } 
}
