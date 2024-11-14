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
#include "TRIGGER/DL1Trigger.h"
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
  m_LED_min1 = 7.16453;
  m_LED_min2 = 13.211;
  m_LED_min3 = 16.1517;
  m_LED_min4 = 24.25;
  m_LED_min5 = 30.186;
  m_LED_max1 = 9.76453;
  m_LED_max2 = 15.811;
  m_LED_max3 = 18.7517;
  m_LED_max4 = 26.85;
  m_LED_max5 = 32.786;
  
  m_do_tree = false;
  gPARMS->SetDefaultParameter( "FCAL-LED:do_tree", m_do_tree);
  m_is_fcal_led_skim = true;
  gPARMS->SetDefaultParameter( "FCAL-LED:is_rawdata", m_is_fcal_led_skim);

  gPARMS->SetDefaultParameter( "FCAL-LED:LED_min1", m_LED_min1);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_min2", m_LED_min2);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_min3", m_LED_min3);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_min4", m_LED_min4);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_min5", m_LED_min5);

  gPARMS->SetDefaultParameter( "FCAL-LED:LED_max1", m_LED_max1);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_max2", m_LED_max2);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_max3", m_LED_max3);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_max4", m_LED_max4);
  gPARMS->SetDefaultParameter( "FCAL-LED:LED_max5", m_LED_max5);
  m_regions_inf[0] = m_LED_min1;
  m_regions_sup[0] = m_LED_max1;
  m_regions_inf[1] = m_LED_min2;
  m_regions_sup[1] = m_LED_max2;
  m_regions_inf[2] = m_LED_min3;
  m_regions_sup[2] = m_LED_max3;
  m_regions_inf[3] = m_LED_min4;
  m_regions_sup[3] = m_LED_max4;
  m_regions_inf[4] = m_LED_min5;
  m_regions_sup[4] = m_LED_max5;
  for (int i = 0; i < 5; i ++) {
    cout << "inf " << m_regions_inf[i] << " sup " << m_regions_sup[i] << endl;
    m_regions_inf[i] *= 1e6;
    m_regions_sup[i] *= 1e6;
  }
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
  if (m_do_tree) {
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
  }
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
 
  bool is_FCAL_LED_trigger = false;
  const DL1Trigger *trig = NULL;
  try {
    loop->GetSingle(trig);
  } catch (...) {}

  // parse the triggers we want to save                                                                                                                                                                   
  if (trig) {
    if (trig->fp_trig_mask & 0x004) {   // Trigger front-panel bit 2                                                                                                                            
      // FCAL LED trigger fired                                                                                                                                                           
      is_FCAL_LED_trigger = true;
    }
  }
  
  if (m_is_fcal_led_skim)
    is_FCAL_LED_trigger = true;

  if (is_FCAL_LED_trigger) {

    vector< const DFCALHit* > hits;
    loop->Get( hits );
    
    vector<const DFCALDigiHit*> digihits;
    loop->Get( digihits );
    
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
    m_integTot = 0;
    
    for( vector< const DFCALHit* >::const_iterator hit = hits.begin(); hit != hits.end(); ++hit ){
      
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
      m_integ[m_nHits] = dHit.pulse_integral - (m_ped[m_nHits]*dHit.nsamples_integral);
      
      m_integOpeak[m_nHits] = m_integ[m_nHits] / m_peak[m_nHits];
      
      int row = fcalGeom.row((**hit).x);
      int col = fcalGeom.column((**hit).y);
      
      m_integTot += m_integ[m_nHits];
      
      Fill2DHistogram("hv_scan","","XYGeo", row, col, ";row;column #;Counts", 59, 0, 59, 59, 0, 59);
      Fill2DWeightedHistogram("hv_scan","","XYGeo_w", row, col, (**hit).E, ";row;column #;E_{max}^{sum} [GeV]", 59, 0, 59, 59, 0, 59);
      Fill2DHistogram("hv_scan","","m_E", m_chan[m_nHits], (**hit).E, ";channel;energy;Counts", 2800, 0, 2800, 1200, 0, 12.);
      Fill2DHistogram("hv_scan","","m_ped", m_chan[m_nHits], m_ped[m_nHits], ";channel;pedestal;Counts", 2800, 0, 2800, 600, 80, 120);
      Fill2DHistogram("hv_scan","","m_peak", m_chan[m_nHits], m_peak[m_nHits], ";channel;peak;Counts", 2800, 0, 2800, 4096, -0.5, 4096.5);
      Fill2DHistogram("hv_scan","","m_integ", m_chan[m_nHits], m_integOpeak[m_nHits], ";channel;integ;Counts", 2800, 0, 2800, 2000, 0., 200000.);
      
      ++m_nHits;
    }
    
    Fill1DHistogram("hv_scan","", "m_integTot", m_integTot, ";Integral total;Counts", 2000, 0., 50e6);
    
    
    for (int j = 0; j < m_nHits; j ++) {
      
      if (m_regions_inf[0] <= m_integTot && m_integTot <= m_regions_sup[4]) {
	
	Fill1DHistogram("hv_scan","", "m_patterntot", m_chan[j], ";channel;Counts", 2800, 0, 2800);
	Fill2DHistogram("hv_scan","", "m_integtot", m_chan[j], m_integ[j], ";channel;integ;Counts", 2800, 0, 2800, 3500, 0., 35000.);
	Fill2DHistogram("hv_scan","", "m_integOpeaktot", m_chan[j], m_integOpeak[j], ";channel;integOpeak;Counts", 2800, 0, 2800, 1000, 0., 1000.);
	
      }
      
    }
    
    for (int i = 0; i < 5; i ++) {
      
      for (int j = 0; j < m_nHits; j ++) {
	/*
	  if (m_regions_inf[0] <= m_integTot && m_integTot <= m_regions_sup[4]) {
	  
	  Fill1DHistogram("hv_scan","", Form("m_patterntot_%d", i), m_chan[j], ";channel;Counts", 2800, 0, 2800);
	  Fill2DHistogram("hv_scan","", Form("m_integtot_%d", i), m_chan[j], m_integ[j], ";channel;integ;Counts", 2800, 0, 2800, 3500, 0., 35000.);
	  Fill2DHistogram("hv_scan","", Form("m_integOpeaktot_%d", i), m_chan[j], m_integOpeak[j], ";channel;integOpeak;Counts", 2800, 0, 2800, 1000, 0., 1000.);
	  
	  }
	*/
	if (m_regions_inf[i] <= m_integTot && m_integTot <= m_regions_sup[i]) {
	  
	  Fill1DHistogram("hv_scan","", Form("m_pattern_%d", i), m_chan[j], ";channel;Counts", 2800, 0, 2800);
	  Fill2DHistogram("hv_scan","", Form("m_integ_%d", i), m_chan[j], m_integ[j], ";channel;integ;Counts", 2800, 0, 2800, 3500, 0., 35000.);
	  Fill2DHistogram("hv_scan","", Form("m_integOpeak_%d", i), m_chan[j], m_integOpeak[j], ";channel;integOpeak;Counts", 2800, 0, 2800, 1000, 0., 1000.);
	  Fill2DHistogram("hv_scan","", Form("m_peak_%d", i), m_chan[j], m_peak[j], ";channel;peak;Counts", 2800, 0, 2800, 4096, -0.5, 4096.5);
	  Fill2DHistogram("hv_scan","", Form("m_E_%d", i), m_chan[j], m_E[j], ";channel;peak;Counts", 2800, 0, 2800, 4096, 0., 12.);
	}
	
      }
      
    }

    if (m_do_tree) {
      //cout <<"Filling" << endl;
      m_tree->Fill();
    }
    japp->RootFillUnLock(this);
  }
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
  //m_tree->Write();
  japp->RootUnLock();

  return NOERROR;
}

