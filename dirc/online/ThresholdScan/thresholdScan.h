//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Feb  4 21:05:50 2019 by ROOT version 6.08/02
// from TTree T/DIRC THR Scan
// found on file: dirc_scan.root
//////////////////////////////////////////////////////////

#ifndef thresholdScan_h
#define thresholdScan_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Headers needed by this particular selector


class thresholdScan : public TSelector {
public :
   TTreeReader     fReader;  //!the tree reader
   TTree          *fChain = 0;   //!pointer to the analyzed TTree or TChain

   // Readers to access the data (delete the ones you do not need).
   TTreeReaderValue<Int_t> thr = {fReader, "thr"};
   TTreeReaderValue<Int_t> slot = {fReader, "slot"};
   TTreeReaderValue<Int_t> fiber = {fReader, "fiber"};
   TTreeReaderValue<Int_t> asic = {fReader, "asic"};
   TTreeReaderArray<Double_t> CHAN = {fReader, "CHAN"};
   TTreeReaderValue<Double_t> CHIP = {fReader, "CHIP"};

    TH2F *hScan[72][3];

   thresholdScan(TTree * /*tree*/ =0) { }
   virtual ~thresholdScan() { }
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

   ClassDef(thresholdScan,0);

};

#endif

#ifdef thresholdScan_cxx
void thresholdScan::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the reader is initialized.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   fReader.SetTree(tree);
}

Bool_t thresholdScan::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}


#endif // #ifdef thresholdScan_cxx
