#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
#include "TSystem.h"

void FasterCountBelowVal(TString fileName){
  
  //cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("Df250WindowRawData");
  
  uint32_t channelnum;         /// Arbitrary global channel number
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  uint32_t itrigger; ///trigger number for cases when this hit was read in a multi-event block (DDAQAddress)
  uint32_t nsamples;           /// Number of samples in the event
  std::vector<uint32_t>* waveform = 0;   /// STL vector of samples in the waveform for the event
  uint32_t w_integral;         /// Sum of all samples in the waveform
  uint32_t w_min;              /// Minimum sample in the waveform
  uint32_t w_max;              /// Maximum sample in the waveform
  uint32_t w_samp1;            /// First sample in the waveform
  uint32_t w_ped;             /// Pedestal, Need to Ask Mark How this is calculated
  float   w_time;            /// The pulse arrival in samples based on teh IU method
  
  tr->SetBranchAddress("channelnum",&channelnum);
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("itrigger",&itrigger);
  tr->SetBranchAddress("nsamples",&nsamples);
  tr->SetBranchAddress("waveform",&waveform);
  tr->SetBranchAddress("w_integral",&w_integral);
  tr->SetBranchAddress("w_min",&w_min);
  tr->SetBranchAddress("w_max",&w_max);
  tr->SetBranchAddress("w_samp1",&w_samp1);
  tr->SetBranchAddress("w_ped",&w_ped);
  tr->SetBranchAddress("w_time",&w_time);
  
  int val = 1000;
  
  map<pair<int,int>, int> counter ; // number of times a channels pulse dips below val
  
  int nsize = (int) tr->GetEntries();

  for (int i=0; i<nsize; i++){
    tr->GetEntry(i);
    
    if (i%100000==0) cout << i << endl;

    // skip non fcal crates
    if (GeneralUtilities::KeepFCALOnly) continue;  
    if (rocid<11 || rocid>23) continue;

      
    char tmp_name[50];
    sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
    std::pair<int,int> loc_pair = map_loc.find(tmp_name)->second;
    int chan_x = loc_pair.first;
    int chan_y = loc_pair.second;
    
    int wsize = waveform->size();
    for (int ii=0; ii<wsize; ii++){
      uint32_t adc_value = (*waveform)[ii];
      if (adc_value<val) counter[make_pair(chan_x,chan_y)]++;
    }
  } // End loop over tree
  
  f->Close();
  cout << counter.size() << endl;
  // loop over map of channels and get the number of events where the pulse dips below val
  for (map<pair<int,int>, int>::const_iterator iter=counter.begin(); iter!=counter.end(); ++iter){
    printf("(%i,%i) has %i events that dip below %i", iter->first.first, iter->first.second, iter->second, val);
  }
}

