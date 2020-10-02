#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TObjArray.h"

/*
Run 1445 had LED pulser data at 150mV
Run 1446 had LED pulser data at 300mV
Run 1995 had LED Pulser data at 600mV
*/

/*
Beam Current:
caget IBCAD00CRCUR6
*/

struct RunInfo_Container{
  RunInfo_Container()  {}
  
  TString run;
  TString config_file;
  TString date;
  TString beam_date;
  TString beam_time;
  double beam_current;
  int filesize; // in kB
  int hv;
  TString mode;
  TString width;
  TString readout_nsb;
  TString readout_nsa;
  TString readout_threshold;
  TString trigger_nsb;
  TString trigger_nsa;
  TString trigger_threshold;
  TString global_threshold;
  TString common_file;
  TString user_file;
  
  // BCAL
  TString bal_readout_nsa;
  TString bal_readout_nsb;
  TString bal_readout_threshold;
  // ST
  TString st_readout_threshold;
  // TOF
  TString tof_readout_threshold;
  // CDC
  
  // FDC
  
  // TAGM
  TString tagm_readout_threshold;
  TString tagm_readout_w_offset;
  // TAGH
  TString tagh_readout_threshold;
  TString tagh_readout_w_offset;
  
};

struct sortByFilesize{
  bool operator()(const RunInfo_Container &left, const RunInfo_Container &right) {
      return left.filesize > right.filesize;
  }
};

struct sortByRun{
  bool operator()(const RunInfo_Container &left, const RunInfo_Container &right) {
      return left.run > right.run;
  }
};

RunInfo_Container RunInfo_parseConfig(RunInfo_Container runinfo){
  
  TString configFile = runinfo.config_file;
  TString inputFileLong = gSystem->ExpandPathName(configFile.Data());
	cout << "Opening file: " << inputFileLong.Data() << endl;

	ifstream filestream(inputFileLong.Data(), ifstream::in);
	if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return runinfo;
	}
	
  //RunInfo_Container runinfo;
  // Get Run Number
	
  bool main_systems = false;
  TString MainSystem = "";
	
	TString line;
  int n = 0;
	
	// Open file and search from top down
	while (line.ReadLine(filestream)){
	  
	  // Remove leading and trailing white spaces
	  line.Remove(TString::kBoth, ' ');
	  
	  // Skip any line that starts with #
    if (line[0]=='#') continue;
	  
	  // look for things enclosed by ==
	  if (line[0]=='='){
      main_systems = true;
      n++;
      if (n>1) n=0;
      continue;
	  }
	  if (main_systems && n==1){
      MainSystem = line;
	  }
    //cout << line << endl;
	  
	  // The first thing to get is the Global Energy Trigger Sum
	  if (MainSystem=="TRIGGER"){
	    if ( line.Contains("TRIG_TYPE") && line.Contains("BFCAL") && line[line.Length()-1]=='0'){
	      TObjArray* tokens = line.Tokenize(" ");
        runinfo.global_threshold = ((TObjString*) tokens->At(6))->GetString().Remove(TString::kBoth, ' ');
	    }
	  }
	  
	  // Next Get the FCAL parameters
	  if (MainSystem=="FCAL"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
	    if ( line.Contains("FADC250_MODE") && nTokens>1 ){
	      runinfo.mode = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.mode = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_W_WIDTH") && nTokens>1 ){
	      runinfo.width = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.width = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_NSB") && nTokens>1 ){
	      runinfo.readout_nsb = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.readout_nsb = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_NSA") && nTokens>1 ){
	      runinfo.readout_nsa = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.readout_nsa = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_READ_THR") && nTokens>1 ){
	      runinfo.readout_threshold = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.readout_threshold = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_TRIG_THR") && nTokens>1 ){
	      runinfo.trigger_threshold = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.trigger_threshold = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_TRIG_NSB") && nTokens>1 ){
	      runinfo.trigger_nsb = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.trigger_nsb = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_TRIG_NSA") && nTokens>1 ){
	      runinfo.trigger_nsa = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.trigger_nsa = "N/A";
	    }
	    
	    if ( line.Contains("FADC250_COM_DIR") && nTokens>1 ){
	      runinfo.common_file = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.common_file = "N/A";
	    }
	    if ( line.Contains("FADC250_COM_VER") && nTokens>1 ){
	      runinfo.common_file += " - "+((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    
	    if ( line.Contains("FADC250_USER_DIR") && nTokens>1 ){
	      runinfo.user_file = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	    if ( line.Contains("FADC250_MODE") && nTokens==1 ){
        runinfo.user_file = "N/A";
	    }
	    if ( line.Contains("FADC250_USER_VER") && nTokens>1 ){
	      runinfo.user_file += " - "+((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
	    }
	  }
	  
	  
	  
	  // Next Get the BCAL parameters
	  if (MainSystem=="BCAL"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
      if ( line.Contains("FADC250_NSB") && nTokens>1 ){
        runinfo.bal_readout_nsb += ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_NSB") && nTokens==1 ){
        runinfo.bal_readout_nsb += "N/A";
      }
      
      if ( line.Contains("FADC250_NSA") && nTokens>1 ){
        runinfo.bal_readout_nsa += ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_NSA") && nTokens==1 ){
        runinfo.bal_readout_nsa += "N/A";
      }
      
      if ( line.Contains("FADC250_READ_THR") && nTokens>1 ){
        runinfo.bal_readout_threshold += ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_READ_THR") && nTokens==1 ){
        runinfo.bal_readout_threshold += "N/A";
      }
	  }
	  
	  // Next Get the ST parameters
	  //============ ST ====================
    //THR 120
	  if (MainSystem=="ST"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
      if ( line.Contains("FADC250_READ_THR") && nTokens>1 ){
        runinfo.st_readout_threshold = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_READ_THR") && nTokens==1 ){
        runinfo.st_readout_threshold = "N/A";
      }
	  }
	
	  // Next Get the TOF parameters
	  //============ TOF ==================
    //THR 210 (baseline is 200)
	  if (MainSystem=="TOF"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
      if ( line.Contains("FADC250_READ_THR") && nTokens>1 ){
        runinfo.tof_readout_threshold = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_READ_THR") && nTokens==1 ){
        runinfo.tof_readout_threshold = "N/A";
      }
	  }
	  
	  // Next Get the CDC parameters
	  //============ CDC ==================
    //Per channel threshold 7 sigma over baseline
	  if (MainSystem=="CDC"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
	  }
	  
	  // Next Get the FDC parameters
	  /*
	    ============ FDC ==================
      Per channel threshold 5 sigma
	  */
	  if (MainSystem=="FDC"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
	  }
	  
	  // Next Get the TAGM parameters
	  /*
	  THR 105
    W_OFFSET 975
	  */
	  if (MainSystem=="TAGM"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
      if ( line.Contains("FADC250_READ_THR") && nTokens>1 ){
        runinfo.tagm_readout_threshold = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_READ_THR") && nTokens==1 ){
        runinfo.tagm_readout_threshold = "N/A";
      }
      
      if ( line.Contains("FADC250_W_OFFSET") && nTokens>1 ){
        runinfo.tagm_readout_w_offset = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_W_OFFSET") && nTokens==1 ){
        runinfo.tagm_readout_w_offset = "N/A";
      }
	  }

	  // Next Get the TAGH parameters
	  /*
	  THR 120
    W_OFFSET 975
	  */
	  if (MainSystem=="TAGH"){
	    TObjArray* tokens = line.Tokenize(" ");
      int nTokens = tokens->GetEntries();
      if ( line.Contains("FADC250_READ_THR") && nTokens>1 ){
        runinfo.tagh_readout_threshold = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_READ_THR") && nTokens==1 ){
        runinfo.tagh_readout_threshold = "N/A";
      }
      
      if ( line.Contains("FADC250_W_OFFSET") && nTokens>1 ){
        runinfo.tagh_readout_w_offset = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
      }
      if ( line.Contains("FADC250_W_OFFSET") && nTokens==1 ){
        runinfo.tagh_readout_w_offset = "N/A";
      }
	  }
	  
    main_systems = false;
	}
	
	return runinfo;
}

RunInfo_Container RunInfo_parseBeamCurrent(RunInfo_Container runinfo){
  
  // Execute command to get beam current for the run period
  // Get time from EVIO files
  // beam_date: Year-Month-Day
  // beam_time: Hour:Minute:Second
  
  TString beam_date = runinfo.beam_date;
  TString beam_time = runinfo.beam_time;
  cout << "Beam Date: " << beam_date << "\tBeam Time: " << beam_time << endl;
  vector<TString> parse_time = StringUtilities::parseTString(beam_time,".");
  beam_time = parse_time[0];
  TString startDate = beam_date+" "+beam_time;
  
  TString inputFileLong = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/BeamCurrent";
  
  // Average the next 10 minutes from the start time
  TString Tcommand = "mySampler -b \""+startDate+"\" -s 1m -n 10 IBCAD00CRCUR6 > "+inputFileLong;
  cout << Tcommand << endl;
  system(Tcommand.Data());
  
  // Parse ConfigFiles for path and date in Year-Month-Day
	cout << "Opening file: " << inputFileLong.Data() << endl;
	ifstream filestream(inputFileLong.Data(), ifstream::in);
	if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return runinfo;
	}
	
	TString line;
  double beam_current = 0;
  double counter = 0;
	
  while (line.ReadLine(filestream)){
    if (line.Contains("Date")) continue;
    TObjArray* tokens = line.Tokenize(" ");
    beam_current += ((TObjString*) tokens->At(2))->GetString().Remove(TString::kBoth, ' ').Atoi();
    counter++;
  }
  
  cout << "Beam Current: " << beam_current << " N: " << counter << endl;
  
  runinfo.beam_current = beam_current/counter;
  
  return runinfo;
}

RunInfo_Container RunInfo_parseEVIO(RunInfo_Container runinfo){
  
  // Execute script to get evio locations and file sizes in kilobytes
  // The outputs file is called EVIOFiles
  // The pattern is: path to hd_rawdata*evio  file size if kB Year-Month-Day  Hour:Minute:Second
  system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/FindEVIO");
  
  // Parse ConfigFiles for path and date in Year-Month-Day
  TString inputFileLong = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/EVIOFiles";
	cout << "Opening file: " << inputFileLong.Data() << endl;
	ifstream filestream(inputFileLong.Data(), ifstream::in);
	if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return runinfo;
	}
	
  TString run = runinfo.run;
	
	TString line;
	
  int filesize = 0;
  int counter = 0;
	
  while (line.ReadLine(filestream)){
    
    TObjArray* tokens = line.Tokenize(" ");
    TString evio_file = ((TObjString*) tokens->At(0))->GetString().Remove(TString::kBoth, ' ');
    if (evio_file.Contains(run)){
      filesize += ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ').Atoi();
      counter++;
      if (counter==1){
        cout << "EVIO: " << line << endl;
        runinfo.beam_date = ((TObjString*) tokens->At(2))->GetString().Remove(TString::kBoth, ' ');
        runinfo.beam_time = ((TObjString*) tokens->At(3))->GetString().Remove(TString::kBoth, ' ');
        runinfo = RunInfo_parseBeamCurrent(runinfo);
      }
    }
  }
  
  runinfo.filesize = filesize;
  
  return runinfo;
}


void RunInfo(){
  
  // Execute Script to get Run Configurations
  // The output file is called ConfigFiles
  // The pattern is: path to fcal_all.conf Year-Month-Day  Hour:Minute:Second
  system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/FindRunConfigs");
  
  // Execute script to get evio locations and file sizes in kilobytes
  // The outputs file is called EVIOFiles
  // The pattern is: path to hd_rawdata*evio  file size if kB Year-Month-Day  Hour:Minute:Second
  system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/FindEVIO");
  
  // Parse ConfigFiles for path and date in Year-Month-Day
  TString inputFileLong = "/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/ConfigFiles";
	cout << "Opening file: " << inputFileLong.Data() << endl;
	ifstream filestream(inputFileLong.Data(), ifstream::in);
	if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return;
	}
	
  vector<RunInfo_Container> run_list;
	
  TString line;
	
  while (line.ReadLine(filestream)){
    
    RunInfo_Container runinfo;
    
    TObjArray* tokens = line.Tokenize(" ");
    runinfo.config_file = ((TObjString*) tokens->At(0))->GetString().Remove(TString::kBoth, ' ');
    runinfo.date = ((TObjString*) tokens->At(1))->GetString().Remove(TString::kBoth, ' ');
    
    vector<TString> tokensRun = StringUtilities::parseTString(runinfo.config_file,"/");
    runinfo.run = tokensRun[5];
    
    // Get File Size and Beam Current
    runinfo = RunInfo_parseEVIO(runinfo);
    
    // Get Config Parameters
    runinfo = RunInfo_parseConfig(runinfo);

    // Get HV Setting For Bases
    // Runs > 1280 had HV set to 300mV
    // Runs >= 1770 had HV set 600mV
    TString run = runinfo.run;
    int irun = run.ReplaceAll("Run","").Atoi();
    if (irun<1280) runinfo.hv = 150;
    else if (irun>=1280 && irun<1770) runinfo.hv = 300;
    else runinfo.hv = 600;
    cout << runinfo.hv << endl;
    run_list.push_back(runinfo);
    
  }
  
  //std::sort(run_list.begin(),run_list.end(),sortByFilesize());
  std::sort(run_list.begin(),run_list.end(),sortByRun());
  cout << "Sorting By Run" << endl;
  
  int nRuns = run_list.size();
  cout << "There are " << nRuns << " runs" << endl;
  
  fstream* outFile = new fstream();
  outFile->open("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/RunInfo/RunInfo.tab", std::fstream::out);
  
  vector<TString> TableCells;
  
  TableCells.push_back("Run");
  TableCells.push_back("Date");
  TableCells.push_back("Filesize (kB)");
  TableCells.push_back("Beam Current (nA)");
  TableCells.push_back("Run Config File");
  TableCells.push_back("Mode");
  TableCells.push_back("FCAL Threshold HV (mV)");
  TableCells.push_back("FCAL Width");
  TableCells.push_back("FCAL Readout NSB");
  TableCells.push_back("FCAL Readout NSA");
  TableCells.push_back("FCAL Readout Threshold");
  TableCells.push_back("FCAL Trigger NSB");
  TableCells.push_back("FCAL Trigger NSA");
  TableCells.push_back("FCAL Trigger Threshold");
  TableCells.push_back("FCAL Global Trigger Sum");
  TableCells.push_back("FCAL Common File");
  TableCells.push_back("FCAL Masked Channels");
  
  // BCAL
  TableCells.push_back("BCAL Readout NSB");
  TableCells.push_back("BCAL Readout NSA");
  TableCells.push_back("BCAL Readout Threshold");
  // ST
  TableCells.push_back("ST Readout Threshold");
  // TOF
  TableCells.push_back("TOF Readout Threshold");
  // CDC
  
  // FDC
  
  // TAGM
  TableCells.push_back("TAGM Readout Threshold");
  TableCells.push_back("TAGM W Offset");
  // TAGH
  TableCells.push_back("TAGH Readout Threshold");
  TableCells.push_back("TAGH W Offset");

  int nCells = TableCells.size();
  TString header = "";
  for (int i=0; i<nCells; i++){
    header += TableCells[i]+"\t";
  }
  *outFile << header << endl;
  
  for (int i=0; i<nRuns; i++){
    RunInfo_Container runinfo = run_list[i];
    
    vector<TString> parseConfig = StringUtilities::parseTString(runinfo.config_file,"/");
    TString config_file = parseConfig[parseConfig.size()-1];
    
    TString Content = "";
    
    for (int j=0; j<nCells; j++){
      if (TableCells[j] == "Run")                       Content += runinfo.run;
      if (TableCells[j] == "Date")                      Content += runinfo.date;
      if (TableCells[j] == "Filesize (kB)")             Content += StringUtilities::int2TString(runinfo.filesize);
      if (TableCells[j] == "Beam Current (nA)")         Content += StringUtilities::double2TString(runinfo.beam_current);
      if (TableCells[j] == "Run Config File")           Content += config_file;
      if (TableCells[j] == "Mode")                      Content += runinfo.mode;
      if (TableCells[j] == "FCAL Threshold HV (mV)")    Content += StringUtilities::int2TString(runinfo.hv);
      if (TableCells[j] == "FCAL Width")                Content += runinfo.width;
      if (TableCells[j] == "FCAL Readout NSB")          Content += runinfo.readout_nsb;
      if (TableCells[j] == "FCAL Readout NSA")          Content += runinfo.readout_nsa;
      if (TableCells[j] == "FCAL Readout Threshold")    Content += runinfo.readout_threshold;
      if (TableCells[j] == "FCAL Trigger NSB")          Content += runinfo.trigger_nsb;
      if (TableCells[j] == "FCAL Trigger NSA")          Content += runinfo.trigger_nsa;
      if (TableCells[j] == "FCAL Trigger Threshold")    Content += runinfo.trigger_threshold;
      if (TableCells[j] == "FCAL Global Trigger Sum")   Content += runinfo.global_threshold;
      if (TableCells[j] == "FCAL Common File")          Content += runinfo.common_file;
      if (TableCells[j] == "FCAL Masked Channels")      Content += runinfo.user_file;
      
      // BCAL
      if (TableCells[j] == "BCAL Readout NSB")          Content += runinfo.bal_readout_nsb;
      if (TableCells[j] == "BCAL Readout NSA")          Content += runinfo.bal_readout_nsa;
      if (TableCells[j] == "BCAL Readout Threshold")    Content += runinfo.bal_readout_threshold;
      // ST
      if (TableCells[j] == "ST Readout Threshold")      Content += runinfo.st_readout_threshold;
      // TOF
      if (TableCells[j] == "TOF Readout Threshold")     Content += runinfo.tof_readout_threshold;
      // CDC

      // FDC

      // TAGM
      if (TableCells[j] == "TAGM Readout Threshold")    Content += runinfo.tagm_readout_threshold;
      if (TableCells[j] == "TAGM W Offset")             Content += runinfo.tagm_readout_w_offset;
      // TAGH
      if (TableCells[j] == "TAGH Readout Threshold")    Content += runinfo.tagh_readout_threshold;
      if (TableCells[j] == "TAGH W Offset")             Content += runinfo.tagh_readout_w_offset;
      
      Content += "\t";
    }
    
    *outFile << Content << endl;
    
  }
  outFile->close();
}


/*
struct sortByMax_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.maxADC > right.maxADC;
    }
};

void RunInfo_parseHotFile(){
  
  TString inputFileLong = "/Users/manny/Code/DAQ/new_code/ROOTscripts/HotChannels.txt";
	cout << "Opening file: " << inputFileLong.Data() << endl;
	ifstream filestream(inputFileLong.Data(), ifstream::in);
	if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return;
	}
		
  TString line;
  
  vector<ADC_Channel> run_list;
  
  while (line.ReadLine(filestream)){
    
    TObjArray* tokens = line.Tokenize(" ");
    
    int X = ((TObjString*) tokens->At(2))->GetString().Remove(TString::kBoth, ' ').Atoi();
    int Y = ((TObjString*) tokens->At(5))->GetString().Remove(TString::kBoth, ' ').Atoi();
    int Num = ((TObjString*) tokens->At(8))->GetString().Remove(TString::kBoth, ' ').Atoi();
    
    ADC_Channel MY_CHANNEL;
    MY_CHANNEL.x = X;
    MY_CHANNEL.y = Y;
    MY_CHANNEL.maxADC = Num;
    run_list.push_back(MY_CHANNEL);
    
  }
  
  std::sort(run_list.begin(),run_list.end(),sortByMax_DSC());
  int n = run_list.size();
  for (int i=0; i<n; i++){
    cout << run_list[i].x << "\t" << run_list[i].y << "\t" << run_list[i].maxADC << endl;
  }
  
}
*/

