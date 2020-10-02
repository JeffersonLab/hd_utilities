// $Id$
//
//    File: JEventProcessor_FCALLEDTree.cc
// Created: Fri May 19 12:39:24 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#include "JEventProcessor_FCALLEDTree.h"
using namespace jana;

#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"

#include "TTree.h"

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_FCALLEDTree());
  }
} // "C"


//------------------
// JEventProcessor_FCALLEDTree (Constructor)
//------------------
JEventProcessor_FCALLEDTree::JEventProcessor_FCALLEDTree()
{

}

//------------------
// ~JEventProcessor_FCALLEDTree (Destructor)
//------------------
JEventProcessor_FCALLEDTree::~JEventProcessor_FCALLEDTree()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCALLEDTree::init(void)
{
  // This is called once at program startup. 
  japp->RootWriteLock();

  m_tree = new TTree( "fcalBlockHits", "FCAL Block Hits" );

  m_tree->Branch( "nHits", &m_nHits, "nHits/I" );
  m_tree->Branch( "chan", m_chan, "chan[nHits]/I" );
  m_tree->Branch( "x", m_x, "x[nHits]/F" );
  m_tree->Branch( "y", m_y, "y[nHits]/F" );
  m_tree->Branch( "E", m_E, "E[nHits]/F" );
  m_tree->Branch( "t", m_t, "t[nHits]/F" );
  m_tree->Branch( "integ", m_integ, "integ[nHits]/F" );
  m_tree->Branch( "ped", m_ped, "ped[nHits]/F" );
  m_tree->Branch( "peak", m_peak, "peak[nHits]/F" );
  
  m_tree->Branch( "run", &m_run, "run/I" );
  m_tree->Branch( "event", &m_event, "event/L" );
  m_tree->Branch( "eTot", &m_eTot, "eTot/F" );

  japp->RootUnLock();
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCALLEDTree::brun(JEventLoop *eventLoop, int32_t runnumber)
{

  // this is not thread safe and may lead to an incorrect run number for
  // a few events in the worst case scenario -- I don't think it is a major problem
  m_run = runnumber;
  
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCALLEDTree::evnt(JEventLoop *loop, uint64_t eventnumber)
{
 
  vector< const DFCALHit* > hits;
  loop->Get( hits );

  if( hits.size() > kMaxHits ) return NOERROR;
  
  vector<const DFCALGeometry*> fcalGeomVect;
  loop->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1)
    return OBJECT_NOT_AVAILABLE;
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  
  japp->RootFillLock(this);

  m_event = eventnumber;
  
  m_nHits = 0;
  m_eTot = 0;
    
  for( vector< const DFCALHit* >::const_iterator hit = hits.begin();
       hit != hits.end();
       ++hit ){

    vector< const DFCALDigiHit* > digiHits;
    (**hit).Get( digiHits );
    if( digiHits.size() != 1 ) std::cout << "ERROR:  wrong size!! " << std::endl;
    
    const DFCALDigiHit& dHit = *(digiHits[0]);

    m_chan[m_nHits] = fcalGeom.channel( (**hit).row, (**hit).column );
    m_x[m_nHits] = (**hit).x;
    m_y[m_nHits] = (**hit).y;
    m_E[m_nHits] = (**hit).E;
    m_t[m_nHits] = (**hit).t;

    m_eTot += (**hit).E;

    m_ped[m_nHits] = (float)dHit.pedestal/dHit.nsamples_pedestal;
    m_peak[m_nHits] = dHit.pulse_peak - m_ped[m_nHits];
    m_integ[m_nHits] = dHit.pulse_integral -
       (m_ped[m_nHits]*dHit.nsamples_integral);
    
    ++m_nHits;
  }

  m_tree->Fill();
  
  japp->RootFillUnLock(this);

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCALLEDTree::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCALLEDTree::fini(void)
{
  // Called before program exit after event processing is finished.
  japp->RootWriteLock();
  m_tree->Write();
  japp->RootUnLock();

  return NOERROR;
}

