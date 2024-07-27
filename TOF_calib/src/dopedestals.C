


void dopedestals(int RNUM){
 
  int NPMTS = 176;         // TOF 1 geometry

  if (RNUM>69999){
    NPMTS = 184;          // TOF 2 geometry
  }

  int NC = NPMTS;

  string f = "localdir/tofdata_run"+to_string(RNUM)+".root";
  cout<<"FILE: "<<f.c_str()<<endl;

  TFile *RF = new TFile(f.c_str(), "READ");

  TH2F *h2d = (TH2F*)RF->Get("TOFcalib/TOFPedestal");
  cout<<h2d<<endl;

  double DATAPED[200];
  string of = "calibration"+to_string(RNUM)+"/pedestals_run"+to_string(RNUM)+".DB";
  ofstream OUTF(of.c_str());

  if (h2d){

    for (int k=1; k<NC+1; k++){
      DATAPED[k-1] = 100;
      string pj = "pj"+to_string(k); 
      TH1F *h1d = (TH1F*)h2d->ProjectionY(pj.c_str(), k, k);
      if (h1d->GetEntries()<100.) 
	continue;

      double max = h1d->GetBinCenter(h1d->GetMaximumBin());
      h1d->Fit("gaus", "Q", "R", max-5., max+5.);
      TF1 *f1 = h1d->GetFunction("gaus");
      max = f1->GetParameter(1);
      double sig = f1->GetParameter(2);
      h1d->Fit("gaus", "Q", "R", max-2.*sig, max+2.*sig);
      f1 = h1d->GetFunction("gaus");
      max = f1->GetParameter(1);

      DATAPED[k-1] = max/4.;
      char line[128];
      sprintf(line,"%6.1f\n",DATAPED[k-1]);
      OUTF<<line;
    }
  }
  OUTF.close();
  RF->Close();

  cout<<"Pedestal output file written for the "<<NC<<" PMTs"<<endl;
  cout<<of.c_str()<<endl;
}
