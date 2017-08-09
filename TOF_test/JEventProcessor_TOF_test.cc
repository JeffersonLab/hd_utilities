// $Id$
//
//    File: JEventProcessor_TOF_test.cc
// Created: Wed May 27 09:41:19 EDT 2015
// Creator: zihlmann (on Linux gluon47.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//


#include "JEventProcessor_TOF_test.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_TOF_test());
  }
} // "C"


//------------------
// JEventProcessor_TOF_test (Constructor)
//------------------
JEventProcessor_TOF_test::JEventProcessor_TOF_test()
{
  

}

//------------------
// ~JEventProcessor_TOF_test (Destructor)
//------------------
JEventProcessor_TOF_test::~JEventProcessor_TOF_test()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_TOF_test::init(void)
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  //
  // japp->RootWriteLock();
  //  ... fill historgrams or trees ...
  // japp->RootUnLock();
  //
  
  TRACKTEST = 1;
  DEBUGX = 0;


  return NOERROR;

}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TOF_test::brun(JEventLoop *eventLoop, int32_t runnumber)
{

  // This is called whenever the run number changes

  cout<<"Generate Histograms brun()"<<endl;

  TDirectory *top = gDirectory;
  sprintf(ROOTFileName,"tofdata_test.root");

  ROOTFile = new TFile(ROOTFileName,"recreate");
  ROOTFile->cd();
  ROOTFile->mkdir("TOFtest");
  ROOTFile->cd("TOFtest");
 

  RunNumber = runnumber;
  //sprintf(ROOTFileName,"tofdata_test.root",RunNumber);

  char hnam[128];
  char htit[128];
  sprintf(htit,"Mean Time Difference Run %d ",runnumber);
  MTdiff = new TH1D("MTdiff", "Mean Time Difference Run",600, -5., 5.);

  adcPeakValue = new TH2D("adcPeakValue","ADC Peak Value for all paddles", 176, 0, 176, 200., 0., 4096.);

  for (int k=0;k<44;k++) {
    sprintf(hnam,"mtDIFF0%d",k);
    sprintf(htit,"Run %d: meantime difference to paddle %d Plane1 ",runnumber,k+1);
    mtDIFF0[k] = new TH2D(hnam, htit, 150, -3., 3., 44, 0., 44.);

    sprintf(hnam,"mtDIFF1%d",k);
    sprintf(htit,"Run %d: meantime difference to paddle %d Plane2",runnumber,k+1);
    mtDIFF1[k] = new TH2D(hnam, htit, 150, -3., 3., 44, 0., 44.);

    sprintf(hnam,"impactangles0%d",k);
    sprintf(htit,"Impact Angle Paddle %d Plane 0",k+1);
    impactangles[0][k] = new TH1D(hnam,htit,100, 0.,20.);


    sprintf(hnam,"impactangles1%d",k);
    sprintf(htit,"Impact Angle Paddle %d Plane 1",k+1);
    impactangles[1][k] = new TH1D(hnam,htit,100, 0.,20.);

  }

  vector<double> speeds;
  eventLoop->GetCalib("TOF/propagation_speed",speeds);
  for (unsigned int k=0; k<speeds.size(); k++) {
    int Plane = k/44;
    int Paddle = k - Plane*44; 
    speed[Plane][Paddle] = speeds[k];
  }


  if (TRACKTEST){
    for (int k=0;k<176;k++) {
      // these histograms can be used to determine the attenuation length
      // separately for left and right PMT and for Energy calibration
      // for the individual PMTs on each side of the paddles.
      char hnam[128];
      char htit[128];
      sprintf(hnam,"adc_hist%d",k);
      int plane = k/88;
      int en = k/44 - plane*2;
      int paddle = k - ((k/44) * 44);
      sprintf(htit,"Hit-position vs Energy Deposition Plane %d Paddle %d End %d",plane, paddle, en);     
      adc_hist[k] = new TH2D(hnam,htit,400, 0., 28000., 42, 0., 252.);
      adc_hist[k]->GetXaxis()->SetTitle("Energy deposition [ADC]");
      adc_hist[k]->GetYaxis()->SetTitle("Distance from paddle end [cm]");

      sprintf(hnam,"adc_histP%d",k);
      sprintf(htit,"Hit-position vs ADC-Peak Plane %d Paddle %d End %d",plane, paddle, en);     
      adc_histP[k] = new TH2D(hnam,htit,400, 0., 4048., 42, 0., 252.);
      adc_histP[k]->GetXaxis()->SetTitle("Signal Peak [ADC]");
      adc_histP[k]->GetYaxis()->SetTitle("Distance from paddle end [cm]");

      sprintf(hnam,"adc_histPad%d",k);
      sprintf(htit,"x vs E from PaddleObject Plane %d Paddle %d End %d",plane, paddle, en);  
      adc_histPad[k] = new TH2D(hnam,htit,400, 0., 28000., 42, 0., 252.);
      adc_histPad[k]->GetXaxis()->SetTitle("Energy deposition [ADC]");
      adc_histPad[k]->GetYaxis()->SetTitle("Distance from paddle end [cm]");

      sprintf(hnam,"E_hist%d",k);
      sprintf(htit,"Energy Deposition at PMT: Plane %d Paddle %d End %d",plane, paddle, en);     
      E_hist[k] = new TH1D(hnam,htit, 100, 0., 0.02);
      E_hist[k]->GetXaxis()->SetTitle("Energy deposition [GeV]");

      sprintf(hnam,"pvsE_hist%d",k);
      sprintf(htit,"Energy Deposition at PMT: Plane %d Paddle %d End %d",plane, paddle, en);     
      pvsE_hist[k] = new TH2D(hnam,htit, 100, 0., 0.02, 20, 0., 5.);
      pvsE_hist[k]->GetXaxis()->SetTitle("Energy deposition [GeV]");
    }
  }

  dedxtracksAll = new TH2D("dedxtracksAll","dEdx All tracks vs momentum",100, 0., 5.,100., 0., 10.e-6);
  dedxtracksFDC = new TH2D("dedxtracksFDC","dEdx FDC tracks vs momentum",100, 0., 5.,100., 0., 10.e-6);

  top->cd();

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TOF_test::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // loop->Get(mydataclasses);
  //
  // japp->RootWriteLock();
  //  ... fill historgrams or trees ...
  // japp->RootUnLock();


  vector <const DL1Trigger*> trig;
  loop->Get(trig);
  
  if (trig.size() == 0){
    //cout<<"no trigger found"<<endl;
    return NOERROR;	
  }

  if (trig[0]->fp_trig_mask>0){ // this is a front pannel trigger
    return NOERROR;
  }

  if (trig[0]->trig_mask & (0x8)){ // This is a pair spectrometer trigger
    return NOERROR;
  }

  // first get Paddle hits to fill 2d histograms
  vector <const DTOFPaddleHit*> PadHits;
  loop->Get(PadHits);

  // loop over paddle hits and dig down to get the raw ADC data that contribute
  // to the paddle hit and plot the hist position in the paddle vs. raw ADC integral.
  // this of course means hits on both ends are required, hence paddle hits!
  for (unsigned int k=0; k<PadHits.size(); k++){
    double xpos = PadHits[k]->pos;
    
    //get raw adc data for this hit
    vector <const DTOFHit*> TofHits;
    PadHits[k]->Get(TofHits);
    
    if (TofHits.size()<2){
      //cout<<eventnumber<<"  "<<TofHits.size()<<endl;
      continue;
    }
    vector <const DTOFDigiHit*> DigiHit;
    TofHits[0]->Get(DigiHit);
    double EL = DigiHit[0]->pulse_integral - (double)(DigiHit[0]->nsamples_integral)/
      (double)(DigiHit[0]->nsamples_pedestal)*(double)(DigiHit[0]->pedestal);
    
    DigiHit.clear();
    TofHits[1]->Get(DigiHit);
    double ER = DigiHit[0]->pulse_integral - (double)(DigiHit[0]->nsamples_integral)/
      (double)(DigiHit[0]->nsamples_pedestal)*(double)(DigiHit[0]->pedestal);

    // the above code assumed that the first DTOFDigiHit was left so if that is not 
    // true swtich (order is not guaranteed)!
    if (!DigiHit[0]->end){
      double tmp = EL;
      EL = ER;
      ER = tmp;
    }
    
    int pad1 = PadHits[k]->orientation*88 + PadHits[k]->bar - 1;
    adc_histPad[pad1]->Fill(EL, 126.-xpos);
    adc_histPad[pad1+44]->Fill(ER, 126.+xpos);
  }
  
  
  vector<const DChargedTrack*> locChargedTrack;
  loop->Get(locChargedTrack);
  DVector3 mypos(0.0,0.0,0.0);
  DVector3 proj_mom(0.0,0.0,0.0);
  Double_t zTOF = 616.;
  const DVector3 tof_origin(0.0,0.0,zTOF);
  const DVector3 tof_normal(0.0,0.0,1.0);
  

  // loop over charged tracks and select the charged track with
  // the best FOM. For these select if momenta larger than 0,75 GeV and
  // that they go forward to the TOF with particle mass close to pion
  for (unsigned int n=0; n < locChargedTrack.size() ; ++n){
    const DChargedTrack *ctrk = locChargedTrack[n];
    const DChargedTrackHypothesis *bestctrack = ctrk->Get_BestFOM();

    vector<const DTrackTimeBased*>loctrk;
    bestctrack->Get(loctrk);
    const DTrackTimeBased *trk = loctrk[0];

    // only look at forward tarcks
    double theta = trk->momentum().Theta()*180./3.14159;
    //double Q = trk->charge();
    double P = trk->momentum().Mag();
    double mass = trk->mass();
    if ( (theta>20.) || (P<0.75) || (mass>.2) || (mass<.1) ) 
      continue;
    
    double dEdxAll = trk->ddEdx_FDC + trk->ddEdx_CDC;
    double dEdxFDC = trk->ddEdx_FDC ;

    //cout<<dEdxFDC<<"   "<<dEdxAll<<endl;
    double s,t,dt;

    // swim this track to TOF plane
    if (trk->rt->GetIntersectionWithPlane(tof_origin,tof_normal,
					  mypos,proj_mom, &s, &t, &dt,SYS_TOF)==NOERROR){ 
      
      vector<const DTOFPoint *>locTOFPoints;
      bestctrack->Get(locTOFPoints);

      unsigned int Size = locTOFPoints.size();
      if ((locTOFPoints.size()>1) && (DEBUGX)){
	cout<<"++++ More than one TOF point for this track +++++ "<<endl;
	for (unsigned int i=0; i < Size ; ++i){
	  const DTOFPoint *TP = locTOFPoints[i];
	  cout<<i<<"  "<<TP->pos.X()<<"   " <<TP->pos.Y()<<"   " <<TP->pos.Z()<<"   " <<endl;
	  vector <const DTOFPaddleHit*> PHits;
	  TP->Get(PHits);
	  for (unsigned int k=0; k<PHits.size(); k++){
	    cout<<"        "<< PHits[k]->orientation<<"   " << PHits[k]->bar<<endl;
	  }
	}
	cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
      }


      // loop over tof points to find match between tof point and track
      // at the TOF plane.


      for (unsigned int i=0; i < Size ; ++i){
	const DTOFPoint *TP = locTOFPoints[i];

	// require both paddles with hits on both end!
	if ((TP->dHorizontalBarStatus>2) && (TP->dVerticalBarStatus>2)) {
	  
	  double dx = fabs(TP->pos.x() - mypos.x());
	  double dy = fabs(TP->pos.y() - mypos.y());

	  //cout<<" DELTA X/Y "<<endl;
	  //cout<<dx<<"   "<<dy<<endl;
	  

	  // require lose match between TOF point location and track location in x and y
	  // calculate for these matched tof points the mean time difference between the 
	  // two paddles generating the tof point and plot that mean time difference.
	  if ((dx<13.) && (dy<13.)) {

	    dedxtracksAll->Fill(trk->pmag(),dEdxAll);
	    dedxtracksFDC->Fill(trk->pmag(),dEdxFDC);

	    vector <const DTOFPaddleHit*> PHits;
	    TP->Get(PHits);
	    double mt1 = (PHits[0]->t_north + PHits[0]->t_south)/2.;
	    double mt2 = (PHits[1]->t_north + PHits[1]->t_south)/2.;
	    double DT = mt1-mt2;
	    int pad1 = PHits[0]->bar-1; // paddle in plane 0
	    int pad2 = PHits[1]->bar-1; // paddle in plane 1

	    // plot mean time difference for all paddles
	    MTdiff->Fill(DT);
	    // plot mean time difference separate for each paddle number
	    if  (PHits[1]->orientation){
	      mtDIFF0[pad1]->Fill(DT,pad2);
	    } else {
	      mtDIFF1[pad2]->Fill(-DT,pad1);
	    }

	    if (TRACKTEST) {
	      double norm = proj_mom.Mag();
	      DVector3 v; 
	      v.SetXYZ(proj_mom.x()/norm, proj_mom.y()/norm, proj_mom.z()/norm);
	      double s = v.Dot(tof_normal);
	      double angle = TMath::ACos(s)*180./3.1415926;
	      // now angle is the impact angle of the track onto the TOF plane
	      // loop over paddle hits histogram the energy left/right
	      // and also depending on the particle momentum for each paddle PMT separately
	      for (unsigned int k=0; k<PHits.size(); k++) {

		int Plane = PHits[k]->orientation;
		int Paddle = PHits[k]->bar - 1;
		// paddle in first plane hit position
		double x = -(PHits[k]->t_north - PHits[k]->t_south)/2.*speed[Plane][Paddle];

		E_hist[Plane*88+Paddle]->Fill(PHits[k]->E_north);
		E_hist[Plane*88+Paddle+44]->Fill(PHits[k]->E_south);
		pvsE_hist[Plane*88+Paddle]->Fill(PHits[k]->E_north, norm);
		pvsE_hist[Plane*88+Paddle+44]->Fill(PHits[k]->E_south, norm);
	      
		vector <const DTOFHit*> TofHits;
		PHits[k]->Get(TofHits);
		
		vector <const DTOFDigiHit*> DigiHit;
		TofHits[0]->Get(DigiHit);
		double EL = DigiHit[0]->pulse_integral - (double)(DigiHit[0]->nsamples_integral)/
		  (double)(DigiHit[0]->nsamples_pedestal)*(double)(DigiHit[0]->pedestal);

		double PeakL = (double) DigiHit[0]->pulse_peak; // Get Peak amplitude for Left PMT

		DigiHit.clear();
		TofHits[1]->Get(DigiHit);
		double ER = DigiHit[0]->pulse_integral - (double)(DigiHit[0]->nsamples_integral)/
		  (double)(DigiHit[0]->nsamples_pedestal)*(double)(DigiHit[0]->pedestal);

		double PeakR = (double) DigiHit[0]->pulse_peak; // Get Peak amplitude for Right PMT
		
		if (!DigiHit[0]->end){
		  double tmp = EL;
		  EL = ER;
		  ER = tmp;
		  
		  tmp = PeakL;
		  PeakL = PeakR;
		  PeakR = tmp;

		}
		//cout<<TofHits[1]->plane<<"   "<<TofHits[1]->bar<<"   "<<TofHits[1]->end<<endl<<endl;
		// plot the energy in terms of ADC integral values for different x hit positions
		adc_hist[Plane*88+Paddle]->Fill(EL, 126.-x);
		adc_hist[Plane*88+Paddle+44]->Fill(ER, 126.+x);

		adc_histP[Plane*88+Paddle]->Fill(PeakL, 126.-x);
		adc_histP[Plane*88+Paddle+44]->Fill(PeakR, 126.+x);

		adcPeakValue->Fill(Plane*88+Paddle,PeakL);
		adcPeakValue->Fill(Plane*88+Paddle+44,PeakR);

		// plot the impact angle of the track on the paddle
		impactangles[Plane][Paddle]->Fill(angle);

	      }

	    }
	  }
	} else if ((TP->dHorizontalBarStatus>2)) { // only horizontal bars have hit on both ends
	  // use these to find hit location in single ended bars in the other plane if present.
	  
	  double dx = fabs(TP->pos.x() - mypos.x());

	  if ((dx<13.) && (fabs(TP->pos.x())<6.)){ // central hit that means overlap with single ended paddle 
	    vector <const DTOFPaddleHit*> PHits;
	    TP->Get(PHits);
	    
	    for (int n=0;n<(int)PHits.size();n++){ // find single ended paddle
	      int paddle = PHits[n]->bar;
	      int plane = PHits[n]->orientation; // 0: vertical,  1: horizontal
	      if ( (!plane) && ( (paddle == 22) || (paddle == 23)) ) {
		int idx = paddle - 1;
		float hpos = 126.-TP->pos.y();
		if (TP->pos.y()<0.) {
		  idx += 44;
		  hpos = 126.+TP->pos.y();
		}
		
		vector <const DTOFHit*> TofHits;
		PHits[n]->Get(TofHits);
		
		vector <const DTOFDigiHit*> DigiHit;
		TofHits[0]->Get(DigiHit);
		double E = DigiHit[0]->pulse_integral - (double)(DigiHit[0]->nsamples_integral)/
		  (double)(DigiHit[0]->nsamples_pedestal)*(double)(DigiHit[0]->pedestal);

		double PeakL = (double) DigiHit[0]->pulse_peak; // Get Peak amplitude for Left PMT
		
		adc_hist[idx]->Fill(E, hpos);
		adc_histP[idx]->Fill(PeakL, hpos);
		adcPeakValue->Fill(idx,PeakL);

	      }
	    }
	  }
	} else if ((TP->dVerticalBarStatus>2)) { // only vertical bars have hit on both ends
	  // use these to find hit location in single ended bars if present.
	  
	  double dy = fabs(TP->pos.y() - mypos.y());

	  if ((dy<13.) && (fabs(TP->pos.y())<6.)){ // central hit ! overlap with single ended paddle 
	    vector <const DTOFPaddleHit*> PHits;
	    TP->Get(PHits);
	    
	    for (int n=0;n<(int)PHits.size();n++){ // find single ended paddle
	      int paddle = PHits[n]->bar;
	      int plane = PHits[n]->orientation; // 0: vertical,  1: horizontal
	      if ( (plane) && ( (paddle == 22) || (paddle == 23)) ) {
		int idx = 88 + paddle - 1;
		float hpos = 126. - TP->pos.x();
		if (TP->pos.x()<0.) {
		  idx += 44;
		  hpos = 126. + TP->pos.x();
		}
		
		vector <const DTOFHit*> TofHits;
		PHits[n]->Get(TofHits);
		
		vector <const DTOFDigiHit*> DigiHit;
		TofHits[0]->Get(DigiHit);
		double E = DigiHit[0]->pulse_integral - (double)(DigiHit[0]->nsamples_integral)/
		  (double)(DigiHit[0]->nsamples_pedestal)*(double)(DigiHit[0]->pedestal);
		
		double PeakL = (double) DigiHit[0]->pulse_peak; // Get Peak amplitude for Left PMT
		
		adc_hist[idx]->Fill(E, hpos);
		adc_histP[idx]->Fill(PeakL, hpos);
		adcPeakValue->Fill(idx,PeakL);

	      }
	    }
	  }
	}
	
      }
    } 
  }

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_TOF_test::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TOF_test::fini(void)
{
  // Called before program exit after event processing is finished.

  WriteRootFile();

  return NOERROR;
}

jerror_t JEventProcessor_TOF_test::WriteRootFile(void)
{
  // Called before program exit after event processing is finished.

  // ROOTFile = new TFile(ROOTFileName,"recreate");
  TDirectory *top = gDirectory;

  ROOTFile->cd();
  ROOTFile->cd("TOFtest");
  MTdiff->Write();
  dedxtracksAll->Write();
  dedxtracksFDC->Write();

  for (int k=0;k<44;k++) {
    mtDIFF0[k]->Write();
    mtDIFF1[k]->Write();
  }

  if (TRACKTEST) { 
    adcPeakValue->Write(); 
    for (int k=0;k<44;k++) {
      impactangles[0][k]->Write();
      impactangles[1][k]->Write();
    }
    for (int k=0;k<176;k++) {
      adc_hist[k]->Write();
      adc_histP[k]->Write();
      adc_histPad[k]->Write();
      E_hist[k]->Write();
      pvsE_hist[k]->Write();
    }
  }

  top->cd();

  return NOERROR;
}
