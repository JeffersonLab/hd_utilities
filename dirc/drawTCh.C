#define glx__sim
#include "../../../../sim-recon/master/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"

void drawTCh(TString infile="hd_root_particle_gun_031000_001.root"){
  if(!glx_initc(infile,1,"data/drawHP")) return;

  TH2F* tch = new TH2F("tch",";ch num; time [ns]", 6912, 0, 6912, 300, 0., 300.);
  TH2F* tch1 = new TH2F("tch1",";ch num1; time [ns]", 6912, 0, 6912, 300, 0., 300.);
  
  DrcHit hit;
  for (Int_t e=0; e<glx_ch->GetEntries(); e++){
    glx_ch->GetEntry(e);
    for (Int_t t=0; t<glx_events->GetEntriesFast(); t++){
      glx_nextEventc(e,t,10);
      if(e > glx_ch->GetEntries()-2) cout<<"particle is "<<glx_names[glx_findPdgId(glx_event->GetPdg())]<<endl;
      if(glx_event->GetParent()>0) continue;
      for(Int_t h=0; h<glx_event->GetHitSize(); h++){
    	hit = glx_event->GetHit(h);
    	Int_t pmt = hit.GetPmtId();
    	Int_t pix = hit.GetPixelId();
    	TVector3 gpos = hit.GetPosition();
    	Double_t time = hit.GetLeadTime();


	Double_t x_row = (pmt%18) *8 + pix%8;
	Double_t y_row = (pmt/18) *8 + pix/8;
	Double_t ch_g = y_row*144 + x_row;

	//if(pmt/18 != 2) continue;

	tch ->Fill(hit.GetChannel(), time);
	tch1->Fill(ch_g, time);
	//	if(pmt != 89) continue;
       	//if(pix != 5) continue;

	cout<<"pmt = "<<pmt<<", pix = "<<pix<<", xrow = "<<x_row<<", y_row = "<<y_row<<", ch_g = "<<ch_g<<endl;
	
	//cout<<"channel - "<<hit.GetChannel()<<endl;
	if(time < 300){
	  if(pmt<108) glx_hdigi[pmt]->Fill(pix%8, 7-pix/8);
	  //if(pmt>=108) glx_hdigi[pmt-108]->Fill(pix%8, 7-pix/8);
	}
      }
    }
  }
  gStyle->SetOptStat(0);
  glx_canvasAdd("glx_cht");
  tch->Draw("colz");

  glx_canvasAdd("glx_cht1");
  tch1->Draw("colz");

  glx_drawDigi("m,p,v\n",0);
  glx_canvasAdd(glx_cdigi);
  //  glx_canvasSave(1,0);
}
