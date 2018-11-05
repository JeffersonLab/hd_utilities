#define glx__sim
#include "TFile.h"
#include "TTree.h"
#include "../../../../sim-recon/master/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"

// plots 1d histogram with N pho / track as a function of the theta angle
// x range of interest is [-100, 100] cm.
void drawNpho(TString infile="data/etaprime2300_all.root"){

  const Int_t nbins = 50;

  TH1F *hb[nbins];
  
  for(Int_t i=0; i<nbins; i++){
    hb[i] = new TH1F(Form("hb_%d", i), Form("hb_%d", i), 100, 0., 200.);
  }

  TH2F* hbary = new TH2F("hbary", "bar #; y [cm];", 49, -0.5, 48.5, 50, -100., 100.);
  TH1F* hNpho = new TH1F("hNpho","; x [cm]", nbins, -100.,100.);
  Double_t bin = 200./nbins;
  
  if(!glx_init(infile,1,"data/drawNpho")) return;
  
  DrcHit hit;
  for (Int_t ievent=0; ievent<glx_ch->GetEntries(); ievent++){
    glx_nextEvent(ievent,1);
    cout<<"parent = "<<glx_event->GetParent()<<endl;
    if(glx_event->GetParent() > 0) continue;

    Double_t x = glx_event->GetPosition().X();
    Double_t y = glx_event->GetPosition().Y();
    cout<<"x = "<<x<<", y = "<<y<<endl;
    if(x < 110. && y < 110.){
      Int_t bar = glx_event->GetId();
      Int_t N = glx_event->GetHitSize();
      Int_t binnum = (Int_t)((x+100.)/bin) + 1;
      cout<<ievent<<" , x = "<<x<<", y = "<<y<<", N = "<<N<<", bin = "<<binnum<<", bar - "<<bar<<endl;
      if(/*(bar == 13 || bar == 12 || bar == 25 || bar == 26) &&*/ (binnum <= 100 && binnum > 0)) { // take only 4 middle bars
        hb[binnum]->Fill(N);
      }
      hbary->Fill(bar, y);
      
      /*  for(Int_t h=0; h<glx_event->GetHitSize(); h++){
	  hit = glx_event->GetHit(h);
	  Int_t pmt = hit.GetPmtId();
	  Int_t pix = hit.GetPixelId();
	  TVector3 gpos = hit.GetPosition();
	  Double_t time = hit.GetLeadTime();
	  if(pmt<102) glx_hdigi[pmt]->Fill(pix%8, 7-pix/8);
	  }*/
    }
  }

  for(Int_t i=0; i<nbins; i++){
    if(hb[i]->GetEntries() > 0.){
      hNpho->SetBinContent(i+1, hb[i]->GetMaximum());
    }
  }
  
  gStyle->SetOptStat(0);
  TCanvas* c0 = new TCanvas("c0","c0", 500,500);
  hbary->Draw("colz");

  TCanvas* c = new TCanvas("c","c",500,500);
  hNpho->Draw(); 
  /*
  TCanvas* ch = new TCanvas("ch","ch",500,500);
  for(Int_t j=0; j<nbins; j++){
    hb[j]->Draw();
     ch->WaitPrimitive();    
     }*/
}
