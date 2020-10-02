//#include "FCALUtilities/StringUtilities.h"
//#include "FCALUtilities/GeneralUtilities.h"
//#include "FCALUtilities/PlotUtilities.h"
//#include "FCALUtilities/DAQUtilities.h"
//#include "FCALUtilities/MonitoringUtilities.h"
#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"


struct Monitoring_Channel{
  
  Monitoring_Channel(TString Name, int X, int Y):name(Name),x(X),y(Y)  {}
  
  TString name;
  
  int x;
  int y;
  
  int n0_vmon_lo;
  int n0_vmon_hi;
  int n0_imon;
  int n0_dyn;
  int n0_temp;
  int n0_status0;
  
  double max_vmon;
  double max_imon;
  double max_dyn;
  double max_temp;
  
  double rms_vmon;
  double rms_imon;
  double rms_dyn;
  double rms_temp;
  
  TGraph* gr_vmon_status0;
  TGraph* gr_imon_status0;
  TGraph* gr_dyn_status0;
  TGraph* gr_temp_status0;
  TGraph* gr_vmon_status1;
  TGraph* gr_imon_status1;
  TGraph* gr_dyn_status1;
  TGraph* gr_temp_status1;

};

struct sort_Monitoring_N0vmonlo_DSC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.n0_vmon_lo > right.n0_vmon_lo;
    }
};
struct sort_Monitoring_N0vmonhi_DSC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.n0_vmon_hi > right.n0_vmon_hi;
    }
};
struct sort_Monitoring_N0imon_DSC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.n0_imon > right.n0_imon;
    }
};
struct sort_Monitoring_N0temp_DSC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.n0_temp > right.n0_temp;
    }
};
struct sort_Monitoring_N0status0_DSC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.n0_status0 > right.n0_status0;
    }
};
struct sort_Monitoring_RMSimon_ASC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.rms_imon < right.rms_imon;
    }
};
struct sort_Monitoring_RMStemp_ASC {
    bool operator()(const Monitoring_Channel &left, const Monitoring_Channel &right) {
        return left.rms_temp < right.rms_temp;
    }
};

TString formatTreeName(int x, int y){
  TString tree_name = TString::Itoa(x,10);
  tree_name += "_";
  tree_name += TString::Itoa(y,10);
  tree_name.ReplaceAll("-","M");
  return tree_name;
}

vector<TString> Monitoring_Studies_basesToMonitor(int skipNum){
  TString prefix = "FCAL_hv_";
  vector<TString> basesToReplace;
  // base to replace
  if (skipNum==1){
    basesToReplace.push_back( prefix+formatTreeName(-25,12) );
    basesToReplace.push_back( prefix+formatTreeName(-1,-16) );
    basesToReplace.push_back( prefix+formatTreeName(-21,-21) );
    basesToReplace.push_back( prefix+formatTreeName(22,-2) );
    basesToReplace.push_back( prefix+formatTreeName(1,-26) );
    basesToReplace.push_back(prefix+formatTreeName(10,15));
    basesToReplace.push_back(prefix+formatTreeName(-16,-22));
    basesToReplace.push_back(prefix+formatTreeName(7,14));
    basesToReplace.push_back(prefix+formatTreeName(19,-14));
    basesToReplace.push_back(prefix+formatTreeName(8,21));
    basesToReplace.push_back(prefix+formatTreeName(2,-11));
    basesToReplace.push_back(prefix+formatTreeName(-17,-24));
    basesToReplace.push_back(prefix+formatTreeName(-20,-9));
    basesToReplace.push_back(prefix+formatTreeName(-2,28));
    basesToReplace.push_back(prefix+formatTreeName(-20,-10));
    basesToReplace.push_back(prefix+formatTreeName(-17,-9));
    basesToReplace.push_back(prefix+formatTreeName(1,-11));
    basesToReplace.push_back(prefix+formatTreeName(-17,22));
    basesToReplace.push_back(prefix+formatTreeName(-5,22));
    basesToReplace.push_back(prefix+formatTreeName(2,15));
    basesToReplace.push_back(prefix+formatTreeName(-8,-9));
  }
  // bases to monitor
  if (skipNum==2){
    basesToReplace.push_back( prefix+formatTreeName(-17,0) );
    basesToReplace.push_back( prefix+formatTreeName(-18,-9) );
    basesToReplace.push_back( prefix+formatTreeName(-22,-7) );
    basesToReplace.push_back( prefix+formatTreeName(-22,-9) );
    basesToReplace.push_back( prefix+formatTreeName(-26,-8) );
    basesToReplace.push_back( prefix+formatTreeName(-9,3) );
    basesToReplace.push_back( prefix+formatTreeName(19,20) );
    basesToReplace.push_back( prefix+formatTreeName(6,21) );
    basesToReplace.push_back( prefix+formatTreeName(8,21) );
  }
  return basesToReplace;
}


TString Monitoring_Studies_formatTitle(TString name, TString type){
  name = name.ReplaceAll("M","-");
  vector<TString> parseTString = StringUtilities::parseTString(name,"_");
  TString title = "("+parseTString[2]+","+parseTString[3]+")";
  if (type=="vmon") title += " Cathode HV";
  else if (type=="dyn")  title += " Dynode HV";
  else if (type=="imon") title += " Current";
  else if (type=="temp") title += " Temperature";
  return title;
}
pair<int,int> Monitoring_Studies_getXY(TString name){
  name = name.ReplaceAll("M","-");
  vector<TString> parseTString = StringUtilities::parseTString(name,"_");
  return std::make_pair(parseTString[2].Atoi(),parseTString[3].Atoi());
}

void Monitoring_Studies_2D(vector<Monitoring_Channel> adcList, TString type, TString savePlots){
  
  int nList = adcList.size();
  
  bool save = savePlots.Length()==0 ? false : true;
  
  TString title = "";
  if (type=="vmon_lo") title = "Number of times HV < 20 Volts";
  if (type=="vmon_hi") title = "Number of times HV > 2100 Volts";
  if (type=="imon")    title = "Number of times Current > 14.5 mA";
  if (type=="temp")    title = "Number of times Temperature > 45 Celsius";
  if (type=="status")  title = "Number of times EPICS did not connect";
  
  map< std::pair<int,int>, double> map_bases;
  
  for (int i=0; i<nList; i++){
    int n0 = 0;
    if (type=="vmon_lo") n0 = adcList[i].n0_vmon_lo;
    else if (type=="vmon_hi") n0 = adcList[i].n0_vmon_hi;
    else if (type=="imon")    n0 = adcList[i].n0_imon;
    else if (type=="temp")    n0 = adcList[i].n0_temp;
    else if (type=="status")  n0 = adcList[i].n0_status0;
    map_bases[std::make_pair(adcList[i].x,adcList[i].y)] = n0;
  }
  TH2F* hist2D = PlotUtilities::Plot2D(map_bases);
  hist2D->SetTitle(title);
  TCanvas* c2D = new TCanvas(savePlots,savePlots,900,900);
  c2D->cd();
  hist2D->Draw("colz");
  if (save) c2D->SaveAs(savePlots+".pdf");
  
}

void Monitoring_Studies_plot(vector<Monitoring_Channel> adcList, int nCanvas, vector<TString> parseType, TString savePlots){
  TCanvas* adc_canv[nCanvas];
  
  int nType = parseType.size();
  
  int c_num = 0;
  int col_num = 0;
  
  bool save = savePlots.Length()==0 ? false : true;
  
  for (int i=0; i<4*nCanvas; i++){
    if (col_num==4) {
      col_num=0; c_num++;
    }
    
    if (col_num==0) {
      char canv_name[50];
      sprintf(canv_name,"%s %i",savePlots.Data(),c_num+1);
      //adc_canv[c_num] = new TCanvas(canv_name,canv_name,900,900);
      adc_canv[c_num] = new TCanvas(canv_name,canv_name,1800,1200);
      adc_canv[c_num]->Divide(4,nType);
    }
    
    col_num++;
    
    for (int j=0; j<nType; j++){
      TString type = parseType[j];
      adc_canv[c_num]->cd(col_num+j*4);
      if (type == "vmon") {
        if (adcList[i].gr_vmon_status0->GetN()>0) {
          adcList[i].gr_vmon_status0->Draw("AP");
          if (adcList[i].gr_vmon_status1->GetN()>0){
            adcList[i].gr_vmon_status1->Draw("P");
          }
        } else {
          adcList[i].gr_vmon_status1->Draw("AP");
        }
      }
      else if (type == "dyn")  {
        if (adcList[i].gr_dyn_status0->GetN()>0) {
          adcList[i].gr_dyn_status0->Draw("AP");
          if (adcList[i].gr_dyn_status1->GetN()>0){
            adcList[i].gr_dyn_status1->Draw("P");
          }
        } else {
          adcList[i].gr_dyn_status1->Draw("AP");
        }
      }
      else if (type == "temp") {
        if (adcList[i].gr_temp_status0->GetN()>0) {
          adcList[i].gr_temp_status0->Draw("AP");
          if (adcList[i].gr_temp_status1->GetN()>0){
            adcList[i].gr_temp_status1->Draw("P");
          }
        } else {
          adcList[i].gr_temp_status1->Draw("AP");
        }
      }
      else if (type == "imon") {
        if (adcList[i].gr_imon_status0->GetN()>0) {
          adcList[i].gr_imon_status0->Draw("AP");
          if (adcList[i].gr_imon_status1->GetN()>0){
            adcList[i].gr_imon_status1->Draw("P");
          }
        } else {
          adcList[i].gr_imon_status1->Draw("AP");
        }
      }
    }
    if (col_num==4){
      if (save){
        adc_canv[c_num]->SaveAs(savePlots+"-"+StringUtilities::int2TString(c_num+1,StringUtilities::int2TString(nCanvas).Length())+".pdf");
      }
    }
  }
}


// Bases_2014-11-14_01:00:00_900.dat.root: 
// Starting Nov 14 and ending December 22 in 1 hour intervals
// Contains: vmon, dyn, temp, imon, dac, time, date
// Color points red if status==0 and black if status==1
// Rank by rms in temp or imon
// Rank by number of times temp or imon > 45
// Rank by number of times vmon < 100
void Monitoring_Studies_openTTree(TString inputRootFile, TString type = "vmon/dyn/imon/temp/dac/status",int nCanvas = 4, int skipNum = 0, TString savePlots = ""){
  
  //int nCanvas = 4;
  TFile* f = new TFile(inputRootFile);
  TList* list = f->GetListOfKeys();
  int nList = list->GetEntries();
  cout << "There are " << nList << " TTrees" << endl;
  
  vector<Monitoring_Channel> adcList;
  
  
  vector<TString> parseType = StringUtilities::parseTString(type,"/");
  int nType = parseType.size();
  
  for (int i=0; i<nList; i++){
    
    
    
    for (int j=0; j<nType; j++){

      type = parseType[j];

      double base_value;
      double timing; // In minutes
      int status;
      
      bool skip = skipNum>0 ? true : false;

      TString tree_name = ((TObjString*)list->At(i))->GetString();
      if (skip){
        // FCAL_hv_M10_M10
        // Only get the following bases:
        vector<TString> basesToReplace = Monitoring_Studies_basesToMonitor(skipNum);
        for (int k=0; k<int(basesToReplace.size()); k++){
          if (tree_name==basesToReplace[k]) {skip = false;}
        }
        if (skip) {continue;}
      }
      
      
      TTree* tr = (TTree*)f->Get(tree_name);
      tr->SetBranchAddress(type,&base_value);
      tr->SetBranchAddress("time",&timing);
      tr->SetBranchAddress("status",&status);

      TGraph* gr_status0 = new TGraph();
      TGraph* gr_status1 = new TGraph();
      gr_status0->SetName(tree_name+":"+type+":status0");
      gr_status1->SetName(tree_name+":"+type+":status1");
      TString title = Monitoring_Studies_formatTitle(tree_name,type);
      gr_status0->SetTitle(title);
      gr_status1->SetTitle(title);

      int n0 = 0;
      int n0_lo =0;
      int n0_hi =0;
      int n0_status0 = 0;
      

      int nEntries = tr->GetEntries();
      for (int k=0; k<nEntries; k++){
        tr->GetEntry(k);
        if (timing<0) continue;
        if (status==0) gr_status0->SetPoint(k,timing,base_value);
        if (status==1) gr_status1->SetPoint(k,timing,base_value);
        if (type=="vmon"){
          if (base_value<20 && status==1)  n0_lo++;
          if (status==0) n0_status0++;
          
          if (base_value>2100) n0_hi++;
          //if (base_value>2000) n0_hi++;
          //if (base_value>2200) n0_hi += 10;
        }
        if (type=="temp" && base_value>45){
          n0++;
        }
        if (type=="imon" && base_value>14.5){
          n0++;
        }
      }
      
      
      if (j==0) {
        std::pair<int,int> coord = Monitoring_Studies_getXY(tree_name);
        Monitoring_Channel my_channel(tree_name,coord.first,coord.second);
        adcList.push_back(my_channel);
      }
      
      Int_t NPoints_status1 = gr_status1->GetN();
      Int_t NPoints_status0 = gr_status0->GetN();
      Double_t* Y_status1   = gr_status1->GetY();
      Double_t* Y_status0   = gr_status0->GetY();
      double max = NPoints_status1>0 ? TMath::MaxElement(NPoints_status1,Y_status1) : TMath::MaxElement(NPoints_status0,Y_status0);
      double rms = NPoints_status1>0 ? TMath::RMS(NPoints_status1,Y_status1)        : TMath::RMS(NPoints_status0,Y_status0);
      
      double max_range_status0 = NPoints_status0>0 ? TMath::MaxElement(NPoints_status0,Y_status0) : 0;
      double max_range_status1 = NPoints_status1>0 ? TMath::MaxElement(NPoints_status1,Y_status1) : 0;
      double max_range = max_range_status1>max_range_status0 ? max_range_status1*1.1 : max_range_status0*1.1;
      gr_status0->GetYaxis()->SetRangeUser(0,max_range);
      gr_status1->GetYaxis()->SetRangeUser(0,max_range);
      
      gr_status0->GetYaxis()->SetTitleOffset(1.8);
      gr_status1->GetYaxis()->SetTitleOffset(1.8);
      gr_status0->GetXaxis()->SetTitle("Time (minutes)");
      gr_status1->GetXaxis()->SetTitle("Time (minutes)");
      gr_status0->SetMarkerColor(kRed);
      gr_status1->SetMarkerColor(kBlack);
      
      if (type=="vmon") {
        gr_status0->GetYaxis()->SetTitle("(V)");
        gr_status1->GetYaxis()->SetTitle("(V)");
        adcList[adcList.size()-1].gr_vmon_status0  = gr_status0;
        adcList[adcList.size()-1].gr_vmon_status1  = gr_status1;
        adcList[adcList.size()-1].max_vmon = max;
        adcList[adcList.size()-1].rms_vmon = rms;
        adcList[adcList.size()-1].n0_vmon_lo  = n0_lo;
        adcList[adcList.size()-1].n0_vmon_hi  = n0_hi;
        adcList[adcList.size()-1].n0_status0  = n0_status0;
      }
      else if (type=="dyn")  {
        gr_status0->GetYaxis()->SetTitle("(V)");
        gr_status1->GetYaxis()->SetTitle("(V)");
        adcList[adcList.size()-1].gr_dyn_status0  = gr_status0;
        adcList[adcList.size()-1].gr_dyn_status1  = gr_status1;
        adcList[adcList.size()-1].max_dyn = max; 
        adcList[adcList.size()-1].rms_dyn = rms; 
        adcList[adcList.size()-1].n0_dyn  = n0;
      }
      else if (type=="temp") {
        gr_status0->GetYaxis()->SetTitle("(C)");
        gr_status1->GetYaxis()->SetTitle("(C)");
        adcList[adcList.size()-1].gr_temp_status0  = gr_status0;
        adcList[adcList.size()-1].gr_temp_status1  = gr_status1;
        adcList[adcList.size()-1].max_temp = max; 
        adcList[adcList.size()-1].rms_temp = rms; 
        adcList[adcList.size()-1].n0_temp  = n0;
      }
      else if (type=="imon") {
        gr_status0->GetYaxis()->SetTitle("(mA)");
        gr_status1->GetYaxis()->SetTitle("(mA)");
        adcList[adcList.size()-1].gr_imon_status0  = gr_status0;
        adcList[adcList.size()-1].gr_imon_status1  = gr_status1;
        adcList[adcList.size()-1].max_imon = max; 
        adcList[adcList.size()-1].rms_imon = rms;
        adcList[adcList.size()-1].n0_imon  = n0;
      }
      // Important!! 
      delete tr;
    }
  }
  
  nCanvas = nCanvas>2800.0/(4.0*4.0) ? 2800.0/(4.0*4.0) : nCanvas;
  
  int nADC = adcList.size();
  cout << "There are " << nADC << " entries in this vector" << endl;
  
  // Plot a certain number of 4 column and number of rows equal to number of types plots 
  //Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"Replace");
  
  // Sort by decreasing order the number of times the status of the channel was 0
  // nCanvas - number of plots
  //std::sort(adcList.begin(),adcList.end(),sort_Monitoring_N0status0_DSC());
  //Monitoring_Studies_2D(adcList, "status", savePlots+"2D-N0status0");
  //Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"N0status0");
  
  // Sort by decreasing order the number of times the voltage dipped below 100 Volts
  std::sort(adcList.begin(),adcList.end(),sort_Monitoring_N0vmonlo_DSC());
  Monitoring_Studies_2D(adcList, "vmon_lo", savePlots+"2D-N0vmonlo");
  Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"N0vmonLo");
  
  // Sort by decreasing order the number of times the voltage surpasses 2100 Volts
  //std::sort(adcList.begin(),adcList.end(),sort_Monitoring_N0vmonhi_DSC());
  //Monitoring_Studies_2D(adcList, "vmon_hi", savePlots+"2D-N0vmonhi");
  //Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"N0vmonHi");
  
  // Sort by decreasing order the number of times the temperature exceeded 45 Celsius
  // Unphysical temperatures must be coming from mismatched ADC value types
  //std::sort(adcList.begin(),adcList.end(),sort_Monitoring_N0temp_DSC());
  //Monitoring_Studies_2D(adcList, "temp", savePlots+"2D-N0temp");
  //Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"N0temp");
  
  // Sort by decreasing order the number of times the current surpasses 14.5 mA
  // Gets bases with stuck high current
 	std::sort(adcList.begin(),adcList.end(),sort_Monitoring_N0imon_DSC());
  Monitoring_Studies_2D(adcList, "imon", savePlots+"2D-N0imon");
  Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"N0imon");
  
  // Sort by the rms of temeperature
  // Hasn't proved useful for diagnostic testing, rarely used
  //std::sort(adcList.begin(),adcList.end(),sort_Monitoring_RMStemp_ASC());
  //Monitoring_Studies_plot(adcList, nCanvas, parseType, savePlots+"RMStemp");
  
}

