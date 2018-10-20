#define njobs_cxx
// The class definition in njobs.h has been generated automatically
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
// root> T->Process("njobs.C")
// root> T->Process("njobs.C","some options")
// root> T->Process("njobs.C+")
//


#include "njobs.h"
#include <TH2.h>
#include <TStyle.h>

void njobs::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();
	
   njobs_vs_time   = new TH1D("njobs_vs_time",   "", 5000E1, 0.0, 5000.0E3);
   nqueued_vs_time = new TH1D("nqueued_vs_time", "", 5000E1, 0.0, 5000.0E3);
}

void njobs::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   TString option = GetOption();

}

Bool_t njobs::Process(Long64_t entry)
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
	
   if( *tstart < *tsubmit ) *tstart = njobs_vs_time->GetXaxis()->GetXmax();
   if( *tend   < *tsubmit ) *tend   = njobs_vs_time->GetXaxis()->GetXmax();

   int ibin_submit = njobs_vs_time->FindBin(*tsubmit);
   int ibin_start = njobs_vs_time->FindBin(*tstart);
   int ibin_end   = njobs_vs_time->FindBin(*tend);
   if(ibin_submit<1) ibin_submit=1;
   if(ibin_start<1) ibin_start=1;
   if(ibin_end > njobs_vs_time->GetNbinsX()) ibin_end=njobs_vs_time->GetNbinsX();

	for(int i=ibin_submit; i<=ibin_start; i++) nqueued_vs_time->SetBinContent(i, nqueued_vs_time->GetBinContent(i) + 1.0);
	for(int i=ibin_start; i<=ibin_end; i++) njobs_vs_time->SetBinContent(i, njobs_vs_time->GetBinContent(i) + 1.0);

   return kTRUE;
}

void njobs::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void njobs::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

}
