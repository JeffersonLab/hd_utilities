// $Id$
//
//    File: JEventProcessor_cppFMWPC.h
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_cppFMWPC_
#define _JEventProcessor_cppFMWPC_

#include <JANA/JEventProcessor.h>

#include <FMWPC/DFMWPCHit.h>
#include <FDC/DFDCHit.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

class JEventProcessor_cppFMWPC:public jana::JEventProcessor{
 public:
  JEventProcessor_cppFMWPC();
  ~JEventProcessor_cppFMWPC();
  const char* className(void){return "JEventProcessor_cppFMWPC";}

  int nFMWPCchambers;
  TH2D *FMWPCwiresT[8];
  TH2D *FMWPCwiresE[8];
  
  TH2D *FDCwiresT[24];

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_cppFMWPC_

