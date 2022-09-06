{
  // Copy a few key CDC-specific histograms from the monitoring plugin output in the current root file into a new smaller file 

  bool amp = 0, dedx = 0, ttod = 0, eff = 0;

  TH1I *a;
  TH2I *an30_100ns;

  TH2I *dedx_p;
  TH2I *dedx_p_pos;

  TH1I *Bz_hist;
  TProfile *constants;
  TProfile2D *profile;
  TH2I *resVsT;
  TH2I *resVsT6;
  TH2I *resVsT9;

  TH1I *MeasDOCA;
  TH1I *ExpDOCA;



  TDirectory *tdir = (TDirectory*)gDirectory->FindObjectAny("CDC_amp");

  if (tdir) {

    tdir->cd();

    a = (TH1I*)gDirectory->Get("a");
    an30_100ns = (TH2I*)gDirectory->Get("an30_100ns");

    amp = 1;

    gDirectory->cd("/");
  }


  tdir = (TDirectory*)gDirectory->FindObjectAny("CDC_dedx");

  if (tdir) {

    tdir->cd();

    dedx_p = (TH2I*)gDirectory->Get("dedx_p");
    dedx_p_pos = (TH2I*)gDirectory->Get("dedx_p_pos");

    dedx = 1;

    gDirectory->cd("/");
  }


  tdir = (TDirectory*)gDirectory->FindObjectAny("CDC_TimeToDistance");

  if (tdir) {

    tdir->cd();

    Bz_hist = (TH1I *) gDirectory->Get("Bz");
    constants = (TProfile*) gDirectory->Get("CDC_TD_Constants");
    profile = (TProfile2D *) gDirectory->Get("Predicted Drift Distance Vs Delta Vs t_drift, FOM 0.9+");
    resVsT = (TH2I*)gDirectory->Get("Residual Vs. Drift Time");
    resVsT6 = (TH2I*)gDirectory->Get("Residual Vs. Drift Time, FOM 0.6+");

    resVsT9 = (TH2I*)gDirectory->Get("Residual Vs. Drift Time, FOM 0.9+");

    ttod = 1;

    gDirectory->cd("/");
  }



  tdir = (TDirectory*)gDirectory->FindObjectAny("CDC_Efficiency");

  if (tdir) {

    tdir->cd();

    MeasDOCA = (TH1I*)(gDirectory->Get("Offline/Measured Hits Vs DOCA"));
    ExpDOCA = (TH1I*)(gDirectory->Get("Offline/Expected Hits Vs DOCA"));

    eff = 1;

    gDirectory->cd("/");
  }



  if ((!amp) && (!dedx) && (!ttod) && (!eff)) exit(0);   // no histograms found


  TFile *fnew = new TFile("cdchistos.root","RECREATE");
  fnew->cd();

  if (amp) {
    fnew->mkdir("CDC_amp");
    fnew->cd("CDC_amp");
    a->Write();
    an30_100ns->Write();
    fnew->cd("/");
  } 

  if (dedx) {
    fnew->mkdir("CDC_dedx");
    fnew->cd("CDC_dedx");
    dedx_p->Write();
    dedx_p_pos->Write();
    fnew->cd("/");
  } 

  if (ttod) {
    fnew->mkdir("CDC_TimeToDistance");
    fnew->cd("CDC_TimeToDistance");
    Bz_hist->Write();
    constants->Write();
    profile->Write();
    resVsT->Write();
    resVsT6->Write();
    resVsT9->Write();
    fnew->cd("/");
  } 

  if (eff) {
    fnew->mkdir("CDC_Efficiency");
    fnew->mkdir("CDC_Efficiency/Online");
    fnew->cd("CDC_Efficiency/Online");
    MeasDOCA->Write();
    ExpDOCA->Write();
    fnew->cd("/");
  }

/*  This would copy the entire directory
TDirectory *tdir = (TDirectory*)gDirectory->Get("CDC_amp")
tdir->ReadAll();

TFile *fnew = new TFile("testdir.root","RECREATE")
fnew->cd()
fnew->mkdir("CDC_amp")
fnew->cd("CDC_amp")
tdir->GetList()->Write()
*/


}
