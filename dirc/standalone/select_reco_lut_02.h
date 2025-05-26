//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Dec 23 13:07:09 2021 by ROOT version 6.08/06
// from TTree dirc/dirc tree
// found on file: /home/gxproj7/volatile/RunPeriod-2019-11/recon/ver01_pass03/hd_root.root
//////////////////////////////////////////////////////////

#ifndef select_reco_lut_02_h
#define select_reco_lut_02_h

#include <iostream>
using namespace std;

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

// Header file for the classes stored in the TTree if any.
#include "TClonesArray.h"
#include "TObject.h"
#include "TVector3.h"

#include "DrcHit.h"

class select_reco_lut_02 : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Fixed size dimensions of array or collections stored in the TTree if any.
   const static int kMaxDrcEvent = 24;

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
   vector<DrcHit>  DrcEvent_fHitArray[kMaxDrcEvent];
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
   TBranch        *b_DrcEvent_fHitArray;
   TBranch        *b_DrcEvent_fMomentum;   //!
   TBranch        *b_DrcEvent_fPosition;   //!
   TBranch        *b_DrcEvent_fMomentum_Truth;   //!
   TBranch        *b_DrcEvent_fPosition_Truth;   //!
   TBranch        *b_DrcEvent_fTest1;   //!
   TBranch        *b_DrcEvent_fTest2;   //!

   select_reco_lut_02(TTree * /*tree*/ =0) : fChain(0) { }
   virtual ~select_reco_lut_02() { }
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

   ClassDef(select_reco_lut_02,0);
};

#endif

#ifdef select_reco_lut_02_cxx
void select_reco_lut_02::Init(TTree *tree)
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
   fChain->SetBranchAddress("DrcEvent.fHitArray", DrcEvent_fHitArray, &b_DrcEvent_fHitArray);
   //   t->SetBranchAddress("vpx",&vpx,&bvpx);
   fChain->SetBranchAddress("DrcEvent.fMomentum", DrcEvent_fMomentum, &b_DrcEvent_fMomentum);
   fChain->SetBranchAddress("DrcEvent.fPosition", DrcEvent_fPosition, &b_DrcEvent_fPosition);
   fChain->SetBranchAddress("DrcEvent.fMomentum_Truth", DrcEvent_fMomentum_Truth, &b_DrcEvent_fMomentum_Truth);
   fChain->SetBranchAddress("DrcEvent.fPosition_Truth", DrcEvent_fPosition_Truth, &b_DrcEvent_fPosition_Truth);
   fChain->SetBranchAddress("DrcEvent.fTest1", DrcEvent_fTest1, &b_DrcEvent_fTest1);
   fChain->SetBranchAddress("DrcEvent.fTest2", DrcEvent_fTest2, &b_DrcEvent_fTest2);
}

Bool_t select_reco_lut_02::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef select_reco_lut_02_cxx
