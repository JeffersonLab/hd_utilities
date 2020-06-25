// $Id$
//
//    File: JEventProcessor_TOFmon.cc
// Created: Wed Sep 18 09:33:05 EDT 2019
// Creator: zihlmann (on Linux gluon48.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#include "JEventProcessor_TOFmon.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_TOFmon());
  }
} // "C"


//------------------
// JEventProcessor_TOFmon (Constructor)
//------------------
JEventProcessor_TOFmon::JEventProcessor_TOFmon()
{
  
}

//------------------
// ~JEventProcessor_TOFmon (Destructor)
//------------------
JEventProcessor_TOFmon::~JEventProcessor_TOFmon()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_TOFmon::init(void)
{
  // This is called once at program startup. 
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TOFmon::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes

  //
  // read in geometry information to get total number of PMTS
  //
  vector<const DTOFGeometry*> tofGeomVect;
  eventLoop->Get( tofGeomVect );
  if(tofGeomVect.size()<1)  return OBJECT_NOT_AVAILABLE;
  const DTOFGeometry& tofGeom = *(tofGeomVect[0]);
  
  TOF_NUM_PLANES = tofGeom.Get_NPlanes();
  TOF_NUM_BARS = tofGeom.Get_NBars();
  TOF_NUM_SHORT_BARS = tofGeom.Get_NShortBars();

  NPMTS = TOF_NUM_BARS*4; 


  cout<<"RUN: "<<runnumber<<endl;
  cout<<"Number of Bars/plane   "<<TOF_NUM_BARS<<endl;
  cout<<"Number of short ones   "<<TOF_NUM_SHORT_BARS<<endl;
  cout<<"Number of PMTs (Total) "<<NPMTS<<endl;

  

  //
  // create root histograms
  //
  sprintf(RootFile,"tofmon.root");
  OUTF = new TFile(RootFile,"RECREATE");
  OUTF->cd();
  OUTF->mkdir("TOFmon")->cd();

  pedestals = new TH2F("pedestals", "TOF pedestals", NPMTS, 0., (float)NPMTS, 350, 0., 350.);
  amplitude = new TH2F("amplitude", "TOF signal amplitude", NPMTS, 0., (float)NPMTS, 
		       1000, 0., 4096.);
  amplitudeMatch = new TH2F("amplitudeMatch", "TOF signal amplitude with TDC Hit", 
			    NPMTS, 0., (float)NPMTS, 1000, 0., 4096.);
  integrals = new TH2F("integrals", "TOF signal integrals", 
		       NPMTS, 0., (float)NPMTS, 1000, 0., 30000.);
  
  ADCtime = new TH2F("ADCtime", "Signal timing ADC",1000, 0., 800., 200, 0., 200.);
  TDCtime = new TH2F("TDCtime", "Signal timing TDC",1000, 0., 5000., 200, 0., 200.);
  TDCtimeMatch = new TH2F("TDCtimeMatch", "Signal timing TDC with ADC Match", 
			  1000, 0., 5000., NPMTS, 0., (float)NPMTS);
  TDChits = new TH1F("TDChits", "TDC hits",NPMTS, 0., (float)NPMTS);

  char hnam[128];
  char htit[128];
  for (int k=0;k<TOF_NUM_BARS;k++){  
    sprintf(hnam,"MTDiff%02d",k);
    sprintf(htit,"MTDiff paddle %d of plane 1 with all others of plane2",k);
    MTDiff[k] = new TH2F(hnam, htit, 600, -15., 15., TOF_NUM_BARS, 0., (float)TOF_NUM_BARS);
  }
  
  ADC2time = 0.0625;
  TDC2time = 0.0234375;

  //TDCtOffset = 1.24043e+03;
  //ADCtOffset = 3.01637e+02;
  TDCtOffset = 420.;
  ADCtOffset = 155.;

  TimingCut = 60.;

  for (int k=0;k<NPMTS;k++){
    sprintf(hnam,"TWalk%03d",k);
    sprintf(htit,"t_TDC - T_ADC vs amplitude");
    TWalk[k] = new TH2F(hnam, htit, 1000, 0., 4096., 1000, -50., 50.);    
  }

  for (int k=0;k<TOF_NUM_BARS*2;k++){
    sprintf(hnam,"TDiffRaw%02d",k);
    sprintf(htit,"Time difference (TDC) RAW");
    DTRaw[k] = new TH1F(hnam,htit, 100, -30., 30.);    

    sprintf(hnam,"AMPvsDTRaw0%02d",k);
    sprintf(htit,"ADC Amplitude (ped subtr.) vs ADC DTRaw [ns] plane 0, PMT %d",k);
    AMPvsDTRaw[0][k] = new TH2F(hnam, htit,200, -20,20., 1000, 0., 4096.);
    sprintf(hnam,"AMPvsDTRaw1%02d",k);
    sprintf(htit,"ADC Amplitude (ped subtr.) vs ADC DTRaw [ns] plane 1, PMT %d",k);
    AMPvsDTRaw[1][k] = new TH2F(hnam, htit,200, -20,20., 1000, 0., 4096.);


    sprintf(hnam,"INTvsDTRaw0%02d",k);
    sprintf(htit,"ADC Integral (ped subtr.) vs ADC DTRaw [ns] plane 0, PMT %d",k);
    INTvsDTRaw[0][k] = new TH2F(hnam, htit,200, -20,20., 600, 0., 20000.);
    sprintf(hnam,"INTvsDTRaw1%02d",k);
    sprintf(htit,"ADC Integral (ped subtr.) vs ADC DTRaw [ns] plane 1, PMT %d",k);
    INTvsDTRaw[1][k] = new TH2F(hnam, htit,200, -20,20., 600, 0., 20000.);


  }
  OUTF->cd();
  OUTF->mkdir("INL")->cd();
  
  for (int k=0;k<6;k++){
    int SLOT = k+3;
    for (int n=0; n<32; n++){
      sprintf(hnam,"Intensities%d%02d",SLOT,n);
      sprintf(htit,"Intensity Distribution SLOT %d, Channel: %02d",SLOT,n);
      Intensities[k][n] = new TH1F(hnam,htit,1024,0.,1024.);

      sprintf(hnam,"INL%d%02d",SLOT,n);
      sprintf(htit,"Integral Non Linearity SLOT %d, Channel: %02d",SLOT,n);
      INL[k][n] = new TH1F(hnam,htit,1024,0.,1024.);
    }   
 
  }
  OUTF->cd();
  OUTF->mkdir("EVST")->cd();
  for (int k=0;k<TOF_NUM_BARS*4;k++){
    sprintf(hnam,"ADCvsTDC%03d",k);
    sprintf(htit,"ADC vs. TDC ");
    ADCvsTDC[k] = new TH2F(hnam,htit, 500, -500., 500., 200, 0., 4000.);    
  }


  OUTF->cd();
  
  NOBEAM = 0;
  BEAM = 0;
  CheckBeam = 1;
  BeamCurrent = 10.;

  cout<<endl<<"Check if Beam is on! ONLY LOOK AT BEAM ON DATA!"<<endl;

  map<string,double> tdcshift;
  if (!eventLoop->GetCalib("/TOF/tdc_shift", tdcshift)){
    TOF_TDC_SHIFT = tdcshift["TOF_TDC_SHIFT"];
  }
  cout<<endl<<endl<<"*************************"<<endl;
  cout<<            "the run number is "<<runnumber<<endl;
  cout<<            "*************************"<<endl;

  TIMINGCUT = 60.;

  cout<<"Timing offsets: "<<TDCtOffset <<" / "<<ADCtOffset<<endl;


  //
  // if walk parameter fit results exist read them in
  //

  char fnam[128];
  sprintf(fnam,"walk_fitresults_run%06d.dat",runnumber);
  struct stat buffer;
  if ((stat(fnam, &buffer) == 0)) {
    ifstream inf;
    inf.open(fnam);
    for (int k=0;k<NPMTS;k++){
      inf>>WalkPar[k][0]>>WalkPar[k][1]>>WalkPar[k][2];
    }
    inf.close();
  } else {
    cout<<"TDC Walk Parameter NOT found initialize to zero!"<<endl;
    for (int k=0;k<NPMTS;k++){
      WalkPar[k][0] = 0;
      WalkPar[k][1] = 0;
      WalkPar[k][2] = 0;
    }
  }


  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TOFmon::evnt(JEventLoop *loop, uint64_t eventnumber)
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
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);
  

  // check 
  vector<const DEPICSvalue*> epicsvalues;
  loop->Get(epicsvalues); 
   
  if (CheckBeam) {
    
    bool isEPICS = loop->GetJEvent().GetStatusBit(kSTATUS_EPICS_EVENT);
    if (isEPICS) {
      for(vector<const DEPICSvalue*>::const_iterator val_itr = epicsvalues.begin();
          val_itr != epicsvalues.end(); val_itr++) {
        const DEPICSvalue* epics_val = *val_itr;
        if (epics_val->name == "IBCAD00CRCUR6") {
          float fconv = atof(epics_val->sval.c_str());
          if (fconv<BeamCurrent) {
            NOBEAM = 1;
            BEAM = 0;
          } else {
            NOBEAM = 0;
            BEAM = 1;
          }
        }
      }
      // this is epics data so not further action needed here
      return NOERROR;
    }
  }

  if (NOBEAM){
    return NOERROR; // only look a data with BEAM  
  }

  // check trigger type

  vector <const DL1Trigger*> Trig;
  loop->Get(Trig);
  if (Trig.size()<1){
    return NOERROR;
  }
  if (!(Trig[0]->trig_mask & 0x7)){ // check if any of the first 3 Trigger bits is set
    return NOERROR;
  }


  vector< const DCAEN1290TDCHit*> CAENHits;
  loop->Get(CAENHits);
  if (CAENHits.size()<=0){
    return NOERROR;  // done here if no TOF TDC data available
  }
  TDChits->Fill((float)CAENHits.size());


  uint32_t locROCID = CAENHits[0]->rocid;
  vector <const DCODAROCInfo*> ROCS;
  loop->Get(ROCS);
  int indx = -1;
  for ( unsigned int n=0; n<ROCS.size(); n++) {
    if (ROCS[n]->rocid == locROCID){
      indx = n;
      break;
    }
  }

  if (indx<0){
    cout<<"ERROR: NO ROCID 1 FOUND!!!"<<endl;
    return NOERROR;
  }
  //
  // get timing shift for 6 fold ambiguity
  //
  uint64_t TriggerTime = ROCS[indx]->timestamp;
  int TriggerBIT = TriggerTime % ((uint64_t)6);
  float TimingShift = TOF_TDC_SHIFT - (float)TriggerBIT;
  int TShift = TimingShift;
  if(TShift <= 0) {
    TShift += 6;
  }
  TimingShift = 4. * (float)TShift ;

  // 
  // count hit intensity for integral non linearity test
  // there are 6 CAEN VX1290 TDCs in Slots 3 to 8
  // this section is not needed for the TOF ADC calibration
  // but for testing and monitoring INL correction in the TDCs
  //
  for (unsigned int j=0; j<CAENHits.size(); j++){
    const DCAEN1290TDCHit *hit = CAENHits[j];
    int S = hit->slot-3;
    int C = hit->channel;
    int bin = hit->time & 0x3FF;

    if (S>5){ // this should never be true
      continue;
    }

    if (S<0){ // this should ALSO neve never ever  be true
      cout<<endl<<"ERROR: CRATE SLOT NUMBER OUT OF RANGE!"<<endl;
      cout<<"Event Number "<<eventnumber<<endl<<endl;
      return NOERROR;
    }

    float time = ((float)hit->time)*TDC2time;
    float t = time - TDCtOffset;
    if (fabs(t)>TIMINGCUT+20.) { // for INL look at random hits not correlated with trigger
      Intensities[S][C]->Fill((float)bin);
    }    
  }

  vector <const DTOFTDCDigiHit*> TDCHits;
  vector <const DTOFDigiHit*> ADCHits;

  /*
  float MeanTime[2][50];
  int MTHit[2][50];
  memset(MTHit,0,4*2*50);
  */

  loop->Get(TDCHits);
  loop->Get(ADCHits);


  vector <const DTOFDigiHit*> ADCHitsLeft[2], ADCHitsRight[2];
  //  vector <const DTOFTDCDigiHit*> TDCHitsLeft[2], TDCHitsRight[2];

  for (unsigned int k=0; k<ADCHits.size(); k++){
    
    const DTOFDigiHit *hit = ADCHits[k];
    
    int plane = hit->plane;
    int bar = hit->bar-1;
    int side = hit->end;
    int idx = plane*TOF_NUM_BARS*2 + bar + side*TOF_NUM_BARS ;
    
    int nsamples_integral = (int)hit->nsamples_integral;
    int nsamples_pedestal = (int)hit->nsamples_pedestal;
    float ped = hit->pedestal/(float)nsamples_pedestal;
    /*
    if (idx == 63){
      cout<< idx<<"    "<< ped<<"  "<<hit->pulse_peak<<"     "<<hit->pulse_integral<<endl;
      cout<<"         "<<hex<<"0x"<<(hit->pulse_time & 0xF0)<<dec<<endl;
    }
    */
    float integr = (float)hit->pulse_integral - (float)nsamples_integral*ped;
    float max = 0;
    max = (float)hit->pulse_peak - ped;
    
    amplitude->Fill((float)idx, max);
    integrals->Fill((float)idx, integr);
    pedestals->Fill((float)idx, ped);
    
    ADCtime->Fill(hit->pulse_time*ADC2time, (float)idx);

    if (TMath::Abs((double)hit->pulse_time*ADC2time - ADCtOffset)<60.){
      if (hit->end){
	ADCHitsRight[hit->plane].push_back(hit);
      } else {
	ADCHitsLeft[hit->plane].push_back(hit);
      }
    }

    
    // these are the single ended bars
    if (TMath::Abs((double)hit->bar - (double)(TOF_NUM_BARS/2.) - 0.5) < (double)TOF_NUM_SHORT_BARS/4.) {
      
      AMPvsDTRaw[hit->plane][bar + side*TOF_NUM_BARS]->Fill(0., max);
      INTvsDTRaw[hit->plane][bar + side*TOF_NUM_BARS]->Fill(0., integr);
      
    } else { // these are the double ended bars
      
      //find matching ADC hit on other side
      for (unsigned int j=k+1; j<ADCHits.size(); j++){	
	const DTOFDigiHit *hit2 = ADCHits[j];
	
	if ( (hit2->plane == hit->plane) &&
	     (hit2->bar == hit->bar)   && 
	     (hit2->end != hit->end) ){
	  
	  // found match calculate time difference
	  int tdiff = hit2->pulse_time - hit->pulse_time;
	  if (hit->end){
	    tdiff = -tdiff;
	  }
	  float dt = (float)tdiff * ADC2time;
	  
	  //int idx2 = hit2->plane*TOF_NUM_BARS*2 + hit2->bar-1 + hit2->end*TOF_NUM_BARS ;
	  
	  int nsamples_integral2 = (int)hit2->nsamples_integral;
	  float nsamples_pedestal2 = (float)hit2->nsamples_pedestal;
	  float ped2 = (float)hit2->pedestal / nsamples_pedestal2;
	  
	  float integr2 = (float)hit2->pulse_integral - (float)nsamples_integral2*ped2;
	  float max2 = 0;
	  max2 = (float)hit2->pulse_peak - ped2;
	  
	  AMPvsDTRaw[hit->plane][hit->bar-1 + hit->end*TOF_NUM_BARS]->Fill(dt, max);
	  AMPvsDTRaw[hit2->plane][hit2->bar-1 + hit2->end*TOF_NUM_BARS]->Fill(dt, max2);

	  INTvsDTRaw[hit->plane][hit->bar-1 + hit->end*TOF_NUM_BARS]->Fill(dt, integr);
	  INTvsDTRaw[hit2->plane][hit2->bar-1 + hit2->end*TOF_NUM_BARS]->Fill(dt, integr2);
	  
	} 
	
      }
      
    }
  }
    

  for (unsigned int j=0; j<TDCHits.size(); j++){  // first loop over TDC hits
    const DTOFTDCDigiHit *hit = TDCHits[j];
    int plane = hit->plane;
    int bar = hit->bar-1;
    int side = hit->end;
    int idx = plane*TOF_NUM_BARS*2 + bar + side*TOF_NUM_BARS ;
    TDCtime->Fill((float)hit->time*TDC2time, (float)idx);

    //find matching ADC hit
    for (unsigned int k=0; k<ADCHits.size(); k++){
      
      const DTOFDigiHit *ADChit = ADCHits[k];
      
      if ( (ADChit->plane == plane) &&
	   (ADChit->bar-1 == bar) &&
	   (ADChit->end == side) ){ 

	// the offset must be determined in the histograms ADCtime and TDCtime
	// TDC TimingShift is due to the 6 fold clock ambiguity.
	float TADC = (float)ADChit->pulse_time * ADC2time - ADCtOffset;
	float TTDC = (float)hit->time * TDC2time;

	if ((TMath::Abs(TADC)>TimingCut)){
	  continue;
	}

	TDCtimeMatch->Fill( TTDC, (float)idx);
	TTDC -= TDCtOffset;
	//cout<<TTDC<<endl;
	if (TMath::Abs(TTDC)>TimingCut) {
	  continue;
	}

	TTDC += TimingShift;

	float DTT = TTDC-TADC;
	//cout<<DTT<<endl;
	if (TMath::Abs(DTT)<150.){ // this is a match
	  
	  float nsamples_pedestal = (float)ADChit->nsamples_pedestal;
	  float ped = (float)ADChit->pedestal / nsamples_pedestal;
	  float max = 0;
	  max = (float)ADChit->pulse_peak - ped;
	  
	  TWalk[idx]->Fill(max, DTT);
	  amplitudeMatch->Fill((float)idx, max);

	}
	
      }
      
    }
    if (TMath::Abs((double)hit->bar - (double)(TOF_NUM_BARS/2.) - 0.5) < (double)TOF_NUM_SHORT_BARS/4.) {
      continue;
    } else {
      for (unsigned int k=j+1; k<TDCHits.size(); k++){
	// fing matching TDC Hit from other end
	const DTOFTDCDigiHit *hit2 = TDCHits[k];
	if ((hit2->plane == plane) && 
	    (hit2->bar == hit->bar) &&
	    (hit2->end != hit->end)){

	  float tdiff = (float)(hit->time - hit2->time) * TDC2time;
	  if (hit2->end>0){
	    tdiff = -tdiff;
	  }
	    
	  DTRaw[hit->plane*TOF_NUM_BARS + hit->bar-1]->Fill(tdiff);	  
	}
      }
    }
 
  }

  vector <const DTOFHit*> TOFHits;
  loop->Get(TOFHits);

  for (int k=0;k<(int)TOFHits.size(); k++){

    const DTOFHit *hit = TOFHits[k];
    int id = hit->plane*TOF_NUM_BARS*2 + hit->end*TOF_NUM_BARS + hit->bar - 1;
    ADCvsTDC[id]->Fill(hit->t_TDC, hit->Amp);
  }


  return NOERROR;
}


//------------------
// erun
//------------------
jerror_t JEventProcessor_TOFmon::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TOFmon::fini(void)
{
  // Called before program exit after event processing is finished.
  
  OUTF->cd();
  OUTF->cd("TOFmon");
  
  pedestals->Write();
  amplitude->Write();

  amplitudeMatch->Write();

  integrals->Write();
  
  ADCtime->Write();
  TDCtime->Write();
  TDCtimeMatch->Write();

  TDChits->Write();

  for (int k=0;k<TOF_NUM_BARS;k++){  
    MTDiff[k]->Write();
  }

  for (int k=0;k<NPMTS;k++){
    TWalk[k]->Write();
  }

  for (int k=0;k<TOF_NUM_BARS*2;k++){
    DTRaw[k]->Write();
    AMPvsDTRaw[0][k]->Write();
    AMPvsDTRaw[1][k]->Write();
    INTvsDTRaw[0][k]->Write();
    INTvsDTRaw[1][k]->Write();
  }

  OUTF->cd();
  OUTF->cd("INL");
  for (int k=0;k<6;k++){
    for (int n=0; n<32; n++){
      Intensities[k][n]->Write();
      double I = Intensities[k][n]->Integral(1,1024);
      if (I>0.){	
        float sum = 0.;
        float RunningInt[1024];
        for (int j=0; j<1024; j++){
          RunningInt[j] = sum; // this makes sure the first entry is zero!
          sum += Intensities[k][n]->GetBinContent(j+1);
        }
        for (int j=0; j<1024; j++){
          float kp = RunningInt[j]/I*1024.;
          float cor = ((float)j) - kp;
          INL[k][n]->Fill( ((float)j), cor);
        }
      }      
      INL[k][n]->Write();
    }   
  }
  OUTF->cd();
  OUTF->cd("EVST");
  for (int k=0;k<TOF_NUM_BARS*2;k++){
    ADCvsTDC[k]->Write();
  }
 
  OUTF->cd();

  OUTF->Close();
  
  return NOERROR;
}

