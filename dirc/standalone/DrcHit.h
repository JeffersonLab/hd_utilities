// -----------------------------------------
// DrcHit.h
// created on: 07.04.2017
// initial author: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DrcHit_h
#define DrcHit_h 1

#include <vector>

#include "TObject.h"
#include "TVector3.h"

class DrcHit : public TObject {

public:   
 
  //Constructor
  DrcHit();

  ~DrcHit(){};
 
  // Accessors 
  Int_t GetType()        { return fType; }
  Int_t GetNreflections()  { return fNreflections; }
  Long64_t GetPathId()  { return fPathId; }
  //  TVector3 GetMomentum()     { return fMomentum; }
  Double_t GetEnergy()       { return fEnergy; }
  TVector3 GetPosition()     { return fPosition; }
  Double_t GetCherenkovMC()  { return fCherenkovMC;}
  
  Int_t GetPmtId()       { return fPmtId; }
  Int_t GetPixelId()     { return fPixelId; }
  Int_t GetChannel() { return fChannel;}
  Double_t GetLeadTime() { return fLeadTime; }
  Double_t GetTotTime() { return fTotTime; }
    
  // Mutators
  void SetType(Int_t val)    { fType = val; }
  void SetNreflections(Int_t val)  { fNreflections = val; }
  void SetPathId(Long64_t val) { fPathId = val; }
  //  void SetMomentum(TVector3 val)    { fMomentum = val; }
  void SetEnergy(Double_t val)    { fEnergy = val; }
  void SetPosition(TVector3 val)    { fPosition = val; }
  void SetCherenkovMC(Double_t val) { fCherenkovMC = val; }
  
  void SetPmtId(Int_t val)   { fPmtId = val; }
  void SetPixelId(Int_t val) { fPixelId = val; }
  void SetChannel(Int_t val) { fChannel=val; }
  void SetLeadTime(Double_t val) { fLeadTime=val; }
  void SetTotTime(Double_t val) { fTotTime=val; } 

protected:

  Int_t fType;
  Int_t fNreflections;
  Long64_t fPathId;
  //  TVector3 fMomentum;
  Double_t fEnergy;
  TVector3 fPosition;
  Double_t fCherenkovMC;
  
  Int_t fPmtId;
  Int_t fPixelId;
  Int_t fChannel;
  Double_t fLeadTime;
  Double_t fTotTime;

  ClassDef(DrcHit,3)
};

#endif

// #if defined(__ROOTCLING__)
// #pragma link C++ class DrcHit+;
// #endif
