//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Dec 21 13:46:08 2021 by ROOT version 6.08/06
// from TTree dirc/dirc tree
// found on file: /home/gxproj7/volatile/RunPeriod-2019-11/recon/ver01_pass03/hd_root.root
//////////////////////////////////////////////////////////

#ifndef test_h
#define test_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "TClonesArray.h"
#include "TObject.h"

class test {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.
   const Int_t kMaxDrcEvent = 24;

   // Declaration of leaf types
   Int_t           DrcEvent_;
   UInt_t          DrcEvent_fUniqueID[kMaxDrcEvent];   //[DrcEvent_]
   UInt_t          DrcEvent_fBits[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fId[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fType[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fPdg[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fParent[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fDcHits[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fTime[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fInvMass[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fMissMass[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fChiSq[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fTofTrackDist[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fTofTrackDeltaT[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fId_Truth[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fTime_Truth[kMaxDrcEvent];   //[DrcEvent_]
   Int_t           DrcEvent_fHitSize[kMaxDrcEvent];   //[DrcEvent_]
 //vector<DrcHit>  DrcEvent_fHitArray[kMaxDrcEvent];
   TVector3        DrcEvent_fMomentum[kMaxDrcEvent];
   TVector3        DrcEvent_fPosition[kMaxDrcEvent];
   TVector3        DrcEvent_fMomentum_Truth[kMaxDrcEvent];
   TVector3        DrcEvent_fPosition_Truth[kMaxDrcEvent];
   Double_t        DrcEvent_fTest1[kMaxDrcEvent];   //[DrcEvent_]
   Double_t        DrcEvent_fTest2[kMaxDrcEvent];   //[DrcEvent_]

   // List of branches
   TBranch        *b_DrcEvent_;   //!
   TBranch        *b_DrcEvent_fUniqueID;   //!
   TBranch        *b_DrcEvent_fBits;   //!
   TBranch        *b_DrcEvent_fId;   //!
   TBranch        *b_DrcEvent_fType;   //!
   TBranch        *b_DrcEvent_fPdg;   //!
   TBranch        *b_DrcEvent_fParent;   //!
   TBranch        *b_DrcEvent_fDcHits;   //!
   TBranch        *b_DrcEvent_fTime;   //!
   TBranch        *b_DrcEvent_fInvMass;   //!
   TBranch        *b_DrcEvent_fMissMass;   //!
   TBranch        *b_DrcEvent_fChiSq;   //!
   TBranch        *b_DrcEvent_fTofTrackDist;   //!
   TBranch        *b_DrcEvent_fTofTrackDeltaT;   //!
   TBranch        *b_DrcEvent_fId_Truth;   //!
   TBranch        *b_DrcEvent_fTime_Truth;   //!
   TBranch        *b_DrcEvent_fHitSize;   //!
   TBranch        *b_DrcEvent_fMomentum;   //!
   TBranch        *b_DrcEvent_fPosition;   //!
   TBranch        *b_DrcEvent_fMomentum_Truth;   //!
   TBranch        *b_DrcEvent_fPosition_Truth;   //!
   TBranch        *b_DrcEvent_fTest1;   //!
   TBranch        *b_DrcEvent_fTest2;   //!

   test(TTree *tree=0);
   virtual ~test();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef test_cxx
test::test(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/home/gxproj7/volatile/RunPeriod-2019-11/recon/ver01_pass03/hd_root.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/home/gxproj7/volatile/RunPeriod-2019-11/recon/ver01_pass03/hd_root.root");
      }
      f->GetObject("dirc",tree);

   }
   Init(tree);
}

test::~test()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t test::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t test::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void test::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("DrcEvent", &DrcEvent_, &b_DrcEvent_);
   fChain->SetBranchAddress("DrcEvent.fUniqueID", DrcEvent_fUniqueID, &b_DrcEvent_fUniqueID);
   fChain->SetBranchAddress("DrcEvent.fBits", DrcEvent_fBits, &b_DrcEvent_fBits);
   fChain->SetBranchAddress("DrcEvent.fId", DrcEvent_fId, &b_DrcEvent_fId);
   fChain->SetBranchAddress("DrcEvent.fType", DrcEvent_fType, &b_DrcEvent_fType);
   fChain->SetBranchAddress("DrcEvent.fPdg", DrcEvent_fPdg, &b_DrcEvent_fPdg);
   fChain->SetBranchAddress("DrcEvent.fParent", DrcEvent_fParent, &b_DrcEvent_fParent);
   fChain->SetBranchAddress("DrcEvent.fDcHits", DrcEvent_fDcHits, &b_DrcEvent_fDcHits);
   fChain->SetBranchAddress("DrcEvent.fTime", DrcEvent_fTime, &b_DrcEvent_fTime);
   fChain->SetBranchAddress("DrcEvent.fInvMass", DrcEvent_fInvMass, &b_DrcEvent_fInvMass);
   fChain->SetBranchAddress("DrcEvent.fMissMass", DrcEvent_fMissMass, &b_DrcEvent_fMissMass);
   fChain->SetBranchAddress("DrcEvent.fChiSq", DrcEvent_fChiSq, &b_DrcEvent_fChiSq);
   fChain->SetBranchAddress("DrcEvent.fTofTrackDist", DrcEvent_fTofTrackDist, &b_DrcEvent_fTofTrackDist);
   fChain->SetBranchAddress("DrcEvent.fTofTrackDeltaT", DrcEvent_fTofTrackDeltaT, &b_DrcEvent_fTofTrackDeltaT);
   fChain->SetBranchAddress("DrcEvent.fId_Truth", DrcEvent_fId_Truth, &b_DrcEvent_fId_Truth);
   fChain->SetBranchAddress("DrcEvent.fTime_Truth", DrcEvent_fTime_Truth, &b_DrcEvent_fTime_Truth);
   fChain->SetBranchAddress("DrcEvent.fHitSize", DrcEvent_fHitSize, &b_DrcEvent_fHitSize);
   fChain->SetBranchAddress("DrcEvent.fMomentum", DrcEvent_fMomentum, &b_DrcEvent_fMomentum);
   fChain->SetBranchAddress("DrcEvent.fPosition", DrcEvent_fPosition, &b_DrcEvent_fPosition);
   fChain->SetBranchAddress("DrcEvent.fMomentum_Truth", DrcEvent_fMomentum_Truth, &b_DrcEvent_fMomentum_Truth);
   fChain->SetBranchAddress("DrcEvent.fPosition_Truth", DrcEvent_fPosition_Truth, &b_DrcEvent_fPosition_Truth);
   fChain->SetBranchAddress("DrcEvent.fTest1", DrcEvent_fTest1, &b_DrcEvent_fTest1);
   fChain->SetBranchAddress("DrcEvent.fTest2", DrcEvent_fTest2, &b_DrcEvent_fTest2);
   Notify();
}

Bool_t test::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void test::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t test::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef test_cxx
