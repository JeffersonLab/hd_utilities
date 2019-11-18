
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
#include "TStopwatch.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TFile.h"
#include "TTree.h"
#include <ctime>

// Make a tm structure representing this date
std::tm make_tm(int year, int month, int day)
{
    std::tm tm = {0};
    tm.tm_year = year - 1900; // years count from 1900
    tm.tm_mon = month - 1;    // months count from January=0
    tm.tm_mday = day;         // days count from 1
    return tm;
}

struct sort_By_Name_Date_ASC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        if (left.type != right.type) return left.type < right.type;
        if (left.date != right.date) return left.date < right.date;
    }
};


int MonitoringToRoot_getTime(TString startDate, TString endDate){
  // Get endDate - startDate in minutes
  vector<TString> parse_startDate = StringUtilities::parseTString(startDate," ");
  vector<TString> parse_endDate   = StringUtilities::parseTString(endDate," ");
  // Start Date Parsed
  TString _startDate = parse_startDate[0];
  vector<TString> _parse_startDate = StringUtilities::parseTString(_startDate,"-");
  TString startYear  = _parse_startDate[0];
  TString startMonth = _parse_startDate[1];
  TString startDay   = _parse_startDate[2];
  TString startTime  = parse_startDate[1];
  vector<TString> parse_startTime = StringUtilities::parseTString(startTime,":");
  TString startHour   = parse_startTime[0];
  TString startMinute = parse_startTime[1];
  TString startSecond = parse_startTime[2];
  // End Date Parsed
  TString _endDate = parse_endDate[0];
  vector<TString> _parse_endDate = StringUtilities::parseTString(_endDate,"-");
  TString endYear  = _parse_endDate[0];
  TString endMonth = _parse_endDate[1];
  TString endDay   = _parse_endDate[2];
  TString endTime  = parse_endDate[1];
  vector<TString> parse_endTime = StringUtilities::parseTString(endTime,":");
  TString endHour   = parse_endTime[0];
  TString endMinute = parse_endTime[1];
  TString endSecond = parse_endTime[2];
  // approximate 30 days in a month
  int days_in_month = 30;
  
  int Year   = StringUtilities::TString2int(endYear) - StringUtilities::TString2int(startYear);
  int Month  = StringUtilities::TString2int(endMonth) - StringUtilities::TString2int(startMonth);
  int Day    = StringUtilities::TString2int(endDay) - StringUtilities::TString2int(startDay);
  int Hour   = StringUtilities::TString2int(endHour) - StringUtilities::TString2int(startHour);
  int Minute = StringUtilities::TString2int(endMinute) - StringUtilities::TString2int(startMinute);
  int Second = StringUtilities::TString2int(endSecond) - StringUtilities::TString2int(startSecond);
  // Get divisions
  int n = Year*12*30*24*60 + Month*30*24*60 + Day*24*60 + Hour*60 + Minute;
  return n;
}

void MonitoringToRoot_getData(TString startDate = "2014-11-14 01:00:00", TString endDate = "2014-12-21 13:00:00"){
  
  int n = MonitoringToRoot_getTime(startDate, endDate);
  // Nominal time slice: 10 minute chunks
  n = n/10;
  // 1 hrour chunks
 // n = n/60;
// 5 min chunks
//n = n/5;
//n = n;
  int counter = 0;
  
  // Get all the variables to pull from the archiver
  //TString suffix[] = {":vmon", ":v0set", ":imon", ":dyn", ":mvb", ":mvt", ":dac"};
  //TString suffix[] = {":vmon", ":v0set", ":imon", ":dac"};
  TString suffix[] = {":vmon", ":dyn", ":v0set", ":imon", ":dac", ":status"};
  int n_suffix = sizeof(suffix)/sizeof(*suffix);
  TString Tcommand = "#!/bin/csh \n\n mySampler -b \""+startDate+"\" -s 1h -n "+StringUtilities::int2TString(n);
  std::map<std::pair<int,int>, TString > loc_map = GeneralUtilities::BaseLocationMapInverse();
  for (map<std::pair<int,int>, TString >::const_iterator iter=loc_map.begin(); iter!=loc_map.end(); ++iter){
    //if (counter!=0) continue;    
    int y = (iter->first).first;
    int x = (iter->first).second;
    TString prefix = " FCAL:hv:"+StringUtilities::int2TString(x)+":"+StringUtilities::int2TString(y);
    for (int j=0; j<n_suffix; j++){
      Tcommand += prefix+suffix[j];
    }
  counter++;
  }
  
  string get_values = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringData/getBaseValues1.sh";
  fstream* outFile = new fstream();
  outFile->open(get_values.c_str(), std::fstream::out);
  *outFile << Tcommand << endl;
  outFile->close();
  
  // Make file executable
  string get_values_exec = "chmod +x "+get_values;
  system(get_values_exec.c_str());

  // Get Values
  // For lines = 1728 Time = 9:23:59
  // For lines = 145 Time = 24 minutes
  cout << "This may take 25 minutes or longer" << endl;
  startDate.ReplaceAll(" ","_");
  TString ToutString = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringData/Bases_"+startDate+"_"+StringUtilities::int2TString(n)+".dat";
  string outString = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringData/getBaseValues1.sh > "+StringUtilities::TString2string(ToutString);
  TStopwatch watch;
  watch.Start();
  cout << outString << endl;
  system(outString.c_str());
  watch.Stop();
  watch.Print();
  
  // Turn text file into root file

}

void MonitoringToRoot(TString inputFile){

  TString inputFileLong = gSystem->ExpandPathName(inputFile.Data());
	cout << "Opening file: " << inputFileLong.Data() << endl;

	ifstream filestream(inputFileLong.Data(), ifstream::in);
	if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return;
	}
	TFile* outFile = TFile::Open(inputFile.Append(".root"),"RECREATE");
	
	// Empty 1 and 2 are necessary since after the 1st line
	// token 1 = date and token 2 = time
	vector < TString > variable_names;
  variable_names.push_back("Empty 1");
  variable_names.push_back("Empty 2");
	
  // vector of structs
  std::vector<ADC_Channel> adc_vec;
  
  /*
  if (s_value.EndsWith(":vmon") || s_value.EndsWith(":imon") || s_value.EndsWith(":v0set")) s_value.Remove(s_value.Length()-4);
  if (s_value.EndsWith(":dac")) s_value.Remove(s_value.Length()-3);
  */
	
	TString line;
  int line_num = 0;
  
  TStopwatch watch;
  watch.Start();
  
	while (line.ReadLine(filestream)){
	  line.Remove(TString::kBoth, ' ');
	  
    TString date = "";
	  
	  TObjArray* tokens = line.Tokenize(" ");
    for (int i=0; i<tokens->GetEntries(); i++){
      TString s_value = ((TObjString*) tokens->At(i))->GetString().Remove(TString::kBoth, ' ');
      // Date var1 var2 ...
      if (line_num==0 && i!=0){
        variable_names.push_back(s_value);
      }
      // Year-Month-Day Hour:Minute:Second va1 val2 ...
      else{
        
        if (i==0) date = s_value;
        else if (i==1) date += " "+s_value;
        else{
          // Get the pv name
          TString name = variable_names[i];
          TString suffix = "";
          if (name.EndsWith(":vmon")) {suffix = "vmon"; name.Remove(name.Length()-5);}
          if (name.EndsWith(":dyn"))  {suffix = "dyn";  name.Remove(name.Length()-4);}
          if (name.EndsWith(":imon")) {suffix = "imon"; name.Remove(name.Length()-5);}
          if (name.EndsWith(":v0set")) {suffix = "v0set"; name.Remove(name.Length()-6);}
          if (name.EndsWith(":dac"))  {suffix = "dac";  name.Remove(name.Length()-4);}
          if (name.EndsWith(":status")) {suffix = "stat"; name.Remove(name.Length()-7);}
          TString prev_name = variable_names[i-1];
          //cout << prev_name << "\t" << name << ":" << endl;
          if ( prev_name.Contains(name+":") ){
            for (int j=int(adc_vec.size())-1; j>=0; j--){
              if (adc_vec[j].absNum==name+"_"+date){
                if (suffix == "vmon") adc_vec[j].hv          = s_value.Atof();
                if (suffix == "dyn")  adc_vec[j].dynode      = s_value.Atof();
                if (suffix == "imon") adc_vec[j].current     = s_value.Atof();
                if (suffix == "v0set") adc_vec[j].temperature      = s_value.Atof();
                if (suffix == "dac")  adc_vec[j].dac         = s_value.Atof();
                if (suffix == "stat") adc_vec[j].status      = s_value.Atoi();
                break;
              }
            }
          }
          else{
            ADC_Channel my_channel(name, date);
            my_channel.absNum = name+"_"+date;
            if (suffix == "vmon") my_channel.hv          = s_value.Atof();
            if (suffix == "dyn")  my_channel.dynode      = s_value.Atof();
            if (suffix == "imon") my_channel.current     = s_value.Atof();
            if (suffix == "v0set") my_channel.temperature      = s_value.Atof();
            if (suffix == "dac")  my_channel.dac         = s_value.Atof();
            if (suffix == "stat") my_channel.status      = s_value.Atof();
            adc_vec.push_back(my_channel);
          }
          
          //cout << name << "\t" << date << "\t" << s_value.Atof() << endl;
        }
      }
    }
	  
	  line_num++;
    //cout << "Line: " << line_num << endl;
    //cout << "Vector Size: " << adc_vec.size() << endl;
	  //if (line_num==4) break;
	}
	
	
  watch.Stop();
  cout << "Reading file took:" << endl;
  watch.Print();
  
	TTree* tr = 0x0;
	
	// Sort
  cout << "Sorting..." << endl;
  std::sort(adc_vec.begin(), adc_vec.end(), sort_By_Name_Date_ASC());
  //std::sort(adc_vec.begin(), adc_vec.end(), sort_ASC());
  cout << "Done Sorting..." << endl;
  int nADC = adc_vec.size();
  
  double time_counter = 0;
  
  for (int i=0; i<nADC; i++){
    
    ADC_Channel empty_channel;
    char date_char[100];
    double my_time = 0;
    
    ADC_Channel my_channel = adc_vec[i];
    TString type = my_channel.type;
    TString date = my_channel.date;
    
    // Create a TTree
    bool create_ttree = false;
    if (i==0) create_ttree = true;
    else if (i>0){
      ADC_Channel my_prev_channel = adc_vec[i-1];
      if (my_channel.type!=my_prev_channel.type){
        create_ttree = true;
        time_counter = 0;
      }
      else if (my_channel.type==my_prev_channel.type){
        time_counter++;
        my_time = double(MonitoringToRoot_getTime(my_prev_channel.date, my_channel.date))*time_counter;
        if (my_time<0) cout << "prev date: " << my_prev_channel.date << "\t current date: " << my_channel.date << endl;
      }
    }
    if (create_ttree){
      cout << "Creating TTree: " << type.ReplaceAll(":","_").ReplaceAll("-","M") << endl;
      tr = new TTree(type.ReplaceAll(":","_").ReplaceAll("-","M"),type.ReplaceAll(":","_").ReplaceAll("-","M"));
      // Create Branches
      tr->Branch("date",&date_char,"date/C");
      tr->Branch("time",&empty_channel.timing,"Minutes/D");
      tr->Branch("vmon",&empty_channel.hv,"Volts/D");
      tr->Branch("dyn",&empty_channel.dynode,"Volts/D");
      tr->Branch("imon",&empty_channel.current,"milliAmps/D");
      tr->Branch("v0set",&empty_channel.temperature,"Volts/D");
      tr->Branch("dac",&empty_channel.dac,"Volts/D");
      tr->Branch("status",&empty_channel.status,"Binary/I");
    }
    
    // Fill branches
    string v0set = StringUtilities::TString2string(my_channel.date);
    sprintf(date_char,"%s",v0set.c_str());
    empty_channel.timing      = my_time;
    empty_channel.hv          = my_channel.hv;
    empty_channel.dynode      = my_channel.dynode;
    empty_channel.current     = my_channel.current;
    empty_channel.temperature       = my_channel.temperature;
    empty_channel.dac         = my_channel.dac;
    empty_channel.status      = my_channel.status;
    tr->Fill();
    
    // Close TTree
    bool close_ttree = false;
    if (i+1!=nADC){
      ADC_Channel my_next_channel = adc_vec[i+1];
      if (my_channel.type!=my_next_channel.type){
        close_ttree = true;
      }
    }
    else if(i+1==nADC) close_ttree = true;
    if (close_ttree) tr->Write();
    
  }
  
  cout << "Finished!!" << endl;
  outFile->Close();
}

// get a map of TGraphs
// key = tree name FCAL_hv_X_Y
// value = TGraph wich is type vs time where time is 10 minute intervals from the start
map<TString, TGraph*> MonitoringToRoot_getTGraphs(TString inFile, TString type){
  
  map<TString, TGraph*> map_graphs;
  
  TFile* f = new TFile(inFile);
  TList* list = f->GetListOfKeys();
  int nList = list->GetEntries();
  cout << "There are " << nList << " TTrees" << endl;
  for (int i=0; i<nList; i++){
    
    double base_value;
    double timing; // Starts at 0 got to N in 10 minute steps
    
    TString tree_name = ((TObjString*)list->At(i))->GetString();
    TTree* tr = (TTree*)f->Get(tree_name);
    tr->SetBranchAddress(type,&base_value);
    tr->SetBranchAddress("time",&timing);
    
    TGraph* gr = new TGraph();
    gr->SetName(tree_name.ReplaceAll("M","-"));
    gr->SetTitle(tree_name.ReplaceAll("M","-"));
    
    int nEntries = tr->GetEntries();
    for (int j=0; j<nEntries; j++){
      tr->GetEntry(j);
      gr->SetPoint(j,timing,base_value);
    }
    map_graphs[tree_name.ReplaceAll("M","-")] = gr;
  }
  return map_graphs;
}

