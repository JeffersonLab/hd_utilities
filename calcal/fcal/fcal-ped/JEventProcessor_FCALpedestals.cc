#include "JEventProcessor_FCALpedestals.h"
#include <JANA/JApplication.h>
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"
#include "FCAL/DFCALCluster.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DVertex.h"
#include "DVector3.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include <TTree.h>
#include "DVector3.h"
#include "PID/DParticleID.h"
#include "TRIGGER/DTrigger.h"
#include "GlueX.h"
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <TDirectory.h>
#include <TH1I.h>
#include <TH2F.h>

using namespace jana;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

//FCAL has only 2800 active channels but and a bunch of inactive blocks. Redundancy in number of channels is to make sure there are enough histograms available to fill all active channels.
const int nChan = 2800;

// Define Histograms
//static TH1I* pedestal[nChan];
static TH2I* pedestal;

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_FCALpedestals());
  }
} // "C"


//------------------
// JEventProcessor_FCALpedestals (Constructor)
//------------------
JEventProcessor_FCALpedestals::JEventProcessor_FCALpedestals()
{

}

//------------------
// ~JEventProcessor_FCALpedestals (Destructor)
//------------------
JEventProcessor_FCALpedestals::~JEventProcessor_FCALpedestals()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCALpedestals::init(void)
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  //
  TDirectory *main = gDirectory;
  gDirectory->mkdir("FCAL_pedestals")->cd();
  
  
  //for (int i = 0; i < nChan; ++i) {
  pedestal = new TH2I("pedestal","Pedestal for Channel",2800,-0.5, 2799.5,500,90,120);
  //}

  main->cd();

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCALpedestals::brun(JEventLoop *eventLoop, 
					     int32_t runnumber)
{

  // get the FCAL z position from the global geometry interface
  DApplication *dapp = 
    dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  const DGeometry *geom = dapp->GetDGeometry(runnumber);
  if( geom ) {

    geom->GetFCALZ( m_FCALfront );
  }
  else{
      
    cerr << "No geometry accessbile." << endl;
    return RESOURCE_UNAVAILABLE;
  }

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCALpedestals::evnt(JEventLoop *eventLoop, 
					     uint64_t eventnumber)
{
  // select events with physics events, i.e., not LED and other front panel triggers
  const DTrigger* locTrigger = NULL; 
  eventLoop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) 
    return NOERROR;
  
 

  // we need an FCAL Geometry object
  //vector< const DFCALGeometry* > geomVec;
  //eventLoop->Get( geomVec );
    
  vector<const DFCALGeometry*> fcalGeomVect;
  eventLoop->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1)
    return OBJECT_NOT_AVAILABLE;
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  

  if( fcalGeomVect.size() != 1 ){
    
    cerr << "No geometry accessbile." << endl;
    return RESOURCE_UNAVAILABLE;
  }
  
  //const DFCALGeometry& fcalGeom = geomVec[0];

  vector< const DFCALDigiHit*  > digiHits;
  eventLoop->Get( digiHits );
   
	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  
  for( vector< const DFCALDigiHit* >::const_iterator dHitItr = digiHits.begin();
       dHitItr != digiHits.end(); ++dHitItr ){
    const DFCALDigiHit& dHit = (**dHitItr);

    m_r = dHit.row ;  
    m_c = dHit.column ;   
    if( !fcalGeom.isBlockActive( m_r, m_c ) ) continue;
    m_chan = fcalGeom.channel( dHit.row, dHit.column );
    m_pedestal = dHit.pedestal / dHit.nsamples_pedestal;

    if( m_pedestal > 0 ) {
      //pedestal[m_chan]->Fill(m_pedestal); 
      pedestal->Fill((double) m_chan, m_pedestal); 
    }
  }

 	japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCALpedestals::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCALpedestals::fini(void)
{

  // Called before program exit after event processing is finished.   
  return NOERROR;
}


