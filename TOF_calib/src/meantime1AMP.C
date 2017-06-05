//
// main function: meantime1(int Run, int REF, int RefPlane)
//
// Determine meantime-difference between ref-paddle and all
// other 90deg. rotated paddles behind/in-front-of it.
// Fit the meantime-difference peak for each paddle w.r.t. the
// reference paddle.
// To get better results apply the walk corrections and make a cut
// the time difference to match the geometric position of the reference
// paddle the cut is +/- 30cm The cut can not be too tight because the
// times are not yet calibrated ;-)
// output file: "calibration/mtpos_refpad%dplane%d.dat",REFPAD,REFPLANE
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

TH2F *dMT;
TH2F *twalkcorr[176];
int DEBUG = 2; // 1: show plots, 2: save plots, 99: save plots and interupt 

int NOWALK = 0;
int REFPAD = 18;
int REFPLANE = 0;
int RunNumber;

void findpeak(double*, double*);

void meantime1AMP(int Run, int REF, int RefPlane){
  
  RunNumber = Run;
  char ROOTFileName[128];
  sprintf(ROOTFileName,"localdir/tofdata_run%d.root",RunNumber);
  if (RunNumber == 99999)
    sprintf(ROOTFileName,"localdir/big%d.root",RunNumber);

  REFPAD = REF;
  REFPLANE = RefPlane;
  int THEPLANE = 1;
  if (REFPLANE){
    THEPLANE = 0;
  }

  // create dMT thefirst time
  if (dMT==NULL){
    dMT = new TH2F("dMT","Mean Time Difference",300, -10.,10.,44,0.,44.);
  }
  char hnam[128];
  char htit[128];
  for (int k=0; k<176; k++){
    sprintf(hnam,"twalkcorr%d",k);
    sprintf(htit,"time walk corrected %d",k);
    twalkcorr[k] = new TH2F(hnam,htit,500,0., 4096.,150.,270.,295.);
  }

  dMT->Reset();
  char htit1[128];
  sprintf(htit1,"Mean Time Difference Ref-Pad%d Plane %d",REFPAD,REFPLANE);
  dMT->SetTitle(htit1);

  //read walk correction factors
  char inf[128];
  sprintf(inf,"calibration%d/tof_walk_parameters_AMP_run%d.DB",RunNumber,RunNumber);
  ifstream INF;
  INF.open(inf);
  int idx;
  double timewalk_parameters_AMP[176][6];
  for (int n=0; n<176; n++){
    INF >>  timewalk_parameters_AMP[n][0] >> timewalk_parameters_AMP[n][1] >> timewalk_parameters_AMP[n][2] 
        >> timewalk_parameters_AMP[n][3] >> timewalk_parameters_AMP[n][4] >> timewalk_parameters_AMP[n][5];
  }
  INF.close();

  // define xpos of the reference paddle by geometry
  int REFPADi = REFPAD-1;
  float xpos= 999.;
  if (REFPADi<19){
    xpos = -15. - 6.0*(18. - REFPADi);
  } else if (REFPADi>24) {
    xpos = 15. + 6.0*(REFPADi-24.);
  } else if (REFPADi<21){
    xpos = -7.5 - 3.0*(20. - REFPADi);
  } else if (REFPADi>22){
    xpos = 7.5 + 3.0*(REFPADi-23.);
  }

  // prepare root file and tree to read from 
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
  float PEAKR[100];
  float PEAKL[100];
  
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
  
  for (unsigned int k=0; k<nentries; k++){
    t3->GetEntry(k);
    //cout<<Event<<" "<<Nhits<<endl;

    // select events that have a hit in the reference paddle
    int idRef;
    for (idRef=0; idRef<Nhits;idRef++){
      if ((Plane[idRef] == REFPLANE) && (Paddle[idRef] == REFPAD)){
	//cout<<idRef<<endl;
	break;
      }
    }

    // in case of hit in reference paddle find ADC data and apply walk correction
    // to get corrected mean time.
    float MT_Ref=999.;
    if (idRef<Nhits){
      // find ADC data for Ref Paddle
      for (int n=0; n<NhitsA; n++){
	if ( (PlaneA[n] == REFPLANE) && 
	     (PaddleA[n] == REFPAD) ) {

	  int idxL = REFPLANE*88 + PaddleA[n]-1;
	  int idxR = idxL + 44;

	  int IDX[2];
	  IDX[0] = idxL;
	  IDX[1] = idxR;
	  double AMP[2];
	  AMP[0] = PEAKL[n]; 
	  AMP[1] = PEAKR[n]; 
	  float corr[2];
	  
	  //apply walk correction
	  for (int i=0; i<2;i++) {
	    double C0 = timewalk_parameters_AMP[IDX[i]][0];
	    double C1 = timewalk_parameters_AMP[IDX[i]][1];
	    double C2 = timewalk_parameters_AMP[IDX[i]][2];
	    double C3 = timewalk_parameters_AMP[IDX[i]][3];
	    double hookx = timewalk_parameters_AMP[IDX[i]][4]; 
	    double refx = timewalk_parameters_AMP[IDX[i]][5];
	    double val_at_ref = C0 + C1*TMath::Power(refx,C2); 
	    double val_at_hook = C0 + C1*TMath::Power(hookx,C2); 
	    double slope = (val_at_hook - C3)/hookx;

	    if (refx>hookx){
	      val_at_ref  = slope * refx + C3; 
	    }

	    double val_at_A = C0 + C1*TMath::Power(AMP[i],C2);
	    if (AMP[i]>hookx){
	      val_at_A = slope * AMP[i] + C3; 
	    }
	    
	    corr[i] = val_at_A - val_at_ref;

	    //cout<<i<<":  "<<hookx<<"  "<<refx<<"   "<<AMP[i]<<" "<<slope<<"  "<<val_at_A<<"  "<<val_at_ref<<endl; 
	    //cout<<"        "<<C0<<"  "<<C1<<"  "<<C2<<endl;

	  }

	  MT_Ref = MeanTime[idRef] - (corr[0]+corr[1])/2.;

	  float tl = MeanTime[idRef] - TimeDiff[idRef];
	  float tr = MeanTime[idRef] + TimeDiff[idRef];
	  float tlA = MeanTimeA[n] - TimeDiffA[n];
	  float trA = MeanTimeA[n] + TimeDiffA[n];
	  
	  tl -= corr[0];
	  tr -= corr[1];

	  float dtL = tl - tlA + TShift;
	  float dtR = tr - trA + TShift;

	  twalkcorr[idxL]->Fill(PEAKL[n], dtL);
	  twalkcorr[idxR]->Fill(PEAKR[n], dtR);
	  
	  break;
	}
      }

      // loop over paddles to find hit in other paddles and apply
      // the walk correction. Then subract the reference mean time from the paddle mean time
      // and fill it in a 2-d histogram.
      float MT_Pad = 0;
      float TD_Pad = 999.;
      for (int n=0; n<Nhits;n++){
	if (Plane[n] == THEPLANE){
	  for (int j=0; j<NhitsA; j++){
	    if ((PlaneA[j] == THEPLANE) && (PaddleA[j] == Paddle[n])){
	      int idxL = 88 * THEPLANE + PaddleA[j]-1;
	      int idxR =  idxL + 44;

	      int IDX[2];
	      IDX[0] = idxL;
	      IDX[1] = idxR;
	      double AMP[2];
	      AMP[0] = PEAKL[n]; 
	      AMP[1] = PEAKR[n]; 
	      float corr[2];

	      //apply walk correction
	      for (int i=0; i<2;i++) {
		double C0 = timewalk_parameters_AMP[IDX[i]][0];
		double C1 = timewalk_parameters_AMP[IDX[i]][1];
		double C2 = timewalk_parameters_AMP[IDX[i]][2];
		double C3 = timewalk_parameters_AMP[IDX[i]][3];
		double hookx = timewalk_parameters_AMP[IDX[i]][4]; 
		double refx = timewalk_parameters_AMP[IDX[i]][5];
		double val_at_ref = C0 + C1*TMath::Power(refx,C2); 
		double val_at_hook = C0 + C1*TMath::Power(hookx,C2); 
		double slope = (val_at_hook - C3)/hookx;

		if (refx>hookx){
		  val_at_ref  = slope * refx + C3; 
		}

		double val_at_A = C0 + C1*TMath::Power(AMP[i],C2);
		if (AMP[i]>hookx){
		  val_at_A = slope * AMP[i] + C3; 
		}
		
		corr[i] = val_at_A - val_at_ref;
	      }
	      MT_Pad = MeanTime[n] - (corr[0]+corr[1])/2.;
	      TD_Pad = TimeDiff[n] - (corr[1]-corr[0])/2.;


	      float tl = MeanTime[n] - TimeDiff[n];
	      float tr = MeanTime[n] + TimeDiff[n];
	      float tlA = MeanTimeA[j] - TimeDiffA[j];
	      float trA = MeanTimeA[j] + TimeDiffA[j];
	      
	      tl -= corr[0];
	      tr -= corr[1];
	      
	      float dtL = tl - tlA + TShift;
	      float dtR = tr - trA + TShift;
	      
	      twalkcorr[idxL]->Fill(PEAKL[j], dtL);
	      twalkcorr[idxR]->Fill(PEAKR[j], dtR);

	      break;
	    }
	  }
	  float thepos = TD_Pad*15.5;  // rought time to distance
	  //cout<<xpos<<"   "<<thepos<<endl;
	  if (TMath::Abs(thepos-xpos)<30.) {
	    float MT = MT_Pad - MT_Ref;
	    dMT->Fill(MT,Paddle[n]-1);
	  }
	}  
      }
    }

  }

  ROOTFile->Close();

  char of[128];

  double ppos[44];
  double psig[44];
  // find the peaks in all the 1-d projections of the 2-d histogram
  findpeak(ppos,psig);
  

  sprintf(of,"calibration%d/mtpos_refpad%dplane%d.dat",RunNumber,REFPAD,REFPLANE);
  ofstream OF;
  OF.open(of);
  if (OF){
    for (int n=0;n<44;n++){
      OF<<n<<" "<<ppos[n]<<" "<<psig[n]<<endl;
    }
  }
  OF.close();

  sprintf(of,"plots/mt_diff_ref%d_pl%d_run%d.root",REF,REFPLANE,Run);
  TFile *RF = new TFile(of,"RECREATE");
  dMT->Write();
  for (int k=0; k<176; k++){
    twalkcorr[k]->Write();
  }
  RF->Close();
} 

void findpeak(double *MTPosition, double *MTSigma){


  // loop over all 44 bins of the vertical axis of the 2-d histograms
  // these are the paddles orthogonal to the reference paddle.
  // find the peak in these 1-d distributions using Gaussian fits.
  
  if (DEBUG==99){
    dMT->Draw("colz");
    gPad->Update();
    getchar();
  }


  for (int k=1;k<45;k++ ){

    TH1D *h = dMT->ProjectionX("h",k,k);

    double pos = 0;
    double sig = 0;

    if (h->GetEntries()>100){
      
      //cout<<"Paddle: "<<k<<endl;
      TSpectrum *speaks = new TSpectrum(4);
      Int_t nfound = 0;
      if (DEBUG) {
	nfound = speaks->Search(h,2,"",0.40);
      } else {
 	nfound = speaks->Search(h,2,"nodraw",0.40);
     }
      
      //printf("Found %d candidate peaks to fit\n",nfound);
      double *xpeaks = speaks->GetPositionX();
      double max = 0.;
      double MaxPeak = 0.;
      int MaxPN;
      for (int pp=0;pp<nfound;pp++) {
	double xp = xpeaks[pp];
	cout<<"Peak "<<pp<<": x = "<<xp<<endl;

	Int_t bin = h->GetXaxis()->FindBin(xp);
	double yp = h->GetBinContent(bin);
	if (yp>MaxPeak){
	  MaxPeak = yp;
	  MaxPN = pp;
	  max = xp;
	}
      }

      double maxc = max;
      double bw = h->GetBinWidth(1);
      double loli = maxc - 4.*bw;
      double hili = maxc + 4.*bw;
      if (loli<-10){
	loli = -10.;
      }
      if (hili>10.){
	hili = 10.;
      }

      h->Fit("gaus","QR","",loli,hili);
      TF1 *f1 = h->GetFunction("gaus");
      pos = f1->GetParameter(1);
      sig = f1->GetParameter(2);
      hili = pos + 1*sig;
      loli = pos - 1*sig;
      h->Fit("gaus","QR","",loli,hili);
      f1 = h->GetFunction("gaus");
      pos = f1->GetParameter(1);
      sig = f1->GetParameter(2);
      hili = pos + 1.*sig;
      loli = pos - 1.*sig;
      h->Fit("gaus","QR","",loli,hili);
      f1 = h->GetFunction("gaus");
      pos = f1->GetParameter(1);      

      if (DEBUG){
	if (DEBUG>1){
	  char exnam[128];
	  if (k==3){
	    dMT->GetYaxis()->SetTitle("Paddle Number [#]");
	    sprintf(exnam,"MeanTime Difference MT_{i}-MT_{ref} [ns]");
	    dMT->GetXaxis()->SetTitle(exnam);
	    dMT->Draw("colz");
	    gPad->SetGrid();
	    gPad->SetLogz(1);
	    gPad->Update();
	    sprintf(exnam,"plots/mtdiff_vs_padnum_RefPad%d_RefPlane%d_run%d.pdf",REFPAD,REFPLANE,RunNumber);
	    if (NOWALK){
	      sprintf(exnam,"plots/mtdiff_vs_padnum_RefPad%d_run%d_nowalk.pdf",REFPAD,RunNumber);
	    }
	    gPad->SaveAs(exnam);
	  }
	  if (DEBUG>2){
	    gPad->SetLogz(0);  
	    h->Draw();
	    sprintf(exnam,"Mean-Time-Difference Pad%d to Ref-Pad%d",k,REFPAD);
	    h->SetTitle(exnam);
	    h->GetXaxis()->SetTitle("MeanTime Difference MT_{i}-MT_{ref} [ns]");
	    h->GetYaxis()->SetTitle("Counts [#]");
	    gPad->SetGrid();
	    gPad->Update();
	    sprintf(exnam,"plots/mtdiff_vs_padnum_proj_%d_ref%d_RefPlane%d_run%d.pdf",k,REFPAD,REFPLANE,RunNumber);
	    if (NOWALK){
	      sprintf(exnam,"plots/mtdiff_vs_padnum_proj_%d_ref%d_run%d_nowalk.pdf",k,REFPAD,RunNumber);
	    }
	    gPad->SaveAs(exnam);
	  }
	}
	h->Draw();
	gPad->Update();
	if (DEBUG == 99)
	  getchar();
      }
    }
    MTPosition[k-1] = pos;
    MTSigma[k-1] = sig;
  }

  return;
}
