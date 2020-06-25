#include <unistd.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TF1.h>
#include <TFile.h>
#include <TCanvas.h>

#include <fstream>


// find signal amplitude of each PMT at +- 10, 8, 6, 4 and 2 ns
// using a simple landau fit to the data from
// the projection of the 2d histogram APMvsDTRawXXX
// where XXX is the PMT number.
//
// write data to file: tof_AAAA_run%d.dat where AAAA is amps or integ depending on what is chosen
//                     these files contain the MPV, error and sigma of the PMT response for all 10
//                     positions along a paddle for all full length paddles.
//                     additional output files are:
//                     landauMPV_runxxxxxx_amplitue.dat or landauMPV_runxxxxx_integral.dat
//                     these files contain the MPV positions at the center of the paddle
//                     attenfit_runxxxxxx_amplitude.dat or attenfit_runxxxxx_integral.dat
//                     these files contain the single exponential fit results
//


int DEBUG = 1;
int INTEG = 0;

float findpeak(TH1D *h, int tag){
  
  float locmax = 0;
  TH1D *hist;
  if (!INTEG) {
    hist = (TH1D*) h->Rebin(10,"hist")->Clone();
  } else {
    hist = (TH1D*) h->Rebin(3,"hist")->Clone();
  }

  if (!INTEG){
    hist->GetXaxis()->SetRangeUser(10., 3600.);
  }
  int nbins = hist->GetSize();

  if (0) {
    for (int n=1;n<nbins-5;n++){
      hist->Fit("pol2","Q","QR",hist->GetBinCenter(n),hist->GetBinCenter(n+4));
      
      TF1 *f1 = hist->GetFunction("pol2");
      if (f1 == NULL){
	hist->Draw();
	gPad->SetGrid();
	gPad->Update();
	cout<<"peakfinding failed!"<<endl;
	locmax = 500.;
	getchar();
	break;
      }
      float quad = f1->GetParameter(2);
      if (quad > 0){
	hist->GetXaxis()->SetRange(n+4,nbins-2);
	int maxbin = hist->GetMaximumBin();
	locmax = hist->GetBinCenter(maxbin);
	//cout<<"Maximum Found at "<<locmax<<endl;
	break;
      }
      
    }
    
    cout<<"Found peak at "<<locmax<<endl;
    //TH1D *hist1 = (TH1D*) h->Rebin(30,"hist1");
  }

  hist->SetBinContent(hist->GetXaxis()->GetNbins(), 0.);
  hist->SetBinContent(hist->GetXaxis()->GetNbins()-1, 0.);
  hist->SetBinContent(hist->GetXaxis()->GetNbins()-2, 0.);
  for (int n=1;n<4;n++){
    hist->SetBinContent(n, 0.);
  }
  int npeaks = 20;
  TSpectrum *s = new TSpectrum(2*npeaks);
  Int_t nfound = s->Search(hist, 2, "", 0.10);
  printf("Found %d peak candidates to fit\n",nfound);
  double *X = s->GetPositionX();
  cout<<"Peak positions: ";
  for (int k=0;k<nfound;k++){
    cout<<"   "<<X[k];
  }
  cout<<endl;

  if (DEBUG==99){
    hist->Draw();
    gPad->Update();
    getchar();
  }


  if (nfound>1){
    locmax = X[0];
    if (locmax<350.){
      locmax=X[1];
      cout<<"change locmax to: "<<locmax<<endl;
    }
    if (X[1]>X[0]){
      locmax = X[1];
    }
  } else if (nfound==1) {
    locmax = X[0];
  } else {
    locmax = 500.;
  }

  if (tag){

    int NB = hist->GetXaxis()->GetNbins();

    double a[5];
    for (int k = 15; k<NB-5; k++){

      for (int n=0; n<5; n++){
	a[n] = hist->GetBinContent(k+n);
      }
      if ( (a[0]<=a[1]) && (a[1]<=a[2]) && 
	   (a[2]>=a[3]) && (a[3]>=a[4]) ){
	locmax = hist->GetBinCenter(k+2);
	break;
      }
    }
    if (1){
      hist->Draw();
      gPad->SetGrid();
      gPad->Update();
      cout<<locmax<<endl;
      getchar();
    }
  }

  return locmax;
}

//int Nbars = 44; // TOF2 will be 46
//int Nshort = 1; // TOF2 will be 2
int Nbars = 46; // TOF2 will be 46
int Nshort = 2; // TOF2 will be 2

void findamp(int RunNumber, int useINT){
  
  char ofnam[128];
  sprintf(ofnam,"results/tof_amps_run%d.dat",RunNumber);  
  if (useINT){
    sprintf(ofnam,"results/tof_integ_run%d.dat",RunNumber);  
    INTEG = 1;
  }
  ofstream OUTF(ofnam);
  
  char Rfnam[128]; // root file name
  sprintf(Rfnam,"localdir/tofmon_run%d.root",RunNumber);
  TFile *RF = NULL;
  RF = new TFile(Rfnam,"READ");

  if (RF == NULL){
    cout<<"error open file "<<Rfnam<<" BAIL!"<<endl;
    return;
  }

  TCanvas *c1 = new TCanvas("c1","PLOTS",900,600);

  if (RunNumber<70000){
    Nbars = 44;
    Nshort = 1;
  }


  double LandauPeak[2][92];
  
  for (int plane=0;plane<2;plane++){
    
    for (int paddle=0; paddle<Nbars; paddle++){
      
      int idx[2];
      idx[0] = paddle;
      idx[1] = paddle + Nbars;
      
      for (int k=0;k<2;k++){  // loop to look at both ends of a paddle
	
	char hnam[128];
	sprintf(hnam, "TOFmon/AMPvsDTRaw%d%02d", plane, idx[k]);
	if (useINT){
	  sprintf(hnam, "TOFmon/INTvsDTRaw%d%02d", plane, idx[k]);
	}
	TH2F *h2d = (TH2F*)RF->Get(hnam);
        if (h2d == NULL){
          cout<<"ERROR not histogram:"<<hnam<<endl;
          continue;
        }

	if (TMath::Abs(paddle - Nbars/2 + 0.5)<Nshort) {

	  TH1D *hpe = (TH1D*)h2d->ProjectionY("hpe", 10, h2d->GetXaxis()->GetNbins()-1);
	  float max = findpeak(hpe, 0);
	  cout<<"Paddle "<<paddle<<"  PMT "<<idx[k]<<"        found peak at: "<<max<<endl;
	  float width = max*0.11;	  
	  float loli = max - 1.5*width;
	  float hili = max + 5*width;
	  hpe->Fit("landau","Q","QR",loli,hili);
	  TF1 *f1 = hpe->GetFunction("landau");
	  
	  float MPV = f1->GetParameter(1);
	  float SIG = f1->GetParameter(2);
	  
	  if (DEBUG==1){
	    hpe->Draw();
	    gPad->SetGrid();
	    gPad->Update();
	    //getchar();
	  }

	  LandauPeak[plane][idx[k]] = MPV;
	  continue;  // do not look at single ended paddles
	}

	TH1D *hpj = (TH1D*)h2d->ProjectionX("hpj", 10, h2d->GetYaxis()->GetNbins()-100);
	double center = hpj->GetMean();
	
	TH1D *h1[10] ;
	double t = -10. + center;
	char hn1[128];
	double XLoc[10];

	for (int n=0;n<10;n++){
	  sprintf(hnam,"h%02d",n+10);
	  t  = -10. + center + 2.*n;
	  int bc = hpj->FindBin(t);
	  XLoc[n] = -10. + 2.*n;
	  if (n>4){
	    t = 2. + center + 2.*(n-5);
	    bc = hpj->FindBin(t);
	    XLoc[n] = 2. + 2.*(n-5);
	  }


	  h1[n] = (TH1D*)h2d->ProjectionY(hnam, bc-3, bc+3); 
	  sprintf(hn1, "Plane %d  PMT# %02d   id%d",plane, idx[k], n);
	  h1[n]->SetTitle(hn1);
	  for (int j=1;j<21;j++){ // set the first 10 bins to zero
	    h1[n]->SetBinContent(j, 0.);
	  }
	}
	
	double Yval[10];
	double dYval[10];

	for (int n=0;n<10;n++) {

	  double MaxBinCenter = h1[n]->GetBinCenter(h1[n]->GetXaxis()->GetNbins()-100);	  
	  h1[n]->GetXaxis()->SetRangeUser(10., MaxBinCenter);
	  float max = findpeak(h1[n], 0);

	  if (max>MaxBinCenter){
	    float newmax = max;
	    int binu = h1[n]->FindBin(MaxBinCenter*0.15);
	    int mf = h1[n]->GetBinContent(9);
	    for (int nb=10; nb<binu; nb++){
	      if (mf<h1[n]->GetBinContent(nb)){
		mf = h1[n]->GetBinContent(nb);
		newmax = h1[n]->GetBinCenter(nb);
	      }
	    }
	    max = newmax;
	  }
	  
	  float width = max*0.15;	  
	  float loli = max - 1.5*width;
	  float hili = max + 5*width;

	  h1[n]->Fit("landau","Q","QR",loli,hili);
	  TF1 *f1 = h1[n]->GetFunction("landau");
	  
	  float MPV = f1->GetParameter(1);
	  float SIG = f1->GetParameter(2);
	  
	  if (DEBUG==99) {
	    h1[n]->Draw();
	    gPad->SetGrid(1);
	    gPad->Update();
	    
	    cout<<"Give MPV location(or zero): ";
	    double NewMPV = 1000.;
	    cin>>NewMPV;
	    if (NewMPV>10.){
	      max = NewMPV;
	      width = max*0.15; 
	      loli = max - 1.5*width;
	      hili = max + 7*width;
	      h1[n]->Fit("landau","Q","QR",loli,hili);
	      f1 = h1[n]->GetFunction("landau");
	      MPV = f1->GetParameter(1);
	      SIG = f1->GetParameter(2);
	      
	      h1[n]->Draw();
	      gPad->SetGrid(1);
	      gPad->Update();
	      getchar();
	      

	    }
	  }

	  width = SIG; 
	  loli = MPV - 1.5*width;
	  hili = MPV + 5*width;
	  h1[n]->Fit("landau","Q","QR",loli,hili);
	  f1 = h1[n]->GetFunction("landau");
	  MPV = f1->GetParameter(1);
	  SIG = f1->GetParameter(2);
	  float dMPV = f1->GetParError(1);

	  Yval[n] = MPV;
	  dYval[n] = dMPV;

	  if (DEBUG == 1){
	    h1[n]->Draw();
	    gPad->SetGrid();
	    gPad->Update();
	    //getchar();
	  }
	  
	  cout<<plane<<" "<<paddle<<" PMT "<<idx[k]<<"  "<<n<<" "
	      <<MPV<<"  "<<dMPV<<"  "<<SIG<<endl;
	  OUTF<<plane<<" "<<paddle<<"     "<<idx[k]<<"  "<<n<<" "
	      <<MPV<<"  "<<dMPV<<"  "<<SIG<<endl;
	  h1[n]->Draw();
	  c1->Modified();
	  
	  if (DEBUG>1) {
	    h1[n]->GetXaxis()->SetTitle("ADC amplitude [arb.]");
	    if (useINT){
	      h1[n]->GetXaxis()->SetTitle("ADC Integral [arb.]");
	    }
	    h1[n]->GetXaxis()->SetRangeUser(50., 3500.);
	    h1[n]->Draw();
	    gPad->SetGrid();
	    gPad->Update();
	    getchar();
	    if (DEBUG>99){
	      c1->SaveAs("landau_fit.pdf");
	      return;
	    }
	  }
	} // end of loop over n positions along paddle

	if (DEBUG==1) {
	  TGraphErrors *gr = new TGraphErrors(10, XLoc, Yval, NULL, dYval);
	  char pnam[128];
	  sprintf(pnam, "Paddle %d  SIDE=%d    Plane %d", paddle+1, k, plane);
	  gr->SetTitle(pnam);
	  gr->Draw("AP");
	  gr->Fit("expo", "", "R", -6., 6.); // fit only central region
	  gr->GetXaxis()->SetTitle("Paddle location in #deltat [ns]");
	  gr->GetYaxis()->SetTitle("Landau MPV PMT-Resp. Amplitude [ADC]");
	  if (useINT){
	    gr->GetYaxis()->SetTitle("Landau MPV PMT-Resp. Integral [ADC]");
	  }
	  gr->SetMarkerStyle(21);
	  gr->SetMarkerColor(4);
	  gPad->SetGrid();
	  gPad->Update();
	  sprintf(pnam,"plots/p%02d_s%d_plane%d_amplitude.gif",paddle+1,k,plane);
	  if (useINT){
	    sprintf(pnam,"plots/p%02d_s%d_plane%d_integral.gif",paddle+1,k,plane);
	  }
	  c1->SaveAs(pnam);
	  //getchar();

	  sprintf(pnam, "results/attenfit_run%d_amplitude.dat",RunNumber);
	  if (useINT){
	    sprintf(pnam, "results/attenfit_run%d_integral.dat",RunNumber);
	  }
	  ofstream OFn;
	  OFn.open(pnam, std::ofstream::app);
	  double con, slope;
	  TF1 *f1 = (TF1*)gr->GetFunction("expo");
	  con = f1->GetParameter(0);
	  slope = f1->GetParameter(1);
	  OFn<<plane<<"   "<<paddle+1<<"    "<<k<<"   "<<con<<"    "<<slope<<endl;
	  OFn.close();

	  double ppos = f1->Eval(0.);
	  LandauPeak[plane][idx[k]] = ppos;


	}

      }
    }

  }
  OUTF.close();
  RF->Close();

  ofstream OF1;
  char fn[128];
  sprintf(fn,"results/landauMPV_run%d_amplitude.dat",RunNumber);
  if (useINT){
    sprintf(fn,"results/landauMPV_run%d_integral.dat",RunNumber);
  }
  OF1.open(fn);
  for (int n=0;n<2;n++){
    for (int j=0; j<Nbars*2; j++){
      OF1<<n<<"  "<<j<<"   "<<LandauPeak[n][j]<<endl;
    }
  }
  OF1.close();

}
