#include <inttypes.h>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>

#include "TText.h"
#include "TLatex.h"
#include "TPaveStats.h"
#include "TVirtualPad.h"
#include "TFrame.h"
#include "TGraphPainter.h"
#include "TString.h"
#include "TCollection.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMinuit.h"
#include "TKey.h"
#include "TDatime.h"
#include "TMath.h"
#include "TAxis.h"
#include "TLine.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH2F.h"
#include "TColor.h"
#include "TStyle.h"
#include "TPaletteAxis.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TObjArray.h"


#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"

struct PMT{
  
  PMT(int X, int Y, double PulsePeak) : x(X), y(Y), pulsePeak(PulsePeak) {}
  PMT(TString Name, double PulsePeak) : name(Name), pulsePeak(PulsePeak) {}
  
  TString name;
  
  int x;
  int y;
  double pulsePeak;
  double pulsePeakRMS;
  int pedestal;
  bool edge;
  int rocid;
  int slot;
  int channel;
};


/*

Goal: Use 4-fold symmetry to get the average at the boundaries
We need the boundary condition on 3 sides but only 2 are unique
The average of puls epeak and equidistant points is used but BE SURE to only include averages of 'good' pmts
Inside the boundary:
Look at nearest neighbor blocks and average all 8 around it

Use the formula:
(1+epsilon)P_i,j = Average pulse peak
where average pulse peak is either the correspoinding bases equidistant from the leds for blocks on the boundary
or the average of 8 neighboring blocks for all other blocks inside the boundary
Repeat the nearest neighbor algo with the same boundary conditions for each of the 4 quadrants

Output a text file with Row Column 1+epsilon where Row and Column come from (x,y) + 29

*/

// Run 1995 = 600 a.u
// Run 1445 = 150 a.u
// Run 1446 = 300 a.u

void gainCalibration_Plot2D(std::map< std::pair<int,int>, double > hist_map, TString title, TString save_file, TCanvas* c1 = 0x0){
  
  if (c1==NULL) c1 = new TCanvas("c1","c1",900,900);
  c1->cd();
  
  TH2F* hist2d = PlotUtilities::Plot2D(hist_map);
  hist2d->SetTitle(title);
  hist2d->Draw("colz");
  if (save_file!=""){
    c1->SaveAs(save_file);
  }
}

void gainCalibration_Plot1D(std::map< std::pair<int,int>, double > hist_map, double step_size, TString title, TString save_file, TCanvas* c2 = 0x0){
  
  if (c2==NULL) c2 = new TCanvas("c2","c2",900,900);
  c2->cd();
  
  std::map< std::pair<int,int>, double >::const_iterator iter;
  vector< double > hist_vals;
  vector< double > hist_weights;
  for (iter=hist_map.begin(); iter!=hist_map.end(); ++iter){
    hist_vals.push_back(iter->second);
    //cout << iter->second << endl;
    hist_weights.push_back(1);
  }
  Int_t nentries = hist_vals.size();
  Double_t* X_vals = &hist_vals[0];
  double max_val = TMath::MaxElement(nentries,X_vals);
  cout << max_val << endl;
  max_val += max_val*0.01;
  double min_val = TMath::MinElement(nentries,X_vals);
  min_val -= min_val*0.01;
  int bin = int((max_val - min_val) / step_size);
  
  TH1F* hist = new TH1F("hist",title,bin,min_val,max_val);
  hist->FillN(nentries,X_vals,&hist_weights[0]);
  hist->SetTitle(title);
  hist->Draw();
  if (save_file!=""){
    c2->SaveAs(save_file);
  }
}


std::map<TString, std::pair<int,int> > getXYFromDAQCoordinates(){
  
  std::map<TString, std::pair<int,int> > base_map;
  
  // crate/slot/channel
  string daq_loc;
  int abs_num;
  // x/y
  string det_loc;
  
  string line;
  std::istringstream lin;
  ifstream inFile("FCALUtilities/DAQCoordinate_Ordered.txt");
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
  //inFile.close();
  return base_map;
}

// map of boundary channels
std::map< std::pair<int,int>, double > getBoundaryChannels( vector< PMT > pmtList){
  
  std::map< std::pair<int,int>, double > boundaryChannels;
  
  int nentries = pmtList.size();
  for (int i=0; i<nentries; i++){
    PMT pmtChannel = pmtList[i];
    if ( !pmtChannel.edge ) continue;
    //if (pmtChannel.x==-29 && pmtChannel.y==3) cout << "Yo: " << pmtChannel.pulsePeak << endl;
    boundaryChannels[std::make_pair(pmtChannel.x,pmtChannel.y)] = pmtChannel.pulsePeak;
  }
  return boundaryChannels;
}

// map of non-boundary channels
std::map< std::pair<int,int>, double > getNonBoundaryChannels( vector< PMT > pmtList){
  
  std::map< std::pair<int,int>, double > nonboundaryChannels;
  
  int nentries = pmtList.size();
  for (int i=0; i<nentries; i++){
    PMT pmtChannel = pmtList[i];
    if ( pmtChannel.edge ) continue;
    nonboundaryChannels[std::make_pair(pmtChannel.x,pmtChannel.y)] = pmtChannel.pulsePeak;
  }
  return nonboundaryChannels;
}

std::vector< PMT > getPMTsFromROOTFile(TString filename){
  
  std::map<TString, std::pair<int,int> > getXY = getXYFromDAQCoordinates();
  
  std::vector< PMT > pmtList;
  std::vector< std::pair<int,int> > tmp_pulseList;
  
  TFile* f = new TFile(filename);
  TTree* tr = (TTree*)f->Get("Df250PulsePedestal");
  
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  uint32_t pedestal;
  uint32_t pulse_peak;
  
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("pedestal",&pedestal);
  tr->SetBranchAddress("pulse_peak",&pulse_peak);
  
  Int_t nsize = (Int_t) tr->GetEntries();
  
  std::map< std::pair<int,int>, double> hist2D_map;
  std::map< std::pair<int,int>, TH1F*> hist_map;
  
  for (int i=0; i<nsize; i++){
    
    tr->GetEntry(i);
    
    // Only include FCAL channels
    if (rocid<11 || rocid>22) continue;
    
    TString DAQChannel;
    char c_DAQChannel[50];
    sprintf(c_DAQChannel,"%i/%i/%i",rocid-11,slot,channel);
    DAQChannel = TString(c_DAQChannel);
    
    int x = 0, y = 0;
    if (getXY.count(DAQChannel)>0){
      x = getXY[DAQChannel].first;
      y = getXY[DAQChannel].second;
    }
    // Add x,y coordinate
    DAQChannel += " "+TString::Itoa(x,10)+","+TString::Itoa(y,10);
    
    // skip any channels without a PMT
    if (x==0 && y==0) continue;
    
    // Some channels don't have a pedestal value so pulse_peak-pedestal can give nonsensical answers
    if ( pulse_peak > 200){
      if ( !(hist_map.count(std::make_pair(x,y))>0) ){
        hist_map[std::make_pair(x,y)] = new TH1F(DAQChannel,DAQChannel,4096,0,4096);
      }
      // pedestal subtracted pulse peak
      hist_map[std::make_pair(x,y)]->Fill(pulse_peak-pedestal);
    }
  }
  
  std::map< std::pair<int,int>, TH1F*>::const_iterator iter;
  for (iter=hist_map.begin(); iter!=hist_map.end(); ++iter){
    TH1F* hist = iter->second;
    TString name = hist->GetTitle();
    vector<TString> parseName = StringUtilities::parseTString(name," ");
    TString DAQCoord = parseName[0];
    TString XYCoord  = parseName[1];
    vector<TString> parseDAQCoord = StringUtilities::parseTString(DAQCoord,"/");
    rocid   = parseDAQCoord[0].Atoi();
    slot    = parseDAQCoord[1].Atoi();
    channel = parseDAQCoord[2].Atoi();
    vector<TString> parseXYCoord = StringUtilities::parseTString(XYCoord,",");
    int x = parseXYCoord[0].Atoi();
    int y = parseXYCoord[1].Atoi();
    double mean = hist->GetMean();
    double rms  = hist->GetRMS();
    
    PMT pmtChannel(name,mean);
    pmtChannel.pulsePeakRMS = rms;
    pmtChannel.rocid   = rocid;
    pmtChannel.slot    = slot;
    pmtChannel.channel = channel;
    pmtChannel.x       = x;
    pmtChannel.y       = y;
    
    if (TMath::Sqrt(x*x + y*y)>28.5) {
      pmtChannel.edge = true;
    } 
    else if (TMath::Abs(x)==0 || TMath::Abs(y)==0){
      pmtChannel.edge = true;
    } 
    else if(TMath::Abs(x)<=2 && TMath::Abs(y)<=2){
      pmtChannel.edge = true;
    } else{
      pmtChannel.edge = false;
    }
    
    pmtList.push_back(pmtChannel);
    
    //cout << hist->GetTitle() << "\t" << mean << "\t" << rms << endl;
    //hist2D_map[iter->first] = mean/2000.0;
  }
  
  cout << "Entries in vector: " << pmtList.size() << endl;
  
  //TH2F* hist2d = PlotUtilities::Plot2D(hist2D_map);
  //hist2d->SetTitle("Channels with pulse peak > 200 counts above pedestal");
  //hist2d->SetTitle("Average pulse peak in Volts");
  //hist2d->SetStats(true);
  //TCanvas* c1 = new TCanvas("c1","c1",900,900);
  //c1->cd();
  //hist2d->Draw("colz");
  
  // Save to a text file
  fstream* outFile = new fstream();
  outFile->open(filename+".dat", std::fstream::out);
  for (int i=0; i<int(pmtList.size()); i++){
    PMT pmtChannel = pmtList[i];
    *outFile << pmtChannel.name << " " << pmtChannel.edge << " " << pmtChannel.pulsePeak << " " << pmtChannel.pulsePeakRMS << endl;
  }
  outFile->close();
  
  return pmtList;
}

std::vector< PMT > getPMTsFromFile(TString filename){
  
  std::vector< PMT > pmtList;
  
  ifstream filestream(filename.Data(), ifstream::in);
  if (!filestream) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return pmtList;
	}
	
	TString line;
	
	while (line.ReadLine(filestream)){
	  
    PMT pmtChannel("",0);
    TString name = "";
	  
    TObjArray* tokens = line.Tokenize(" ");
    for (int i=0; i<tokens->GetEntries(); i++){
      TString s_value = ((TObjString*) tokens->At(i))->GetString().Remove(TString::kBoth, ' ');
      
      // rocid/slot/channel
      if (i==0){
        name += s_value;
        vector<TString> parseDAQCoord = StringUtilities::parseTString(s_value,"/");
        pmtChannel.rocid   = parseDAQCoord[0].Atoi();
        pmtChannel.slot    = parseDAQCoord[1].Atoi();
        pmtChannel.channel = parseDAQCoord[2].Atoi();
      }
      
      // x,y
      if (i==1){
        name += " " + s_value;
        vector<TString> parseXYCoord = StringUtilities::parseTString(s_value,",");
        pmtChannel.x = parseXYCoord[0].Atoi();
        pmtChannel.y = parseXYCoord[1].Atoi();
      }
      
      // edge
      if (i==2){
        if (s_value=="1") {pmtChannel.edge = true;}
        else {pmtChannel.edge = false;}
      }
      
      // pulse peak average over run
      if (i==3){
        pmtChannel.pulsePeak = s_value.Atof();
      }
      
      // pulse peak rms
      if (i==4){
        pmtChannel.pulsePeakRMS = s_value.Atof();
      }
    }
    pmtChannel.name = name;
    pmtList.push_back(pmtChannel);
  }
  
  cout << "Size: " << pmtList.size() << endl;
  
  return pmtList;
	
}

// Do iterative calculation of gain by using the average of the 8 neighboring blocks
// 1+epsilon = Average of 8 neighbors / pulse peak
std::map< std::pair<int,int>, double > gainCalibration_NonBoundaryChannels(std::map< std::pair<int,int>, double > FourFoldAvg_BoundaryChannels, std::map< std::pair<int,int>, double > nonBoundaryChannels, std::map< std::pair<int,int>, double > gainConstants){
  // nonBoundaryChannels
  //FourFoldAvg_BoundaryChannels
  std::map< std::pair<int,int>, double >::const_iterator iter;
  std::map< std::pair<int,int>, double > newGainConstants;
  for (iter=nonBoundaryChannels.begin(); iter!=nonBoundaryChannels.end(); ++iter){
    int x = iter->first.first;
    int y = iter->first.second;
    
    // Average the 8 neighboring channels
    double avgPulsePeak = 0;
    double counter = 0;
    for (int i=-1; i<=1; i++){
      for (int j=-1; j<=1; j++){
        if (i==0 && j==0) continue;
        int new_x = x+i;
        int new_y = y+j;
        bool neighborOnBoundary = FourFoldAvg_BoundaryChannels.count( std::make_pair(TMath::Abs(new_x),TMath::Abs(new_y)) )>0 ? true : false;
        if (neighborOnBoundary){
          avgPulsePeak += FourFoldAvg_BoundaryChannels[std::make_pair(TMath::Abs(new_x),TMath::Abs(new_y))];
          counter++;
          //cout << " Boundary ";
        }
        if ( nonBoundaryChannels.count(std::make_pair(new_x,new_y))>0 ){
          avgPulsePeak += nonBoundaryChannels[std::make_pair(new_x,new_y)]*gainConstants[std::make_pair(new_x,new_y)];
          counter++;
          //cout << " NonBoundary ";
        }
      } // loop through up-down
    } // loop through left-right
    avgPulsePeak *= 1.0 / counter;
    
    
    newGainConstants[iter->first] = avgPulsePeak / iter->second; 
    
    if (avgPulsePeak / iter->second > 10){
      cout << "Current x,y: " << x << "," << y << "\tAvg: " << avgPulsePeak << "\tGain: " << avgPulsePeak / iter->second << "\tPulsePeak: " << iter->second << endl;
    }
    
  }
  return newGainConstants;
}

std::map< std::pair<int,int>, double > gainCalibration_NonBoundaryChannels(std::map< std::pair<int,int>, double > FourFoldAvg_BoundaryChannels, std::map< std::pair<int,int>, double > nonBoundaryChannels){
  
  std::map< std::pair<int,int>, double > gainConstants;
  for (std::map< std::pair<int,int>, double >::const_iterator iter=nonBoundaryChannels.begin(); iter!=nonBoundaryChannels.end(); ++iter){
    gainConstants[iter->first] = 1;
  }
  
  return gainCalibration_NonBoundaryChannels(FourFoldAvg_BoundaryChannels, nonBoundaryChannels, gainConstants);
}

void gainCalibration(TString filename, int niterations, TString dir=""){
  
  if (dir!="" && !dir.EndsWith("/")) dir += "/";
  
  std::vector< PMT > pmtList = getPMTsFromFile(filename);
  std::map< std::pair<int,int>, double > nonBoundaryChannels = getNonBoundaryChannels(pmtList);
  std::map< std::pair<int,int>, double > BoundaryChannels = getBoundaryChannels(pmtList);
  
  cout << "Number of non-boundary channels: " << nonBoundaryChannels.size() << endl;
  cout << "Number of boundary channels: " << BoundaryChannels.size() << endl;
  
  std::map< std::pair<int,int>, double >::const_iterator iter;
  
  std::map< std::pair<int,int>, double > FourFoldAvg_BoundaryChannels;
  std::map< std::pair<int,int>, double > tmpFourFoldAvg_BoundaryChannels;
  std::map< std::pair<int,int>, int > FourFoldAvg_Counter;
  // Get the 4-fold average for boundary channels
  for (iter=BoundaryChannels.begin(); iter!=BoundaryChannels.end(); ++iter){
    int x = iter->first.first;
    int y = iter->first.second;
    FourFoldAvg_BoundaryChannels[std::make_pair( TMath::Abs(x),TMath::Abs(y) )] += iter->second;
    FourFoldAvg_Counter[std::make_pair( TMath::Abs(x),TMath::Abs(y) )]++;
  }
  for (iter=FourFoldAvg_BoundaryChannels.begin(); iter!=FourFoldAvg_BoundaryChannels.end(); ++iter){
    int avg = iter->second / double(FourFoldAvg_Counter[iter->first]);
    tmpFourFoldAvg_BoundaryChannels[iter->first] = avg;
  }
  FourFoldAvg_BoundaryChannels.clear();
  FourFoldAvg_Counter.clear();
  FourFoldAvg_BoundaryChannels = tmpFourFoldAvg_BoundaryChannels;
  tmpFourFoldAvg_BoundaryChannels.clear();
  
  // Calculate 1+epsilon for the boundary channels
  std::map< std::pair<int,int>, double > boundary_gains;
  for (iter=BoundaryChannels.begin(); iter!=BoundaryChannels.end(); ++iter){
    int x = iter->first.first;
    int y = iter->first.second;
    double gain = FourFoldAvg_BoundaryChannels[std::make_pair(TMath::Abs(x),TMath::Abs(y))] / iter->second;
    boundary_gains[iter->first] = gain;
  }
  
  
  // Iterate the gains
  std::map< std::pair<int,int>, double > gainConstants;
  for (int i=0; i<niterations; i++){
    if (i==0) gainConstants = gainCalibration_NonBoundaryChannels(FourFoldAvg_BoundaryChannels,nonBoundaryChannels);
    else{
      gainConstants = gainCalibration_NonBoundaryChannels(FourFoldAvg_BoundaryChannels,nonBoundaryChannels, gainConstants);
    }
    // Plot gains for non-boundary channels
    if (dir!="")  gainCalibration_Plot1D(gainConstants, 0.1, "Gains For Channels Not Along Boundary, iteration "+TString::Itoa(i,10), dir+"GainsNotAlongBoundary_1DHist_Iter"+TString::Itoa(i,10)+".jpg");
    if (dir!="")  gainCalibration_Plot2D(gainConstants, "Gains For Channels Not Along Boundary, iteration "+TString::Itoa(i,10), dir+"GainsNotAlongBoundary_2DHist_Iter"+TString::Itoa(i,10)+".jpg");
  }
  
  std::map< std::pair<int,int>, double > all_gains;
  
  // Output Gains to file
  fstream* outFile = new fstream();
  outFile->open(dir+"Gains.txt", std::fstream::out);
  for (iter=gainConstants.begin(); iter!=gainConstants.end(); ++iter){
    all_gains[iter->first] = iter->second;
    *outFile << iter->first.first << " " << iter->first.second << " " << iter->second << endl;
  }
  for (iter=boundary_gains.begin(); iter!=boundary_gains.end(); ++iter){
    all_gains[iter->first] = iter->second;
    *outFile << iter->first.first << " " << iter->first.second << " " << iter->second << endl;
  }
  outFile->close();
  if (dir!="") gainCalibration_Plot1D(all_gains, 0.1, "Gains For All Channels", dir+"GainsForAllChannels_1DHist.jpg");
  if (dir!="") gainCalibration_Plot2D(all_gains, "Gains For All Channels", dir+"GainsForAllChannels_2DHist.jpg");
  
  // ********************
  // Useful Plots To Make
  // ********************
  
  // 1+epsilon = 4-fold Avg / Pulse Peak
  if (dir!="") gainCalibration_Plot1D(boundary_gains, 0.1, "Gains For Channels Along Boundary", dir+"GainsAlongBoundary_1DHist.jpg");
  if (dir!="") gainCalibration_Plot2D(boundary_gains, "Gains Along Boundary", dir+"GainsAlongBoundary_2DHist.jpg");
  
  // 4-fold average
  if (dir!="") gainCalibration_Plot2D(FourFoldAvg_BoundaryChannels, "4-fold average", dir+"4FoldAverage.jpg");
  
  // Channels used in the boundary
  std::map< std::pair<int,int>, double > channelsUsed_map;
  for (iter=nonBoundaryChannels.begin(); iter!=nonBoundaryChannels.end(); ++iter){
    channelsUsed_map[iter->first] = 1;
  }
  for (iter=BoundaryChannels.begin(); iter!=BoundaryChannels.end(); ++iter){
    channelsUsed_map[iter->first] = 100;
  }
  if (dir!="") gainCalibration_Plot2D(channelsUsed_map, "Boundary and non-Boundary Channels", dir+"BoundaryNonBoundaryChannels_2DHist.jpg");
  
}

// Plot HV
// HV File: SetTo600mV.snap
// Gain File: Gains.txt
void gainCalibration_compareHV(TString hv_file, TString gains_file, TString dir = ""){
  
  ifstream hvInfile(hv_file.Data(), ifstream::in);
  if (!hvInfile) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return;
	}
	ifstream gainInfile(gains_file.Data(), ifstream::in);
  if (!gainInfile) {
		std::cout << "ERROR: Failed to open data file " << std::endl;
		return;
	}
	
  std::map< std::pair<int,int>, double > set_hv;
  std::map< std::pair<int,int>, double > gainCalibrated_hv;
	
	TString line;
  bool skip_line = true;
	
	while (line.ReadLine(hvInfile)){
	  
	  // FCAL:hv:7:-29:v0set 1 1643.94
    if ( line.Contains("FCAL:hv:") ) skip_line = false;
	  
    if (skip_line) continue;
    
    int x = 0, y = 0;
    double hv = 0;
	  
    TObjArray* tokens = line.Tokenize(" ");
    for (int i=0; i<tokens->GetEntries(); i++){
      TString s_value = ((TObjString*) tokens->At(i))->GetString().Remove(TString::kBoth, ' ');
      
      // XY coordinate
      if (i==0){
        vector<TString> parsePVName = StringUtilities::parseTString(s_value,":");
        x = parsePVName[2].Atoi();
        y = parsePVName[3].Atoi();
      }
      if (i==2) hv = s_value.Atof();
    }
    set_hv[std::make_pair(x,y)] = hv;
  }
  
  while (line.ReadLine(gainInfile)){
	  
    int x = 0, y = 0;
    double gain = 0, hv = 0;
	  
    TObjArray* tokens = line.Tokenize(" ");
    for (int i=0; i<tokens->GetEntries(); i++){
      TString s_value = ((TObjString*) tokens->At(i))->GetString().Remove(TString::kBoth, ' ');
      
      // X Y gain
      if (i==0) x = s_value.Atoi();
      if (i==1) y = s_value.Atoi();
      if (i==2) gain = s_value.Atof();
    }
    
    if ( set_hv.count(std::make_pair(x,y))>0 ){
      hv = set_hv[std::make_pair(x,y)];
    }
    
    double calib_hv = hv*gain;
    //if (calib_hv>1750) calib_hv = 1750;
    
    gainCalibrated_hv[std::make_pair(x,y)] = calib_hv;
  }
  
  // Plot hv
  if (dir!="") gainCalibration_Plot1D(set_hv, 1, "Set HV", dir+"SetHV_1DHist.jpg");
  if (dir!="") gainCalibration_Plot2D(set_hv, "Set HV", dir+"SetHV_2DHist.jpg");
  if (dir!="") gainCalibration_Plot1D(gainCalibrated_hv, 1, "Gain Calibrated HV", dir+"GainCalibratedHV_1DHist.jpg");
  if (dir!="") gainCalibration_Plot2D(gainCalibrated_hv, "Gain Calibrated HV", dir+"GainCalibratedHV_2DHist.jpg");
  
}

// *************
// Instructions
// *************
// Step 1: Write the average pulse peak from a ROOT file to a text file
// Execute: std::vector< PMT > getPMTsFromROOTFile(TString filename)
// Step 2: Calculate the gains for all channels using an iterative technique for non-boundary channels
// Execute: void gainCalibration(TString filename, int niterations, TString dir="")
// Step 3: Compare old HV with new HV based on gains
// Execute: void gainCalibration_compareHV(TString hv_file, TString gains_file, TString dir = "")

