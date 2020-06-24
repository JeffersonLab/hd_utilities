// $Id$
//
//    File: JEventProcessor_TOFmon.h
// Created: Wed Sep 18 09:33:05 EDT 2019
// Creator: zihlmann (on Linux gluon48.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//
// new version of TOFmon to handle the new TOF2 upgraded detector with more paddles.



#ifndef _JEventProcessor_TOFmon_
#define _JEventProcessor_TOFmon_

#include <sys/stat.h>

#include <JANA/JEventProcessor.h>

#include <TOF/DTOFHit.h>
#include <TOF/DTOFDigiHit.h>
#include <TOF/DTOFTDCDigiHit.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/DCAEN1290TDCHit.h>
#include <DAQ/DEPICSvalue.h>
#include "TRIGGER/DL1Trigger.h"
#include <DANA/DStatusBits.h>
#include <TOF/DTOFGeometry.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TMath.h>

using namespace std;
using namespace jana;

class JEventProcessor_TOFmon:public jana::JEventProcessor{
 public:
  JEventProcessor_TOFmon();
  ~JEventProcessor_TOFmon();
  const char* className(void){return "JEventProcessor_TOFmon";}

  double TimingCut;
  char RootFile[128];
  TFile *OUTF;

  int TOF_NUM_PLANES;
  int TOF_NUM_BARS;  
  int TOF_NUM_SHORT_BARS;
  int NPMTS;
  
  TH1F *adchitcount;
  TH2F *amplitude;
  TH2F *amplitudeMatch;
  TH2F *pedestals;
  TH2F *integrals;
  TH2F *ADCtime;
  TH2F *TDCtime;
  TH2F *TDCtimeMatch;
  TH1F *TDChits;
  TH2F *TWalk[200];
  TH1F *DTRaw[100];
  TH2F *AMPvsDTRaw[2][100];
  TH2F *INTvsDTRaw[2][100];
  TH2F *MTDiff[50];
  TH2F *ADCvsTDC[200];

  TH1F *Intensities[6][32];
  TH1F *INL[6][32];

  TH1F *dt_21_p1_p2;
  TH1F *dt_21_p1_p2corr;

  float TIMINGCUT;
  float TOF_TDC_SHIFT;
  float ADC2time ;
  float TDC2time ;
  float TDCtOffset;
  float ADCtOffset;
  float WalkPar[200][3];

  float walkpar[200][6];
  jerror_t WalkFit( TH2F *h, int idx);

  float BeamCurrent;
  int NOBEAM;
  int BEAM;
  int CheckBeam;
 
 private:
  jerror_t init(void);	
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
  jerror_t erun(void);
  jerror_t fini(void);
};

#endif // _JEventProcessor_TOFmon_

