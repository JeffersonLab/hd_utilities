#define glx__sim
#include "/work/halld2/home/jrsteven/2018-dirc/builds/sim-recon/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"

void drawHP(TString infile="drc.root"){
  if(!glx_initc(infile,1,"data/drawHP")) return;

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

	//      	if(pmt != 36) continue;
	//if(pix != 0) continue;
	//cout<<"channel - "<<hit.GetChannel()<<endl;
	if(pmt<108) glx_hdigi[pmt]->Fill(pix%8, 7-pix/8);
    	//if(pmt>=108) glx_hdigi[pmt-108]->Fill(pix%8, 7-pix/8);
      }
    }
  }
  glx_drawDigi("m,p,v\n",0);
  glx_canvasAdd(glx_cdigi);
  glx_canvasSave(1,0);

}
