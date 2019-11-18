#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
#include "TColor.h"


void plot_2d_roc(){
  // key = crate/slot/channel where crate starts at 0 and ends at 11
  // crate + 1 = rocfcal#
  std::map<TString, std::pair<int,int> > loc_map = GeneralUtilities::BaseLocationMap();
  std::map<TString, std::pair<int,int> >::const_iterator iter;
  std::map< std::pair<int,int>, double> detector_map;
  for (iter = loc_map.begin(); iter != loc_map.end(); ++iter){
    TString daq = iter->first;
    vector<TString> parse_daq = StringUtilities::parseTString(daq,"/");
    int roc = StringUtilities::TString2int(parse_daq[0])+1;
    double val = roc;
    //if (roc==1) val = 0.1;
    //else val = (roc-1)*1;
    detector_map[iter->second] = val;
  }
  
  // Add 100 for dark
  // Add 150 for bright
  Int_t palette[12];
  // red
  palette[0] = TColor::GetColor("#FFCEC2"); 
  palette[1] = 2; 
  palette[2] = TColor::GetColor("#8b0000"); 
  // green
  palette[3] = TColor::GetColor("#b6fcd5"); 
  palette[4] = 3; 
  palette[5] = TColor::GetColor("#49a178");
  // blue
  palette[6] = TColor::GetColor("#6897bb"); 
  palette[7] = 4; 
  palette[8] = TColor::GetColor("#002654"); 
  // yellow
  palette[9] = TColor::GetColor("#FFFFCC"); 
  palette[10] = 5; 
  palette[11] = TColor::GetColor("#eeb111");
  
  gStyle->SetPalette(12,palette);
  
  TH2F* hist_2d = PlotUtilities::Plot2D(detector_map, -30, 30, -30, 30, 0.5, 12.5);
  hist_2d->SetTitle("Location of daq crates");
  TCanvas* c1 = new TCanvas("c1","c1",1000,1000);
  c1->cd();
  hist_2d->Draw("colz");
}

void plot_2d_rings(TString ring = "Outer/Middle/Inner"){
  
  double rmin = 0;
  if (ring == "Outer")    rmin = 22;
  if (ring == "Middle")   rmin = 15;
  if (ring == "Inner")    rmin = 8;
  
  // key = crate/slot/channel where crate starts at 0 and ends at 11
  // crate + 1 = rocfcal#
  std::map<TString, std::pair<int,int> > loc_map = GeneralUtilities::BaseLocationMap();
  std::map<TString, std::pair<int,int> >::const_iterator iter;
  std::map< std::pair<int,int>, double> detector_map;
  for (iter = loc_map.begin(); iter != loc_map.end(); ++iter){
    int x = (iter->second).first;
    int y = (iter->second).second;
    double val = 1;
    if ( TMath::Power(TMath::Power(x,2.0)+TMath::Power(y,2.0),0.5) > rmin )  val = 10;
    //if (roc==1) val = 0.1;
    //else val = (roc-1)*1;
    detector_map[iter->second] = val;
  }
  TH2F* hist_2d = PlotUtilities::Plot2D(detector_map, -30, 30, -30, 30, 0, -1);
  hist_2d->SetTitle(ring+" Ring Turned On");
  TCanvas* c1 = new TCanvas("c1","c1",1000,1000);
  c1->cd();
  hist_2d->Draw("col");
}
