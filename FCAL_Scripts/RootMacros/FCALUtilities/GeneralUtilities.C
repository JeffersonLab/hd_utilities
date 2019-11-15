#include "GeneralUtilities.h"


// ***************
// Global Variables
// ***************
bool GeneralUtilities::CalibrateDAC = false;
bool GeneralUtilities::PrintOverflows = false;
bool GeneralUtilities::RemoveOverflows = true;
// Typs of sorts
bool GeneralUtilities::sort_by_channel_event = true;
bool GeneralUtilities::sort_by_channel = false;
bool GeneralUtilities::sort_by_event = false;
bool GeneralUtilities::sort_by_slot  = false;

bool GeneralUtilities::KeepFCALOnly = true;

// ***************
// Based on a crate slot channel and side it can determine whether a slot exists
// Where the 1st crate on the north side can be 0 or 11
// ***************
bool GeneralUtilities::SlotExists_OLD(int crate, int slot, bool north){
  
  if (crate<10){
    if (north){
      if (crate==3 && slot == 19) return false;
      if (crate==5 && slot == 19) return false;
    }
    bool south = !north;
    if (south){
      if (crate==0 && slot == 19) return false;
      if (crate==2 && slot == 19) return false;
    }
  } else{ // Add 11 to north crates and 17 to south
    if (crate==14 && slot == 19) return false;
    if (crate==16 && slot == 19) return false;
    if (crate==17 && slot == 19) return false;
    if (crate==19 && slot == 19) return false;
  }
  if (slot==0 || slot==1 || slot==2 || slot==11 || slot==12 || slot==20){
    return false;
  }
  return true;
}

bool 
GeneralUtilities::SlotExists(int crate, int slot){
  
  if (crate==14 && slot==19)  return false;
  if (crate==16 && slot==19)  return false;
  if (crate==17 && slot==19)  return false;
  if (crate==19 && slot==19)  return false;
  
  if (slot==0 || slot==1 || slot==2 || slot==11 || slot==12 || slot==20){
    return false;
  }
  return true;
}


// ***************
// Returns a map of the bases location
// key = crate/slot/channel (where the 1st north crate starts at 0 and the 1st sout crate starts at 6)
// value = pair(x,y)
// ***************
std::map<TString, std::pair<int,int> > GeneralUtilities::BaseLocationMap(){
  
  std::map<TString, std::pair<int,int> > base_map;
  
  // crate/slot/channel
  string daq_loc;
  int abs_num;
  // x/y
  string det_loc;
  
  string line;
  std::istringstream lin;
  ifstream inFile("RootMacros/FCALUtilities/DAQCoordinate_Ordered.txt");
  if(inFile.is_open())
  {
    while( getline (inFile,line) )
    {
      lin.clear();
      lin.str(line);
      std::istringstream iss(line);
      
      if (iss >> daq_loc >> abs_num >> det_loc){
        //cout << daq_loc << "\t" << abs_num << "\t" << det_loc << endl;
        vector<TString> parseTString = StringUtilities::parseTString(det_loc,"/");
        base_map[TString(daq_loc)] = std::make_pair(StringUtilities::TString2int(parseTString[0]),StringUtilities::TString2int(parseTString[1]));
      }
    }
  }
  return base_map;
}

// ***************
// Returns a map of the bases location
// key = pair(x,y)
// value = crate/slot/channel (where the 1st north crate starts at 0 and the 1st sout crate starts at 6)
// ***************
std::map<std::pair<int,int>, TString > GeneralUtilities::BaseLocationMapInverse(){
  
  std::map<std::pair<int,int>, TString > base_map;
  
  // crate/slot/channel
  string daq_loc;
  int abs_num;
  // x/y
  string det_loc;
  
  string line;
  std::istringstream lin;
  ifstream inFile("RootMacros/FCALUtilities/DAQCoordinate_Ordered.txt");
  if(inFile.is_open())
  {
    while( getline (inFile,line) )
    {
      
      lin.clear();
      lin.str(line);
      std::istringstream iss(line);
      
      
      if (iss >> daq_loc >> abs_num >> det_loc){
        //cout << daq_loc << "\t" << abs_num << "\t" << det_loc << endl;
        vector<TString> parseTString = StringUtilities::parseTString(det_loc,"/");
        base_map[std::make_pair(StringUtilities::TString2int(parseTString[0]),StringUtilities::TString2int(parseTString[1]))] = TString(daq_loc);
      }
    }
  }
  return base_map;
}

// ***************
// Returns a map of the bases location
// key = crate/slot/channel (where the 1st north crate starts at 0 and the 1st sout crate starts at 6)
// value = absolute position of the base (arbitrary unique number for each base)
// ***************
std::map<TString, int > GeneralUtilities::BaseAbsMap(){
  
  std::map<TString, int > abs_map;
  
  // crate/slot/channel
  string daq_loc;
  int abs_num;
  // x/y
  string det_loc;
  
  string line;
  ifstream inFile("RootMacros/FCALUtilities/DAQCoordinate_Ordered.txt");
  if(inFile.is_open())
  {
    while( getline (inFile,line) )
    {
      inFile >> daq_loc >> abs_num >> det_loc;
      //cout << daq_loc << "\t" << abs_num << "\t" << det_loc << endl;
      vector<TString> parseTString = StringUtilities::parseTString(det_loc,"/");
      abs_map[TString(daq_loc)] = abs_num;
    }
  }
  return abs_map;
}

// ***************
// Returns a map matching bases to location
// key = canid
// value = pair(x,y)
// ***************
map<int,std::pair<int,int> > GeneralUtilities::baseToCoordMap(int i){
  string temp_line;
  ifstream temp_myfile;
  if (i==0) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB.tab");
  if (i==1) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_07-29-14.tab");
  if (i==2) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_08-09-14.tab");
  if (i==3) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_08-26-14.tab");
  
  map<int,std::pair<int,int> > canToBaseMap2;
  
  if (temp_myfile.is_open())
  {
    while( getline (temp_myfile,temp_line) )
    {
      vector<TString> v_line = StringUtilities::parseString(temp_line," ");
      TString baseID = v_line[3];
      TString x = v_line[1];
      TString y = v_line[2];
      std::pair<int,int> coord;
      coord = std::make_pair(StringUtilities::TString2int(x),StringUtilities::TString2int(y) );
      canToBaseMap2[StringUtilities::TString2int(baseID)] = coord;
      //cout << baseID << "\t" << "(" << x << "," << y << ")" << endl;
    }
  }
  //cout << "Map Size = " << canToBaseMap2.size() << endl;
  return canToBaseMap2;
}
// ***************
// Returns a map matching bases to location
// key = pair(x,y)
// value = canid
// ***************
map<std::pair<int,int>, int > GeneralUtilities::coordToCANMap(int i){
  string temp_line;
  ifstream temp_myfile;
  if (i==0) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB.tab");
  if (i==1) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_07-29-14.tab");
  if (i==2) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_08-09-14.tab");
  if (i==3) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_08-26-14.tab");
  
  map<std::pair<int,int>, int > canToBaseMap2;
  
  if (temp_myfile.is_open())
  {
    while( getline (temp_myfile,temp_line) )
    {
      vector<TString> v_line = StringUtilities::parseString(temp_line," ");
      TString canID = v_line[3];
      TString x = v_line[1];
      TString y = v_line[2];
      std::pair<int,int> coord;
      coord = std::make_pair(StringUtilities::TString2int(x),StringUtilities::TString2int(y) );
      //canToBaseMap2[StringUtilities::TString2int(baseID)] = coord;
      canToBaseMap2[coord] = StringUtilities::TString2int(canID);
      //cout << baseID << "\t" << "(" << x << "," << y << ")" << endl;
    }
  }
  //cout << "Map Size = " << canToBaseMap2.size() << endl;
  return canToBaseMap2;
}

// ***************
// Returns a map matching bases to location
// key = pair(x,y)
// value = canid
// ***************
map<std::pair<int,int>, int > GeneralUtilities::coordToBaseIDMap(int i){
  string temp_line;
  ifstream temp_myfile;
  if (i==0) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB.tab");
  if (i==1) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_07-29-14.tab");
  if (i==2) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_08-09-14.tab");
  if (i==3) temp_myfile.open("RootMacros/ExportedDB/Exported_FCAL_DB_Before_08-26-14.tab");
  
  map<std::pair<int,int>, int > canToBaseMap2;
  
  if (temp_myfile.is_open())
  {
    while( getline (temp_myfile,temp_line) )
    {
      vector<TString> v_line = StringUtilities::parseString(temp_line," ");
      TString baseID = v_line[0];
      TString x = v_line[1];
      TString y = v_line[2];
      std::pair<int,int> coord;
      coord = std::make_pair(StringUtilities::TString2int(x),StringUtilities::TString2int(y) );
      //canToBaseMap2[StringUtilities::TString2int(baseID)] = coord;
      canToBaseMap2[coord] = StringUtilities::TString2int(baseID);
      //cout << baseID << "\t" << "(" << x << "," << y << ")" << endl;
    }
  }
  //cout << "Map Size = " << canToBaseMap2.size() << endl;
  return canToBaseMap2;
}

// file in the following order:
// strand Row Col Position canid A B
map<std::pair<int,int>, TString > GeneralUtilities::coordToExpertMap(){
  string temp_line;
  ifstream temp_myfile;
  temp_myfile.open("RootMacros/ExportedDB/FullFCALInfo.txt");
  map<std::pair<int,int>, TString > mymap;
  
  if (temp_myfile.is_open())
  {
    while( getline (temp_myfile,temp_line) )
    {
      vector<TString> v_line = StringUtilities::parseString(temp_line," ");
      TString baseID = v_line[4];
      TString x = v_line[2];
      TString y = v_line[1];
      TString strand  = v_line[0];
      TString pos  = v_line[3];
      // expert notation = Quattro:Port:Pos
      int quattro = StringUtilities::TString2int(strand)/4;
      if (StringUtilities::TString2int(strand)%4>0) quattro += 1;
      TString port = "A";
      if ((StringUtilities::TString2int(strand)-1)%4==1) port = "B";
      if ((StringUtilities::TString2int(strand)-1)%4==2) port = "C";
      if ((StringUtilities::TString2int(strand)-1)%4==3) port = "D";
      
      TString expert = StringUtilities::int2TString(quattro)+":"+port+":"+pos;
      std::pair<int,int> coord;
      coord = std::make_pair(StringUtilities::TString2int(x),StringUtilities::TString2int(y) );
      mymap[coord] = expert;
      //cout << baseID << "\t" << "(" << x << "," << y << ")" << endl;
    }
  }
  //cout << "Map Size = " << canToBaseMap2.size() << endl;
  return mymap;
}

map<std::pair<int,int>, TString > GeneralUtilities::coordToNonExpertMap(){
  string temp_line;
  ifstream temp_myfile;
  temp_myfile.open("RootMacros/ExportedDB/FullFCALInfo.txt");
  map<std::pair<int,int>, TString > mymap;
  
  if (temp_myfile.is_open())
  {
    while( getline (temp_myfile,temp_line) )
    {
      vector<TString> v_line = StringUtilities::parseString(temp_line," ");
      TString baseID = v_line[4];
      TString x = v_line[2];
      TString y = v_line[1];
      TString strand  = v_line[0];
      TString pos  = v_line[3];
      // nonexpert notation = Quadrant:Strand:Pos
      int _strand = StringUtilities::TString2int(strand);
      int quadrant = 0;
      if (_strand>=1 && _strand<=7)   quadrant = 2;
      if (_strand>=8 && _strand<=14)  quadrant = 3;
      if (_strand>=15 && _strand<=21) quadrant = 1;
      if (_strand>=22 && _strand<=28) quadrant = 4;
      
      TString nonexpert = StringUtilities::int2TString(quadrant)+":"+strand+":"+pos;
      std::pair<int,int> coord;
      coord = std::make_pair(StringUtilities::TString2int(x),StringUtilities::TString2int(y) );
      mymap[coord] = nonexpert;
      //cout << baseID << "\t" << "(" << x << "," << y << ")" << endl;
    }
  }
  //cout << "Map Size = " << canToBaseMap2.size() << endl;
  return mymap;
}

// ***************
// Based on the crate slot channel it checks whether that channel is connected to base
// This is based on a new numbering scheme for the crates where the 1st north crate is 11
// ***************
bool GeneralUtilities::RemoveADC(int crate, int slot, int channel){
  if (crate==12 && slot==3  && channel==0)  return true;
  if (crate==12 && slot==3  && channel==1)  return true;
  if (crate==14 && slot==3  && channel==15) return true;
  if (crate==14 && slot==18 && channel==15) return true;
  if (crate==15 && slot==3  && channel==8)  return true;
  if (crate==15 && slot==3  && channel==12) return true;
  if (crate==16 && slot==3  && channel==15) return true;
  if (crate==16 && slot==18 && channel==15) return true;
  if (crate==17 && slot==3  && channel==15) return true;
  if (crate==17 && slot==18 && channel==15) return true;
  if (crate==18 && slot==3  && channel==14) return true;
  if (crate==18 && slot==3  && channel==15) return true;
  if (crate==19 && slot==3  && channel==15) return true;
  if (crate==19 && slot==18 && channel==15) return true;
  if (crate==21 && slot==3  && channel==3)  return true;
  if (crate==21 && slot==3  && channel==7)  return true;
  return false;
}

// ******************
// Allows One to specify an adc to keep
// Numbers start at 1-12
// ******************
int GeneralUtilities::KeepSpecificCrate = -1;
int GeneralUtilities::KeepSpecificSlot = -1;
int GeneralUtilities::KeepSpecificChannel = -1;
bool GeneralUtilities::KeepADC(int crate,int slot,int channel){
  if (GeneralUtilities::KeepSpecificCrate==-1){
    return true;
  }
  else if (GeneralUtilities::KeepSpecificCrate==crate && GeneralUtilities::KeepSpecificSlot==slot && GeneralUtilities::KeepSpecificChannel==channel){
    return true;
  }
  return false;
}


// ***************
// Standardize Names
// ***************
TString GeneralUtilities::formatName(TString filename){
  vector<TString> parseFile = StringUtilities::parseTString(filename,"/");
  TString fname = parseFile[int(parseFile.size())-1];
  if (filename.Contains(".txt")){
    fname = fname.Remove(fname.Length()-4,fname.Length());
  }
  else{
    fname = fname.Remove(fname.Length()-5,fname.Length());
    if (fname.Contains(".0")) {fname = fname.Remove(fname.Length()-2,fname.Length());}
  }
  return fname;
}

// ***************
// Standardize Titles
// ***************
TString GeneralUtilities::formatTitle(int crate, int slot, int channel){
  char name[50];
  sprintf(name,"Crate %i Slot %02i Channel %02i",crate,slot,channel);
  TString _name = name;
  return _name;
}

TString GeneralUtilities::formatTitle2(int crate, int slot, int channel){
  char name[50];
  sprintf(name,"%i/%02i/%02i",crate,slot,channel);
  TString _name = name;
  return _name;
}



