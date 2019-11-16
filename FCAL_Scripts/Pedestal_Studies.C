/*
#include "FCALUtilities/StringUtilities.h"
#include "FCALUtilities/GeneralUtilities.h"
#include "FCALUtilities/PlotUtilities.h"
#include "FCALUtilities/DAQUtilities.h"
#include "FCALUtilities/MonitoringUtilities.h"
*/
#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
#include "TSystem.h"

// get a vector of the 1st 4 sample ADC values for each channel and event
// Fill a histogram for each channel of these values
// After some number of events fit for the Mean and Width
// Plot Mean value vs event with errors for the width
// Project TGraph on y-axis and get mean
// Make a histogram of the Mean[0] - Mean[i]. The spread of this is a good indicator of the pedestal stability
// Make a histogram over all channels of the spread of Mean[0]-Mean[i] and the mean
// Make a histogram over all channels of the Mean

// In total the final plots will be:
// 1) Histogram of Mean
// 2) Histogram of the Spread of Mean[0]-Mean[i] for all channels
// 3) Histogram of the Mean of Mean[0]-Mean[i] for all channels
// 4) Rank the TGraph by Spread of Mean[0]-Mean[i] and plot the 1st 12



struct sortBySpreadDiff_DSC {
    bool operator()(const Pedestal_Study &left, const Pedestal_Study &right) {
        return left.spread_diff > right.spread_diff;
    }
};

vector< Pedestal_Study > Pedestal_Studies_List(TString fileName, int nEvtsAvg=100){
  vector< Pedestal_Study > pedStudyList;
  
  // Sort by crate slot channel and event
  std::vector< ADC_Channel > pedList = DAQUtilities::manny_plugin_loop(fileName, "absnum");
  int nList = pedList.size();
  
  // Initialize ROOT Containers
  TGraphErrors* grE_mean_vs_event = 0x0;
  
  TH1F* h_ped = 0x0;
  vector<TH1F*> h_ped_vec;
  
  int evt_counter = 0;
  
  for (int i=0; i<nList; i++){
    ADC_Channel ped = pedList[i];
    
    char name[50];
    sprintf(name, "%i/%i/%i (%i,%i)", ped.crate, ped.slot, ped.channel, ped.x, ped.y);
    TString Tname = TString(name);
    
    if (grE_mean_vs_event==0x0){
      grE_mean_vs_event = new TGraphErrors();
      grE_mean_vs_event->SetName("grE_mean_vs_event "+Tname);
      grE_mean_vs_event->SetTitle("Ped Vs Event "+Tname);
    }
    if (h_ped==0x0){
      h_ped = new TH1F("h_ped "+Tname+" Event = "+StringUtilities::int2TString(ped.event),"Pedestals: "+Tname+" Event = "+StringUtilities::int2TString(ped.event),4096,0,4096);
    }
    
    vector<uint32_t> ped_vec = ped.waveform;
    int nPed = ped_vec.size();
    for (int j=0; j<nPed; j++){
      h_ped->Fill(ped_vec[j]);
    }
    
    evt_counter++;
    bool do_avg = evt_counter%nEvtsAvg==0 ? true : false;
    
    bool end_of_channel = false;
    if (i+1!=nList){
      if (ped.crate!=pedList[i+1].crate || ped.slot!=pedList[i+1].slot || ped.channel!=pedList[i+1].channel){
        end_of_channel = true;
      }
    } else {
      end_of_channel = true;
    }
    
    if (end_of_channel || do_avg){
      h_ped->GetXaxis()->SetRangeUser(50,150);
      h_ped->Fit("gaus","0Q");
      TF1* fit = h_ped->GetFunction("gaus");
      double fit_mean  = fit->GetParameter(1);
      double fit_width = fit->GetParameter(2);
      TString status = gMinuit->fCstatu;
      if (!(status.Contains("CONVERGED"))) {
        fit_mean  = h_ped->GetMean();
        fit_width = h_ped->GetRMS();
      }
      h_ped_vec.push_back(h_ped);
      cout << Tname << "\t" << fit_mean << endl;
      h_ped = 0x0;
      evt_counter = 0;
      int point = grE_mean_vs_event->GetN();
      grE_mean_vs_event->SetPoint(point, ped.event, fit_mean);
      grE_mean_vs_event->SetPointError(point, 0, fit_width);
      
      if (end_of_channel){
        
        int nPoints = grE_mean_vs_event->GetN();
        Double_t* grE_mean_vs_eventY = grE_mean_vs_event->GetY();
        Double_t* grE_mean_vs_eventX = grE_mean_vs_event->GetX();
        double mean_min = TMath::MinElement(nPoints,grE_mean_vs_eventY);
        double mean_max = TMath::MaxElement(nPoints,grE_mean_vs_eventY);
        mean_min = mean_min-10>0 ? mean_min-10 : 0;
        mean_max = mean_max>1000 ? 300 : mean_max+10;
        
        
        TGraph* gr_mean_diff = new TGraph();
        gr_mean_diff->SetName("gr_mean_diff "+Tname);
        gr_mean_diff->SetTitle("Mean[0] - Mean[i]: "+Tname);
        
        TH1F* h_mean = new TH1F("h_mean "+Tname, "Pedestal Mean: "+Tname,400,80,120);
        TH1F* h_mean_diff = new TH1F("h_spread "+Tname, "Mean[0]-Mean[i]: "+Tname,100,-5,5);
        
        for (int j=0; j<nPoints; j++){
          h_mean_diff->Fill(grE_mean_vs_eventY[0] - grE_mean_vs_eventY[j]);
          gr_mean_diff->SetPoint(gr_mean_diff->GetN(), grE_mean_vs_eventX[j], grE_mean_vs_eventY[0] - grE_mean_vs_eventY[j]);
          h_mean->Fill(grE_mean_vs_eventY[j]);
        }
        
        Double_t* gr_mean_diffY = gr_mean_diff->GetY();
        int nDiff = gr_mean_diff->GetN();
        double mean_diff_min = TMath::MinElement(nDiff,gr_mean_diffY);
        double mean_diff_max = TMath::MaxElement(nDiff,gr_mean_diffY);
        mean_diff_min = mean_diff_min-10>0 ? mean_diff_min-10 : 0;
        mean_diff_max = mean_diff_max>1000 ? 300 : mean_diff_max+10;
        
        Pedestal_Study pedStudy;
        pedStudy.crate             = ped.crate;
        pedStudy.slot              = ped.slot;
        pedStudy.channel           = ped.channel;
        pedStudy.x                 = ped.x;
        pedStudy.y                 = ped.y;
        pedStudy.name              = Tname;
        pedStudy.spread            = h_mean->GetRMS();
        pedStudy.mean              = h_mean->GetMean();
        pedStudy.mean_min          = int(mean_diff_min);
        pedStudy.mean_max          = int(mean_diff_max);
        pedStudy.spread_diff       = h_mean_diff->GetRMS();
        pedStudy.mean_diff         = h_mean_diff->GetMean();
        pedStudy.mean_diff_min     = int(mean_diff_min);
        pedStudy.mean_diff_max     = int(mean_diff_max);
        pedStudy.grE_mean_vs_event = grE_mean_vs_event;
        pedStudy.gr_mean_diff      = gr_mean_diff;
        pedStudy.h_mean            = h_mean;
        pedStudy.h_mean_diff       = h_mean_diff;
        pedStudy.h_ped_vec         = h_ped_vec;
        
        pedStudyList.push_back(pedStudy);
        
        grE_mean_vs_event = 0x0;
        h_ped_vec.clear();
        
      } // End of end_of_channel
      
    } // End of end_of_channel || doAvg
    
  } // End loop of ADC_Channel vector
  return pedStudyList;
}

void Pedestal_Studies(TString fileName, int nEvtsAvg=100){
  //vector< Pedestal_Study > pedList = Pedestal_Studies_List(fileName, nEvtsAvg);
  vector< Pedestal_Study > pedList = DAQUtilities::manny_plugin_pedestalStudy(fileName, nEvtsAvg);
  int nList = pedList.size();
  cout << "Number in vector: " << nList << endl;
  
  // Sort in order of the spread in Mean[0]-Mean[i]
  std::sort(pedList.begin(),pedList.end(),sortBySpreadDiff_DSC());
  
  
  
  // Plot the worst 16 TGraphs
  TCanvas* c_grE_mean_vs_event = new TCanvas("c_grE_mean_vs_event","c_grE_mean_vs_event",1000,1000);
  c_grE_mean_vs_event->Divide(4,4);
  
  TCanvas* c_gr_mean_diff = new TCanvas("c_gr_mean_diff","c_gr_mean_diff",1000,1000);
  c_gr_mean_diff->Divide(4,4);
  
  
  
  
  // Plot histograms of the Mean, Mean[0]-Mean[i], and the Spread of each
  TH1F* h_mean = new TH1F("h_mean","Pedestal Means", 400,80,120);
  TCanvas* c_h_mean = new TCanvas("c_h_mean","c_h_mean",1000,1000);
  
  TH1F* h_mean_diff = new TH1F("h_mean_diff","Pedestal Mean[0] - Mean[i]", 100,-50,50);
  TCanvas* c_h_mean_diff = new TCanvas("c_h_mean_diff","c_h_mean_diff",1000,1000);
  
  TH1F* h_spread = new TH1F("h_spread","Pedestal Spread in Mean", 500,0,50);
  TCanvas* c_h_spread = new TCanvas("c_h_spread","c_h_spread",1000,1000);
  
  TH1F* h_spread_diff = new TH1F("h_spread_diff","Pedestal Spread in Mean[0] - Mean[i]", 500,0,50);
  TCanvas* c_h_spread_diff = new TCanvas("c_h_spread_diff","c_h_spread_diff",1000,1000);
  
  
  for (int i=0; i<nList; i++){
    h_mean->Fill(pedList[i].mean);
    h_mean_diff->Fill(pedList[i].mean_diff);
    h_spread->Fill(pedList[i].spread);
    h_spread_diff->Fill(pedList[i].spread_diff);
    
    if (i<16){
      c_grE_mean_vs_event->cd(i+1);
      pedList[i].grE_mean_vs_event->Draw("AP");
      c_gr_mean_diff->cd(i+1);
      pedList[i].gr_mean_diff->Draw("AP");
    }
  }
  
  c_h_mean->cd();
  h_mean->Draw();
  c_h_mean_diff->cd();
  h_mean_diff->Draw();
  c_h_spread->cd();
  h_spread->Draw();
  c_h_spread_diff->cd();
  h_spread_diff->Draw();
  
}

void Pedestal_Studies_Check(TString fileName, int nEvtsAvg=100){
  //vector< Pedestal_Study > pedList = Pedestal_Studies_List(fileName, nEvtsAvg);
  vector< Pedestal_Study > pedList = DAQUtilities::manny_plugin_pedestalStudy(fileName, nEvtsAvg);
  cout << "Number in vector: " << pedList.size() << endl;
  
  TCanvas* c_grE_mean_vs_event = new TCanvas("c_grE_mean_vs_event","c_grE_mean_vs_event",1000,1000);
  c_grE_mean_vs_event->Divide(4,4);
  
  TCanvas* c_gr_mean_diff = new TCanvas("c_gr_mean_diff","c_gr_mean_diff",1000,1000);
  c_gr_mean_diff->Divide(4,4);
  
  TCanvas* c_h_mean = new TCanvas("c_h_mean","c_h_mean",1000,1000);
  c_h_mean->Divide(4,4);
  
  TCanvas* c_h_mean_diff = new TCanvas("c_h_mean_diff","c_h_mean_diff",1000,1000);
  c_h_mean_diff->Divide(4,4);
  
  
  for (int i=0; i<16; i++){
    cout << pedList[i].crate << "/" << pedList[i].slot << "/" << pedList[i].channel << endl;
    cout << pedList[i].spread << "\t" << pedList[i].mean << endl;
  
    c_grE_mean_vs_event->cd(i+1);
    pedList[i].grE_mean_vs_event->Draw("AP");
    c_gr_mean_diff->cd(i+1);
    pedList[i].gr_mean_diff->Draw("AP");
    c_h_mean->cd(i+1);
    cout << pedList[i].h_mean << endl;
    //TH1F* h_mean = pedList[i].h_mean;
    cout << "HName = " << pedList[i].h_mean->GetEntries() << "\t" << pedList[i].h_mean->GetName() << endl;
    c_h_mean_diff->cd(i+1);
    //TH1F* h_mean_diff = pedList[i].h_mean_diff;
    //cout << "HName = " << pedList[i].h_mean_diff->GetEntries() << "\t" << pedList[i].h_mean_diff->GetName() << endl;
    
  }
  /*
  TCanvas* c_h_ped = new TCanvas("c_h_ped","c_h_ped",1000,1000);
  c_h_ped->Divide(4,4);
  vector<TH1F*> h_ped_vec = pedList[0].h_ped_vec;
  int nPed = h_ped_vec.size()>16 ? 16 : h_ped_vec.size();
  for (int i=0; i<nPed; i++){
    c_h_ped->cd(i+1);
    h_ped_vec[i]->Draw();
  }
  */
}




