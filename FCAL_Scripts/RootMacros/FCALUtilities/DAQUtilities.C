
#include "DAQUtilities.h"
#include "PlotUtilities.h"


// First and Last event of DAQ run
int DAQUtilities::first_event = 0;
int DAQUtilities::last_event = 0;

// Some Logic In Case you don't need certain variables
// This could speed up processing
// By default everything will be true
bool DAQUtilities::get_waveform = false;
bool DAQUtilities::marks_timing = true;
bool DAQUtilities::get_coordinates = true;
bool DAQUtilities::custom_timing = false;


// Sort function
vector< ADC_Channel > DAQUtilities::sortADC(vector< ADC_Channel > adcList, TString sortby){
  // Sort vector
  cout << "Sorting..." << endl;
  cout << "Currently sorting by: " << sortby << endl;

  if(sortby == "nosort") {
    GeneralUtilities::sort_by_slot = false;
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_channel=false;
    GeneralUtilities::sort_by_event=false;
  }  

  if(sortby == "channel") {
    GeneralUtilities::sort_by_slot = false;
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_channel=true;
    GeneralUtilities::sort_by_event=false;
  }  

  if(sortby == "event") {
    GeneralUtilities::sort_by_slot = false;
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_channel=false;
    GeneralUtilities::sort_by_event=true;
  }  

  if(sortby == "absnum") {
    GeneralUtilities::sort_by_slot = false;
    GeneralUtilities::sort_by_channel_event = true;
    GeneralUtilities::sort_by_channel=false;
    GeneralUtilities::sort_by_event=false;
  }  

  if(sortby == "slot") {
    GeneralUtilities::sort_by_slot = true;
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_channel=false;
    GeneralUtilities::sort_by_event=false;
  }  

  if (GeneralUtilities::sort_by_channel_event) {
    std::sort(adcList.begin(),adcList.end(),sort_ASC());
    GeneralUtilities::sort_by_channel = false;
    GeneralUtilities::sort_by_event = false;
    GeneralUtilities::sort_by_slot  = false;
  }
    
  
  if (GeneralUtilities::sort_by_channel){
    std::sort(adcList.begin(),adcList.end(),sortByChannel_ASC());
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_event = false;
    GeneralUtilities::sort_by_slot  = false;
  }
    
  if (GeneralUtilities::sort_by_event){
    std::sort(adcList.begin(),adcList.end(),sortByEvent_ASC());
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_channel = false;
    GeneralUtilities::sort_by_slot  = false;
  }
    
  if (GeneralUtilities::sort_by_slot){
    std::sort(adcList.begin(),adcList.end(),sortBySlot());
    GeneralUtilities::sort_by_channel_event = false;
    GeneralUtilities::sort_by_channel = false;
    GeneralUtilities::sort_by_event = false;
  }
  return adcList;
}


std::map<TString, TGraph*> DAQUtilities::adcValues_TGraph(TString fileName, int everyN, double pulser_freq, TString adc, bool compressed_mode, int threshold, int NSB, int NSA, bool manny_plugin){
  
  TString adc_title = "";
  if (adc=="ped")    adc_title = "Pedestal: ";
  if (adc=="max")    adc_title = "Pulse Height: ";
  if (adc=="integral") adc_title = "Pulse Integral: ";
  if (adc=="charge") adc_title = "Pulse Integral: ";
  if (adc=="timing") adc_title = "Timing: ";
  
  bool compute = false;
  if (NSB!=NSA) compute = true;
  
  TString sortby = "absnum";
  std::vector< ADC_Channel > adcList;
  bool mark_ped_time = false;
  if (!compressed_mode && !manny_plugin) adcList = DAQUtilities::main_loop(fileName, sortby, mark_ped_time, false, false, threshold, NSB, NSA, compute);
  if ( compressed_mode && !manny_plugin) adcList = DAQUtilities::main_loop_compressedMode(fileName, sortby);
  if (manny_plugin)     adcList = DAQUtilities::manny_plugin_loop(fileName, sortby);
  int adcSize = adcList.size();
  
  map<TString, TGraph*> adc_map;
  
  TGraph* adc_gr = 0x0;
  double avgADC = 0;
  int gcounter = 0;
  int counter = 0;
  
  for (int i=0; i<adcSize; i++){
    ADC_Channel MY_ADC = adcList[i];
    int MY_CRATE        = MY_ADC.crate;
    int MY_SLOT         = MY_ADC.slot;
    int MY_CHANNEL      = MY_ADC.channel;
    double MY_PEDESTAL  = MY_ADC.pedestal;
  
    int MY_EVENT        = MY_ADC.event;
    int MY_X            = MY_ADC.x;
    int MY_Y            = MY_ADC.y;
    double MY_TIMING    = double(MY_ADC.timing);
    double MY_CHARGE    = MY_ADC.charge;
    double MY_INTEGRAL  = MY_ADC.wIntegral;
    double MY_MAXADC;
    if (!compressed_mode) MY_MAXADC = double(MY_ADC.maxADC);
    
    TString name = GeneralUtilities::formatTitle(MY_CRATE, MY_SLOT, MY_CHANNEL);
    //TString name = adc_title + DAQUtilities::formatTitle2(MY_CRATE, MY_SLOT, MY_CHANNEL);
    TString gr_title = adc_title + GeneralUtilities::formatTitle2(MY_CRATE, MY_SLOT, MY_CHANNEL)+" ("+StringUtilities::int2TString(MY_X)+","+StringUtilities::int2TString(MY_Y)+")";
    
    bool doAvg = (MY_EVENT-DAQUtilities::first_event+1) % everyN == 0;
    if ( MY_EVENT==DAQUtilities::last_event) doAvg = true;
    
    bool fill_map = false;
    
    if (adc_gr==0x0){
      adc_gr = new TGraph();
      adc_gr->SetName(name);
      adc_gr->SetTitle(gr_title);
    }
    
    if ( i+1!=adcSize ){
      ADC_Channel MY_NEXT_ADC = adcList[i+1];
      int MY_NEXT_CRATE    = MY_NEXT_ADC.crate;
      int MY_NEXT_SLOT     = MY_NEXT_ADC.slot;
      int MY_NEXT_CHANNEL  = MY_NEXT_ADC.channel;
      
      // Check if the previous element is the same crate/slot/channel as the current element
      // If they are then fill a tgraph with pedestal values
      if (MY_CRATE==MY_NEXT_CRATE && MY_SLOT==MY_NEXT_SLOT && MY_CHANNEL==MY_NEXT_CHANNEL){
        fill_map = false;
      }
      
      // If Not then fill a map with the tgraph
      if (MY_CRATE!=MY_NEXT_CRATE || MY_SLOT!=MY_NEXT_SLOT || MY_CHANNEL!=MY_NEXT_CHANNEL){
        fill_map = true;
      }
    }
    
    if ( i+1==adcSize ){
      fill_map = true;
    }
    
    if (!fill_map){
      //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
      if (adc=="ped")      avgADC += MY_PEDESTAL;
      if (adc=="max")      avgADC += MY_MAXADC;
      if (adc=="charge")   avgADC += MY_CHARGE;
      if (adc=="integral") avgADC += MY_INTEGRAL;
      if (adc=="timing")   avgADC += MY_TIMING;
      counter ++;
      // x is in hours
      if (doAvg) {
        double yval = avgADC / double(counter);
        if (adc=="charge") yval *= 1.0/1000.0;
        double xval = 1.0/pulser_freq*1/3600.0*double(everyN)*double(gcounter);
        if (pulser_freq<1) xval = MY_EVENT;
        //cerr << gcounter << "\t" << xval << "\t" << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << "\t" << yval << endl;
        adc_gr->SetPoint(gcounter, xval, yval ); 
        avgADC=0; counter=0;
        gcounter++;
      }
    }
    
    if (fill_map){
      //cerr << "Fitting" << endl;
      //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
      //cout << "mean: " << TMath::Mean(adc_gr->GetN(), adc_gr->GetY()) << endl;
      //adc_gr->GetXaxis()->SetTitle("Time (hours)");
      //if (adc=="charge")  adc_gr->GetYaxis()->SetTitle("Events in Thousands");
      adc_map[name] = adc_gr;
      adc_gr = 0x0;
      gcounter = 0;
    }
    
    //cout << name << "\t" << event-DAQUtilities::first_event+1 << "\t" << x << "," << y << "\tAvg: " << doAvg << endl;
    
  }
  
  //DAQUtilities::plotGraphs(fileName, ped_map, true, dir, "Time (hours)", "Pedestal (Channels)");
  //TString ytitle = "";
  //if (adc=="charge") ytitle = "Events (in thousands)";
  //if (plot) {DAQUtilities::plotAllGraphs2(fileName, adc_map, true, dir, "Time (hours)", ytitle, 0, 0, adc);}
  return adc_map;
  
}

std::vector< ADC_Channel > 
DAQUtilities::manny_waveform_loop(TString fileName, TString sortby){
  
  //vector<std::pair<int,ADC_Channel> > adcList;t
  vector< ADC_Channel > adcList;
  
  // Number of Samples in an event that need to be above the trigger threshold to be counter as a signal
  // int triggerCounts = 2;
  
  
  //cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  
  
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("SpecialWaveforms");
  
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  Float_t pedestal;             /// Pedestal, Need to Ask Mark How this is calculated
  std::vector<uint32_t>* waveform = 0;   /// STL vector of samples in the waveform for the event
  
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("w_ped",&pedestal);
  tr->SetBranchAddress("waveform",&waveform);
  
  cout << "Manny Plugin" << endl;
  int prev_channel = 0;
  
  Int_t nsize = (Int_t) tr->GetEntries();


  tr->GetEntry(0);
  first_event = eventnum;
  
  tr->GetEntry(nsize-1);
  last_event = eventnum;
  
  TString T_last = StringUtilities::int2TString(last_event);

  for (Int_t i=0; i<nsize; i++){
    tr->GetEntry(i);
    
    //cout << i << " " << rocid << " " << slot << " " << channel << endl;
    // skip non fcal crates
    
    if (GeneralUtilities::KeepFCALOnly && (rocid<11 || rocid>23)) continue;

    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    
    if (!GeneralUtilities::KeepADC(rocid-10,slot,channel))
      continue;
    
    ADC_Channel my_channel(rocid-10,slot,channel,eventnum);
    
    if (get_coordinates){
      char tmp_name[50];
      sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
      std::pair<int,int> loc_pair = map_loc.find(tmp_name)->second;
      my_channel.x = loc_pair.first;
      my_channel.y = loc_pair.second;
    }
    
    vector<uint32_t> my_waveform;
    int wsize = waveform->size();
    //cout << "N = " << wsize << endl;
    for (int ii=0; ii<wsize; ii++){
      uint32_t adc_value = (*waveform)[ii];
      my_waveform.push_back(adc_value); 
    }
    my_channel.waveform = my_waveform;
    
    char temp_absNum[100];
    sprintf(temp_absNum,"%02i_%02i_%02i_",my_channel.crate,my_channel.slot,my_channel.channel);
    TString t_temp_absNum = temp_absNum;
    t_temp_absNum += "_"+StringUtilities::int2TString(my_channel.event,T_last.Length()+1);
    my_channel.absNum = t_temp_absNum;    
    my_channel.pedestal   = pedestal;
    adcList.push_back(my_channel);
    //cout << i << " " << my_channel.crate << " " << my_channel.slot << " " << my_channel.channel << " " << my_channel.event << endl;
  } // End loop over tree
  
  
  
  f->Close();
  
  return DAQUtilities::sortADC(adcList, sortby);
}

std::vector< ADC_Channel > 
DAQUtilities::manny_plugin_loop(TString fileName, TString sortby){
  
  //vector<std::pair<int,ADC_Channel> > adcList;
  vector< ADC_Channel > adcList;
  
  // Number of Samples in an event that need to be above the trigger threshold to be counter as a signal
  // int triggerCounts = 2;
  
  
  //cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  
  
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("Df250WindowRawDataPedestal");
  
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  Float_t pedestal;             /// Pedestal, Need to Ask Mark How this is calculated
  std::vector<uint32_t>* ped_vec = 0;
  
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("w_ped",&pedestal);
  tr->SetBranchAddress("w_ped_vec",&ped_vec);
  
  cout << "Manny Plugin" << endl;
  int prev_channel = 0;
  
  Int_t nsize = (Int_t) tr->GetEntries();


  tr->GetEntry(0);
  first_event = eventnum;
  
  tr->GetEntry(nsize-1);
  last_event = eventnum;
  
  TString T_last = StringUtilities::int2TString(last_event);

  for (Int_t i=0; i<nsize; i++){
    tr->GetEntry(i);
    
    //if (i<33687230) continue;
    
    //cout << i << " " << rocid << " " << slot << " " << channel << endl;
    // skip non fcal crates
    
    if (GeneralUtilities::KeepFCALOnly && (rocid<11 || rocid>23)) continue;

    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    
    if (!GeneralUtilities::KeepADC(rocid-10,slot,channel))
      continue;
    
    ADC_Channel my_channel(rocid-10,slot,channel,eventnum);
    
    if (get_coordinates){
      char tmp_name[50];
      sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
      
      if (map_loc.count(tmp_name)>0){
        std::pair<int,int> loc_pair = map_loc.find(tmp_name)->second;
        my_channel.x = loc_pair.first;
        my_channel.y = loc_pair.second;
      } else{
        cout << "Could not find coordinate for " << tmp_name << endl;
      }
    }
    
    vector<uint32_t> my_waveform;
    int wsize = ped_vec->size();
    //cout << "N = " << wsize << endl;
    for (int ii=0; ii<wsize; ii++){
      uint32_t adc_value = (*ped_vec)[ii];
      my_waveform.push_back(adc_value); 
    }
    my_channel.waveform = my_waveform;
    
    char temp_absNum[100];
    sprintf(temp_absNum,"%02i_%02i_%02i_",my_channel.crate,my_channel.slot,my_channel.channel);
    TString t_temp_absNum = temp_absNum;
    t_temp_absNum += "_"+StringUtilities::int2TString(my_channel.event,T_last.Length()+1);
    my_channel.absNum = t_temp_absNum;    
    my_channel.pedestal   = pedestal;
    adcList.push_back(my_channel);
    //cout << i << " " << my_channel.crate << " " << my_channel.slot << " " << my_channel.channel << " " << my_channel.event << endl;
  } // End loop over tree
  //cout << "Done with loop" << endl;
  
  
  f->Close();
  
  return DAQUtilities::sortADC(adcList, sortby);
}


void 
DAQUtilities::overflowFraction(TString fileName){
  
  vector< Pedestal_Study > pedStudyList;
  
  //cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("Df250WindowRawDataPedestal");
  
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  uint32_t channelnum;
  uint32_t overflow;
  std::vector<uint32_t>* ped_vec = 0;
  
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("channelnum",&channelnum);
  tr->SetBranchAddress("overflow",&overflow);
  
  Int_t nsize = (Int_t) tr->GetEntries();
  
  tr->GetEntry(0);
  first_event = eventnum;
  
  tr->GetEntry(nsize-1);
  last_event = eventnum;
  
  std::map< std::pair<int,int>, double> overflow_map;
  TH1F* hist_overflow = new TH1F("hist_overflow","Overflow",2,0,2);
  
  for (int i=0; i<nsize; i++){
    tr->GetEntry(i);
    
    if (GeneralUtilities::KeepFCALOnly && (rocid<11 || rocid>23)) continue;
    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    if (!GeneralUtilities::KeepADC(rocid-10,slot,channel))
      continue;
    
    if ( overflow==0) continue;
    
    hist_overflow->Fill(overflow);
    
    int x=0, y=0; 
    char tmp_name[50];
    sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
    if (map_loc.count(tmp_name)>0){
      x = map_loc[tmp_name].first;
      y = map_loc[tmp_name].second;
    } else{
      cout << "Could not find coordinate for " << tmp_name << endl;
    }
    
    overflow_map[ std::make_pair(x,y) ]++;
    
  }
  
  std::map< std::pair<int,int>, double> overflow_frac;
  for (std::map< std::pair<int,int>, double>::const_iterator iter=overflow_map.begin(); iter!=overflow_map.end(); ++iter){
    overflow_frac[iter->first] = double(iter->second)/double(last_event-first_event)*100;
  }
  overflow_map.clear();
  TH2F* hist_overfow_frac = PlotUtilities::Plot2D(overflow_frac);
  hist_overfow_frac->SetTitle("Percent of overflows");
  
  TCanvas* c_hist_overflow = new TCanvas("c_hist_overflow","c_hist_overflow",900,900);
  c_hist_overflow->cd();
  hist_overflow->Draw();
  
  TCanvas* c_hist_overflow_frac = new TCanvas("c_hist_overflow_frac","c_hist_overflow_frac",900,900);
  c_hist_overflow_frac->cd();
  hist_overfow_frac->Draw("colz");
  
}


vector< Pedestal_Study > 
DAQUtilities::manny_plugin_pedestalStudy(TString fileName, int nEvtsAvg){
  
  vector< Pedestal_Study > pedStudyList;
  
  //cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  bool raw = true;
  
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("Df250WindowRawDataPedestal");
  if (tr->GetEntries()==0){
    tr = (TTree*)f->Get("Df250PulseIntegral");
    raw = false;
  }
  
  
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  uint32_t channelnum;
  Float_t pedestal;             /// Pedestal, average of 1st 4 samples
  uint32_t pedestal_compressed;
  std::vector<uint32_t>* ped_vec = 0;
  
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("channelnum",&channelnum);
  if (raw) {
    cout << "raw" << endl;
    tr->SetBranchAddress("w_ped",&pedestal);
    tr->SetBranchAddress("w_ped_vec",&ped_vec);
  } else {
    cout << "Compressed" << endl;
    tr->SetBranchAddress("pedestal",&pedestal_compressed);
  }
  
  cout << "Manny Plugin" << endl;
  int prev_channel = 0;
  
  Int_t nsize = (Int_t) tr->GetEntries();
  
  tr->GetEntry(0);
  first_event = eventnum;
  
  tr->GetEntry(nsize-1);
  last_event = eventnum;
  
  TString T_last = StringUtilities::int2TString(last_event);
  
  map<TString, int> map_counter;
  map<TString, TH1F*> map_hist_ped;
  map<TString, TGraphErrors*> map_grE_meanVevent;
  
  int evt_counter = 0;

  for (Int_t i=0; i<nsize; i++){
    
    tr->GetEntry(i);
    
    if (GeneralUtilities::KeepFCALOnly && (rocid<11 || rocid>23)) continue;
    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    if (!GeneralUtilities::KeepADC(rocid-10,slot,channel))
      continue;
    
    
    if (!raw){
      pedestal = float(pedestal_compressed);
    }
    
    // Get Crate Slot Channel
    char name[50];
    sprintf(name, "%i/%i/%i", rocid-10, slot, channel);
    TString Tname = TString(name);
    
    // Check if a TH1F exists
    if ( map_hist_ped.count(Tname)>0 ) {
      if (map_hist_ped[Tname]==0x0) map_hist_ped[Tname] = new TH1F("hist_ped_"+Tname,"hist_ped_"+Tname,4096,0,4096);
    } else{
      map_hist_ped[Tname] = new TH1F("hist_ped_"+Tname,"hist_ped_"+Tname,4096,0,4096);
    }
    // Fill Histogram
    map_hist_ped[Tname]->Fill( pedestal );
    
    
    map_counter[Tname]++;
    if (map_counter[Tname] >= nEvtsAvg) {
      map_counter[Tname] = 0;
    }
    
    bool do_avg = map_counter[Tname]==0 ? true : false;
    
    
    if (do_avg){
      // Get (x,y) coordinates
      int x=0, y=0; 
      if (get_coordinates){
        char tmp_name[50];
        sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
        if (map_loc.count(tmp_name)>0){
          x = map_loc[tmp_name].first;
          y = map_loc[tmp_name].second;
        } else{
          cout << "Could not find coordinate for " << tmp_name << endl;
        }
      }
      
      TString title = Tname +" ("+StringUtilities::int2TString(x)+","+StringUtilities::int2TString(y)+")";
      // Check map for existence
      if (map_grE_meanVevent.count(Tname)>0){
        if (map_grE_meanVevent[Tname]==0x0){
          map_grE_meanVevent[Tname] = new TGraphErrors();
          map_grE_meanVevent[Tname]->SetName("grE_mean_vs_event_"+title);
          map_grE_meanVevent[Tname]->SetTitle("Ped Vs Event "+title);
        }
      } else{
        map_grE_meanVevent[Tname] = new TGraphErrors();
        map_grE_meanVevent[Tname]->SetName("grE_mean_vs_event_"+title);
        map_grE_meanVevent[Tname]->SetTitle("Ped Vs Event "+title);
      }
      
      // Get Pedestal Mean and Width
      double fit_mean  = map_hist_ped[Tname]->GetMean();
      double fit_width = map_hist_ped[Tname]->GetRMS();
      
      // Fit Pedestal
      /*
      h_ped->GetXaxis()->SetRangeUser(50,150);
      h_ped->Fit("gaus","0Q");
      TF1* fit = h_ped->GetFunction("gaus");
      double fit_mean  = fit->GetParameter(1);
      double fit_width = fit->GetParameter(2);
      TString status = gMinuit->fCstatu;
      if (!(status.Contains("CONVERGED"))) {
        fit_mean  = h_ped->GetMean();
        fit_width = h_ped->GetRMS();
      }
      */
      //cout << Tname << "\t" << fit_mean << endl;
      map_hist_ped[Tname]->SetName("");
      map_hist_ped[Tname] = 0x0;
      
      
      int point = map_grE_meanVevent[Tname]->GetN();
      map_grE_meanVevent[Tname]->SetPoint(point, eventnum, fit_mean);
      map_grE_meanVevent[Tname]->SetPointError(point, 0, fit_width);
      
      
    } // End Average
    
    
  } // End loop over tree
  //cout << "Done with loop" << endl;
  
  
  // Clean up
  for (map<TString, TGraphErrors*>::const_iterator iter=map_grE_meanVevent.begin(); iter!=map_grE_meanVevent.end(); ++iter) {
    
    TString Tname = iter->first;
    vector<TString> parseTname = StringUtilities::parseTString(Tname,"/");
    TGraphErrors* grE_mean_vs_event = iter->second;
    int x=0, y=0; 
    if (get_coordinates){
      char temp_name[50];
      sprintf(temp_name,"%i/%i/%i",parseTname[0].Atoi()-1,parseTname[1].Atoi(),parseTname[2].Atoi());
      if (map_loc.count(temp_name)>0){
        x = map_loc[temp_name].first;
        y = map_loc[temp_name].second;
      } else{
        cout << "Could not find coordinate for " << temp_name << endl;
      }
    }
    
    Tname += " ("+StringUtilities::int2TString(x)+","+StringUtilities::int2TString(y)+")";
    
    int nPoints = grE_mean_vs_event->GetN();
    Double_t* grE_mean_vs_eventY = grE_mean_vs_event->GetY();
    Double_t* grE_mean_vs_eventX = grE_mean_vs_event->GetX();
    double mean_min = TMath::MinElement(nPoints,grE_mean_vs_eventY);
    double mean_max = TMath::MaxElement(nPoints,grE_mean_vs_eventY);
    mean_min = mean_min-10>0 ? mean_min-10 : 0;
    mean_max = mean_max>1000 ? 300 : mean_max+10;
    
    
    TGraph* gr_mean_diff = new TGraph();
    gr_mean_diff->SetName("gr_mean_diff "+Tname);
    gr_mean_diff->SetTitle("Mean[0] - Mean[i]: "+Tname);
    
    TH1F* h_mean = new TH1F("h_mean_"+Tname, "Pedestal Mean: "+Tname,400,80,120);
    TH1F* h_mean_diff = new TH1F("h_spread_"+Tname, "Mean[0]-Mean[i]: "+Tname,100,-5,5);
    
    for (int j=0; j<nPoints; j++){
      h_mean_diff->Fill(grE_mean_vs_eventY[0] - grE_mean_vs_eventY[j]);
      gr_mean_diff->SetPoint(gr_mean_diff->GetN(), grE_mean_vs_eventX[j], grE_mean_vs_eventY[0] - grE_mean_vs_eventY[j]);
      h_mean->Fill(grE_mean_vs_eventY[j]);
    }
    
    Double_t* gr_mean_diffY = gr_mean_diff->GetY();
    int nDiff = gr_mean_diff->GetN();
    double mean_diff_min = TMath::MinElement(nDiff,gr_mean_diffY);
    double mean_diff_max = TMath::MaxElement(nDiff,gr_mean_diffY);
    mean_diff_min = mean_diff_min-10>0 ? mean_diff_min-10 : 0;
    mean_diff_max = mean_diff_max>1000 ? 300 : mean_diff_max+10;
    
    Pedestal_Study pedStudy;
    pedStudy.crate             = parseTname[0].Atoi();
    pedStudy.slot              = parseTname[1].Atoi();
    pedStudy.channel           = parseTname[2].Atoi();
    pedStudy.x                 = x;
    pedStudy.y                 = y;
    pedStudy.name              = Tname;
    pedStudy.spread            = h_mean->GetRMS();
    pedStudy.mean              = h_mean->GetMean();
    pedStudy.mean_min          = int(mean_diff_min);
    pedStudy.mean_max          = int(mean_diff_max);
    pedStudy.spread_diff       = h_mean_diff->GetRMS();
    pedStudy.mean_diff         = h_mean_diff->GetMean();
    pedStudy.mean_diff_min     = int(mean_diff_min);
    pedStudy.mean_diff_max     = int(mean_diff_max);
    pedStudy.grE_mean_vs_event = grE_mean_vs_event;
    pedStudy.gr_mean_diff      = gr_mean_diff;
    pedStudy.h_mean            = h_mean;
    pedStudy.h_mean_diff       = h_mean_diff;
    //cout << pedStudy.h_mean << " " << pedStudy.h_mean->GetEntries() << " " << pedStudy.h_mean->GetTitle() << endl;
    pedStudyList.push_back(pedStudy);
    
  } // End of end_of_channel
  
  
  map_grE_meanVevent.clear();
  map_hist_ped.clear();
  
  
  f->Close();
  
  return pedStudyList;
}

std::vector< ADC_Channel > 
DAQUtilities::main_loop_compressedMode(TString fileName, TString sortby){
  
  //vector<std::pair<int,ADC_Channel> > adcList;
  vector< ADC_Channel > adcList;
  
  // Number of Samples in an event that need to be above the trigger threshold to be counter as a signal
  // int triggerCounts = 2;
  
  
  cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  
  
  TFile* f = new TFile(fileName);
  // Pulse Integral
  TTree* trPI = (TTree*)f->Get("Df250PulseIntegral");
  // Pulse Time
  TTree* trPT = (TTree*)f->Get("Df250PulseTime");
  // Trigger Time
  TTree* trTT = (TTree*)f->Get("Df250TriggerTime");
  
  // Pulse Integral branches
  uint32_t channelnum;         /// Arbitrary global channel number
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  uint32_t itrigger; ///trigger number for cases when this hit was read in a multi-event block (DDAQAddress)
  uint32_t integral;         /// Sum of all samples in the waveform
  uint32_t pedestal;             /// Pedestal, Avg 1st 4 samples
  uint32_t i_quality_factor;    // Quality factor for Pulse Integral
  uint32_t i_pulse_number;    // Pulse number for Pulse Integral
  // Pulse Time branches
  uint32_t t_quality_factor;    // Quality factor for Pulse Time
  uint32_t t_pulse_number;    // Pulse number for Pulse Time
  uint32_t timing;     /// The pulse arrival in samples based on the IU method
  // Trigger Time branches
  uint32_t trig_time;     /// The pulse arrival for trigger
  
  trPI->SetBranchAddress("channelnum",&channelnum);
  trPI->SetBranchAddress("eventnum",&eventnum);
  trPI->SetBranchAddress("rocid",&rocid);
  trPI->SetBranchAddress("slot",&slot);
  trPI->SetBranchAddress("channel",&channel);
  trPI->SetBranchAddress("itrigger",&itrigger);
  trPI->SetBranchAddress("integral",&integral);
  trPI->SetBranchAddress("pedestal",&pedestal);
  trPI->SetBranchAddress("quality_factor",&i_pulse_number);
  trPI->SetBranchAddress("pulse_number",&i_pulse_number);
  
  trPT->SetBranchAddress("quality_factor",&t_pulse_number);
  trPT->SetBranchAddress("pulse_number",&t_pulse_number);
  trPT->SetBranchAddress("time",&timing);
  
  trTT->SetBranchAddress("time",&trig_time);
  
  
  int prev_channel = 0;
  
  Int_t nsize = (Int_t) trPI->GetEntries();


  trPI->GetEntry(0);
  first_event = eventnum;
  
  trPI->GetEntry(nsize-1);
  last_event = eventnum;
  
  TString T_last = StringUtilities::int2TString(last_event);
  
  cerr << "First Event = "  << first_event  << endl;
  cerr << "Last Event = "   << last_event   << endl;

  for (Int_t i=0; i<nsize; i++){
    trPI->GetEntry(i);
    trPT->GetEntry(i);


    // Checks that every adc is being read by the DAQ
    if (channel!=0){
      if (prev_channel+1!=int(channel)){
        cerr << "Missing an ADC Channel" << endl;
        cerr << "Previous channel = " << prev_channel << endl;
        cerr << "Current Channel  = " << channel << endl;
        cerr << "\t" << rocid << "/" << slot << "/" << channel << endl;
        //exit(EXIT_SUCCESS);
      }
    }
    prev_channel = channel;
    
    if (i%100000==0)
      cout << i << endl;
    
    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    
    
    ADC_Channel my_channel(rocid-10,slot,channel,eventnum);
    
    if (get_coordinates){
      char tmp_name[50];
      sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
      std::pair<int,int> loc_pair = map_loc.find(tmp_name)->second;
      my_channel.x = loc_pair.first;
      my_channel.y = loc_pair.second;
    }
    
    char temp_absNum[100];
    sprintf(temp_absNum,"%02i_%02i_%02i_",my_channel.crate,my_channel.slot,my_channel.channel);
    TString t_temp_absNum = temp_absNum;
    t_temp_absNum += "_"+StringUtilities::int2TString(eventnum,T_last.Length()+1);
    my_channel.absNum = t_temp_absNum;
    
    my_channel.pedestal   = pedestal;
    my_channel.wIntegral  = integral;
    my_channel.channelnum = channelnum;
    // time is in units of 62.5 pico second, below is the conversion to samples
    my_channel.timing = double(timing)*62.5/1000.0/4.0;
    my_channel.charge     = my_channel.wIntegral - my_channel.pedestal*50;
    
    adcList.push_back(my_channel);
    

  } // End loop over tree
  
  f->Close();
  
  return DAQUtilities::sortADC(adcList, sortby);
}
///////////////////////addesd by Adesh for integral/peak ratio study
std::vector< ADC_Channel > 
DAQUtilities::main_loop_intvspeak(TString fileName, TString sortby, bool mark_PedTimeCalc, bool debug, bool reduce, int threshold, int NSB, int NSA, bool compute_integral){
  
  //vector<std::pair<int,ADC_Channel> > adcList;
  vector< ADC_Channel > adcList;
  
  // Number of Samples in an event that need to be above the trigger threshold to be counter as a signal
  // int triggerCounts = 2;
  
  
  //cout << "Loading Base Location Map" << endl;
  map<TString,std::pair<int,int> > map_loc = GeneralUtilities::BaseLocationMap();
  
  
  
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("Df250WindowRawData");
  
  // Pulse Integral
  TTree* trPI = (TTree*)f->Get("Df250PulseIntegral");
  // Pulse Pedestal
  TTree* trPP = (TTree*)f->Get("Df250PulsePedestal");
  
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
  
  // Pulse Pedestal branch
  uint32_t pulse_peak;    // Peak height
  
  // Pulse Integral branch
   uint32_t pulse_integral;    // Pulse integral
   uint32_t pulse_pedestal;    // Pulse pedestal   
   uint32_t pulse_nsamples;    // Pulse nsamples
     
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
  
  trPP->SetBranchAddress("pulse_peak",&pulse_peak);
 
  trPI->SetBranchAddress("integral",&pulse_integral);
  trPI->SetBranchAddress("pedestal",&pulse_pedestal);
  trPI->SetBranchAddress("nsamples_integral",&pulse_nsamples);    
    
  int prev_channel = 0;
  
  Int_t nsize = (Int_t) tr->GetEntries();


  tr->GetEntry(0);
  first_event = eventnum;
  
  tr->GetEntry(nsize-1);
  last_event = eventnum;
  
  TString T_last = StringUtilities::int2TString(last_event);
  
  //cerr << "First Event = "  << first_event  << endl;
  //cerr << "Last Event = "   << last_event   << endl;

  for (Int_t i=0; i<nsize; i++){
    tr->GetEntry(i);
    trPP->GetEntry(i);
    trPI->GetEntry(i);
    // Checks that every adc is being read by the DAQ
    if (channel!=0){
      if (prev_channel+1!=int(channel)){
        //cerr << "Missing an ADC Channel" << endl;
        //cerr << "Previous channel = " << prev_channel << endl;
        //cerr << "Current Channel  = " << channel << endl;
        //cerr << "\t" << rocid << "/" << slot << "/" << channel << endl;
        //exit(EXIT_SUCCESS);
      }
    }
    prev_channel = channel;
    
    if (i%100000==0)
      //cout << i << endl;
    
    /*
    rocid = StudyBases::crateTranslation(rocid,north);
    if (rocid>10){
      rocid -= 11;
      if (rocid>5){
        rocid -= 6;
      }
    }
    */

    // skip non fcal crates
    if (GeneralUtilities::KeepFCALOnly) continue;  
    if (rocid<11 || rocid>23) continue;

    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    
    if (!GeneralUtilities::KeepADC(rocid-10,slot,channel))
      continue;
      
    // This is temporary
    if (eventnum>10) break;
      
    int overflow = 0;
    int wsize = waveform->size();
    vector<uint32_t> my_waveform;
    for (int ii=0; ii<wsize; ii++){
      uint32_t adc_value = (*waveform)[ii];
      if (adc_value>=4096) overflow = 1;
      if (adc_value>=4096 && GeneralUtilities::PrintOverflows) cerr << "ADC Exceeds 4095: " << rocid << "/" << slot << "/" << channel << "\t" << adc_value << "\t" << eventnum << endl;
      
      if (adc_value<4096 && GeneralUtilities::RemoveOverflows)  my_waveform.push_back(adc_value); 
      if (!GeneralUtilities::RemoveOverflows) my_waveform.push_back(adc_value); 
    }
    

    ADC_Channel my_channel(rocid-10,slot,channel,eventnum);

    my_channel.overFlow = overflow;

    if (get_coordinates){
      char tmp_name[50];
      sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
      std::pair<int,int> loc_pair = map_loc.find(tmp_name)->second;
      my_channel.x = loc_pair.first;
      my_channel.y = loc_pair.second;
    }
    
    my_channel.channelnum = channelnum;
    
    if (!mark_PedTimeCalc)
      my_channel.timing = calcTiming(my_waveform, my_channel.crate, slot, channel, debug);
    if (mark_PedTimeCalc)
      my_channel.timing = w_time;
    
    if (get_waveform)
      my_channel.waveform = my_waveform;
    
    char temp_absNum[100];
    sprintf(temp_absNum,"%02i_%02i_%02i_",my_channel.crate,slot,channel);
    TString t_temp_absNum = temp_absNum;
    t_temp_absNum += "_"+StringUtilities::int2TString(eventnum,T_last.Length()+1);
    my_channel.absNum = t_temp_absNum;
    
    if (!mark_PedTimeCalc)  my_channel.pedestal   = calcPedestal(my_waveform, my_channel.crate, slot, channel, debug);
    if ( mark_PedTimeCalc)  my_channel.pedestal   = double(w_ped)/double(10);
    
    my_channel.wIntegral  = w_integral;
    my_channel.minADC = TMath::MinElement(my_waveform.size(),&my_waveform[0]);
    
    if (!compute_integral) {
      my_channel.charge     = my_channel.wIntegral - my_channel.pedestal*nsamples;
      my_channel.maxADC = TMath::MaxElement(my_waveform.size(),&my_waveform[0]) - my_channel.pedestal;
    }
    if ( compute_integral) {
      my_channel.charge  = calcEnergySum(*waveform, threshold, NSB, NSA, int(my_channel.pedestal));
      nsamples = NSB+NSA;
      my_channel.maxADC = calcTriggerPeak(*waveform, threshold, NSB, NSA, int(my_channel.pedestal));
      //my_channel.ratio = my_channel.wIntegral/my_channel.maxADC;
      //cout<< "Max= " << my_channel.maxADC << endl;
      //cout<< "Ratio= " << my_channel.ratio << endl;
     // cout<< my_channel.wIntegral <<"\t"<<my_channel.maxADC<<"\t"<< my_channel.ratio << endl;
    }
    
    my_channel.eSum = calcEnergySum(my_waveform, threshold, NSB, NSA);
    my_channel.nSamples = nsamples;
    my_channel.rms = TMath::RMS(my_waveform.size(),&my_waveform[0]);
    my_channel.pintegral = double(pulse_integral); 
    my_channel.ppedestal = double(pulse_pedestal);
    my_channel.pnsamples = double(pulse_nsamples);
    my_channel.ppeak = double(pulse_peak);
    my_channel.peakint = double(pulse_integral) - (double(pulse_pedestal) * double(pulse_nsamples));
    my_channel.peakht = double(pulse_peak) - double(pulse_pedestal);
     
//cout << pulse_integral <<"\t"<< pulse_pedestal <<"\t"<< pulse_nsamples <<"\t"<< pulse_peak << endl;
    if (reduce && my_channel.charge>my_channel.pedestal) adcList.push_back(my_channel);
    
    if (!reduce) adcList.push_back(my_channel);
    
    //cout<< "Max= " << my_channel.maxADC << endl;
  } // End loop over tree
  
  
  
  f->Close();
  
  return DAQUtilities::sortADC(adcList, sortby);
}

///////////////////////////

std::vector< ADC_Channel > 
DAQUtilities::main_loop(TString fileName, TString sortby, bool mark_PedTimeCalc, bool debug, bool reduce, int threshold, int NSB, int NSA, bool compute_integral){
  
  //vector<std::pair<int,ADC_Channel> > adcList;
  vector< ADC_Channel > adcList;
  
  // Number of Samples in an event that need to be above the trigger threshold to be counter as a signal
  // int triggerCounts = 2;
  
  
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
  
  
  int prev_channel = 0;
  
  Int_t nsize = (Int_t) tr->GetEntries();


  tr->GetEntry(0);
  first_event = eventnum;
  
  tr->GetEntry(nsize-1);
  last_event = eventnum;
  
  TString T_last = StringUtilities::int2TString(last_event);
  
  //cerr << "First Event = "  << first_event  << endl;
  //cerr << "Last Event = "   << last_event   << endl;

  for (Int_t i=0; i<nsize; i++){
    tr->GetEntry(i);


    // Checks that every adc is being read by the DAQ
    if (channel!=0){
      if (prev_channel+1!=int(channel)){
        //cerr << "Missing an ADC Channel" << endl;
        //cerr << "Previous channel = " << prev_channel << endl;
        //cerr << "Current Channel  = " << channel << endl;
        //cerr << "\t" << rocid << "/" << slot << "/" << channel << endl;
        //exit(EXIT_SUCCESS);
      }
    }
    prev_channel = channel;
    
    if (i%100000==0)
      //cout << i << endl;
    
    /*
    rocid = StudyBases::crateTranslation(rocid,north);
    if (rocid>10){
      rocid -= 11;
      if (rocid>5){
        rocid -= 6;
      }
    }
    */

    // skip non fcal crates
    if (GeneralUtilities::KeepFCALOnly) continue;  
    if (rocid<11 || rocid>23) continue;

    if (GeneralUtilities::RemoveADC(rocid, slot, channel) && !GeneralUtilities::CalibrateDAC)
      continue;
    
    if (!GeneralUtilities::KeepADC(rocid-10,slot,channel))
      continue;
    int overflow = 0;
    int wsize = waveform->size();
    vector<uint32_t> my_waveform;
    for (int ii=0; ii<wsize; ii++){
      uint32_t adc_value = (*waveform)[ii];
      if (adc_value>=4096) overflow = 1;
      if (adc_value>=4096 && GeneralUtilities::PrintOverflows) cerr << "ADC Exceeds 4095: " << rocid << "/" << slot << "/" << channel << "\t" << adc_value << "\t" << eventnum << endl;
      
      if (adc_value<4096 && GeneralUtilities::RemoveOverflows)  my_waveform.push_back(adc_value); 
      if (!GeneralUtilities::RemoveOverflows) my_waveform.push_back(adc_value); 
    }
    

    ADC_Channel my_channel(rocid-10,slot,channel,eventnum);

    my_channel.overFlow = overflow;

    if (get_coordinates){
      char tmp_name[50];
      sprintf(tmp_name,"%i/%i/%i",rocid-11,slot,channel);
      std::pair<int,int> loc_pair = map_loc.find(tmp_name)->second;
      my_channel.x = loc_pair.first;
      my_channel.y = loc_pair.second;
    }
    
    my_channel.channelnum = channelnum;
    
    if (!mark_PedTimeCalc)
      my_channel.timing = calcTiming(my_waveform, my_channel.crate, slot, channel, debug);
    if (mark_PedTimeCalc)
      my_channel.timing = w_time;
    
    if (get_waveform)
      my_channel.waveform = my_waveform;
    
    char temp_absNum[100];
    sprintf(temp_absNum,"%02i_%02i_%02i_",my_channel.crate,slot,channel);
    TString t_temp_absNum = temp_absNum;
    t_temp_absNum += "_"+StringUtilities::int2TString(eventnum,T_last.Length()+1);
    my_channel.absNum = t_temp_absNum;
    
    if (!mark_PedTimeCalc)  my_channel.pedestal   = calcPedestal(my_waveform, my_channel.crate, slot, channel, debug);
    if ( mark_PedTimeCalc)  my_channel.pedestal   = double(w_ped)/double(10);
    if (!compute_integral) my_channel.wIntegral  = w_integral;
    if ( compute_integral) my_channel.wIntegral  = calcEnergySum(*waveform, threshold, NSB, NSA);
    my_channel.maxADC = TMath::MaxElement(my_waveform.size(),&my_waveform[0]) - my_channel.pedestal;
    my_channel.minADC = TMath::MinElement(my_waveform.size(),&my_waveform[0]);
    my_channel.charge     = my_channel.wIntegral - my_channel.pedestal*nsamples;
    my_channel.eSum = calcEnergySum(my_waveform, threshold, NSB, NSA);
    my_channel.nSamples = nsamples;
    my_channel.rms = TMath::RMS(my_waveform.size(),&my_waveform[0]);

    if (reduce && my_channel.charge>my_channel.pedestal) adcList.push_back(my_channel);
    
    if (!reduce) adcList.push_back(my_channel);
    

  } // End loop over tree
  
  
  
  f->Close();
  
  return DAQUtilities::sortADC(adcList, sortby);
}

double 
DAQUtilities::calcPedestal(std::vector<uint32_t> waveform, int rocid, int slot, int channel, bool debug){
  double average_ped = 0;
  double nPed = 5.0;
  for (int i=0; i<int(nPed); i++){
    average_ped += waveform[i];
    if (debug && rocid==5&&slot==5&&channel==5){
      cerr << "\n" << endl;
      cerr << "Crate/Slot/Channel: " << rocid << "/" << slot << "/" << channel << endl;
      cerr << "Avg: " << average_ped << endl;
    }
  }
  if (debug) cerr << "Final Avg: " << average_ped / nPed << endl;
  return average_ped / nPed;
}

int 
DAQUtilities::calcTiming(std::vector<uint32_t> waveform, int rocid, int slot, int channel, bool debug){
  uint32_t wsize = waveform.size();
  
  uint32_t max_adc = TMath::MaxElement(wsize,&waveform[0]);
  uint32_t max_samp = TMath::LocMax(wsize,&waveform[0]);
  uint32_t adc1 = 0;
  uint32_t adc2 = max_adc;
  uint32_t samp1 = 0;
  uint32_t samp2 = 0;
  
  for (uint32_t ii=0; ii<wsize; ii++){
    
    
    uint32_t adc_value = waveform[ii];
    
    if (debug && rocid==0&&slot==5&&channel==14){
      cerr << adc_value << "\t" << ii << endl;
    }
    
    //if (adc_value == max_adc)  max_samp = ii;

    if (adc_value < max_adc && ii < max_samp){

      if (adc_value < int(double(max_adc)/2.0+50.0+0.5)){
        if (adc1 < adc_value){
          adc1 = adc_value;
          samp1 = ii;
        }
      }


      //if (adc_value > max_adc/2){
      //  if (adc2 > adc_value){
      //    adc2 = adc_value;
      //    samp2 = ii;
      //  }
      //}
    }

  }// End loop over waveform
  
  samp2 = samp1 + 1;
  for (uint32_t ii=0; ii<wsize; ii++){
    uint32_t adc_value = waveform[ii];
    if (ii==samp2) adc2 = adc_value;
  }
  
  double aL = (double(adc2)-double(adc1))/4.0;
  double bL = double(adc2) - aL*double(samp2)*4.0;
  double t0 = (double(max_adc)/2 - bL) / aL;
  
  if (debug && rocid==0&&slot==5&&channel==14){
    cerr << "\n" << endl;
    cerr << "Crate/Slot/Channel: " << rocid << "/" << slot << "/" << channel << endl;
    cerr << "Max ADC: " << max_adc << ", Lower side: " << adc1 << ", Upper side: " << adc2 << endl;
    cerr << "Max Samp: " << max_samp << ", Lower side: " << samp1 << ", Upper side: " << samp2 << endl;
    cerr << "aL: " << aL << ", bL: " << bL << ", t0: " << t0 << endl;
  }
  return int(t0+0.5);
}


double 
DAQUtilities::calcEnergySum(std::vector<uint32_t> waveform, int threshold, int NSB, int NSA, int PEDESTAL){
  int nSize = waveform.size();
  //cout << "New Calculation" << endl;
  int TC = 0;
  // Get TC (time of threshold crossing)
  for (int i=0; i<nSize; i++){
    if (waveform[i]>threshold) {TC = i; break;}
  }
  
  double sum = 0;
  int lower_bound = TC-NSB;
  int upper_bound = TC+NSA;
  if (TC==0) lower_bound=0;
  else if (TC==nSize-1) upper_bound = nSize-1;
  
  if (lower_bound<0) lower_bound=0;
  if (upper_bound>=nSize) upper_bound = nSize-1;
  
  for (int i=lower_bound; i<=upper_bound; i++){
    double temp = double(waveform[i])-double(PEDESTAL);
    sum += temp;
    //cout << "Val = " << waveform[i] << ", PED = " << PEDESTAL << ", Sum = " << sum << endl;
  }
  return sum;
}

double 
DAQUtilities::calcTriggerPeak(std::vector<uint32_t> waveform, int threshold, int NSB, int NSA, int PEDESTAL){
  int nSize = waveform.size();
  //cout << "New Calculation" << endl;
  int TC = 0;
  // Get TC (time of threshold crossing)
  for (int i=0; i<nSize; i++){
    if (waveform[i]>threshold) {TC = i; break;}
  }
  
  double max = 0;
  int lower_bound = TC-NSB;
  int upper_bound = TC+NSA;
  if (TC==0) lower_bound=0;
  else if (TC==nSize-1) upper_bound = nSize-1;
  
  if (lower_bound<0) lower_bound=0;
  if (upper_bound>=nSize) upper_bound = nSize-1;
  
  for (int i=lower_bound; i<=upper_bound; i++){
    double temp = double(waveform[i])-double(PEDESTAL);
    if (temp>max) max=temp;
    //cout << "Val = " << waveform[i] << ", PED = " << PEDESTAL << ", Max = " << max << endl;
  }
  return max;
}


// **************************
// Algorithm For Searching For Cosmic Tracks
// **************************
void 
DAQUtilities::getCosmics(TString fileName, double triggerCharge, int nhitscut, int nColumns, bool ped_time){
  
  std::vector< ADC_Channel > adcList = main_loop(fileName, "absnum", ped_time, false, true);
  
  //TH1F* hist = new TH1F("hist","hist",8192,-4096,4096);
  
  map< std::pair<int,int>, double> coord_charge;
  std::vector< ADC_Channel > cosmicList;
  
  std::vector< std::pair<int, std::vector< ADC_Channel > > > cosmicByHits;
  
  for (uint32_t i=0; i<adcList.size(); i++){
    ADC_Channel mychannel = adcList[i];
    int event = mychannel.event;
    int channelnum = mychannel.channelnum;
    
    int crate = mychannel.crate;
    int slot = mychannel.slot;
    int channel = mychannel.channel;
    
    int x = mychannel.x;
    int y = mychannel.y;
    
    double charge   = mychannel.charge;
    
    // Compare the current with the next entry in the vector
    bool end_of_file = (i==adcList.size()-1) ? true : false;
    bool end_of_event = false;
    if (!end_of_file){
      ADC_Channel next_myevent = adcList[i+1];
      int next_event = next_myevent.event;
      end_of_event = (next_event==event) ? false : true;
    }
    
    if (charge>triggerCharge){
      
      ADC_Channel mycosmics(crate,slot,channel,event);
      mycosmics.x = x;
      mycosmics.y = y;
      mycosmics.charge = charge;
      //cerr << mycosmics.crate << "/" << mycosmics.slot << "/" << mycosmics.channel << "\t (" << mycosmics.x << "," << mycosmics.y << ")\t" << mycosmics.charge << endl;
      cosmicList.push_back(mycosmics);
    }
    
    if (end_of_file || end_of_event){
      std::pair<int, std::vector< ADC_Channel > > cosmic_pair;
      cosmic_pair = std::make_pair(cosmicList.size(),cosmicList);
      cosmicByHits.push_back(cosmic_pair);
      
      cosmicList.clear();
    }
    
    if (!end_of_file && !end_of_event){
      
    }
  }
  
  std::sort(cosmicByHits.begin(),cosmicByHits.end(),sortByCosmicHits_DSC());
  
  bool vert_track = false;
  
  std::vector<int> xList;
  
  int nHits = cosmicByHits.size();
  int nC = 100;
  int counter = 0;
  
  cout << "Total: " << nHits << endl;
  
  // Get Top 10 Cosmic Events
  TCanvas* nCanvas[nC];
  for (int i=0; i<nHits; i++){
    std::pair<int, std::vector< ADC_Channel > > mycosmics = cosmicByHits[i];
    int nHits = mycosmics.first;
    std::vector< ADC_Channel > cosmicList2 = mycosmics.second;
    if (nHits==0) continue;
    cerr << "Number of cosmic hits: " << nHits << endl;
    
    std::map< std::pair<int,int>, double> detector_map;
    for (int ii=0; ii<cosmicList2.size(); ii++){
      double charge = cosmicList2[ii].charge;
      int x = cosmicList2[ii].x;
      int y = cosmicList2[ii].y;
      xList.push_back(x);
      std::pair<int,int> coord;
      coord = std::make_pair(x,y);
      detector_map[coord] = charge;
      cerr << cosmicList2[ii].crate << "/" << cosmicList2[ii].slot << "/" << cosmicList2[ii].channel << "\t(" << x << "," << y << "):\t" << charge << "\tevent:\t" << cosmicList2[ii].event <<  endl;
    }
    int xMax = TMath::MaxElement(xList.size(), &xList[0]);
    int xMin = TMath::MinElement(xList.size(), &xList[0]);
    if (xMax-xMin < nColumns && nHits>nhitscut) vert_track = true;
    else vert_track = false;
    xList.clear();
    
    if ( ( vert_track || i<=20 ) ){
      
      if (counter<101){
        nCanvas[counter] = new TCanvas("nCanvas"+StringUtilities::int2TString(counter+1),"nCanvas"+StringUtilities::int2TString(counter+1),1000,1000);
        nCanvas[counter]->cd();
        TH2F* hist2D = PlotUtilities::Plot2D(detector_map, -5, 30, -30, 30, 0, -1);
        hist2D->SetTitle("Rank: "+StringUtilities::int2TString(i+1));
        hist2D->Draw("colz");
        cerr << endl;
      }
      counter++;
    }
    
    //plot2D(detector_map, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
    
  }
  cout << "Total col: " << counter << endl;
  
  //TCanvas* c1 = new TCanvas("c1","c1",600,600);
  //c1->cd();
  //hist->Draw();
}

std::vector< ADC_Channel > DAQUtilities::getMeanPedestal(std::vector< ADC_Channel > vec_adc){
  
  vector< ADC_Channel > vec_ped;
  vector<TString> failedConvergence;
  
  TH1F* hist = 0x0;
  for (int i=0; i<vec_adc.size(); i++){
    ADC_Channel MY_ADC = vec_adc[i];
    int MY_CRATE    = MY_ADC.crate;
    int MY_SLOT     = MY_ADC.slot;
    int MY_CHANNEL  = MY_ADC.channel;
    
    //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
    
    bool fit = false;
    
    if (hist==0x0){
      hist = new TH1F("hist","Histogram of Pedestals", 4096, 0, 4096);
    }
    
    if ( i+1!=int(vec_adc.size()) ){
      ADC_Channel MY_NEXT_ADC = vec_adc[i+1];
      int MY_NEXT_CRATE    = MY_NEXT_ADC.crate;
      int MY_NEXT_SLOT     = MY_NEXT_ADC.slot;
      int MY_NEXT_CHANNEL  = MY_NEXT_ADC.channel;
      
      // Check if the previous element is the same crate/slot/channel as the current element
      // If they are then fill a histogram with pedestal values
      if (MY_CRATE==MY_NEXT_CRATE && MY_SLOT==MY_NEXT_SLOT && MY_CHANNEL==MY_NEXT_CHANNEL){
        fit = false;
      }
      
      // If Not then fit a gaussian and record mean
      if (MY_CRATE!=MY_NEXT_CRATE || MY_SLOT!=MY_NEXT_SLOT || MY_CHANNEL!=MY_NEXT_CHANNEL){
        fit = true;
      }
    }
    
    if ( i+1==int(vec_adc.size()) ){
      fit = true;
    }
    
    if (!fit){
      //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
      hist->Fill(MY_ADC.pedestal);
    }
    
    if (fit){
      //cerr << "Fitting" << endl;
      //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
      hist->Fill(MY_ADC.pedestal);
      hist->Fit("gaus","Q0");
      TString status = gMinuit->fCstatu;
      TF1* fitDac = hist->GetFunction("gaus");
      
      ADC_Channel MY_FINAL_ADC(MY_CRATE,MY_SLOT,MY_CHANNEL,0);
      
      if (!(status.Contains("CONVERGED"))) {
        char message[100];
        sprintf(message,"Crate %i Slot %i Channel %i",MY_CRATE,MY_SLOT,MY_CHANNEL);
        failedConvergence.push_back(message);
        MY_FINAL_ADC.pedestal = hist->GetMean();
      } else{
        MY_FINAL_ADC.pedestal = fitDac->GetParameter(1);
      }
      vec_ped.push_back(MY_FINAL_ADC);
      hist = 0x0;
    }
  }
  
  cerr << "Failed Fits" << endl;
  for (int i=0; i<failedConvergence.size(); i++){
    cerr << failedConvergence[i] << endl;
  }
  
  return vec_ped;
  
}

// The algorithm below assumes the map is ordered
void writeRocConfig(std::map<TString,int> calib_map, TString dir, TString version){
  
  ofstream* outFile = NULL;
  
  if (!dir.EndsWith("/")) dir+="/";
  dir += "rocfcal";
  
  int crate_counter = 0;
  // Slots go from 3-10 13-18 or 19
  int slot_counter = 3;
  for (std::map<TString,int>::const_iterator iter=calib_map.begin(); iter!=calib_map.end(); ++iter){
    // key: crate_slot_channel
    vector<TString> parseTString = StringUtilities::parseTString(iter->first,"_");
    int crateNum = StringUtilities::TString2int(parseTString[0]);
    
    //cout << crate_counter << endl;
    if (crate_counter==0) crate_counter = crateNum;
    //cout << crate_counter << endl;
    //cout << crateNum << endl;
    
    // Below: Create a config file and fill crate configurations
    if (crateNum==crate_counter) {
      
      if ( (crateNum>1&&crateNum<=6) || (crateNum>7&&crateNum<=12)) {*outFile << "\n"; outFile->close();}
      
      char roc[100];
      sprintf(roc,"\n\n\nCRATE \t rocfcal%i",crateNum);
      
      cerr << "Crate: " << crateNum << endl;
      
      string outName = StringUtilities::TString2string(dir+StringUtilities::int2TString(crateNum)+"_fadc250_ver"+version+".cnf");
      cerr << outName << endl;
      outFile = new ofstream(outName.c_str());
      
      // Read in header config
      ifstream header("config_header.cnf");
      string line;
      if (header.is_open()){
        while (getline(header,line)){
          *outFile << line << endl;
        }
      }
      
      *outFile << roc << endl;
      
      crate_counter++;
      slot_counter = 3;
    }
    
    // Below: Fill slot configurations
    int slotNum = StringUtilities::TString2int(parseTString[1]);
    if (slotNum == slot_counter){
      
      cerr << "Slot: " << slotNum << endl;
      
      *outFile << "\n\n\n\n############################" << endl;
      // Write slot
      *outFile << "FADC250_SLOTS\t" << slotNum << endl;
      *outFile << "#########################\n\n" << endl;
      
      *outFile << "FADC250_ALLCH_DAC\t";
      
      if (slotNum==10) slot_counter += 3;
      else slot_counter++;
    }
    
    // Below: Fill appropriate DAC values for each channel
    cerr << "Channel: " << parseTString[2] << endl;
    *outFile << iter->second << "\t";
    
    
  }
  
  *outFile << "\n";
  
  outFile->close();
  
}

void DAQUtilities::calibrateDAC(TString dac3200, TString dac3300, TString dir, TString version){
  
  GeneralUtilities::CalibrateDAC = true;
  // main_loop(TString fileName, TString sortby = "channel, event, slot, or absnum", bool mark_PedTimeCalc = true, bool debug = false, bool reduce = false, TString runMode = "1")
  std::vector< ADC_Channel > vec_3200 = main_loop(dac3200, "absnum", true, false, false);
  std::vector< ADC_Channel > vec_adc3200 = DAQUtilities::getMeanPedestal(vec_3200);
  
  std::vector< ADC_Channel > vec_3300 = main_loop(dac3300, "absnum", true, false, false);
  std::vector< ADC_Channel > vec_adc3300 = DAQUtilities::getMeanPedestal(vec_3300);
  
  std::map<TString,int> calib_map;
  
  TH1F* h_slope = new TH1F("h_slope","Histogram of DAC slopes",3000,-1.5,1.5);
  
  string outName = "dac_";
  if (dac3200.Contains("n6")) outName += "north_";
  if (dac3200.Contains("s6")) outName += "south_";
  outName += "slopes.txt";
  ofstream slopes_file(outName.c_str());
  
  for (int i=0; i<vec_adc3200.size(); i++){
    
    ADC_Channel ADC_3200 = vec_adc3200[i];
    int CRATE_3200       = ADC_3200.crate;
    int SLOT_3200        = ADC_3200.slot;
    int CHANNEL_3200     = ADC_3200.channel;
    double PEDESTAL_3200 = ADC_3200.pedestal;
    
    ADC_Channel ADC_3300 = vec_adc3300[i];
    int CRATE_3300       = ADC_3300.crate;
    int SLOT_3300        = ADC_3300.slot;
    int CHANNEL_3300     = ADC_3300.channel;
    double PEDESTAL_3300 = ADC_3300.pedestal;
    
    if ( CRATE_3200!=CRATE_3300 || SLOT_3200!=SLOT_3300 || CHANNEL_3200!=CHANNEL_3300 )
      cerr << "Error: Crate or Slot or Channel don't match up!!" << endl;
    
    char absNum[100];
    sprintf(absNum,"%02i_%02i_%02i",CRATE_3200,SLOT_3200,CHANNEL_3200);
    TString t_absNum = absNum;
    cout << t_absNum << endl;
    
    // Calculate New DAC Values
    // (3300-3200)/(mean3300-mean3200) = slope
    //(calib_dac-3300) = slope * (100-mean3300)
    //calib_dac = 33000 + 100/(mean3300-mean3200) * (100-mean3300)
    double SLOPE = 100.0 / (PEDESTAL_3300-PEDESTAL_3200);
    h_slope->Fill(SLOPE);
    slopes_file << CRATE_3200 << "\t" << SLOT_3200 << "\t" << CHANNEL_3200 << "\t" << SLOPE << endl;
    calib_map[t_absNum] = int(3300.0 + SLOPE * (100.0-PEDESTAL_3300) + 0.5);
    //calib_map[t_absNum] = int(PEDESTAL_3200);
  }
  
  slopes_file.close();
  
  TCanvas* c_slope = new TCanvas("c_slope","c_slope",600,600);
  c_slope->cd();
  h_slope->Draw();
  
  writeRocConfig(calib_map, dir, version);
  
  // When finished set this bool to false
  GeneralUtilities::CalibrateDAC = false;
}
