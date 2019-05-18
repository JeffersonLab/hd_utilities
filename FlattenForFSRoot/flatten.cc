
#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TList.h"
#include "TMap.h"
#include "TIterator.h"


using namespace std;

  // note:  ran into strange memory problems when these were much larger (10000,100)?
static const int MAXCOMBOS = 1000;
static const int MAXPARTICLES = 50;

void ConvertFile(TString inFileName, TString outFileName);
void ConvertTree(TString treeName);

TFile* gInputFile;
TFile* gOutputFile;
float  gChi2DOFCut;


int main(int argc, char** argv){
  cout << endl;
  cout << "***********************************************************" << endl;
  cout << "This program converts trees from the standard TTree format" << endl;
  cout << "in the GlueX analysis code to a flat TTree (\"FS Format\")." << endl << endl;
  cout << "The final state is determined automatically from the input file." << endl << endl;
  cout << "Usage:" << endl;
  cout << "  flatten  <input file name> <output file name> [Chi2DOF cut value]" << endl << endl;
  cout << "Notes:" << endl;
  cout << "  * the output tree name is ntFSGlueX for now " << endl;
  cout << "  * the output tree name could also be derived from the input" << endl;
  cout << "     tree name (this is commented out)" << endl;
  cout << "  * the input tree name should contain \"_Tree\" (if this standard" << endl;
  cout << "     changes in the GlueX code, this code can be easily modified)" << endl;
  cout << "***********************************************************" << endl;
  if ((argc != 3) && (argc != 4)) exit(0);
  TString inFileName(argv[1]);
  TString outFileName(argv[2]);
  if (argc == 3) gChi2DOFCut = 1000.0;
  if (argc == 4) gChi2DOFCut = atof(argv[3]);
  ConvertFile(inFileName,outFileName);
}


TString FSParticleType(TString gluexParticleType){
  if (gluexParticleType.Contains("AntiLambda"))  return TString("ALambda");
  if (gluexParticleType.Contains("Lambda"))      return TString("Lambda");
  if (gluexParticleType.Contains("Positron"))    return TString("e+");
  if (gluexParticleType.Contains("Electron"))    return TString("e-");
  if (gluexParticleType.Contains("MuonPlus"))    return TString("mu+");
  if (gluexParticleType.Contains("MuonMinus"))   return TString("mu-");
  if (gluexParticleType.Contains("AntiProton"))  return TString("p-");
  if (gluexParticleType.Contains("Proton"))      return TString("p+");
  if (gluexParticleType.Contains("Eta"))         return TString("eta");
  if (gluexParticleType.Contains("Photon"))      return TString("gamma");
  if (gluexParticleType.Contains("KPlus"))       return TString("K+");
  if (gluexParticleType.Contains("KMinus"))      return TString("K-");
  if (gluexParticleType.Contains("KShort"))      return TString("Ks");
  if (gluexParticleType.Contains("PiPlus"))      return TString("pi+");
  if (gluexParticleType.Contains("PiMinus"))     return TString("pi-");
  if (gluexParticleType.Contains("Pi0"))         return TString("pi0");
  return TString("");
}


int FSParticleOrder(TString gluexParticleType){
  if (gluexParticleType.Contains("AntiLambda"))  return 15;
  if (gluexParticleType.Contains("Lambda"))      return 16;
  if (gluexParticleType.Contains("Positron"))    return 14;
  if (gluexParticleType.Contains("Electron"))    return 13;
  if (gluexParticleType.Contains("MuonPlus"))    return 12;
  if (gluexParticleType.Contains("MuonMinus"))   return 11;
  if (gluexParticleType.Contains("AntiProton"))  return 9;
  if (gluexParticleType.Contains("Proton"))      return 10;
  if (gluexParticleType.Contains("Eta"))         return 8;
  if (gluexParticleType.Contains("Photon"))      return 7;
  if (gluexParticleType.Contains("KPlus"))       return 6;
  if (gluexParticleType.Contains("KMinus"))      return 5;
  if (gluexParticleType.Contains("KShort"))      return 4;
  if (gluexParticleType.Contains("PiPlus"))      return 3;
  if (gluexParticleType.Contains("PiMinus"))     return 2;
  if (gluexParticleType.Contains("Pi0"))         return 1;
  return 0;
}

TString particleClass(TString gluexParticleType){
  if (gluexParticleType.Contains("AntiLambda"))  return TString("DecayingToCharged");
  if (gluexParticleType.Contains("Lambda"))      return TString("DecayingToCharged");
  if (gluexParticleType.Contains("Positron"))    return TString("Charged");
  if (gluexParticleType.Contains("Electron"))    return TString("Charged");
  if (gluexParticleType.Contains("MuonPlus"))    return TString("Charged");
  if (gluexParticleType.Contains("MuonMinus"))   return TString("Charged");
  if (gluexParticleType.Contains("AntiProton"))  return TString("Charged");
  if (gluexParticleType.Contains("Proton"))      return TString("Charged");
  if (gluexParticleType.Contains("Eta"))         return TString("DecayingToNeutral");
  if (gluexParticleType.Contains("Photon"))      return TString("Neutral");
  if (gluexParticleType.Contains("KPlus"))       return TString("Charged");
  if (gluexParticleType.Contains("KMinus"))      return TString("Charged");
  if (gluexParticleType.Contains("KShort"))      return TString("DecayingToCharged");
  if (gluexParticleType.Contains("PiPlus"))      return TString("Charged");
  if (gluexParticleType.Contains("PiMinus"))     return TString("Charged");
  if (gluexParticleType.Contains("Pi0"))         return TString("DecayingToNeutral");
  return TString("");
}


void ConvertFile(TString inFileName, TString outFileName){
  gInputFile  = new TFile(inFileName);
  gOutputFile = new TFile(outFileName,"recreate");
  TList* fileList = gInputFile->GetListOfKeys();
  for (int i = 0; i < fileList->GetEntries(); i++){
    TString treeName(fileList->At(i)->GetName());
    if (treeName.Contains("_Tree")){
      ConvertTree(treeName);
    }
  }
  gInputFile->Close();
  gOutputFile->Close();
}



void ConvertTree(TString treeName){

    // input and output tree names
  TString inNT(treeName);
  //TString outNT("nt");  outNT += inNT;  outNT.Replace(outNT.Index("_Tree"),5,"");
  TString outNT("ntFSGlueX");

    // setup for the input tree
  TTree *inTree = (TTree*) gInputFile->Get(inNT);


   // **********************************************************************
   // STEP 1:  PARSE FINAL STATE INFORMATION USING THE USER INFO IN THE TREE
   // **********************************************************************

     // **********************************************************************
     // STEP 1A:  simple checks on the input root file
     // **********************************************************************

  cout << endl << endl << "DOING SIMPLE CHECKS ON THE INPUT ROOT FILE:" << endl << endl;

  double inTargetCenterZ = -1.0;
  {
    TList* userInfo = inTree->GetUserInfo();
        if (userInfo){ cout << "  OK: found UserInfo" << endl; }
        else { cout << "  ERROR:  could not find UserInfo" << endl; exit(1); }
    TList* rootMothers = (TList*) userInfo->FindObject("ParticleNameList");
        if (rootMothers){ cout << "  OK: found ParticleNameList" << endl; }
        else { cout << "  ERROR:  could not find ParticleNameList" << endl; exit(1); }
    TMap* rootDecayProductMap = (TMap*) userInfo->FindObject("DecayProductMap");
        if (rootDecayProductMap){ cout << "  OK: found DecayProductMap" << endl; }
        else { cout << "  ERROR:  could not find DecayProductMap" << endl; exit(1); }
    TMap* miscInfo = (TMap*) userInfo->FindObject("MiscInfoMap");
        if (miscInfo){ cout << "  OK: found MiscInfoMap" << endl; }
        else { cout << "  ERROR:  could not find MiscInfoMap" << endl; exit(1); }
    TObjString* kinFitType = (TObjString*) miscInfo->GetValue("KinFitType");
        if (kinFitType->GetString() != "" && 
            kinFitType->GetString() != "0")
             { cout << "  OK: found KinFitType = "  << kinFitType->GetString() << endl; }
        else { cout << "  ERROR: bad KinFitType = " << kinFitType->GetString() << endl; exit(1); }
    TObjString* tosTCZ = (TObjString*) miscInfo->GetValue("Target__CenterZ");
        if (tosTCZ) 
             { cout << "  OK: found Target__CenterZ = "  << tosTCZ->GetString() << endl; }
        else { cout << "  ERROR: could not find Target__CenterZ " << endl; exit(1); }
    TString tsTCZ(tosTCZ->GetString());
        if (tsTCZ.IsFloat())
             { string sTCZ(""); for (int i=0; i<tsTCZ.Length(); i++){ sTCZ += tsTCZ[i]; }
               inTargetCenterZ = atof(sTCZ.c_str()); 
               cout << "            inTargetCenterZ = " << inTargetCenterZ << endl; }
        else { cout << "  ERROR: Target__CenterZ is not a number" << endl; exit(1); }
  }


     // **********************************************************************
     // STEP 1B:  extract particle names from the root file 
     // **********************************************************************

  cout << endl << endl << "READING PARTICLE NAMES FROM THE ROOT FILE:" << endl << endl;

  map< TString, vector<TString> > decayProductMap;  // from mothers to daughters

  {
    TList* userInfo = inTree->GetUserInfo();
    vector<TString> eraseVector; // (to remove double-counting)
    TList* rootMothers = (TList*) userInfo->FindObject("ParticleNameList");
    TMap* rootDecayProductMap = (TMap*) userInfo->FindObject("DecayProductMap");
    for (int i = 0; i < rootMothers->GetEntries(); i++){
      TObjString* rootMother = (TObjString*) rootMothers->At(i);
      cout << rootMother->GetString() << endl;
      TList* rootDaughters = (TList*) rootDecayProductMap->GetValue(rootMother->GetString());
      vector<TString> daughters;
      if (rootDaughters){
        for (int j = 0; j < rootDaughters->GetEntries(); j++){
          TObjString* rootDaughter = (TObjString*) rootDaughters->At(j);
          cout << "    " << rootDaughter->GetString() << endl;
          daughters.push_back(rootDaughter->GetString());
          eraseVector.push_back(rootDaughter->GetString());
        }
      }
      decayProductMap[rootMother->GetString()] = daughters;
    }
    for (unsigned int i = 0; i < eraseVector.size(); i++){
      decayProductMap.erase(eraseVector[i]);
    }
  }


     // **********************************************************************
     // STEP 1C:  put the particle names in the right order 
     // **********************************************************************

  cout << endl << endl << "PUTTING PARTICLE NAMES IN THE RIGHT ORDER:" << endl << endl;

  vector< vector<TString> > orderedParticleNames;

  {
    for (map<TString, vector<TString> >::const_iterator mItr = decayProductMap.begin();
         mItr != decayProductMap.end(); mItr++){
      if (FSParticleType(mItr->first) != ""){
        vector<TString> vp;
        vp.push_back(mItr->first);
        vector<TString> addp = mItr->second;
        if (addp.size() == 2){
          if (FSParticleOrder(addp[0]) < FSParticleOrder(addp[1])){
            vp.push_back(addp[1]);  vp.push_back(addp[0]);
          }
          else{
            vp.push_back(addp[0]);  vp.push_back(addp[1]);
          }
        }
        orderedParticleNames.push_back(vp);
      }
    }
    for (unsigned int i = 0; i < orderedParticleNames.size(); i++){
      for (unsigned int j = i + 1; j < orderedParticleNames.size(); j++){
        if (FSParticleOrder(orderedParticleNames[j][0]) >
            FSParticleOrder(orderedParticleNames[i][0])){
          vector<TString> temp = orderedParticleNames[i];
          orderedParticleNames[i] = orderedParticleNames[j];
          orderedParticleNames[j] = temp;
        }
      }
    }
  }


     // **********************************************************************
     // STEP 1D:  make maps from names to indices
     // **********************************************************************

  map<TString, TString> mapNameToFSIndex;
  map<TString, int> mapNameToParticleIndex;

  {
    int particleIndex = 0;
    for (unsigned int im = 0; im < orderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < orderedParticleNames[im].size(); id++){
      TString name = orderedParticleNames[im][id];
      TString fsIndex("");  fsIndex += (im+1);
      if (id == 1) fsIndex += "a";
      if (id == 2) fsIndex += "b";
      cout << fsIndex << ". ";
      cout << name << " ";
      mapNameToFSIndex[name] = fsIndex;
      mapNameToParticleIndex[name] = particleIndex++;
      cout << "(" << mapNameToParticleIndex[name] << ")   ";
    }
    cout << endl;
    }
    cout << endl << endl << endl;
  }

   // **********************************************************************
   // STEP 2:  SET UP TO READ THE INPUT TREE (IN ANALYSIS TREE FORMAT)
   // **********************************************************************

        // **************************************
        // ***** 2A. COMBO-INDEPENDENT DATA *****
        // **************************************

        //   *** Non-Particle Data ***

  UInt_t inRunNumber;        
      inTree->SetBranchAddress("RunNumber", &inRunNumber);
  ULong64_t inEventNumber;
      inTree->SetBranchAddress("EventNumber", &inEventNumber);
  TLorentzVector* inX4_Production;
      inTree->SetBranchAddress("X4_Production", &inX4_Production);
  UInt_t inNumBeam;
      inTree->SetBranchAddress("NumBeam", &inNumBeam);
  UInt_t inNumChargedHypos;
      inTree->SetBranchAddress("NumChargedHypos", &inNumChargedHypos);
  UInt_t inNumNeutralHypos; 
      inTree->SetBranchAddress("NumNeutralHypos", &inNumNeutralHypos);
  UInt_t inNumCombos;
      inTree->SetBranchAddress("NumCombos", &inNumCombos);

        //   *** Beam Particles (indexed using ComboBeam__BeamIndex) ***

  TClonesArray *inBeam__P4_Measured = new TClonesArray("TLorentzVector");
      inTree->SetBranchAddress("Beam__P4_Measured", &(inBeam__P4_Measured));
  TClonesArray *inBeam__X4_Measured = new TClonesArray("TLorentzVector");
      inTree->SetBranchAddress("Beam__X4_Measured", &(inBeam__X4_Measured));


        //   *** Charged Track Hypotheses (indexed using <particleName>__ChargedIndex) ***

  TClonesArray *inChargedHypo__P4_Measured = new TClonesArray("TLorentzVector");
      inTree->SetBranchAddress("ChargedHypo__P4_Measured",&(inChargedHypo__P4_Measured));
  Float_t inChargedHypo__ChiSq_Tracking[MAXPARTICLES] = {}; 
      inTree->SetBranchAddress("ChargedHypo__ChiSq_Tracking", inChargedHypo__ChiSq_Tracking);
  UInt_t  inChargedHypo__NDF_Tracking[MAXPARTICLES] = {};   
      inTree->SetBranchAddress("ChargedHypo__NDF_Tracking", inChargedHypo__NDF_Tracking);

        //   *** Neutral Particle Hypotheses (indexed using <particleName>__NeutralIndex) ***

  TClonesArray *inNeutralHypo__P4_Measured = new TClonesArray("TLorentzVector");
      inTree->SetBranchAddress("NeutralHypo__P4_Measured",&(inNeutralHypo__P4_Measured));
  
  Float_t inShower_Quality[MAXPARTICLES] = {};
      inTree->SetBranchAddress("NeutralHypo__ShowerQuality", inShower_Quality);

        // ************************************
        // ***** 2B. COMBO-DEPENDENT DATA *****
        // ************************************

        //   *** Particle-Independent Data (indexed by combo) ***

  Float_t inRFTime_Measured[MAXCOMBOS] = {};
      inTree->SetBranchAddress("RFTime_Measured", inRFTime_Measured);  
  Float_t inRFTime_KinFit[MAXCOMBOS] = {};
      inTree->SetBranchAddress("RFTime_KinFit", inRFTime_KinFit);  
  Float_t inChiSq_KinFit[MAXCOMBOS] = {};
      inTree->SetBranchAddress("ChiSq_KinFit", inChiSq_KinFit);
  UInt_t inNDF_KinFit[MAXCOMBOS] = {};
      inTree->SetBranchAddress("NDF_KinFit", inNDF_KinFit);
  Float_t inEnergy_Unused[MAXCOMBOS] = {};
      inTree->SetBranchAddress("Energy_UnusedShowers", inEnergy_Unused);


        //   *** Combo Beam Particles (indexed by combo) ***

  Int_t inBeamIndex[MAXCOMBOS] = {};
      inTree->SetBranchAddress("ComboBeam__BeamIndex", inBeamIndex);
  TClonesArray *inBeam__P4_KinFit = new TClonesArray("TLorentzVector");
      inTree->SetBranchAddress("ComboBeam__P4_KinFit", &(inBeam__P4_KinFit));


        //   *** Combo Tracks ***
        //   *** Combo Neutrals ***
        //   *** Combo Decaying Particles ***
        //     (all indexed by particleIndex and combo)

  TClonesArray *inP4_KinFit[MAXPARTICLES] = {}; 
  Int_t inChargedIndex[MAXPARTICLES][MAXCOMBOS] = {};
  Int_t inNeutralIndex[MAXPARTICLES][MAXCOMBOS] = {};
  {
    for (unsigned int im = 0; im < orderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < orderedParticleNames[im].size(); id++){
      TString name = orderedParticleNames[im][id];
      int pIndex = mapNameToParticleIndex[name];

        //   *** Combo Tracks ***

      if (particleClass(name) == "Charged"){
        TString var_P4_KinFit(name); var_P4_KinFit += "__P4_KinFit";
            inP4_KinFit[pIndex] = new TClonesArray("TLorentzVector");
            inTree->SetBranchAddress(var_P4_KinFit,&(inP4_KinFit[pIndex]));
        TString var_ChargedIndex(name);  var_ChargedIndex += "__ChargedIndex";  
            inTree->SetBranchAddress(var_ChargedIndex,inChargedIndex[pIndex]);
      }

        //   *** Combo Neutrals ***

      if (particleClass(name) == "Neutral"){
        TString var_P4_KinFit(name); var_P4_KinFit += "__P4_KinFit";
            inP4_KinFit[pIndex] = new TClonesArray("TLorentzVector");
            inTree->SetBranchAddress(var_P4_KinFit,&(inP4_KinFit[pIndex]));
        TString var_NeutralIndex(name);  var_NeutralIndex += "__NeutralIndex";  
            inTree->SetBranchAddress(var_NeutralIndex,inNeutralIndex[pIndex]);
      }

        //   *** Combo Decaying Particles ***

      if (particleClass(name).Contains("Decaying")){
      }

    }
    }
  }


   // **********************************************************************
   // STEP 3:  SET UP THE OUTPUT TREE INFORMATION (IN FSROOT FORMAT)
   // **********************************************************************

  gOutputFile->cd();
  TTree outTree(outNT, outNT);

    // non-particle information

  float outRunNumber;     outTree.Branch("Run",        &outRunNumber,  "Run/F");
  float outEventNumber;   outTree.Branch("Event",      &outEventNumber,"Event/F");
  float outChi2;          outTree.Branch("Chi2",       &outChi2,       "Chi2/F");
  float outChi2DOF;       outTree.Branch("Chi2DOF",    &outChi2DOF,    "Chi2DOF/F");
  float outRFTime;        outTree.Branch("RFTime",     &outRFTime,     "RFTime/F");
  float outRFDeltaT;      outTree.Branch("RFDeltaT",   &outRFDeltaT,   "RFDeltaT/F");
  float outEnUnusedSh;    outTree.Branch("EnUnusedSh", &outEnUnusedSh, "EnUnusedSh/F");
  float outProdVx;        outTree.Branch("ProdVx",     &outProdVx,     "ProdVx/F");
  float outProdVy;        outTree.Branch("ProdVy",     &outProdVy,     "ProdVy/F");
  float outProdVz;        outTree.Branch("ProdVz",     &outProdVz,     "ProdVz/F");
  float outProdVt;        outTree.Branch("ProdVt",     &outProdVt,     "ProdVt/F");
  float outPxPB;          outTree.Branch("PxPB",       &outPxPB,       "PxPB/F");
  float outPyPB;          outTree.Branch("PyPB",       &outPyPB,       "PyPB/F");
  float outPzPB;          outTree.Branch("PzPB",       &outPzPB,       "PzPB/F");
  float outEnPB;          outTree.Branch("EnPB",       &outEnPB,       "EnPB/F");
  float outRPxPB;         outTree.Branch("RPxPB",      &outRPxPB,      "RPxPB/F");
  float outRPyPB;         outTree.Branch("RPyPB",      &outRPyPB,      "RPyPB/F");
  float outRPzPB;         outTree.Branch("RPzPB",      &outRPzPB,      "RPzPB/F");
  float outREnPB;         outTree.Branch("REnPB",      &outREnPB,      "REnPB/F");

    // particle information

  float  outPx[MAXPARTICLES]={},  outPy[MAXPARTICLES]={},  outPz[MAXPARTICLES]={},  outEn[MAXPARTICLES]={};
  float outRPx[MAXPARTICLES]={}, outRPy[MAXPARTICLES]={}, outRPz[MAXPARTICLES]={}, outREn[MAXPARTICLES]={};
  float outTkChi2[MAXPARTICLES]={}, outTkNDF[MAXPARTICLES]={};
  float outQuality[MAXPARTICLES]={};
  {
    for (unsigned int im = 0; im < orderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < orderedParticleNames[im].size(); id++){
      TString name = orderedParticleNames[im][id];
      int pIndex = mapNameToParticleIndex[name];
      TString fsIndex = mapNameToFSIndex[name];
      TString vPx("PxP"); vPx += fsIndex; outTree.Branch(vPx,&outPx[pIndex],vPx+"/F");
      TString vPy("PyP"); vPy += fsIndex; outTree.Branch(vPy,&outPy[pIndex],vPy+"/F");
      TString vPz("PzP"); vPz += fsIndex; outTree.Branch(vPz,&outPz[pIndex],vPz+"/F");
      TString vEn("EnP"); vEn += fsIndex; outTree.Branch(vEn,&outEn[pIndex],vEn+"/F");
      TString vRPx("RPxP"); vRPx += fsIndex; outTree.Branch(vRPx,&outRPx[pIndex],vRPx+"/F");
      TString vRPy("RPyP"); vRPy += fsIndex; outTree.Branch(vRPy,&outRPy[pIndex],vRPy+"/F");
      TString vRPz("RPzP"); vRPz += fsIndex; outTree.Branch(vRPz,&outRPz[pIndex],vRPz+"/F");
      TString vREn("REnP"); vREn += fsIndex; outTree.Branch(vREn,&outREn[pIndex],vREn+"/F");
      TString vQual("Quality"); vQual += fsIndex; outTree.Branch(vQual, &outQuality[pIndex], vQual+"/F");
      
      if (particleClass(name) == "Charged"){
        TString vTkNDF("TkNDFP"); vTkNDF += fsIndex;
            outTree.Branch(vTkNDF,&outTkNDF[pIndex],vTkNDF+"/F");
        TString vTkChi2("TkChi2P"); vTkChi2 += fsIndex;
            outTree.Branch(vTkChi2,&outTkChi2[pIndex],vTkChi2+"/F");
      }
    }
    }
  }



   // **********************************************************************
   // STEP 4:  DO THE CONVERSION
   // **********************************************************************

  cout << "STARTING THE CONVERSION... " << endl << endl;


    // loop over the input tree

  Long64_t nEntries = inTree->GetEntries();
  for (Long64_t iEntry = 0; iEntry < nEntries; iEntry++){
    if ((iEntry+1) % 10000 == 0) cout << "entry = " << iEntry+1 << endl;
    inTree->GetEntry(iEntry);
    if (inNumCombos > MAXCOMBOS){
      cout << "ERROR:  Too many combos (" << inNumCombos << ")!" << endl;
      exit(0);
    }
    if (inNumChargedHypos + inNumNeutralHypos > MAXPARTICLES){
      cout << "ERROR:  Too many hypotheses!" << endl;
      cout << "   NumChargedHypos = " << inNumChargedHypos << endl;
      cout << "   NumNeutralHypos = " << inNumNeutralHypos << endl;
      exit(0);
    }


      // loop over combos

    for (UInt_t ic = 0; ic < inNumCombos; ic++){

        // non-particle information

      TLorentzVector *p4, *p4a, *p4b, *x4;
      outRunNumber   = inRunNumber;
      outEventNumber = inEventNumber;
      outChi2        = inChiSq_KinFit[ic];
      outChi2DOF     = -1; if (inNDF_KinFit[ic]>0.0) outChi2DOF = outChi2/inNDF_KinFit[ic];
      outRFTime      = inRFTime_KinFit[ic];
              //   line from jon z. for timing info:
              //  Double_t rf_timing = locBeamX4_Measured.T() - (dComboWrapper->Get_RFTime_Measured() 
              //     + (locBeamX4_Measured.Z()- dComboWrapper->Get_TargetCenter().Z())/29.9792458 );
          x4 = (TLorentzVector*)inBeam__X4_Measured->At(inBeamIndex[ic]);
          double tB  = x4->T();
          double tRF = inRFTime_Measured[ic];
          double zB  = x4->Z();
          double zT  = inTargetCenterZ;
      outRFDeltaT    = tB - (tRF + (zB - zT)/29.9792458);
      outEnUnusedSh  = inEnergy_Unused[ic];
      outProdVx      = inX4_Production->X();
      outProdVy      = inX4_Production->Y();
      outProdVz      = inX4_Production->Z();
      outProdVt      = inX4_Production->T();
          p4 = (TLorentzVector*)inBeam__P4_KinFit->At(ic);
      outPxPB = p4->Px();
      outPyPB = p4->Py();
      outPzPB = p4->Pz();
      outEnPB = p4->E();
          p4 = (TLorentzVector*)inBeam__P4_Measured->At(inBeamIndex[ic]);
      outRPxPB = p4->Px();
      outRPyPB = p4->Py();
      outRPzPB = p4->Pz();
      outREnPB = p4->E();

        // particle information

      for (unsigned int im = 0; im < orderedParticleNames.size(); im++){
      for (unsigned int id = 0; id < orderedParticleNames[im].size(); id++){
        TString name = orderedParticleNames[im][id];
        int pIndex = mapNameToParticleIndex[name];

          // charged tracks

        if (particleClass(name) == "Charged"){ 
          p4 = (TLorentzVector*)inP4_KinFit[pIndex]->At(ic);
            outPx[pIndex] = p4->Px();
            outPy[pIndex] = p4->Py();
            outPz[pIndex] = p4->Pz();
            outEn[pIndex] = p4->E();
          p4 = (TLorentzVector*)inChargedHypo__P4_Measured->At(inChargedIndex[pIndex][ic]);
            outRPx[pIndex] = p4->Px();
            outRPy[pIndex] = p4->Py();
            outRPz[pIndex] = p4->Pz();
            outREn[pIndex] = p4->E();
          outTkNDF [pIndex] = inChargedHypo__NDF_Tracking  [(inChargedIndex[pIndex][ic])];
          outTkChi2[pIndex] = inChargedHypo__ChiSq_Tracking[(inChargedIndex[pIndex][ic])];
        }

          // neutral particles

        if (particleClass(name) == "Neutral"){ 
          p4 = (TLorentzVector*)inP4_KinFit[pIndex]->At(ic);
            outPx[pIndex] = p4->Px();
            outPy[pIndex] = p4->Py();
            outPz[pIndex] = p4->Pz();
            outEn[pIndex] = p4->E();
          p4 = (TLorentzVector*)inNeutralHypo__P4_Measured->At(inNeutralIndex[pIndex][ic]);
            outRPx[pIndex] = p4->Px();
            outRPy[pIndex] = p4->Py();
            outRPz[pIndex] = p4->Pz();
            outREn[pIndex] = p4->E();
        
            outQuality[pIndex] = inShower_Quality[pIndex];
        }

          // decaying to charged tracks

        if (particleClass(name) == "DecayingToCharged"){ 
          int pIndex1 = mapNameToParticleIndex[orderedParticleNames[im][1]];
          int pIndex2 = mapNameToParticleIndex[orderedParticleNames[im][2]];
          p4a = (TLorentzVector*)inP4_KinFit[pIndex1]->At(ic);
          p4b = (TLorentzVector*)inP4_KinFit[pIndex2]->At(ic);
            outPx[pIndex] = p4a->Px() + p4b->Px();
            outPy[pIndex] = p4a->Py() + p4b->Py();
            outPz[pIndex] = p4a->Pz() + p4b->Pz();
            outEn[pIndex] = p4a->E()  + p4b->E();
          p4a = (TLorentzVector*)inChargedHypo__P4_Measured->At(inChargedIndex[pIndex1][ic]);
          p4b = (TLorentzVector*)inChargedHypo__P4_Measured->At(inChargedIndex[pIndex2][ic]);
            outRPx[pIndex] = p4a->Px() + p4b->Px();
            outRPy[pIndex] = p4a->Py() + p4b->Py();
            outRPz[pIndex] = p4a->Pz() + p4b->Pz();
            outREn[pIndex] = p4a->E()  + p4b->E();
        }

          // decaying to neutral particles

        if (particleClass(name) == "DecayingToNeutral"){ 
          int pIndex1 = mapNameToParticleIndex[orderedParticleNames[im][1]];
          int pIndex2 = mapNameToParticleIndex[orderedParticleNames[im][2]];
          p4a = (TLorentzVector*)inP4_KinFit[pIndex1]->At(ic);
          p4b = (TLorentzVector*)inP4_KinFit[pIndex2]->At(ic);
            outPx[pIndex] = p4a->Px() + p4b->Px();
            outPy[pIndex] = p4a->Py() + p4b->Py();
            outPz[pIndex] = p4a->Pz() + p4b->Pz();
            outEn[pIndex] = p4a->E()  + p4b->E();
          p4a = (TLorentzVector*)inNeutralHypo__P4_Measured->At(inNeutralIndex[pIndex1][ic]);
          p4b = (TLorentzVector*)inNeutralHypo__P4_Measured->At(inNeutralIndex[pIndex2][ic]);
            outRPx[pIndex] = p4a->Px() + p4b->Px();
            outRPy[pIndex] = p4a->Py() + p4b->Py();
            outRPz[pIndex] = p4a->Pz() + p4b->Pz();
            outREn[pIndex] = p4a->E()  + p4b->E();
        }

      }}

      // print some information (for debugging only)

      if (iEntry+1 == 1 && ic+1 == 1){ 
        cout << endl << "PRINTING TEST INFORMATION FOR FIVE EVENTS..." << endl << endl;
      }
      if (iEntry < 5){
        cout << "EVENT: " << inEventNumber << " (combo no. " << ic+1 << ")" << endl;
        cout << "  NumChargedHypos = " << inNumChargedHypos << endl;
        cout << "  NumNeutralHypos = " << inNumNeutralHypos << endl;
        cout << "  NumBeam   = " << inNumBeam << endl;
        cout << "  NumCombos = " << inNumCombos << endl;
        cout << "  ProdVertex = " << inX4_Production->X() << " " 
                                  << inX4_Production->Y() << " " 
                                  << inX4_Production->Z() << " " 
                                  << inX4_Production->T() << endl; 
        for (unsigned int im = 0; im < orderedParticleNames.size(); im++){
        for (unsigned int id = 0; id < orderedParticleNames[im].size(); id++){
          TString name = orderedParticleNames[im][id];
          int pIndex = mapNameToParticleIndex[name];
          TString fsIndex = mapNameToFSIndex[name];
          double px = outPx[pIndex];
          double py = outPy[pIndex];
          double pz = outPz[pIndex];
          double en = outEn[pIndex];
          double mass = sqrt(fabs(en*en - px*px - py*py - pz*pz));
          double rpx = outRPx[pIndex];
          double rpy = outRPy[pIndex];
          double rpz = outRPz[pIndex];
          double ren = outREn[pIndex];
          double rmass = sqrt(fabs(ren*ren - rpx*rpx - rpy*rpy - rpz*rpz));
          cout << "    " << fsIndex << ".  " << name << " (" << pIndex << ")  " 
               << mass << "  " << rmass << endl;
        }}
      }
      if (iEntry+1 == 5 && ic+1 == inNumCombos){ 
        cout << endl << endl << "DONE PRINTING TEST INFORMATION FOR FIVE EVENTS" << endl << endl;
        cout << "CONTINUING THE CONVERSION... " << endl << endl;
      }

        // make cuts

      if (outChi2DOF > gChi2DOFCut) continue;

        // fill the tree

      outTree.Fill();

    }

  }

    // write output information

  cout << endl << endl << "WRITING THE OUTPUT TREE..." << endl;
  gOutputFile->cd();
  outTree.Write();
  cout << "FINISHED" << endl << endl;

}

