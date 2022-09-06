// Get CDC monitoring info from rootfile

// Check ttod, dedx, eff.

// Print results to stdout and to file _inspection.txt

// TTOD: For the plot of residuals vs drift time, the mean should be less than 15um and the sigma should be less than 150um.
// dE/dx: At 1.5GeV/c the fitted peak mean should be within 1% of 2.02 keV/cm.
// Efficiency: The efficiency should be 0.98 or higher at 0cm DOCA, gradually fall to 0.97 at approximately 0.5mm and then more steeply through 0.9 at approximately 0.64cm.


int check_ttod(float &mean, float &sigma) {
  // return 0: ok, 1: histo missing, 2: not enough counts/fit problem

  mean=0;
  sigma=0;
  
  TH2I *hist = (TH2I *) gDirectory->Get("/CDC_TimeToDistance/Residual Vs. Drift Time");
  if (!hist) return 1;

  if (hist->GetEntries()<1e6) return 2;
  
  // convert from cm to um
  mean = 1e4*(float)hist->GetMean(2);
  sigma = 1e4*(float)hist->GetRMS(2);

  return 0;

}


int check_dedx(float &mean, float &res) {

  // return 0: ok, 1: histo missing, 2: not enough counts/fit problem

  mean=0;
  res=0;
  
  TH2I *hist = (TH2I *) gDirectory->Get("/CDC_dedx/dedx_p_pos");
  if (!hist) return 1;
  
  float pcut = 1.5;    // draw cut through histo at p=1.5 GeV/c
  int pbin = hist->GetXaxis()->FindBin(pcut);
  TH1D *p = hist->ProjectionY("p1",pbin,pbin);

  if (p->GetEntries()<5e4) return 2;
  
  TF1 *g = new TF1("gaus","gaus",0,12);
  double fitstat = p->Fit(g,"Q0WE");  // fitstat=0 is good

  if (fitstat) return 2;
  
  if (fitstat == 0) {
    double pars[3];
    g->GetParameters(&pars[0]);
    res = (float)(2.0*pars[2]/pars[1]);
    mean = (float)pars[1];
  }

  return 0;  

}


int check_dedx_lowstats(float &mean, float &res) {


  // return 0: ok, 1: histo missing, 2: not enough counts/fit problem
  // use bins 38-48 and scale up by 1.02174 to match the value from bin 38
  
  mean=0;
  res=0;
  
  TH2I *hist = (TH2I *) gDirectory->Get("/CDC_dedx/dedx_p_pos");
  if (!hist) return 1;

  TH1D *p = hist->ProjectionY("p1",38,48);
  if (p->GetEntries()<5000) return 2;

  TF1 *g = new TF1("gaus","gaus",0,12);
  double fitstat = p->Fit(g,"Q0WE");  // fitstat=0 is good

  if (fitstat) return 2;
  
  if (fitstat == 0) {
    double pars[3];
    g->GetParameters(&pars[0]);
    res = (float)(2.0*pars[2]/pars[1]);
    mean = 1.02174*(float)pars[1];
  }

  return 0;  

}



int check_eff(float &e1, float &e2, float &e3) {

  // return 0: ok, 1: histo missing

  e1=0;
  e2=0;
  e3=0;

  TH1I *MeasDOCA = (TH1I*)(gDirectory->Get("/CDC_Efficiency/Offline/Measured Hits Vs DOCA"));
  TH1I *ExpDOCA = (TH1I*)(gDirectory->Get("/CDC_Efficiency/Offline/Expected Hits Vs DOCA"));
  if (!MeasDOCA) return 1;
  if (!ExpDOCA) return 1;

  //  if (hist->GetEntries()<1e6) return 2;

  TGraphAsymmErrors *f = new  TGraphAsymmErrors(MeasDOCA, ExpDOCA, "ac");

  e1 = f->Eval(0); //GetBinContent(pbin);
  e2 = f->Eval(0.5);
  e3 = f->Eval(0.64);
  
  return 0;
  
}


void inspect_rootfile(void) {

// TTOD: For the plot of residuals vs drift time, the mean should be less than 15um and the sigma should be less than 150um.
// dE/dx: At 1.5GeV/c the fitted peak mean should be within 1% of 2.02 keV/cm.
// Efficiency: The efficiency should be 0.98 or higher at 0cm DOCA, gradually fall to 0.97 at approximately 0.5mm and then more steeply through 0.9 at approximately 0.64cm.


  float ttodmean, ttodsigma, dedxmean, dedxres, e0, e5, e6;

  float ttodmeanmax=15.0;
  float ttodsigmax = 150.0;
  bool ttodgood = 0;

  float dedxmin = 2.0;
  float dedxmax = 2.0402;
  bool dedxgood = 0;

  float e0min = 0.97;
  float e5min = 0.96;
  float e6min = 0.89;
  bool effgood = 0;  
  


  int ttodcode = check_ttod(ttodmean,ttodsigma);
  
  if ((ttodsigma>0) && (fabs(ttodmean)<ttodmeanmax) && (ttodsigma < ttodsigmax)) ttodgood = 1;
  
  
  int dedxcode = check_dedx(dedxmean,dedxres);
  if (dedxcode==2) dedxcode = check_dedx_lowstats(dedxmean,dedxres);
  
  if ((dedxmean >= dedxmin) && (dedxmax <= dedxmax)) dedxgood = 1;
  if (dedxres > 0.5) dedxgood = 0;   // undocumented extra


  int effcode = check_eff(e0, e5, e6);

  if ((e0 >= e0min) && (e5>=e5min) && (e6>=e6min)) effgood = 1;


  bool allgood = ttodgood & dedxgood & effgood;
  
  printf("ttod %i %.1f %.1f  dedx %i %.4f %.2f  eff %i %.2f %.2f %.2f  all %i\n",ttodgood, ttodmean, ttodsigma, dedxgood, dedxmean, dedxres, effgood, e0, e5, e6, allgood);

  FILE *fout = fopen("_inspection.txt","w");
  fprintf(fout,"ttod %i %.1f %.1f  dedx %i %.4f %.2f  eff %i %.2f %.2f %.2f  all %i\n",ttodgood, ttodmean, ttodsigma, dedxgood, dedxmean, dedxres, effgood, e0, e5, e6, allgood);
  fclose(fout);
  
}
