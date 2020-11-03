// $Id$
//
//    File: JEventProcessor_TAGGER1.cc
// Created: Fri Jul  5 08:22:20 EDT 2019
// Creator: zihlmann (on Linux ifarm1801 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_TAGGER1.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
  InitJANAPlugin(app);
  app->AddProcessor(new JEventProcessor_TAGGER1());
}
} // "C"


//------------------
// JEventProcessor_TAGGER1 (Constructor)
//------------------
JEventProcessor_TAGGER1::JEventProcessor_TAGGER1()
{
  
}

//------------------
// ~JEventProcessor_TAGGER1 (Destructor)
//------------------
JEventProcessor_TAGGER1::~JEventProcessor_TAGGER1()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_TAGGER1::init(void)
{

  // This is called once at program startup. 
  ThreadCounter = 0;
  useRF = 1;

  BINADC_2_TIME = 0.0625;


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TAGGER1::brun(JEventLoop *eventLoop, int32_t runnumber)
{

  if (ThreadCounter<1){
    TDirectory *dir1 = gDirectory;
    OUTF = new TFile("TAGGER1_ps_test.root","RECREATE");
    OUTF->cd();

    fcalrow22 = new TH1D("fcalrow22", "FCAL HITS row 22", 200, 0., 200.);
    fcalrow23 = new TH1D("fcalrow23", "FCAL HITS row 23", 200, 0., 200.);
    fcalrow26 = new TH1D("fcalrow26", "FCAL HITS row 26", 200, 0., 200.);
    
    tagmmult = new TH1D("tagmmult", "Tagger microscope hit multiplicity", 300, 0., 300.);
    taghmult = new TH1D("taghmult", "Tagger hodoscope hit multiplicity", 300, 0, 300.); 

    tagmmultC = new TH1D("tagmmultC", "Tagger microscope hits ADC+TDC, ADC", 3, 0., 3.);
    taghmultC = new TH1D("taghmultC", "Tagger hodoscope hits ADC+TDC, ADC", 3, 0, 3.); 
    
    HitCounts = new TH1D("HitCounts", "Tagger Hit Counter without and with TDC mic and hod", 4, 0., 4.);

    InTimeCounter[0] = new TH1D("InTimecounter0", 
				"Tagger Mic. Hits in time", 20, 0., 20.);
    InTimeCounter[1] = new TH1D("InTimecounter1", 
				"Tagger Hod. Hits in time", 20, 0., 20.);
    InTimeCounter[2] = new TH1D("InTimecounter2", 
				"Tagger Mic. Hits in time And Match PS Energy", 20, 0., 20.);
    InTimeCounter[3] = new TH1D("InTimecounter3", 
				"Tagger Hod. Hits in time And Match PS Energy", 20, 0., 20.);

    PStagm = new TH1D("PStagm", "PStime minus tagm time", 5000, -120., 100.);
    PStagh = new TH1D("PStagh", "PStime minus tagh time", 5000, -120., 100.);

    TagmTime = new TH1D("TagmTime", "tagm time", 5000, -120., 100.);
    TaghTime = new TH1D("TaghTime", "tagh time", 5000, -120., 100.);
    
    PStagm2d = new TH2D("PStagm2d", "Microscope Counter ID vs. tagm time - PStime", 1000, -120., 100., 120, 0., 120.);
    PStagh2d = new TH2D("PStagh2d", "Hodoscope Counter  ID vs. tagh time - PStime",  1000, -120., 100.,  350, 0., 350.);
    
    PStagmEnergyInTime = new TH1D("PStagmEnergyInTime", "PSEnergy - TaggerEnergy in time photons microscope", 2000, -4., 4.);
    PStagmEnergyOutOfTime = new TH1D("PStagmEnergyOutOfTime", "PSEnergy - TaggerEnergy out of time photons microscope", 2000, -4., 4.);
    PStaghEnergyInTime = new TH1D("PStaghEnergyInTime", "PSEnergy - TaggerEnergy in time photons hodoscope", 2000, -4., 4.);
    PStaghEnergyOutOfTime = new TH1D("PStaghEnergyOutOfTime", "PSEnergy - TaggerEnergy outo of time photons hodoscope", 2000, -4., 4.);
    
    PStagmEIT = new TH2D("PStagmEIT", "TAGM ID vs (PSEnergy - TaggerEnergy) in time photons", 2000, -4., 4., 120, 0, 120);
    PStagmEOOT = new TH2D("PStagmEOOT", "TAGM ID vs PSEnergy - (TaggerEnergy) out of time photons", 2000, -4., 4., 120, 0, 120);
    
    PStaghEIT = new TH2D("PStaghEIT", "TAGH ID vs (PSEnergy - TaggerEnergy) in time photons", 2000, -4., 4., 350, 0, 350);
    PStaghEOOT = new TH2D("PStaghEOOT", "TAGH ID vs PSEnergy - (TaggerEnergy) out of time photons", 2000, -4., 4., 350, 0, 350);
    
    outoftimeH   = new TH1D("outoftimeH",   "outoftime deltat tagger hits H", 10000, -100., 100.);
    outoftimeM   = new TH1D("outoftimeM",   "outoftime deltat tagger hits M ", 5000, -100., 100.);
    outoftimeMIX = new TH1D("outoftimeMIX", "outoftime deltat tagger hits MIX", 5000, -100., 100.);
    outoftimeHij  = new TH1D("outoftimeHij",  "outoftime deltat tagger hits Hij i=220 j=100", 5000, -100., 100.);
    
    correlationC = new TH2D("correlationC", "Tagger Counter correlations when in time", 350, 0, 350., 350, 0., 350.);
    correlationE = new TH1D("correlationE", "Tagger Counter Energy Sum when in time", 1000, 4., 13.);
    


    accidentalfenceH = new TH1D("accidentalfenceH", "Accidental Picket Fence with Ref time from Mic.",
				10000, -100., 100.);
    accidentalfenceM = new TH1D("accidentalfenceM", "Accidental Picket Fence with Ref time from Hod.",
				10000, -100., 100.);
    
    acc = new TH2D("acc", "Microscope Counter ID vs. acc time difference with bunch -11", 1000, -120., 100., 120, 0., 120.);
    fencePS = new TH1D("fencePS", "tagm time minus PS_time", 6000, -120., 120.);
    fenceRF = new TH1D("fenceRF", "tagm time minus RF_time", 6000, -120., 120.);
    RFminusPS1 = new TH1D("RFminusPS1", "RF_time - PS_time ONE only", 3000, -120., 120.);
    RFminusPS2 = new TH1D("RFminusPS2", "RF_time - PS_time One or more", 3000, -120., 120.);
    
    PSleft = new TH2D("PSleft", "PS time minus RF left", 500, -100., 100., 150, 0., 150.);
    PSright = new TH2D("PSright", "PS time minus RF right", 500, -100., 100., 150, 0., 150.);
    
    
    pstimeL = new TH2D("pstimeL", "PS ADC timing LEFT", 800, 0., 200., 200, 0., 200.);
    pstimeR = new TH2D("pstimeR", "PS ADC timing LEFT", 800, 0., 200., 200, 0., 200.);
    dir1->cd();
  }
  ThreadCounter++;


  // This is called whenever the run number changes
  cout<<"RunNumber = "<<runnumber<<endl;
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TAGGER1::evnt(JEventLoop *loop, uint64_t eventnumber)
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
 

  vector <const DL1Trigger*> trig;
  loop->Get(trig);

  
  if (trig.size() == 0){
    //cout<<"no trigger found"<<endl;
    return NOERROR;     
  }

  if (trig[0]->trig_mask < 1){ //
    //cout<<"no physics strigger"<<endl;
    return NOERROR;
  }

  //cout<<"physics strigger found: "<<trig[0]->trig_mask<<endl;

  vector <const DFCALHit*> fcalhits;
  loop->Get(fcalhits);
  for (int k=0; k<(int)fcalhits.size(); k++) {

     const DFCALHit* hit = fcalhits[k];
     if (hit->row == 22){
         fcalrow22->Fill((double)hit->column);
     }
  
     if (hit->row == 26){
         fcalrow26->Fill((double)hit->column);
     }
     if (hit->row == 23){
         fcalrow23->Fill((double)hit->column);
     }
  }


  if (trig[0]->trig_mask < 8){ //
    //cout<<"no PS strigger"<<endl;
    return NOERROR;
  }


  //cout<<"TRIGGER TYPE: 0x"<<hex<<trig[0]->trig_mask<<dec<<endl; // 0x8
  
  vector <const DBeamPhoton*> Beam;
  loop->Get(Beam);
  int NBeamPhotons = Beam.size();
  vector <int> FoundBeam;

  vector <const DPSPair*> PSPairs;
  loop->Get(PSPairs);

  if (PSPairs.size() != 1){  // Note most of the time it is 1, just to make it clean
    return NOERROR;
  }

  vector <const DTAGHHit*> taghHits;
  loop->Get(taghHits);
  vector <const DTAGMHit*> tagmHits;
  loop->Get(tagmHits);
  taghmult->Fill((double)taghHits.size());
  tagmmult->Fill((double)tagmHits.size());

  double b = 0;
  double s = 0;
  for (int n=0; n<(int)taghHits.size(); n++){
    const DTAGHHit *h = taghHits[n];
    if ((h->has_fADC) && (h->has_TDC)){
      b +=1.;
    } else if (h->has_fADC){
      s +=1;
    }
  }
  taghmultC->Fill(0.,s);
  taghmultC->Fill(1.,b);

  b = 0;
  s = 0;
  for (int n=0; n<(int)tagmHits.size(); n++){
    const DTAGMHit *h = tagmHits[n];
    if ((h->has_fADC) && (h->has_TDC)){
      b +=1.;
    } else if (h->has_fADC){
      s +=1;
    }
  }
  tagmmultC->Fill(0.,s);
  tagmmultC->Fill(1.,b);



  vector <const DRFTime*> RFTime;
  if (useRF) {
    loop->Get(RFTime);
  }


  // do some stats on PS hits
  vector <const DPSDigiHit*> PShits;
  loop->Get(PShits);
  for (int j=0; j<(int)PShits.size(); j++) {
    if (PShits[j]->arm) {
      pstimeR->Fill(PShits[j]->pulse_time*BINADC_2_TIME,  PShits[j]->column);
    } else {
      pstimeL->Fill(PShits[j]->pulse_time*BINADC_2_TIME,  PShits[j]->column);
    }
  }
  if (useRF) {
    PSleft->Fill(PSPairs[0]->left->t-RFTime[0]->dTime, PSPairs[0]->left->column);
    PSright->Fill(PSPairs[0]->right->t-RFTime[0]->dTime, PSPairs[0]->right->column);
  }



  vector <const DBeamPhoton*> OutOfTimeBeamM;
  vector <const DBeamPhoton*> OutOfTimeBeamH;

  vector <const DBeamPhoton*> OutOfTimeBeamPhotons;

  double Width = 4.008; // 249.5MHz correspondes to 4.008ns

  double tpair = (PSPairs[0]->ee.first->t + PSPairs[0]->ee.second->t) / 2.;
  double epair = (PSPairs[0]->ee.first->E + PSPairs[0]->ee.second->E);


  //
  // SELECTION CRITERIA for which accidental bunches to use, how many and now far 
  //                    away from the center
  //                    the (1) in the selection if statement can be replaced by
  //                    (delta>0) to only select bunches on the right side.
  //

  double NBBunches = 4.;
  double AddTShift = 0.;

  double CountWwO[4] = {0.,0.,0.,0.}; // count mic and hod hits with and without TDC hit (for prompts)


  // now here starts the actuall analysis for TAGGER hits
  // using the DBeamPhoton objecst to relate beam photons to tagger counters.
  // Note for all beam photons it is tested that the corresponding tagger hit
  // had both an ADC and a TDC hit
  //

  int InTimeBeamPhoton[2] = {0,0};
  int InTimeBeamPhotonAndGoodEnergy[2] = {0,0};

  for (int k=0; k<NBeamPhotons; k++){

    // first update lists for out of time beam photons separately
    // for tagger microscope and hodoscope hits
    float dt = Beam[k]->time() - tpair;
    if (TMath::Abs(dt) >  Width*3./2.) {      
      if (Beam[k]->dSystem == SYS_TAGM){
	OutOfTimeBeamM.push_back(Beam[k]);
      }else {
	OutOfTimeBeamH.push_back(Beam[k]);
      }
    }

    if (Beam[k]->dSystem == SYS_TAGM){
      TagmTime->Fill(Beam[k]->time());
    } else {
      TaghTime->Fill(Beam[k]->time());
    }

    // this part is used for Moller stuff only and is not relevant for the
    // tagger hit analysis
    if (TMath::Abs(dt)>Width*3./2.){
      OutOfTimeBeamPhotons.push_back(Beam[k]);
    } 

    // now do some analysis based on which tagger system hit
    // caused the beam photon
    if (Beam[k]->dSystem == SYS_TAGM){
      const DTAGMHit* h = NULL;
      Beam[k]->GetSingle(h);
      if (h) {
	if ( (h->has_fADC) and (h->has_TDC)){
	  CountWwO[1] += 1.; 
	} else if (h->has_fADC){
	  CountWwO[0] +=1 ;
	  continue; // do not consider tagger hits without TDC hit.
	}
      }

      double delta = Beam[k]->time() - tpair;
      PStagm->Fill(delta);
      PStagm2d->Fill(delta, Beam[k]->dCounter);
      
      if (TMath::Abs(delta)<Width/2.){
	double DeltaE = Beam[k]->momentum().Mag() - epair;
	PStagmEnergyInTime->Fill(-DeltaE);
	PStagmEIT->Fill(-DeltaE, Beam[k]->dCounter);

	InTimeBeamPhoton[0]++;
	if (TMath::Abs(DeltaE)<0.01){
	  InTimeBeamPhotonAndGoodEnergy[0]++;
	}
	
      } else if ( ( TMath::Abs(delta) >  Width*3./2. + AddTShift*Width ) && 
		  ( TMath::Abs(delta) <  Width*3./2. 
		    + AddTShift*Width + NBBunches*Width) &&  // +/- x beam bunches! 
		  (1) )  {  // can be used in case to select ONLY + side
	double DeltaE = Beam[k]->momentum().Mag() - epair;
	PStagmEnergyOutOfTime->Fill(-DeltaE);
	PStagmEOOT->Fill(-DeltaE, Beam[k]->dCounter);

      }

    } else { // now this is for Tagger hodoscope hits
      
      const DTAGHHit* h = NULL;
      Beam[k]->GetSingle(h);
      if (h) {
	if ( (h->has_fADC) and (h->has_TDC)){
	  CountWwO[3] += 1.; 
	} else if (h->has_fADC){
	  CountWwO[2] +=1 ;
	  continue; // do not consider tagger with without TDC data
	}
      }

      double delta = Beam[k]->time() - tpair;
      PStagh->Fill(delta);
      PStagh2d->Fill(delta, Beam[k]->dCounter);
      if (TMath::Abs(delta)<Width/2.){
	double DeltaE = Beam[k]->momentum().Mag() - epair;
	PStaghEnergyInTime->Fill(-DeltaE);
	PStaghEIT->Fill(-DeltaE, Beam[k]->dCounter);
	
	InTimeBeamPhoton[1]++;
	if (TMath::Abs(DeltaE)<0.01){
	  InTimeBeamPhotonAndGoodEnergy[1]++;
	}
	
      } else if( ( TMath::Abs(delta) >  Width*3./2. + AddTShift*Width ) && 
		 ( TMath::Abs(delta) <  Width*3./2. 
		   + AddTShift*Width + NBBunches*Width) &&  // +/- x beam bunches! 
		 (1) )  {  // ONLY use + side
	
	double DeltaE = Beam[k]->momentum().Mag() - epair;
	PStaghEnergyOutOfTime->Fill(-DeltaE);
	PStaghEOOT->Fill(-DeltaE, Beam[k]->dCounter);

      }
      
    }
  }

  for (int n=0;n<4;n++) {
    HitCounts->Fill((double)n, CountWwO[n]);
  }

  for (int n=0;n<2;n++) {
    InTimeCounter[n]->Fill((double)InTimeBeamPhoton[n]);
    InTimeCounter[n+2]->Fill((double)InTimeBeamPhotonAndGoodEnergy[n]);
  }
  
  //
  // Now look at hodoscope and microscope out of time hits
  // and try to analyse purely accidental hist
  //

  //loop over hoddoscope out of time hits
  const DBeamPhoton *HRef = NULL; // hodoscope beam photon reference
  for ( int n=0; n < ((int)OutOfTimeBeamH.size()-1); n++){

    const DBeamPhoton* b1 = OutOfTimeBeamH[n];
    double delta1 = b1->time()-tpair;

    if ( (TMath::Abs(delta1-5*Width)<Width) && (HRef == NULL)){
    //if ( ((b1->dCounter)==30) && (HRef == NULL)){
      HRef = b1;
    }

    if ((TMath::Abs(delta1 - 15*Width) > 5*Width + Width/2.) || (delta1<0)){
      continue;
    }

    // match with other hodoscope hits
    for ( int j=0; j<(int)OutOfTimeBeamH.size(); j++){
      if (j==n){
	continue;
      }

      const DBeamPhoton* b2 = OutOfTimeBeamH[j];
      
      if (TMath::Abs(b1->momentum().Mag() + b2->momentum().Mag() - 11.6)<1.){
	continue; // get rid of mollers
      }
      if (TMath::Abs((int)b1->dCounter - (int)b2->dCounter)<8){
	continue; // get rid of neibouring hits.
      }
      
      double deltat = b1->time() - b2->time();
      outoftimeH->Fill(deltat);
    }
  }
  
  // loop over microscope out of time hits

  const DBeamPhoton *MRef = NULL; // Microscope reference hit
  for (int n=0; n<((int)OutOfTimeBeamM.size()-1); n++){

    const DBeamPhoton* b1 = OutOfTimeBeamM[n];
    double delta1 = b1->time()-tpair;

    if ( (TMath::Abs(delta1-5*Width)<Width) && (MRef == NULL)){
    //if ( ((b1->dCounter)==30) && (MRef == NULL)){
      MRef = b1;
    }

    if ((TMath::Abs(delta1 - 15*Width) > 5*Width + Width/2.) || (delta1<0)){
      continue;
    }

    //match with other microscope out of time hits
    for (int j=0; j<(int)OutOfTimeBeamM.size(); j++){
      if (j==n){
	continue;
      }
      const DBeamPhoton* b2 = OutOfTimeBeamM[j];
 
     if  (TMath::Abs(b1->momentum().Mag() + b2->momentum().Mag() - 11.6) < 1.1){
       continue; // get rid of potential mollers
     }
     if (TMath::Abs((int)b1->dCounter - (int)b2->dCounter)<8){
       continue; // get rid of neibouring hits.
     }
     
     double deltat = b1->time() - b2->time();
     outoftimeM->Fill(deltat);
     
    }
  }

  // use reference tagger hit to create accidental hit pattern in the other tagger det.
  if (HRef){
    for (int n=0; n<((int)OutOfTimeBeamM.size()); n++){
      const DBeamPhoton* b1 = OutOfTimeBeamM[n];
      double dt = b1->time() - HRef->time();
      accidentalfenceM->Fill(dt);
    }
  }

  if (MRef){
    for (int n=0; n<((int)OutOfTimeBeamH.size()); n++){
      const DBeamPhoton* b1 = OutOfTimeBeamH[n];
      double dt = b1->time() - MRef->time();
      accidentalfenceH->Fill(dt);
    }
  }





  // look for potential Moller pairs
  for (int n=0; n<((int)OutOfTimeBeamPhotons.size()-1); n++){
    const DBeamPhoton* b1 = OutOfTimeBeamPhotons[n];
    for (int j=n+1; j<(int)OutOfTimeBeamPhotons.size(); j++){
      const DBeamPhoton* b2 = OutOfTimeBeamPhotons[j];
      if (TMath::Abs(b1->time()-b2->time()) < Width/2.){
	int idx1 = b1->dCounter;
	int idx2 = b2->dCounter;
	if (b1->dSystem == SYS_TAGM){
	  idx1+=130;
	} else {
	  if (idx1>130){
	    idx1 += 70;
	  }
	}
	if (b2->dSystem == SYS_TAGM){
	  idx2+=130;
	} else {
	  if (idx2>130){
	    idx2 += 70;
	  }
	}

	correlationC->Fill(idx1, idx2);
	double E = (11.6-b1->momentum().Mag()) + (11.6-b2->momentum().Mag());
	correlationE->Fill(E);
      }
    }
  }

  if (useRF) {
    if (PSPairs.size()== 1){ 
      RFminusPS1->Fill(RFTime[0]->dTime - tpair);
    }
    if (PSPairs.size() > 1){ 
      RFminusPS2->Fill(RFTime[0]->dTime - tpair);
    }
  }

  for (int k=0; k<NBeamPhotons; k++){
    const DBeamPhoton* b1 = Beam[k];
  
    if (b1->dSystem != SYS_TAGM){ // look only at microscope
      continue;
    }

    float dt1 = b1->time() - tpair;

    if (PSPairs.size()>0){ 
      fencePS->Fill(dt1);
    }

    fenceRF->Fill(b1->time() - RFTime[0]->dTime);

    if (TMath::Abs(dt1 - (15.*Width))> 6*Width+Width/2.){  // look only at accidentals in bunch +12 pm 7 bunches.
      continue;
    }
    
    for (int n=0; n<NBeamPhotons; n++){
      if (n == k){
	continue;
      }
      
      const DBeamPhoton* b2 = Beam[n];
      
      float dt2 = b2->time() - tpair;
      if (b2->dSystem != SYS_TAGM){ // look only at microscope
	continue;
      }
      if (TMath::Abs(dt2)<Width/2.){  // look only at accidentals 
	continue;
      }
      if (TMath::Abs((int)b1->dCounter - (int)b2->dCounter) < 10 ){
	continue;
      }
      if (TMath::Abs((int)b1->momentum().Mag() + (int)b2->momentum().Mag() - 11.6) < 0.2 ){
	continue;
      }

      float DT = dt1-dt2;
      acc->Fill(DT, b1->dCounter);

    }

  }  
  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_TAGGER1::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.

  ThreadCounter--;
  
  if (ThreadCounter<1) {
    TDirectory *dir1 = gDirectory;
    OUTF->cd();
    tagmmult->Write();
    taghmult->Write();  
    tagmmultC->Write();
    taghmultC->Write();  
    HitCounts->Write();
    InTimeCounter[0]->Write();
    InTimeCounter[1]->Write();
    InTimeCounter[2]->Write();
    InTimeCounter[3]->Write();
    fcalrow22->Write();
    fcalrow23->Write();
    fcalrow26->Write();
    TagmTime->Write();
    TaghTime->Write();
    PStagm->Write();
    PStagh->Write();
    PStagm2d->Write();
    PStagh2d->Write();
    PStagmEnergyInTime->Write();
    PStagmEnergyOutOfTime->Write();
    PStaghEnergyInTime->Write();
    PStaghEnergyOutOfTime->Write();
    outoftimeH->Write();
    outoftimeM->Write();
    outoftimeMIX->Write();
    outoftimeHij->Write();

    accidentalfenceM->Write();
    accidentalfenceH->Write();
    
    PStagmEIT->Write();
    PStagmEOOT->Write();
    PStaghEIT->Write();
    PStaghEOOT->Write();
    
    correlationC->Write();
    correlationE->Write();
    
    acc->Write();
    fencePS->Write();
    fenceRF->Write();
    RFminusPS1->Write();
    RFminusPS2->Write();
    
    PSleft->Write();
    PSright->Write();
    
    pstimeL->Write();
    pstimeR->Write();
    
    OUTF->Close();
    
    dir1->cd();
  }
  
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TAGGER1::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}

