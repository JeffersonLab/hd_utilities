#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"

std::map< std::pair<int,int>, double> empty_map(){
  std::map<TString, std::pair<int,int> > loc_map = GeneralUtilities::BaseLocationMap();
  std::map< std::pair<int,int>, double> my_map;
  for (std::map<TString, std::pair<int,int> >::const_iterator iter = loc_map.begin(); iter != loc_map.end(); ++iter){
    my_map[iter->second] = 1;
  }
  return my_map;
}

void track_cosmics(TString fileName, double threshold, TString dir = ""){
  
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "event");
  int nList = adcList.size();
  
  //TH1F* hist = new TH1F("hist","hist",8192,-4096,4096);
  
  map< std::pair<int,int>, double> coord_charge;
  std::vector< ADC_Channel > cosmicList;
  
  std::vector< std::pair<int, std::vector< ADC_Channel > > > cosmicByHits;
  
  for (int i=0; i<nList; i++){
    ADC_Channel mychannel = adcList[i];
    int event = mychannel.event;
        
    int crate = mychannel.crate;
    int slot = mychannel.slot;
    int channel = mychannel.channel;
    
    int x = mychannel.x;
    int y = mychannel.y;
    
    double charge   = mychannel.charge;
    double maxADC   = mychannel.maxADC;
    
    // Compare the current with the next entry in the vector
    bool end_of_file = (i==int(adcList.size())-1) ? true : false;
    bool end_of_event = false;
    if (!end_of_file){
      ADC_Channel next_myevent = adcList[i+1];
      int next_event = next_myevent.event;
      end_of_event = (next_event==event) ? false : true;
    }
    
    if (maxADC>threshold && maxADC < 4096){
      
      ADC_Channel mycosmics(crate,slot,channel,event);
      mycosmics.x = x;
      mycosmics.y = y;
      mycosmics.charge = charge;
      mycosmics.maxADC = maxADC;
      mycosmics.event = event;
      cerr << mycosmics.crate << "/" << mycosmics.slot << "/" << mycosmics.channel << "\t (" << mycosmics.x << "," << mycosmics.y << ")\t Event: " << mycosmics.event << "\t" << mycosmics.maxADC << endl;
      cosmicList.push_back(mycosmics);
    }
    
    if (end_of_file || end_of_event){
      std::pair<int, std::vector< ADC_Channel > > cosmic_pair;
      cosmic_pair = std::make_pair(cosmicList.size(),cosmicList);
      cosmicByHits.push_back(cosmic_pair);
      
      cosmicList.clear();
    }
  }
  
  std::sort(cosmicByHits.begin(),cosmicByHits.end(),sortByCosmicHits_DSC());
  
  int nHits = cosmicByHits.size();
  cout << "Total: " << nHits << endl;
  
  // Get Top 10 Cosmic Events
  int nC = 10;
  TCanvas* nCanvas[nC];
  std::map< std::pair<int,int>, double> start_empty_map = empty_map();
  for (int i=0; i<nC; i++){
    std::pair<int, std::vector< ADC_Channel > > mycosmics = cosmicByHits[i];
    nHits = mycosmics.first;
    std::vector< ADC_Channel > cosmicList2 = mycosmics.second;
    
    cerr << "Number of cosmic hits: " << nHits << endl;
    if (nHits==0) continue;
    
    TString title = "# Cosmic Tracks = " + StringUtilities::int2TString(nHits)+", Event # " + StringUtilities::int2TString(cosmicList2[0].event);
    std::map< std::pair<int,int>, double> detector_map = start_empty_map;
    for (int ii=0; ii<int(cosmicList2.size()); ii++){
      double val = cosmicList2[ii].maxADC;
      int x = cosmicList2[ii].x;
      int y = cosmicList2[ii].y;
      detector_map[std::make_pair(x,y)] = val;
      //cerr << cosmicList2[ii].crate << "/" << cosmicList2[ii].slot << "/" << cosmicList2[ii].channel << "\t(" << x << "," << y << "):\t" << charge << "\tevent:\t" << cosmicList2[ii].event <<  endl;
    }
    
    nCanvas[i] = new TCanvas("nCanvas_"+StringUtilities::int2TString(i),"nCanvas_"+StringUtilities::int2TString(i),1000,1000);
    nCanvas[i]->cd();
    TH2F* hist2D = PlotUtilities::Plot2D(detector_map, -30, 30, -30, 30, 0, -1);
    hist2D->SetTitle(title);
    hist2D->Draw("colz");
    if (dir !=""){
      if (!dir.EndsWith("/")) dir +="/";
      nCanvas[i]->SaveAs(dir+"Cosmics_"+StringUtilities::int2TString(i)+".pdf");
      nCanvas[i]->SaveAs(dir+"Cosmics_"+StringUtilities::int2TString(i)+".png");
    }
  }
}
