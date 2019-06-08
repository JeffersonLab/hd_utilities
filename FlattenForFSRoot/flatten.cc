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


  // maximum array indices
  // [careful:  there were strange memory problems when these were set to 10000 and 100]
static const int MAXCOMBOS = 1000;
static const int MAXPARTICLES = 50;

  // main routines to do the conversions
void ConvertFile(TString inFileName, TString outFileName);
void ConvertTree(TString treeName);

  // utility functions (collected at the end of this file) 
  //   [containing mostly conversions between conventions]
TString PDGReadableName(int pdgID);
TString FSParticleType(TString glueXParticleType);
TString particleClass(TString glueXParticleType);
int PDGIDNumber(TString glueXParticleType);
int FSParticleOrder(TString glueXParticleType);
int FSParticleOrder(int pdgID);
pair<int,int> FSCode(vector< vector<TString> > glueXParticleTypes);
pair<int,int> FSCode(vector< vector<int> > pdgIDs);
  // utility functions for MC truth parsing
vector< vector<int> > OrderedThrownIndices(int numThrown, int pids[], int parentIndices[]);
int FSMCExtras(int numThrown, int pids[]);

  // global input parameters
TFile* gInputFile;
TFile* gOutputFile;
float  gChi2DOFCut;
bool gIsMC;



// **************************************
//   MAIN
// **************************************


int main(int argc, char** argv){
  cout << endl;
  cout << "***********************************************************" << endl;
  cout << "This program converts trees from the standard TTree format" << endl;
  cout << "in the GlueX analysis code to a flat TTree (\"FS Format\")." << endl << endl;
  cout << "The final state is determined automatically from the input file." << endl << endl;
  cout << "Usage:" << endl;
  cout << "  flatten  <input file name> <output file name> <MC: 0 or 1> " << endl;
  cout << "            [optional Chi2DOF cut value]" << endl << endl;
  cout << "Notes:" << endl;
  cout << "  * the output tree name is ntFSGlueX for now " << endl;
  cout << "  * the output tree name could also be derived from the input" << endl;
  cout << "     tree name (this is commented out)" << endl;
  cout << "  * the input tree name should contain \"_Tree\" (if this standard" << endl;
  cout << "     changes in the GlueX code, this code can be easily modified)" << endl;
  cout << "  * this will not work for all possible final states" << endl;
  cout << "***********************************************************" << endl << endl;
  if ((argc != 4) && (argc != 5)){
     cout << "ERROR: wrong number of arguments -- see usage notes above" << endl;
     exit(0);
  }
  TString inFileName(argv[1]);
  TString outFileName(argv[2]);
  TString isMC(argv[3]);
       if (isMC == "0"){ gIsMC = false; }
  else if (isMC == "1"){ gIsMC = true; }
  else {
     cout << "ERROR: 3rd argument should be 1 or 0 to specify whether this is or is not MC" << endl;
     exit(0);
  }
  if (argc == 4) gChi2DOFCut = 1000.0;
  if (argc == 5) gChi2DOFCut = atof(argv[4]);
  ConvertFile(inFileName,outFileName);
}



// **************************************
//   ConvertFile
// **************************************

void ConvertFile(TString inFileName, TString outFileName){
  int nTrees = 0;
  gInputFile  = new TFile(inFileName);
  gOutputFile = new TFile(outFileName,"recreate");
  TList* fileList = gInputFile->GetListOfKeys();
  for (int i = 0; i < fileList->GetEntries(); i++){
    TString treeName(fileList->At(i)->GetName());
    if (treeName.Contains("_Tree")){
      if (nTrees == 0) ConvertTree(treeName);
      if (nTrees >= 1){
        cout << "WARNING: found more than one tree -- only converted the first" << endl;
        cout << "           extra tree = " << treeName << endl;
      }
      nTrees++;
    }
  }
  gInputFile->Close();
  gOutputFile->Close();
  if (nTrees == 0){
    cout << "WARNING: could not find any trees" << endl;
  }
}



// **************************************
//   ConvertTree:  all the work is done here
// **************************************

void ConvertTree(TString treeName){

  cout << endl << "CONVERTING THE TREE NAMED: " << treeName << endl;

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
        else { cout << "  ERROR:  could not find UserInfo" << endl; exit(0); }
    TList* rootMothers = (TList*) userInfo->FindObject("ParticleNameList");
        if (rootMothers){ cout << "  OK: found ParticleNameList" << endl; }
        else { cout << "  ERROR:  could not find ParticleNameList" << endl; exit(0); }
    TMap* rootDecayProductMap = (TMap*) userInfo->FindObject("DecayProductMap");
        if (rootDecayProductMap){ cout << "  OK: found DecayProductMap" << endl; }
        else { cout << "  ERROR:  could not find DecayProductMap" << endl; exit(0); }
    TMap* rootNameToPIDMap = (TMap*) userInfo->FindObject("NameToPIDMap");
        if (rootNameToPIDMap){ cout << "  OK: found NameToPIDMap" << endl; }
        else { cout << "  ERROR:  could not find NameToPIDMap" << endl; exit(0); }
    TMap* miscInfo = (TMap*) userInfo->FindObject("MiscInfoMap");
        if (miscInfo){ cout << "  OK: found MiscInfoMap" << endl; }
        else { cout << "  ERROR:  could not find MiscInfoMap" << endl; exit(0); }
    TObjString* kinFitType = (TObjString*) miscInfo->GetValue("KinFitType");
        if (kinFitType->GetString() != "" && 
            kinFitType->GetString() != "0")
             { cout << "  OK: found KinFitType = "  << kinFitType->GetString() << endl; }
        else { cout << "  ERROR: bad KinFitType = " << kinFitType->GetString() << endl; exit(0); }
    TObjString* tosTCZ = (TObjString*) miscInfo->GetValue("Target__CenterZ");
        if (tosTCZ) 
             { cout << "  OK: found Target__CenterZ = "  << tosTCZ->GetString() << endl; }
        else { cout << "  ERROR: could not find Target__CenterZ " << endl; exit(0); }
    TString tsTCZ(tosTCZ->GetString());
        if (tsTCZ.IsFloat())
             { string sTCZ(""); for (int i=0; i<tsTCZ.Length(); i++){ sTCZ += tsTCZ[i]; }
               inTargetCenterZ = atof(sTCZ.c_str()); 
               cout << "            inTargetCenterZ = " << inTargetCenterZ << endl; }
        else { cout << "  ERROR: Target__CenterZ is not a number" << endl; exit(0); }
  }


     // **********************************************************************
     // STEP 1B:  extract particle names from the root file 
     // **********************************************************************

  cout << endl << endl << "READING PARTICLE NAMES FROM THE ROOT FILE:" << endl << endl;

  map< TString, vector<TString> > decayProductMap;  // from mothers to daughters (glueXNames)
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
     // STEP 1C:  perform checks on the final state
     // **********************************************************************

  cout << endl << endl << "PERFORMING CHECKS ON THE FINAL STATE:" << endl << endl;
  {
    if (decayProductMap.size() == 0){
      cout << endl << "  ERROR: no final state partices found" << endl;
      exit(0);
    }
    for (map<TString, vector<TString> >::const_iterator mItr = decayProductMap.begin();
         mItr != decayProductMap.end(); mItr++){
      TString motherName = mItr->first;
      TString motherFSType = FSParticleType(motherName);
      if (motherFSType == "--") motherFSType = "** NOT USED **";
      vector<TString> daughterNames = mItr->second;
      vector<TString> daughterFSTypes;
      for (unsigned int i = 0; i < daughterNames.size(); i++){
        daughterFSTypes.push_back(FSParticleType(daughterNames[i]));
      }
      cout << motherName << ": " << motherFSType << endl;
      for (unsigned int i = 0; i < daughterNames.size(); i++){
        cout << "    " << daughterNames[i] << ": " << daughterFSTypes[i] << endl;
      }
      if (motherFSType == "pi0" && (daughterNames.size() != 2 || 
            !(daughterFSTypes[0] == "gamma" && daughterFSTypes[1] == "gamma"))){
        cout << "  ERROR: unrecognized pi0 decay" << endl;
      }
      if (motherFSType == "eta" && (daughterNames.size() != 2 || 
            !(daughterFSTypes[0] == "gamma" && daughterFSTypes[1] == "gamma"))){
        cout << "  ERROR: unrecognized eta decay" << endl;
      }
      if (motherFSType == "Ks" && (daughterNames.size() != 2 || 
            !((daughterFSTypes[0] == "pi+" && daughterFSTypes[1] == "pi-") || 
              (daughterFSTypes[1] == "pi+" && daughterFSTypes[0] == "pi-")))){
        cout << "  ERROR: unrecognized Ks decay" << endl;
      }
      if (motherFSType == "Lambda" && (daughterNames.size() != 2 || 
            !((daughterFSTypes[0] == "p+" && daughterFSTypes[1] == "pi-") || 
              (daughterFSTypes[1] == "p+" && daughterFSTypes[0] == "pi-")))){
        cout << "  ERROR: unrecognized Lambda decay" << endl;
      }
      if (motherFSType == "ALambda" && (daughterNames.size() != 2 || 
            !((daughterFSTypes[0] == "p-" && daughterFSTypes[1] == "pi+") || 
              (daughterFSTypes[1] == "p-" && daughterFSTypes[0] == "pi+")))){
        cout << "  ERROR: unrecognized ALambda decay" << endl;
      }
    }
  }


     // **********************************************************************
     // STEP 1D:  extract PDG numbers from the root file (not used, just checking)
     // **********************************************************************

  cout << endl << endl << "READING PDG NUMBERS FROM THE ROOT FILE:" << endl << endl;
  //map< TString, int > nameToPIDMap;  // map from name to PDG ID (not used)
  {
    TList* userInfo = inTree->GetUserInfo();
    TMap* rootNameToPIDMap = (TMap*) userInfo->FindObject("NameToPIDMap");
    TMapIter tmapIter(rootNameToPIDMap);
    TObjString* rootName = (TObjString*) tmapIter();
    while (rootName != NULL){
      TObjString* rootPID =  (TObjString*) rootNameToPIDMap->GetValue(rootName->GetString());
      TString sName = rootName->GetString();
      TString sPID  = rootPID->GetString();
      cout << sName << ":  " << sPID << endl;
      rootName = (TObjString*) tmapIter.Next();
    }
  }


     // **********************************************************************
     // STEP 1E:  put the particle names in the right order 
     // **********************************************************************

  cout << endl << endl << "PUTTING PARTICLES IN THE RIGHT ORDER AND SETTING INDICES:" << endl << endl;

  vector< vector<TString> > orderedParticleNames;  // (glueXNames)
  {
    for (map<TString, vector<TString> >::const_iterator mItr = decayProductMap.begin();
         mItr != decayProductMap.end(); mItr++){
      if (FSParticleType(mItr->first) != "--"){
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
  pair<int,int> reconstructedFSCode = FSCode(orderedParticleNames);
  cout << "  DecayCode1 = " << reconstructedFSCode.first << endl;
  cout << "  DecayCode2 = " << reconstructedFSCode.second << endl << endl;


     // **********************************************************************
     // STEP 1F:  make maps from names to indices
     // **********************************************************************

  map<TString, TString> mapGlueXNameToFSIndex;
  map<TString, int> mapGlueXNameToParticleIndex;

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
      mapGlueXNameToFSIndex[name] = fsIndex;
      mapGlueXNameToParticleIndex[name] = particleIndex++;
      cout << "(" << mapGlueXNameToParticleIndex[name] << ")   ";
    }
    cout << endl;
    }
    cout << endl << endl << endl;
  }


   // **********************************************************************
   // STEP 2:  SET UP TO READ THE INPUT TREE (IN ANALYSIS TREE FORMAT)
   // **********************************************************************

        // ******************************
        // ***** 2A. SIMULATED DATA *****
        // ******************************

        //   *** Thrown Non-Particle Data ***

  UInt_t inNumThrown;
      if (gIsMC) inTree->SetBranchAddress("NumThrown", &inNumThrown);


        //   *** Thrown Beam Particle ***

  Float_t inThrownBeam__GeneratedEnergy;
      if (gIsMC) inTree->SetBranchAddress("ThrownBeam__GeneratedEnergy", &inThrownBeam__GeneratedEnergy);


        //   *** Thrown Products ***

  Int_t  inThrown__ParentIndex[MAXPARTICLES] = {};   
      if (gIsMC) inTree->SetBranchAddress("Thrown__ParentIndex", inThrown__ParentIndex);
  Int_t  inThrown__PID[MAXPARTICLES] = {};   
      if (gIsMC) inTree->SetBranchAddress("Thrown__PID", inThrown__PID);
  Int_t  inThrown__MatchID[MAXPARTICLES] = {};   
      if (gIsMC) inTree->SetBranchAddress("Thrown__MatchID", inThrown__MatchID);
  Float_t  inThrown__MatchFOM[MAXPARTICLES] = {};   
      if (gIsMC) inTree->SetBranchAddress("Thrown__MatchFOM", inThrown__MatchFOM);
  TClonesArray *inThrown__P4 = new TClonesArray("TLorentzVector");
      inTree->SetBranchAddress("Thrown__P4",&(inThrown__P4));



        // **************************************
        // ***** 2B. COMBO-INDEPENDENT DATA *****
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
  Bool_t inIsThrownTopology;
      if (gIsMC) inTree->SetBranchAddress("IsThrownTopology", &inIsThrownTopology);


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
  Float_t inNeutralHypo__ShowerQuality[MAXPARTICLES] = {};
      inTree->SetBranchAddress("NeutralHypo__ShowerQuality", inNeutralHypo__ShowerQuality);


        // ************************************
        // ***** 2C. COMBO-DEPENDENT DATA *****
        // ************************************

        //   *** Particle-Independent Data (indexed by combo) ***

  Float_t inRFTime_Measured[MAXCOMBOS] = {};
      inTree->SetBranchAddress("RFTime_Measured", inRFTime_Measured);  
  //Float_t inRFTime_KinFit[MAXCOMBOS] = {};
  //    inTree->SetBranchAddress("RFTime_KinFit", inRFTime_KinFit);  
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
      int pIndex = mapGlueXNameToParticleIndex[name];

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

    // MC information

  float outMCDecayCode1;  if (gIsMC) outTree.Branch("MCDecayCode1",&outMCDecayCode1,"MCDecayCode1/F");
  float outMCDecayCode2;  if (gIsMC) outTree.Branch("MCDecayCode2",&outMCDecayCode2,"MCDecayCode2/F");
  float outMCExtras;      if (gIsMC) outTree.Branch("MCExtras",    &outMCExtras,    "MCExtras/F");
  float outMCSignal;      if (gIsMC) outTree.Branch("MCSignal",    &outMCSignal,    "MCSignal/F");

    // particle information

  float  outPx[MAXPARTICLES]={},  outPy[MAXPARTICLES]={},  outPz[MAXPARTICLES]={},  outEn[MAXPARTICLES]={};
  float outRPx[MAXPARTICLES]={}, outRPy[MAXPARTICLES]={}, outRPz[MAXPARTICLES]={}, outREn[MAXPARTICLES]={};
  float outTkChi2[MAXPARTICLES]={}, outTkNDF[MAXPARTICLES]={};
  float outShQuality[MAXPARTICLES]={};
  {
    for (unsigned int im = 0; im < orderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < orderedParticleNames[im].size(); id++){
      TString name = orderedParticleNames[im][id];
      int pIndex = mapGlueXNameToParticleIndex[name];
      TString fsIndex = mapGlueXNameToFSIndex[name];
      TString vPx("PxP"); vPx += fsIndex; outTree.Branch(vPx,&outPx[pIndex],vPx+"/F");
      TString vPy("PyP"); vPy += fsIndex; outTree.Branch(vPy,&outPy[pIndex],vPy+"/F");
      TString vPz("PzP"); vPz += fsIndex; outTree.Branch(vPz,&outPz[pIndex],vPz+"/F");
      TString vEn("EnP"); vEn += fsIndex; outTree.Branch(vEn,&outEn[pIndex],vEn+"/F");
      TString vRPx("RPxP"); vRPx += fsIndex; outTree.Branch(vRPx,&outRPx[pIndex],vRPx+"/F");
      TString vRPy("RPyP"); vRPy += fsIndex; outTree.Branch(vRPy,&outRPy[pIndex],vRPy+"/F");
      TString vRPz("RPzP"); vRPz += fsIndex; outTree.Branch(vRPz,&outRPz[pIndex],vRPz+"/F");
      TString vREn("REnP"); vREn += fsIndex; outTree.Branch(vREn,&outREn[pIndex],vREn+"/F");
      if (particleClass(name) == "Charged"){
        TString vTkNDF("TkNDFP"); vTkNDF += fsIndex;
            outTree.Branch(vTkNDF,&outTkNDF[pIndex],vTkNDF+"/F");
        TString vTkChi2("TkChi2P"); vTkChi2 += fsIndex;
            outTree.Branch(vTkChi2,&outTkChi2[pIndex],vTkChi2+"/F");
      }
      if (particleClass(name) == "Neutral"){
        TString vQual("ShQualityP"); vQual += fsIndex;
            outTree.Branch(vQual, &outShQuality[pIndex], vQual+"/F");
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

     // print some information (for debugging only)

    if (iEntry+1 == 1){ 
      cout << endl << "PRINTING TEST INFORMATION FOR FIVE EVENTS..." << endl << endl;
    }
    if (iEntry < 5){
      cout << endl << endl;
      cout << "  ***************************" << endl;
      cout << "  ******* NEW EVENT " << iEntry+1 << " *******" << endl;
      cout << "  ***************************" << endl;
      cout << endl << endl;
    }


      // if MC, start parsing truth information

    map<TString, int> mapGlueXNameToThrownIndex;

    vector< vector<int> > orderedThrownIndices;
    vector< vector<int> > orderedThrownPDGNumbers;

    if (gIsMC){
      orderedThrownIndices = OrderedThrownIndices(inNumThrown,inThrown__PID,inThrown__ParentIndex);
      orderedThrownPDGNumbers = orderedThrownIndices;
      for (unsigned int i = 0; i < orderedThrownPDGNumbers.size(); i++){
      for (unsigned int j = 0; j < orderedThrownPDGNumbers[i].size(); j++){
        orderedThrownPDGNumbers[i][j] = inThrown__PID[orderedThrownIndices[i][j]];
      }}
      pair<int,int> fsCode = FSCode(orderedThrownPDGNumbers);
      outMCDecayCode1 = fsCode.first;
      outMCDecayCode2 = fsCode.second;
      outMCExtras = FSMCExtras(inNumThrown,inThrown__PID);
      outMCSignal = 0;
      if ((reconstructedFSCode.first == fsCode.first) &&
          (reconstructedFSCode.second == fsCode.second) &&
          (outMCExtras < 0.1)) outMCSignal = 1;
    }


      // print a few events to make sure MC makes sense

    if (gIsMC && iEntry < 5){
    //if (gIsMC && (iEntry < 5||inIsThrownTopology)){
      cout << endl << endl;
      cout << "  **** TRUTH INFO STUDY FOR EVENT " << iEntry+1 << " ****" << endl;
      cout << "  NumThrown = " << inNumThrown << endl;
      cout << "  GeneratedEnergy = " << inThrownBeam__GeneratedEnergy << endl;
      cout << "  FSCode = " << outMCDecayCode2 << "_" << outMCDecayCode1 << endl;
      cout << "  IsThrownTopology = " << inIsThrownTopology << endl;
      for (int i = 0; i < inNumThrown; i++){      
        cout << "    THROWN INDEX = " << i << endl;
        cout << "      PID = " << inThrown__PID[i] << endl;
        cout << "      PDG Name = " << PDGReadableName(inThrown__PID[i]) << endl;
        cout << "      Parent Index = " << inThrown__ParentIndex[i] << endl;
      }
      cout << endl << endl;
    }


      // loop over combos

    for (UInt_t ic = 0; ic < inNumCombos; ic++){

        // non-particle information

      TLorentzVector *p4, *p4a, *p4b, *x4;
      outRunNumber   = inRunNumber;
      outEventNumber = inEventNumber;
      outChi2        = inChiSq_KinFit[ic];
      outChi2DOF     = -1; if (inNDF_KinFit[ic]>0.0) outChi2DOF = outChi2/inNDF_KinFit[ic];
      //outRFTime      = inRFTime_KinFit[ic];
      outRFTime      = inRFTime_Measured[ic];
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
        int pIndex = mapGlueXNameToParticleIndex[name];

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
          outShQuality[pIndex] = inNeutralHypo__ShowerQuality[(inNeutralIndex[pIndex][ic])];
        }

          // decaying to charged tracks

        if (particleClass(name) == "DecayingToCharged"){ 
          int pIndex1 = mapGlueXNameToParticleIndex[orderedParticleNames[im][1]];
          int pIndex2 = mapGlueXNameToParticleIndex[orderedParticleNames[im][2]];
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
          int pIndex1 = mapGlueXNameToParticleIndex[orderedParticleNames[im][1]];
          int pIndex2 = mapGlueXNameToParticleIndex[orderedParticleNames[im][2]];
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

      if (iEntry < 5){
        cout << "  *******************************" << endl;
        cout << "  **** INFO FOR EVENT " << iEntry+1 << " ****" << endl;
        cout << "  *******************************" << endl;
        cout << "EVENT: " << inEventNumber << " (combo no. " << ic+1 << ")" << endl;
        if (gIsMC) cout << "  NumThrown = " << inNumThrown << endl;
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
          int pIndex = mapGlueXNameToParticleIndex[name];
          TString fsIndex = mapGlueXNameToFSIndex[name];
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
















// **************************************
//   utility functions
// **************************************


static const int kpdgPsi2S      = 100443;     
static const int kpdgGamma      = 22;         
static const int kpdgFSRGamma   = -22;        
static const int kpdgHc         = 10443;      
static const int kpdgChic0      = 10441;      
static const int kpdgChic1      = 20443;      
static const int kpdgChic2      = 445;        
static const int kpdgJpsi       = 443;        
static const int kpdgEtac       = 441;        
static const int kpdgPhi        = 333;        
static const int kpdgOmega      = 223;        
static const int kpdgPi0        = 111;        
static const int kpdgPip        = 211;        
static const int kpdgPim        = -211;       
static const int kpdgRho0       = 113;        
static const int kpdgRhop       = 213;        
static const int kpdgRhom       = -213;       
static const int kpdgEtaprime   = 331;        
static const int kpdgEta        = 221;        
static const int kpdgKs         = 310;        
static const int kpdgKl         = 130;        
static const int kpdgKp         = 321;        
static const int kpdgKm         = -321;       
static const int kpdgPp         = 2212;       
static const int kpdgPm         = -2212;      
static const int kpdgN          = 2112;       
static const int kpdgAntiN      = -2112;
static const int kpdgDelta0     = 2114;
static const int kpdgDeltap     = 2214;
static const int kpdgDeltapp    = 2224;
static const int kpdgEp         = -11;         
static const int kpdgEm         = 11;        
static const int kpdgMup        = -13;         
static const int kpdgMum        = 13;        
static const int kpdgTaup       = -15;        
static const int kpdgTaum       = 15;         
static const int kpdgNuE        = 12;         
static const int kpdgNuMu       = 14;         
static const int kpdgNuTau      = 16;         
static const int kpdgAntiNuE    = -12;        
static const int kpdgAntiNuMu   = -14;        
static const int kpdgAntiNuTau  = -16;        
static const int kpdgF0600      = 9000221;    
static const int kpdgK0         = 311;        
static const int kpdgAntiK0     = -311;       
static const int kpdgKstarp     = 323;        
static const int kpdgKstarm     = -323;       
static const int kpdgKstar0     = 313;        
static const int kpdgAntiKstar0 = -313;
static const int kpdgLambda     = 3122;
static const int kpdgALambda    = -3122;
static const int kpdgDp         = 411;
static const int kpdgDm         = -411;
static const int kpdgD0         = 421;
static const int kpdgDA         = -421;
static const int kpdgDstarp     = 413;
static const int kpdgDstarm     = -413;
static const int kpdgDstar0     = 423;
static const int kpdgDstarA     = -423;


TString PDGReadableName(int id){
  TString name("");
  if      (id == kpdgPsi2S)      name = "psi(2S)";
  else if (id == kpdgGamma)      name = "gamma";
  else if (id == kpdgFSRGamma)   name = "FSRgamma";
  else if (id == kpdgHc)         name = "h_c";
  else if (id == kpdgChic0)      name = "chi_c0";
  else if (id == kpdgChic1)      name = "chi_c1";
  else if (id == kpdgChic2)      name = "chi_c2";
  else if (id == kpdgJpsi)       name = "J/psi";
  else if (id == kpdgEtac)       name = "eta_c";
  else if (id == kpdgPhi)        name = "phi";
  else if (id == kpdgOmega)      name = "omega";
  else if (id == kpdgPi0)        name = "pi0";
  else if (id == kpdgPip)        name = "pi+";
  else if (id == kpdgPim)        name = "pi-";
  else if (id == kpdgRho0)       name = "rho0";
  else if (id == kpdgRhop)       name = "rho+";
  else if (id == kpdgRhom)       name = "rho-";
  else if (id == kpdgEtaprime)   name = "etaprime";
  else if (id == kpdgEta)        name = "eta";
  else if (id == kpdgKs)         name = "K_S0";
  else if (id == kpdgKl)         name = "K_L0";
  else if (id == kpdgKp)         name = "K+";
  else if (id == kpdgKm)         name = "K-";
  else if (id == kpdgPp)         name = "p+";
  else if (id == kpdgPm)         name = "p-";
  else if (id == kpdgN)          name = "N";
  else if (id == kpdgAntiN)      name = "antiN";
  else if (id == kpdgDelta0)     name = "Delta0";
  else if (id == kpdgDeltap)     name = "Delta+";
  else if (id == kpdgDeltapp)    name = "Delta++";
  else if (id == kpdgEp)         name = "e+";
  else if (id == kpdgEm)         name = "e-";
  else if (id == kpdgMup)        name = "mu+";
  else if (id == kpdgMum)        name = "mu-";
  else if (id == kpdgTaup)       name = "tau+";
  else if (id == kpdgTaum)       name = "tau-";
  else if (id == kpdgNuE)        name = "nu";
  else if (id == kpdgNuMu)       name = "nu";
  else if (id == kpdgNuTau)      name = "nu";
  else if (id == kpdgAntiNuE)    name = "nu";
  else if (id == kpdgAntiNuMu)   name = "nu";
  else if (id == kpdgAntiNuTau)  name = "nu";
  else if (id == kpdgF0600)      name = "f0(600)";
  else if (id == kpdgK0)         name = "K0";
  else if (id == kpdgAntiK0)     name = "K0";
  else if (id == kpdgKstarp)     name = "K*+";
  else if (id == kpdgKstarm)     name = "K*-";
  else if (id == kpdgKstar0)     name = "K*0";
  else if (id == kpdgAntiKstar0) name = "K*0";
  else if (id == kpdgLambda)     name = "Lambda";
  else if (id == kpdgALambda)    name = "ALambda";
  else if (id == kpdgDp)         name = "D+";
  else if (id == kpdgDm)         name = "D-";
  else if (id == kpdgD0)         name = "D0";
  else if (id == kpdgDA)         name = "D0bar";
  else if (id == kpdgDstarp)     name = "D*+";
  else if (id == kpdgDstarm)     name = "D*-";
  else if (id == kpdgDstar0)     name = "D*0";
  else if (id == kpdgDstarA)     name = "D*0bar";
  else{
    name += id;
  }
  return name;
}


vector< vector<int> > OrderedThrownIndices(int numThrown, int pids[], int parentIndices[]){
  vector< vector<int> > orderedThrownIndices;
  {
    map<int, vector<int> > mapThrownIndexToDaughters;
    for (int i = 0; i < numThrown; i++){
      int parentIndex = parentIndices[i];
      mapThrownIndexToDaughters[parentIndex].push_back(i);
    }
    map<int, bool> mapUsedIndices;
    for (int i = 0; i < numThrown; i++){
      mapUsedIndices[i] = false;
    }
    for (int i = 0; i < numThrown; i++){ 
      if (mapUsedIndices[i] == true) continue;
      if (mapThrownIndexToDaughters.find(i) == mapThrownIndexToDaughters.end()) continue;
      vector<int> daughterIndices = mapThrownIndexToDaughters[i];
      if (daughterIndices.size() != 2) continue;
      int parentIndex = i;
      int parentID = pids[parentIndex];
      int daughterIndex1 = daughterIndices[0];
      int daughterIndex2 = daughterIndices[1];
      int daughterID1 = pids[daughterIndex1];
      int daughterID2 = pids[daughterIndex2];
      if ((parentID == kpdgKs      && daughterID1 == kpdgPip   && daughterID2 == kpdgPim)   || 
          (parentID == kpdgLambda  && daughterID1 == kpdgPp    && daughterID2 == kpdgPim)   ||
          (parentID == kpdgALambda && daughterID1 == kpdgPm    && daughterID2 == kpdgPip)   ||
          (parentID == kpdgPi0     && daughterID1 == kpdgGamma && daughterID2 == kpdgGamma) ||
          (parentID == kpdgEta     && daughterID1 == kpdgGamma && daughterID2 == kpdgGamma)){
        vector<int> addIndices;
        addIndices.push_back(parentIndex);     mapUsedIndices[parentIndex] = true;
        addIndices.push_back(daughterIndex1);  mapUsedIndices[daughterIndex1] = true;
        addIndices.push_back(daughterIndex2);  mapUsedIndices[daughterIndex2] = true;
        orderedThrownIndices.push_back(addIndices);
      }
      if ((parentID == kpdgKs      && daughterID1 == kpdgPim && daughterID2 == kpdgPip) || 
          (parentID == kpdgLambda  && daughterID1 == kpdgPim && daughterID2 == kpdgPp) ||
          (parentID == kpdgALambda && daughterID1 == kpdgPip && daughterID2 == kpdgPm)){
        vector<int> addIndices;
        addIndices.push_back(parentIndex);     mapUsedIndices[parentIndex] = true;
        addIndices.push_back(daughterIndex2);  mapUsedIndices[daughterIndex2] = true;
        addIndices.push_back(daughterIndex1);  mapUsedIndices[daughterIndex1] = true;
        orderedThrownIndices.push_back(addIndices);
      }
    }
    for (int i = 0; i < numThrown; i++){ 
      if (mapUsedIndices[i] == true) continue;
      if (mapThrownIndexToDaughters.find(i) != mapThrownIndexToDaughters.end()) continue;
      int index = i;
      int pdgID = pids[index];
      if ((pdgID == kpdgEp) || (pdgID == kpdgEm) || (pdgID == kpdgMup) || (pdgID == kpdgMum) ||
          (pdgID == kpdgPp) || (pdgID == kpdgPm) || (pdgID == kpdgGamma) || 
          (pdgID == kpdgKp) || (pdgID == kpdgKm) || (pdgID == kpdgPip) || (pdgID == kpdgPim)){
        vector<int> addIndex;
        addIndex.push_back(index);    mapUsedIndices[index] = true;
        orderedThrownIndices.push_back(addIndex);
      }
    }
    for (unsigned int i = 0; i < orderedThrownIndices.size(); i++){
      for (unsigned int j = i + 1; j < orderedThrownIndices.size(); j++){
        if (FSParticleOrder(pids[orderedThrownIndices[j][0]]) >
            FSParticleOrder(pids[orderedThrownIndices[i][0]])){
          vector<int> temp = orderedThrownIndices[i];
          orderedThrownIndices[i] = orderedThrownIndices[j];
          orderedThrownIndices[j] = temp;
        }
      }
    }
  }
  return orderedThrownIndices;
}


int FSMCExtras(int numThrown, int pids[]){
  int mcExtras = 0;
  for (int i = 0; i < numThrown; i++){
    if ((pids[i] == kpdgNuE) ||
        (pids[i] == kpdgNuMu) ||
        (pids[i] == kpdgNuTau) ||
        (pids[i] == kpdgAntiNuE) ||
        (pids[i] == kpdgAntiNuMu) ||
        (pids[i] == kpdgAntiNuTau)) mcExtras += 1000;
    if  (pids[i] == kpdgKl)         mcExtras += 100;
    if  (pids[i] == kpdgN)          mcExtras += 10;
    if  (pids[i] == kpdgAntiN)      mcExtras += 1;
  }
  return mcExtras;
}



TString FSParticleType(TString glueXParticleType){
  if (glueXParticleType.Contains("AntiLambda"))  return TString("ALambda");
  if (glueXParticleType.Contains("Lambda"))      return TString("Lambda");
  if (glueXParticleType.Contains("Positron"))    return TString("e+");
  if (glueXParticleType.Contains("Electron"))    return TString("e-");
  if (glueXParticleType.Contains("MuonPlus"))    return TString("mu+");
  if (glueXParticleType.Contains("MuonMinus"))   return TString("mu-");
  if (glueXParticleType.Contains("AntiProton"))  return TString("p-");
  if (glueXParticleType.Contains("Proton"))      return TString("p+");
  if (glueXParticleType.Contains("Eta"))         return TString("eta");
  if (glueXParticleType.Contains("Photon"))      return TString("gamma");
  if (glueXParticleType.Contains("KPlus"))       return TString("K+");
  if (glueXParticleType.Contains("KMinus"))      return TString("K-");
  if (glueXParticleType.Contains("KShort"))      return TString("Ks");
  if (glueXParticleType.Contains("PiPlus"))      return TString("pi+");
  if (glueXParticleType.Contains("PiMinus"))     return TString("pi-");
  if (glueXParticleType.Contains("Pi0"))         return TString("pi0");
  return TString("--");
}


TString particleClass(TString glueXParticleType){
  if (glueXParticleType.Contains("AntiLambda"))  return TString("DecayingToCharged");
  if (glueXParticleType.Contains("Lambda"))      return TString("DecayingToCharged");
  if (glueXParticleType.Contains("Positron"))    return TString("Charged");
  if (glueXParticleType.Contains("Electron"))    return TString("Charged");
  if (glueXParticleType.Contains("MuonPlus"))    return TString("Charged");
  if (glueXParticleType.Contains("MuonMinus"))   return TString("Charged");
  if (glueXParticleType.Contains("AntiProton"))  return TString("Charged");
  if (glueXParticleType.Contains("Proton"))      return TString("Charged");
  if (glueXParticleType.Contains("Eta"))         return TString("DecayingToNeutral");
  if (glueXParticleType.Contains("Photon"))      return TString("Neutral");
  if (glueXParticleType.Contains("KPlus"))       return TString("Charged");
  if (glueXParticleType.Contains("KMinus"))      return TString("Charged");
  if (glueXParticleType.Contains("KShort"))      return TString("DecayingToCharged");
  if (glueXParticleType.Contains("PiPlus"))      return TString("Charged");
  if (glueXParticleType.Contains("PiMinus"))     return TString("Charged");
  if (glueXParticleType.Contains("Pi0"))         return TString("DecayingToNeutral");
  return TString("");
}

int PDGIDNumber(TString glueXParticleType){
  if (glueXParticleType.Contains("AntiLambda"))  return kpdgALambda;
  if (glueXParticleType.Contains("Lambda"))      return kpdgLambda;
  if (glueXParticleType.Contains("Positron"))    return kpdgEp;
  if (glueXParticleType.Contains("Electron"))    return kpdgEm;
  if (glueXParticleType.Contains("MuonPlus"))    return kpdgMup;
  if (glueXParticleType.Contains("MuonMinus"))   return kpdgMum;
  if (glueXParticleType.Contains("AntiProton"))  return kpdgPm;
  if (glueXParticleType.Contains("Proton"))      return kpdgPp;
  if (glueXParticleType.Contains("Eta"))         return kpdgEta;
  if (glueXParticleType.Contains("Photon"))      return kpdgGamma;
  if (glueXParticleType.Contains("KPlus"))       return kpdgKp;
  if (glueXParticleType.Contains("KMinus"))      return kpdgKm;
  if (glueXParticleType.Contains("KShort"))      return kpdgKs;
  if (glueXParticleType.Contains("PiPlus"))      return kpdgPip;
  if (glueXParticleType.Contains("PiMinus"))     return kpdgPim;
  if (glueXParticleType.Contains("Pi0"))         return kpdgPi0;
  return 0;
}


int FSParticleOrder(TString glueXParticleType){
  return FSParticleOrder(PDGIDNumber(glueXParticleType));
}


int FSParticleOrder(int pdgID){
  if (pdgID == kpdgLambda)  return 16;
  if (pdgID == kpdgALambda) return 15;
  if (pdgID == kpdgEp)      return 14;
  if (pdgID == kpdgEm)      return 13;
  if (pdgID == kpdgMup)     return 12;
  if (pdgID == kpdgMum)     return 11;
  if (pdgID == kpdgPp)      return 10;
  if (pdgID == kpdgPm)      return 9;
  if (pdgID == kpdgEta)     return 8;
  if (pdgID == kpdgGamma)   return 7;
  if (pdgID == kpdgKp)      return 6;
  if (pdgID == kpdgKm)      return 5;
  if (pdgID == kpdgKs)      return 4;
  if (pdgID == kpdgPip)     return 3;
  if (pdgID == kpdgPim)     return 2;
  if (pdgID == kpdgPi0)     return 1;
  return 0;
}


pair<int,int> FSCode(vector< vector<TString> > glueXParticleTypes){
  vector< vector<int> > pdgIDs;
  for (unsigned int i = 0; i < glueXParticleTypes.size(); i++){
    vector<int> addPDGIDs;
    for (unsigned int j = 0; j < glueXParticleTypes[i].size(); j++){
      addPDGIDs.push_back(PDGIDNumber(glueXParticleTypes[i][j]));
    }
    pdgIDs.push_back(addPDGIDs);
  }
  return FSCode(pdgIDs);
}


pair<int,int> FSCode(vector< vector<int> > pdgIDs){
  int code1 = 0;
  int code2 = 0;
  for (unsigned int i = 0; i < pdgIDs.size(); i++){
    int pdgID = pdgIDs[i][0];
         if (pdgID  == kpdgLambda)  { code2 += 100000000; }
    else if (pdgID  == kpdgALambda) { code2 += 10000000; }
    else if (pdgID  == kpdgEp)      { code2 += 1000000; }
    else if (pdgID  == kpdgEm)      { code2 += 100000; }
    else if (pdgID  == kpdgMup)     { code2 += 10000; }
    else if (pdgID  == kpdgMum)     { code2 += 1000; }
    else if (pdgID  == kpdgPp)      { code2 += 100; }
    else if (pdgID  == kpdgPm)      { code2 += 10; }
    else if (pdgID  == kpdgEta)     { code2 += 1; }
    else if (pdgID  == kpdgGamma)   { code1 += 1000000; }
    else if (pdgID  == kpdgKp)      { code1 += 100000; }
    else if (pdgID  == kpdgKm)      { code1 += 10000; }
    else if (pdgID  == kpdgKs)      { code1 += 1000; }
    else if (pdgID  == kpdgPip)     { code1 += 100; }
    else if (pdgID  == kpdgPim)     { code1 += 10; }
    else if (pdgID  == kpdgPi0)     { code1 += 1; }
  }
  return pair<int,int>(code1,code2);
}


