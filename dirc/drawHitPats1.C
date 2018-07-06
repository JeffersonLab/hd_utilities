#define glx__sim
#include "../../../../sim-recon/master/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"

void drawHitPats1(double xmin=-40., double xmax=45., double ymin=-40., double ymax=45.){
  TString infile = "data/kaons/kaons.root";
  if(!glx_initc(infile,1,"data/drawHP")) return;
      
      DrcHit hit;
      TVector3 hpos;
      int Neve = 0;
      if(Neve == 0) Neve = glx_ch->GetEntries();
      for (Int_t e=0; e<Neve; e++){
	glx_ch->GetEntry(e);
	for (Int_t t=0; t<glx_events->GetEntriesFast(); t++){
	  glx_nextEventc(e,t,10);
	  if(glx_event->GetParent()>0) continue;
	  hpos = glx_event->GetPosition();
	  if(hpos.X() < xmin || hpos.X() > xmax || hpos.Y() < ymin || hpos.Y() > ymax) continue;
	  for(Int_t h=0; h<glx_event->GetHitSize(); h++){
	    hit = glx_event->GetHit(h);
	    Int_t pmt = hit.GetPmtId();
	    Int_t pix = hit.GetPixelId();
	    TVector3 gpos = hit.GetPosition();
	    Double_t time = hit.GetLeadTime();
	    if(pmt<108) glx_hdigi[pmt]->Fill(pix%8, 7-pix/8);
	    //if(pmt>=108) glx_hdigi[pmt-108]->Fill(pix%8, 7-pix/8);
	  }
	}
      }

      glx_drawDigi("m,p,v\n",0);
      glx_cdigi->SaveAs(Form("data/drawHitPat/hp_x%.1f_y%.1f.png",xmin + (xmax-xmin)*0.5, ymin + (ymax-ymin)*0.5));

      cout<<"x = "<<xmin + (xmax-xmin)*0.5<<", y = "<<ymin + (ymax-ymin)*0.5<<endl;
      cout<<"xmin = "<<xmin<<", xmax = "<<xmax<<", ymin = "<<ymin<<", ymax = "<<ymax<<endl;
        
}
