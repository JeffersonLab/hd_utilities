#define glx__sim
#include "/work/halld2/home/jrsteven/2018-dirc/builds/sim-recon/src/plugins/Analysis/pid_dirc/DrcEvent.h"
#include "/work/halld2/home/jrsteven/2018-dirc/builds/sim-recon/src/plugins/Analysis/lut_dirc/DrcLutNode.h"
#include "glxtools.C"

void reco_lut(TString infile="out.root",TString inlut="lut_all_avr.root",double theta=0, double phi=-90, double mom=4){

  if(!glx_initc(infile,1,"data/reco_lut")) return;
  const int nodes = 15000;
  const int luts = 48;
  
  TFile *fLut = new TFile(inlut);
  TTree *tLut=(TTree *) fLut->Get("lut_dirc") ;
  TClonesArray* cLut[luts];
  for(int l=0; l<luts; l++){
    cLut[l] = new TClonesArray("DrcLutNode");
    tLut->SetBranchAddress(Form("LUT_%d",l),&cLut[l]); 
  }  
  tLut->GetEntry(0);

  DrcLutNode *lutNode[luts][nodes];
  for(int l=0; l<luts; l++){
    for(int i=0; i<nodes; i++) lutNode[l][i] = (DrcLutNode*) cLut[l]->At(i);
  }
  TGaxis::SetMaxDigits(4);
  
  TVector3 fnX1 = TVector3 (1,0,0);   
  TVector3 fnY1 = TVector3( 0,1,0);
  TVector3 fnZ1 = TVector3( 0,0,1);
  double radiatorL = 4*1225;
  double barend =  2940; // 4*1225-1960;

  double minChangle = 0.6;
  double maxChangle = 0.9;
  double sum1,sum2,noise = 0.3;
  double criticalAngle = asin(1.00028/1.47125); // n_quarzt = 1.47125; //(1.47125 <==> 390nm)
  double evtime,luttheta,tangle,lenz;
  int64_t pathid;
  TVector3 momInBar,dir,dird;

  TF1 *fit = new TF1("fgaus","[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2]) +x*[3]+[4]",minChangle,maxChangle);
  TSpectrum *spect = new TSpectrum(10);   
  TH1F *hAngle[5], *hLnDiff[5];
  TF1  *fAngle[5];
  double mAngle[5];
  TH1F *hDiff = new TH1F("hDiff",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
  TH1F *hDiffT = new TH1F("hDiffT",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
  TH1F *hDiffD = new TH1F("hDiffD",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
  TH1F *hDiffR = new TH1F("hDiffR",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
  TH1F *hTime = new TH1F("hTime",";propagation time [ns];entries [#]",   1000,0,200);
  TH1F *hCalc = new TH1F("hCalc",";calculated time [ns];entries [#]",   1000,0,200);
  TH1F *hNph = new TH1F("hNph",";detected photons [#];entries [#]", 150,0,150);
  TH1F *hNphC = new TH1F("hNphC",";detected photons [#];entries [#]", 150,0,150);
  TH2F *hTime_Calc = new TH2F("hTime_Calc",";propagation time [ns]; calculated time [ns]; entries [#]",  1000,0,200, 1000,0,200); 
 
  double momentum=4;
  for(int i=0; i<5; i++){
    hAngle[i]= new TH1F(Form("hAngle_%d",i),  "cherenkov angle;#theta_{C} [rad];entries/N_{max} [#]", 250,0.6,1);
    mAngle[i] = acos(sqrt(momentum * momentum + glx_mass[i]*glx_mass[i])/momentum/1.473);  //1.4738
    fAngle[i] = new TF1(Form("fAngle_%d",i),"[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2])",0.7,0.9);
    fAngle[i]->SetParameter(0,1);        // const
    fAngle[i]->SetParameter(1,mAngle[i]);// mean
    fAngle[i]->SetParameter(2,0.0085);    // sigma
    hAngle[i]->SetMarkerStyle(20);
    hAngle[i]->SetMarkerSize(0.8);
    hLnDiff[i] = new TH1F(Form("hLnDiff_%d",i),";ln L(#pi) - ln L(K);entries [#]",100,-200,200);
  }
  
  hAngle[2]->SetLineColor(4);
  hAngle[3]->SetLineColor(2);
  hAngle[2]->SetMarkerColor(kBlue+1);
  hAngle[3]->SetMarkerColor(kRed+1);
  fAngle[2]->SetLineColor(4);
  fAngle[3]->SetLineColor(2);
  fAngle[2]->SetLineStyle(2);
  fAngle[3]->SetLineStyle(2);

  hLnDiff[2]->SetLineColor(4);
  hLnDiff[3]->SetLineColor(2);

  // cuts
  double cut_tdiffd=2;
  double cut_tdiffr=3;
    
  DrcHit hit;
  for (int e = 0; e < glx_ch->GetEntries(); e++){
    glx_ch->GetEntry(e);
    
    for (int t = 0; t < glx_events->GetEntriesFast(); t++){
      glx_nextEventc(e,t,10);
      momInBar = glx_event->GetMomentum();
      int pdgId = glx_findPdgId(glx_event->GetPdg());
      int bar = glx_event->GetId();

      if(bar>=luts) continue;

      //if(glx_event->GetParent()>0) continue;
      // if(hLnDiff[pdgId]->GetEntries()>200) continue;
      
      sum1=0;
      sum2=0;
      int nph=0;
      int nphc=0;
      //      hNphC->Fill(glx_event->GetHitSize());
      
      for(int h = 0; h < glx_event->GetHitSize(); h++){
    	hit = glx_event->GetHit(h);
    	int pmt = hit.GetPmtId();
    	int pix = hit.GetPixelId();
    	double hitTime = hit.GetLeadTime(); //glx_event->GetTime();
	TVector3 gpos = hit.GetPosition();

	//if(hitTime>48) continue;
	nphc++;
	
       	bool reflected = hitTime>48;
	//bool reflected = hitTime>34;
	
	if(glx_event->GetPosition().Y()<0) lenz = fabs(barend+glx_event->GetPosition().X()*10); //80 
	else lenz =fabs(glx_event->GetPosition().X()*10-barend);
	double rlenz = 2*radiatorL - lenz; // reflected
	double dlenz = lenz; // direct

	if(reflected) lenz = 2*radiatorL - lenz;
	
	int sensorId = 100*pmt + pix;
	bool isGood(false);

	double p1,p2;
	for(int i = 0; i < lutNode[bar][sensorId]->Entries(); i++){
	  dird   = lutNode[bar][sensorId]->GetEntry(i);
	  evtime = lutNode[bar][sensorId]->GetTime(i);
	  pathid = lutNode[bar][sensorId]->GetPathId(i);
	  bool samepath(false);
	  if(fabs(pathid-hit.GetPathId())<0.0001) samepath=true;
	  p1=hit.GetPathId();
	  if(samepath){
	    p2=pathid;	    
	  }
	  //if(!samepath) continue;
	  
	  for(int r=0; r<2; r++){
	    if(!reflected && r==1) continue;
		    
	    if(r) lenz = rlenz;	      
	    else lenz = dlenz;
	    
	    for(int u = 0; u < 4; u++){
	      if(u == 0) dir = dird;
	      if(u == 1) dir.SetXYZ( dird.X(),-dird.Y(),  dird.Z());
	      if(u == 2) dir.SetXYZ( dird.X(), dird.Y(), -dird.Z());
	      if(u == 3) dir.SetXYZ( dird.X(),-dird.Y(), -dird.Z());
	      if(r) dir.SetXYZ( -dir.X(), dir.Y(), dir.Z());	   
	      if(dir.Angle(fnY1) < criticalAngle || dir.Angle(fnZ1) < criticalAngle) continue;

	      luttheta = dir.Angle(TVector3(-1,0,0));
	      if(luttheta > TMath::PiOver2()) luttheta = TMath::Pi()-luttheta;
	      tangle = momInBar.Angle(dir);//-0.002; //correction
	    
	      //double bartime = lenz/cos(luttheta)/222.0; //198 //203.767 for 1.47125
	      double bartime = lenz/cos(luttheta)/208.0; //198 //203.767 for 1.47125
	      //double bartime = lenz/cos(luttheta)/198.;
	      double totalTime = bartime+evtime;
	      hTime->Fill(hitTime);
	      hCalc->Fill(totalTime);
	      hTime_Calc->Fill(hitTime, totalTime);	     
	      //cout<<lenz<<endl;
 
	      if(fabs(tangle-0.5*(mAngle[2]+mAngle[3]))<0.2){
		hDiff->Fill(totalTime-hitTime);
		if(samepath){
		  hDiffT->Fill(totalTime-hitTime);
		  if(r) hDiffR->Fill(totalTime-hitTime);
		  else hDiffD->Fill(totalTime-hitTime);
		}
	      }

	      if(!r && fabs(totalTime-hitTime)>cut_tdiffd) continue;
	      if(r && fabs(totalTime-hitTime) >cut_tdiffr) continue;

	      hAngle[pdgId]->Fill(tangle);
	      if(fabs(tangle-0.5*(mAngle[2]+mAngle[3]))>0.02) continue;
	      isGood=true;	  

	      sum1 += TMath::Log(fAngle[2]->Eval(tangle)+0.0001);
	      sum2 += TMath::Log(fAngle[3]->Eval(tangle)+0.0001);	    
	    }
	  }
	}

	//if(!isGood) std::cout<<"pathid "<<hit.GetPathId() << "  "<<hit.GetNreflections() <<std::endl;
	
	if(isGood) {
	  nph++;
	  if(pmt<108) glx_hdigi[pmt]->Fill(pix%8, 7-pix/8);
	}
      }
      if(nph<5) continue;
      hNph->Fill(nph);
      hNphC->Fill(nphc);
      
      double sum = sum1-sum2;      
      hLnDiff[pdgId]->Fill(sum);      
      
      // TCanvas *c = new TCanvas("c","c",800,500);
      // hAngle[3]->Draw();
      // c->Update();
      // c->WaitPrimitive();
      // hAngle[3]->Reset();
    }
  }

  TString nid=Form("_%2.2f_%2.2f",theta,phi);
  //glx_drawDigi("m,p,v\n",0);
  //glx_cdigi->SetName("hp"+nid);
  //glx_canvasAdd(glx_cdigi);
  glx_canvasAdd("hAngle"+nid,800,400);
  
  hAngle[2]->Scale(1/hAngle[2]->GetMaximum());
  hAngle[3]->Scale(1/hAngle[3]->GetMaximum());
  
  double cherenkovreco[5],spr[5];

  for(auto i=0; i<5; i++){
    if(hAngle[i]->GetEntries()<100) continue;
    
    int nfound = spect->Search(hAngle[i],1,"goff",0.9);
    if(nfound>0) cherenkovreco[i] = spect->GetPositionX()[0];
    else cherenkovreco[i] =  hAngle[i]->GetXaxis()->GetBinCenter(hAngle[i]->GetMaximumBin());
    if(cherenkovreco[i]>0.85) cherenkovreco[i]=0.82;
    
    if(i==2)  fit->SetLineColor(kBlue);
    if(i==3)  fit->SetLineColor(kRed);
    fit->SetParameters(100,cherenkovreco[i],0.010,10);
    fit->SetParNames("p0","#theta_{c}","#sigma_{c}","p3","p4");
    fit->SetParLimits(0,0.1,1E6);
    fit->SetParLimits(1,cherenkovreco[i]-0.04,cherenkovreco[i]+0.04);
    fit->SetParLimits(2,0.005,0.030); // width
    hAngle[i]->Fit("fgaus","I","",cherenkovreco[i]-0.04,cherenkovreco[i]+0.04);
    hAngle[i]->Fit("fgaus","M","",cherenkovreco[i]-0.04,cherenkovreco[i]+0.04);

    cherenkovreco[i] = fit->GetParameter(1);
    spr[i] = fit->GetParameter(2);    
  }
  
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
 
  TF1 *ff;
  double sep=0,esep=0, m1=0,m2=0,s1=0,s2=0; 
  if(hLnDiff[3]->GetEntries()>200){
    hLnDiff[3]->Fit("gaus","S");
    ff = hLnDiff[3]->GetFunction("gaus");
    ff->SetLineColor(1);
    m1=ff->GetParameter(1);
    s1=ff->GetParameter(2);
  }
  if(hLnDiff[2]->GetEntries()>200){
    hLnDiff[2]->Fit("gaus","S");
    ff = hLnDiff[2]->GetFunction("gaus");
    ff->SetLineColor(1);
    m2=ff->GetParameter(1);
    s2=ff->GetParameter(2);
  }
  if(s1>0 && s2>0)
    sep = (fabs(m2-m1))/(0.5*(s1+s2));

  cout<<hAngle[2]->Integral()<<endl;
  hAngle[2]->GetXaxis()->SetRangeUser(0.7,0.9);
  //hAngle[2]->GetYaxis()->SetRangeUser(0,1.2);
  hAngle[2]->Draw();
  hAngle[3]->Draw("same");
  fAngle[3]->Draw("same");
  fAngle[2]->Draw("same");

  //return;

  TLine *line = new TLine(0,0,0,1000);
  line->SetX1(mAngle[3]);
  line->SetX2(mAngle[3]);
  line->SetY1(0);
  line->SetY2(1.2);
  line->SetLineColor(kRed);
  line->Draw();

  TLine *line2 = new TLine(0,0,0,1000);
  line2->SetX1(mAngle[2]);
  line2->SetX2(mAngle[2]);
  line2->SetY1(0);
  line2->SetY2(1.2);
  line2->SetLineColor(kBlue);
  line2->Draw();

  TLine *line3 = new TLine(0,0,0,1000);
  line3->SetLineStyle(2);
  line3->SetX1(0.5*(mAngle[2]+mAngle[3])-0.025);
  line3->SetX2(0.5*(mAngle[2]+mAngle[3])-0.025);
  line3->SetY1(0);
  line3->SetY2(1.2);
  line3->SetLineColor(1);
  line3->Draw();

  TLine *line4 = new TLine(0,0,0,1000);
  line4->SetLineStyle(2);
  line4->SetX1(0.5*(mAngle[2]+mAngle[3])+0.025);
  line4->SetX2(0.5*(mAngle[2]+mAngle[3])+0.025);
  line4->SetY1(0);
  line4->SetY2(1.2);
  line4->SetLineColor(1);
  line4->Draw();


  TLegend *leg = new TLegend(0.1,0.5,0.4,0.85);
  leg->SetFillColor(0);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->AddEntry(hAngle[2],Form("#theta_{c}^{#pi} = %2.4f rad",cherenkovreco[2]),"");
  leg->AddEntry(hAngle[3],Form("#theta_{c}^{K} = %2.4f rad",cherenkovreco[3]),"");
  leg->AddEntry(hAngle[2],Form("#sigma_{c}^{#pi} = %2.1f mrad",spr[2]*1000),"");
  leg->AddEntry(hAngle[3],Form("#sigma_{c}^{K} = %2.1f mrad",spr[3]*1000),"");
  leg->Draw();

  TLegend *lnpa = new TLegend(0.7,0.67,0.9,0.85);
  lnpa->SetFillColor(0);
  lnpa->SetFillStyle(0);
  lnpa->SetBorderSize(0);
  lnpa->SetFillStyle(0);
  lnpa->AddEntry(hAngle[2],"pions","lp");
  lnpa->AddEntry(hAngle[3],"kaons","lp");
  lnpa->Draw();
  
  // fAngle[2]->Draw("same");
  // fAngle[3]->Draw("same");
 
  glx_canvasAdd("hTime"+nid,800,400);
  
  hTime->Draw();
  hCalc->SetLineColor(2);
  hCalc->Draw("same");
  TLegend *leg1 = new TLegend(0.5,0.6,0.85,0.80);
  leg1->SetFillColor(0);
  leg1->SetFillStyle(0);
  leg1->SetBorderSize(0);
  leg1->SetFillStyle(0);
  leg1->AddEntry(hTime,"measured in geant","lp");
  leg1->AddEntry(hCalc,"calculated","lp");
  leg1->Draw();

  TCanvas *cc = new TCanvas("cc", "cc", 600, 400);
  hTime_Calc->Draw("colz");

  glx_canvasAdd("hDiff"+nid,800,400);
  hDiff->Draw();
  hDiffT->SetLineColor(kRed+1);
  hDiffT->Draw("same");
  hDiffD->SetLineColor(kGreen+1);
  hDiffD->Draw("same");
  hDiffR->SetLineColor(kOrange);
  hDiffR->Draw("same");

  double maxTD= hDiffD->GetXaxis()->GetBinCenter(hDiffD->GetMaximumBin());
  double maxTR= hDiffR->GetXaxis()->GetBinCenter(hDiffR->GetMaximumBin());

  double maxTT= hTime->GetXaxis()->GetBinCenter(hTime->GetMaximumBin());
  
  line = new TLine(0,0,0,1000);
  line->SetLineStyle(2);
  line->SetX1(-cut_tdiffd);
  line->SetX2(-cut_tdiffd);
  line->SetY1(0);
  line->SetY2(hDiff->GetMaximum()+0.05*hDiff->GetMaximum());
  line->SetLineColor(1);
  line->Draw();

  line2 = new TLine(0,0,0,1000);
  line2->SetLineStyle(2);
  line2->SetX1(cut_tdiffd);
  line2->SetX2(cut_tdiffd);
  line2->SetY1(0);
  line2->SetY2(hDiff->GetMaximum()+0.05*hDiff->GetMaximum());
  line2->SetLineColor(1);
  line2->Draw();

  TLegend *leg2 = new TLegend(0.6,0.57,0.9,0.85);
  leg2->SetFillColor(0);
  leg2->SetFillStyle(0);
  leg2->SetBorderSize(0);
  leg2->SetFillStyle(0);
  leg2->AddEntry(hDiff,"all ambiguities","lp");
  leg2->AddEntry(hDiffT,"MC path in EV","lp");
  leg2->AddEntry(hDiffD,"MC path in EV for direct photons","lp");
  leg2->AddEntry(hDiffR,"MC path in EV for reflected photons","lp");
  leg2->Draw();

  
  glx_canvasAdd("hLnDiff"+nid,800,400);
  hLnDiff[2]->SetTitle(Form("sep = %2.2f s.d.",sep));
  hLnDiff[2]->Draw();
  hLnDiff[3]->Draw("same");

  TLegend *lnpl = new TLegend(0.7,0.67,0.9,0.85);
  lnpl->SetFillColor(0);
  lnpl->SetFillStyle(0);
  lnpl->SetBorderSize(0);
  lnpl->SetFillStyle(0);
  lnpl->AddEntry(hLnDiff[2],"pions","lp");
  lnpl->AddEntry(hLnDiff[3],"kaons","lp");
  lnpl->Draw();

  glx_canvasAdd("hNph"+nid,800,400);

  double nph = glx_fit(hNph,40,100,40).X();
  //hNph->GetFunction("glx_gaust")->SetLineColor(1);
  hNph->Draw();
  hNphC->SetLineColor(kRed);
  hNphC->Draw("same");

  TLegend *lnph = new TLegend(0.6,0.65,0.9,0.85);
  lnph->SetFillColor(0);
  lnph->SetFillStyle(0);
  lnph->SetBorderSize(0);
  lnph->SetFillStyle(0);
  lnph->AddEntry(hNphC,"simulated","lp");
  lnph->AddEntry(hNph,"reconstructed","lp");
  lnph->Draw();
  
  std::cout<<"separation = "<< sep << "  nph = "<<nph <<std::endl;
  std::cout<<"maxTD "<<maxTD<<"  maxTR "<<maxTR<<std::endl;

  TString rpath = infile;
  rpath.ReplaceAll("out","res");
  rpath=Form("res_%2.2f.root",theta);
  TFile fc(rpath,"recreate");
  TTree *tc = new TTree("reco","reco");
  tc->Branch("theta",&theta,"theta/D");
  tc->Branch("phi",&phi,"prt_phi/D");
  tc->Branch("sep",&sep,"sep/D");
  tc->Branch("esep",&esep,"esep/D");
  tc->Branch("mom",&mom,"prt_mom/D");
  tc->Branch("nph",&nph,"nph/D");
  tc->Branch("spr",&spr[3],"spr/D");
  tc->Branch("maxTD",&maxTD,"maxTD/D");
  tc->Branch("maxTR",&maxTR,"maxTR/D");
  tc->Branch("maxTT",&maxTT,"maxTT/D");
  tc->Fill();
  tc->Write();
  fc.Write();
  fc.Close();

  //glx_canvasSave(0,0);
  
}
