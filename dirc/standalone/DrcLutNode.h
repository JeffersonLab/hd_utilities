// -----------------------------------------
// DrcLutNode.h - Container for look-up table
//
// created on: 29.11.2017
// initial athor: r.dzhygadlo at gsi.de

#ifndef DrcLutNode_h
#define DrcLutNode_h 1

#include "TObject.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include <vector>
#include <iostream>

class DrcLutNode : public TObject {

public:    
  
  // Default constructor
  DrcLutNode ();

  // Standard constructors
  DrcLutNode (Int_t detectorId);

  // Copy constructor 
  //DrcLutNode (DrcLutNode& node) { *this = node; }  

  // Modifiers
  void AddEntry(Int_t lutId, Int_t nodeId, TVector3 dir, Long64_t pathid, Int_t nrefl, Double_t time, TVector3 hitpos, TVector3 digipos,  Double_t weight=1);
  void AddEntry(Int_t lutId, Int_t nodeId, TVector3 dir, Long64_t pathid, Int_t nrefl, Double_t time, TVector3 hitpos, TVector3 digipos,  Double_t weight,
		TVector3 d1,TVector3 d2,TVector3 d3,TVector3 d4,TVector3 d5,TVector3 d6,TVector3 d7,TVector3 d8);
  void SetDigiPos(TVector3 pos){fDigiPos = pos;}

  // Accessors
  Int_t Entries() { return fSize; }
  Double_t GetLutId() { return fLutId; }
  Double_t GetDetectorId() { return fDetectorId; }
  TVector3 GetEntry(Int_t entry) { return fNodeArray[entry]; }
  //TVector3 GetEntryCs(Int_t entry, Int_t side) { return fNodeArrayCs[side][entry]; }
  Long64_t GetPathId(Int_t entry){ return fPathIdArray[entry]; }
  Double_t GetWeight(Int_t entry){ return fWeightArray[entry]; }
  Int_t GetNRefl(Int_t entry){ return fNReflArray[entry]; }
  Double_t GetTime(Int_t entry){ return fTimeArray[entry]; }
  TVector3 GetHitPos(Int_t entry){ return fHitPos[entry]; }
  TVector3 GetDigiPos(){ return fDigiPos; }

protected:

  Int_t fLutId;
  Int_t fDetectorId;
  Int_t fSize;
  TVector3 fDigiPos;

  std::vector<TVector3> fNodeArray;
  //std::vector<TVector3> fNodeArrayCs[9];
  std::vector<TVector3> fHitPos;
  std::vector<Long64_t> fPathIdArray;
  std::vector<Double_t> fWeightArray;
  std::vector<Int_t> fNReflArray;
  std::vector<Double_t> fTimeArray;

protected: 
  ClassDef(DrcLutNode, 1);
  
};

#endif
