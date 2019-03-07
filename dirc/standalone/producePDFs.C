#define glx__sim
#include "../../../../sim-recon/master/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"

void producePDFs(TString infile="drc.root",TString outfile="pdfs.root"){
  if(!glx_initc(infile,1,"data/drawHP")) return;

  Int_t nPho[5];
  for(Int_t i=0; i<5; i++){
      nPho[i] = 0;
  }

  // histograms for storing time pdfs for different particle species (5)
  TH1F *htime[5][glx_npix];

  for(Int_t j=0; j<5; j++){
    for(Int_t i=0; i<glx_npix; i++){
      htime[j][i] = new TH1F(Form("time_%d for ",i)+glx_names[j],"pdf; hit time [ns]; entries [#]", 1000, 0., 50.);
      //      cout<<Form("time_%d for ",i) + glx_names[j]<<endl;
    }
  }

  Double_t time;
  Int_t ch, pid, pmt, pix;
  DrcHit hit;
  for (Int_t e=0; e<glx_ch->GetEntries(); e++){
    glx_ch->GetEntry(e);
    for (Int_t t=0; t<glx_events->GetEntriesFast(); t++){
      glx_nextEventc(e,t,10);
      if(glx_event->GetParent()>0) continue;
      pid = glx_findPdgId(glx_event->GetPdg());
      for(Int_t h=0; h<glx_event->GetHitSize(); h++){
    	hit = glx_event->GetHit(h);
    	pmt = hit.GetPmtId();
        pix = hit.GetPixelId();
    	time = hit.GetLeadTime();
	ch = glx_getChNum(pmt, pix);
	nPho[pid]++;
	htime[pid][ch]->Fill(time);
      }
    }
  }

  TFile efile(outfile, "RECREATE");
  for(Int_t i=0; i<5; i++){
    cout<<"Npho in pix = "<<(Double_t)nPho[i]<<endl;;
    for(Int_t j=0; j<glx_npix; j++){
      if(htime[i][j]->GetEntries() > 0){
	htime[i][j]->Scale(1/(Double_t)nPho[i]);
	htime[i][j]->Write();
      }
    }
  }

  efile.Write();
  efile.Close();
}
