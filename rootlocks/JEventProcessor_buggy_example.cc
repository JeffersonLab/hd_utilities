// $Id$
//
//    File: JEventProcessor_buggy_example.cc
//

#include "JEventProcessor_buggy_example.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_buggy_example());
}
} // "C"


//------------------
// JEventProcessor_buggy_example (Constructor)
//------------------
JEventProcessor_buggy_example::JEventProcessor_buggy_example()
{

}

//------------------
// ~JEventProcessor_buggy_example (Destructor)
//------------------
JEventProcessor_buggy_example::~JEventProcessor_buggy_example()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_buggy_example::init(void)
{
	// This is called once at program startup. 

  japp->RootWriteLock();
  
  TDirectory *main = gDirectory;
  gDirectory->mkdir("buggy_example")->cd();

  bestfom_dedx_p = new TH2D("bestfom_dedx_p","CDC dE/dx vs p, 4+ hits used, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  bestfom_dedx_p_pos = new TH2D("bestfom_dedx_p_pos","CDC dE/dx vs p, q+, 4+ hits used, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  bestfom_dedx_p_neg = new TH2D("bestfom_dedx_p_neg","CDC dE/dx vs p, q-, 4+ hits used, best FOM;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);


  dedx_p = new TH2D("dedx_p","CDC dE/dx vs p, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  dedx_p_pos = new TH2D("dedx_p_pos","CDC dE/dx vs p, q+, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  dedx_p_neg = new TH2D("dedx_p_neg","CDC dE/dx vs p, q-, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);


  intdedx_p = new TH2D("intdedx_p","CDC dE/dx (from integral) vs p, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  intdedx_p_pos = new TH2D("intdedx_p_pos","CDC dE/dx (from integral) vs p, q+, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  intdedx_p_neg = new TH2D("intdedx_p_neg","CDC dE/dx (from integral) vs p, q-, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);



  main->cd();

  japp->RootUnLock();
  
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_buggy_example::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_buggy_example::evnt(JEventLoop *loop, uint64_t eventnumber)
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

  // select events with physics events, i.e., not LED and other front panel triggers
  const DTrigger* locTrigger = NULL; 
  loop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) return NOERROR;


  const DVertex* locVertex  = NULL;
  loop->GetSingle(locVertex);
  double vertexz = locVertex->dSpacetimeVertex.Z();
  if ((vertexz < 52.0) || (vertexz > 78.0)) return NOERROR;



  vector<const DChargedTrack*> ctracks;
  loop->Get(ctracks);
  
  for (uint32_t i=0; i<(uint32_t)ctracks.size(); i++) {  
      
    // get the best hypo
    const DChargedTrackHypothesis *hyp=ctracks[i]->Get_BestFOM();    
    if (hyp == NULL) continue;
      
    const DTrackTimeBased *track = hyp->Get_TrackTimeBased();
    //    uint16_t ntrackhits_cdc = (uint16_t)track->measured_cdc_hits_on_track;

    int nhits = (int)track->dNumHitsUsedFordEdx_CDC; 
    if (nhits < 4) continue;

    double charge = track->charge();
    DVector3 mom = track->momentum();
    double p = mom.Mag();

    double dedx = 1.0e6*track->ddEdx_CDC_amp;

    if (dedx > 0) {

      japp->RootFillLock(this);

      bestfom_dedx_p->Fill(p,dedx);
    
      if (charge > 0) {
        bestfom_dedx_p_pos->Fill(p,dedx);
	return;  //BUG
      } else {
        bestfom_dedx_p_neg->Fill(p,dedx);
      } 

      japp->RootFillUnLock(this);

    }
  }

  
  vector<const DTrackTimeBased*> tracks;
  loop->Get(tracks);
  if (tracks.size() ==0) return NOERROR;


  for (uint32_t i=0; i<tracks.size(); i++) {

    int nhits = (int)tracks[i]->dNumHitsUsedFordEdx_CDC; 
    if (nhits < 4) continue;

    double charge = tracks[i]->charge();
    DVector3 mom = tracks[i]->momentum();
    double p = mom.Mag();

    double dedx = 1.0e6*tracks[i]->ddEdx_CDC_amp;

    if (dedx > 0) {

      dedx_p->Fill(p,dedx);   //BUG
      
      japp->RootWriteLock();    
      if (charge > 0) {
        dedx_p_pos->Fill(p,dedx);
      } else {
        dedx_p_neg->Fill(p,dedx);
      } 

      japp->RootUnLock();

    }

    // repeat for dedx from integral

    dedx = 1.0e6*tracks[i]->ddEdx_CDC;

    if (dedx > 0) {

      japp->RootFillLock(this);

      intdedx_p->Fill(p,dedx);
    
      if (charge > 0) {
        intdedx_p_pos->Fill(p,dedx);
      } else {
        intdedx_p_neg->Fill(p,dedx);
      } 

      japp->RootFillUnLock(this);

    }



  }

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_buggy_example::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_buggy_example::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

