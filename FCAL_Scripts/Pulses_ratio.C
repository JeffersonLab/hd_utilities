#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
#include "TSystem.h"
struct sortByMax_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.maxADC > right.maxADC;
    }
};

struct sortByMax_ASC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.maxADC < right.maxADC;
    }
};

struct sortByRMS_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.rms > right.rms;
    }
};

struct sortByPedestal_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.pedestal > right.pedestal;
    }
};

TH2F* Pulses_singleEventMaxADC(TString fileName, int event){
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  std::map< std::pair<double,double>, double> occupancy;
  
  for (int i=0; i<nList; i++){
    if (adcList[i].event != event) continue;
    occupancy[std::make_pair(adcList[i].x,adcList[i].y)] = adcList[i].maxADC;
    cout << adcList[i].x << "," << adcList[i].y << "\t" << adcList[i].maxADC << endl;
  }
  TH2F* hist2D = PlotUtilities::Plot2D(occupancy);
  hist2D->SetTitle("Event: "+StringUtilities::int2TString(event));
  return hist2D;

}

TGraph* single_channel(TString fileName, int crate, int slot, int channel, int event){

  // Skip all other crate slot channels
  GeneralUtilities::KeepSpecificCrate = crate;
  GeneralUtilities::KeepSpecificSlot = slot;
  GeneralUtilities::KeepSpecificChannel = channel;
  
  // Keep overflow bits
  GeneralUtilities::RemoveOverflows = false;
  
  // Save waveform to ADC_Channel struct
  DAQUtilities::get_waveform = true;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  DAQUtilities::get_waveform = false;
  
  int nEvents = adcList.size();
  //cout << "There are " << nEvents << " Events" << endl;
  
  vector<double> avg_waveform;
  char single_title[50];
  char avg_title[50];
  TGraph* gr     = new TGraph();  
  for (int i=0; i<nEvents; i++){
    ADC_Channel mychannel = adcList[i];
    int myevent = mychannel.event;
    
    if (myevent!=event)
      continue;
    char gr_name[50];
    sprintf(gr_name,"Event %i %i/%i/%i (%i,%i)", event,crate,slot,channel,mychannel.x,mychannel.y);
    gr->SetTitle(gr_name);
    gr->SetName("graph");
    // Get waveform
    vector<uint32_t> waveform = mychannel.waveform;
    int nSamples = waveform.size();
    
    for (int ii=0; ii<nSamples; ii++){
      gr->SetPoint(ii,ii+1,waveform[ii]);
    }
  }
  GeneralUtilities::KeepSpecificCrate = -1;
  return gr;
}

TGraph* single_channel(TString fileName, int x, int y, int event){
  std::map<std::pair<int,int>, TString > loc_inverse = GeneralUtilities::BaseLocationMapInverse();
  TGraph* gr = 0x0;
  if (loc_inverse.count(std::make_pair(x,y))>0){
    // crate/slot/channel (crate starts at 0-11)
    TString daq_loc = loc_inverse.find(std::make_pair(x,y))->second;
    vector<TString> parse_daq = StringUtilities::parseTString(daq_loc,"/");
    int crate = StringUtilities::TString2int(parse_daq[0])+1;
    int slot = StringUtilities::TString2int(parse_daq[1]);
    int channel = StringUtilities::TString2int(parse_daq[2]);
    gr = single_channel(fileName, crate, slot, channel, event);
  } else{
    cout << "Could not find (" << x << "," << y << ")!!" << endl;
  }
  return gr;
}

TGraph* single_channel(vector<uint32_t> waveform, int crate, int slot, int channel, int x, int y, int event){

  char single_title[50];
  char avg_title[50];
  TGraph* gr     = new TGraph(); 
  char gr_name[50];
  sprintf(gr_name,"Event %i %i/%i/%i (%i,%i)", event,crate,slot,channel,x,y);
  gr->SetTitle(gr_name);
  gr->SetName("graph");

  int nSamples = waveform.size();
    
  for (int ii=0; ii<nSamples; ii++){
    gr->SetPoint(ii,ii+1,waveform[ii]);
  }
  return gr;
}

TGraph* single_channel_ppeak(vector<uint32_t> waveform, int crate, int slot, int channel, int x, int y, int event, int pulse_peak){

  char single_title[50];
  char avg_title[50];
  TGraph* gr     = new TGraph(); 
  char gr_name[50];
  sprintf(gr_name,"Event %i %i/%i/%i (%i,%i)", event,crate,slot,channel,x,y);
  gr->SetTitle(gr_name);
  gr->SetName("graph");

  int nSamples = waveform.size();
    
  for (int ii=0; ii<nSamples; ii++){
  if (pulse_peak >200){
    gr->SetPoint(ii,ii+1,waveform[ii]);
  }
  }
  return gr;
}


TGraph* Pulses_oneChannel(TString fileName, int crate, int slot, int channel, int event, TString dir = "", bool plot = true){

  // Skip all other crate slot channels
  GeneralUtilities::KeepSpecificCrate = crate;
  GeneralUtilities::KeepSpecificSlot = slot;
  GeneralUtilities::KeepSpecificChannel = channel;
  
  // Keep overflow bits
  GeneralUtilities::RemoveOverflows = false;
  
  // Save waveform to ADC_Channel struct
  DAQUtilities::get_waveform = true;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  DAQUtilities::get_waveform = false;
  
  int nEvents = adcList.size();
  cout << "There are " << nEvents << " Events" << endl;
  
  vector<double> avg_waveform;
  char single_title[50];
  char avg_title[50];
  TCanvas* c_avg = 0x0;
  TCanvas* c_one = 0x0;
  TGraph* gr_avg = new TGraph();  
  TGraph* gr     = new TGraph();  
  for (int i=0; i<nEvents; i++){
    ADC_Channel mychannel = adcList[i];
    int myevent = mychannel.event;
    
    // Set Title
    if (i==0){
      gr_avg->SetTitle("Average Waveform: "+GeneralUtilities::formatTitle(crate,slot,channel)+" ("+StringUtilities::int2TString(mychannel.x)+","+StringUtilities::int2TString(mychannel.y)+")");
      gr_avg->SetName("Average Waveform: "+GeneralUtilities::formatTitle(crate,slot,channel)+" ("+StringUtilities::int2TString(mychannel.x)+","+StringUtilities::int2TString(mychannel.y)+")");
      gr->SetTitle("Event "+StringUtilities::int2TString(event)+" Waveform: "+GeneralUtilities::formatTitle(crate,slot,channel)+" ("+StringUtilities::int2TString(mychannel.x)+","+StringUtilities::int2TString(mychannel.y)+")");
      gr->SetName("Event "+StringUtilities::int2TString(event)+" Waveform: "+GeneralUtilities::formatTitle(crate,slot,channel)+" ("+StringUtilities::int2TString(mychannel.x)+","+StringUtilities::int2TString(mychannel.y)+")");
    }
    
    // Get waveform
    vector<uint32_t> waveform = mychannel.waveform;
    int nSamples = waveform.size();
    //cout << "NSamples = " << nSamples << endl;
    
    // At first set avg to waveform
    if (i==0){
      for (int ii=0; ii<nSamples; ii++){
        avg_waveform.push_back(waveform[ii]);
      }
      
    } else{ // Then add each sample
      for (int ii=0; ii<nSamples; ii++){
        avg_waveform[ii] += waveform[ii];
      }
    }
    
    // At the end plot the avg waveform
    if (i+1==nEvents){
      for (int ii=0; ii<nSamples; ii++){
        gr_avg->SetPoint(ii,ii+1,avg_waveform[ii]/double(nEvents));
      }
      if (plot){
        sprintf(avg_title,"Average Waveform: Crate %i Slot %i Channel %i",crate,slot,channel);  
        c_avg = new TCanvas(avg_title,avg_title,600,600);
        c_avg->cd();
        gr_avg->Draw("A");
        gr_avg->GetXaxis()->SetTitle("Samples [4ns]");
        gr_avg->GetYaxis()->SetTitle("ADC Counts");
        gr_avg->Draw("AP");
      }
      // Save plots
      if (dir!=""){
        if (!dir.EndsWith("/")) dir += "/";
        c_avg->SaveAs(dir+GeneralUtilities::formatName(fileName)+"_Avg_"+StringUtilities::int2TString(crate)+"-"+StringUtilities::int2TString(slot)+"-"+StringUtilities::int2TString(channel)+".pdf");
        c_one->SaveAs(dir+GeneralUtilities::formatName(fileName)+"_One_"+StringUtilities::int2TString(crate)+"-"+StringUtilities::int2TString(slot)+"-"+StringUtilities::int2TString(channel)+".pdf");
      }
    }
    
    //cout << "MyEvent " << myevent << "\t" << event << endl;
    
    if (myevent!=event)
      continue;
    
    for (int ii=0; ii<nSamples; ii++){
      gr->SetPoint(ii,ii+1,waveform[ii]);
    }
    if (plot){
      sprintf(single_title,"Single Waveform: Crate %i Slot %i Channel %i Event %i",crate,slot,channel,event);
      c_one = new TCanvas(single_title,single_title,600,600);
      c_one->cd();
      gr->Draw("A");
      gr->GetXaxis()->SetTitle("Samples [4ns]");
      gr->GetYaxis()->SetTitle("ADC Counts");
      gr->Draw("AP");
    }
  }
  GeneralUtilities::KeepSpecificCrate = -1;
  return gr;
}

TGraph* Pulses_oneChannel(TString fileName, int x, int y, int event, TString dir = "", bool plot = true){
  std::map<std::pair<int,int>, TString > loc_inverse = GeneralUtilities::BaseLocationMapInverse();
  TGraph* gr = 0x0;
  if (loc_inverse.count(std::make_pair(x,y))>0){
    // crate/slot/channel (crate starts at 0-11)
    TString daq_loc = loc_inverse.find(std::make_pair(x,y))->second;
    vector<TString> parse_daq = StringUtilities::parseTString(daq_loc,"/");
    int crate = StringUtilities::TString2int(parse_daq[0])+1;
    int slot = StringUtilities::TString2int(parse_daq[1]);
    int channel = StringUtilities::TString2int(parse_daq[2]);
    gr = Pulses_oneChannel(fileName, crate, slot, channel, event, dir, plot);
  } else{
    cout << "Could not find that coordinate!!" << endl;
  }
  return gr;
}


void Pulses_find_any_channel(TString fileName, int nPulses, double ADClowerBound, double ADCupperBound, int skipPulses = 0, bool ROverFlows = true){
  
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  int nList = adcList.size();
  
  std::sort(adcList.begin(),adcList.end(),sortByMax_DSC());
  
  int nCanvas = nPulses / 12;
  if ((nPulses % 12) != 0) nCanvas++;
  
  TCanvas* c_all[nCanvas];
  
  int pnum = 0;
  int cnum = 0;

  int pulse_num = 0;

  nPulses += skipPulses;
  
  
  for (int i=0; i<nPulses; i++){    
    
    if (i==nList){
      cout << "No more channels to loop through" << endl;
      break;
    }
      
    ADC_Channel MY_CHANNEL = adcList[i];
    double maxADC = MY_CHANNEL.maxADC+100.0; // to compensate for the pedestal subtraction
    int MY_CRATE = MY_CHANNEL.crate;
    int MY_SLOT  = MY_CHANNEL.slot;
    int MY_CHAN  = MY_CHANNEL.channel;
    int MY_EVENT = MY_CHANNEL.event;
    int x = MY_CHANNEL.x;
    int y = MY_CHANNEL.y;
    int MY_OVERFLOW = MY_CHANNEL.overFlow;
    //cout << MY_OVERFLOW << endl;
    if (ROverFlows && MY_OVERFLOW==1) {nPulses++; continue;} 

    if (maxADC>ADClowerBound && maxADC<ADCupperBound && MY_CHANNEL.crate>=1 && MY_CHANNEL.crate<=12){
      //cout << "Max ADC = " << maxADC << "\t(" << x << "," << y << ")\t Event = " << MY_EVENT<< endl;
      pulse_num++;
      //cout << "Pulse Num = " << pulse_num << endl;
      if (pulse_num<=skipPulses) continue;
      pnum++;
    } 
    else{
      nPulses++; continue;
    }
    
    if (pnum>12) {pnum=1;cnum++;}
    
    if (pnum==1){
      c_all[cnum] = new TCanvas("c_all_"+StringUtilities::int2TString(cnum),"Top Pulses "+StringUtilities::int2TString(cnum+1)+"/"+StringUtilities::int2TString(nCanvas),1000,1000);
      c_all[cnum]->Divide(3,4);
    }
    
    //cout << "cnum = " << cnum << "\t" << "pnum = " << pnum << endl;
    TGraph* gr = single_channel(fileName, MY_CRATE, MY_SLOT, MY_CHAN, MY_EVENT);
    c_all[cnum]->cd(pnum);
    gr->Draw("AP");
    
  }
}


void Pulses_find_one_channel(TString fileName, int nPulses, double ADClowerBound, double ADCupperBound, int crate, int slot, int channel){
  
  
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  int nList = adcList.size();
  //cout << nList << endl;
  
  std::sort(adcList.begin(),adcList.end(),sortByMax_DSC());
  cout << "Sorting by max ADC" << endl;
  
  int nCanvas = nPulses / 12;
  if ((nPulses % 12) != 0) nCanvas++;
  
  TCanvas* c_all[nCanvas];
  
  int pnum = 0;
  int cnum = 0;
  
  for (int i=0; i<nPulses; i++){   

    if (i==nList){
      cout << "No more channels to loop through" << endl;
      break;
    } 

    ADC_Channel MY_CHANNEL = adcList[i];
    double maxADC = MY_CHANNEL.maxADC+100.0; // to compensate for the pedestal subtraction
    int MY_CRATE = MY_CHANNEL.crate;
    int MY_SLOT  = MY_CHANNEL.slot;
    int MY_CHAN  = MY_CHANNEL.channel;
    int MY_EVENT = MY_CHANNEL.event;
    int x = MY_CHANNEL.x;
    int y = MY_CHANNEL.y;

    if (maxADC>ADClowerBound && maxADC<ADCupperBound && MY_CRATE==crate && MY_SLOT==slot && MY_CHAN==channel){
      pnum++;
    } 
    else{
      nPulses++;
      continue;
    }
    
    if (pnum>12) {pnum=1;cnum++;}
    
    if (pnum==1){
      char canv_title[50];
      sprintf(canv_title,"c_%i_%i Pulse %i/%i",x,y,cnum+1,nCanvas);
      c_all[cnum] = new TCanvas(canv_title,canv_title,1000,1000);
      c_all[cnum]->Divide(3,4);
    }

    TGraph* gr = single_channel(fileName, MY_CRATE, MY_SLOT, MY_CHAN, MY_EVENT);
    c_all[cnum]->cd(pnum);
    gr->Draw("AP");
    
  }
  
}

void Pulses_find_one_channel(TString fileName, int nPulses, double ADClowerBound, double ADCupperBound, int x, int y){
  std::map<std::pair<int,int>, TString > loc_inverse = GeneralUtilities::BaseLocationMapInverse();
  TGraph* gr = 0x0;
  if (loc_inverse.count(std::make_pair(x,y))>0){
    // crate/slot/channel (crate starts at 0-11)
    TString daq_loc = loc_inverse.find(std::make_pair(x,y))->second;
    vector<TString> parse_daq = StringUtilities::parseTString(daq_loc,"/");
    int crate = StringUtilities::TString2int(parse_daq[0])+1;
    int slot = StringUtilities::TString2int(parse_daq[1]);
    int channel = StringUtilities::TString2int(parse_daq[2]);
    Pulses_find_one_channel(fileName, nPulses, ADClowerBound, ADCupperBound, crate, slot, channel);
  } else{
    cout << "Could not find that coordinate!!" << endl;
  }
}

// Look at all neigboring channels and plot pulses NOT common to any
std::vector< ADC_Channel > Pulses_studyOneHotChannel(TString fileName, int x, int y, double ADClowerBound, double ADCupperBound, double neighborThreshold, bool plot){
  
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  std::vector< ADC_Channel > hot_channel;
  
  int pulse_num = 0;
  
  int hot_index = 0;

  int n_found = 0;
  bool thisChannel_exists = false;
  for (int k=0; k<nList; k++){
  
    bool plot_pulse = false;

    ADC_Channel MY_CHANNEL = adcList[k];
    int MY_CRATE  = MY_CHANNEL.crate;
    int MY_SLOT   = MY_CHANNEL.slot;
    int MY_CHAN   = MY_CHANNEL.channel;
    int MY_X      = MY_CHANNEL.x;
    int MY_Y      = MY_CHANNEL.y;
    int MY_EVENT  = MY_CHANNEL.event;
    double MY_MAX = MY_CHANNEL.maxADC;
    
    if (MY_X==x && MY_Y==y && MY_MAX>ADClowerBound && MY_MAX<ADCupperBound){
      thisChannel_exists = true;
      hot_index = k;
      //cout << "Channel exists" << endl;
    }
    
    /*
    // There are a max of 8 neighboring channels
    for (int i=-1; i<=1; i++){
      int new_x = 0, new_y = 0;
      for (int j=-1; j<=1; j++){
        if (i==0 && j==0) continue;
        new_x = x+i;
        new_y = y+j;
        
        if (MY_X==new_x && MY_Y==new_y && MY_MAX>neighborThreshold) {
          n_found++;
          //cout << "Found a neighbor" << endl;
        }
        
      }
    }
    */

    if (!thisChannel_exists && MY_MAX>neighborThreshold) n_found++; 
    
    // Check if next event is different from current event
    if (k+1!=nList){
      ADC_Channel MY_NEXT_CHANNEL = adcList[k+1];
      int MY_NEXT_EVENT = MY_NEXT_CHANNEL.event;
      //cout << "Next Event = " << MY_NEXT_EVENT << endl;
      // If next event is different from current event:
      if (MY_NEXT_EVENT!=MY_EVENT) {
        if (n_found>0 && thisChannel_exists){
          //cout << "Found " << n_found << endl;
          n_found = 0;
          thisChannel_exists = false;
        }
        else if (n_found==0 && thisChannel_exists){
          plot_pulse = true;
          n_found = 0;
          thisChannel_exists = false;
          cout << "Cuurent k = " << k << ", Hot k = " << hot_index << endl;
        }
      }
    }
    if (k+1==nList && n_found==0 && thisChannel_exists) plot_pulse = true;
    
    

    if (plot_pulse){
      hot_channel.push_back(adcList[hot_index]);
      pulse_num++;
      //cout << x << "," << y << "\t" << MY_EVENT << endl;
      
    }
    //if (pulse_num==nPulses) {
    //  break;
    //}
  } 
  int nPulses = pulse_num;
  if (nPulses>12) nPulses = 12;
  
  int nCanvas = nPulses / 12;
  if ((nPulses % 12) != 0) nCanvas++;
  
  TCanvas* c_all[nCanvas];
  TCanvas* c_all_2D[nCanvas];
  
  int pnum = 0;
  int cnum = 0;
  
  int nHot = hot_channel.size();
  std::sort(hot_channel.begin(),hot_channel.end(),sortByMax_DSC());
  
  if (plot){
    for (int j=0; j<nHot; j++){
      pnum++;
      //cout << "Pnum = " << pnum << "Pulse num = " << pulse_num << endl;
  
      if (pnum>12) {pnum=1;cnum++;}
      if (pnum==1){
        char canv_title[50];
        sprintf(canv_title,"c_%i_%i Pulse %i/%i",x,y,cnum+1,nCanvas);
        char canv_title2D[50];
        sprintf(canv_title2D,"c_%i_%i Occupancy %i/%i",x,y,cnum+1,nCanvas);
        c_all[cnum] = new TCanvas(canv_title,canv_title,1000,1000);
        c_all[cnum]->Divide(3,4);
        c_all_2D[cnum] = new TCanvas(canv_title2D,canv_title2D,1000,1000);
      }
      TGraph* gr = single_channel(fileName, hot_channel[j].x, hot_channel[j].y, hot_channel[j].event);
      c_all[cnum]->cd(pnum);
      gr->Draw("AP");
      
      c_all_2D[cnum]->cd();
      TH2F* hist2D = Pulses_singleEventMaxADC(fileName, hot_channel[j].event);
      hist2D->Draw("colz");
    }
  }
  return hot_channel;
}


// Look at all neigboring channels and plot pulses NOT common to any
void Pulses_studyHotChannel(TString fileName, double ADClowerBound, double ADCupperBound, double neighborThreshold){
  
  std::map<std::pair<int,int>, TString > map_bases = GeneralUtilities::BaseLocationMapInverse();
  
  std::map< std::pair<int,int>, int > map_hot;
  std::map< std::pair<int,int>, int >::const_iterator myiter;
  
  for(std::map<std::pair<int,int>, TString >::const_iterator iter=map_bases.begin(); iter!=map_bases.end(); ++iter){
    int x = iter->first.first;
    int y = iter->first.second;
    std::vector< ADC_Channel > hotchannel = Pulses_studyOneHotChannel(fileName, x, y, ADClowerBound, ADCupperBound, neighborThreshold, false);
    int nHot = hotchannel.size();
    cout << "Hot Channel " << x << "," << y << " Has " << nHot << " Single Hits" << endl;
    for (int i=0; i<nHot; i++){
      map_hot[iter->first]++;
    } 
  }
  
  for (myiter=map_hot.begin(); myiter!=map_hot.end(); ++myiter){
    cout << "X = " << myiter->first.first << ", Y = " << myiter->first.second << "\t Number = " << myiter->second << endl;
  }
  cout << "\n{";
  for (myiter=map_hot.begin(); myiter!=map_hot.end(); ++myiter){
    cout << myiter->first.first << ",";
  }
  cout << "}\n{";
  for (myiter=map_hot.begin(); myiter!=map_hot.end(); ++myiter){
    cout << myiter->first.second << ",";
  }
  cout << "}";

}

void Pulses_studyHotChannelNew(TString fileName, TString title, double ADClowerBound, double ADCupperBound, double neighborThreshold){
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  std::vector< ADC_Channel > adcListByMax;
  
  std::vector< ADC_Channel > adcListHot;
  
  std::map< std::pair<int,int>, double> mapHot;
  
  for (int i=0; i<nList; i++){
    
    adcListByMax.push_back(adcList[i]);
    
    bool doSort = false;
    
    if (i+1!=nList){
      if (adcList[i].event!=adcList[i+1].event){
        doSort = true;
      }
    }
    
    if (i+1==nList){
      doSort = true;
    }
    
    if (doSort){
      // Sort by Max ADC
      std::sort(adcListByMax.begin(), adcListByMax.end(), sortByMax_DSC());

      bool largeChannel = false;
      bool nextChannel = false;

      // After sorting from biggest to smallest
      // determine if 1st element is in ADC Bounds
      if (adcListByMax[0].maxADC>ADClowerBound && adcListByMax[0].maxADC<ADCupperBound)
        largeChannel = true;
      // Next determine if the next channel is within the neighbor Threshold range
      if (adcListByMax[1].maxADC>neighborThreshold)
        nextChannel = true;

      // Store the single hit channel
      if (largeChannel && !nextChannel){
        adcListHot.push_back(adcListByMax[0]);
        mapHot[std::make_pair(adcListByMax[0].x,adcListByMax[0].y)]++;
      }

      adcListByMax.clear();
    }
  }
  
  // Output Hot Channels
  int nHot = adcListHot.size();
  cout << "There are " << nHot << " Single Hit Channels" << endl;
  for (int i=0; i<nHot; i++){
    cout << "X = " << adcListHot[i].x << "\tY = " << adcListHot[i].y << "\tEvent = " << adcListHot[i].event << endl;
  }

  TCanvas* c_2d = new TCanvas("c_2D","c_2D",1000,1000);
  c_2d->cd();
  TH2F* hist_2D = PlotUtilities::Plot2D(mapHot);
  hist_2D->SetTitle(title);
  hist_2D->Draw("colz");

  adcListHot.clear();
  // Output Sorted Channels
  for (std::map< std::pair<int,int>,double>::const_iterator iter = mapHot.begin(); iter != mapHot.end(); ++iter){
    ADC_Channel my_channel;
    my_channel.x = iter->first.first;
    my_channel.y = iter->first.second;
    my_channel.maxADC = iter->second;
    adcListHot.push_back(my_channel);
  }
  std::sort(adcListHot.begin(), adcListHot.end(), sortByMax_DSC());
  nHot = adcListHot.size();
  for (int i=0; i<nHot; i++){
    cout << "X = " << adcListHot[i].x << "\tY = " << adcListHot[i].y << "\tTimes = " << adcListHot[i].maxADC << endl;
  }
  
}



// Plot the sum of the maximum pulse peak per channel
void Pulses_peak2D(TString fileName, bool excludeOverflows = false){
  DAQUtilities::get_waveform = false;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  int nList = adcList.size();
  
  map< std::pair<int,int>, double> pulsePeak;
  
  for (int i=0; i<nList; i++){
    ADC_Channel MY_CHANNEL = adcList[i];
    if (excludeOverflows && MY_CHANNEL.maxADC>=4096) continue;
    double voltage = MY_CHANNEL.maxADC * 2.0/4096.0;
    pulsePeak[std::make_pair(MY_CHANNEL.x,MY_CHANNEL.y)] += voltage;
  }
  
  TH2F* hist2d = PlotUtilities::Plot2D(pulsePeak);
  hist2d->SetTitle("Pulse Peak Voltage");
  TCanvas* c_2d = new TCanvas(fileName+"_2D",fileName+"_2D",600,600);
  c_2d->cd();
  hist2d->Draw("colz");
}

void Pulses_occupancy(TString fileName, bool excludeOverflows = false){
  DAQUtilities::get_waveform = false;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  int nList = adcList.size();
  
  map< std::pair<int,int>, double> pulsePeak;
  
  for (int i=0; i<nList; i++){
    ADC_Channel MY_CHANNEL = adcList[i];
    if (excludeOverflows && MY_CHANNEL.maxADC>=4096) continue;
    pulsePeak[std::make_pair(MY_CHANNEL.x,MY_CHANNEL.y)] ++;
  }
  
  TH2F* hist2d = PlotUtilities::Plot2D(pulsePeak);
  hist2d->SetTitle("Occupancy");
  TCanvas* c_2d = new TCanvas(fileName+"_occupancy_2D",fileName+"_occupancy_2D",600,600);
  c_2d->cd();
  hist2d->Draw("colz");
}

void Pulses_hist(TString fileName, int crate, int slot, int channel){
  // Skip all other crate slot channels
  GeneralUtilities::KeepSpecificCrate = crate;
  GeneralUtilities::KeepSpecificSlot = slot;
  GeneralUtilities::KeepSpecificChannel = channel;
  
  // Keep overflow bits
  GeneralUtilities::RemoveOverflows = false;
  
  // Save waveform to ADC_Channel struct
  DAQUtilities::get_waveform = true;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "nosort");
  DAQUtilities::get_waveform = false;
  
  int nEvents = adcList.size();
  cout << "There are " << nEvents << " Events" << endl;
  
  vector<double> avg_waveform;
  char c_title[50];
  sprintf(c_title,"Crate %i Slot %i Channel %i",crate,slot,channel);
  TCanvas* c_avg = new TCanvas("c_"+TString(c_title),c_title,600,600);
  TH1F* hist_ped = new TH1F("h"+TString(c_title),"Histogram of ADC", 1000,0,4096);  
  
  int x = 0;
  int y = 0;
  
  int x_max = 0;
  
  for (int i=0; i<nEvents; i++){
    ADC_Channel mychannel = adcList[i];
    if (i==0){
      x = mychannel.x;
      y = mychannel.y;
    }
    // Get waveform
    vector<uint32_t> waveform = mychannel.waveform;
    int nSamples = waveform.size();
    for (int ii=0; ii<nSamples; ii++){
      hist_ped->Fill(waveform[ii]);
      if (waveform[ii] > x_max) x_max = waveform[ii];
    }
  }
  hist_ped->SetTitle(TString(c_title)+" ("+StringUtilities::int2TString(x)+","+StringUtilities::int2TString(y)+")");
  hist_ped->GetXaxis()->SetRangeUser(0,x_max+50);
  c_avg->cd();
  hist_ped->Draw();
  GeneralUtilities::KeepSpecificCrate = -1;
}

void Pulses_hist(TString fileName, int x, int y){
  std::map<std::pair<int,int>, TString > loc_inverse = GeneralUtilities::BaseLocationMapInverse();
  TGraph* gr = 0x0;
  if (loc_inverse.count(std::make_pair(x,y))>0){
    // crate/slot/channel (crate starts at 0-11)
    TString daq_loc = loc_inverse.find(std::make_pair(x,y))->second;
    vector<TString> parse_daq = StringUtilities::parseTString(daq_loc,"/");
    int crate = StringUtilities::TString2int(parse_daq[0])+1;
    int slot = StringUtilities::TString2int(parse_daq[1]);
    int channel = StringUtilities::TString2int(parse_daq[2]);
    Pulses_hist(fileName, crate, slot, channel);
  } else{
    cout << "Could not find that coordinate!!" << endl;
  }
}

/*
Need to better understand what we are triggering on. 
Maybe looking at 2d plots of hits per event and then look at the the traces for those hits will give some clues. 
Use cin to pause for each event
Do the energy sum calculation: Look up NSA and NSB, negative integrals are zeroed
*/
void Pulses_triggerStudy(TString fileName, int start_event, int custom_max_pedSubtracted = 10, bool get_waveform = true, bool sortMaxBigToSmall = true,int threshold=150, int NSB=3, int NSA=10, bool compute_integral = false){
  DAQUtilities::get_waveform = get_waveform;
  //static std::vector< ADC_Channel > main_loop_intvspeak(TString fileName, TString sortby = "channel, event, slot, or absnum", \
    bool mark_PedTimeCalc = true, bool debug = false, bool reduce = false, int threshold=130, int NSB=1, int NSA=2, bool compute_integral = false);
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop_intvspeak(fileName, "event",true,false,false,threshold,NSB,NSA,compute_integral);
  int nList = adcList.size();
  
  std::vector< ADC_Channel > noisy_bases;

  int prevEvent = 0;

  for (int i=0; i<nList; i++){
    
    ADC_Channel MY_CHANNEL = adcList[i];
    int MY_EVENT = MY_CHANNEL.event;
    if (MY_EVENT<start_event) continue;

    if (MY_CHANNEL.maxADC<custom_max_pedSubtracted) continue;
    noisy_bases.push_back(MY_CHANNEL);    
  }

  std::vector< ADC_Channel > noisy_bases2;
  std::map<std::pair<int,int>, double> map_detector;
  std::vector< std::pair<int,int> > coord;
  TCanvas* c_trigger = 0x0;
  
  int nBases = noisy_bases.size();
  cout << "There are " << nBases << " Channels and Events" << endl;
  for (int i=0; i<nBases; i++){
    ADC_Channel MY_CHANNEL = noisy_bases[i];
    int MY_X = MY_CHANNEL.x;
    int MY_Y = MY_CHANNEL.y;
    int MY_EVENT = MY_CHANNEL.event;
    double MY_MAX = MY_CHANNEL.maxADC + MY_CHANNEL.pedestal;
    double MY_INTEGRAL = MY_CHANNEL.charge; // Not Pedestal!
    if (MY_CHANNEL.charge<=0) MY_INTEGRAL = 0;

    map_detector[std::make_pair(MY_X,MY_Y)] = MY_INTEGRAL;
    noisy_bases2.push_back(MY_CHANNEL);
    coord.push_back(std::make_pair(MY_X,MY_Y));

    bool plot = false;
    if (i+1!=nBases) {
      ADC_Channel MY_NEXT_CHANNEL = noisy_bases[i+1];
      int MY_NEXT_EVENT = MY_NEXT_CHANNEL.event;
      if (MY_NEXT_EVENT != MY_EVENT) plot = true;
    }
    if (i+1==nBases) plot = true;
  
  //char outdir[255];
	//sprintf(outdir,"../plots/FCALnoise/%s",plotlabelst.Data());
	//printf("\nCreating directory %s\n",outdir);

	//Int_t result = gSystem->mkdir(outdir,1); // make directory recursively
	//if (result != 0) {
	//	if (result == -1) {
	//		printf("Directory %s already exists\n\n",outdir);
	//	} else {
	//		printf("cannot make directory %s with result %i\n\n",outdir,result);
	//		exit(result);
	//	}
	//} else {
	//	printf("\nCreated directory %s\n\n",outdir);	
	//}


    if (plot){


      c_trigger = new TCanvas("c_trigger","c_trigger",900,900);
      // Plot Pulse Inegral on 2D map of detector
      TH2F* hist2D = PlotUtilities::Plot2D(map_detector);
      hist2D->SetTitle("Pulse Integral: Event "+StringUtilities::int2TString(MY_EVENT));
      c_trigger->cd();
      c_trigger->Update();
      hist2D->Draw("colz");
      c_trigger->cd();
      //c_trigger->Print("%s/pedwidth_%s_R%02iS%02i.png",outdir,plotlabel,crateindex,slotindex);
      //c_trigger->Modified();
      c_trigger->Update();
      //cout << gSystem->ProcessEvents() << endl;

      cout << "Press Enter To Continue:" << endl;
      cin.get();
      if (sortMaxBigToSmall)  {std::sort(noisy_bases2.begin(),noisy_bases2.end(),sortByMax_DSC());}
      if (!sortMaxBigToSmall) {std::sort(noisy_bases2.begin(),noisy_bases2.end(),sortByMax_ASC());}
      
      int nGraph = coord.size();
      cout << "N Graphs = " << nGraph << endl;   
      if (nGraph>12) nGraph=12;
      int nCanvas = nGraph/12;
      if (nGraph%12!=0) nCanvas++;
      TCanvas* c_pulses[nCanvas];
      int pad_counter = 0;
      int cnv_counter = 0;
      
      for (int j=0; j<nGraph; j++){
        pad_counter++;
        if (pad_counter>12) {pad_counter=1; cnv_counter++;}
        int x = noisy_bases2[j].x;
        int y = noisy_bases2[j].y;
        int pulse_peak = noisy_bases2[j].ppeak;
        if (compute_integral){
          cout << "Event = " << MY_EVENT << "\t" << x << "\t" << y << "\t"<<  noisy_bases2[j].charge/noisy_bases2[j].maxADC  <<"\t"<<noisy_bases2[j].charge<<"\t"<<noisy_bases2[j].pedestal<<"\t"<<noisy_bases2[j].nSamples<<"\t" <<noisy_bases2[j].maxADC<<endl;
      
        } else{
          cout << "Event = " << MY_EVENT << "\t" << x << "\t" << y << "\t"<<  noisy_bases2[j].charge/noisy_bases2[j].maxADC  <<"\t"<<noisy_bases2[j].charge<<"\t"<<noisy_bases2[j].pedestal<<"\t"<<noisy_bases2[j].nSamples<<"\t" <<noisy_bases2[j].maxADC<<endl;
        }        
      
        TGraph* gr = 0x0;
        if (!get_waveform) single_channel(fileName, x, y, MY_EVENT);
        //if (get_waveform) gr = single_channel_ppeak(noisy_bases2[j].waveform, noisy_bases2[j].crate, noisy_bases2[j].slot, noisy_bases2[j].channel, x, y, MY_EVENT, pulse_peak);
        if (get_waveform) gr = single_channel(noisy_bases2[j].waveform, noisy_bases2[j].crate, noisy_bases2[j].slot, noisy_bases2[j].channel, x, y, MY_EVENT);
        if (pad_counter==1){
          char cnv_name[50];
          sprintf(cnv_name,"pulses_%i",cnv_counter+1);
          c_pulses[cnv_counter] = new TCanvas(cnv_name, cnv_name, 900,900);
          c_pulses[cnv_counter]->Divide(4,3);
        }
        c_pulses[cnv_counter]->cd(pad_counter);
        gr->Draw("AP");
         c_pulses[cnv_counter]->Update(); 
      }

      //c_trigger->cd();
      //c_trigger->Update();
      
      
      cout << "Press Enter To Continue:" << endl;
      cin.get();

      noisy_bases2.clear();
      map_detector.clear();
      coord.clear();
    }
  
  }
  
}

void Pulses_ringing(TString fileName, int cutOff_RMS, bool get_waveform = false){
  DAQUtilities::get_waveform = get_waveform;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  //std::vector< ADC_Channel > noisy_bases;
  std::map< std::pair<int,int> ,int> map_noisy;
  std::vector< ADC_Channel > noisy_bases_sorted;
  
  for (int i=0; i<nList; i++){
    
    ADC_Channel MY_CHANNEL = adcList[i];
    int MY_EVENT = MY_CHANNEL.event;
    int MY_RMS = MY_CHANNEL.rms;
    int MY_X = MY_CHANNEL.x;
    int MY_Y = MY_CHANNEL.y;
    if (get_waveform){
      vector<uint32_t> new_waveform;
      vector<uint32_t> waveform = MY_CHANNEL.waveform;
      for (int j=0; j<int(waveform.size()); j++){
        // Exclude values in signal window
        //if ( (j+1>=25 && j+1<=50) || (j+1>70 && j+1<=80) ) continue;
        new_waveform.push_back(waveform[j]);
      }
      MY_RMS = TMath::RMS(new_waveform.size(),&new_waveform[0]);
    }
    
    if (MY_RMS>cutOff_RMS) {
      cout << "Event = " << MY_EVENT << "\t(" << MY_X << "," << MY_Y << ")\t" << MY_RMS << endl;
      //noisy_bases.push_back(MY_CHANNEL);
      map_noisy[std::make_pair(MY_X,MY_Y)]++;
    }
  }

  cout << "Bad Channels = " << map_noisy.size() << endl;
  std::map<std::pair<int,int>,int>::const_iterator iter;
  for (iter=map_noisy.begin(); iter!=map_noisy.end(); ++iter){
    int x = iter->first.first;
    int y = iter->first.second;
    //cout << x << " Number of Times = " << y << endl;
    ADC_Channel my_channel(10,10,10);
    my_channel.x = x;
    my_channel.y = y;
    my_channel.maxADC = iter->second;
    noisy_bases_sorted.push_back(my_channel);
  }
  std::sort(noisy_bases_sorted.begin(),noisy_bases_sorted.end(),sortByMax_DSC());
  for (int i=0; i<int(noisy_bases_sorted.size()); i++){
    int x = noisy_bases_sorted[i].x;
    int y = noisy_bases_sorted[i].y;
    double number = noisy_bases_sorted[i].maxADC;
    cout << x << "," << y << "\t" << number << endl;
  }

/*
  int nBases = noisy_bases.size();
  cout << "There are " << nBases << " noisy bases" << endl;
  // Sort by Largest RMS
  std::sort(noisy_bases.begin(),noisy_bases.end(),sortByRMS_DSC());
  if (nBases > 60) nBases = 60;
  int nCanvas = nBases/12;
  int pad_num = 0;
  int canvas_num = 0;
  if (nBases%12!=0) nCanvas++;
  TCanvas* c_bases[nCanvas];
  for (int j=0; j<nBases; j++){
    pad_num++;
    if (pad_num>12){ pad_num=1; canvas_num++;}
        
    if (pad_num==1){
      char canvas_name[50];
      sprintf(canvas_name,"Noisy Pulse %i",canvas_num+1);
      c_bases[canvas_num] = new TCanvas(canvas_name,canvas_name,1200,1200);
      c_bases[canvas_num]->Divide(3,4);
    }
        
    c_bases[canvas_num]->cd(pad_num);
    int x = noisy_bases[j].x;
    int y = noisy_bases[j].y;
    int event = noisy_bases[j].event;
        
    TGraph* gr = single_channel(fileName, x, y, event);
    gr->Draw("AP");
        
  }
*/
}

void Pulses_ringingHist(TString fileName, bool get_waveform = false){
  DAQUtilities::get_waveform = get_waveform;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  TCanvas* c_ringingHist = new TCanvas("c_ringingHist","c_ringingHist",900,900);
  c_ringingHist->cd();
  TH1F* h_ringing = new TH1F("h_ringing","h_ringing",1000,0,100);
  
  for (int i=0; i<nList; i++){
    
    ADC_Channel MY_CHANNEL = adcList[i];
    int MY_RMS = MY_CHANNEL.rms;
    if (get_waveform){
      vector<uint32_t> new_waveform;
      vector<uint32_t> waveform = MY_CHANNEL.waveform;
      for (int j=0; j<int(waveform.size()); j++){
        // Exclude values in signal window
        //if ( (j+1>=25 && j+1<=50) || (j+1>70 && j+1<=80) ) continue;
        new_waveform.push_back(waveform[j]);
      }
      MY_RMS = TMath::RMS(new_waveform.size(),&new_waveform[0]);
    }
    h_ringing->Fill(MY_RMS);
  }
  h_ringing->SetTitle("RMS of Pulses");
  h_ringing->SetXTitle("RMS");
  h_ringing->Draw();
}

void Pulses_ringingPlot(TString fileName, int crate, int slot, int channel, int cutOff_RMS, bool get_waveform = true){
  DAQUtilities::get_waveform = get_waveform;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  std::vector< ADC_Channel > noisy_bases;
  
  for (int i=0; i<nList; i++){
    
    ADC_Channel MY_CHANNEL = adcList[i];
    int MY_RMS = MY_CHANNEL.rms;
    if ( !(MY_CHANNEL.crate==crate && MY_CHANNEL.slot==slot && MY_CHANNEL.channel==channel) ) continue;
    if (get_waveform){
      vector<uint32_t> new_waveform;
      vector<uint32_t> waveform = MY_CHANNEL.waveform;
      for (int j=0; j<int(waveform.size()); j++){
        // Exclude values in signal window
        //if ( (j+1>=25 && j+1<=50) || (j+1>70 && j+1<=80) ) continue;
        new_waveform.push_back(waveform[j]);
      }
      MY_RMS = TMath::RMS(new_waveform.size(),&new_waveform[0]);
    }
    
    if (MY_RMS>cutOff_RMS) {
      cout << "Event = " << MY_CHANNEL.event << "\t(" << MY_CHANNEL.x << "," << MY_CHANNEL.y << ")\t" << MY_RMS << endl;
      noisy_bases.push_back(MY_CHANNEL);
    }
  }
  
  // Sort
  std::sort(noisy_bases.begin(),noisy_bases.end(),sortByRMS_DSC());
  int nNoisy = noisy_bases.size();
  cout << "Bad Channels = " << nNoisy << endl;
  if (nNoisy>12) nNoisy=12;
  TCanvas* c_rms = new TCanvas("c_rms","c_rms",900,900);
  c_rms->Divide(4,3);
  for (int i=0; i<nNoisy; i++){
    cout << "Event = " << noisy_bases[i].event << "\t(" << noisy_bases[i].x << "," << noisy_bases[i].y << ")\t" << noisy_bases[i].rms << endl;
    c_rms->cd(i+1);
    TGraph* gr = 0x0;
    if (!get_waveform) gr = single_channel(fileName, noisy_bases[i].x, noisy_bases[i].y, noisy_bases[i].event);
    if (get_waveform)  gr = single_channel(noisy_bases[i].waveform, noisy_bases[i].crate, noisy_bases[i].slot, noisy_bases[i].channel, noisy_bases[i].x, noisy_bases[i].y, noisy_bases[i].event);
    gr->Draw("AP");
  }
    
}

void Pulses_ringingPlot(TString fileName, int x, int y, int cutOff_RMS, bool get_waveform = true){
  std::map<std::pair<int,int>, TString > loc_inverse = GeneralUtilities::BaseLocationMapInverse();
  TGraph* gr = 0x0;
  if (loc_inverse.count(std::make_pair(x,y))>0){
    // crate/slot/channel (crate starts at 0-11)
    TString daq_loc = loc_inverse.find(std::make_pair(x,y))->second;
    vector<TString> parse_daq = StringUtilities::parseTString(daq_loc,"/");
    int crate = StringUtilities::TString2int(parse_daq[0])+1;
    int slot = StringUtilities::TString2int(parse_daq[1]);
    int channel = StringUtilities::TString2int(parse_daq[2]);
    Pulses_ringingPlot(fileName, crate, slot, channel, cutOff_RMS, get_waveform);
  } else{
    cout << "Could not find (" << x << "," << y << ")!!" << endl;
  }
}

// Sort by pedestal
void Pulses_pedestalWaveforms(TString fileName, int nPulses=12){
  std::vector< ADC_Channel > adcList = DAQUtilities::manny_waveform_loop(fileName, "nosort");
  // Sort by pedestal
  std::sort(adcList.begin(), adcList.end(), sortByPedestal_DSC());
  int nList = adcList.size();
  
  if (nPulses<0 || nList<=nPulses) nPulses = nList;
  
  int nCanvas = nPulses / 12;
  if ((nPulses % 12) != 0) nCanvas++;
  
  TCanvas* c_ped_waveform[nCanvas];
  
  int pnum = 0;
  int cnum = 0;
  
  for (int i=0; i<nPulses; i++){
    pnum++;
    
    if (pnum==1){
      char name[50];
      sprintf(name,"Pedestal Waveforms %i/%i",cnum+1,nCanvas);
      c_ped_waveform[cnum] =  new TCanvas(name,name,1000,1000);
      c_ped_waveform[cnum]->Divide(4,3);
    }
    c_ped_waveform[cnum]->cd(pnum);
    TGraph* gr = new TGraph();
    char title[50];
    sprintf(title,"Waveform: %i/%i/%i (%i,%i)", adcList[i].crate, adcList[i].slot, adcList[i].channel, adcList[i].x, adcList[i].y);
    gr->SetName(title);
    gr->SetTitle(title);
    vector<uint32_t> waveform = adcList[i].waveform;
    for (int ii=0; ii<int(waveform.size()); ii++){
      gr->SetPoint(ii,ii+1,waveform[ii]);
    }
    gr->Draw("AP");
    if (pnum==12) {cnum++; pnum=0;}
  }
  
}






