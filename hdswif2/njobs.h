//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sun Aug 26 13:15:40 2018 by ROOT version 6.08/06
// from TTree slurminfo/SLURM Info.
// found on file: Memory Directory
//////////////////////////////////////////////////////////

#ifndef njobs_h
#define njobs_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TH1.h>

// Headers needed by this particular selector


class njobs : public TSelector {
public :

	int RunMin=0;
	int RunMax=999999;
	void SetRunRange(int RunMin, int RunMax){ this->RunMin = RunMin; this->RunMax = RunMax; }

   TTreeReader     fReader;  //!the tree reader
   TTree          *fChain = 0;   //!pointer to the analyzed TTree or TChain

   // Readers to access the data (delete the ones you do not need).
   TTreeReaderValue<Float_t> tsubmit = {fReader, "tsubmit"};
   TTreeReaderValue<Float_t> tstart = {fReader, "tstart"};
   TTreeReaderValue<Float_t> tend = {fReader, "tend"};
   TTreeReaderValue<Float_t> cpu = {fReader, "cpu"};
   TTreeReaderValue<Float_t> latency = {fReader, "latency"};
   TTreeReaderValue<Int_t> run = {fReader, "run"};
   TTreeReaderValue<Int_t> file = {fReader, "file"};

	TH1D *njobs_vs_time;
	TH1D *nqueued_vs_time;

   njobs(TTree * /*tree*/ =0) { }
   virtual ~njobs() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   ClassDef(njobs,0);

};

#endif

#ifdef njobs_cxx
void njobs::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the reader is initialized.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   fReader.SetTree(tree);
}

Bool_t njobs::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}


#endif // #ifdef njobs_cxx
