void viewwalk(int run){

  string location("");
  int NPMTS = 184;
  /*
  if (run>69999){
    location.append("calibration2019fall/");
  } else if ((run<70000) && (run>59999)){
    location.append("calib2019spring/");
    NPMTS = 176;
  }
  */
  string dnam("calibration");
  dnam += to_string(run);
  dnam.append("/");

  string rootfile("walk_results_run");
  rootfile += to_string(run);
  rootfile.append(".root");
  
  string fnam = location+dnam+rootfile;
  TFile *RF = new TFile(fnam.c_str(), "READ");
  
  
  for (int k=0; k<NPMTS; k++){
    string h2dnam("Twalk");
    h2dnam += to_string(k);
    TH2D *h2d = (TH2D*)RF->Get(h2dnam.c_str());
    
    string hfitnam("fit1hist");
    hfitnam += to_string(k);
    TF1 *f1 = (TF1*)RF->Get(hfitnam.c_str());

    h2d->Draw("colz");
    f1->Draw("same");
    gPad->Update();
    getchar();

  }


}
