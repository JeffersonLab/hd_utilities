// $Id$
//
//    File: JEventProcessor_FCALLEDTree.h
// Created: Fri May 19 12:39:24 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#ifndef _JEventProcessor_FCALLEDTree_
#define _JEventProcessor_FCALLEDTree_

class TTree;

#include <JANA/JEventProcessor.h>

class JEventProcessor_FCALLEDTree:public jana::JEventProcessor{
 public:
  JEventProcessor_FCALLEDTree();
  ~JEventProcessor_FCALLEDTree();
  const char* className(void){return "JEventProcessor_FCALLEDTree";}

  enum { kMaxHits = 10000 };
  
 private:
  
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  TTree* m_tree;

  int m_nHits;

  int m_chan[kMaxHits];
  float m_x[kMaxHits];
  float m_y[kMaxHits];
  float m_E[kMaxHits];
  float m_t[kMaxHits];
  float m_integ[kMaxHits];
  float m_ped[kMaxHits];
  float m_peak[kMaxHits];

  int m_run;
  uint64_t m_event;
  float m_eTot;
  
};

#endif // _JEventProcessor_FCALLEDTree_

