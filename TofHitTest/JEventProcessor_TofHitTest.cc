// $Id$
//
//    File: JEventProcessor_TofHitTest.cc
// Created: Tue Jul 11 07:43:17 EDT 2017
// Creator: zihlmann (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_TofHitTest.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_TofHitTest());
  }
} // "C"


//------------------
// JEventProcessor_TofHitTest (Constructor)
//------------------
JEventProcessor_TofHitTest::JEventProcessor_TofHitTest()
{
  
}

//------------------
// ~JEventProcessor_TofHitTest (Destructor)
//------------------
JEventProcessor_TofHitTest::~JEventProcessor_TofHitTest()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_TofHitTest::init(void)
{
  // This is called once at program startup. 

  japp->RootFillLock(this);
  TDirectory *top = gDirectory;

  RF = new TFile("tofhittest.root","RECREATE");
  RF->cd();

  char hnam[128];
  char htit[128];
  for (int k=0;k<176;k++){
    sprintf(hnam,"dt%03d",k);
    sprintf(htit,"ADCt-TDCt vs. ADC_amplitude PMT#%03d",k+1);
    dt[k] = new TH2D(hnam, htit, 100, 0., 4096., 400, -10.,10.);
    dt[k]->GetXaxis()->SetTitle("Signal amplitude [ADC counts]");
    dt[k]->GetYaxis()->SetTitle("ADC time minus TDC time [ns]");
  }
  for (int k=0;k<2;k++){
    for (int j=0;j<88;j++){
      sprintf(hnam,"singles%d%02d",k,j);
      sprintf(htit,"ADC only hits,  Plane %d, PMT#%02d",k, j); 
      singles[k][j] = new TH1D(hnam, htit, 800, -200., 200.);
      singles[k][j]->GetXaxis()->SetTitle("ADC single hit time [ns]");

      sprintf(hnam,"adct%d%02d",k,j);
      sprintf(htit,"ADC times,  Plane %d, PMT#%02d",k, j); 
      adct[k][j] = new TH1D(hnam, htit, 1600, -200., 200.);
      adct[k][j]->GetXaxis()->SetTitle("ADC time [ns]");
 
      sprintf(hnam,"tdct%d%02d",k,j);
      sprintf(htit,"TDC times,  Plane %d, PMT#%02d",k, j); 
      tdct[k][j] = new TH1D(hnam, htit, 1600, -200., 200.);
      tdct[k][j]->GetXaxis()->SetTitle("TDC time [ns]");
    }
  }

  top->cd();

  japp->RootFillUnLock(this);

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TofHitTest::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TofHitTest::evnt(JEventLoop *loop, uint64_t eventnumber)
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

  vector <const DTOFHit*> TOFHits;
  loop->Get(TOFHits);

  for (unsigned int k=0; k<TOFHits.size(); k++){
    
    const DTOFHit* hit = TOFHits[k];

    if (hit->has_fADC && hit->has_TDC){
      int idx = 88*hit->plane + 44*hit->end + hit->bar-1;
      dt[idx]->Fill(hit->Amp, hit->t_fADC-hit->t); // hit->t is walk corrected TDC time
      adct[hit->plane][hit->end*44 + hit->bar-1]->Fill(hit->t_fADC);
      tdct[hit->plane][hit->end*44 + hit->bar-1]->Fill(hit->t);
   } else {
      singles[hit->plane][hit->end*44 + hit->bar-1]->Fill(hit->t_fADC);
    }

  }




  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_TofHitTest::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TofHitTest::fini(void)
{
  // Called before program exit after event processing is finished.

  TDirectory *top = gDirectory;
  
  RF->cd();
 
  for (int k=0;k<176;k++){
    dt[k]->Write();
  }
  for (int k=0;k<2;k++){
    for (int j=0;j<88;j++){
      singles[k][j]->Write();
      adct[k][j]->Write();
      tdct[k][j]->Write();
    }
  }

  RF->Close();
  top->cd();

  return NOERROR;
}

