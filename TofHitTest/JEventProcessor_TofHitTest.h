// $Id$
//
//    File: JEventProcessor_TofHitTest.h
// Created: Tue Jul 11 07:43:17 EDT 2017
// Creator: zihlmann (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TofHitTest_
#define _JEventProcessor_TofHitTest_

#include <JANA/JEventProcessor.h>
using namespace std;
using namespace jana;

#include <TOF/DTOFHit.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>


class JEventProcessor_TofHitTest:public jana::JEventProcessor{
 public:
  JEventProcessor_TofHitTest();
  ~JEventProcessor_TofHitTest();
  const char* className(void){return "JEventProcessor_TofHitTest";}
  
  TFile *RF;

  TH2D *dt[176]; // ADC-TDC time difference vs. ADC amplitude (after corrections applied)
  TH1D *singles[2][88];
  TH1D *adct[2][88];
  TH1D *tdct[2][88];
 
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TofHitTest_

