// pupose: find ADC peak position of mips in ADC histogram

int AMP = 1;

int DEBUG = 0;

void findp(int R, int amp){

  AMP = amp;

  char fnam[128];
  sprintf(fnam,"calibration%d/adchists_run%d.root",R,R);

  TFile *RF = new TFile(fnam,"READ");
  
  double FitPar[176][4];

  double Range[2] = {2000.,10000.};
  double Range1[2] = {5000.,15000.};
  char hn[128] = "ADCHists";
  if (AMP){
    Range[0] = 300.;
    Range[1] = 1200.;
    Range1[0] = 700.;
    Range1[1] = 1800.;
    sprintf(hn,"ADCPeakHists");
  }

  char hnam[128];
  for (int k=0;k<176;k++){
    sprintf(hnam,"%s%d",hn,k);
    TH1D *h = (TH1D*)RF->Get(hnam); 
    if (h==NULL){
      cout<<"No amplitude histogram found! quit..."<<endl;
      return;
    }
    for (int n=0;n<4;n++){
      FitPar[k][n]  = 0.;
    }
    if (h->GetEntries()>10000){
      h->GetXaxis()->SetRangeUser(Range[0],Range[1]);
      if ( (k==85) || (k==86) || (k==87) || (k==88)){
	h->GetXaxis()->SetRangeUser(Range1[0],Range1[1]);
      }
      int MaxBin = h->GetMaximumBin();
      double max = h->GetBinCenter(MaxBin);
      double hili = max + 10.*h->GetBinWidth(MaxBin);
      double loli = max - 6.*h->GetBinWidth(MaxBin);
      h->Fit("landau","","R",loli,hili);
      TF1 *f1  = h->GetFunction("landau");
      double sig = f1->GetParameter(2);
      hili = max + 3*sig;
      loli = max - 1.5*sig;
      h->Fit("landau","","R",loli,hili);

      if (DEBUG>2){
	h->GetXaxis()->SetRangeUser(0.,4096);
	h->Draw();
	h->GetXaxis()->SetTitle("ADC counts");
	h->GetYaxis()->SetTitle("Counts [#]");
	gPad->Update();
	gPad->SaveAs("pmt_response.pdf");
	getchar();
	return;
      }
      f1 = h->GetFunction("landau");
      FitPar[k][0] = f1->GetParameter(1);
      FitPar[k][1] = f1->GetParError(1);
      FitPar[k][2] = f1->GetParameter(2);
      FitPar[k][3] = f1->GetParError(2);
    }
  }
  

  RF->Close();

  sprintf(fnam,"ADC_positions/adc_integral_run%d.dat",R);
  if (AMP) {
    sprintf(fnam,"ADC_positions/adc_amplitudes_run%d.dat",R);
  }
  ofstream OUTF(fnam);
  char str[128];
  for (int k=0;k<176;k++){
    sprintf(str,"%3d  %12.4e  %12.4e  %12.4e  %12.4e",k,
	    FitPar[k][0],FitPar[k][1],FitPar[k][2],FitPar[k][3]);
    OUTF<<str<<endl;
  }
  OUTF.close();

}
