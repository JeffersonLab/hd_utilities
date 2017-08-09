// $Id$
//
//    File: JEventProcessor_TOF_test.h
// Created: Wed May 27 09:41:19 EDT 2015
// Creator: zihlmann (on Linux gluon47.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TOF_test_
#define _JEventProcessor_TOF_test_

#include <JANA/JEventProcessor.h>

#include <PID/DChargedTrack.h>
#include <PID/DChargedTrackHypothesis.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TOF/DTOFPoint.h>
#include <TOF/DTOFPaddleHit.h>
#include <TOF/DTOFHit.h>
#include <TOF/DTOFDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <DAQ/Df250PulsePedestal.h>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

#include "GlueX.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace jana;

class JEventProcessor_TOF_test:public jana::JEventProcessor{
 public:
  JEventProcessor_TOF_test();
  ~JEventProcessor_TOF_test();
  const char* className(void){return "JEventProcessor_TOF_test";}
  
  int DEBUGX;
  int RunNumber;
  char ROOTFileName[128];
  TFile *ROOTFile;
  TH1D *MTdiff;

  TH2D *mtDIFF0[44];
  TH2D *mtDIFF1[44];
  TH1D *E_hist[176] ;
  TH2D *pvsE_hist[176] ;
  TH2D *adc_hist[176] ;
  TH2D *adc_histP[176] ;
  TH2D *adc_histPad[176] ;
  TH2D *adcPeakValue;

  TH1D *impactangles[2][44];
  int TRACKTEST;
  double speed[2][45];
  TH2D *dedxtracksAll;
  TH2D *dedxtracksFDC;


 private:
  jerror_t WriteRootFile(void);						
  jerror_t init(void);						
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
  jerror_t erun(void);	
  jerror_t fini(void);
};

#endif // _JEventProcessor_TOF_test_

