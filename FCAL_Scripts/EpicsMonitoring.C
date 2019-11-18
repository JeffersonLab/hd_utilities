
#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
/*
#include "FCALUtilities/StringUtilities.h"
#include "FCALUtilities/GeneralUtilities.h"
#include "FCALUtilities/PlotUtilities.h"
#include "FCALUtilities/DAQUtilities.h"
#include "FCALUtilities/MonitoringUtilities.h"
*/
// pv = FCAL:X:Y:readback
string pvName(string pv){
  vector<TString> parseTString = StringUtilities::parseTString(StringUtilities::string2TString(pv),":");
  return StringUtilities::TString2string(parseTString[0]+":"+parseTString[1]+":"+parseTString[2]+":"+parseTString[3]);
}

string expertName(string pv, map<std::pair<int,int>, TString > expert_map){
  vector<TString> parseTString = StringUtilities::parseTString(StringUtilities::string2TString(pv),":");
  int Col = StringUtilities::TString2int(parseTString[2]);
  int Row = StringUtilities::TString2int(parseTString[3]);
  TString name = expert_map.find(std::make_pair(Col,Row))->second;
  return StringUtilities::TString2string(name);
}

string nonexpertName(string pv, map<std::pair<int,int>, TString > nonexpert_map){
  vector<TString> parseTString = StringUtilities::parseTString(StringUtilities::string2TString(pv),":");
  int Col = StringUtilities::TString2int(parseTString[2]);
  int Row = StringUtilities::TString2int(parseTString[3]);
  TString name = nonexpert_map.find(std::make_pair(Col,Row))->second;
  return StringUtilities::TString2string(name);
}

void daqToCoord(int crate, int slot, int channel){
  std::map<TString, std::pair<int,int> > baseloc = GeneralUtilities::BaseLocationMap();
  TString daqName = StringUtilities::int2TString(crate-1)+"/"+StringUtilities::int2TString(slot)+"/"+StringUtilities::int2TString(channel);
  if (baseloc.count(daqName)>0){
    pair<int,int> coord = baseloc.find(daqName)->second;
    cout << "(" << coord.first << "," << coord.second << ")" << endl;
    } else{
      cout << "Not a valid DAQ chanel" << endl;
    }
}

TString coordToDAQ(int x, int y){
  std::map<std::pair<int,int>, TString > baseloc = GeneralUtilities::BaseLocationMapInverse();
  std::pair<int,int> coord = std::make_pair(x,y);
  if (baseloc.count(coord)>0){
    TString daq = baseloc.find(coord)->second;
    vector<TString> parse_daq = StringUtilities::parseTString(daq,"/");
    return StringUtilities::int2TString(StringUtilities::TString2int(parse_daq[0])+1)+"/"+parse_daq[1]+"/"+parse_daq[2];
    } else{
      cout << "Not a valid Coordinate" << endl;
      return "";
    }
}

int coordToCAN(int x, int y){
  std::map<std::pair<int,int>, int > canID_map = GeneralUtilities::coordToCANMap(0);
  std::pair<int,int> coord = std::make_pair(x,y);
  if (canID_map.count(coord)>0){
    int canID = canID_map.find(coord)->second;
    return canID;
    } else{
      cout << "Not a valid Coordinate" << endl;
      return 0;
    }
}

int coordToBaseID(int x, int y){
  std::map<std::pair<int,int>, int > baseID_map = GeneralUtilities::coordToBaseIDMap(0);
  std::pair<int,int> coord = std::make_pair(x,y);
  if (baseID_map.count(coord)>0){
    int canID = baseID_map.find(coord)->second;
    return canID;
    } else{
      cout << "Not a valid Coordinate" << endl;
      return 0;
    }
}

std::pair<int,int> pvCoord(string pv){
  vector<TString> parseTString = StringUtilities::parseTString(StringUtilities::string2TString(pv),":");
  int x = StringUtilities::TString2int(parseTString[2]);
  int y = StringUtilities::TString2int(parseTString[3]);
  return std::make_pair(x,y);
}

std::map<string, double> map_values(string monitoringFile, TString inequality = ">/</=", double Value = 0){
  std::map<string, double> mapValues;
  
  string pvName;
  string s_val;
  
  string line;
  std::istringstream lin;
  ifstream inFile(monitoringFile.c_str());
  if(inFile.is_open())
  {
    while( getline (inFile,line) )
    {
      
      lin.clear();
      lin.str(line);
      std::istringstream iss(line);
      
      
      if (iss >> pvName >> s_val){
        
        double val = 0;
        if (s_val=="OFF") val = 0;
        else if (s_val=="ON")  val = 1;
        else val = StringUtilities::TString2double(StringUtilities::string2TString(s_val));
        //cout << pvName << "\t" << val << endl;
        if (inequality==">" && val>Value) mapValues[pvName] = val;
        if (inequality=="<" && val<Value) mapValues[pvName] = val;
        if (inequality=="=" && int(val)==int(Value)) mapValues[pvName] = val;
      }
    }
  }
  
  return mapValues;
}

int scan_monitoring(TString readback = ":status/:vmon", TString inequality = ">/</=", double Value = 0, bool compareSetpoint = false, bool expertLocation = true, bool nonexpertLocation = true){
  double temp_Value = Value;
  cout << "This may take a few minutes..." << endl;
  string monitoringFile = "";
  string monitoringFile2 = "";
  if (!compareSetpoint && readback==":status"){
    monitoringFile = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/StatusValues.txt";
    system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetStatus > /gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/StatusValues.txt");
  }
  
  if (!compareSetpoint && readback==":vmon"){
    monitoringFile = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVValues.txt";
    system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetHVStatus > /gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVValues.txt");
  }
  
  if (compareSetpoint){
    temp_Value = -1;
    inequality = ">";
    monitoringFile = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVValues.txt";
    system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetHVStatus > /gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVValues.txt");
    
    monitoringFile2 = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVSet.txt";
    system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetHVSet > /gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVSet.txt");
  }
  
  map<std::pair<int,int>, TString > expert_map = GeneralUtilities::coordToExpertMap();
  map<std::pair<int,int>, TString > nonexpert_map = GeneralUtilities::coordToNonExpertMap();
  
  std::map<string, double>::const_iterator iter;
  std::map<string, double>::const_iterator iter2;
  
  std::map<string, double> mapValues = map_values(monitoringFile, inequality, temp_Value);
  
  if (compareSetpoint)  {
    std::map<string, double> mapValues3;
    std::map<string, double> mapValues2 = map_values(monitoringFile2, ">", temp_Value);
    iter = mapValues.begin();
    iter2 = mapValues2.begin();
    while(iter!=mapValues.end()){
      string pv1 = pvName(iter->first);
      string pv2 = pvName(iter2->first);
      if (pv1!=pv2) {cout << "There is a problem!!" << endl; return 0;}
      double diff = iter->second - iter2->second;
      string name = "";
      if (expertLocation) name += "Expert: " + expertName(iter->first, expert_map)+"\t";
      if (nonexpertLocation) name += "NonExpert: " + nonexpertName(iter->first, nonexpert_map)+"\t";
      name += iter->first;
      if ( fabs(diff)>Value ) mapValues3[name] = diff;
      ++iter;
      ++iter2;
    }
    
    cout << "There are " << mapValues.size() << " Channels Where |vmon-v0set| > 300 Volts" << endl;
    cout << "Below is :vmon - v0set" << endl;
    for (iter = mapValues3.begin(); iter != mapValues3.end(); ++iter){
      cout << iter->first << "\t" << iter->second << endl;
    }
    return 1;
  }
  
  // List bad channels
  cout << "There are " << mapValues.size() << " Bad Channels" << endl;
  for (iter = mapValues.begin(); iter != mapValues.end(); ++iter){
    string name = "";
    if (expertLocation) name += "Expert: " + expertName(iter->first, expert_map)+"\t";
    if (nonexpertLocation) name += "NonExpert: " + nonexpertName(iter->first, nonexpert_map)+"\t";
    name += iter->first;
    cout << name << "\t" << iter->second << endl;
  }
  
  return 1;
}

std::map<std::pair<int,int>, double> map_values_2d(string monitoringFile){
  std::map<std::pair<int,int>, double> mapValues;
  
  
  string pvName;
  string s_val;
  string line;
  std::istringstream lin;
  ifstream inFile(monitoringFile.c_str());
  if(inFile.is_open())
  {
    while( getline (inFile,line) )
    {
      
      lin.clear();
      lin.str(line);
      std::istringstream iss(line);
      
      
      if (iss >> pvName >> s_val){
        
        std::pair<int,int> coord = pvCoord(pvName);
        double val = StringUtilities::TString2double(s_val);
        mapValues[coord] = val;
        //if (val<10) cout << pvName << endl;
      }
    }
  }
  
  return mapValues;
}

void plot_voltages_2d(bool new_data = false){
  string monitoringFile = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVValues.txt";
  if (new_data) {
    cout << "This may take a few minutes..." << endl;
    system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetHVStatus > /gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/HVValues.txt");
  }
  std::map<std::pair<int,int>, double> mapValues = map_values_2d(monitoringFile);
  TH2F* hist2d = PlotUtilities::Plot2D(mapValues, -30, 30, -30, 30, 0, -1);
  TCanvas* c1 = new TCanvas("c1","c1",600,600);
  c1->cd();
  hist2d->Draw("colz");
}

void get_daq(double r_comp){

  std::map<std::pair<int,int>, TString > base_map = GeneralUtilities::BaseLocationMapInverse();
  for (std::map<std::pair<int,int>, TString >::const_iterator iter = base_map.begin(); iter != base_map.end(); ++iter){
    std::pair<int,int> coord = iter->first;
    int x = coord.first;
    int y = coord.second;
    //double r = TMath::Sqrt(TMath::Power(double(x),2)+TMath::Power(double(y),2));
    if (fabs(x)<=r_comp && fabs(y)<=r_comp) cout << "X = " << x << "\tY = " << y << "\t" << coordToDAQ(x,y) << endl;
  }
}

void more_channelsMask(){
  int x[]={-23,-21,-20,-18,-17,-17,-16,-16,-15,-15,-14,-13,-13,-11,-10,-10,-10,-10,-9,-9,-9,-6,-6,-6,-4,-3,-2,-2,-1,-1,0,2,3,3,4,4,5,5,5,5,6,7,7,7,8,10,12,12,13,13,16,16,19,19,21,23,25};
  int y[]={-15,10,15,7,0,10,-23,6,-18,2,-9,3,15,25,-20,-9,0,7,2,27,28,-4,5,7,3,-11,-6,12,10,14,26,29,-15,9,-1,18,-13,-4,6,28,-1,5,10,13,-8,-10,-4,0,14,25,-9,18,-13,7,-1,13,-16};
  int n = sizeof(x)/sizeof(*x);
  cout << n << endl;
  for (int i=0; i<n; i++){
    cout << coordToDAQ(x[i],y[i]) << endl;
  }
}

void basesToReplace(){

  vector< std::pair<int,int> > vec_basesToReplace;

  vec_basesToReplace.push_back( std::make_pair(-25,12)  );
  vec_basesToReplace.push_back( std::make_pair(1,-26)   );
  vec_basesToReplace.push_back( std::make_pair(1,-16)   );
  vec_basesToReplace.push_back( std::make_pair(22,-2)   );
  vec_basesToReplace.push_back( std::make_pair(21,-21)  );
  vec_basesToReplace.push_back( std::make_pair(10,15)   );
  vec_basesToReplace.push_back( std::make_pair(-16,-22) );
  vec_basesToReplace.push_back( std::make_pair(7,14)    );
  vec_basesToReplace.push_back( std::make_pair(19,-14)  );
  vec_basesToReplace.push_back( std::make_pair(8,21)    );
  vec_basesToReplace.push_back( std::make_pair(2,-11)   );
  vec_basesToReplace.push_back( std::make_pair(-17,-24) );
  vec_basesToReplace.push_back( std::make_pair(-20,-9)  );
  vec_basesToReplace.push_back( std::make_pair(-2,28)   );
  vec_basesToReplace.push_back( std::make_pair(-20,-10) );
  vec_basesToReplace.push_back( std::make_pair(-17,-9)  );
  vec_basesToReplace.push_back( std::make_pair(1,-11)   );
  vec_basesToReplace.push_back( std::make_pair(-17,22)  );
  vec_basesToReplace.push_back( std::make_pair(-5,22)   );
  vec_basesToReplace.push_back( std::make_pair(2,15)    );
  vec_basesToReplace.push_back( std::make_pair(-8,-9)   );

  for (int i=0; i<int(vec_basesToReplace.size()); i++){
    int x = vec_basesToReplace[i].first;
    int y = vec_basesToReplace[i].second;
    cout << "(x,y) Location: (" << x << "," << y << ")\t CAN ID: " << coordToCAN(x, y) << "\tBase ID: " << coordToBaseID(x,y) << endl;
  }
}

void mask_slot(int slotnum, vector<int> slot){
  int counter = 0;
  TString output = "#################\nFADC250_SLOTS   "+StringUtilities::int2TString(slotnum)+"\n#################\n";
  output += "\n#       channel:   0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15\nFADC250_TRG_MASK   ";
  for (int i=0; i<16; i++){
    bool found = false;
    for (int j=0; j<int(slot.size()); j++){
      int channel = slot[j];
      if (channel==i) {found=true; break;}
    }
    if (found) output += "1";
    else output += "0";
    
    if (i<10)  output += "   ";
    if (i>=10) output += "    ";
    
  }
  cout << output << endl;
}

void mask_inner(double r_comp){
  vector<ADC_Channel> adc_list;
  std::map<std::pair<int,int>, TString > base_map = GeneralUtilities::BaseLocationMapInverse();
  for (std::map<std::pair<int,int>, TString >::const_iterator iter = base_map.begin(); iter != base_map.end(); ++iter){
    std::pair<int,int> coord = iter->first;
    int x = coord.first;
    int y = coord.second;
    double r = TMath::Sqrt(TMath::Power(double(x),2)+TMath::Power(double(y),2));
    //if (fabs(x)<=r_comp && fabs(y)<=r_comp) {
    if (r<=r_comp) {
      ADC_Channel MY_CHANNEL;
      MY_CHANNEL.x = x;
      MY_CHANNEL.y = y;
      MY_CHANNEL.absNum = iter->second;
      adc_list.push_back(MY_CHANNEL);
      cout << "X = " << x << "\tY = " << y << "\t" << coordToDAQ(x,y) << endl;
    }
  }
  std::sort(adc_list.begin(),adc_list.end(),sort_ASC());
  int nList = adc_list.size();
  vector<int> slot_vec;
  for (int i=0; i<nList; i++){
    vector<TString> parseTString = StringUtilities::parseTString(adc_list[i].absNum,"/");
    int crate = parseTString[0].Atoi();
    int slot = parseTString[1].Atoi();
    int channel = parseTString[2].Atoi();
    
    
    if (i+1!=nList){
      
      vector<TString> parseTString2 = StringUtilities::parseTString(adc_list[i+1].absNum,"/");
      int next_crate = parseTString2[0].Atoi();
      int next_slot = parseTString2[1].Atoi();
      int next_channel = parseTString2[2].Atoi();
      
      if (next_crate==crate && next_slot==slot){
        slot_vec.push_back(channel);
      }
      
      if (next_crate!=crate || next_slot!=slot){
        cout << "Crate " << crate << " Slot " << slot << endl;
        mask_slot(slot,slot_vec);
        slot_vec.clear();
      }
      
    }
    
    if (i+1==nList) {
      cout << "Crate " << crate << " Slot " << slot << endl;
      slot_vec.push_back(channel);
      mask_slot(slot,slot_vec);
      slot_vec.clear();
    }
  }
}


