
/*********************************************************************************
 *
 * 2015/03/31 Kei Moriya
 *
 * Parse the output text file of timePlugin_mem_vmem.txt to create graphs.
 *
 *********************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>
#include <map>
#include "unistd.h" // to use optarg
#include <assert.h>
#include <stdlib.h>

// ROOT header files
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TTree.h"
#include "TMath.h"
#include "TLatex.h"
#include "TLine.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLorentzVector.h"
#include "TProfile.h"
#include "TGraphErrors.h"

using namespace std;

int main(int argc, char **argv){

  std::string RUNPERIOD = "";
  int  VER_INIT =  9;
  int  VER_LAST = 15;
  bool debug   = false;

  const Int_t colors[] = {kBlack, kRed, kGreen+2, kBlue, kMagenta, kPink-9};

  extern char* optarg;
  // Check command line arguments
  int c;
  while((c = getopt(argc,argv,"hV:F:R:d")) != -1){
    switch(c){
    case 'h':
      cout << "process_timePlugin_mem_vmem: " << endl;
      cout << "Options:" << endl;
      cout << "\t-h    This message" << endl;
      cout << "\t-V    Version number of first launch to analyze" << endl;
      cout << "\t-F    Set final  launch version to process" << endl;
      cout << "\t-R    Run period" << endl;
      cout << "\t-d    Print debug messages" << endl;
      exit(-1);
      break;
    case 'F':
      VER_LAST = atoi(optarg);
      break;
    case 'V':
      VER_INIT = atoi(optarg);
      break;
    case 'R':
      RUNPERIOD = optarg;
      break;
    case 'd':
      debug = true;
      break;
    default:
      break;
    }
  }

  if(RUNPERIOD != "2014_10" && RUNPERIOD != "2015_03"){
    cout << "RUNPERIOD must be 2014_10 or 2015_03" << endl;
    abort();
  }
  cout << "RUNPERIOD: " << RUNPERIOD << endl;
  char command[200];
  sprintf(command,"mkdir -p figures/%s",RUNPERIOD.c_str());
  system(command);

  std::vector<int> SKIPPED_LAUNCHES;
  if(RUNPERIOD=="2014_10")      SKIPPED_LAUNCHES.push_back(14);
  else if(RUNPERIOD=="2015_03") SKIPPED_LAUNCHES.push_back(1);
  cout << "Skipped launches: " << endl;
  for(int i=0;i<SKIPPED_LAUNCHES.size();i++){
    cout << "   ver" << setw(2) << setfill('0') << SKIPPED_LAUNCHES[i] << endl;
  }
  cout << "-----------------------------" << endl;

  // Get the launch ver numbers
  std::vector<int> LAUNCHVERS;
  for(int i=VER_INIT;i<=VER_LAST;i++){
    std::vector<int>::iterator it;
    it = find (SKIPPED_LAUNCHES.begin(), SKIPPED_LAUNCHES.end(), i);
    if (it != SKIPPED_LAUNCHES.end()){
      cout << "skipping launch ver" << setw(2) << setfill('0') << i << endl;
    }else{
      LAUNCHVERS.push_back(i);
    }
  }

  cout << "TOTAL OF " << LAUNCHVERS.size() << " LAUNCHES" << endl;
  for(int i=0;i<LAUNCHVERS.size();i++){
    cout << "  " << setw(2) << setfill('0') << i+1 << " : " <<  setw(2) << setfill('0') << LAUNCHVERS[i] << endl;
  }
  int NVERS = LAUNCHVERS.size();

  ifstream IN;
  if(RUNPERIOD == "2014_10"){
    IN.open("timePlguin_mem_vmem.txt");
  }else if(RUNPERIOD == "2015_03"){
    IN.open("timePlguin_mem_vmem_2015_03.txt");
  }
  if(!IN){
    cout << "infile timePlguin_mem_vmem.txt does not exist!!!" << endl;
    abort();
  }

  // input variables
  int run, file;
  std::string timePlugin[NVERS]; // in sec, can be "NULL"
  std::string mem[NVERS];        // in kb,  can be "NULL"
  std::string vmem[NVERS];       // in kb,  can be "NULL"
  std::string nevents[NVERS];    // can be "NULL"

  int run_previous = -999;
  int hundreds_previous = -100;

  // This is for every single run, file
  int ntotalAll = 0;

  bool isNewRun = false;

  // Graphs will always be created as correlations of
  // most recent launch against previous launches.

  // Compare only to past 4 launches
  const Int_t N = 4;
  TGraph *gtimePlugin[N];
  TGraph *gmem[N];
  TGraph *gvmem[N];
  TGraph *gnevents[N];
  TH1F *hnevents_diff[N];
  TGraph *gtimePlugin_per_event[N];
  char gname[200];
  for(Int_t i=0;i<N;i++){
    gtimePlugin[i] = new TGraph();
    sprintf(gname,"gtimePlugin_ver%2.2d_ver%2.2d",VER_LAST,LAUNCHVERS[i]);
    gtimePlugin[i]->SetTitle(gname);
    gmem[i] = new TGraph();
    sprintf(gname,"gmem_ver%2.2d_ver%2.2d",VER_LAST,LAUNCHVERS[i]);
    gmem[i]->SetTitle(gname);
    gvmem[i] = new TGraph();
    sprintf(gname,"gvmem_ver%2.2d_ver%2.2d",VER_LAST,LAUNCHVERS[i]);
    gvmem[i]->SetTitle(gname);
    gnevents[i] = new TGraph();
    sprintf(gname,"gnevents_ver%2.2d_ver%2.2d",VER_LAST,LAUNCHVERS[i]);
    gnevents[i]->SetTitle(gname);
    sprintf(gname,"hnevents_diff_ver%2.2d_ver%2.2d",VER_LAST,LAUNCHVERS[i]);
    hnevents_diff[i] = new TH1F(gname,"",200,-100,100);

    gtimePlugin_per_event[i] = new TGraph();
    sprintf(gname,"gtimePlugin_per_event_ver%2.2d_ver%2.2d",VER_LAST,LAUNCHVERS[i]);
    gtimePlugin_per_event[i]->SetTitle(gname);
  }

  Double_t timePlugin_value[NVERS];
  Double_t mem_value[NVERS];
  Double_t vmem_value[NVERS];
  Double_t nevents_value[NVERS];
  Double_t timePlugin_per_event_value[NVERS];

  while(IN >> run >> file){
    for(int i=0;i<NVERS;i++) IN >> timePlugin[i];
    for(int i=0;i<NVERS;i++) IN >> mem[i];
    for(int i=0;i<NVERS;i++) IN >> vmem[i];
    for(int i=0;i<NVERS;i++) IN >> nevents[i];

    if(run != run_previous){
      if(debug) cout << "beginning of new run " << run << endl;
      run_previous = run;
      isNewRun = true;

      if(run / 100 != hundreds_previous){
	if(debug) cout << "start of new hundreds: " << run / 100 * 100 << " with run = " << run << endl;
	hundreds_previous = (run / 100);
      } // end of start of new hundreds
    } // end of beginning of new run

    if(debug) cout << "processing run = " << run << " file = " << file << endl;

    // Format input timePlugin, mem, vmem, nevents values to numbers
    for(int i=0;i<NVERS;i++){
      if(debug) cout << "i = " << i << " timePlugin[" << i << "] = " << timePlugin[i] << endl;

      // timePlugin
      if(timePlugin[i] == "NULL") timePlugin_value[i] = -999;
      else                        timePlugin_value[i] = atof(timePlugin[i].c_str()) / 60.;

      if(debug) cout << "timePlugin value = " << timePlugin_value[i] << endl;

      // mem
      if(mem[i] == "NULL") mem_value[i] = -999;
      else{
	size_t pos = mem[i].find("k");
	if(pos==std::string::npos){
	  cout << "For run " << run << " file " << file << " \"kb\" was not found for mem[" << i << "] = " << mem[i] << endl;
	}
	if(debug) cout << "mem value will be set to : " << mem[i].replace(pos,2,"") << endl;
	mem_value[i] = atof(mem[i].replace(pos,2,"").c_str()) / 1024.;
      }
      if(debug) cout << "mem value = " << mem_value[i] << endl;

      // vmem
      if(vmem[i] == "NULL") vmem_value[i] = -999;
      else{
	size_t pos = vmem[i].find("k");
	if(pos==std::string::npos){
	  cout << "For run " << run << " file " << file << " \"kb\" was not found for vmem[" << i << "] = " << vmem[i] << endl;
	}
	if(debug) cout << "vmem value will be set to : " << vmem[i].replace(pos,2,"") << endl;
	vmem_value[i] = atof(vmem[i].replace(pos,2,"").c_str()) / 1024.;
      }
      if(debug) cout << "vmem value = " << vmem_value[i] << endl;

      // nevents
      if(nevents[i] == "NULL") nevents_value[i] = -999;
      else                     nevents_value[i] = atof(nevents[i].c_str());
      if(debug) cout << "nevents value = " << nevents[i] << endl;

      // timePlugin_per_event
      if(timePlugin_value[i] == -999 || nevents_value[i] == -999)
	timePlugin_per_event_value[i] = -999;
      else
	timePlugin_per_event_value[i] = timePlugin_value[i] / nevents_value[i];

    }

    // Fill graphs with values
    for(Int_t i=0;i<N;i++){
      // Values with index of NVERS-1 are final launch
      gtimePlugin[i]->SetPoint(gtimePlugin[i]->GetN(),timePlugin_value[i],timePlugin_value[NVERS-1]);
      gmem[i]->SetPoint(gmem[i]->GetN(),mem_value[i],mem_value[NVERS-1]);
      gvmem[i]->SetPoint(gvmem[i]->GetN(),vmem_value[i],vmem_value[NVERS-1]);
      gnevents[i]->SetPoint(gnevents[i]->GetN(),nevents_value[i],nevents_value[NVERS-1]);
      hnevents_diff[i]->Fill(nevents_value[NVERS-1] - nevents_value[i]);
      gtimePlugin_per_event[i]->SetPoint(gtimePlugin_per_event[i]->GetN(),timePlugin_per_event_value[i],timePlugin_per_event_value[NVERS-1]);
    }

    ntotalAll++;
  } // end of loop over input file
  cout << "total of " << ntotalAll << " lines processed" << endl;

  // Draw graphs
  TCanvas *canvas = new TCanvas("canvas","canvas",1800,900);
  canvas->SetTopMargin(0.03);
  canvas->SetBottomMargin(0.11);
  canvas->SetLeftMargin(0.08);
  canvas->SetRightMargin(0.03);

  TLine *line_equal = new TLine(0,0,1,1);
  line_equal->SetLineColor(28);
  line_equal->SetLineStyle(4);
  line_equal->SetLineWidth(1);

  for(Int_t i=0;i<N;i++){

    // timePlugin
    gtimePlugin[i]->SetTitle("");
    sprintf(gname,"plugin time for ver %2.2d (min)",LAUNCHVERS[i]);
    gtimePlugin[i]->GetXaxis()->SetTitle(gname);
    gtimePlugin[i]->GetXaxis()->CenterTitle(gname);
    gtimePlugin[i]->GetXaxis()->SetTitleSize(0.050);
    gtimePlugin[i]->GetXaxis()->SetTitleFont(132);
    gtimePlugin[i]->GetXaxis()->SetLabelSize(0.040);
    gtimePlugin[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"plugin time for ver %2.2d (min)",VER_LAST);
    gtimePlugin[i]->GetYaxis()->SetTitle(gname);
    gtimePlugin[i]->GetYaxis()->CenterTitle(gname);
    gtimePlugin[i]->GetYaxis()->SetTitleSize(0.050);
    gtimePlugin[i]->GetYaxis()->SetTitleFont(132);
    gtimePlugin[i]->GetYaxis()->SetLabelSize(0.040);
    gtimePlugin[i]->GetYaxis()->SetLabelFont(132);
    gtimePlugin[i]->GetYaxis()->SetTitleOffset(0.700);

    gtimePlugin[i]->SetMarkerColor(colors[0]);
    gtimePlugin[i]->SetMarkerSize(1.2);
    gtimePlugin[i]->SetMarkerStyle(20);
    gtimePlugin[i]->SetLineColor(colors[0]);
    gtimePlugin[i]->SetLineWidth(1);
    gtimePlugin[i]->SetLineStyle(1);

    gtimePlugin[i]->GetXaxis()->SetLimits(0,360);
    gtimePlugin[i]->SetMinimum(0);
    gtimePlugin[i]->SetMaximum(360);

    gtimePlugin[i]->Draw("AP");

    line_equal->SetX2(360);
    line_equal->SetY2(360);
    line_equal->Draw("same");

    sprintf(gname,"figures/%s/001___timePlugin_ver%2.2d_ver%2.2d.png",RUNPERIOD.c_str(),VER_LAST,LAUNCHVERS[i]);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

    // mem
    gmem[i]->SetTitle("");
    sprintf(gname,"mem for ver %2.2d (MB)",LAUNCHVERS[i]);
    gmem[i]->GetXaxis()->SetTitle(gname);
    gmem[i]->GetXaxis()->CenterTitle(gname);
    gmem[i]->GetXaxis()->SetTitleSize(0.050);
    gmem[i]->GetXaxis()->SetTitleFont(132);
    gmem[i]->GetXaxis()->SetLabelSize(0.040);
    gmem[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"mem for ver %2.2d (MB)",VER_LAST);
    gmem[i]->GetYaxis()->SetTitle(gname);
    gmem[i]->GetYaxis()->CenterTitle(gname);
    gmem[i]->GetYaxis()->SetTitleSize(0.050);
    gmem[i]->GetYaxis()->SetTitleFont(132);
    gmem[i]->GetYaxis()->SetLabelSize(0.040);
    gmem[i]->GetYaxis()->SetLabelFont(132);
    gmem[i]->GetYaxis()->SetTitleOffset(0.700);

    gmem[i]->SetMarkerColor(colors[1]);
    gmem[i]->SetMarkerSize(1.2);
    gmem[i]->SetMarkerStyle(20);
    gmem[i]->SetLineColor(colors[1]);
    gmem[i]->SetLineWidth(1);
    gmem[i]->SetLineStyle(1);

    gmem[i]->GetXaxis()->SetLimits(0,5000);
    gmem[i]->SetMinimum(0);
    gmem[i]->SetMaximum(5000);

    gmem[i]->Draw("AP");

    line_equal->SetX2(5000);
    line_equal->SetY2(5000);
    line_equal->Draw("same");

    sprintf(gname,"figures/%s/002___mem_ver%2.2d_ver%2.2d.png",RUNPERIOD.c_str(),VER_LAST,LAUNCHVERS[i]);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

    // vmem
    gvmem[i]->SetTitle("");
    sprintf(gname,"vmem for ver %2.2d (MB)",LAUNCHVERS[i]);
    gvmem[i]->GetXaxis()->SetTitle(gname);
    gvmem[i]->GetXaxis()->CenterTitle(gname);
    gvmem[i]->GetXaxis()->SetTitleSize(0.050);
    gvmem[i]->GetXaxis()->SetTitleFont(132);
    gvmem[i]->GetXaxis()->SetLabelSize(0.040);
    gvmem[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"vmem for ver %2.2d (MB)",VER_LAST);
    gvmem[i]->GetYaxis()->SetTitle(gname);
    gvmem[i]->GetYaxis()->CenterTitle(gname);
    gvmem[i]->GetYaxis()->SetTitleSize(0.050);
    gvmem[i]->GetYaxis()->SetTitleFont(132);
    gvmem[i]->GetYaxis()->SetLabelSize(0.040);
    gvmem[i]->GetYaxis()->SetLabelFont(132);
    gvmem[i]->GetYaxis()->SetTitleOffset(0.700);

    gvmem[i]->SetMarkerColor(colors[2]);
    gvmem[i]->SetMarkerSize(1.2);
    gvmem[i]->SetMarkerStyle(20);
    gvmem[i]->SetLineColor(colors[2]);
    gvmem[i]->SetLineWidth(1);
    gvmem[i]->SetLineStyle(1);

    gvmem[i]->GetXaxis()->SetLimits(0,5000);
    gvmem[i]->SetMinimum(0);
    gvmem[i]->SetMaximum(5000);

    gvmem[i]->Draw("AP");

    line_equal->SetX2(5000);
    line_equal->SetY2(5000);
    line_equal->Draw("same");

    sprintf(gname,"figures/%s/003___vmem_ver%2.2d_ver%2.2d.png",RUNPERIOD.c_str(),VER_LAST,LAUNCHVERS[i]);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

    // nevents
    gnevents[i]->SetTitle("");
    sprintf(gname,"# events for ver %2.2d",LAUNCHVERS[i]);
    gnevents[i]->GetXaxis()->SetTitle(gname);
    gnevents[i]->GetXaxis()->CenterTitle(gname);
    gnevents[i]->GetXaxis()->SetTitleSize(0.050);
    gnevents[i]->GetXaxis()->SetTitleFont(132);
    gnevents[i]->GetXaxis()->SetLabelSize(0.040);
    gnevents[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"# events for ver %2.2d",VER_LAST);
    gnevents[i]->GetYaxis()->SetTitle(gname);
    gnevents[i]->GetYaxis()->CenterTitle(gname);
    gnevents[i]->GetYaxis()->SetTitleSize(0.050);
    gnevents[i]->GetYaxis()->SetTitleFont(132);
    gnevents[i]->GetYaxis()->SetLabelSize(0.040);
    gnevents[i]->GetYaxis()->SetLabelFont(132);
    gnevents[i]->GetYaxis()->SetTitleOffset(0.700);

    gnevents[i]->SetMarkerColor(colors[3]);
    gnevents[i]->SetMarkerSize(1.2);
    gnevents[i]->SetMarkerStyle(20);
    gnevents[i]->SetLineColor(colors[3]);
    gnevents[i]->SetLineWidth(1);
    gnevents[i]->SetLineStyle(1);

    gnevents[i]->GetXaxis()->SetLimits(0,35000);
    gnevents[i]->SetMinimum(0);
    gnevents[i]->SetMaximum(35000);

    gnevents[i]->Draw("AP");

    line_equal->SetX2(35000);
    line_equal->SetY2(35000);
    line_equal->Draw("same");

    sprintf(gname,"figures/%s/004___nevents_ver%2.2d_ver%2.2d.png",RUNPERIOD.c_str(),VER_LAST,LAUNCHVERS[i]);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

    // hnevents_diff
    hnevents_diff[i]->SetTitle("");
    sprintf(gname,"# events for ver %2.2d - # events for ver %2.2d",VER_LAST, LAUNCHVERS[i]);
    hnevents_diff[i]->GetXaxis()->SetTitle(gname);
    hnevents_diff[i]->GetXaxis()->CenterTitle(gname);
    hnevents_diff[i]->GetXaxis()->SetTitleSize(0.050);
    hnevents_diff[i]->GetXaxis()->SetTitleFont(132);
    hnevents_diff[i]->GetXaxis()->SetLabelSize(0.040);
    hnevents_diff[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"counts");
    hnevents_diff[i]->GetYaxis()->SetTitle(gname);
    hnevents_diff[i]->GetYaxis()->CenterTitle(gname);
    hnevents_diff[i]->GetYaxis()->SetTitleSize(0.050);
    hnevents_diff[i]->GetYaxis()->SetTitleFont(132);
    hnevents_diff[i]->GetYaxis()->SetLabelSize(0.040);
    hnevents_diff[i]->GetYaxis()->SetLabelFont(132);
    hnevents_diff[i]->GetYaxis()->SetTitleOffset(0.700);

    hnevents_diff[i]->SetLineColor(colors[4]);
    hnevents_diff[i]->SetLineWidth(1);
    hnevents_diff[i]->SetLineStyle(1);

    hnevents_diff[i]->SetMinimum(0.5);
    // hnevents_diff[i]->SetMaximum(35000);

    canvas->SetLogy(1);
    hnevents_diff[i]->Draw("");

    line_equal->SetX1(0);
    line_equal->SetX2(0);
    line_equal->SetY1(0);
    line_equal->SetY2(hnevents_diff[i]->GetMaximum() * 1.05);
    line_equal->Draw("same");

    sprintf(gname,"figures/%s/005___hnevents_diff_ver%2.2d_ver%2.2d.png",RUNPERIOD.c_str(),VER_LAST,LAUNCHVERS[i]);
    canvas->SaveAs(gname);
    canvas->SetLogy(0);
    //_____________________________________________________________________________________________

    // timePlugin_per_event
    gtimePlugin_per_event[i]->SetTitle("");
    sprintf(gname,"plugin time/event for ver %2.2d (sec)",LAUNCHVERS[i]);
    gtimePlugin_per_event[i]->GetXaxis()->SetTitle(gname);
    gtimePlugin_per_event[i]->GetXaxis()->CenterTitle(gname);
    gtimePlugin_per_event[i]->GetXaxis()->SetTitleSize(0.050);
    gtimePlugin_per_event[i]->GetXaxis()->SetTitleFont(132);
    gtimePlugin_per_event[i]->GetXaxis()->SetLabelSize(0.040);
    gtimePlugin_per_event[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"plugin time/event for ver %2.2d (sec)",VER_LAST);
    gtimePlugin_per_event[i]->GetYaxis()->SetTitle(gname);
    gtimePlugin_per_event[i]->GetYaxis()->CenterTitle(gname);
    gtimePlugin_per_event[i]->GetYaxis()->SetTitleSize(0.050);
    gtimePlugin_per_event[i]->GetYaxis()->SetTitleFont(132);
    gtimePlugin_per_event[i]->GetYaxis()->SetLabelSize(0.040);
    gtimePlugin_per_event[i]->GetYaxis()->SetLabelFont(132);
    gtimePlugin_per_event[i]->GetYaxis()->SetTitleOffset(0.700);

    gtimePlugin_per_event[i]->SetMarkerColor(colors[5]);
    gtimePlugin_per_event[i]->SetMarkerSize(1.2);
    gtimePlugin_per_event[i]->SetMarkerStyle(20);
    gtimePlugin_per_event[i]->SetLineColor(colors[5]);
    gtimePlugin_per_event[i]->SetLineWidth(1);
    gtimePlugin_per_event[i]->SetLineStyle(1);

    gtimePlugin_per_event[i]->GetXaxis()->SetLimits(pow(10.,-5.),1.);
    gtimePlugin_per_event[i]->SetMinimum(pow(10.,-5.));
    gtimePlugin_per_event[i]->SetMaximum(1.);

    canvas->SetLogx(1);
    canvas->SetLogy(1);
    gtimePlugin_per_event[i]->Draw("AP");

    line_equal->SetX1(pow(10.,-5.));
    line_equal->SetY1(pow(10.,-5.));
    line_equal->SetX2(1.);
    line_equal->SetY2(1.);
    line_equal->Draw("same");

    sprintf(gname,"figures/%s/006___timePlugin_per_event_ver%2.2d_ver%2.2d.png",RUNPERIOD.c_str(),VER_LAST,LAUNCHVERS[i]);
    canvas->SaveAs(gname);
    canvas->SetLogx(0);
    canvas->SetLogy(0);
    //_____________________________________________________________________________________________

  }

  return 0;
}
