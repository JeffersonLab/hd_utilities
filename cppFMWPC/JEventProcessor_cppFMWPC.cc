// $Id$
//
//    File: JEventProcessor_cppFMWPC.cc
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//



// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
using namespace jana;
#include "JEventProcessor_cppFMWPC.h"
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_cppFMWPC());
  }
} // "C"


//------------------
// JEventProcessor_cppFMWPC (Constructor)
//------------------
JEventProcessor_cppFMWPC::JEventProcessor_cppFMWPC()
{
  
}

//------------------
// ~JEventProcessor_cppFMWPC (Destructor)
//------------------
JEventProcessor_cppFMWPC::~JEventProcessor_cppFMWPC()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_cppFMWPC::init(void)
{
  // This is called once at program startup. 

  TDirectory *top = gDirectory;
  top->cd();

  gDirectory->mkdir("FMWPC")->cd();

  nFMWPCchambers = 6;
  char hnam[128], htit[128];
  for ( int k=0; k<nFMWPCchambers ;k++){
    sprintf(hnam,"FMWPCwiresT%d",k);
    sprintf(htit,"Chamber %d FMWPC hit time vs. wire number",k);
    FMWPCwiresT[k] = new TH2D(hnam, htit,  145, 0., 145., 100, 0., 500.);

    sprintf(hnam,"FMWPCwiresE%d",k);
    sprintf(htit,"Chamber %d FMWPC hit dE vs. wire number",k);
    FMWPCwiresE[k] = new TH2D(hnam, htit,  145, 0., 145., 500, 0., 100.);
  }


  for (int k=0; k<24; k++){
    sprintf(hnam,"FDCwiresT%d",k);
    sprintf(htit,"Plane %d FDC hit time vs. wire number",k);
    FDCwiresT[k] = new TH2D(hnam, htit,  100, 0., 100., 100, 0., 500.);
  }
  
  top->cd();


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_cppFMWPC::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_cppFMWPC::evnt(JEventLoop *loop, uint64_t eventnumber)
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
  

  vector < const DFMWPCHit*> fmwpcHits;
  loop->Get(fmwpcHits);

  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    FMWPCwiresT[hit->layer-1]->Fill((double)hit->wire, (double)hit->t);
    FMWPCwiresE[hit->layer-1]->Fill((double)hit->wire, (double)hit->dE);
  }

  vector <const DFDCHit*> fdcHits;
  loop->Get(fdcHits);
  
  for (int k=0; k<(int)fdcHits.size(); k++){
    const DFDCHit *h = fdcHits[k];
    if (h->plane != 2){
      // this is not a wire
      continue;
    }
    int l = h->gLayer - 1 ;
    int w = h->element;
    FDCwiresT[l]->Fill((double)w, h->t);
  }

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_cppFMWPC::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_cppFMWPC::fini(void)
{
  // Called before program exit after event processing is finished.

  return NOERROR;
}

