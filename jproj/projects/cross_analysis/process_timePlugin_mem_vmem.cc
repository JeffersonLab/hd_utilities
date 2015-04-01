
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

  int  NVERS   = 4;
  int  VER_INIT = 9;
  bool debug   = false;

  const Int_t colors[] = {kBlack, kRed, kGreen+2, kBlue, kMagenta};

  extern char* optarg;
  // Check command line arguments
  int c;
  while((c = getopt(argc,argv,"hN:V:d")) != -1){
    switch(c){
    case 'h':
      cout << "process_timePlugin_mem_vmem: " << endl;
      cout << "Options:" << endl;
      cout << "\t-h    This message" << endl;
      cout << "\t-N    Set number of launch versions to process" << endl;
      cout << "\t-V    Version number of first launch to analyze" << endl;
      cout << "\t-d    Print debug messages" << endl;
      exit(-1);
      break;
    case 'N':
      NVERS = atoi(optarg);
      break;
    case 'V':
      VER_INIT = atoi(optarg);
      break;
    case 'd':
      debug = true;
      break;
    default:
      break;
    }
  }

  ifstream IN("timePlguin_mem_vmem.txt");
  if(!IN){
    cout << "infile timePlguin_mem_vmem.txt does not exist!!!" << endl;
    abort();
  }

  // input variables
  int run, file;
  std::string timePlugin[NVERS]; // in sec, can be "NULL"
  std::string mem[NVERS];        // in kb,  can be "NULL"
  std::string vmem[NVERS];       // in kb,  can be "NULL"

  int run_previous = -999;
  int hundreds_previous = -100;

  // This is for every single run, file
  int ntotalAll = 0;

  bool isNewRun = false;

  // Graphs will always be created as correlations of
  // most recent launch against previous launches.
  TGraph *gtimePlugin[NVERS-1];
  TGraph *gmem[NVERS-1];
  TGraph *gvmem[NVERS-1];
  char gname[200];
  for(Int_t i=0;i<NVERS-1;i++){
    gtimePlugin[i] = new TGraph();
    sprintf(gname,"gtimePlugin_ver%2.2d_ver%2.2d",VER_INIT + NVERS - 1,VER_INIT + i);
    gtimePlugin[i]->SetTitle(gname);
    gmem[i] = new TGraph();
    sprintf(gname,"gmem_ver%2.2d_ver%2.2d",VER_INIT + NVERS - 1,VER_INIT + i);
    gmem[i]->SetTitle(gname);
    gvmem[i] = new TGraph();
    sprintf(gname,"gvmem_ver%2.2d_ver%2.2d",VER_INIT + NVERS - 1,VER_INIT + i);
    gvmem[i]->SetTitle(gname);
  }

  Double_t timePlugin_value[NVERS];
  Double_t mem_value[NVERS];
  Double_t vmem_value[NVERS];

  while(IN >> run >> file){
    for(int i=0;i<NVERS;i++) IN >> timePlugin[i];
    for(int i=0;i<NVERS;i++) IN >> mem[i];
    for(int i=0;i<NVERS;i++) IN >> vmem[i];

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

    // Format input timePlugin, mem, vmem values to numbers
    for(int i=0;i<NVERS;i++){
      if(debug) cout << "i = " << i << " timePlugin[" << i << "] = " << timePlugin[i] << endl;

      if(timePlugin[i] == "NULL") timePlugin_value[i] = -999;
      else                        timePlugin_value[i] = atof(timePlugin[i].c_str()) / 60.;

      if(debug) cout << "timePlugin value = " << timePlugin_value[i] << endl;

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
    }

    // Fill graphs with values
    for(Int_t i=0;i<NVERS-1;i++){
      gtimePlugin[i]->SetPoint(gtimePlugin[i]->GetN(),timePlugin_value[i],timePlugin_value[NVERS-1]);
      gmem[i]->SetPoint(gmem[i]->GetN(),mem_value[i],mem_value[NVERS-1]);
      gvmem[i]->SetPoint(gvmem[i]->GetN(),vmem_value[i],vmem_value[NVERS-1]);
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

  for(Int_t i=0;i<NVERS-1;i++){

    // timePlugin
    gtimePlugin[i]->SetTitle("");
    sprintf(gname,"plugin time for ver %2.2d (min)",VER_INIT + i);
    gtimePlugin[i]->GetXaxis()->SetTitle(gname);
    gtimePlugin[i]->GetXaxis()->CenterTitle(gname);
    gtimePlugin[i]->GetXaxis()->SetTitleSize(0.050);
    gtimePlugin[i]->GetXaxis()->SetTitleFont(132);
    gtimePlugin[i]->GetXaxis()->SetLabelSize(0.040);
    gtimePlugin[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"plugin time for ver %2.2d (min)",VER_INIT + NVERS - 1);
    gtimePlugin[i]->GetYaxis()->SetTitle(gname);
    gtimePlugin[i]->GetYaxis()->CenterTitle(gname);
    gtimePlugin[i]->GetYaxis()->SetTitleSize(0.050);
    gtimePlugin[i]->GetYaxis()->SetTitleFont(132);
    gtimePlugin[i]->GetYaxis()->SetLabelSize(0.040);
    gtimePlugin[i]->GetYaxis()->SetLabelFont(132);
    gtimePlugin[i]->GetYaxis()->SetTitleOffset(0.650);

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

    sprintf(gname,"figures/001___timePlugin_ver%2.2d_ver%2.2d.png",VER_INIT+NVERS-1,VER_INIT+i);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

    // mem
    gmem[i]->SetTitle("");
    sprintf(gname,"mem for ver %2.2d (MB)",VER_INIT + i);
    gmem[i]->GetXaxis()->SetTitle(gname);
    gmem[i]->GetXaxis()->CenterTitle(gname);
    gmem[i]->GetXaxis()->SetTitleSize(0.050);
    gmem[i]->GetXaxis()->SetTitleFont(132);
    gmem[i]->GetXaxis()->SetLabelSize(0.040);
    gmem[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"mem for ver %2.2d (MB)",VER_INIT + NVERS - 1);
    gmem[i]->GetYaxis()->SetTitle(gname);
    gmem[i]->GetYaxis()->CenterTitle(gname);
    gmem[i]->GetYaxis()->SetTitleSize(0.050);
    gmem[i]->GetYaxis()->SetTitleFont(132);
    gmem[i]->GetYaxis()->SetLabelSize(0.040);
    gmem[i]->GetYaxis()->SetLabelFont(132);
    gmem[i]->GetYaxis()->SetTitleOffset(0.650);

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

    sprintf(gname,"figures/002___mem_ver%2.2d_ver%2.2d.png",VER_INIT+NVERS-1,VER_INIT+i);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

    // vmem
    gvmem[i]->SetTitle("");
    sprintf(gname,"vmem for ver %2.2d (MB)",VER_INIT + i);
    gvmem[i]->GetXaxis()->SetTitle(gname);
    gvmem[i]->GetXaxis()->CenterTitle(gname);
    gvmem[i]->GetXaxis()->SetTitleSize(0.050);
    gvmem[i]->GetXaxis()->SetTitleFont(132);
    gvmem[i]->GetXaxis()->SetLabelSize(0.040);
    gvmem[i]->GetXaxis()->SetLabelFont(132);

    sprintf(gname,"vmem for ver %2.2d (MB)",VER_INIT + NVERS - 1);
    gvmem[i]->GetYaxis()->SetTitle(gname);
    gvmem[i]->GetYaxis()->CenterTitle(gname);
    gvmem[i]->GetYaxis()->SetTitleSize(0.050);
    gvmem[i]->GetYaxis()->SetTitleFont(132);
    gvmem[i]->GetYaxis()->SetLabelSize(0.040);
    gvmem[i]->GetYaxis()->SetLabelFont(132);
    gvmem[i]->GetYaxis()->SetTitleOffset(0.650);

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

    sprintf(gname,"figures/003___vmem_ver%2.2d_ver%2.2d.png",VER_INIT+NVERS-1,VER_INIT+i);
    canvas->SaveAs(gname);
    //_____________________________________________________________________________________________

  }

  return 0;
}
