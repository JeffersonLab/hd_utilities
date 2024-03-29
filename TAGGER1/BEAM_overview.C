#include <fstream>


using namespace std;
// deltaTall microscope counter time differneces RANDOM trigers
// PStagmEnergyInTme
// PStagmEnergyOutOfTme
// PStaghEnergyInTme
// PStaghEnergyOutOfTme

// PStaghEIT  to look at hod. above and below mic. separately
// PStaghEOOT

void BEAM_overview(int R, int ID) {

  // ID = 1: looking at histograms generated by the BEAM_online plugin
  // ID = 2: looking at histograms generated by the TAGGER1 plugin

  char outfile[128];
  char period[128];
  if ((R>49999) && (R<60000)){
    sprintf(period, "localdir/fall18");
    sprintf(outfile,"accidental_scaling_factors_BEAM_fall18.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_fall18.dat");
    }
  } else if ((R>59999) && (R<70000)){
    sprintf(period, "localdir/spring19");
    sprintf(outfile,"accidental_scaling_factors_BEAM_spring19.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_spring19.dat");
      //sprintf(outfile,"accidental_scaling_factors_TAGG_spring19_2Bunches.dat");
     }
  } else if ((R>69999) && (R<80000)){
    sprintf(period, "localdir/fall19");
    sprintf(outfile,"accidental_scaling_factors_BEAM_fall19.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_fall19.dat");
    }
  } else if ((R>39999) && (R<50000)){
    sprintf(period, "localdir/spring18");
    sprintf(outfile,"accidental_scaling_factors_BEAM_spring18.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_spring18.dat");
    }
  } else if ((R>29999) && (R<40000)){
    sprintf(period, "localdir/fall17");
    sprintf(outfile,"accidental_scaling_factors_BEAM_fall17.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_fall17.dat");
    }
  } else if ((R>19999) && (R<30000)){
    sprintf(period, "localdir/spring17");
    sprintf(outfile,"accidental_scaling_factors_BEAM_spring17.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_spring17.dat");
    }
  } else if ((R>9999) && (R<20000)){
    sprintf(period, "localdir/spring16");
    sprintf(outfile,"accidental_scaling_factors_BEAM_spring16.dat");
    if (ID == 2){
      sprintf(outfile,"accidental_scaling_factors_TAGG_spring16.dat");
    }
  }

  double ScalingFactors[10];
  double BunchScale = 20.;
  char inf[128];
  if (ID == 1){
    sprintf(inf, "%s/hd_root_run%d.root", period, R);
  } else if (ID == 2){
    sprintf(inf, "%s/pstreeresults_run%d.root", period, R);
    BunchScale = 2.;
  }

  // IMPORTANT: THIS VALUE HAS TO MATCH WHAT IS USED IN THE PLUGIN "TAGGER1"
  BunchScale = 8.;

  TFile *RF = new TFile( inf, "READ");
  RF->cd();

  if (ID == 1){
    TDirectory *BEAMdir = (TDirectory*)gDirectory->FindObjectAny("BEAM");
    if(!BEAMdir) return;
    BEAMdir->cd();
  }


  TH2D *deltaTall;
  if (ID == 1) {
    deltaTall = (TH2D*)gDirectory->FindObjectAny("deltaTall");
    if (deltaTall->GetEntries()<10000){
      RF->Close();
      return;
    }
  }

  TH1D *dtp;

  if (ID == 1){
    dtp = deltaTall->ProjectionX("dtp",1,10);
    dtp->SetTitle("Microscope counters Random trigger timing");
    dtp->GetXaxis()->SetTitle("#Deltat [ns]");
  }

  TH1D *PStagmEnergyInTime = (TH1D*)gDirectory->FindObjectAny("PStagmEnergyInTime");
  TH1D *PStagmEnergyOutOfTime = (TH1D*)gDirectory->FindObjectAny("PStagmEnergyOutOfTime");
  
  TH1D *PStaghEnergyInTime = (TH1D*)gDirectory->FindObjectAny("PStaghEnergyInTime");
  TH1D *PStaghEnergyOutOfTime = (TH1D*)gDirectory->FindObjectAny("PStaghEnergyOutOfTime");

  if (PStagmEnergyInTime->GetEntries()<100000){
    RF->Close();
    return;
  }

  TCanvas *cBEAM = new TCanvas("cBEAM","Beam Conditions TAGGER-PS Matches", 900, 600 );

  cBEAM->Divide(1,4);
  gStyle->SetTitleSize(0.1,"t");
  gStyle->SetOptStat(0);

  if (ID == 1){
    cBEAM->cd(1);
    dtp->Draw();
    dtp->GetXaxis()->SetLabelSize(0.1);
    dtp->GetYaxis()->SetLabelSize(0.1);
    dtp->GetXaxis()->SetTitle("#Deltat [ns]");
    dtp->GetXaxis()->SetTitleSize(0.1);
    dtp->GetXaxis()->SetTitleOffset(0.4);
    
    int binl = dtp->FindBin(0.0-2.004);
    int binh = dtp->FindBin(0.0+2.004);
    double Ic = dtp->Integral(binl, binh);
    int binll = dtp->FindBin(0.0-2.004 - 10.*4.008);
    int binhh = dtp->FindBin(0.0+2.004 + 10.*4.008);
    double Il = dtp->Integral(binll, binl-1);
    double Ih = dtp->Integral(binh+1, binhh);
    double BG = (Il+Ih)/2./10.;
    double Rrand = Ic/BG;
    char text0[128];
    sprintf(text0, "Scaling: %5.2f",Rrand);
    TText *t0 = new TText(0.15, 0.7, text0);
    t0->SetTextSize(0.1);
    t0->SetNDC();
    t0->Draw();
    ScalingFactors[0] = Rrand;
    
    double Y[50];
    double X[50];
    int counter = 0;
    for (int k=1;k<15;k++){
      int bin2 = dtp->FindBin(0.0+2.004 + k*4.008);
      int bin1 = dtp->FindBin(0.0+2.004 + (k-1)*4.008);
      Il = dtp->Integral(bin1, bin2);
      Y[counter] = (double)Ic / (double)Il;
      X[counter] =  k*4.008;
      counter++;
    }
    for (int k=1;k<15;k++){
      int bin2 = dtp->FindBin(0.0-2.004 - k*4.008);
      int bin1 = dtp->FindBin(0.0-2.004 - (k-1)*4.008);
      Il = dtp->Integral(bin2, bin1);
      Y[counter] = (double)Ic / (double)Il ;
      X[counter] =  -k*4.008;
      counter++;
    }

    cBEAM->cd(2);
    TGraphErrors *gr = new TGraphErrors(counter, X, Y, NULL, NULL);
    gr->SetTitle("Peak Integrals normalized Icenter/Iside");
    gr->SetMarkerStyle(33);
    gr->SetMarkerColor(4);
    gr->Draw("AP");
    gr->GetXaxis()->SetTitleSize(0.1);
    gr->GetXaxis()->SetTitleOffset(0.4);
    gPad->SetGrid();
    gr->GetYaxis()->SetRangeUser(0.8, 1.2);
    gr->GetXaxis()->SetLabelSize(0.1);
    gr->GetYaxis()->SetLabelSize(0.1);
    gr->GetXaxis()->SetTitle("#Deltat [ns]");
    gPad->Update();
  } else {
    ScalingFactors[0] = 1.;
  }


  cBEAM->cd(3);
  PStagmEnergyOutOfTime->Scale(1./BunchScale);
  PStagmEnergyInTime->Draw();
  PStagmEnergyInTime->GetXaxis()->SetTitleSize(0.1);
  PStagmEnergyInTime->GetXaxis()->SetTitleOffset(0.4);
  PStagmEnergyInTime->GetXaxis()->SetLabelSize(0.1);
  PStagmEnergyInTime->GetYaxis()->SetLabelSize(0.1);
  PStagmEnergyInTime->GetXaxis()->SetTitle("#DeltaE [GeV]");

  double ISignalL = PStagmEnergyInTime->Integral(700, 750);
  double IBackgrL = PStagmEnergyOutOfTime->Integral(700, 750);
  double ScaleL = ISignalL/IBackgrL;
  double ScaleLErr = TMath::Sqrt(ISignalL/IBackgrL/IBackgrL*(1.+ScaleL));
  double ISignalR = PStagmEnergyInTime->Integral(1180, 1400);
  double IBackgrR = PStagmEnergyOutOfTime->Integral(1180, 1400);
  double ScaleR = ISignalR/IBackgrR;
  double ScaleRErr = TMath::Sqrt(ISignalR/IBackgrR/IBackgrR*(1.+ScaleR));
  double ScaleErr = (1./ScaleLErr/ScaleLErr + 1./ScaleRErr/ScaleRErr);
  double Scale = (ScaleL/ScaleLErr/ScaleLErr + ScaleR/ScaleRErr/ScaleRErr)/ScaleErr;
  ScaleErr = TMath::Sqrt(1./ScaleErr);

  TH1D* h1 = (TH1D*)PStagmEnergyOutOfTime->Clone();

  h1->Scale(Scale);
  h1->SetLineColor(2);
  h1->Draw("same");
  gPad->SetLogy(1);
  gPad->SetGrid();
  char text1[128];
  sprintf(text1, "Scaling: %5.2f",Scale);
  TText *t1 = new TText(0.12, 0.7, text1);
  t1->SetTextSize(0.1);
  t1->SetNDC();
  t1->Draw();
  ScalingFactors[1]=Scale;

  cBEAM->cd(4);
  PStaghEnergyOutOfTime->Scale(1./BunchScale);
  PStaghEnergyInTime->Draw();
  PStaghEnergyInTime->GetXaxis()->SetTitleSize(0.1);
  PStaghEnergyInTime->GetXaxis()->SetTitleOffset(0.4);
  PStaghEnergyInTime->GetXaxis()->SetLabelSize(0.1);
  PStaghEnergyInTime->GetYaxis()->SetLabelSize(0.1);
  PStaghEnergyInTime->GetXaxis()->SetTitle("#DeltaE [GeV]");
  PStaghEnergyOutOfTime->SetLineColor(7);
  PStaghEnergyOutOfTime->Draw("same");
  gPad->SetLogy(1);
  gPad->SetGrid();
  gPad->Update();
  //getchar();
  ISignalL = PStaghEnergyInTime->Integral(700, 750);
  IBackgrL = PStaghEnergyOutOfTime->Integral(700, 750);
  ScaleL = ISignalL/IBackgrL;
  ScaleLErr = TMath::Sqrt(ISignalL/IBackgrL/IBackgrL*(1.+ScaleL));
  ISignalR = PStaghEnergyInTime->Integral(1180, 1400);
  IBackgrR = PStaghEnergyOutOfTime->Integral(1180, 1400);
  ScaleR = ISignalR/IBackgrR;
  ScaleRErr = TMath::Sqrt(ISignalR/IBackgrR/IBackgrR*(1.+ScaleR));
  ScaleErr = (1./ScaleLErr/ScaleLErr + 1./ScaleRErr/ScaleRErr);
  Scale = (ScaleL/ScaleLErr/ScaleLErr + ScaleR/ScaleRErr/ScaleRErr)/ScaleErr;
  ScaleErr = TMath::Sqrt(1./ScaleErr);

  ScalingFactors[2] = Scale;

  TH1D* h = (TH1D*)PStaghEnergyOutOfTime->Clone();
  h->Scale(Scale);
  h->SetLineColor(2);
  h->Draw("same");

  sprintf(text1, "Scaling: %5.2f",Scale);
  TText *t2 = new TText(0.12, 0.7, text1);
  t2->SetTextSize(0.1);
  t2->SetNDC();
  t2->Draw();

  if (ID == 2) {
    // do hod. separately for above and below microscope

    
    TH2D *h2dInT = (TH2D*)RF->Get("PStaghEIT");
    TH2D *h2dOuT = (TH2D*)RF->Get("PStaghEOOT");
    
    TH1D *pjInTA = (TH1D*)h2dInT->ProjectionX("pjInTA", 1, 135); //above mic
    TH1D *pjInTB = (TH1D*)h2dInT->ProjectionX("pjInTB", 150, 275); //below mic
    
    TH1D *pjOuTA = (TH1D*)h2dOuT->ProjectionX("pjOuTA", 1, 135); //above mic
    TH1D *pjOuTB = (TH1D*)h2dOuT->ProjectionX("pjOuTB", 150, 275); //below mic
    
    pjOuTA->Scale(1./BunchScale);
    pjOuTB->Scale(1./BunchScale);

    ISignalL = pjInTA->Integral(700, 750);
    IBackgrL = pjOuTA->Integral(700, 750);
    ScaleL = ISignalL/IBackgrL;
    ScaleLErr = TMath::Sqrt(ISignalL/IBackgrL/IBackgrL*(1.+ScaleL));
    ISignalR = pjInTA->Integral(1180, 1400);
    IBackgrR = pjOuTA->Integral(1180, 1400);
    ScaleR = ISignalR/IBackgrR;
    ScaleRErr = TMath::Sqrt(ISignalR/IBackgrR/IBackgrR*(1.+ScaleR));
    ScaleErr = (1./ScaleLErr/ScaleLErr + 1./ScaleRErr/ScaleRErr);
    Scale = (ScaleL/ScaleLErr/ScaleLErr + ScaleR/ScaleRErr/ScaleRErr)/ScaleErr;
    ScaleErr = TMath::Sqrt(1./ScaleErr);
    
    cout<<endl<<endl<<" LEFT: "<<ScaleL<<"       SCALER: "<<ScaleR<<"     S="<<Scale  <<endl<<endl;

    ScalingFactors[3] = Scale;
    pjOuTA->Scale(Scale);
    pjOuTA->SetLineColor(2);
    cBEAM->cd(1);
    pjInTA->SetTitle("Hodoscope Counters above microscope");
    pjInTA->Draw();
    pjOuTA->Draw("same");
    gPad->SetGrid();
    gPad->SetLogy(1);
    sprintf(text1, "Scaling: %5.2f",Scale);
    TText *t5 = new TText(0.12, 0.7, text1);
    t5->SetTextSize(0.1);
    t5->SetNDC();
    t5->Draw();

    ISignalL = pjInTB->Integral(700, 750);
    IBackgrL = pjOuTB->Integral(700, 750);
    ScaleL = ISignalL/IBackgrL;
    ScaleLErr = TMath::Sqrt(ISignalL/IBackgrL/IBackgrL*(1.+ScaleL));
    ISignalR = pjInTB->Integral(1180, 1400);
    IBackgrR = pjOuTB->Integral(1180, 1400);
    ScaleR = ISignalR/IBackgrR;
    ScaleRErr = TMath::Sqrt(ISignalR/IBackgrR/IBackgrR*(1.+ScaleR));
    ScaleErr = (1./ScaleLErr/ScaleLErr + 1./ScaleRErr/ScaleRErr);
    Scale = (ScaleL/ScaleLErr/ScaleLErr + ScaleR/ScaleRErr/ScaleRErr)/ScaleErr;
    ScaleErr = TMath::Sqrt(1./ScaleErr);
    
    cout<<endl<<endl<<" LEFT: "<<ScaleL<<"       SCALER: "<<ScaleR<<"     S="<<Scale  <<endl<<endl;

    ScalingFactors[4] = Scale;
    pjOuTB->Scale(Scale);
    pjOuTB->SetLineColor(2);
    cBEAM->cd(2);
    pjInTB->SetTitle("Hodoscope Counters below microscope");
    pjInTB->Draw();
    pjOuTB->Draw("same");
    gPad->SetGrid();
    gPad->SetLogy(1);
    sprintf(text1, "Scaling: %5.2f",Scale);
    TText *t6 = new TText(0.12, 0.7, text1);
    t6->SetTextSize(0.1);
    t6->SetNDC();
    t6->Draw();

  }



  char of[128];
  if (ID == 1){
    sprintf(of,"plots/BEAM_online_BEAM_run%d.pdf",R);
    cBEAM->SaveAs(of);
    sprintf(of,"plots/BEAM_online_BEAM_run%d.gif",R);
    cBEAM->SaveAs(of);
  } else if (ID==2){
    sprintf(of,"plots/BEAM_online_TAGG_run%d.pdf",R);
    cBEAM->SaveAs(of);
    sprintf(of,"plots/BEAM_online_TAGG_run%d.gif",R);
    cBEAM->SaveAs(of);
  }


  if (ID == 2) {
    
    // look at outoftimehodoscope and microscope hits 
    
    
    TH1D *h1H = (TH1D*)RF->Get("outoftimeH");
    TH1D *h1M = (TH1D*)RF->Get("outoftimeM");
    if (!h1H){
      return;
    }
    if (!h1M){
      return;
    }

    h1H->Fit("gaus","","R", -1., 1.);
    TF1 *f1 = h1H->GetFunction("gaus");
    double max = f1->GetParameter(1);
    double sig = f1->GetParameter(2);
    h1H->Fit("gaus","","R", max-2.*sig, max+2*sig);
    f1 = h1H->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig = f1->GetParameter(2);
    double offseth = max;

    h1M->Fit("gaus","","R", -1., 1.);
    f1 = h1M->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig = f1->GetParameter(2);
    h1M->Fit("gaus","","R", max-2.*sig, max+2*sig);
    f1 = h1M->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig = f1->GetParameter(2);
    double offsetm = max;

    double np = 3.5;
    double npt = 6.;

    double WIDTH = 4.008;
    double ISigM = h1M->Integral(h1M->GetXaxis()->FindBin(-WIDTH/2.+offsetm), 
				 h1M->GetXaxis()->FindBin(WIDTH/2.+offsetm));
    double ISigH = h1H->Integral(h1H->GetXaxis()->FindBin(-WIDTH/2.+offseth), 
				 h1H->GetXaxis()->FindBin(WIDTH/2.+offseth));
    
    double IBgM = h1M->Integral(h1M->GetXaxis()->FindBin(-(WIDTH*np)+offsetm), 
				h1M->GetXaxis()->FindBin(-WIDTH/2.+offsetm))+
      h1M->Integral(h1M->GetXaxis()->FindBin(WIDTH/2.+offsetm), 
		    h1M->GetXaxis()->FindBin(WIDTH*np)+offsetm);
    double IBgH = h1H->Integral(h1H->GetXaxis()->FindBin(-(WIDTH*np)+offseth), 
				h1H->GetXaxis()->FindBin(-WIDTH/2.)+offseth)+
      h1H->Integral(h1H->GetXaxis()->FindBin(WIDTH/2.+offseth), 
		    h1H->GetXaxis()->FindBin(WIDTH*np)+offseth);
    

    IBgM /= npt;
    IBgH /= npt;
    
    double ScaleM = ISigM/IBgM;
    double ScaleH = ISigH/IBgH;
    
    ScalingFactors[5] = ScaleM;
    ScalingFactors[6] = ScaleH;
    
  }
  
  // new approach
  if ((ID == 2) && (0)) {
    
    // look at outoftimehodoscope and microscope hits     
    
    TH1D *h1H = (TH1D*)RF->Get("accidentalfenceH");
    TH1D *h1M = (TH1D*)RF->Get("accidentalfenceM");

    h1H->Fit("gaus","","R", -1., 1.);
    TF1 *f1 = h1H->GetFunction("gaus");
    double max = f1->GetParameter(1);
    double sig = f1->GetParameter(2);
    h1H->Fit("gaus","","R", max-2.*sig, max+2*sig);
    f1 = h1H->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig = f1->GetParameter(2);
    double offseth = max;

    h1M->Fit("gaus","","R", -1., 1.);
    f1 = h1M->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig = f1->GetParameter(2);
    h1M->Fit("gaus","","R", max-2.*sig, max+2*sig);
    f1 = h1M->GetFunction("gaus");
    max = f1->GetParameter(1);
    sig = f1->GetParameter(2);
    double offsetm = max;

    
    double WIDTH = 4.008;
    double np = 6.5;
    double npt = 12.;


    double ISigM = h1M->Integral(h1M->GetXaxis()->FindBin(-WIDTH/2.+offsetm), 
				 h1M->GetXaxis()->FindBin(WIDTH/2.+offsetm));
    double ISigH = h1H->Integral(h1H->GetXaxis()->FindBin(-WIDTH/2.+offseth), 
				 h1H->GetXaxis()->FindBin(WIDTH/2.+offseth));
    
    double IBgM = h1M->Integral(h1M->GetXaxis()->FindBin(-(WIDTH*np)+offsetm), 
				h1M->GetXaxis()->FindBin(-WIDTH/2.+offsetm))+
      h1M->Integral(h1M->GetXaxis()->FindBin(WIDTH/2.+offsetm), 
		    h1M->GetXaxis()->FindBin(WIDTH*np+offsetm));
    double IBgH = h1H->Integral(h1H->GetXaxis()->FindBin(-(WIDTH*np)+offseth), 
				h1H->GetXaxis()->FindBin(-WIDTH/2.+offseth))+
      h1H->Integral(h1H->GetXaxis()->FindBin(WIDTH/2.+offseth), 
		    h1H->GetXaxis()->FindBin(WIDTH*np+offseth));
    

    IBgM /= npt;
    IBgH /= npt;
    
    double ScaleM = ISigM/IBgM;
    double ScaleH = ISigH/IBgH;
    
    ScalingFactors[7] = ScaleM;
    ScalingFactors[8] = ScaleH;
    
  }
  
  
  
  ofstream OUTF;
  if (ID == 1){
    OUTF.open( outfile, std::ofstream::out | std::ofstream::app);
  } else {
    OUTF.open(outfile, std::ofstream::out | std::ofstream::app);
  }
  OUTF<<R<<"  " << ScalingFactors[0]<<"   " <<ScalingFactors[1]<<"  "<<ScalingFactors[2];
  if (ID == 1){
    OUTF<<endl;
  } else {
    OUTF<<"      "<<ScalingFactors[3]<<"    "<<ScalingFactors[4];
    OUTF<<"      "<<ScalingFactors[5]<<"    "<<ScalingFactors[6];
    OUTF<<"      "<<ScalingFactors[7]<<"    "<<ScalingFactors[8]<<endl;
  }

  OUTF.close();
}
