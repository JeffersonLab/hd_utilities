#define thresholdScan_cxx
// The class definition in thresholdScan.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.


// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// root> T->Process("thresholdScan.C")
// root> T->Process("thresholdScan.C","some options")
// root> T->Process("thresholdScan.C+")
//


#include "thresholdScan.h"
#include <TH2.h>
#include <TStyle.h>

void thresholdScan::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
    
   for(int i=0; i<72; i++) {
       for(int j=0; j<3; j++) {
           hScan[i][j] = new TH2F(Form("hScan_%d_%d",i,j),Form("hScan_%d_%d",i,j),64,0, 64,100,150,250);
           
       }
   }
}

void thresholdScan::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

Bool_t thresholdScan::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // When processing keyed objects with PROOF, the object is already loaded
   // and is available via the fObject pointer.
   //
   // This function should contain the \"body\" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   fReader.SetEntry(entry);
    
    int locFiber = (*slot)*24 + *fiber;
    cout<<"threshold = "<<*thr<<" fiber = "<<*fiber<<" slot = "<<*slot<<" global fiber = "<<locFiber<<" ASIC = "<<*asic<<endl;
    
    for(int i=0; i<64; i++) {
	//cout<<locFiber<<" "<<i<<endl;
        hScan[locFiber][*asic]->Fill(i,*thr,CHAN[i]);
    }

   return kTRUE;
}

void thresholdScan::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void thresholdScan::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
    
    TFile *fout = new TFile("scanHist.root", "recreate");
    
    for(int i=0; i<72; i++) {
        for(int j=0; j<3; j++) {
            hScan[i][j]->Write();
        }
    }
    
    fout->Close();
}
