// $Id$
//
//    File: JEventProcessor_FCALpedestals.h
// Created: Fri Jan 30 08:18:41 EST 2015
// Creator: shepherd (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCALpedestals_
#define _JEventProcessor_FCALpedestals_

#include <JANA/JEventProcessor.h>
#include <FCAL/DFCALGeometry.h>

class DFCALHit;
class TTree;

class JEventProcessor_FCALpedestals:public jana::JEventProcessor{
 public:

 
  JEventProcessor_FCALpedestals();
  ~JEventProcessor_FCALpedestals();
  const char* className(void){return "JEventProcessor_FCALpedestals";}

 private:

  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

 

  double m_FCALfront;
  
 
  int m_r;
  int m_c;
  int m_chan;
  float m_pedestal;


 
};

#endif // _JEventProcessor_FCALpedestals_

