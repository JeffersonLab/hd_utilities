//
// purpose: determine location of baseline peak to generate a input file
//          for ccdb listing the baselines for each PMT of the run being
//          analyzed. The input file is tofdata_runXXXXX.root same as
//          for the calibration but only using the histogram TOFpedestal

int DEBUG = 0;

void baseline(int R){

  string dirloc;
  int NPMTS = 176;
  string f("tofdata_run");
  if (R>69999){
    //string s("/cache/halld/home/zihlmann/TOF_calib/fall19/");
    string s("/work/halld2/home/zihlmann/HallD/work/tofcalib/jobs/Run");
    s += to_string(R)+"/";
    dirloc = s;
    NPMTS = 184;
  } else if (R>56999){
    string s("/cache/halld/home/zihlmann/TOF_calib/spring19/");
    dirloc = s;
  } else if (R>46999){
    string s("/cache/halld/home/zihlmann/TOF_calib/fall18/");
    dirloc = s;
  } else if (R>36999){
    string s("/cache/halld/home/zihlmann/TOF_calib/spring18/");
    dirloc = s;
  } else {
    cout<<"run range invalid! BAIL!"<<endl;
    return;
  }
  
  f += to_string(R)+".root";
  if (R>69999){ // special only for this!!!!!
    string b("hd_root_tofcalib.root");
    f = b;
  }
  string inf = dirloc+f;
  TFile *RF = new TFile(inf.c_str(), "READ");

  TH2D *h2d = (TH2D*)RF->Get("TOFcalib/TOFPedestal");
  if (!h2d){
    cout<<"historgram TOFcalib/TOFPedestal NOT FOUND!"<<endl;
    return;
  }

  double PedPos[200];
  for (int n=1; n<(NPMTS+1); n++){

    PedPos[n-1] = 100.;

    TH1D *h = (TH1D*) h2d->ProjectionY("pj",n,n);
    if (h->GetEntries()<1000){
      continue;
    }
    int maxbin = h->GetMaximumBin();
    int nbins = h->GetXaxis()->GetNbins();

    if ((maxbin>nbins-10)||(maxbin<10)){
      continue;
    }
    double pos,w;
    pos = h->GetBinCenter(maxbin);
    w = h->GetBinWidth(maxbin);
    //cout<<n<<"  " << pos<<"  "<<w<<"   "<<maxbin<<endl;
    h->Fit("gaus","Q","R", pos-5*w, pos+3*w);
    TF1 *f1 = h->GetFunction("gaus");
    pos = f1->GetParameter(1);
    w = f1->GetParameter(2);
    h->Fit("gaus","Q","R", pos-4.*w, pos+2.*w);
    f1 = h->GetFunction("gaus");
    pos = f1->GetParameter(1);
    w = f1->GetParameter(2);
 
    //cout<<"          "<<pos<<" / "<<w<<endl;
   
    PedPos[n-1] = pos/4.;

    if (DEBUG>1){
      h->Draw();
      gPad->SetGrid();
      gPad->Update();
      sleep(1);
    }

  }

  char of[128];
  sprintf(of, "pedestals/pedpos_run%d.dat",R);
  ofstream OUTF;
  OUTF.open(of,std::ofstream::out);
  for (int n=0; n<NPMTS; n++){
    OUTF<<PedPos[n]<<endl;
  }
  OUTF.close();

}

