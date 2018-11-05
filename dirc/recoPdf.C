#define glx__sim
#include "../../../../sim-recon/master/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "glxtools.C"
#include <TVirtualFitter.h>
#include <TKey.h>

TLine *gLine = new TLine(0,0,0,1000);

//void recoPdf(TString path="pdfs.root", TString data="pikadata.root",  Double_t sigma=200,Bool_t debug=false, Double_t r1=0, Double_t r2=0, Int_t nforpdf=0){
//void recoPdf(TString path="pdfs_atbar.root", TString data="pikadata_atbar.root",  Double_t sigma=200,Bool_t debug=false, Double_t r1=0, Double_t r2=0, Int_t nforpdf=0){
  void recoPdf(TString path="pdfs_atbar4.root", TString data="pikadata4.root",  Double_t sigma=200,Bool_t debug=false, Double_t r1=0, Double_t r2=0, Int_t nforpdf=0){
//  void recoPdf(TString path="pdfs_atbar2.root", TString data="pikadata_atbar2.root",  Double_t sigma=200,Bool_t debug=true, Double_t r1=0, Double_t r2=0, Int_t nforpdf=0){
// void recoPdf(TString path="pdfs_atbar2.root", TString data="pikadata2Gev.root",  Double_t sigma=200,Bool_t debug=false, Double_t r1=0, Double_t r2=0, Int_t nforpdf=0){
TCanvas *cc;
  if(debug) cc = new TCanvas("cc","cc",800,400);
  
  TH1F *hpdff[glx_npixtot],*hpdfs[glx_npixtot], *hl[5],*hnph[5],*hll[5];
  TGraph *gpdff[glx_npixtot],*gpdfs[glx_npixtot];
  for(Int_t i=0; i<5; i++){
    hl[i] = new TH1F(Form("hl_%d",i),"pdf; LE time [ns]; entries [#]", 1000,0,50);
    hnph[i] = new TH1F(Form("hnph_%d",i),"; detected photons [#]; entries [#]", 160,0,160);
    hll[i] = new TH1F(Form("hll_%d",i),"hll; ln L(p) - ln L(#pi); entries [#]",200,-80,80); //120,-60,60
  }
  TH1F *hnphf =  new TH1F("hnphf","hnphf",200,0,200);
  TH1F *hnphs =  new TH1F("hnphs","hnphs",200,0,200);
  
  TF1 *pdff[glx_npixtot],*pdfs[glx_npixtot];
  TFile f(path);

  TRandom rand;
  
  std::cout<<"total number of pixels = "<<glx_npixtot<<std::endl;  
  for(Int_t i=0; i<glx_npixtot; i++){
    hpdff[i] = (TH1F*)f.Get(Form("h_%d_pion",i));
    hpdfs[i] = (TH1F*)f.Get(Form("h_%d_kaon",i));
    //    if(!hpdfs[i] || !hpdff[i]) continue;
    
    hpdff[i]->Rebin(3);
    hpdfs[i]->Rebin(3);
    //    std::cout<<"Nen = "<<hpdfs[i]->GetEntries()<<std::endl;
    hpdff[i]->SetLineColor(2);
    hpdfs[i]->SetLineColor(4);

    gpdff[i] = new TGraph(hpdff[i]);
    gpdfs[i] = new TGraph(hpdfs[i]);

  }
 
  TF1 *F1 = new TF1("gaus0","[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2])",0,150);
  TF1 *F2 = new TF1("gaus1","[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2])",0,150);
  F1->SetParameter(0,1);
  F2->SetParameter(0,1);
  F1->SetParameter(1,63);
  F2->SetParameter(1,50);
  F1->SetParameter(2,11);
  F2->SetParameter(2,9);
  
  TFile f1(data);
  if(!glx_initc(data,1,"data/dataForPdf")) return;
  Double_t theta(0);
  TVirtualFitter *fitter;
  Double_t nph,time, time0;//,timeres(-1);
  DrcHit fHit;
  Int_t totalf(0),totals(0),mcp,pix,ch;
  std::cout<<"entries = "<<glx_ch->GetEntries()<<std::endl;
  Int_t firstCount = 0;
  
  for (Int_t ievent=0; ievent<glx_ch->GetEntries(); ievent++){
    glx_ch->GetEntry(ievent);
    for(Int_t t=0; t < glx_events->GetEntriesFast(); t++){
      glx_nextEventc(ievent, t, 100);
      Double_t aminf,amins, sum(0),sumf(0),sums(0);
      Int_t nGoodHits(0), nHits =glx_event->GetHitSize();
      if (fabs(glx_event->GetPdg()) != 321 && fabs(glx_event->GetPdg()) != 211) continue;
      Int_t pid = glx_findPdgId(glx_event->GetPdg());
      time0 = glx_event->GetTime();
      //if(debug)std::cout<<"===================== event === "<< ievent <<", pid - "<<pid<<std::endl;

      if(nHits < 40.) continue;
	
      Int_t mult[glx_npixtot];
      memset(mult, 0, sizeof(mult));
      for(Int_t i=0; i<nHits; i++){
	fHit = glx_event->GetHit(i);
	mcp = fHit.GetPmtId();
	pix=fHit.GetPixelId();
	ch = glx_getChNum(mcp, pix);

	//	if(ch != 2304) continue; /// !!!!!!!!
	//std::cout<<"===================== event === "<< ievent <<", pid - "<<pid<<std::endl;	
	time = fHit.GetLeadTime() - time0;
	time+= rand.Gaus(0,0.3);
	std::cout<<"t = "<<time<<", ch = "<<ch<<std::endl;
	glx_hdigi[mcp]->Fill(pix%8, pix/8);
	
	nGoodHits++;

	//       	if(!gpdff[ch] || !gpdfs[ch]) continue;
	aminf = gpdff[ch]->Eval(time);
	amins = gpdfs[ch]->Eval(time);
	std::cout<<"aminf = "<<aminf<<", amins = "<<amins<<std::endl;
	if(amins < 1e-20 || aminf < 1e-20) continue;
        if(debug){
	  firstCount++;
	  TString x=(aminf>amins)? " <====== KAON" : "";
          std::cout<<Form("f %1.6f s %1.6f mcp %d pix %d   pid %d",aminf,amins,mcp,pix,pid)<<"  "<<x <<std::endl;
	  cc->cd();
	  //glx_axisTime800x500(hpdff[ch]);
	  //glx_axisTime800x500(hpdfs[ch]);
	  std::cout<<"hist 1 "<<hpdff[ch]<<", hist2 "<<hpdfs[ch]<<std::endl;
	  if(hpdff[ch] && hpdfs[ch]) std::cout<<"exist"<<std::endl;
	  if(!hpdff[ch] || !hpdfs[ch]) std::cout<<"NOOOOO"<<std::endl;
       	  if(firstCount == 1) glx_normalize(hpdff[ch],hpdfs[ch]); //!!!!
	  hpdff[ch]->SetLineColor(2);
	  hpdfs[ch]->SetLineColor(4);
	  hpdff[ch]->Draw();
	  hpdff[ch]->SetTitle(Form("mcp=%d  pix=%d",mcp,pix));
	  hpdff[ch]->GetXaxis()->SetTitle("LE time [ns]");
	  hpdff[ch]->GetYaxis()->SetTitle("PDF value");
	  //hpdff[ch]->GetXaxis()->SetRangeUser(0,40);
	  hpdfs[ch]->Draw("same");
	  gpdff[ch]->Draw("PL same");
	  gpdfs[ch]->Draw("PL same");
	  cc->Update();
	  gLine->SetLineWidth(2);
	  gLine->SetX1(time);
	  gLine->SetX2(time);
	  // gLine->SetY1(cc->GetUymin());
	  //gLine->SetY2(cc->GetUymax());
	  hpdff[ch]->GetXaxis()->SetRangeUser(20.,60.);
	  gLine->Draw();
	  cc->Update();
	  cc->WaitPrimitive();
	}
	
	Double_t noise = 1e-6; //1e-7; // nHits //1e-5
	
	sumf+=TMath::Log((aminf+noise));
	sums+=TMath::Log((amins+noise));
	
	// Double_t res;
	// if(aminf>amins){
	// res=100*(aminf-amins)/amins;
	// sumf+=TMath::Log((res+noise));
	// }else{
	// res=100*(amins-aminf)/amins;
	// sums+=TMath::Log((res+noise));
	// }
	hl[pid]->Fill(time);
	
      }
      //      cout<<"n good hits = "<<nGoodHits<<endl; 
      if(nGoodHits<1) continue; // !!!!!!! was 5
      hnph[pid]->Fill(nGoodHits);
      
      sum = sumf-sums;      
      hll[pid]->Fill(sum);
      std::cout<<"sumf "<<sumf<<", sums = "<<sums<<". sum = "<<sum<<std::endl;
    }
  }
  
  gStyle->SetOptStat(0);
  //gStyle->SetOptTitle(0);

  //  glx_drawDigi("m,p,v\n",2017,1.3,0);
  //  glx_drawDigi("m,p,v\n",2017);
  //glx_canvasAdd(glx_cdigi);
  
  glx_canvasAdd("ll",800,400);

  glx_normalize(hll[3],hll[2]);
  // hll[3]->GetYaxis()->SetNdivisions(9,5,0);
  
  TCanvas* c1 = new TCanvas("c1", "2", 500, 500);
  hll[2]->Draw();

  TCanvas* c2 = new TCanvas("c2", "3", 500,500);
  hll[3]->Draw();
  
  TF1 *ff;
  Double_t sep(0),esep(0),m1,m2,s1,s2,dm1,dm2,ds1,ds2;
  if(hll[3]->GetEntries()>10 && hll[2]->GetEntries()>10){
    hll[3]->Fit("gaus","Sq","");
    ff = hll[3]->GetFunction("gaus");
    ff->SetLineColor(1);
    m1=ff->GetParameter(1);
    s1=ff->GetParameter(2);
    dm1=ff->GetParError(1);
    ds1=ff->GetParError(2);

    hll[2]->Fit("gaus","Sq");
    ff = hll[2]->GetFunction("gaus");
    ff->SetLineColor(1);
    m2=ff->GetParameter(1);
    s2=ff->GetParameter(2);
    dm2=ff->GetParError(1);
    ds2=ff->GetParError(2);

    sep = (fabs(m1-m2))/(0.5*(s1+s2));

    Double_t e1,e2,e3,e4;
    e1=2/(s1+s2)*dm1;
    e2=2/(s1+s2)*dm2;
    e3=-((2*(m1 + m2))/((s1 + s2)*(s1 + s2)))*ds1;
    e4=-((2*(m1 + m2))/((s1 + s2)*(s1 + s2)))*ds2;
    esep=sqrt(e1*e1+e2*e2+e3*e3+e4*e4);
  }
  
  hll[3]->SetTitle(Form("separation = %1.2f",sep));
  hll[3]->SetLineColor(2);
  hll[3]->Draw();
  hll[2]->SetLineColor(4);
  hll[2]->Draw("same");

  TLegend *leg = new TLegend(0.65,0.65,0.83,0.87);
  leg->SetFillColor(0);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->AddEntry(hll[2],"pions ","lp");
  leg->AddEntry(hll[3],"kaons","lp");
  leg->Draw();
  
  glx_canvasAdd("hnph",800,400);

  glx_normalize(hnph[3],hnph[2]);
  hnph[3]->Fit("gaus");
  ff = hnph[3]->GetFunction("gaus");
  ff->SetLineColor(1);
  nph=ff->GetParameter(1);
  hnph[3]->SetLineColor(2);
  hnph[3]->Draw();
  hnph[2]->SetLineColor(4);
  hnph[2]->Draw("same");

  TLegend *leg1 = new TLegend(0.65,0.65,0.83,0.87);
  leg1->SetFillColor(0);
  leg1->SetFillStyle(0);
  leg1->SetBorderSize(0);
  leg1->SetFillStyle(0);
  leg1->AddEntry(hnph[2],"pions ","lp");
  leg1->AddEntry(hnph[3],"kaons","lp");
  leg1->Draw();

  std::cout<<dm1<<" "<<dm2<<" "<<ds1 <<" "<<ds2<<std::endl;
  std::cout<<path<<" separation "<< sep <<" +/- "<<esep <<std::endl;
  std::cout<<"entries:  pi "<<hll[2]->GetEntries()<<" p "<<hll[3]->GetEntries() <<std::endl;

  TString output=data;
  output.ReplaceAll(".root", "_res.root");
  TFile fc(output,"recreate");
  TTree *tc = new TTree("reco","reco");
  tc->Branch("sep",&sep,"sep/D");
  tc->Branch("esep",&esep,"esep/D");
  tc->Branch("sigma",&sigma,"sigma/D");
  tc->Branch("nph",&nph,"nph/D");
  tc->Branch("r1",&r1,"r1/D");
  tc->Branch("r2",&r2,"r2/D");
  tc->Fill();
  tc->Write();
  fc.Write();
  fc.Close();
  
}
