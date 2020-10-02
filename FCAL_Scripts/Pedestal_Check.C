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

#include <numeric>

struct sortByData_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.data > right.data;
    }
};

struct sortByNoise_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.overFlow > right.overFlow;
    }
};

struct sortByPedestal_DSC {
    bool operator()(const ADC_Channel &left, const ADC_Channel &right) {
        return left.pedestal > right.pedestal;
    }
};

// Get the pedestal on a channel basis, check for |pedestal-100| > 10
void Pedestal_Check_crates(TString filename, TString dir = "", double pulse_freq = 0){
  
  // Get all channels on the crate
  GeneralUtilities::CalibrateDAC = true;
  std::map<TString, TGraph*> pedestal = DAQUtilities::adcValues_TGraph(filename, 1, pulse_freq, "ped");
  
  vector<TH1F*> bad_peds;
  vector<double> mean;
  vector<double> rms;
  
  for (std::map<TString, TGraph*>::const_iterator iter=pedestal.begin(); iter!=pedestal.end(); ++iter){
    bool new_crate = false;
    TString name = iter->first; //Crate %i Slot %02i Channel %02i
    TGraph* gr = iter->second;
    TString title = gr->GetTitle();
    TH1F* hist = new TH1F("hist","Histogram of Pedestals",2000,0,200);
    hist->SetTitle(title);
    hist->SetXTitle("Pedestal (ADC Counts)");
    Int_t N = gr->GetN();
    Double_t* Ped = gr->GetY();
    for (int i=0; i<N; i++){
      hist->Fill(Ped[i]);
if(Ped[i] >130)  {
cout<<i+3<<"\t"<<Ped[i]<<endl;
}
    }
    // Fit with a gaussian
    hist->Fit("gaus","0");
    // Check for convergence
    TString status = gMinuit->fCstatu;
    if (!(status.Contains("CONVERGED"))) {
      cerr << "Not Converge:\t" << title << endl;
    }
    TF1* fit = hist->GetFunction("gaus");
    mean.push_back(fit->GetParameter(1));
    rms.push_back(fit->GetParameter(2));
    if (fabs(mean[mean.size()-1]-100)>10){
      bad_peds.push_back(hist);
    }
  }
  
  int nBad = bad_peds.size();
  cout << "There are " << nBad << " channels with pedestal outside +/- 10 count range of 100" << endl;
  TCanvas* c_peds[nBad];
  for (int i=0; i<nBad; i++){
    c_peds[i] = new TCanvas("c_ped_"+StringUtilities::int2TString(i),"c_ped_"+StringUtilities::int2TString(i),600,600);
    c_peds[i]->cd();
    bad_peds[i]->Draw();
  }
  



  double avgPed = TMath::Mean(mean.size(),&mean[0]);
  double avgRMS = TMath::Mean(rms.size(),&rms[0]);
  cout << "Average Pedestal For All Crates: " << avgPed << endl;
  cout << "Average RMS For All Crates: " << avgRMS << endl;
  
}

// Get the pedestal on a channel basis, check for |pedestal-100| > 10
void Pedestal_Check_pedestalFile(TString inputRootFile, TString outputTextFile, double pulse_freq = 0){
  
  // Get all channels on the crate
  //GeneralUtilities::CalibrateDAC = true;
  GeneralUtilities::CalibrateDAC = false;
  std::map<TString, TGraph*> pedestal = DAQUtilities::adcValues_TGraph(inputRootFile, 1, pulse_freq, "ped");
  cout << "Here" << endl;
  fstream* outFile = new fstream();
  string _fileName = StringUtilities::TString2string(outputTextFile);
  outFile->open(_fileName.c_str(), std::fstream::out);

  TH1F* hist_all = new TH1F("hist_all","hist_all",200,0,200);
  
  for (std::map<TString, TGraph*>::const_iterator iter=pedestal.begin(); iter!=pedestal.end(); ++iter){
    bool new_crate = false;
    TString name = iter->first; //Crate %i Slot %02i Channel %02i
    TGraph* gr = iter->second;
    TString title = gr->GetTitle();
    TH1F* hist = new TH1F("hist","Histogram of Pedestals",2000,0,200);
    hist->SetTitle(title);
    hist->SetXTitle("Pedestal (ADC Counts)");
    Int_t N = gr->GetN();
    Double_t* Ped = gr->GetY();
    for (int i=0; i<N; i++){
      hist->Fill(Ped[i]);
    }
    // Fit with a gaussian
    hist->Fit("gaus","0");
    // Check for convergence
    TString status = gMinuit->fCstatu;
    if (!(status.Contains("CONVERGED"))) {
      cerr << "Not Converge:\t" << title << endl;
    }
    TF1* fit = hist->GetFunction("gaus");
    double mean = fit->GetParameter(1);
    double rms  = fit->GetParameter(2);
    hist_all->Fill(mean);
    *outFile << title << "\t" << mean << endl;
  } 
  TCanvas* c_all = new TCanvas("c_all","c_all",600,600);
  c_all->cd();
  hist_all->Draw();
}

// Plot of Average Pedestal over all channels with RMS pedestal as error vs Event or Time
void Pedestal_Check_stability(TString filename, int everyN = 1){
  
  TGraphErrors* gr_avg = new TGraphErrors();
  gr_avg->SetName("AveragePedestal");
  gr_avg->SetTitle("Average Pedstal vs Event");
  
  // Get all channels on the crate
  GeneralUtilities::CalibrateDAC = true;
  TString sortby = "event";
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(filename, sortby);
  int adcSize = adcList.size();
  
  vector<double> vector_pedestal;
  
  int g_counter = 0;
  
  for (int i=0; i<adcSize; i++){
    ADC_Channel MY_ADC = adcList[i];
    int MY_EVENT        = MY_ADC.event;
    double MY_PEDESTAL  = MY_ADC.pedestal;
    
    bool doAvg = (MY_EVENT-DAQUtilities::first_event+1) % everyN == 0;
    if ( MY_EVENT==DAQUtilities::last_event) doAvg = true;
    
    bool set_point = false;
    
    if ( i+1!=adcSize ){
      ADC_Channel MY_NEXT_ADC = adcList[i+1];
      int MY_NEXT_EVENT       = MY_NEXT_ADC.event;
      
      // Check if the previous element is the same event as the current element
      // If they are then sum the energy
      if (MY_EVENT==MY_NEXT_EVENT){
        set_point = false;
      }
      
      // If Not then set a point in tgraph
      if (MY_EVENT!=MY_NEXT_EVENT){
        set_point = true;
      }
    }
    
    if (!set_point){
      //if (MY_PEDESTAL > 120) cout << "(" << MY_ADC.x << "," << MY_ADC.y << ")\tEvent = " << MY_EVENT << "\t" << MY_PEDESTAL << endl;
      //if (MY_ADC.x==-5 && MY_ADC.y==-3) cout << "(" << MY_ADC.x << "," << MY_ADC.y << ")\tEvent = " << MY_EVENT << "\t" << MY_PEDESTAL << endl;
      vector_pedestal.push_back(MY_PEDESTAL);

    }
    
    if ( i+1==adcSize ){
      set_point = true;
    }
    
    if (set_point){
      double mean = TMath::Mean(vector_pedestal.size(), &vector_pedestal[0]);
      double rms  = TMath::RMS(vector_pedestal.size(), &vector_pedestal[0]);
//cout<<mean<<"\t"<<rms<<endl;

      gr_avg->SetPoint(g_counter,g_counter+3,mean);
      gr_avg->SetPointError(g_counter,0,rms);
      g_counter++;
    }
    
  }
  
  TCanvas* c1 = new TCanvas("c1","c1",600,600);
  c1->cd();
  gr_avg->Draw("A");  
  gr_avg->GetXaxis()->SetTitle("Event");
  gr_avg->Draw("AP");

TCanvas* c2 = new TCanvas("c2","c2",600,600);
  c2->cd();
TH1F *h1 = new TH1F("h1","Pull",100,100,-100);
for (int i=0; i<vector_pedestal.size(); i++){
double sum = std::accumulate(vector_pedestal.begin(), vector_pedestal.end(), 0.0);
double ped_mean = sum / vector_pedestal.size();
double ped_rms  = TMath::RMS(vector_pedestal.size(), &vector_pedestal[0]);
//cout<<ped_mean<<"\t"<<ped_rms<<endl;
		
		h1->Fill((vector_pedestal[i]-ped_mean)/ped_rms);
		}
h1->Draw();

}

// Plot the pedestal and RMS for all channels
void Pedestal_Check_oscillations(TString filename, double cutOffRMS = 5, double devMean = 5, TString sortBy = "rms/noise", bool plot = false, bool manny_plugin = false){
  // Get all channels on the crate
  GeneralUtilities::CalibrateDAC = true;
  // static std::map<TString, TGraph*> adcValues_TGraph(TString fileName, int everyN, double pulser_freq, TString adc = "ped, max, charge, timing", bool compressed_mode = false, int threshold=100, int NSB=1, int NSA=1, bool manny_plugin=false);
  std::map<TString, TGraph*> pedestal = DAQUtilities::adcValues_TGraph(filename, 1, 0, "ped", false, 100, 1, 1, manny_plugin);
  
  //vector<TH1F*> bad_peds;
  vector<ADC_Channel> bad_peds;
  vector<double> mean;
  vector<double> rms;
  
  std::map< std::pair<int,int>, double> map_bad;
  
  TH1F* hist_ped = new TH1F("hist_ped","hist_ped",4096,0,4096);
  TH1F* hist_rms = new TH1F("hist_rms","hist_rms",100,0,10);
  
  for (std::map<TString, TGraph*>::const_iterator iter=pedestal.begin(); iter!=pedestal.end(); ++iter){
    TString name = iter->first; //Crate %i Slot %02i Channel %02i
    TGraph* gr = iter->second;
    TString title = gr->GetTitle();
    
    Int_t N = gr->GetN();
    Double_t* Ped = gr->GetY();
    Double_t* Events = gr->GetX();
    
    for (int i=0; i<N; i++){
      if (Ped[i]-100 > 30){
        //cout << "Large Pedestal: " << title << "\tEvent = " << Events[i] << endl;
      }
    }
    
    double _mean = TMath::Mean(N,Ped);
    double _rms = TMath::RMS(N,Ped);
    
    
    int count_noise = 0;
    
    for (int i=0; i<N; i++){
      if (fabs(Ped[i]-_mean) > devMean){
        count_noise++;
      }
      if (fabs(Ped[i]-_mean) > 3000){
        count_noise+=1000;
        cout << "Large Deviation: " << title << "\tEvent = " << Events[i] << endl;
      }
    }
    
    //cout << "Noise = " << count_noise << endl;
    
    hist_ped->Fill(_mean);
    hist_rms->Fill(_rms);
    
    mean.push_back(_mean);
    rms.push_back(_rms);
    
    if (_rms>cutOffRMS){
      ADC_Channel my_channel;
      // the graph title is in the follwoing format
      // Pedestal: MY_CRATE/MY_SLOT/MY_CHANNEL (MY_X,MY_Y)
      vector<TString> parseTString = StringUtilities::parseTString(title," ");
      TString crateString = parseTString[1];
      vector<TString> parseCrate  = StringUtilities::parseTString(crateString,"/");
      my_channel.crate   = parseCrate[0].Atoi();
      my_channel.slot    = parseCrate[1].Atoi();
      my_channel.channel = parseCrate[2].Atoi();
      
      TString coordString = parseTString[2];
      coordString = coordString.Remove(0,1);
      coordString = coordString.Remove(coordString.Length()-1,1);
      vector<TString> parseCoord = StringUtilities::parseTString(coordString,",");
      my_channel.x  = parseCoord[0].Atoi();
      my_channel.y  = parseCoord[1].Atoi();
      
      my_channel.data = _rms;
      my_channel.pedestal = _mean;
      
      my_channel.pulse = gr;
      
      // This counts the number of events outside some specified range given by
      // devMean
      my_channel.overFlow = count_noise;
      
      bad_peds.push_back(my_channel);
      
      map_bad[std::make_pair(my_channel.x,my_channel.y)] = _rms;
      
    }
  }
  
  int max_ped = TMath::MaxElement(mean.size(),&mean[0]);
  int max_rms = TMath::MaxElement(rms.size(),&rms[0]);
  
  // Set Range
  hist_ped->GetXaxis()->SetRangeUser(0,max_ped);
  hist_rms->GetXaxis()->SetRangeUser(0,max_rms);
  
  TCanvas* c_ped = new TCanvas("c_ped","c_ped",1000,1000);
  c_ped->Divide(1,2);
  c_ped->cd(1);
  hist_ped->Draw();
  c_ped->cd(2);
  hist_rms->Draw();
  
  // Sort by rms
  if (sortBy == "rms")
    std::sort(bad_peds.begin(),bad_peds.end(),sortByData_DSC());
  else if (sortBy == "noise")
    std::sort(bad_peds.begin(),bad_peds.end(),sortByNoise_DSC());
  else 
    cout << "Not Sorting!!" << endl;
  
  int nGraphs = bad_peds.size();
  cout << "There are " << nGraphs << " channels RMS > "<<cutOffRMS << endl;
  
  int nCanvas = nGraphs/16;
  if (nGraphs%16!=0) nCanvas++;
  
  TCanvas* c_gr[nCanvas];
  
  int cnum = 0;
  int pnum = 1;
  
  for (int i=0; i<nGraphs; i++){
    
    cout << bad_peds[i].crate << "/" << bad_peds[i].slot << "/" << bad_peds[i].channel << "\t(" << bad_peds[i].x << "," << bad_peds[i].y << ")\tMean = " << bad_peds[i].pedestal << "\tRMS = " << bad_peds[i].data << "\tNoise = " << bad_peds[i].overFlow << endl;
    
    if (pnum>16) {pnum=1; cnum++;}
    //cout << "cnum = " << cnum << "\tpnum = " << pnum << endl;
    if (pnum==1 && plot && cnum<20){
      char c_title[50];
      sprintf(c_title,"Canvas %i",cnum+1);
      c_gr[cnum] = new TCanvas(c_title,c_title,1000,1000);
      c_gr[cnum]->Divide(4,4);
    }
    
    if (plot && cnum<20) {
      c_gr[cnum]->cd(pnum);
      bad_peds[i].pulse->Draw("AP");
     }
    pnum++;
  }
  
  TCanvas* c_2d = new TCanvas("c_2d","c_2d",1000,1000);
  c_2d->cd();
  TH2F* hist2d = PlotUtilities::Plot2D(map_bad);
  hist2d->SetTitle("Large RMS Channels");
  hist2d->Draw("colz");
   cout << "There are " << nGraphs << " channels RMS > "<<cutOffRMS << endl;
}


TGraph* Pedestal_Check_single_channel(vector<uint32_t> waveform, int crate, int slot, int channel, int x, int y, int event){

  char single_title[50];
  char avg_title[50];
  TGraph* gr     = new TGraph(); 
  char gr_name[50];
  sprintf(gr_name,"Event %i %i/%i/%i (%i,%i)", event,crate,slot,channel,x,y);
  gr->SetTitle(gr_name);
  gr->SetName("graph");

  int nSamples = waveform.size();
    
  for (int ii=0; ii<nSamples; ii++){
    gr->SetPoint(ii,ii+1,waveform[ii]);
  }
  return gr;
}

// Plot the pedestal and RMS for all channels
void Pedestal_Check_largePedestals(TString fileName, int crate, int slot, int channel){
  DAQUtilities::get_waveform = true;
  GeneralUtilities::RemoveOverflows = false;
  std::vector< ADC_Channel > adcList = DAQUtilities::main_loop(fileName, "channelnum");
  int nList = adcList.size();
  
  std::vector< ADC_Channel > pedList;
  
  for (int i=0; i<nList; i++){
    if (crate!=adcList[i].crate || slot!=adcList[i].slot || channel!=adcList[i].channel) continue;
    
    pedList.push_back(adcList[i]);
    
  }
  
  // Sort by pedestal
  int nPed = pedList.size();
  if (nPed>16) nPed = 16;
  std::sort(pedList.begin(),pedList.end(),sortByPedestal_DSC());
  
  TCanvas* c_ped = new TCanvas("c_ped","c_ped",1000,1000);
  c_ped->Divide(4,4);
  for (int i=0; i<nPed; i++){
    c_ped->cd(i+1);
    Pedestal_Check_single_channel(pedList[i].waveform, pedList[i].crate, pedList[i].slot, pedList[i].channel, pedList[i].x, pedList[i].y, pedList[i].event)->Draw("AP");
  }
  GeneralUtilities::RemoveOverflows = true;
}


