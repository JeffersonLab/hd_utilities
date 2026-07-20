//
// function: timedifference(int Run, int REF, int REFPLANE)
//
// USE time difference of TOF;
// calculate time difference of reference paddle 
// apply walk correction to the timing
// find paddle hits in the other plane and plot them
// in 2d histogram: paddle-number-other-plane vs time difference
// the find the timing peak in each projection of the paddle number
// and fit the peak location. This is the central position in time
// defined by the geomtric location of the paddle behind/infrontof 
// the reference paddle.
// output file: "calibration/deltat_tpos_refpad%dplane%d.dat",REF,REFPLANE
//
#include "TROOT.h"
#include "TStyle.h"
#include "TMath.h"
#include "TPad.h"
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TF1.h>
#include <TProfile.h>
#include "TSpectrum.h"
#include <fstream>
#include <iostream>

using namespace std;

TH2D *histTD;
int DEBUG = 2; //1: plot histogram, 2: plot and save, 99: interupt

int RunNumber; 
#define NumPMTMax 200
int NPMTS = 0;
int BARS_PER_PLANE = 0; // including 2 short padeles being one
int PMTS_PER_PLANE = 0; 

double GoodMax46[46] = {-8.32894,   -7.90461,   -7.50287,   -7.10451,   -6.71452,   -6.31505, 
			-5.92609,   -5.54767,   -5.16655,   -4.80322,   -4.42346,   -4.05127,   
			-3.67765,   -3.30541, -2.93335,   -2.5596,    -2.19842,   -1.86964,   
			-1.59219,   -1.36424,   -1.17633,   0.,   0.,   0.,  0.,   0.127368,   
			0.313616,   0.542539,   0.826777,   1.14066,    1.51999,    1.8838,     
			2.26335,  2.62937,    2.99592,    3.36639,    3.74159,    4.11813,    
			4.50512,    4.87066,    5.26162,  5.64035,    6.03455,    6.42168,    
			6.82834,    7.21001};

double GoodMax44[44] = {   -8.19623,     -7.79666,     -7.39403,     -7.01673,     -6.61736,     -6.23234,   
			   -5.85287,     -5.47775,     -5.10316,     -4.72913,     -4.35873,    -3.98374,  
			   -3.61354,    -3.23543,    -2.86764,    -2.49516,    -2.11718,    -1.74356,  
			   -1.38293,    -1.0965,     -0.909526,   0.,    0.,    0.0228085,    0.210944,   
			   0.486436,     0.864817,     1.23786,      1.60965,      1.98247,      2.3538,     
			   2.7172,       3.10519,      3.47743,      3.83941,      4.22042,      4.59204,    
			   4.9624,      5.35841,      5.7274,       6.11531,      5.36404,     6.88482,    
			   7.29651};    
double *GoodMax;
double ExpectedMax = -9999.;

void timedifference(int Run, int REF, int REFPLANE){
  
  RunNumber = Run;
  NPMTS = 176;            // TOF 1 geometry
  GoodMax = GoodMax44;
  int NShorts = 1;
  if (RunNumber>69999){
    NPMTS = 184;          // TOF 2 geometry
    GoodMax = GoodMax46;
    NShorts = 2;
  }
  BARS_PER_PLANE = NPMTS/4;
  PMTS_PER_PLANE = NPMTS/2;

  if (TMath::Abs(REF - BARS_PER_PLANE/2 - 0.5 )<NShorts){
    return;
  }

  char ROOTFileName[128];
  sprintf(ROOTFileName,"localdir/tofdata_run%d.root",RunNumber);
  if (RunNumber == 99999)
    sprintf(ROOTFileName,"localdir/big%d.root",RunNumber);

  int REFPAD = REF;
  int THEPLANE = 1;
  if ( REFPLANE ){
    THEPLANE = 0;
  }
  if (histTD==NULL){
    histTD = new TH2D("histTD","Time Difference",47, -1.5, 45.5 , 1600, -12., 12.);
  }

  histTD->Reset();

  //read walk correction factors
  char inf[128];
  sprintf(inf,"calibration%d/tof_walk_parameters_run%d.dat",RunNumber,RunNumber);
  ifstream INF;
  INF.open(inf);
  int idx;
  double FitPar[200][17];
  double dummy;
  for (int n=0; n<NPMTS; n++){
    INF >> idx;
    for (int s=0;s<15;s++) {
      INF >> FitPar[n][s];
    }
    INF>>dummy; // this is the chi2 of both fits
  }
  INF.close();
  double ReferenceLoc = 1500.;  // this is the reference ADC value to calibrate to. any walk correction is zero to this point

  TFile *ROOTFile = new TFile(ROOTFileName);
  TTree *t3 = (TTree*)ROOTFile->Get("TOFcalib/t3");
  int MaxHits = 100;
  int Event;
  int Nhits;
  float TShift;
  int Plane[100];
  int Paddle[100];
  float MeanTime[100];
  float TimeDiff[100];
  int NhitsA;
  int PlaneA[100];
  int PaddleA[100];
  float MeanTimeA[100];
  float TimeDiffA[100];
  float ADCL[100];
  float ADCR[100];
  float PEAKL[100];
  float PEAKR[100];
   
  t3->SetBranchAddress("Event",&Event);
  t3->SetBranchAddress("Nhits",&Nhits);
  t3->SetBranchAddress("TShift",&TShift);
  t3->SetBranchAddress("Plane",Plane);
  t3->SetBranchAddress("Paddle",Paddle);
  t3->SetBranchAddress("MeanTime",MeanTime);
  t3->SetBranchAddress("TimeDiff",TimeDiff);

  t3->SetBranchAddress("NhitsA",&NhitsA);
  t3->SetBranchAddress("PlaneA",PlaneA);
  t3->SetBranchAddress("PaddleA",PaddleA);
  t3->SetBranchAddress("MeanTimeA",MeanTimeA);
  t3->SetBranchAddress("TimeDiffA",TimeDiffA);
  t3->SetBranchAddress("ADCL",ADCL);
  t3->SetBranchAddress("ADCR",ADCR);
  t3->SetBranchAddress("PEAKL",PEAKL);
  t3->SetBranchAddress("PEAKR",PEAKR);
  
  unsigned int nentries = (unsigned int) t3->GetEntries();
  cout<<"Number of Entries "<<nentries<<endl;
  /*
  if (nentries>100000000){
    nentries = 100000000;
    cout<<"  use only 100000000";
  }
  cout<<endl;
  */
  for (unsigned int k=0; k<nentries; k++){
    t3->GetEntry(k);
    //cout<<Event<<" "<<Nhits<<endl;
    if (!(k%10000000)){
      TDatime a;
      cout<<"Current time is "<<a.Get()<<endl;
    }

    int idRef;
    for (idRef=0; idRef<Nhits;idRef++){
      if ((Plane[idRef] == REFPLANE) && (Paddle[idRef] == REFPAD)){
	//cout<<idRef<<endl;
	break;
      }
    }

    float TD_REF = 999.;
    if (idRef<Nhits){

      // find ADC data for Ref Paddle
      for (int n=0; n<NhitsA; n++){
	if ( (PlaneA[n] == REFPLANE) && 
	     (PaddleA[n] == REFPAD) ) {

	  int idxL = REFPLANE*PMTS_PER_PLANE + REFPAD-1;
	  int idxR = idxL + BARS_PER_PLANE;


	  // apply walk correction
	  double ADCval = PEAKL[n];
	  if (ADCval>4090){
	    ADCval = 4090;
	  }
	  double a1 = FitPar[idxL][0]
	    +FitPar[idxL][2]/ADCval
	    +FitPar[idxL][4]/ADCval/ADCval
	    +FitPar[idxL][6]/ADCval/ADCval/ADCval/ADCval
	    +FitPar[idxL][8]/TMath::Sqrt(ADCval);
	  
	  // ReferenceLoc is chosen to be 1500. ADC counts
	  double a2 = FitPar[idxL][0]
	    +FitPar[idxL][2]/ReferenceLoc
	    +FitPar[idxL][4]/ReferenceLoc/ReferenceLoc
	    +FitPar[idxL][6]/ReferenceLoc/ReferenceLoc/ReferenceLoc/ReferenceLoc
	    +FitPar[idxL][8]/TMath::Sqrt(ReferenceLoc);
	  
	  float tcL = a1 - a2;
	  
	  ADCval = PEAKR[n];
	  if (ADCval>4090){
	    ADCval = 4090;
	  }
	  a1 = FitPar[idxR][0]
	    +FitPar[idxR][2]/ADCval
	    +FitPar[idxR][4]/ADCval/ADCval
	    +FitPar[idxR][6]/ADCval/ADCval/ADCval/ADCval
	    +FitPar[idxR][8]/TMath::Sqrt(ADCval);

	  // ReferenceLoc is chosen to be 1500. ADC counts
	  a2 = FitPar[idxR][0]
	    +FitPar[idxR][2]/ReferenceLoc
	    +FitPar[idxR][4]/ReferenceLoc/ReferenceLoc
	    +FitPar[idxR][6]/ReferenceLoc/ReferenceLoc/ReferenceLoc/ReferenceLoc
	    +FitPar[idxR][8]/TMath::Sqrt(ReferenceLoc);


	  float tcR = a1 - a2; 

	  float tcorr = (tcR - tcL)/2.;
	  //cout<<endl;
	  //cout<<tcorr<<"  "<<ADCL[n]<<"  "<<ADCR[n]<<endl;
	  // apply walk correction:
	  TD_REF = TimeDiff[idRef] - tcorr;
	  break;
	}
      }

      for (int n=0; n<Nhits;n++){
	if (Plane[n] == THEPLANE){ 
	  histTD->Fill((double)Paddle[n],TD_REF);
	}  
      }
    }
    
  }

  ROOTFile->Close();

  TH1D *hproj1D = (TH1D*)histTD->ProjectionY("hproj1D",3, BARS_PER_PLANE+3);
  double CenterOFF = hproj1D->GetMean();
  cout<<"Center OFFSET "<<CenterOFF<<endl;
  double tpos[100];
  double dtpos[100];

  // NOTE THE OFFSET OF 3!!!!!!!!!
  for (int j=3; j<3+BARS_PER_PLANE; j++){
    double p = 0.;
    double dp = 0.;
    tpos[j-3] = p;
    dtpos[j-3] = dp;
    TH1D *hp = histTD->ProjectionY("hp",j,j);
    char hnamx[128];
    sprintf(hnamx,"Projection #Deltat to paddel %d REFPAD%d plane%d",j-2,REFPAD,REFPLANE);
    hp->SetTitle(hnamx);

    if (hp->Integral(1,hp->GetNbinsX()-2)>100){

      if (hp->GetEntries()<10000) {
	hp->Rebin(16);
      } else if (hp->GetEntries()<20000) {
	hp->Rebin(8);
      } else {
	hp->Rebin(4);
      }

      if (j-2<8){
	//hp->Rebin(4);
	hp->GetXaxis()->SetRangeUser(-12.,-3.);
      } else if (j-2<17){
	//hp->Rebin(4);
	hp->GetXaxis()->SetRangeUser(-12.,0.);
      } else if (j-2<22){
	//hp->Rebin(4);
	hp->GetXaxis()->SetRangeUser(-12.,1.);
      } else if (j-2>BARS_PER_PLANE-8){
	//hp->Rebin(4);
	hp->GetXaxis()->SetRangeUser(3.,12.);
      } else if (j-2>BARS_PER_PLANE-17){
	//hp->Rebin(4);
	hp->GetXaxis()->SetRangeUser(-1.,12.);
      } else {
	//hp->Rebin(6);
      }


      TSpectrum *speaks = new TSpectrum(6);
      Int_t nfound=0;
      if (DEBUG){
	nfound = speaks->Search(hp,2,"",0.10);
      } else {
	nfound = speaks->Search(hp,2,"nodraw",0.10);
      }

      //printf("Found %d candidate peaks to fit\n",nfound);
      double *xpeaks = speaks->GetPositionX();

      // for paddles on the left of the center take left most peak
      // for paddles on the right of the center take right most peak
      double max = 100.;

      double MaxPeak = 0;
      int MaxPeakLoc=0;
      double MinXloc = 999999.;
      double MaxXloc = -100000.;
      int MinXlocPos = 0;
      int MaxXlocPos = 0;
      
      if (nfound == 1) {
	double xp = xpeaks[0];
	int b = hp->FindBin(xp);
	double by = hp->GetBinContent(b);
	max = xpeaks[0];
	if (DEBUG>1) {
	  cout<<"PeakID: 0    bin is: "<<hp->FindBin(max)
	      <<"   content is "<<hp->GetBinContent(hp->FindBin(max))
	      <<"   xposition = "<<max<<endl;
	  MaxXloc = xp;
	  MinXloc = xp;
	}

      } else if (nfound>1) {

	for (int pp=0;pp<nfound;pp++) {
	  double xp = xpeaks[pp];
	  int b = hp->FindBin(xp);
	  double by = hp->GetBinContent(b);
	  if (DEBUG>1) {
	    cout<<"PeakID: "<<pp<<"    bin is: "<<b
		<<"   content is "<<by<<"   xposition = "<<xp<<endl;
	  }
	  if (by > MaxPeak){
	    MaxPeak = by;
	    MaxPeakLoc = pp;
	  }
	  if (xp<MinXloc){
	    MinXloc = xp;
	    MinXlocPos = pp;
	  }
	  if (xp>MaxXloc){
	    MaxXloc = xp;
	    MaxXlocPos = pp;
	  }
	}

	cout<<"MinX = "<<MinXloc<<"       MaxX = "<<MaxXloc<<endl;
	
	int maxPOS;
	if (j-2<BARS_PER_PLANE/2){
	  max = MinXloc;
	  maxPOS = MinXlocPos;
	} else {
	  max = MaxXloc;
	  maxPOS = MaxXlocPos;
	}
	
      } else {
	cout<<"Error! No peaks found"<<endl; 
      }

      if (DEBUG>98){
	hp->Draw();
	gPad->Update();
	gPad->SetGrid();
	gPad->Update();
	cout<<"MAX is at "<<max<<"     Max Peak is at: "<<xpeaks[MaxPeakLoc]<<endl;
	//getchar();
	sleep(1);
      }
      
      if (TMath::Abs(j-25)>10) {
	if (max != xpeaks[MaxPeakLoc]){
	  max = xpeaks[MaxPeakLoc];
	}
      }

      if (ExpectedMax < -100.){
	ExpectedMax = max;
      } else {
	double d = GoodMax[j-3] - GoodMax[j-4];
	ExpectedMax += d;
      }


      //      if (TMath::Abs(max - GoodMax[j-2-1])>3.2){ // off by more than 1ns from expected postion TOO MUCH
      if ( TMath::Abs(max - ExpectedMax) > 0.5){

	cout<<"max out of range try to find better max for j-2="<<j-2<<endl;
	cout<<"expected position: " << ExpectedMax << endl;

	for (int pp=0;pp<nfound;pp++) {
	  double xp = xpeaks[pp];

	  if (TMath::Abs(xp - ExpectedMax)<0.5){
	    max = xp;
	  }

	}

      }

      cout<<"MINX MAXX: "<<MinXloc<<"  "<<MaxXloc<<"    j="<<j
	  <<"     max="<<max
	  <<"     Nentries = "<<hp->GetEntries()<<endl;   
      //}

      double bw = hp->GetBinWidth(10);
      double hili = max + 5.*bw;
      double loli = max - 5.*bw;
      if ((j-2<BARS_PER_PLANE/2)){
	hili = max + 8*bw;
	if (j-1 == 21) hili = max + 5*bw;
      } else {
	loli = max - 8*bw;
	if (j-1 == 25) loli = max - 5*bw;
      }


      hp->Fit("gaus","RQ0","",loli,hili);
      TF1 *f1 = hp->GetFunction("gaus");
      if (DEBUG>2){
	hp->Draw();
	gPad->Update();
	gPad->SetGrid();
	cout<<"MAX is at "<<max<<endl;
	//getchar();
	sleep(1);
      }

      double sig1 = 1.2;
      double sig2 = 1.;

      //      p = f1->GetParameter(1);
      p = max;
      double s = f1->GetParameter(2);
      hili = p+s*sig1;
      loli = p-s*sig1;
      if ((j-2<BARS_PER_PLANE/2))
	hili = p+s*sig2;
      else {
	loli = p-s*sig2;
      }

      hp->Fit("gaus","RQ","",loli,hili);
      f1 = hp->GetFunction("gaus");
      p = f1->GetParameter(1);
      dp = f1->GetParError(1);

      if ( ((j-2)>16) && ((j-2)<30) ) {
	sig1 = 1.2;
	sig2 = .85;
      }
      hili = p+s*sig1;
      loli = p-s*sig1;
      if ((j-2<BARS_PER_PLANE/2)) {
	hili = p+s*sig2;
	if (j-2 == 21){
	  hili = max + 5.*bw;
	  loli = max - 8.*bw;
	}
      } else {
	loli = p-s*sig2;
	if (j-2 == 26){
	  hili = max + 8.*bw;
	  loli = max - 5.*bw;
	}
      }

      hili = max + 3.*bw;
      loli = max - 3.*bw;

      hp->Fit("gaus","RQ","",loli,hili);
      f1 = hp->GetFunction("gaus");
      p = f1->GetParameter(1);
      dp = f1->GetParError(1);

      if (f1->GetParameter(2)>0.35){
	hili = max + 2.*bw;
	loli = max - 2.5*bw;
	
	hp->Fit("gaus","RQ","",loli,hili);
	f1 = hp->GetFunction("gaus");
	p = f1->GetParameter(1);
	dp = f1->GetParError(1);
      }

      cout<<"SIGMA = "<<f1->GetParameter(2)<<"      POS = "<<p 
	  <<"     d(P-E) = "<<p-ExpectedMax<<endl;
      if (DEBUG>2) {
	hp->Draw();
	gPad->Update();

	cout<<"j ="<< j << "   BARS_PER_PLANE="<<BARS_PER_PLANE<<endl;
	//getchar();
	sleep(1);
      }

      if (DEBUG) {
	char fnam[128];
	if (DEBUG>1){
	  if (j==3) {
	    sprintf(fnam,"Paddle number other plane vs. #Deltat Paddle REFPAD%d",REFPAD);
	    histTD->SetTitle(fnam);
	    histTD->GetYaxis()->SetTitle("#Deltat [ns]");
	    histTD->GetXaxis()->SetTitle("Paddle number [#]");
	    histTD->SetStats(0);
	    histTD->Draw("colz");
	    gPad->SetGrid();
	    //gPad->SetLogz(1);
	    gPad->Update();
	    //gPad->SaveAs("plots/test.root");
	    sprintf(fnam,"plots/paddleNumber_vs_deltatRefPad%d_plane%d_run%d.pdf",REFPAD,REFPLANE,RunNumber);
	    gPad->SaveAs(fnam);
	    sprintf(fnam,"C/paddleNumber_vs_deltatRefPad%d_plane%d_run%d.C",REFPAD,REFPLANE,RunNumber);
	    gPad->SaveAs(fnam);
	  }
	  if (!((j-3)%5) || (DEBUG>2)){
	    hp->GetXaxis()->SetTitle("#Deltat [ns]");
	    char hnam1[128];
	    sprintf(hnam1,"Projection #Deltat to paddel %d REFPAD%d plane%d",j-2,REFPAD,REFPLANE);
	    hp->SetTitle(hnam1);
	    gStyle->SetOptFit(1); 
	    hp->GetXaxis()->SetRangeUser(p-2.,p+2);
	    hp->Draw();
	    //	    f1->Draw("same");
	    gPad->SetLogz(0);
	    gPad->Update();
	    sprintf(hnam1,"plots/deltat_fitposition_p%d_repad%d_plane%d_run%d.pdf",j-2,REFPAD,REFPLANE,RunNumber);
	    gPad->SaveAs(hnam1);
	    sprintf(hnam1,"C/deltat_fitposition_p%d_repad%d_plane%d_run%d.C",j-2,REFPAD,REFPLANE,RunNumber);
	    gPad->SaveAs(hnam1);
	  }
	}
	if (DEBUG == 99){
	  hp->Draw();
	  gPad->SetGrid();
	  gPad->Update();
	  getchar();
	}
      }
    } else {
      cout<<"Error histogram is empty!!!! Paddle "<<j-2<<endl;
      p = 0;
      dp = 0;
      ExpectedMax += .3;
    }
    tpos[j-3] = p;
    dtpos[j-3] = dp;
  }

  char of[128];
  sprintf(of,"calibration%d/deltat_tpos_refpad%dplane%d.dat",RunNumber,REF,REFPLANE);
  ofstream OF(of, std::ofstream::out);

  if (OF){
    cout<<"Write time difference results to file: "<<REF<<" "<<REFPLANE<<endl;
    for (int n=0; n<BARS_PER_PLANE; n++){ 
      OF<<n<<"  "<<tpos[n]<<"  "<<dtpos[n]<<endl;
    }
  } else {
    cout<<"Error open file "<<of<<endl;
  }
  OF.close();
  if (!(REF%10)){
    sprintf(of,"calibration%d/deltat_tpos_refpad%dplane%d.root",RunNumber,REF,REFPLANE);
    TFile *f = new TFile(of,"RECREATE");
    histTD->Write();
    f->Close();
  }

} 

