
#include "MonitoringUtilities.h"
#include "PlotUtilities.h"


map<TString, TGraph*> MonitoringUtilities::Monitoring_adcVsTime(std::vector<ADC_Channel> adcList, string monitoringFile, int everyN, TString dir, string adc, bool plot, int j){
  
  // Aboout Once every Minute and a Half
  //double freq = 1.0/90.0; // Hz
  // correction factor = 22/24
  //double freq = 22.0/24.0 * 1.0/90.0; // Hz
  
  //ped, max, charge, timing
  TString adc_title = "";
  if (adc=="CAT") adc_title = "HV (V): ";
  if (adc=="CUR") adc_title = "C (mA): ";
  if (adc=="TEM") adc_title = "T (C): ";
  /* 
  ** 14 dynodes plus cathode are split into the top 8 and bottom 8, 
  ** and it's the potential that powers those top or bottom 8.  
  ** Essentially the potential from the 24V before it gets distributed to the cathode and dynodes
  */
  // Dynode 1
  if (adc=="DYN") adc_title = "Dyn (V): ";
  // Medium Voltage Bottom
  if (adc=="MVB") adc_title = "MVB (V): ";
  // Medium Voltage Top
  if (adc=="MVT") adc_title = "MVT (V): ";
  // Digital to analog converter.  The thing that actually turns the bits from the CAN into a real potential
  if (adc=="DAC") adc_title = "DAC: ";
  
  // sortby = channel, event, slot, or absnum
  int adcSize = adcList.size();
  cout << "Size: " << adcSize << endl;
  if (adcSize==0) {adcList = MonitoringUtilities::Monitoring_HV(monitoringFile,j);}
  //std::vector<ADC_Channel> adcList = GeneralAnalysis::Monitoring_HV(fileName, adc);
  
  
  map<TString, TGraph*> adc_map;
  
  TGraph* adc_gr = 0x0;
  
  double avgADC = 0;
  int gcounter = 0;
  int counter = 0;
  double SET_TIME = 0;
  
  for (int i=0; i<adcSize; i++){
    ADC_Channel MY_ADC = adcList[i];
    int MY_CRATE        = MY_ADC.crate;
    //cout << "Crate: " << MY_CRATE << endl;
    int MY_SLOT         = MY_ADC.slot;
    //cout << "Slot: " << MY_SLOT << endl;
    int MY_CHANNEL      = MY_ADC.channel;
    //cout << "Channel: " << MY_CHANNEL << endl;
    
    double MY_ADC_VAL = 0;
    if (adc=="CAT") MY_ADC_VAL = MY_ADC.hv;
    if (adc=="CUR") MY_ADC_VAL = MY_ADC.current;
    if (adc=="TEM") MY_ADC_VAL = MY_ADC.temperature;
    if (adc=="DYN") MY_ADC_VAL = MY_ADC.dynode;
    if (adc=="MVB") MY_ADC_VAL = MY_ADC.mvb;
    if (adc=="MVT") MY_ADC_VAL = MY_ADC.mvt;
    if (adc=="DAC") MY_ADC_VAL = MY_ADC.dac;
    //cout << "ADC_VAL: " << MY_ADC_VAL << endl;
    
  
    int MY_EVENT        = MY_ADC.event;
    int MY_X            = MY_ADC.x;
    int MY_Y            = MY_ADC.y;
    
    double MY_TIME = MY_ADC.timing;
    
    TString name = GeneralUtilities::formatTitle(MY_CRATE, MY_SLOT, MY_CHANNEL);
    //TString name = adc_title + GeneralAnalysis::formatTitle2(MY_CRATE, MY_SLOT, MY_CHANNEL);
    TString T_coord = " ("+StringUtilities::int2TString(MY_X)+","+StringUtilities::int2TString(MY_Y)+")";
    TString gr_title = adc_title + GeneralUtilities::formatTitle2(MY_CRATE, MY_SLOT, MY_CHANNEL)+T_coord;
    //TString gr_title = adc_title + T_coord;
    
    bool doAvg = MY_EVENT % everyN == 0;
    //if ( MY_EVENT==GeneralAnalysis::last_event) doAvg = true;
    
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
      
      // Check if the next element is the same crate/slot/channel as the current element
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
    
    // Get Timing
    if (i!=0) {
      ADC_Channel MY_PREV_ADC = adcList[i-1];
      double MY_PREV_TIME = MY_PREV_ADC.timing;
      int MY_PREV_CRATE   = MY_PREV_ADC.crate;
      int MY_PREV_SLOT    = MY_PREV_ADC.slot;
      int MY_PREV_CHANNEL = MY_PREV_ADC.channel;
      
      if (MY_CRATE!=MY_PREV_CRATE || MY_SLOT!=MY_PREV_SLOT || MY_CHANNEL!=MY_PREV_CHANNEL){
        SET_TIME = 0;
      } 
      if (MY_CRATE==MY_PREV_CRATE && MY_SLOT==MY_PREV_SLOT && MY_CHANNEL==MY_PREV_CHANNEL){
        double TempTime = MY_TIME - MY_PREV_TIME;
        if ( TempTime<0 ) TempTime += 24;
        SET_TIME += TempTime;
      }
      
    }
    
    if (fill_map){
      //cerr << "Fitting" << endl;
      //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
      adc_map[name] = adc_gr;
      adc_gr = 0x0;
      gcounter = 0;
    }
    
    if (!fill_map){
      //cerr << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
      avgADC += MY_ADC_VAL;
      counter ++;
      // x is in hours
      if (doAvg) {
        //cerr << gcounter << "\t" << 1.0/pulser_freq*1/3600.0*double(everyN)*double(gcounter) << "\t" << MY_CRATE << "/" << MY_SLOT << "/" << MY_CHANNEL << endl;
        //adc_gr->SetPoint(gcounter, 1.0/freq*1/3600.0*double(everyN)*double(gcounter), avgADC / double(counter) ); 
        //cout << "time: " << SET_TIME << endl;
        //cout << "setting point" << endl;
        adc_gr->SetPoint(gcounter, SET_TIME, avgADC / double(counter) ); 
        avgADC=0; counter=0;
        gcounter++;
      }
    }
    
    //cout << name << "\t" << event-GeneralAnalysis::first_event+1 << "\t" << x << "," << y << "\tAvg: " << doAvg << endl;
    
  }
  
  //GeneralAnalysis::plotGraphs(fileName, ped_map, true, dir, "Time (hours)", "Pedestal (Channels)");
  if (plot) {PlotUtilities::PlotAllGraphs(monitoringFile, adc_map, true, dir, "Time (hours)", "", 0, 0, adc);}
  //if (removeGlobal) {adcList.clear();}
  return adc_map;
}


// ndates and nlines
std::pair<int,int> MonitoringUtilities::CountDatesInFile(string fileName){
  int ndates = 0;
  int nlines = 0;
  string line;
  ifstream myfile;
  myfile.open(fileName.c_str());
  if (myfile.is_open())
  {
    while( getline (myfile,line) )
    {
      nlines++;
      if (line.compare(0,5,"Time:")==0){
        ndates++;
      }
    }
  }
  TString t_dates = StringUtilities::int2TString(ndates);
  std::pair<int,int> nlines_pair;
  nlines_pair = std::make_pair(t_dates.Length(),nlines);
  return nlines_pair;
}
// date = Thu Jul 24 12:41:54 2014
double MonitoringUtilities::DateToHours(TString date){
  vector<TString> parseDate = StringUtilities::parseTString(date," ");
  vector<TString> parseTime =  StringUtilities::parseTString(parseDate[3],":");
  double hour   = StringUtilities::TString2double(parseTime[0]);
  double minute = StringUtilities::TString2double(parseTime[1]);
  double second = StringUtilities::TString2double(parseTime[2]);
  return (hour + minute/60.0 + second/3600.0);
}

// Reading Frome a text file and return a map of key= Crate Slot Channel and val= TGraph
std::vector<ADC_Channel> MonitoringUtilities::Monitoring_HV(string monitoringFile, int j){
  
  cout << "Loading Base to XY Map" << endl;
  std::map<int,std::pair<int,int> > baseToCoordMap2 = GeneralUtilities::baseToCoordMap(j);
  
  cout << "Loading XY to Crate/Slot/Channel Map" << endl;
  std::map<std::pair<int,int>, TString > coordToDAQ_map = GeneralUtilities::BaseLocationMapInverse();
  
  // Count the number of lines in the text file
  std::pair<int,int> nlines_pair = MonitoringUtilities::CountDatesInFile(monitoringFile);
  int ndigits = nlines_pair.first;
  int nlines = nlines_pair.second;
  
  // vector of structs
  std::vector<ADC_Channel> adc_vec;
  
  // setup variables
  int base_id; 
  double adc_val;
  string adc_type;
  
  TString date[2];
  int counter = 0;
  int time_counter = 0;
  
  string line;
  std::istringstream lin;
  ifstream myfile;
  myfile.open(monitoringFile.c_str());
  
  if (myfile.is_open())
  {
    while( getline (myfile,line) )
    {
      counter++;
      
      lin.clear();
      lin.str(line);
      std::istringstream iss(line);
      
      bool write_map = false;
      if (counter==nlines) write_map = true;
      
      if (line.compare(0,5,"Time:")==0){
        line.erase(0,6);
        
        if (counter==1) date[0] = StringUtilities::string2TString(line);
        if (counter!=1) {
          write_map = true;
          date[1] = StringUtilities::string2TString(line);
        }
      }
      
      else if (iss >> base_id >> adc_type >> adc_val) {
        
        
        if (StringUtilities::string2TString(line).Contains("|")) continue;
        
        
        if (base_id==0) {
        }
        
        // Write out to HV
        if (base_id!=0) {
          //cout << base_id << endl;
          std::pair<int,int> coord = baseToCoordMap2.find(base_id)->second;
          
          int x = coord.first; int y = coord.second;
          //cout << "x = " << x <<"\t y = " << y << endl;
          TString DAQ_loc = coordToDAQ_map.find(coord)->second;
          vector<TString> parse_DAQ = StringUtilities::parseTString(DAQ_loc,"/");
          int crate   = StringUtilities::TString2int(parse_DAQ[0])+1;
          int slot    = StringUtilities::TString2int(parse_DAQ[1]);
          int channel = StringUtilities::TString2int(parse_DAQ[2]);
          
          //cout << crate << "/" << slot << "/" << channel << endl;
          
          ADC_Channel my_channel(crate, slot, channel, time_counter+1);
          my_channel.x = x;
          my_channel.y = y;
          
          char temp_absNum[100];
          sprintf(temp_absNum,"%02i_%02i_%02i_",my_channel.crate,my_channel.slot,my_channel.channel);
          TString t_temp_absNum = temp_absNum;
          t_temp_absNum += "_"+StringUtilities::int2TString(time_counter+1,ndigits);
          my_channel.absNum = t_temp_absNum;
          
          my_channel.date = date[0];
          my_channel.timing = DateToHours(my_channel.date);
          
          // Find if a crate slot channel has already been added to this list
          int adcSize = adc_vec.size();
          bool found = false;
          if (adc_type!="CAT"){
            for (int i=0; i<adcSize; i++){
              if (my_channel.absNum==adc_vec[adcSize-i-1].absNum) {
                if (adc_type=="CAT") adc_vec[adcSize-i-1].hv           = adc_val;
                if (adc_type=="CUR") adc_vec[adcSize-i-1].current      = adc_val*1000; // convert Amps to milliAmps
                if (adc_type=="TEM") adc_vec[adcSize-i-1].temperature  = adc_val;
                if (adc_type=="DYN") adc_vec[adcSize-i-1].dynode       = adc_val;
                if (adc_type=="MVB") adc_vec[adcSize-i-1].mvb          = adc_val;
                if (adc_type=="MVT") adc_vec[adcSize-i-1].mvt          = adc_val;
                if (adc_type=="DAC") adc_vec[adcSize-i-1].dac          = adc_val;
                found = true; break;
              }
            }
          }
          if (!found){
            if (adc_type=="CAT") my_channel.hv           = adc_val;
            if (adc_type=="CUR") my_channel.current      = adc_val*1000; // convert Amps to milliAmps
            if (adc_type=="TEM") my_channel.temperature  = adc_val;
            if (adc_type=="DYN") my_channel.dynode       = adc_val;
            if (adc_type=="MVB") my_channel.mvb          = adc_val;
            if (adc_type=="MVT") my_channel.mvt          = adc_val;
            if (adc_type=="DAC") my_channel.dac          = adc_val;
            adc_vec.push_back(my_channel);
          }
          
        }
      }
      
      if (write_map){
        //cout << date[0].Prepend(StringUtilities::int2TString(time_counter,ndigits)+"_") << endl;
        time_counter++;
        //date[0].Prepend(StringUtilities::int2TString(time_counter,ndigits)+"_");
        date[0] = date[1];
      }
      
    } // end while loop over file
  }
  myfile.close();
  
  // Sort by absnum
  TString sortby = "absnum";
  if(sortby == "absnum") {
    GeneralUtilities::sort_by_slot = false;
    GeneralUtilities::sort_by_channel_event = true;
    GeneralUtilities::sort_by_channel=false;
    GeneralUtilities::sort_by_event=false;
  }
  
  if (GeneralUtilities::sort_by_channel_event) {
    cout << "Sorting..." << endl;
    std::sort(adc_vec.begin(),adc_vec.end(),sort_ASC());
    GeneralUtilities::sort_by_channel = false;
    GeneralUtilities::sort_by_event = false;
    GeneralUtilities::sort_by_slot  = false;
  }
  
  // Monitoring_07-30-14.txt
  // Test code
  //for (int i=0; i<int(adc_vec.size()); i++){
  //  cout << adc_vec[i].absNum << "\t";
  //  if (which_adc=="CAT") cout << adc_vec[i].hv << endl;
  //  if (which_adc=="CUR") cout << adc_vec[i].current  << endl;
  //  if (which_adc=="TEM") cout << adc_vec[i].temperature << endl;
  //  
  //}
  
  
  return adc_vec;
}

// key = Row:Col, value = strand #
//std::map<std::pair<string,int> > matchStrand(){
vector<BaseStrand> MonitoringUtilities::MatchStrand(){
  
  vector<BaseStrand> baseStrands;
  
  int edges[] = {7, 10, 13, 14, 16, 17, 19, 20, 21, 22, 23, 23, 24, 25, 25, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 27, 27, 27, 26, 25, 25, 24, 23, 23, 22, 21, 20, 19, 17, 16, 14, 13, 10, 7};
  int x = -7;
  int y = 29;
  int strand = 1;
  int counter = 0;
  bool next_row = false;
  bool south = false;
  bool north = !south;
  bool from_middle = false;
  bool from_edge = !from_middle;
  for (int i=0; i<2800; i++){
    
    next_row = false;
    bool switch_now = false;
    if (x==-2 && y==1) switch_now = true;
    if (x==29 && y==0) switch_now = true;
    
    if (y>0 && x<=0) {south = true;  north = !south;}
    if (y>0 && x>0)  {south = false; north = !south;}
    if (y<0 && x>=0) {south = false; north = !south;}
    if (y<0 && x<0)  {south = true;  north = !south;}
    
    from_middle  = (counter+1)%2==0;
    from_edge    = !from_middle;
    
    if (south && strand>=8){
      from_middle  = (counter+1)%2==1;
      from_edge    = !from_middle;
    }
    if (north && strand>=22){
      from_middle  = (counter+1)%2==1;
      from_edge    = !from_middle;
    }
    
    double absNum = (i+1)%100;
    if (absNum==0) absNum = 100;
    if ((south && y<=0) || (north && y<=-1)) absNum = 101 - absNum;
    
    BaseStrand my_basestrand(absNum, x, y, strand);
    baseStrands.push_back(my_basestrand);
    //cout << absNum << "\t" << x << "\t" << y << "\tStrand: " << strand << endl;
    
    //cout << "from middle:\t" << from_middle << endl;
    
    // Checks the middle
    if (from_edge){
      if (south && y>0 && x== 0) next_row = true;
      if (south && y<0 && x==-1) next_row = true;
      if (north && y>0 && x== 1) next_row = true;
      if (north && y<0 && x== 0) next_row = true;
      if ((fabs(y)==1 || y==0) && fabs(x)==2) next_row = true;
    }
    // checks the edges
    if (from_middle){
      if (south && x==edges[counter]*-1)  next_row = true;
      if (north && x==edges[counter])     next_row = true;
    }
    
    if (!next_row){
      if (south && from_edge) x++;
      if (south && from_middle) x--;
      if (north && from_edge) x--;
      if (north && from_middle) x++;
    }
    
    if (next_row) {
      y--;
      if (y>=-29)  counter++;
      if (y< -29) {counter = 0; y = 29; north = true; south = !north;}
      
      if (south && from_middle) x = edges[counter]*-1;
      if (south && from_edge && y>1) x = 0;
      if (south && from_edge && (fabs(y)==1 || y==0)) x = -2;
      if (south && from_edge && y<-1) x = -1;
      
      if (south && switch_now) x = edges[counter]*-1;
      
      if (north && from_middle) x = edges[counter];
      if (north && from_edge && y>1) x = 1;
      if (north && from_edge && (fabs(y)==1 || y==0)) x = 2;
      if (north && from_edge && y<-1) x = 0;
      
      if (north && switch_now) x = 2;
    }
    
    if ((i+1)%100==0) strand++;
    
  }
  
  return baseStrands;
}

std::pair<int,int> MonitoringUtilities::GetStrandPair(int strand, int Pos){
  if (Pos>100) cout << "Please enter a number <= 100" << endl;
  vector<BaseStrand> FullList = MonitoringUtilities::MatchStrand();
  std::pair<int,int> strandpair;
  int counter = 0;
  for (int i=0; i<2800; i++){
    BaseStrand MY_BASESTRAND = FullList[i];
    int absPos = MY_BASESTRAND.absPos;
    int X = MY_BASESTRAND.X;
    int Y = MY_BASESTRAND.Y;
    int Strand = MY_BASESTRAND.Strand;
    if (strand==Strand && Pos==absPos){
      strandpair = std::make_pair(X,Y);
      break;
    }
    
  }
  return strandpair;
}

int MonitoringUtilities::GetStrand(int x, int y){
  
  vector<BaseStrand> FullList = MonitoringUtilities::MatchStrand();
  std::pair<int,int> strandpair;
  int counter = 0;
  for (int i=0; i<2800; i++){
    BaseStrand MY_BASESTRAND = FullList[i];
    int absPos = MY_BASESTRAND.absPos;
    int X = MY_BASESTRAND.X;
    int Y = MY_BASESTRAND.Y;
    int Strand = MY_BASESTRAND.Strand;
    if (x==X && y==Y){
      return Strand;
      cout << "Strand: " << Strand << "\tPos: " << absPos << endl;
      break;
    }
  }
  return 0;
}
