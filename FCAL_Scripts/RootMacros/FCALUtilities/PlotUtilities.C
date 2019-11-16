
#include "PlotUtilities.h"


// Some Plotting Utilities


bool PlotUtilities::getFullList = false;

vector<TString> PlotUtilities::bad_adcList;
void PlotUtilities::Clear_bad_adcList(){
  PlotUtilities::bad_adcList.clear();
}

// ******************
// Input Map MUST have a key in the following format
// Crate %i Slot %02i Channel %02i
// ******************
void PlotUtilities::PlotAllHist_OLD(TString fileName, std::map<TString,TH1F*> hist_map, double xmin, double xmax, bool plot, bool fit, TString dir, TString xtitle, TString ytitle){
  
  bool north = true;
  if (fileName.Contains("fcal_s6")) north = false;
  
  if (dir!=""){
    plot = true;
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  std::vector<TString> failedConvergence;
  
  for (int i=0; i<_crate; i++){
    for (int ii=0; ii<_slot; ii++){
      if (!GeneralUtilities::SlotExists_OLD(i, ii, north)) continue;
      TCanvas* c_hist = new TCanvas("c_hist","Histogram All Channels",600,600);
      c_hist->Divide(4,4);
      for (int iii=0; iii<_ch; iii++){
        char name[50];
        sprintf(name,"Crate %i Slot %02i Channel %02i",i,ii,iii);
        TString t_name = name;
        if (hist_map.count(t_name)){
          c_hist->cd(iii+1);
          TH1F* h_temp = hist_map.find(t_name)->second;
          if (fit) h_temp->Fit("gaus","0Q");
          h_temp->GetXaxis()->SetRangeUser(xmin,xmax);
          h_temp->SetXTitle(xtitle);
          h_temp->SetYTitle(ytitle);
          TString status = gMinuit->fCstatu;
          if (!(status.Contains("CONVERGED"))) {
            cerr << "Warning: " << h_temp->GetName() << "\n Didnot Converge" << endl;
            failedConvergence.push_back(h_temp->GetName());
          }
          TF1* _fit = 0x0;
          if (fit) _fit = h_temp->GetFunction("gaus");
          h_temp->Draw();
          if (fit) _fit->Draw("same");
          
        }
      } // End loop over channel
      if (plot){
        TString fname = GeneralUtilities::formatName(fileName);
        fname += "_Hist_Crate-"+StringUtilities::int2TString(i)+"_Slot-"+StringUtilities::int2TString(ii);
        c_hist->SaveAs(dir+fname+".eps");
      }
    } // End loop over slots
  } // End loop over crates
  
  if (fit){
    cerr << "There are " << failedConvergence.size() << " convergence problems" << endl;
    for (uint32_t i=0; i<failedConvergence.size(); i++){
      cerr << failedConvergence[i] << endl;
    }
  }
}



// ******************
// Input Map MUST have a key in the following format
// Crate %i Slot %02i Channel %02i
// ******************

void PlotUtilities::PlotAllGraphs_OLD(TString fileName, std::map<TString,TGraph*> gr_map, double xmin, double xmax, bool plot, TString dir, TString xtitle, TString ytitle){
  
  bool north = true;
  if (fileName.Contains("fcal_s6")) north = false;
  
  if (dir!=""){
    plot = true;
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  for (int i=0; i<_crate; i++){
    for (int ii=0; ii<_slot; ii++){
      if (!GeneralUtilities::SlotExists_OLD(i, ii, north)) continue;
      TCanvas* c_gr = new TCanvas("c_gr","Graph All Channels",600,600);
      c_gr->Divide(4,4);
      for (int iii=0; iii<_ch; iii++){
        char name[50];
        sprintf(name,"Crate %i Slot %02i Channel %02i",i,ii,iii);
        TString t_name = name;
        if (gr_map.count(t_name)){
          c_gr->cd(iii+1);
          TGraph* gr_temp = gr_map.find(t_name)->second;
          gr_temp->Draw("A");
          gr_temp->GetXaxis()->SetRangeUser(xmin,xmax);
          gr_temp->GetXaxis()->SetTitle(xtitle);
          gr_temp->GetYaxis()->SetTitle(ytitle);
          gr_temp->Draw("AP");
        }
      } // End loop over channel
      if (plot){
        TString fname = GeneralUtilities::formatName(fileName);
        fname += "_Graph_Crate-"+StringUtilities::int2TString(i)+"_Slot-"+StringUtilities::int2TString(ii);
        c_gr->SaveAs(dir+fname+".eps");
      }
    } // End loop over slots
  } // End loop over crates
}

void PlotUtilities::PlotAllGraphs(TString fileName, std::map<TString,TGraph*> gr_map, bool plot, TString dir, TString xtitle, TString ytitle, double xmin, double xmax, TString adc_type){
  
  bad_adcList.clear();
  
  vector<TString> bad_adcPos;
  
  int bins =0; double x_min=0; double x_max=0;
  TString title = "";
  if (adc_type=="charge") {title = "Integral RMS / Mean"; bins=100; x_min=0; x_max=1;}  
  if (adc_type=="ped") {title = "Pedestal: "+GeneralUtilities::formatName(fileName); bins=1000; x_min=50; x_max=150;}  
  if (adc_type=="CAT") {title = "HV RMS / Mean: "+GeneralUtilities::formatName(fileName); bins=100; x_min=0; x_max=1;}

  TH1F* hist = new TH1F("hist",title,bins,x_min,x_max);
  //vector<TString> bad_adc_list;  
 
  if (dir!=""){
    plot = true;
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  TCanvas* c_gr = new TCanvas("c_gr","Graph All Channels",1000,1000);
  
  std::map<TString,TGraph*>::const_iterator iter_prev;
  std::map<TString,TGraph*>::const_iterator iter_next;
  for (std::map<TString,TGraph*>::const_iterator iter=gr_map.begin(); iter!=gr_map.end(); ++iter){
    
    bool new_tcanvas = false;
    bool save_tcanvas = false;
    
    TString curName = iter->first;
    int curCrate = StringUtilities::TString2int( StringUtilities::parseTString(curName," ")[1] );
    int curSlot = StringUtilities::TString2int( StringUtilities::parseTString(curName," ")[3] );
    int curChannel = StringUtilities::TString2int( StringUtilities::parseTString(curName," ")[5] );
    
    if (iter!=gr_map.begin()){
      iter_prev = iter;
      --iter_prev;
      TString prevName = iter_prev->first;
      int prevSlot = StringUtilities::TString2int( StringUtilities::parseTString(prevName," ")[3] );
      if (curSlot!=prevSlot) new_tcanvas=true;
    }
    
    if (iter!=--gr_map.end()){
      iter_next = iter;
      ++iter_next;
      TString nextName = iter_next->first;
      int nextSlot = StringUtilities::TString2int( StringUtilities::parseTString(nextName," ")[3] );
      if (curSlot!=nextSlot) save_tcanvas=true;
    }
    
    if (iter==gr_map.begin()) new_tcanvas=true;
    if (iter != gr_map.end() && iter == --gr_map.end())   save_tcanvas=true;
    
    if (new_tcanvas){
      c_gr = new TCanvas("c_gr","Graph All Channels",1000,1000);
      c_gr->Divide(4,4);
    }
    
    c_gr->cd(curChannel+1);
    
    TGraph* gr_temp = iter->second;
    Int_t gr_N      = gr_temp->GetN();
    Double_t* gr_Y  = gr_temp->GetY();
    double max_y = TMath::MaxElement(gr_N,gr_Y);
    double min_y = TMath::MinElement(gr_N,gr_Y);
    double mean_y = TMath::Mean(gr_N,gr_Y);
    double rms_y  = TMath::RMS(gr_N,gr_Y);
    bool flag_pad = false;
    bool push_info = false;
    if (adc_type=="ped"){
      if ( fabs(max_y-100)>10) {flag_pad=true; cout << "Too High:\t" << curName << endl;}
      if ( fabs(min_y-100)>10) {flag_pad=true; cout << "Too Low:\t"  << curName << endl;}
      if (fabs(max_y-100)>10 || fabs(min_y-100)>10) {push_info = true;}
    }
    if (adc_type=="charge"){
      if ( rms_y/mean_y*100>2 ) {flag_pad=true; cout << "Too Wide:\t"  << curName << endl;}
      if ( fabs(min_y)<0.1) {flag_pad=true; cout << "Too Low:\t"  << curName << endl;}
      if (rms_y/mean_y*100>2 || fabs(min_y)<0.1)  {push_info = true;}
    }
    if (adc_type=="CAT"){
      if (min_y<800) {flag_pad=true; push_info = true;}
      //if (rms_y/mean_y*100>2){flag_pad=true; push_info = true;}
    }
    if (flag_pad) {
      //c_gr->SetBorderSize(6); c_gr->SetHighLightColor(kRed); 
      //TFrame* frame = c_gr->GetFrame(); frame->SetBorderSize(6); frame->SetFillColor(kRed);
      TVirtualPad* pad = c_gr->GetPad(curChannel+1); pad->SetFillColor(kRed-10);
    }
    if (push_info || getFullList){
      bad_adcList.push_back(curName); bad_adcPos.push_back(gr_temp->GetTitle());
    }
    
    if (adc_type=="charge") {hist->Fill(rms_y/mean_y);}
    if (adc_type=="ped") {hist->Fill(mean_y);}
    if (adc_type=="CAT"){hist->Fill(rms_y/mean_y);}

    if (plot){
      gr_temp->Draw("A");
      if (int(xmin*1e5)!=int(xmax*1e5)) gr_temp->GetXaxis()->SetRangeUser(xmin,xmax);
      gr_temp->GetXaxis()->SetTitle(xtitle);
      gr_temp->GetXaxis()->SetTitleSize(0.06);
      gr_temp->GetYaxis()->SetTitle(ytitle);
      gr_temp->GetYaxis()->SetTitleOffset(2);
      gr_temp->GetYaxis()->SetLabelSize(0.06);
      gr_temp->Draw("AP");
    }
    
    if (dir!="" && save_tcanvas){
      TString fname = GeneralUtilities::formatName(fileName);
      fname += "_"+adc_type+"Graph_Crate-"+StringUtilities::int2TString(curCrate)+"_Slot-"+StringUtilities::int2TString(curSlot);
      c_gr->SaveAs(dir+fname+".pdf");
    }
    
  }
  TCanvas* c_fracError = new TCanvas("c_fracError","c_fracError",800,800);
  c_fracError->cd();
  hist->Draw();
  if (adc_type=="ped") c_fracError->SaveAs(dir+GeneralUtilities::formatName(fileName)+"_Pedestal_Hist.eps");
  if (adc_type=="charge") c_fracError->SaveAs(dir+GeneralUtilities::formatName(fileName)+"_Charge_RMS_Hist.eps");
  if (adc_type=="CAT") c_fracError->SaveAs(dir+GeneralUtilities::formatName(fileName)+"_HV_RMS_Hist.eps");

  for (int i=0; i<bad_adcList.size(); i++) {cout << i+1 << "\t" << bad_adcList[i] << "\t" << bad_adcPos[i] << endl;}
  
  //if (adc_type=="TEM" || adc_type=="CUR") {bad_adcList.clear();}

}




void PlotUtilities::PlotSingleGraphs(TString fileName, std::map<TString,TGraph*> gr_map, double xmin, double xmax, bool plot, TString dir, TString xtitle, TString ytitle){
  
  bool north = true;
  if (fileName.Contains("fcal_s6")) north = false;
  
  if (dir!=""){
    plot = true;
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  for (std::map<TString,TGraph*>::const_iterator iter=gr_map.begin(); iter!=gr_map.end(); ++iter){
    TCanvas* c_gr = new TCanvas("c_gr","Graph Single Channels",600,600);
    c_gr->cd();
    TGraph* gr_temp = iter->second;
    gr_temp->Draw("A");
    gr_temp->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_temp->GetXaxis()->SetTitle(xtitle);
    gr_temp->GetYaxis()->SetTitle(ytitle);
    gr_temp->Draw("AP");
    
    if (plot){
      TString fname = GeneralUtilities::formatName(fileName);
      TString name = iter->first;
      fname += "_Graph_"+name;
      c_gr->SaveAs(dir+fname+".eps");
    }
  }
}




//*********************
// Below is a 4x4 canvas
// Each column is a channel
// Odd and Odd+1 rows are the same channel with a different variable being plotted
// In this way each canvas hold 8 unique channels each with 2 variables
//*********************
void PlotUtilities::Plot_8_UniqueChannelsPerCanvas(TString fileName, std::map<TString,TGraph*> gr_map1, std::map<TString,TGraph*> gr_map2, bool plot, TString dir){
  
  //vector<TString> bad_adcList;  
 
  if (dir!=""){
    plot = true;
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  
  TCanvas* c_gr = new TCanvas("c_gr","Graph All Channels",3000,2000);
  
  std::map<TString,TGraph*>::const_iterator iter_prev;
  std::map<TString,TGraph*>::const_iterator iter_next;
  for (std::map<TString,TGraph*>::const_iterator iter=gr_map1.begin(); iter!=gr_map1.end(); ++iter){
    
    bool new_tcanvas = false;
    bool save_tcanvas = false;
    
    TString curName = iter->first;
    int curCrate = StringUtilities::TString2int( StringUtilities::parseTString(curName," ")[1] );
    int curSlot = StringUtilities::TString2int( StringUtilities::parseTString(curName," ")[3] );
    int curChannel = StringUtilities::TString2int( StringUtilities::parseTString(curName," ")[5] );
    
    if (iter!=gr_map1.begin()){
      iter_prev = iter;
      --iter_prev;
      TString prevName = iter_prev->first;
      int prevSlot = StringUtilities::TString2int( StringUtilities::parseTString(prevName," ")[3] );
      if (curSlot!=prevSlot) new_tcanvas=true;
    }
    
    if (iter!=--gr_map1.end()){
      iter_next = iter;
      ++iter_next;
      TString nextName = iter_next->first;
      int nextSlot = StringUtilities::TString2int( StringUtilities::parseTString(nextName," ")[3] );
      if (curSlot!=nextSlot) save_tcanvas=true;
    }
    
    if (iter==gr_map1.begin()) new_tcanvas=true;
    if (iter != gr_map1.end() && iter == --gr_map1.end())   save_tcanvas=true;
    
    if (new_tcanvas){
      c_gr = new TCanvas("c_gr","Graph All Channels",3000,2000);
      c_gr->Divide(8,4);
    }
    
    
    // Go To Rows 1 and 3
    if (curChannel<8) c_gr->cd(curChannel+1); // Go To Row 1
    else c_gr->cd(curChannel+9); // Go To Row 3
    
    
    TGraph* gr_temp = iter->second;
    Int_t gr_N      = gr_temp->GetN();
    Double_t* gr_Y  = gr_temp->GetY();
    double max_y = TMath::MaxElement(gr_N,gr_Y);
    double min_y = TMath::MinElement(gr_N,gr_Y);
    double mean_y = TMath::Mean(gr_N,gr_Y);
    double rms_y  = TMath::RMS(gr_N,gr_Y);
    bool flag_pad = false;
    
    if ( rms_y/mean_y*100>2 ) {flag_pad=true; cout << "Too Wide:\t"  << curName << endl;}
    if ( fabs(min_y)<100) {flag_pad=true; cout << "Too Low:\t"  << curName << endl;}
    
    if (flag_pad) {
      //c_gr->SetBorderSize(6); c_gr->SetHighLightColor(kRed); 
      //TFrame* frame = c_gr->GetFrame(); frame->SetBorderSize(6); frame->SetFillColor(kRed);
      if (curChannel<8) {TVirtualPad* pad = c_gr->GetPad(curChannel+1); pad->SetFillColor(kRed-10);}
      else {TVirtualPad* pad = c_gr->GetPad(curChannel+9); pad->SetFillColor(kRed-10);}
      
    }
    

    if (plot){
      gr_temp->Draw("A");
      gr_temp->GetYaxis()->SetTitle("");
      gr_temp->GetXaxis()->SetTitle("");
      gr_temp->GetXaxis()->SetTitleSize(0.06);
      gr_temp->GetYaxis()->SetTitleOffset(2);
      gr_temp->GetYaxis()->SetLabelSize(0.06);
      gr_temp->Draw("AP");
    }
    
    // Go to Rows 2 and 4
    if (curChannel<8) c_gr->cd(curChannel+9); // Go To Row 2
    else c_gr->cd(curChannel+17); // Go To Row 4
    // Find by crate/slot/channel
    if (gr_map2.count(curName)>0){
      TGraph* gr_temp2 = gr_map2.find(curName)->second;
    
      if (plot){
        gr_temp2->Draw("A");
        gr_temp2->GetYaxis()->SetTitle("");
        gr_temp2->GetXaxis()->SetTitle("");
        gr_temp2->GetXaxis()->SetTitleSize(0.06);
        gr_temp2->GetYaxis()->SetTitleOffset(2);
        gr_temp2->GetYaxis()->SetLabelSize(0.06);
        gr_temp2->Draw("AP");
      }
    }
    
    if (dir!="" && save_tcanvas){
      TString fname = fileName;
      fname += "_CombinedGraph_Crate-"+StringUtilities::int2TString(curCrate)+"_Slot-"+StringUtilities::int2TString(curSlot);
      c_gr->SaveAs(dir+fname+".eps");
    }
    
  }
  
  for (int i=0; i<bad_adcList.size(); i++) cout << i+1 << "\t" << bad_adcList[i] << endl;

}


// Below will take in a list of crate slot channels one wants to plot
// The TCanvas will be divided into a 4x4 grid with
// Row 1: Charge vs Time
// Row 2: HV vs Time
// Row 3: Temperature vs Time
// Row 4: Current vs Time
void PlotUtilities::Plot_4_UniqueChannelsPerCanvas(TString fileName, std::vector<TString> adcList, std::map<TString,TGraph*> gr_map1, std::map<TString,TGraph*> gr_map2, std::map<TString,TGraph*> gr_map3, std::map<TString,TGraph*> gr_map4, TString dir, int n,  TString* replaced_array){
  
  TString xtitle = "Time (hours)";
    
  TGraph* gr_1 = 0x0;
  TGraph* gr_2 = 0x0;
  TGraph* gr_3 = 0x0;
  TGraph* gr_4 = 0x0;
  
  if (dir!=""){
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  int counter = 0;
  
  TCanvas* c_gr = new TCanvas("c_gr","Graph 4 Unique Channels Per Canvas",2000,2000);
  
  int adcSize = adcList.size();
  for (int i=0; i<adcSize; i++){
    
    TString adcName = adcList[i];
    cout << "ADC Name: " << adcName << endl;
    
    bool color_blue = false;
    for (int j=0; j<n; j++){
      if (replaced_array[j] == adcList[i]) {color_blue = true; break;}
    }
    
    bool new_tcanvas = false;
    bool save_tcanvas = false;
    
    if ( (i+1)%4==0 || (i+1)==adcSize) {save_tcanvas=true;}
    if ( (i+1)%4==1) {new_tcanvas=true;}
    
    if (new_tcanvas){
      c_gr = new TCanvas("c_gr","Graph Some Channels",2000,2000);
      c_gr->Divide(4,4);
    }
    
    // Go To Row 1
    c_gr->cd(i%4+1);
    if (color_blue){
      TVirtualPad* pad = c_gr->GetPad(i%4+1); pad->SetFillColor(kBlue-10);
    }
    if (gr_map1.count(adcName)>0){
      gr_1 = gr_map1.find(adcName)->second;
      gr_1->Draw("A");
      gr_1->GetYaxis()->SetTitle("");
      gr_1->GetXaxis()->SetTitle(xtitle);
      gr_1->GetXaxis()->SetTitleSize(0.07);
      gr_1->GetYaxis()->SetLabelSize(0.07);
      gr_1->Draw("AP");
    }
    
    // Go To Row 2
    c_gr->cd(i%4+5);
    if (gr_map2.count(adcName)>0){
      gr_2 = gr_map2.find(adcName)->second;
      gr_2->Draw("A");
      gr_2->GetYaxis()->SetTitle("");
      gr_2->GetXaxis()->SetTitle(xtitle);
      gr_2->GetXaxis()->SetTitleSize(0.07);
      gr_2->GetYaxis()->SetLabelSize(0.07);
      gr_2->Draw("AP");
    }
    
    // Go To Row 3
    c_gr->cd(i%4+9);
    if (gr_map3.count(adcName)>0){
      gr_3 = gr_map3.find(adcName)->second;
      gr_3->Draw("A");
      gr_3->GetYaxis()->SetTitle("");
      gr_3->GetXaxis()->SetTitle(xtitle);
      gr_3->GetXaxis()->SetTitleSize(0.07);
      gr_3->GetYaxis()->SetLabelSize(0.07);
      gr_3->Draw("AP");
    }
    
    // Go To Row 4
    c_gr->cd(i%4+13);
    if (gr_map4.count(adcName)>0){
      gr_4 = gr_map4.find(adcName)->second;
      gr_4->Draw("A");
      gr_4->GetYaxis()->SetTitle("");
      gr_4->GetXaxis()->SetTitle(xtitle);
      gr_4->GetXaxis()->SetTitleSize(0.07);
      gr_4->GetYaxis()->SetLabelSize(0.07);
      gr_4->Draw("AP");
    }
    
    if (dir!="" && save_tcanvas){
      counter++;
      TString fname = fileName;
      fname += "_"+StringUtilities::int2TString(counter);
      c_gr->SaveAs(dir+fname+".eps");
    }
  }
}

// Below will compare the same crate slot channels over multiple runs
// inputs is a map< "S6_X_N6_Y", map<"crate slot channel", tgraph> >
// The TCanvas will be divided into a # Runs x 4 grid with
// Row 1: Charge vs Time
// Row 2: HV vs Time
// Row 3: Temperature vs Time
// Row 4: Current vs Time
void 
PlotUtilities::Plot_MultiRuns_4Rows(TString fileName, std::vector<TString> adcList, \
std::map< TString, std::map<TString,TGraph*> > gr_map1, \
std::map< TString, std::map<TString,TGraph*> > gr_map2, \
std::map< TString, std::map<TString,TGraph*> > gr_map3, \
std::map< TString, std::map<TString,TGraph*> > gr_map4, TString dir)
{
  TString xtitle = "Time (hours)";
  
  TGraph* gr_1 = 0x0;
  TGraph* gr_2 = 0x0;
  TGraph* gr_3 = 0x0;
  TGraph* gr_4 = 0x0;
  
  if (dir!=""){
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  TCanvas* c_gr = new TCanvas("c_gr","Graph All Channels",2000,2000);
  
  int adcSize = adcList.size();
  for (int i=0; i<adcSize; i++){
    
    TString adcName = adcList[i];
    cout << "ADC Name: " << adcName << endl;
    
    std::map<TString, int> mapRun;
    std::map<TString, TGraph*> mapAllRuns;
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map1.begin(); iter != gr_map1.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      if (runNumber=="S6_49_N6_59" || runNumber=="S6_50_N6_60") {
        if (adcName==GeneralUtilities::formatTitle(4,18,9)) adcName=GeneralUtilities::formatTitle(4,18,8);
        if (adcName==GeneralUtilities::formatTitle(8,9,12)) adcName=GeneralUtilities::formatTitle(8,9,11);
        if (adcName==GeneralUtilities::formatTitle(6,8,11)) adcName=GeneralUtilities::formatTitle(6,8,10);
      } 
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("1_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map2.begin(); iter != gr_map2.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      if (runNumber=="S6_49_N6_59" || runNumber=="S6_50_N6_60") {
        if (adcName==GeneralUtilities::formatTitle(4,18,9)) adcName=GeneralUtilities::formatTitle(4,18,8);
        if (adcName==GeneralUtilities::formatTitle(8,9,12)) adcName=GeneralUtilities::formatTitle(8,9,11);
        if (adcName==GeneralUtilities::formatTitle(6,8,11)) adcName=GeneralUtilities::formatTitle(6,8,10);
      }
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("2_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map3.begin(); iter != gr_map3.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      if (runNumber=="S6_49_N6_59" || runNumber=="S6_50_N6_60") {
        if (adcName==GeneralUtilities::formatTitle(4,18,9)) adcName=GeneralUtilities::formatTitle(4,18,8);
        if (adcName==GeneralUtilities::formatTitle(8,9,12)) adcName=GeneralUtilities::formatTitle(8,9,11);
        if (adcName==GeneralUtilities::formatTitle(6,8,11)) adcName=GeneralUtilities::formatTitle(6,8,10);
      }
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("3_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map4.begin(); iter != gr_map4.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      if (runNumber=="S6_49_N6_59" || runNumber=="S6_50_N6_60") {
        if (adcName==GeneralUtilities::formatTitle(4,18,9)) adcName=GeneralUtilities::formatTitle(4,18,8);
        if (adcName==GeneralUtilities::formatTitle(8,9,12)) adcName=GeneralUtilities::formatTitle(8,9,11);
        if (adcName==GeneralUtilities::formatTitle(6,8,11)) adcName=GeneralUtilities::formatTitle(6,8,10);
      }
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("4_")] = mapADC.find(adcName)->second;
      }
    }
    
    int maxCol = mapRun.size();
    
    bool new_tcanvas = true;
    bool save_tcanvas = true;
    
    if (new_tcanvas){
      c_gr = new TCanvas("c_gr","Graph Some Channels",500*maxCol,2000);
      c_gr->Divide(maxCol,4);
    }
    
    int padCounter = 1;
    for (std::map<TString,int>::const_iterator it = mapRun.begin(); it != mapRun.end(); ++it){
      TString runNumber = it->first;
      // Row 1
      c_gr->cd(padCounter);
      TString row1 = "1_"+runNumber;
      if (mapAllRuns.count(row1)>0){
        gr_1 = mapAllRuns.find(row1)->second;
        TString title = gr_1->GetTitle();
        gr_1->SetTitle(runNumber + ": " + title);
        gr_1->Draw("A");
        gr_1->GetYaxis()->SetTitle("");
        gr_1->GetXaxis()->SetTitle(xtitle);
        gr_1->GetXaxis()->SetTitleSize(0.07);
        gr_1->GetYaxis()->SetLabelSize(0.07);
        gr_1->Draw("AP");
      }
      // Row 2
      c_gr->cd(padCounter+maxCol);
      TString row2 = "2_"+runNumber;
      if (mapAllRuns.count(row2)>0){
        gr_2 = mapAllRuns.find(row2)->second;
        gr_2->Draw("A");
        gr_2->GetYaxis()->SetTitle("");
        gr_2->GetXaxis()->SetTitle(xtitle);
        gr_2->GetXaxis()->SetTitleSize(0.07);
        gr_2->GetYaxis()->SetLabelSize(0.07);
        gr_2->Draw("AP");
      }
      // Row 3
      c_gr->cd(padCounter+2*maxCol);
      TString row3 = "3_"+runNumber;
      if (mapAllRuns.count(row3)>0){
        gr_3 = mapAllRuns.find(row3)->second;
        gr_3->Draw("A");
        gr_3->GetYaxis()->SetTitle("");
        gr_3->GetXaxis()->SetTitle(xtitle);
        gr_3->GetXaxis()->SetTitleSize(0.07);
        gr_3->GetYaxis()->SetLabelSize(0.07);
        gr_3->Draw("AP");
      }
      // Row 4
      c_gr->cd(padCounter+3*maxCol);
      TString row4 = "4_"+runNumber;
      if (mapAllRuns.count(row4)>0){
        gr_4 = mapAllRuns.find(row4)->second;
        gr_4->Draw("A");
        gr_4->GetYaxis()->SetTitle("");
        gr_4->GetXaxis()->SetTitle(xtitle);
        gr_4->GetXaxis()->SetTitleSize(0.07);
        gr_4->GetYaxis()->SetLabelSize(0.07);
        gr_4->Draw("AP");
      }
      padCounter++;
    }
    
    if (dir!="" && save_tcanvas){
      TString fname = fileName;
      fname += "_"+StringUtilities::int2TString(i+1,2);
      c_gr->SaveAs(dir+fname+".eps");
    }
  }
}


void 
PlotUtilities::Plot_MultiRuns_6Rows(TString fileName, std::vector<TString> adcList, \
std::map< TString, std::map<TString,TGraph*> > gr_map1, \
std::map< TString, std::map<TString,TGraph*> > gr_map2, \
std::map< TString, std::map<TString,TGraph*> > gr_map3, \
std::map< TString, std::map<TString,TGraph*> > gr_map4, \
std::map< TString, std::map<TString,TGraph*> > gr_map5, \
std::map< TString, std::map<TString,TGraph*> > gr_map6, TString dir)
{
  TString xtitle = "Time (hours)";
  
  TGraph* gr_1 = 0x0;
  TGraph* gr_2 = 0x0;
  TGraph* gr_3 = 0x0;
  TGraph* gr_4 = 0x0;
  TGraph* gr_5 = 0x0;
  TGraph* gr_6 = 0x0;
  
  if (dir!=""){
    if (!dir.EndsWith("/")) dir += "/";
  }
  
  TCanvas* c_gr = new TCanvas("c_gr","Graph All Channels",2000,2000);
  
  int adcSize = adcList.size();
  for (int i=0; i<adcSize; i++){
    
    TString adcName = adcList[i];
    cout << "ADC Name: " << adcName << endl;
    
    std::map<TString, int> mapRun;
    std::map<TString, TGraph*> mapAllRuns;
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map1.begin(); iter != gr_map1.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i]; 
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("1_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map2.begin(); iter != gr_map2.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("2_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map3.begin(); iter != gr_map3.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("3_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map4.begin(); iter != gr_map4.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("4_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map5.begin(); iter != gr_map5.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("5_")] = mapADC.find(adcName)->second;
      }
    }
    
    for (std::map<TString, std::map<TString, TGraph*> >::const_iterator iter = gr_map6.begin(); iter != gr_map6.end(); ++iter){
      TString runNumber = iter->first;
      adcName = adcList[i];
      std::map<TString, TGraph*> mapADC = iter->second;
      if (mapADC.count(adcName)>0){
        mapRun[runNumber]++;
        mapAllRuns[runNumber.Prepend("6_")] = mapADC.find(adcName)->second;
      }
    }
    
    int maxCol = mapRun.size();
    
    bool new_tcanvas = true;
    bool save_tcanvas = true;
    
    if (new_tcanvas){
      c_gr = new TCanvas("c_gr","Graph Some Channels",500*maxCol,3000);
      c_gr->Divide(maxCol,6);
    }
    
    int padCounter = 1;
    for (std::map<TString,int>::const_iterator it = mapRun.begin(); it != mapRun.end(); ++it){
      TString runNumber = it->first;
      // Row 1
      c_gr->cd(padCounter);
      TString row1 = "1_"+runNumber;
      if (mapAllRuns.count(row1)>0){
        gr_1 = mapAllRuns.find(row1)->second;
        TString title = gr_1->GetTitle();
        gr_1->SetTitle(runNumber + ": " + title);
        gr_1->Draw("A");
        gr_1->GetYaxis()->SetTitle("");
        gr_1->GetXaxis()->SetTitle(xtitle);
        gr_1->GetXaxis()->SetTitleSize(0.07);
        gr_1->GetYaxis()->SetLabelSize(0.07);
        gr_1->Draw("AP");
      }
      // Row 2
      c_gr->cd(padCounter+maxCol);
      TString row2 = "2_"+runNumber;
      if (mapAllRuns.count(row2)>0){
        gr_2 = mapAllRuns.find(row2)->second;
        gr_2->Draw("A");
        gr_2->GetYaxis()->SetTitle("");
        gr_2->GetXaxis()->SetTitle(xtitle);
        gr_2->GetXaxis()->SetTitleSize(0.07);
        gr_2->GetYaxis()->SetLabelSize(0.07);
        gr_2->Draw("AP");
      }
      // Row 3
      c_gr->cd(padCounter+2*maxCol);
      TString row3 = "3_"+runNumber;
      if (mapAllRuns.count(row3)>0){
        gr_3 = mapAllRuns.find(row3)->second;
        gr_3->Draw("A");
        gr_3->GetYaxis()->SetTitle("");
        gr_3->GetXaxis()->SetTitle(xtitle);
        gr_3->GetXaxis()->SetTitleSize(0.07);
        gr_3->GetYaxis()->SetLabelSize(0.07);
        gr_3->Draw("AP");
      }
      // Row 4
      c_gr->cd(padCounter+3*maxCol);
      TString row4 = "4_"+runNumber;
      if (mapAllRuns.count(row4)>0){
        gr_4 = mapAllRuns.find(row4)->second;
        gr_4->Draw("A");
        gr_4->GetYaxis()->SetTitle("");
        gr_4->GetXaxis()->SetTitle(xtitle);
        gr_4->GetXaxis()->SetTitleSize(0.07);
        gr_4->GetYaxis()->SetLabelSize(0.07);
        gr_4->Draw("AP");
      }
      
      // Row 5
      c_gr->cd(padCounter+4*maxCol);
      TString row5 = "5_"+runNumber;
      if (mapAllRuns.count(row5)>0){
        gr_5 = mapAllRuns.find(row5)->second;
        gr_5->Draw("A");
        gr_5->GetYaxis()->SetTitle("");
        gr_5->GetXaxis()->SetTitle(xtitle);
        gr_5->GetXaxis()->SetTitleSize(0.07);
        gr_5->GetYaxis()->SetLabelSize(0.07);
        gr_5->Draw("AP");
      }
      
      // Row 6
      c_gr->cd(padCounter+5*maxCol);
      TString row6 = "6_"+runNumber;
      if (mapAllRuns.count(row6)>0){
        gr_6 = mapAllRuns.find(row6)->second;
        gr_6->Draw("A");
        gr_6->GetYaxis()->SetTitle("");
        gr_6->GetXaxis()->SetTitle(xtitle);
        gr_6->GetXaxis()->SetTitleSize(0.07);
        gr_6->GetYaxis()->SetLabelSize(0.07);
        gr_6->Draw("AP");
      }
      padCounter++;
    }
    
    if (dir!="" && save_tcanvas){
      TString fname = fileName;
      fname += "_"+StringUtilities::int2TString(i+1,2);
      c_gr->SaveAs(dir+fname+".pdf");
    }
  }
}

// ******************
// The key MUST be a pair<x,y> and value will be whatever you like
// ******************

TH2F* 
PlotUtilities::Plot2D(std::map< std::pair<double,double>, double> detector_map, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax){
  std::map< std::pair<double,double>, double>::const_iterator iter;
  
  TH2F* h2D = new TH2F("h2D", "The Detector",xmax-xmin,xmin,xmax,ymax-ymin,ymin,ymax);
  h2D->SetStats(false);
  h2D->SetXTitle("X (Column)");
  h2D->SetYTitle("Y (Row)");
  
  // If the user inputs a zmin and zmax where zmin > zmax then zmax will be autoscaled
  if (zmax > zmin)  h2D->SetMaximum(zmax);
  
  h2D->SetMinimum(zmin);
  
  for (iter=detector_map.begin(); iter!=detector_map.end(); ++iter){
    std::pair<double,double> coord = iter->first;
    double x = coord.first;
    double y = coord.second;
    double value = iter->second;
    
    h2D->Fill(x,y,value);
  }
  
  return h2D;  
}

TH2F* 
PlotUtilities::Plot2D(std::map< std::pair<int,int>, double> detector_map, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax){
  std::map< std::pair<int,int>, double>::const_iterator iter;
  
  TH2F* h2D = new TH2F("h2D", "The Detector",xmax-xmin,xmin,xmax,ymax-ymin,ymin,ymax);
  h2D->SetStats(false);
  h2D->SetXTitle("X (Column)");
  h2D->SetYTitle("Y (Row)");
  
  // If the user inputs a zmin and zmax where zmin > zmax then zmax will be autoscaled
  if (zmax > zmin)  h2D->SetMaximum(zmax);
  
  h2D->SetMinimum(zmin);
  
  for (iter=detector_map.begin(); iter!=detector_map.end(); ++iter){
    std::pair<int,int> coord = iter->first;
    double x = coord.first;
    double y = coord.second;
    double value = iter->second;
    
    h2D->Fill(x,y,value);
  }
  
  return h2D;  
}

