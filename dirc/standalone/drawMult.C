#define glx__sim
#include "../../../../sim-recon/master/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"

//void drawMult(TString infile="data/out_kaons_100k.root"){
//void drawMult(TString infile="data/sim_etaprime2300.root"){
void drawMult(TString infile="data/sim_hprime2600_100k_tr_nodc.root"){
//void drawMult(TString infile="data/sim_hprime2600_100k_notr_nodc.root"){

  //void drawMult(TString infile="data/sim_hprime2600.root"){
  //void drawMult(TString infile="data/sim_hprime2600.root"){
  if(!glx_initc(infile,1,"data/drawHP")) return;

  //  gStyle->SetOptStat(0);
  TH2F *hPoint = new TH2F("hPoint",";x [cm]; y [cm]",200,-120,120,200,-120,120);
  TH1F *hMult = new TH1F("hMult",";detected photons [#]; [#]",500,0,500);
  TH1F *hEnergy = new TH1F("hEnergy",";photon energy [GeV]; [#]",100,0,10);
  
  const auto nmax(20);
  double minx=-100, maxx=100;
  TH1F *hMultX[nmax];
  for(auto i=0; i<nmax; i++){
    hMultX[i] = new TH1F(Form("hMultX_%d",i),Form("hMultX_%d;x [cm]; stat [#]",i),300,0,300);
  }
  
  TVector3 hpos,gpos;
  DrcHit hit;
  for (auto e=0; e<glx_ch->GetEntries(); e++){
    glx_ch->GetEntry(e);
    for (auto t=0; t<glx_events->GetEntriesFast(); t++){
      glx_nextEventc(e,t,100);
      if(glx_event->GetParent()>0) continue;
      hpos = glx_event->GetPosition();
      double x(hpos.X()), y(hpos.Y());      
      hPoint->Fill(x, y);
      
      int nhits=glx_event->GetHitSize();
      for(auto h=0; h<nhits; h++){
    	hit = glx_event->GetHit(h);
    	Int_t pmt = hit.GetPmtId();
    	Int_t pix = hit.GetPixelId();
    	gpos = hit.GetPosition();
    	Double_t time = hit.GetLeadTime();
    	if(pmt<108) glx_hdigi[pmt]->Fill(pix%8, 7-pix/8);
    	//if(pmt>=108) glx_hdigi[pmt-108]->Fill(pix%8, 7-pix/8);
	hEnergy->Fill(hit.GetEnergy()*1E9);
      }

      hMult->Fill(nhits);
      if(fabs(fabs(y)-12)<4){
	int xid = nmax*(x-minx)/(maxx - minx);
	if(xid>=0 && xid<nmax) hMultX[xid]->Fill(nhits);
      }
    }
  }

  TGaxis::SetMaxDigits(2);
  glx_drawDigi();
  glx_canvasAdd(glx_cdigi);

  glx_canvasSave(1,0);
  
  TGaxis::SetMaxDigits(4);
  glx_canvasAdd("hPoint",500,500);
  hPoint->Draw("colz");

  glx_canvasAdd("hMult",800,400);
  hMult->Draw();
  
  glx_canvasAdd("hMultX");

  TGraph *gMult = new TGraph();
  for(auto i=0; i<nmax; i++){
    double nph = glx_fit(hMultX[i],40,50,30).X();
    hMultX[i]->Draw();

    double xpos = minx + 0.5*(maxx - minx)/nmax + i*(maxx - minx)/nmax;
    glx_waitPrimitive("hMultX");
    gMult->SetPoint(i,xpos,nph);
  }

  glx_canvasAdd("gMultX",800,400);
  gMult->GetXaxis()->SetRangeUser(-110,110);
  gMult->GetYaxis()->SetRangeUser(0,150);
  gMult->GetXaxis()->SetTitle("x [cm]");
  gMult->GetYaxis()->SetTitle("detected photons [#]");
  gMult->SetMarkerStyle(20);
  gMult->SetMarkerSize(0.8);
  gMult->Draw("APL");

  glx_canvasAdd("hEnergy");
  hEnergy->Draw();
  
  glx_canvasSave(0);
  
}

