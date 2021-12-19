// $Id$
//
//    File: JEventProcessor_TAGGER1.h
// Created: Fri Jul  5 08:22:20 EDT 2019
// Creator: zihlmann (on Linux ifarm1801 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGGER1_
#define _JEventProcessor_TAGGER1_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>

#include <TRIGGER/DL1Trigger.h>
#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>

#include <RF/DRFTime.h>

#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGHDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCTDCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSPair.h>

#include <DAQ/DCODAROCInfo.h>
#include <TAGGER/DTAGHTDCDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>
#include <FCAL/DFCALHit.h>

#include <RF/DRFTime.h>

using namespace std;
using namespace jana;




class JEventProcessor_TAGGER1:public jana::JEventProcessor{
 public:
  JEventProcessor_TAGGER1();
  ~JEventProcessor_TAGGER1();
  const char* className(void){return "JEventProcessor_TAGGER1";}

  int ThreadCounter;

  int useRF;
  double BINADC_2_TIME;
  TFile *OUTF;
  TH1D *tagmmult, *taghmult;
  TH1D *tagmmultC, *taghmultC;
  TH1D *HitCounts;
  TH1D *InTimeCounter[4];
  TH1D *fcalrow22, *fcalrow23, *fcalrow26;
  TH1D *PStagm;
  TH1D *PStagh;
  TH1D *TagmTime;
  TH1D *TaghTime;

  TH2D *PStagm2d;
  TH2D *PStagh2d;

  TH1D *PStagmEnergyInTime;
  TH1D *PStagmEnergyOutOfTime;
  TH1D *PStaghEnergyInTime;
  TH1D *PStaghEnergyOutOfTime;
  TH1D *outoftimeM;
  TH1D *outoftimeH;
  TH1D *outoftimeMIX;
  TH1D *outoftimeHij;

  TH1D *accidentalfenceM;
  TH1D *accidentalfenceH;
  
  TH2D *PStagmEIT;
  TH2D *PStagmEOOT;
  TH2D *PStaghEIT;
  TH2D *PStaghEOOT;
  
  TH2D *correlationC;
  TH1D *correlationE;
  
  TH2D *acc;
  TH1D *fencePS;
  TH1D *fenceRF;
  TH1D *RFminusPS1;
  TH1D *RFminusPS2;

  TH2D *PSleft;
  TH2D *PSright;

  TH2D *pstimeL;
  TH2D *pstimeR;

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TAGGER1_

