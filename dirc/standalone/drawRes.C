#include "glxtools.C"
#include <algorithm>
#include <iterator>
#include <TGraphAsymmErrors.h>


void drawRes(TString in="data/scan7/res_all.root"){

  glx_savepath = "data/drawRes";
  gStyle->SetOptStat(0);
  gStyle->SetPalette(55);
  gStyle->SetNumberContours(99);
   
  TChain ch("reco"); ch.Add(in);  
  Double_t sep,esep,spr,mom,theta,phi,nph;
  ch.SetBranchAddress("theta",&theta);
  ch.SetBranchAddress("phi",&phi);
  ch.SetBranchAddress("sep",&sep);
  ch.SetBranchAddress("esep",&esep);
  ch.SetBranchAddress("spr",&spr);
  ch.SetBranchAddress("mom",&mom);
  ch.SetBranchAddress("nph",&nph);

  TH2F *hSep_pol = new TH2F("hSep",";#theta [deg];#varphi [deg]",72,0,360,(11.4-1.2)/0.2+1+6,0,11.4);
  TH2F *hSep = new TH2F("hSep",";#theta [deg];#varphi [deg]",(11.4-1.2)/0.2+1,1.2,11.4,36,-170,10);
  
  for(Int_t i=0; i<ch.GetEntries(); i++){
    ch.GetEvent(i);
    hSep->Fill(theta,phi,sep);
    hSep_pol->Fill(180-phi,theta,sep);
  }

  // // interpolate emptiness
  // double pval;
  // for(int i=0; i<hSep_pol->GetNbinsX()*hSep_pol->GetNbinsX(); i++){
  //   double val = hSep_pol->GetBinContent(i);
  //   if(val<0.001 && )  hSep_pol->SetBinContent(i,pval);
  //   else pval=val;
  // }
  
  Int_t colors[]={1,kGreen+1,kRed+2,kRed,4,5,6,7,8,9,10};


  glx_canvasAdd("hSep_pol",800,800);
  gPad->SetTheta(90.);
  gPad->SetPhi(0.);
  hSep_pol->Draw("lego2 polz");

  glx_canvasAdd("hSep",800,500);
  hSep->Draw("colz");
  
  glx_canvasSave(0,0);
}
