#if !defined(GENERAL_H)
#define GENERAL_H


#include <inttypes.h>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>

#include "TText.h"
#include "TLatex.h"
#include "TPaveStats.h"
#include "TVirtualPad.h"
#include "TFrame.h"
#include "TGraphPainter.h"
#include "TString.h"
#include "TCollection.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMinuit.h"
#include "TKey.h"
#include "TDatime.h"
#include "TMath.h"
#include "TAxis.h"
#include "TLine.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH2F.h"
#include "TColor.h"
#include "TStyle.h"
#include "TPaletteAxis.h"

#include "StringUtilities.h"


/*
  This gets the histogram drawn to a canvas from the command line TTree::Draw()
  TH1F* htemp = (TH1F*)gPad->GetPrimitive("htemp");
*/

int slots[] = {3,4,5,6,7,8,9,10,13,14,15,16,17,18,19};
const int _crate = 6, _slot = 20, _ch = 16, nsamp = 100;
  

struct ADC_Channel{
  
  ADC_Channel()  {}
  ADC_Channel(TString Type, TString Date) : type(Type), date(Date) {}
  ADC_Channel(TString Type, TString Date, double Data) : type(Type), date(Date), data(Data) {}
  ADC_Channel(int Crate, int Slot, int Channel, int Event) : crate(Crate), slot(Slot), channel(Channel), event(Event){}
  
  int crate;
  int slot;
  int channel;
  int event;
  TString type;
  TString date;
  double data;
  
  int channelnum;
  TString absNum;
  
  int x;
  int y;
  
  double timing;
  uint32_t maxADC;
  uint32_t minADC;
  
  double wIntegral;
  double pedestal;
  double charge;
  double eSum;
  
  double pintegral;
  double ppedestal;
  double pnsamples;     
  double ppeak;     
  double peakint;
  double peakht;
  double ratio;
  
  int nSamples;
  double rms;
  
  double hv;
  double current;
  double temperature;
  double dynode;
  double mvb;
  double mvt;
  double dac;
  int status;
  int overFlow;
  TGraph* pulse;

  vector<uint32_t> waveform;

};

// ADC will be stored in a struct
// Struct that holds crate/slot/channel


struct sortByChannel_ASC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.channelnum < right.channelnum;
    }
};

struct sortByEvent_ASC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.event < right.event;
    }
};

struct sort_ASC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.absNum < right.absNum;
    }
};

struct sortByCosmicHits_DSC {
    bool operator()(const std::pair<int, std::vector<ADC_Channel> > &left, const std::pair<int, std::vector<ADC_Channel> > &right) {
        return left.first > right.first;
    }
};

struct sortBySlot {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
      char left_crateslot[10];
      char right_crateslot[10];
      sprintf(left_crateslot,"%i/%02i",left.crate,left.slot);
      sprintf(right_crateslot,"%i/%02i",right.crate,right.slot);
        return left_crateslot < right_crateslot;
    }
};

class GeneralUtilities{
  public:
    
    static bool CalibrateDAC;
    static bool PrintOverflows;
    static bool RemoveOverflows;
    static bool sort_by_channel_event;
    static bool sort_by_channel;
    static bool sort_by_event;
    static bool sort_by_slot;

    static bool KeepFCALOnly;
    
    static bool SlotExists_OLD(int crate, int slot, bool north);
    static bool SlotExists(int crate, int slot);
    
    static std::map<TString, std::pair<int,int> > BaseLocationMap();
    static std::map<std::pair<int,int>, TString > BaseLocationMapInverse();
    static std::map<TString, int > BaseAbsMap();
    static map<int,std::pair<int,int> > baseToCoordMap(int i);
    static map<std::pair<int,int>, int > coordToCANMap(int i);
    static map<std::pair<int,int>, int > coordToBaseIDMap(int i);
    static map<std::pair<int,int>, TString > coordToExpertMap();
    static map<std::pair<int,int>, TString > coordToNonExpertMap();
    
    static bool RemoveADC(int crate, int slot, int channel);
    
    static int  KeepSpecificCrate;
    static int  KeepSpecificSlot;
    static int  KeepSpecificChannel;
    static bool KeepADC(int crate=1,int slot=1,int channel=1);
    
    static TString formatName(TString filename);
    
    static TString formatTitle(int crate, int slot, int channel);
    static TString formatTitle2(int crate, int slot, int channel);
    
};
#endif
