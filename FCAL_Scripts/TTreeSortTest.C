#include "RootMacros/FCALUtilities/StringUtilities.h"
#include "RootMacros/FCALUtilities/GeneralUtilities.h"
#include "RootMacros/FCALUtilities/PlotUtilities.h"
#include "RootMacros/FCALUtilities/DAQUtilities.h"
#include "RootMacros/FCALUtilities/MonitoringUtilities.h"
#include "TTreeIndex.h"
#include "TSystem.h"

void sortTTree(TString fileName){
  TFile* f = new TFile(fileName);
  TTree* tr = (TTree*)f->Get("Df250WindowRawDataPedestal");
  
  uint32_t eventnum;           /// Event number
  uint32_t rocid;              /// Crate number
  uint32_t slot;               /// Slot number in crate
  uint32_t channel;            /// Channel number in slot
  uint32_t channelnum;
  Float_t pedestal;             /// Pedestal, Need to Ask Mark How this is calculated
  std::vector<uint32_t>* ped_vec = 0;
  
  tr->SetBranchAddress("eventnum",&eventnum);
  tr->SetBranchAddress("rocid",&rocid);
  tr->SetBranchAddress("slot",&slot);
  tr->SetBranchAddress("channel",&channel);
  tr->SetBranchAddress("channelnum",&channelnum);
  tr->SetBranchAddress("w_ped",&pedestal);
  tr->SetBranchAddress("w_ped_vec",&ped_vec);
  
  
  tr->BuildIndex("channelnum","eventnum");
  TTreeIndex* index = (TTreeIndex*) tr->GetTreeIndex();
  
  for (int i=0; i<index->GetN(); i++ ) {
    Long64_t local = tr->LoadTree( index->GetIndex()[i] );  
    tr->GetEntry(local);
    cout << local << "\t" << rocid << "/" << slot << "/" << channel << "\t" << eventnum << endl; 
  }
  
  
}
