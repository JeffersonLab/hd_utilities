// glxtools.C - useful functions for glx*
// created on: 07.04.2017
// initial author: r.dzhygadlo at gsi.de
// -----------------------------------------


#include "TROOT.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TMath.h"
#include "TChain.h"
#include "TGaxis.h"
#include "TColor.h"
#include "TString.h"
#include "TArrayD.h"
#include "TSpectrum.h"
#include "TSpectrum2.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "TRandom2.h"
#include "TError.h"
#include "TPaveStats.h"
#include "TObjString.h"
#include "TApplication.h"
#include <TLegend.h>
#include <TAxis.h>
#include <TPaletteAxis.h>
#include <TRandom.h>
#include <TCutG.h>

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef glx__sim
class DrcEvent;
class DrcHit;
DrcEvent* glx_event(0);
#endif

Int_t glx_geometry=0;

const Int_t  glx_nrow(6),glx_ncol(18);
const Int_t  glx_npmt(glx_nrow*glx_ncol);
const Int_t  glx_npix(64);
const Int_t  glx_maxch(glx_npmt*glx_npix);
const Int_t  glx_nch(glx_npmt*glx_npix);
const Int_t  glx_npixtot(glx_npmt*glx_npix*2); // 2 optical box

TChain*  glx_ch(0);
Int_t    glx_entries(0), glx_momentum(0),glx_pdg(0),glx_test1(0),glx_test2(0);
Double_t glx_theta(0),glx_phi(0);
TString  glx_savepath(""), glx_info("");
TH2F*    glx_hdigi[glx_npmt];
TPad*    glx_hpads[glx_npmt];
TPad*    glx_hpglobal;
TCanvas* glx_cdigi;
TClonesArray* glx_events;
int glx_apdg[]={11,13,211,321,2212};
double glx_mass[] = {0.000511,0.1056584,0.139570,0.49368,0.9382723};
TString glx_names[] = {"electron","muon","pion","kaon","proton"};

Int_t map_mpc[glx_maxch/64][glx_npix];
Int_t map_pmt[glx_maxch];
Int_t map_pix[glx_maxch];
Int_t map_row[glx_maxch];
Int_t map_col[glx_maxch];
Int_t map_ssp_slot[glx_maxch];
Int_t map_ssp_fiber[glx_maxch];
Double_t glx_particleArray[3000];

void glx_createMap(){
  for(Int_t ch=0; ch<glx_maxch; ch++){
    Int_t pmt = ch/64;
    Int_t pix = ch%64;	
    Int_t col = pix/2 - 8*(pix/16);
    Int_t row = pix%2 + 2*(pix/16);
    pix = col+8*row;
    int slot = 4;
    int br= pmt%18;
    if(pmt>53) br+=18;

    if((br>8 && br<18) || br>21) slot=5;
    
    if(br<4) br=12+br%4;
    else if(br<8) br=20+br%4;
    else if(br<12) br=4+br%4;
    else if(br<16) br=12+br%4;
    else if(br<18) br=20+br%4;
    else if(br<22) br=16+(br-2)%4;
    else if(br<26) br=(br-2)%4;
    else if(br<30) br=8+(br-2)%4;
    else if(br<34) br=16+(br-2)%4;
    else if(br<36) br=22+(br-2)%4;
      
    map_mpc[pmt][pix]=ch;
    map_pmt[ch] = pmt;
    map_pix[ch] = pix;
    map_row[ch] = row;
    map_col[ch] = col;
    map_ssp_slot[ch] = slot;
    map_ssp_fiber[ch] = br;
  } 

  for(Int_t i=0; i<5; i++){
    glx_particleArray[glx_apdg[i]]=i;
  }
}

Int_t glx_getChNum(Int_t npmt, Int_t npix){
  Int_t ch = -1;
  ch = 64*npmt+npix;
  return ch;
}

TPaletteAxis * glx_palette;

TString glx_drawDigi(TString digidata="", Int_t layoutId = 0, Double_t maxz = 0, Double_t minz = 0){
  if(!glx_cdigi) glx_cdigi = new TCanvas("glx_cdigi","glx_cdigi",800,350); //1200,480);
  glx_cdigi->cd();
  if(!glx_hpglobal){
    glx_hpglobal = new TPad("P","T",0.005,0.06,0.95,0.94);
    glx_hpglobal->SetFillStyle(0);
    glx_hpglobal->Draw();
  }
  glx_hpglobal->cd();
  Int_t nrow(glx_nrow), ncol(glx_ncol);
 
  if(layoutId > 0){
  }else{
    float bw = 0.001, bh = 0.005;
    Int_t padi = 0;
    if(!glx_hpads[0]){
      for(int i=0; i<ncol; i++){
	for(int j=0; j<nrow;j++){
	  glx_hpads[padi] =  new TPad(Form("P%d",padi),"T", i/(Double_t)ncol+bw, 1-(j/(Double_t)nrow+bh), (i+1)/(Double_t)ncol-bw, 1-((j+1)/(Double_t)nrow-bh), 21);
	  // glx_hpads[padi]->SetFillColor(kCyan-8);
	  glx_hpads[padi]->SetFillColor(kCyan-10);
	  glx_hpads[padi]->SetMargin(0.04,0.04,0.04,0.04);
	  // if((j+1)%6 != 0)
	  glx_hpads[padi]->Draw();
	  padi++;	   
	}
      }
    }

  }
    
  Double_t tmax;
  Double_t max=0;
  if(maxz==0){
    for(Int_t p=0; p<nrow*ncol;p++){
      tmax = glx_hdigi[p]->GetMaximum();
      if(max<tmax) max = tmax;
    }
  }else{
    max = maxz;
  }

  if(maxz==-2 && minz==-2){ // optimize range
    for(Int_t p=0; p<nrow*ncol;p++){
      tmax = glx_hdigi[p]->GetMaximum();
      if(max<tmax) max = tmax;
    }
    if(max < 100) max = 100;
    Int_t tbins = 2000;
    TH1F *h = new TH1F("","",tbins,0,max);
    for(Int_t p=0; p<nrow*ncol;p++){
      for(Int_t i=0; i<64; i++){
	Double_t val = glx_hdigi[p]->GetBinContent(i);
	if(val!=0) h->Fill(val);
      }
    }
    Double_t integral;
    for(Int_t i=0; i<tbins; i++){
      integral = h->Integral(0,i);
      if(integral>5) {
	minz = h->GetBinCenter(i);
	break;
      } 
    }

    for(Int_t i=tbins; i>0; i--){
      integral = h->Integral(i,tbins);
      if(integral>5) {
	max = h->GetBinCenter(i);
	break;
      } 
    }
  }

  Int_t nnmax(0);  
  glx_hdigi[nnmax]->GetZaxis()->SetLabelSize(0.06);
  
  for(Int_t m=0; m<nrow*ncol;m++){
    Int_t nm=(5-m%6)*glx_ncol + m/6;
    glx_hpads[m]->cd();
    glx_hpads[m]->SetName(Form("p_%d",nm));
    glx_hdigi[nm]->Draw("col");
    if(maxz==-1)  max = glx_hdigi[nm]->GetBinContent(glx_hdigi[nm]->GetMaximumBin());
    if(nnmax<glx_hdigi[nm]->GetEntries()) nnmax=nm;
    glx_hdigi[nm]->SetMaximum(max);
    glx_hdigi[nm]->SetMinimum(minz);
    for(Int_t i=1; i<=8; i++){
      for(Int_t j=1; j<=8; j++){
  	Double_t weight = (double)(glx_hdigi[nm]->GetBinContent(i,j))/(double)max *255;
  	if(weight > 0) digidata += Form("%d,%d,%d\n", nm, (j-1)*8+i-1, (Int_t)weight);
      }
    }
  }

  glx_cdigi->Modified();
  glx_cdigi->Update();

  // nnmax++; //!
  
  glx_cdigi->cd();
  cout<<"+++nnmax = "<<nnmax<<endl;
  glx_hdigi[nnmax]->GetZaxis()->SetLabelSize(0.04);
  glx_hdigi[nnmax]->GetZaxis()->SetTickLength(0.01);
  if(glx_palette) delete glx_palette;
  glx_palette = new TPaletteAxis(0.955,0.1,0.965,0.90,((TH1 *)(glx_hdigi[nnmax])->Clone()));
  glx_palette->Draw();
  
  glx_cdigi->Modified();
  glx_cdigi->Update();
  return digidata;
}
void glx_initDigi(Int_t type=0){
  TGaxis::SetMaxDigits(3);
  if(type == 0){
    for(Int_t m=0; m<glx_npmt;m++){	
      glx_hdigi[m] = new TH2F( Form("pmt%d", m),Form("pmt%d", m),8,0.,8.,8,0.,8.);
      glx_hdigi[m]->SetStats(0);
      glx_hdigi[m]->SetTitle(0);
      glx_hdigi[m]->GetXaxis()->SetNdivisions(10);
      glx_hdigi[m]->GetYaxis()->SetNdivisions(10);
      glx_hdigi[m]->GetXaxis()->SetLabelOffset(100);
      glx_hdigi[m]->GetYaxis()->SetLabelOffset(100);
      glx_hdigi[m]->GetXaxis()->SetTickLength(1);
      glx_hdigi[m]->GetYaxis()->SetTickLength(1);
      glx_hdigi[m]->GetXaxis()->SetAxisColor(15);
      glx_hdigi[m]->GetYaxis()->SetAxisColor(15);
    }
  }
}

void glx_resetDigi(){
    for(Int_t m=0; m<glx_npmt;m++){	
      glx_hdigi[m]->Reset("M");
    }
}

void glx_axisHits800x500(TH2 * hist){
  hist->SetStats(0);
  hist->SetTitle(Form("%d hits",(Int_t)hist->GetEntries()));
  hist->GetXaxis()->SetTitle("z, [cm]");
  hist->GetXaxis()->SetTitleSize(0.05);
  hist->GetXaxis()->SetTitleOffset(0.8);
  hist->GetYaxis()->SetTitle("y, [cm]");
  hist->GetYaxis()->SetTitleSize(0.05);
  hist->GetYaxis()->SetTitleOffset(0.7);
}

void glx_axisAngle800x500(TH2 * hist){
  hist->SetStats(0);
  hist->SetTitle(Form("%d hits",(Int_t)hist->GetEntries()));
  hist->GetXaxis()->SetTitle("#theta, [degree]");
  hist->GetXaxis()->SetTitleSize(0.05);
  hist->GetXaxis()->SetTitleOffset(0.8);
  hist->GetYaxis()->SetTitle("photons per track, [#]");
  hist->GetYaxis()->SetTitleSize(0.05);
  hist->GetYaxis()->SetTitleOffset(0.7);
}

void glx_axisAngle800x500(TH1 * hist){
  hist->SetStats(0);
  hist->SetTitle(Form("%d hits",(Int_t)hist->GetEntries()));
  hist->GetXaxis()->SetTitle("#theta [degree]");
  hist->GetXaxis()->SetTitleSize(0.05);
  hist->GetXaxis()->SetTitleOffset(0.8);
  hist->GetYaxis()->SetTitle("photons per track, [#]");
  hist->GetYaxis()->SetTitleSize(0.05);
  hist->GetYaxis()->SetTitleOffset(0.7);
}

void glx_axisTime800x500(TH2 * hist){
  hist->GetXaxis()->SetTitle("time [ns]");
  hist->GetXaxis()->SetTitleSize(0.05);
  hist->GetXaxis()->SetTitleOffset(0.8);
  hist->GetYaxis()->SetTitle("entries #");
  hist->GetYaxis()->SetTitleSize(0.05);
  hist->GetYaxis()->SetTitleOffset(0.7);
  hist->SetLineColor(1);
}

void glx_axisTime800x500(TH1 * hist, TString xtitle = "time [ns]"){
  TGaxis::SetMaxDigits(3);
  hist->GetXaxis()->SetTitle(xtitle);
  hist->GetXaxis()->SetTitleSize(0.05);
  hist->GetXaxis()->SetTitleOffset(0.8);
  hist->GetYaxis()->SetTitle("entries [#]");
  hist->GetYaxis()->SetTitleSize(0.05);
  hist->GetYaxis()->SetTitleOffset(0.7);
  hist->SetLineColor(1);
}

void glx_setPrettyStyle(){
  // Canvas printing details: white bg, no borders.
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasBorderSize(0);

  // Canvas frame printing details: white bg, no borders.
  gStyle->SetFrameFillColor(0);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(0);

  // Plot title details: centered, no bg, no border, nice font.
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleW(0.8);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFillColor(0);

  // Font details for titles and labels.
  gStyle->SetTitleFont(42, "xyz");
  gStyle->SetTitleFont(42, "pad");
  gStyle->SetLabelFont(42, "xyz");
  gStyle->SetLabelFont(42, "pad");

  // Details for stat box.
  gStyle->SetStatColor(0);
  gStyle->SetStatFont(42);
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatX(0.975);
  gStyle->SetStatY(0.9);

  // gStyle->SetOptStat(0);
}

void glx_setRootPalette(Int_t pal = 0){

 // pal =  1: rainbow\n"
 // pal =  2: reverse-rainbow\n"
 // pal =  3: amber\n"
 // pal =  4: reverse-amber\n"
 // pal =  5: blue/white\n"
 // pal =  6: white/blue\n"
 // pal =  7: red temperature\n"
 // pal =  8: reverse-red temperature\n"
 // pal =  9: green/white\n"
 // pal = 10: white/green\n"
 // pal = 11: orange/blue\n"
 // pal = 12: blue/orange\n"
 // pal = 13: white/black\n"
 // pal = 14: black/white\n"

  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
  gStyle->SetNumberContours(NCont);

  if (pal < 1 && pal> 14) return;
  else pal--;

  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[14][NRGBs]   = {{ 0.00, 0.00, 0.87, 1.00, 0.51 },
			       { 0.51, 1.00, 0.87, 0.00, 0.00 },
			       { 0.17, 0.39, 0.62, 0.79, 1.00 },
			       { 1.00, 0.79, 0.62, 0.39, 0.17 },
			       { 0.00, 0.00, 0.00, 0.38, 1.00 },
			       { 1.00, 0.38, 0.00, 0.00, 0.00 },
			       { 0.00, 0.50, 0.89, 0.95, 1.00 },
			       { 1.00, 0.95, 0.89, 0.50, 0.00 },
			       { 0.00, 0.00, 0.38, 0.75, 1.00 },
			       { 0.00, 0.34, 0.61, 0.84, 1.00 },
			       { 0.75, 1.00, 0.24, 0.00, 0.00 },
			       { 0.00, 0.00, 0.24, 1.00, 0.75 },
			       { 0.00, 0.34, 0.61, 0.84, 1.00 },
			       { 1.00, 0.84, 0.61, 0.34, 0.00 }
  };
  Double_t green[14][NRGBs] = {{ 0.00, 0.81, 1.00, 0.20, 0.00 },		    
			       { 0.00, 0.20, 1.00, 0.81, 0.00 },
			       { 0.01, 0.02, 0.39, 0.68, 1.00 },
			       { 1.00, 0.68, 0.39, 0.02, 0.01 },
			       { 0.00, 0.00, 0.38, 0.76, 1.00 },
			       { 1.00, 0.76, 0.38, 0.00, 0.00 },
			       { 0.00, 0.00, 0.27, 0.71, 1.00 },
			       { 1.00, 0.71, 0.27, 0.00, 0.00 },
			       { 0.00, 0.35, 0.62, 0.85, 1.00 },
			       { 1.00, 0.75, 0.38, 0.00, 0.00 },
			       { 0.24, 1.00, 0.75, 0.18, 0.00 },
			       { 0.00, 0.18, 0.75, 1.00, 0.24 },
			       { 0.00, 0.34, 0.61, 0.84, 1.00 },
			       { 1.00, 0.84, 0.61, 0.34, 0.00 }
  };
  Double_t blue[14][NRGBs]  = {{ 0.51, 1.00, 0.12, 0.00, 0.00 },
			       { 0.00, 0.00, 0.12, 1.00, 0.51 },
			       { 0.00, 0.09, 0.18, 0.09, 0.00 },
			       { 0.00, 0.09, 0.18, 0.09, 0.00 },
			       { 0.00, 0.47, 0.83, 1.00, 1.00 },
			       { 1.00, 1.00, 0.83, 0.47, 0.00 },
			       { 0.00, 0.00, 0.00, 0.40, 1.00 },
			       { 1.00, 0.40, 0.00, 0.00, 0.00 },
			       { 0.00, 0.00, 0.00, 0.47, 1.00 },
			       { 1.00, 0.47, 0.00, 0.00, 0.00 },
			       { 0.00, 0.62, 1.00, 0.68, 0.12 },
			       { 0.12, 0.68, 1.00, 0.62, 0.00 },
			       { 0.00, 0.34, 0.61, 0.84, 1.00 },
			       { 1.00, 0.84, 0.61, 0.34, 0.00 }
  };


  TColor::CreateGradientColorTable(NRGBs, stops, red[pal], green[pal], blue[pal], NCont);
 
}

#ifdef glx__sim
bool glx_init(TString inFile="../build/hits.root", Int_t bdigi=0, TString savepath=""){
  if(inFile==""){
    std::cout<<"glxtools: no input file "<<std::endl;    
    return false;
  }
  if(savepath!="") glx_savepath=savepath;
  glx_setRootPalette(1);
  delete glx_ch;

  glx_ch = new TChain("dirc");

  glx_ch->Add(inFile);
  glx_ch->SetBranchAddress("DrcEvent", &glx_event);
  glx_entries = glx_ch->GetEntries();
  std::cout<<"Entries in chain:  "<<glx_entries <<std::endl;
  if(bdigi == 1) glx_initDigi();
  return true;
}

bool glx_initc(TString inFile="../build/hits.root", Int_t bdigi=0, TString savepath=""){
  if(inFile=="") return false;
  if(savepath!="") glx_savepath=savepath;
  glx_setRootPalette(1);
  delete glx_ch;

  glx_ch = new TChain("dirc");
  glx_ch->Add(inFile);
  glx_events = new TClonesArray("DrcEvent");
  glx_ch->SetBranchAddress("DrcEvent", &glx_events);
  
  glx_entries = glx_ch->GetEntries();
  std::cout<<"Entries in chain:  "<<glx_entries <<std::endl;
  if(bdigi == 1) glx_initDigi();
  return true;
}

void glx_nextEvent(Int_t ievent, Int_t printstep){
  glx_ch->GetEntry(ievent);
  if(ievent%printstep==0 && ievent!=0) cout<<"Event # "<<ievent<< " # hits "<<glx_event->GetHitSize()<<endl;
  if(ievent == 0){
    if(gROOT->GetApplication()){
      TIter next(gROOT->GetApplication()->InputFiles());
      TObjString *os=0;
      while((os = (TObjString*)next())){
	glx_info += os->GetString()+" ";
      }
      glx_info += "\n";
    }
    glx_momentum = glx_event->GetMomentum().Mag() +0.01;    
    glx_pdg =  glx_event->GetPdg();
    glx_test1 = glx_event->GetTest1();
    glx_test2 = glx_event->GetTest2();
    glx_theta=glx_event->GetMomentum().Theta()*180/TMath::Pi();
    glx_phi=glx_event->GetMomentum().Phi()*180/TMath::Pi();
  }
}

void glx_nextEventc(Int_t ievent,Int_t itrack, Int_t printstep){
  glx_event= (DrcEvent*) glx_events->At(itrack);
  if(ievent%printstep==0 && ievent!=0 && itrack==0) cout<<"Event # "<<ievent<< " # hits "<<glx_event->GetHitSize()<<endl;
  if(ievent == 0 && itrack==0 && gROOT->GetApplication()){
    TIter next(gROOT->GetApplication()->InputFiles());
    TObjString *os=0;
    while((os = (TObjString*)next())){
      glx_info += os->GetString()+" ";
    }
    glx_info += "\n";
  }
  glx_momentum = glx_event->GetMomentum().Mag() +0.01;    
  glx_pdg =  glx_event->GetPdg();
  glx_test1 = glx_event->GetTest1();
  glx_test2 = glx_event->GetTest2();
  glx_theta=glx_event->GetMomentum().Theta()*180/TMath::Pi();
  glx_phi=glx_event->GetMomentum().Phi()*180/TMath::Pi();
}

#endif

TSpectrum *glx_spect = new TSpectrum(2);
TF1 *glx_gaust;
TVector3 glx_fit(TH1F *h, Double_t range = 3, Double_t threshold=20, Double_t limit=2, Int_t peakSearch=1,TString opt=""){
  Int_t binmax = h->GetMaximumBin();
  Double_t xmax = h->GetXaxis()->GetBinCenter(binmax);
  glx_gaust = new TF1("glx_gaust","[0]*exp(-0.5*((x-[1])/[2])^2)",xmax-range,xmax+range);
  glx_gaust->SetNpx(500);
  glx_gaust->SetParNames("const","mean","sigma");
  glx_gaust->SetLineColor(2);
  Double_t integral = h->Integral(h->GetXaxis()->FindBin(xmax-range),h->GetXaxis()->FindBin(xmax+range));
  Double_t xxmin, xxmax, sigma1(0), mean1(0), sigma2(0), mean2(0);
  xxmax = xmax;
  xxmin = xxmax;
  Int_t nfound(1);
  if(integral>threshold){

    if(peakSearch == 1){
      glx_gaust->SetParameter(1,xmax);
      glx_gaust->SetParameter(2,0.2);
      glx_gaust->SetParLimits(2,0.005,limit);
      h->Fit("glx_gaust",opt,"MQN",xxmin-range, xxmax+range);
    }

    if(peakSearch == 2){
      nfound = glx_spect->Search(h,4,"",0.1);
      std::cout<<"nfound  "<<nfound <<std::endl;
      if(nfound==1){
	glx_gaust =new TF1("glx_gaust","gaus(0)",xmax-range,xmax+range);
	glx_gaust->SetNpx(500);
	glx_gaust->SetParameter(1,glx_spect->GetPositionX()[0]);
      }else if(nfound==2) {
	Double_t p1 = glx_spect->GetPositionX()[0];
	Double_t p2 = glx_spect->GetPositionX()[1];
	if(p1>p2) {
	  xxmax = p1;
	  xxmin = p2;
	}else {
	  xxmax = p1;
	  xxmin = p2;
	}
	glx_gaust =new TF1("glx_gaust","gaus(0)+gaus(3)",xmax-range,xmax+range);
	glx_gaust->SetNpx(500);
	glx_gaust->SetParameter(0,1000);
	glx_gaust->SetParameter(3,1000);

	glx_gaust->FixParameter(1,xxmin);
	glx_gaust->FixParameter(4,xxmax);
	glx_gaust->SetParameter(2,0.1);
	glx_gaust->SetParameter(5,0.1);
	h->Fit("glx_gaust","","MQN",xxmin-range, xxmax+range);
	glx_gaust->ReleaseParameter(1);
	glx_gaust->ReleaseParameter(4);
      }

      glx_gaust->SetParameter(2,0.2);
      glx_gaust->SetParameter(5,0.2);
    }

    h->Fit("glx_gaust",opt,"MQN",xxmin-range, xxmax+range);
    mean1 = glx_gaust->GetParameter(1);
    sigma1 = glx_gaust->GetParameter(2);
    if(sigma1>10) sigma1=10;

    if(peakSearch == 2){
      mean2 = (nfound==1) ? glx_gaust->GetParameter(1) : glx_gaust->GetParameter(4);
      sigma2 = (nfound==1) ? glx_gaust->GetParameter(2) : glx_gaust->GetParameter(5);
    }
  }
  delete glx_gaust;
  return TVector3(mean1,sigma1,mean2);
}

TString glx_randStr(Int_t len = 10){
  gSystem->Sleep(1500);
  srand (time(NULL));
  TString str = ""; 
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
    str += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  return str;
}

Int_t glx_getColorId(Int_t ind, Int_t style =0){
  Int_t cid = 1;
  if(style==0) {
    cid=ind+1;
    if(cid==5) cid =8;
    if(cid==3) cid =15;
  }
  if(style==1) cid=ind+300;
  return cid;
}

Int_t glx_shiftHist(TH1F *hist, Double_t double_shift){
  Int_t bins=hist->GetXaxis()->GetNbins();
  Double_t xmin=hist->GetXaxis()->GetBinLowEdge(1);
  Double_t xmax=hist->GetXaxis()->GetBinUpEdge(bins);
  double_shift=double_shift*(bins/(xmax-xmin));
  Int_t shift=0;
  if(double_shift<0) shift=TMath::FloorNint(double_shift);
  if(double_shift>0) shift=TMath::CeilNint(double_shift);
  if(shift==0) return 0;
  if(shift>0){
    for(Int_t i=1; i<=bins; i++){
      if(i+shift<=bins) hist->SetBinContent(i,hist->GetBinContent(i+shift));
      if(i+shift>bins) hist->SetBinContent(i,0);
    }
    return 0;
  }
  if(shift<0){
    for(Int_t i=bins; i>0; i--){
      if(i+shift>0) hist->SetBinContent(i,hist->GetBinContent(i+shift));
      if(i+shift<=0) hist->SetBinContent(i,0);
    }    
    return 0;
  }
  return 1;
} 

void glx_writeInfo(TString filename){
  ofstream myfile;
  myfile.open (filename);
  myfile << glx_info+"\n";
  myfile.close();
}

void glx_writeString(TString filename, TString str){
  ofstream myfile;
  myfile.open (filename);
  myfile << str+"\n";
  myfile.close();
}

TString glx_createDir(){
  TString finalpath = glx_savepath;

  if(finalpath =="") return "";
  
  if(glx_savepath == "auto") {
    TString dir = "data";
    gSystem->mkdir(dir);
    TDatime *time = new TDatime();
    TString path(""), stime = Form("%d.%d.%d", time->GetDay(),time->GetMonth(),time->GetYear()); 
    gSystem->mkdir(dir+"/"+stime);
    for(Int_t i=0; i<1000; i++){
      path = stime+"/"+Form("arid-%d",i);
      if(gSystem->mkdir(dir+"/"+path)==0) break;
    }
    gSystem->Unlink(dir+"/last");
    gSystem->Symlink(path, dir+"/last");
    finalpath = dir+"/"+path;
  }else{
    gSystem->mkdir(glx_savepath,kTRUE);
  }
  glx_writeInfo(finalpath+"/readme");
  return finalpath;
}

void glx_addInfo(TString str){
  glx_info += str+"\n";
}

void glx_save(TPad *c= NULL,TString path="", TString name="", Int_t what=0, Int_t style=0){
  if(c && path != "") {
    bool bstate = gROOT->IsBatch();
    gROOT->SetBatch(1);
    Int_t w = 800, h = 400;
    if(style != -1){
      if(style == 1) {w = 800; h = 500;}
      if(style == 2) {w = 800; h = 600;}
      if(style == 3) {w = 800; h = 400;}
      if(style == 5) {w = 800; h = 900;} 
      if(style == 0){ 
	w = ((TCanvas*)c)->GetWindowWidth();
	h = ((TCanvas*)c)->GetWindowHeight();
      }

      TCanvas *cc;
      if(TString(c->GetName()).Contains("cdigi") || TString(c->GetName()).Contains("hp_")) cc = (TCanvas*) c;
      else {
	cc = new TCanvas(TString(c->GetName())+"exp","cExport",0,0,w,h);
	cc = (TCanvas*) c->DrawClone();      
	cc->SetCanvasSize(w,h);
	if(fabs(cc->GetBottomMargin()-0.1)<0.001) cc->SetBottomMargin(0.12);
      }

      if(style == 0) {
	if(fabs(cc->GetBottomMargin()-0.1)<0.001) cc->SetBottomMargin(0.12);
	TIter next(cc->GetListOfPrimitives());
	TObject *obj;
	
	while((obj = next())){
	  if(obj->InheritsFrom("TH1")){
	    TH1F *hh = (TH1F*)obj;
	    hh->GetXaxis()->SetTitleSize(0.06);
	    hh->GetYaxis()->SetTitleSize(0.06);

	    hh->GetXaxis()->SetLabelSize(0.05);
	    hh->GetYaxis()->SetLabelSize(0.05);
	    
	    hh->GetXaxis()->SetTitleOffset(0.85);
	    hh->GetYaxis()->SetTitleOffset(0.76);
	  }
	  if(obj->InheritsFrom("TGraph")){
	    TGraph *gg = (TGraph*)obj;
	    gg->GetXaxis()->SetLabelSize(0.05);
	    gg->GetXaxis()->SetTitleSize(0.06);
	    gg->GetXaxis()->SetTitleOffset(0.84);

	    gg->GetYaxis()->SetLabelSize(0.05);
	    gg->GetYaxis()->SetTitleSize(0.06);
	    gg->GetYaxis()->SetTitleOffset(0.7);
	  }
	  if(obj->InheritsFrom("TF1")){
	    TF1 *f = (TF1*)obj;
	    f->SetNpx(500);
	  }
	}
      }
      
      cc->Modified();
      cc->Update();
      
      cc->Print(path+"/"+name+".png");
      if(what==0) cc->Print(path+"/"+name+".pdf");
      if(what==0) cc->Print(path+"/"+name+".eps");
      if(what==0) cc->Print(path+"/"+name+".C");
    }else{
      c->Print(path+"/"+name+".png");
      if(what==0) c->Print(path+"/"+name+".pdf");
      if(what==0) c->Print(path+"/"+name+".eps");
      if(what==0) c->Print(path+"/"+name+".C");
    }
    gROOT->SetBatch(bstate);
  }
}

TString glx_createSubDir(TString dir="dir"){
  gSystem->mkdir(dir);
  return dir;
}

TList *glx_canvasList;
void glx_canvasAdd(TString name="c",Int_t w=800, Int_t h=600){
  if(!glx_canvasList) glx_canvasList = new TList();
  TCanvas *c = new TCanvas(name,name,0,0,w,h); 
  glx_canvasList->Add(c);
}

void glx_canvasAdd(TCanvas *c){
  if(!glx_canvasList) glx_canvasList = new TList();
  glx_canvasList->Add(c);
}

void glx_canvasCd(TString name="c"){
  
}

void glx_canvasDel(TString name="c"){
  TIter next(glx_canvasList);
  TCanvas *c=0;
  while(((c = (TCanvas*) next()))){
    if(c->GetName()==name) glx_canvasList->Remove(c);
  }
}

void glx_waitPrimitive(TCanvas *c){
  c->Modified(); 
  c->Update(); 
  c->WaitPrimitive();
}

// Ahmed
TCanvas *glx_canvasGet(TString name="c"){
  TIter next(glx_canvasList);
  TCanvas *c=0;
  while((c = (TCanvas*) next())){
    if(c->GetName()==name || name=="*") break;
  }
  return c;
}

void glx_waitPrimitive(TString name, TString prim=""){
  TIter next(glx_canvasList);
  TCanvas *c=0;
  while((c = (TCanvas*) next())){
    if(TString(c->GetName())==name){
      c->Modified();
      c->Update();
      c->WaitPrimitive(prim);
    }
  }
}

// style = 0 - for web blog
// style = 1 - for talk 
// what = 0 - save in png, pdf, root formats
// what = 1 - save in png format
void glx_canvasSave(Int_t what=0, Int_t style=0){
  TIter next(glx_canvasList);
  TCanvas *c=0;
  TString path = glx_createDir();
  while((c = (TCanvas*) next())){
    glx_save(c, path, c->GetName(), what,style);
    glx_canvasList->Remove(c);
  }
}  

void glx_normalize(TH1F* hists[],Int_t size){
  Double_t max = 0;
  Double_t min = 0;
  for(Int_t i=0; i<size; i++){
    Double_t tmax =  hists[i]->GetBinContent(hists[i]->GetMaximumBin());
    Double_t tmin = hists[i]->GetMinimum();
    if(tmax>max) max = tmax;
    if(tmin<min) min = tmin;
  }
  max += 0.05*max;
  for(Int_t i=0; i<size; i++){
    hists[i]->GetYaxis()->SetRangeUser(min,max);
  }
}

int glx_findPdgId(int pdg){
  int pdgId=0; // electron by default 
  if(fabs(pdg) == 13) pdgId=1;
  if(fabs(pdg) == 211) pdgId=2;
  if(fabs(pdg) == 321) pdgId=3;
  if(fabs(pdg) == 2212) pdgId=4;
  return pdgId;
}

void glx_normalize(TH1F* h1,TH1F* h2){
  Double_t max = (h1->GetMaximum()>h2->GetMaximum())? h1->GetMaximum() : h2->GetMaximum();
  max += max*0.1;
  h1->GetYaxis()->SetRangeUser(0,max);
  h2->GetYaxis()->SetRangeUser(0,max);
}
