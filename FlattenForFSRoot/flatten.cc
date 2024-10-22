#include <iostream>
#include <sstream>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TList.h"
#include "TMap.h"
#include "TIterator.h"
#include "TSystem.h"
#include "TObjString.h"
using namespace std;


  // maximum array indices
  //  careful:  these are taken from halld_recon/src/libraries/ANALYSIS/DEventWriterROOT.cc
  //       and then scaled to give a buffer
  //     (if the sizes are too big, strange behavior sometimes results)
static const int MAXTHROWN    =  20*2;
static const int MAXBEAM      =  20*2;
static const int MAXTRACKS    =  50*1;
static const int MAXNEUTRALS  =  15*2;
static const int MAXCOMBOS    = 100*50;
static const int MAXPARTICLES =  MAXTRACKS + MAXNEUTRALS;

  // utility functions (collected at the end of this file)
  //   [containing mostly conversions between conventions]
TString PDGReadableName(int pdgID);
TString FSParticleType(TString glueXParticleType);
TString GlueXParticleClass(TString glueXParticleType);
int GlueXNeutralsCounter(TString glueXParticleType);
int PDGIDNumber(TString glueXParticleType);
map<TString, vector<TString> > GlueXDecayProductMap(int fsCode1, int fsCode2);
int FSParticleOrder(TString glueXParticleType);
int FSParticleOrder(int pdgID);
pair<int,int> FSCode(vector< vector<TString> > glueXParticleTypes);
pair<int,int> FSCode(vector< vector<int> > pdgIDs);
  // utility functions for MC truth parsing
vector< vector<int> > OrderedThrownIndices(int numThrown, int pids[], int parentIndices[]);
vector<int> MCDecayParticles(int numThrown, int pids[], int parentIndices[]);
void DisplayMCThrown(int numThrown, int pids[], int parentIndices[], TClonesArray* p4s);
int FSMCExtras(int numThrown, int pids[]);
int BaryonNumber(int fsCode1, int fsCode2, int mcExtras = 0);
int Charge(int fsCode1, int fsCode2, int mcExtras = 0);
bool GetPolarizationAngle(int runNumber, int& polarizationAngle);



// **************************************
//   MAIN
// **************************************


int main(int argc, char** argv){
  cout << endl;
  cout << "***********************************************************" << endl;
  cout << "This program converts trees from the standard TTree format in the" << endl;
  cout << "  GlueX analysis environment to a flat TTree (\"FSRoot Format\")." << endl << endl;
  cout << "The output tree format is compatible with that used by the external" << endl;
  cout << "  FSRoot package located here:   https://github.com/remitche66/FSRoot" << endl << endl;
  cout << "The final state is determined automatically from the input file." << endl << endl;
  cout << "Usage:" << endl;
  cout << "  flatten  -in    <input file name>                     (required)" << endl;
  cout << "           -out   [output file name or none]            (default: none)" << endl;
  cout << "                   (if no output specified, just print info and quit)"   << endl;
  cout << "           -mc    [is this mc? -1, 0, or 1]             (default: -1)" << endl;
  cout << "                   (-1: determine automatically; 0: no; 1: yes)"   << endl;
  cout << "           -mctag [MCExtras_MCDecayCode2_MCDecayCode1]  (default: none)" << endl;
  cout << "                   (pick out a single final state from MC)"  << endl;
  cout << "           -chi2  [optional Chi2/DOF cut value]         (default: 1000)" << endl;
  cout << "           -RFDeltaT  [optional cut on abs(RFDeltaT)]         (default: no cut)" << endl;
  cout << "           -shQuality  [optional shower quality cut value] (default: -1 (no cut))" << endl;
  cout << "           -massWindows  [pi0, eta, (A)Lambda, Ks windows (GeV)] (default: -1 (no cut))" << endl;
  cout << "                        (uses the most constrained four-momenta)" << endl;
  cout << "           -numUnusedTracks   [optional cut (<= cut)]   (default: -1 (no cut))" << endl;
  cout << "           -numUnusedNeutrals [optional cut (<= cut)]   (default: -1 (no cut))" << endl;
  cout << "           -numNeutralHypos   [optional cut (<= cut)]   (default: -1 (no cut))" << endl;
  cout << "           -usePolarization   [get polarization angle from RCDB? 0 or 1]   (default: 0)" << endl;
  cout << "           -addPID    [include PID info in the output tree? 0 or 1]   (default: 1)" << endl;
  cout << "           -dirc [include PID information from the DIRC if available? 0 or 1] (default: 0)" << endl;
  cout << "           -flattenpi0 [flatten pi0s to just gamma gamma? 0 or 1]   (default: 0)" << endl;
  cout << "           -flatteneta [flatten etas to just gamma gamma? 0 or 1]   (default: 0)" << endl;
  cout << "           -addUnusedNeutrals  [include 4-vectors for unused neutrals? " << endl;
  cout << "                                0 or number to include] (default: 0)" << endl;  
  cout << "           -combos [ if there are multiple combos in an event with the same chi2, then " << endl;
  cout << "                      0: keep all combos and print warnings (default) " << endl;  
  cout << "                      1: keep only one combo in the output tree" << endl;
  cout << "                      2: keep all combos and suppress warnings ]" << endl;
  cout << "           -mcChecks  [check for baryon number violation, etc.," << endl;
  cout << "                       when parsing truth information?  0 or 1] (default: 1)" << endl;
  cout << "           -safe  [check array sizes?  0 or 1]          (default: 1)" << endl;
  cout << "           -print [print to screen: " << endl;
  cout << "                   -1 (less); 0 (regular); 1 (more); 2 (even more)]    (default: 0)" << endl;
  cout << endl;
  cout << "Notes:" << endl;
  cout << "  * multiple input files can be specified using wildcards, but they"  << endl;
  cout << "     should all be of the same type (same final state, MC or data, etc) " << endl;
  cout << "  * the input tree name should contain \"_Tree\" (if this standard" << endl;
  cout << "     changes in the GlueX code, this code can be easily modified)" << endl;
  cout << "  * the output tree name is ntFSGlueX_[code2]_[code1], where [code1] " << endl;
  cout << "      and [code2] specify the final state (Documentation/GlueXFSRootFormat.pdf)" << endl;
  cout << "  * this works for a large variety of final states (but not all)" << endl;
  cout << "  * the \"safe\" option first reads a limited number of variables" << endl;
  cout << "      from the tree, then does checks on array sizes, etc.," << endl;
  cout << "      then reads in the rest of the tree -- this is slower, but avoids" << endl;
  cout << "      segmentation faults if array sizes are exceeded" << endl;
  cout << "***********************************************************" << endl << endl;
  if (argc < 3){
     cout << "ERROR: wrong number of arguments -- see usage notes above" << endl;
     exit(0);
  }


   // **********************************************************************
   // STEP 1:  PARSE THE COMMAND LINE ARGUMENTS
   // **********************************************************************

  vector<TString> gInFileNames;
  TString gOutFileName("none");
  int gInputIsMC = -1;
  TString gMCTag = "";
  TString gMCTagExtras = "0";
  TString gMCTagDecayCode2 = "0";
  TString gMCTagDecayCode1 = "0";
  double gChi2DOFCut = 1000.0;
  double gRFDeltaTCut = -1.0;
  double gShQualityCut = -1;
  double gMassWindows = -1;
  int gNumUnusedTracksCut = -1;
  int gNumUnusedNeutralsCut = -1;
  int gNumNeutralHyposCut = -1;
  bool gSafe = true;
  bool gUsePolarization = false;
  bool gAddPID = true;
  bool gUseDIRC = false;
  bool gFlattenpi0 = false;
  bool gFlatteneta = false;
  int gAddUnusedNeutrals = 0;
  int gCombos = 0;
  bool gMCChecks = true;
  int gPrint = 0;
  {
  TString flag = "";
  for (int i = 0; i < argc; i++){
    TString argi(argv[i]);
    if ((argi == "-in")||(argi == "-out")||(argi == "-mc")||(argi == "-mctag")
        ||(argi == "-chi2")||(argi == "-RFDeltaT")||(argi == "-shQuality")||(argi == "-massWindows")
        ||(argi == "-numUnusedTracks")||(argi == "-usePolarization")||(argi == "-numUnusedNeutrals")
        ||(argi == "-mcChecks")||(argi == "-addPID")||(argi == "-dirc")||(argi == "-flattenpi0")||(argi == "-flatteneta")
        ||(argi=="-addUnusedNeutrals")||(argi == "-combos")
        ||(argi == "-numNeutralHypos")||(argi == "-safe")||(argi == "-print")){
      flag = argi;
      continue;
    }
    if (flag == "-in")  gInFileNames.push_back(argi);
    if (flag == "-out") gOutFileName = argi;
    if (flag == "-mc"){ if (argi == "1") gInputIsMC = 1;
                        if (argi == "0") gInputIsMC = 0; }
    if (flag == "-mctag"){ gMCTag = argi; }
    if (flag == "-chi2"){ gChi2DOFCut = atof(argi); }
    if (flag == "-RFDeltaT"){ gRFDeltaTCut = atof(argi); }
    if (flag == "-shQuality"){ gShQualityCut = atof(argi); }
    if (flag == "-massWindows"){ gMassWindows = atof(argi); }
    if (flag == "-numUnusedTracks"){ gNumUnusedTracksCut = atoi(argi); }
    if (flag == "-numUnusedNeutrals"){ gNumUnusedNeutralsCut = atoi(argi); }
    if (flag == "-numNeutralHypos"){ gNumNeutralHyposCut = atoi(argi); }
    if (flag == "-usePolarization"){ if (argi == "1") gUsePolarization = true; }
    if (flag == "-addPID"){ if (argi == "0") gAddPID = false; }
    if (flag == "-dirc"){   if( argi == "1") gUseDIRC = true; }
    if (flag == "-flattenpi0"){ if (argi == "1") gFlattenpi0 = true; }
    if (flag == "-flatteneta"){ if (argi == "1") gFlatteneta = true; }
    if (flag == "-addUnusedNeutrals"){ gAddUnusedNeutrals = atoi(argi); }
    if (flag == "-combos"){ gCombos = atoi(argi); }
    if (flag == "-mcChecks"){ if (argi == "0") gMCChecks = false; }
    if (flag == "-safe"){ if (argi == "0") gSafe = false; }
    if (flag == "-print"){ gPrint = atoi(argi); }
  }
  if (gMCTag == "none") gMCTag = "";
  if (gMCTag != ""){
    int numUnderscores = 0;
    for (int j = 0; j < gMCTag.Length(); j++){
      TString digit = TString(gMCTag[j]);
      if (digit == "_") numUnderscores++;
      if (numUnderscores == 0 && digit.IsDigit()) gMCTagExtras += digit;
      if (numUnderscores == 1 && digit.IsDigit()) gMCTagDecayCode2 += digit;
      if (numUnderscores == 2 && digit.IsDigit()) gMCTagDecayCode1 += digit;
    }
  }
  cout << endl;
  cout << "INPUT PARAMETERS:" << endl << endl;
  cout << "  input files:           " << endl;
  for (unsigned int i = 0; i < gInFileNames.size(); i++){
    cout << "    (" << i+1 <<") " << gInFileNames[i] << endl;
  }
  cout << "  output file:           " << gOutFileName << endl;
  cout << "  MC?                    " << gInputIsMC << endl;
  if (gMCTag != "")
  cout << "  MC Tag:                " << gMCTagExtras << "_" << gMCTagDecayCode2
                                                      << "_" << gMCTagDecayCode1 << endl;
  if (gMCTag == "")
  cout << "  MC Tag:                " << "none" << endl;
  cout << "  chi2/dof cut:          " << gChi2DOFCut << endl;
  cout << "  RFDeltaT cut:          " << gRFDeltaTCut << endl;
  cout << "  shower quality cut:    " << gShQualityCut << endl;
  cout << "  mass windows:          " << gMassWindows << endl;
  cout << "  numUnusedTracks cut:   " << gNumUnusedTracksCut << endl;
  cout << "  numUnusedNeutrals cut: " << gNumUnusedNeutralsCut << endl;
  cout << "  numNeutralHypos cut:   " << gNumNeutralHyposCut << endl;
  cout << "  use polarization?      " << gUsePolarization << endl;
  cout << "  use PID?               " << gAddPID << endl;
  cout << "  use DIRC?              " << gUseDIRC << endl;
  cout << "  flatten pi0s?          " << gFlattenpi0 << endl;
  cout << "  flatten etas?          " << gFlatteneta << endl;
  cout << "  add unused neutrals?   " << gAddUnusedNeutrals << endl;
  cout << "  combos option:         " << gCombos << endl;
  cout << "  MC checks?             " << gMCChecks << endl;
  cout << "  safe mode?             " << gSafe << endl;
  cout << endl;
  if ((gInFileNames.size() == 0) || (gOutFileName == "")){
     cout << "ERROR: specify input and output files -- see usage notes above" << endl;
     exit(0);
  }
  if ((gMCTag != "") &&
      GlueXDecayProductMap(gMCTagDecayCode1.Atoi(),gMCTagDecayCode2.Atoi()).size() == 0){
    cout << "ERROR: no particles corresponding to mctag = " << gMCTag << endl;
    exit(0);
  }
  }


   // **********************************************************************
   // STEP 2:  SET UP THE INPUT AND OUTPUT FILES
   // **********************************************************************

     // **********************************************************************
     // STEP 2A:  set up the output file
     // **********************************************************************

  TFile* gOutputFile = NULL;
  if (gOutFileName != "none") gOutputFile = new TFile(gOutFileName,"RECREATE");

     // **********************************************************************
     // STEP 2B:  set up the input tree
     // **********************************************************************

  TChain* gInTree = NULL;
  TTree* gInTreeFirst = NULL;
  {
    TString inTreeName = "";
    int nTrees = 0;
    TFile* inputFile  = new TFile(gInFileNames[0]);
    TList* fileList = inputFile->GetListOfKeys();
    for (int i = 0; i < fileList->GetEntries(); i++){
      TString treeName(fileList->At(i)->GetName());
      if (treeName.Contains("_Tree")){
        nTrees++;
        if (nTrees == 1) inTreeName = treeName;
        if (nTrees >= 2){
          cout << "WARNING: found more than one tree -- only converting the first" << endl;
          cout << "           extra tree = " << treeName << endl;
        }
      }
    }
    if (nTrees == 0){
      cout << "ERROR: could not find any trees" << endl;
      exit(0);
    }
    gInTreeFirst = (TTree*) inputFile->Get(inTreeName);
    gInTree = new TChain(inTreeName);
    for (unsigned int i = 0; i < gInFileNames.size(); i++){
      gInTree->Add(gInFileNames[i]);
    }
  }

   // **********************************************************************
   // STEP 3:  PARSE FINAL STATE INFORMATION USING THE USER INFO IN THE TREE
   // **********************************************************************

     // **********************************************************************
     // STEP 3A:  simple checks on the input root file
     // **********************************************************************

  cout << endl << endl << "DOING SIMPLE CHECKS ON THE INPUT ROOT FILE:" << endl << endl;

  double inTargetCenterZ = -1.0;
  bool gUseParticles;
  bool gUseMCParticles;
  bool gUseMCInfo;
  bool gUseKinFit;
  bool gUseKinFitVtx;
  {
    bool isMC = false;
    bool isMCAna = false;
    bool isMCGen = false;
    bool isMCGenTag = false;
    bool hasRunNumber = false;
    bool hasNumNeutralHypos = false;
    bool hasNumThrown = false;
    TObjArray* branchArray = gInTreeFirst->GetListOfBranches();
    int nBranches = gInTreeFirst->GetNbranches();
    for (int i = 0; i < nBranches; i++){
      TString branch = branchArray->At(i)->GetName();
      if (branch == "RunNumber") hasRunNumber = true;
      if (branch == "NumNeutralHypos") hasNumNeutralHypos = true;
      if (branch == "NumThrown") hasNumThrown = true;
    }
    if (hasRunNumber){ cout << "  OK: found RunNumber" << endl; }
    else { cout << "  ERROR:  could not find RunNumber" << endl; exit(0); }
    if (hasNumNeutralHypos){ cout << "  OK: found NumNeutralHypos" << endl; }
    else { cout << "  OK: no NumNeutralHypos" << endl; }
    if (hasNumThrown){ cout << "  OK: found NumThrown" << endl; }
    else { cout << "  OK: no NumThrown" << endl; }
    if (!hasNumNeutralHypos && !hasNumThrown){
      cout << "  ERROR: no NumNeutralHypos and no NumThrown" << endl; exit(0); }
    isMC    = hasNumThrown;
    isMCGen = hasNumThrown && !hasNumNeutralHypos;
    isMCAna = hasNumThrown && hasNumNeutralHypos;
    if (!isMC){ cout << "    ==> treating this as ANALYZED DATA" << endl; }
    if (isMCGen){ cout << "    ==> treating this as THROWN MC" << endl; }
    if (isMCAna){ cout << "    ==> treating this as ANALYZED MC" << endl; }
    if (gInputIsMC == 1 && !isMC){
      cout << "  OVERRIDING ERROR: format incompatible with MC" << endl; exit(0); }
    if (gInputIsMC == 0 && isMCGen){
      cout << "  OVERRIDING ERROR: format incompatible with ANALYZED DATA" << endl; exit(0); }
    if (gInputIsMC == 0 && isMCAna){
      isMC = false; isMCGen = false; isMCAna = false;
      cout << "    OVERRIDING: treating this as ANALYZED DATA instead" << endl; }
    if (isMCGen && gMCTag != "") isMCGenTag = true;
    gUseParticles = (isMCAna || !isMC);
    gUseMCParticles = (isMCAna || isMCGenTag);
    gUseMCInfo = (isMC);
    cout << "      info to include in output..." << endl;
    cout << "          particle info:  ";
      if (gUseParticles){ cout << "YES"; } else{ cout << "NO"; } cout << endl;
    cout << "          MC particle info:  ";
      if (gUseMCParticles){ cout << "YES"; } else{ cout << "NO"; } cout << endl;
    cout << "          MC info:  ";
      if (gUseMCInfo){ cout << "YES"; } else{ cout << "NO"; } cout << endl;
    TList* userInfo = gInTreeFirst->GetUserInfo();
        if (userInfo){ cout << "  OK: found UserInfo" << endl; }
        else { cout << "  ERROR:  could not find UserInfo" << endl; exit(0); }
    if (gUseParticles){
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
          if (kinFitType && kinFitType->GetString() != "")
               { cout << "  OK: found KinFitType = " << kinFitType->GetString() << endl; }
          else { cout << "  ERROR:  could not find KinFitType" << endl; exit(0); }
      gUseKinFit = (kinFitType->GetString() == "1") || (kinFitType->GetString() == "4");
      gUseKinFitVtx = (kinFitType->GetString() == "4");
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
  }


     // **********************************************************************
     // STEP 3B:  extract particle names from the root file
     // **********************************************************************

  cout << endl << endl << "READING PARTICLE NAMES FROM THE ROOT FILE:" << endl << endl;

  map< TString, vector<TString> > gDecayProductMap;  // from mothers to daughters (glueXNames)
  if (gUseParticles){
    TList* userInfo = gInTreeFirst->GetUserInfo();
    vector<TString> eraseVector; // (to remove double-counting)
    TList* rootMothers = (TList*) userInfo->FindObject("ParticleNameList");
    TMap* rootDecayProductMap = (TMap*) userInfo->FindObject("DecayProductMap");
      // print the contents of ParticleNameList and DecayProductMap to the screen
    for (int i = 0; i < rootMothers->GetEntries(); i++){
      TObjString* rootMother = (TObjString*) rootMothers->At(i);
      TString motherFSType = FSParticleType(rootMother->GetString());
      cout << rootMother->GetString() << ": " << motherFSType << endl;
      TList* rootDaughters = (TList*) rootDecayProductMap->GetValue(rootMother->GetString());
      if (rootDaughters){
        for (int j = 0; j < rootDaughters->GetEntries(); j++){
          TObjString* rootDaughter = (TObjString*) rootDaughters->At(j);
          TString daughterFSType = FSParticleType(rootDaughter->GetString());
          cout << "    " << rootDaughter->GetString() << ": " << daughterFSType << endl;
        }
      }
    }
      // try to fill gDecayProductMap
    for (int i = 0; i < rootMothers->GetEntries(); i++){
      TObjString* rootMother = (TObjString*) rootMothers->At(i);
      TString motherFSType = FSParticleType(rootMother->GetString());
      TList* rootDaughters = (TList*) rootDecayProductMap->GetValue(rootMother->GetString());
      vector<TString> daughters;
      if (rootDaughters){
        if (rootDaughters->GetEntries() == 2){
          TObjString* rootDaughter1 = (TObjString*) rootDaughters->At(0);
          TObjString* rootDaughter2 = (TObjString*) rootDaughters->At(1);
          TString daughter1FSType = FSParticleType(rootDaughter1->GetString());
          TString daughter2FSType = FSParticleType(rootDaughter2->GetString());
          if (((motherFSType == "pi0")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && gFlattenpi0) ||
              ((motherFSType == "eta")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && gFlatteneta))
            eraseVector.push_back(rootMother->GetString());
          if (((motherFSType == "pi0")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && !gFlattenpi0) ||
              ((motherFSType == "eta")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && !gFlatteneta) ||
              ((motherFSType == "Ks")      && (daughter1FSType == "pi+")   && (daughter2FSType == "pi-"))   ||
              ((motherFSType == "Ks")      && (daughter1FSType == "pi-")   && (daughter2FSType == "pi+"))   ||
              ((motherFSType == "Lambda")  && (daughter1FSType == "p+")    && (daughter2FSType == "pi-"))   ||
              ((motherFSType == "Lambda")  && (daughter1FSType == "pi-")   && (daughter2FSType == "p+"))    ||
              ((motherFSType == "ALambda") && (daughter1FSType == "p-")    && (daughter2FSType == "pi+"))   ||
              ((motherFSType == "ALambda") && (daughter1FSType == "pi+")   && (daughter2FSType == "p-"))){
            daughters.push_back(rootDaughter1->GetString());
            daughters.push_back(rootDaughter2->GetString());
            eraseVector.push_back(rootDaughter1->GetString());
            eraseVector.push_back(rootDaughter2->GetString());
          }
        }
      }
      else if (rootMother->GetString().Contains("Decaying")){
        if (i < rootMothers->GetEntries()-2){
          TObjString* rootDaughter1 = (TObjString*) rootMothers->At(i+1);
          TObjString* rootDaughter2 = (TObjString*) rootMothers->At(i+2);
          TString daughter1FSType = FSParticleType(rootDaughter1->GetString());
          TString daughter2FSType = FSParticleType(rootDaughter2->GetString());
          if (((motherFSType == "pi0")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && gFlattenpi0) ||
              ((motherFSType == "eta")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && gFlatteneta))
            eraseVector.push_back(rootMother->GetString());
          if (((motherFSType == "pi0")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && !gFlattenpi0) ||
              ((motherFSType == "eta")     && (daughter1FSType == "gamma") && (daughter2FSType == "gamma") && !gFlatteneta) ||
              ((motherFSType == "Ks")      && (daughter1FSType == "pi+")   && (daughter2FSType == "pi-"))   ||
              ((motherFSType == "Ks")      && (daughter1FSType == "pi-")   && (daughter2FSType == "pi+"))   ||
              ((motherFSType == "Lambda")  && (daughter1FSType == "p+")    && (daughter2FSType == "pi-"))   ||
              ((motherFSType == "Lambda")  && (daughter1FSType == "pi-")   && (daughter2FSType == "p+"))    ||
              ((motherFSType == "ALambda") && (daughter1FSType == "p-")    && (daughter2FSType == "pi+"))   ||
              ((motherFSType == "ALambda") && (daughter1FSType == "pi+")   && (daughter2FSType == "p-"))){
            daughters.push_back(rootDaughter1->GetString());
            daughters.push_back(rootDaughter2->GetString());
            eraseVector.push_back(rootDaughter1->GetString());
            eraseVector.push_back(rootDaughter2->GetString());
            i += 2;
          }
        }
      }
      if (!rootMother->GetString().Contains("Decaying") || daughters.size() > 0)
        gDecayProductMap[rootMother->GetString()] = daughters;
    }
    for (unsigned int i = 0; i < eraseVector.size(); i++){
      gDecayProductMap.erase(eraseVector[i]);
    }
  }
  if (!gUseParticles && gUseMCParticles){
    cout << "  skipping, setting particles using mctag instead" << endl;
    gDecayProductMap = GlueXDecayProductMap(gMCTagDecayCode1.Atoi(),gMCTagDecayCode2.Atoi());
  }
  if (gUseParticles || gUseMCParticles){
    if (gDecayProductMap.size() == 0){
      cout << endl << "  ERROR: no final state partices found" << endl;
      exit(0);
    }
  }

     // **********************************************************************
     // STEP 3C:  perform checks on the final state
     // **********************************************************************

  cout << endl << endl << "PERFORMING CHECKS ON THE FINAL STATE:" << endl << endl;

  bool gCheckFSOkay = true;
  if (gDecayProductMap.size() == 0){ cout << "  skipping, no final state particles" << endl; }
  else{
    for (map<TString, vector<TString> >::const_iterator mItr = gDecayProductMap.begin();
         mItr != gDecayProductMap.end(); mItr++){
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
        cout << "  ERROR: unrecognized pi0 decay" << endl;  gCheckFSOkay = false;
      }
      if (motherFSType == "eta" && (daughterNames.size() != 2 ||
            !(daughterFSTypes[0] == "gamma" && daughterFSTypes[1] == "gamma"))){
        cout << "  ERROR: unrecognized eta decay" << endl;  gCheckFSOkay = false;
      }
      if (motherFSType == "Ks" && (daughterNames.size() != 2 ||
            !((daughterFSTypes[0] == "pi+" && daughterFSTypes[1] == "pi-") ||
              (daughterFSTypes[1] == "pi+" && daughterFSTypes[0] == "pi-")))){
        cout << "  ERROR: unrecognized Ks decay" << endl;  gCheckFSOkay = false;
      }
      if (motherFSType == "Lambda" && (daughterNames.size() != 2 ||
            !((daughterFSTypes[0] == "p+" && daughterFSTypes[1] == "pi-") ||
              (daughterFSTypes[1] == "p+" && daughterFSTypes[0] == "pi-")))){
        cout << "  ERROR: unrecognized Lambda decay" << endl;  gCheckFSOkay = false;
      }
      if (motherFSType == "ALambda" && (daughterNames.size() != 2 ||
            !((daughterFSTypes[0] == "p-" && daughterFSTypes[1] == "pi+") ||
              (daughterFSTypes[1] == "p-" && daughterFSTypes[0] == "pi+")))){
        cout << "  ERROR: unrecognized ALambda decay" << endl;  gCheckFSOkay = false;
      }
      if (motherName.Contains("Decaying") && motherFSType != "pi0"
                                          && motherFSType != "eta"
                                          && motherFSType != "Ks"
                                          && motherFSType != "Lambda"
                                          && motherFSType != "ALambda"){
        cout << "  WARNING: unrecognized decaying particle: " << motherName << endl;
        cout << "     DOUBLE-CHECK all the parsing and ordering below" << endl;
      }
    }
  }


     // **********************************************************************
     // STEP 3D:  extract PDG numbers from the root file (not used, just checking)
     // **********************************************************************

  cout << endl << endl << "READING PDG NUMBERS FROM THE ROOT FILE:" << endl << endl;

  if (!gUseParticles){ cout << "  skipping, not using particle information" << endl; }
  else{
    TList* userInfo = gInTreeFirst->GetUserInfo();
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
     // STEP 3E:  put the particle names in the right order
     // **********************************************************************

  cout << endl << endl << "PUTTING PARTICLES IN THE RIGHT ORDER AND SETTING INDICES:" << endl << endl;

  vector< vector<TString> > gOrderedParticleNames;  // (glueXNames)
  if (gDecayProductMap.size() == 0){ cout << "  skipping, no final state particles" << endl; }
  else{
    for (map<TString, vector<TString> >::const_iterator mItr = gDecayProductMap.begin();
         mItr != gDecayProductMap.end(); mItr++){
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
        gOrderedParticleNames.push_back(vp);
      }
    }
    for (unsigned int i = 0; i < gOrderedParticleNames.size(); i++){
      for (unsigned int j = 0; j < gOrderedParticleNames.size()-1; j++){
        if (FSParticleOrder(gOrderedParticleNames[j+1][0]) >
            FSParticleOrder(gOrderedParticleNames[j][0])){
          vector<TString> temp = gOrderedParticleNames[j+1];
          gOrderedParticleNames[j+1] = gOrderedParticleNames[j];
          gOrderedParticleNames[j] = temp;
        }
      }
    }
  }
  pair<int,int> gReconstructedFSCode = FSCode(gOrderedParticleNames);
  int gNumFSNeutrals = 0;
  for (unsigned int i = 0; i < gOrderedParticleNames.size(); i++){
    gNumFSNeutrals += GlueXNeutralsCounter(gOrderedParticleNames[i][0]);
  }

  TString gOutTreeName = "ntFSGlueX";
  gOutTreeName += "_";
  gOutTreeName += gReconstructedFSCode.second;
  gOutTreeName += "_";
  gOutTreeName += gReconstructedFSCode.first;

  cout << endl;
  cout << "  DecayCode1    = " << gReconstructedFSCode.first << endl;
  cout << "  DecayCode2    = " << gReconstructedFSCode.second << endl;
  cout << "  NumFSNeutrals = " << gNumFSNeutrals << endl;
  cout << "  OutTreeName   = " << gOutTreeName << endl << endl;

  if (!gCheckFSOkay){
    cout << "ERROR: problem parsing this final state." << endl;
    exit(0);
  }

     // **********************************************************************
     // STEP 3F:  make maps from names to indices
     // **********************************************************************

  map<TString, TString> gMapGlueXNameToFSIndex;
  map<TString, int> gMapGlueXNameToParticleIndex;
  {
    int particleIndex = 0;
    for (unsigned int im = 0; im < gOrderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < gOrderedParticleNames[im].size(); id++){
      TString name = gOrderedParticleNames[im][id];
      TString fsIndex("");  fsIndex += (im+1);
      if (id == 1) fsIndex += "a";
      if (id == 2) fsIndex += "b";
      cout << fsIndex << ". ";
      cout << name << " ";
      gMapGlueXNameToFSIndex[name] = fsIndex;
      gMapGlueXNameToParticleIndex[name] = particleIndex++;
      cout << "(" << gMapGlueXNameToParticleIndex[name] << ")   ";
    }
    cout << endl;
    }
    cout << endl;
  }



   // **********************************************************************
   // STEP 4:  SET UP TO READ THE INPUT TREE (IN ANALYSIS TREE FORMAT)
   // **********************************************************************

   if (!gOutputFile) return 0;

        // ******************************
        // ***** 4A. SIMULATED DATA *****
        // ******************************

        //   *** Thrown Non-Particle Data ***

  UInt_t inNumThrown = 0;
      TBranch* brNumThrown = nullptr;
      if (gUseMCInfo) gInTree->SetBranchAddress("NumThrown", &inNumThrown, &brNumThrown);


        //   *** Thrown Beam Particle ***

  TLorentzVector* inThrownBeam__X4 = NULL;
      if (gUseMCInfo) gInTree->SetBranchAddress("ThrownBeam__X4", &inThrownBeam__X4);
  TLorentzVector* inThrownBeam__P4 = NULL;
      if (gUseMCInfo) gInTree->SetBranchAddress("ThrownBeam__P4", &inThrownBeam__P4);
  Float_t inThrownBeam__GeneratedEnergy = -1.0;
      if (gUseMCInfo) gInTree->SetBranchAddress("ThrownBeam__GeneratedEnergy", &inThrownBeam__GeneratedEnergy);


        //   *** Thrown Products ***

  Int_t  inThrown__ParentIndex[MAXTHROWN] = {};
      if (gUseMCInfo) gInTree->SetBranchAddress("Thrown__ParentIndex", inThrown__ParentIndex);
  Int_t  inThrown__PID[MAXTHROWN] = {};
      if (gUseMCInfo) gInTree->SetBranchAddress("Thrown__PID", inThrown__PID);
  Int_t  inThrown__MatchID[MAXTHROWN] = {};
      if (gUseMCParticles&&gUseParticles) gInTree->SetBranchAddress("Thrown__MatchID", inThrown__MatchID);
  Float_t  inThrown__MatchFOM[MAXTHROWN] = {};
      if (gUseMCParticles&&gUseParticles) gInTree->SetBranchAddress("Thrown__MatchFOM", inThrown__MatchFOM);
  TClonesArray *inThrown__P4 = NULL;
      if (gUseMCParticles) inThrown__P4 = new TClonesArray("TLorentzVector", MAXTHROWN);
      if (gUseMCParticles) gInTree->GetBranch       ("Thrown__P4")->SetAutoDelete(kFALSE);
      if (gUseMCParticles) gInTree->SetBranchAddress("Thrown__P4",&(inThrown__P4));
  TClonesArray *inThrown__X4 = NULL;
      if (gUseMCParticles) inThrown__X4 = new TClonesArray("TLorentzVector", MAXTHROWN);
      if (gUseMCParticles) gInTree->GetBranch       ("Thrown__X4")->SetAutoDelete(kFALSE);
      if (gUseMCParticles) gInTree->SetBranchAddress("Thrown__X4",&(inThrown__X4));



        // **************************************
        // ***** 4B. COMBO-INDEPENDENT DATA *****
        // **************************************

        //   *** Non-Particle Data ***

  UInt_t inRunNumber = 0;
      TBranch* brRunNumber = nullptr;
      gInTree->SetBranchAddress("RunNumber", &inRunNumber,&brRunNumber);
  ULong64_t inEventNumber = 0;
      TBranch* brEventNumber = nullptr;
      gInTree->SetBranchAddress("EventNumber", &inEventNumber,&brEventNumber);
  TLorentzVector* inX4_Production = NULL;
      if (gUseParticles) gInTree->SetBranchAddress("X4_Production", &inX4_Production);
  UInt_t inNumBeam = 0;
      TBranch* brNumBeam = nullptr;
      if (gUseParticles) gInTree->SetBranchAddress("NumBeam", &inNumBeam,&brNumBeam);
  UInt_t inNumChargedHypos = 0;
      TBranch* brNumChargedHypos = nullptr;
      if (gUseParticles) gInTree->SetBranchAddress("NumChargedHypos", &inNumChargedHypos,&brNumChargedHypos);
  UInt_t inNumNeutralHypos = 0;
      TBranch* brNumNeutralHypos = nullptr;
      if (gUseParticles) gInTree->SetBranchAddress("NumNeutralHypos", &inNumNeutralHypos,&brNumNeutralHypos);
  UInt_t inNumCombos = 0;
      TBranch* brNumCombos = nullptr;
      if (gUseParticles) gInTree->SetBranchAddress("NumCombos", &inNumCombos,&brNumCombos);
  UChar_t inNumUnusedTracks = 0;
      TBranch* brNumUnusedTracks = nullptr;
      if (gUseParticles) gInTree->SetBranchAddress("NumUnusedTracks", &inNumUnusedTracks,&brNumUnusedTracks);
  Bool_t inIsThrownTopology = false;
      if (gUseParticles&&gUseMCParticles) gInTree->SetBranchAddress("IsThrownTopology", &inIsThrownTopology);


        //   *** Beam Particles (indexed using ComboBeam__BeamIndex) ***

  TClonesArray *inBeam__P4_Measured;
      if (gUseParticles) inBeam__P4_Measured = new TClonesArray("TLorentzVector",MAXBEAM);
      if (gUseParticles) gInTree->GetBranch       ("Beam__P4_Measured")->SetAutoDelete(kFALSE);
      if (gUseParticles) gInTree->SetBranchAddress("Beam__P4_Measured", &(inBeam__P4_Measured));
  TClonesArray *inBeam__X4_Measured;
      if (gUseParticles) inBeam__X4_Measured = new TClonesArray("TLorentzVector",MAXBEAM);
      if (gUseParticles) gInTree->GetBranch       ("Beam__X4_Measured")->SetAutoDelete(kFALSE);
      if (gUseParticles) gInTree->SetBranchAddress("Beam__X4_Measured", &(inBeam__X4_Measured));


        //   *** Charged Track Hypotheses (indexed using <particleName>__ChargedIndex) ***

  TClonesArray *inChargedHypo__P4_Measured;
      if (gUseParticles) inChargedHypo__P4_Measured = new TClonesArray("TLorentzVector",MAXTRACKS);
      if (gUseParticles) gInTree->GetBranch       ("ChargedHypo__P4_Measured")->SetAutoDelete(kFALSE);
      if (gUseParticles) gInTree->SetBranchAddress("ChargedHypo__P4_Measured",&(inChargedHypo__P4_Measured));
  TClonesArray *inChargedHypo__X4_Measured;
      if (gUseParticles) inChargedHypo__X4_Measured = new TClonesArray("TLorentzVector",MAXTRACKS);
      if (gUseParticles) gInTree->GetBranch       ("ChargedHypo__X4_Measured")->SetAutoDelete(kFALSE);
      if (gUseParticles) gInTree->SetBranchAddress("ChargedHypo__X4_Measured",&(inChargedHypo__X4_Measured));
  Float_t inChargedHypo__ChiSq_Tracking[MAXTRACKS] = {};
      if (gUseParticles) gInTree->SetBranchAddress("ChargedHypo__ChiSq_Tracking", inChargedHypo__ChiSq_Tracking);
  UInt_t  inChargedHypo__NDF_Tracking[MAXTRACKS] = {};
      if (gUseParticles) gInTree->SetBranchAddress("ChargedHypo__NDF_Tracking", inChargedHypo__NDF_Tracking);
  UInt_t  inChargedHypo__NDF_DCdEdx[MAXTRACKS] = {};
      if (gUseParticles && gAddPID) gInTree->SetBranchAddress("ChargedHypo__NDF_DCdEdx", inChargedHypo__NDF_DCdEdx);
  Float_t inChargedHypo__ChiSq_DCdEdx[MAXTRACKS] = {};
      if (gUseParticles && gAddPID) gInTree->SetBranchAddress("ChargedHypo__ChiSq_DCdEdx", inChargedHypo__ChiSq_DCdEdx);
  Float_t inChargedHypo__dEdx_CDC[MAXTRACKS] = {};
      if (gUseParticles && gAddPID) gInTree->SetBranchAddress("ChargedHypo__dEdx_CDC", inChargedHypo__dEdx_CDC);
  Float_t inChargedHypo__dEdx_FDC[MAXTRACKS] = {};
      if (gUseParticles && gAddPID) gInTree->SetBranchAddress("ChargedHypo__dEdx_FDC", inChargedHypo__dEdx_FDC);

  Float_t inChargedHypo__Lpi_DIRC[MAXTRACKS] = {};
  Float_t inChargedHypo__Lele_DIRC[MAXTRACKS] = {};
  Float_t inChargedHypo__Lk_DIRC[MAXTRACKS] = {};
  Float_t inChargedHypo__Lp_DIRC[MAXTRACKS] = {};
  Int_t   inChargedHypo__NumPhotons_DIRC[MAXTRACKS] = {};
  Float_t inChargedHypo__x_DIRC[MAXTRACKS] = {};
  Float_t inChargedHypo__y_DIRC[MAXTRACKS] = {};



  if (gUseParticles && gUseDIRC){
    // this one is a test case to see if the DIRC info is included in the tree
    int flag = gInTree->SetBranchAddress("ChargedHypo__Lpi_DIRC",inChargedHypo__Lpi_DIRC);
    if(flag!=0){
      cout << "SetBranchAddress for DIRC returned " << flag << endl;
      cout << "Switching DIRC output off" << endl;
      gUseDIRC=false;
    }
    else{
      gInTree->SetBranchAddress("ChargedHypo__Lele_DIRC",inChargedHypo__Lele_DIRC);
      gInTree->SetBranchAddress("ChargedHypo__Lk_DIRC",inChargedHypo__Lk_DIRC);
      gInTree->SetBranchAddress("ChargedHypo__Lp_DIRC",inChargedHypo__Lp_DIRC);
      gInTree->SetBranchAddress("ChargedHypo__NumPhotons_DIRC",inChargedHypo__NumPhotons_DIRC);
      gInTree->SetBranchAddress("ChargedHypo__ExtrapolatedX_DIRC",inChargedHypo__x_DIRC);
      gInTree->SetBranchAddress("ChargedHypo__ExtrapolatedY_DIRC",inChargedHypo__y_DIRC);
    }
  }


        //   *** Neutral Particle Hypotheses (indexed using <particleName>__NeutralIndex) ***

  TClonesArray *inNeutralHypo__P4_Measured;
      if (gUseParticles) inNeutralHypo__P4_Measured = new TClonesArray("TLorentzVector",MAXNEUTRALS);
      if (gUseParticles) gInTree->GetBranch       ("NeutralHypo__P4_Measured")->SetAutoDelete(kFALSE);
      if (gUseParticles) gInTree->SetBranchAddress("NeutralHypo__P4_Measured",&(inNeutralHypo__P4_Measured));
  TClonesArray *inNeutralHypo__X4_Measured;
      if (gUseParticles) inNeutralHypo__X4_Measured = new TClonesArray("TLorentzVector",MAXNEUTRALS);
      if (gUseParticles) gInTree->GetBranch       ("NeutralHypo__X4_Measured")->SetAutoDelete(kFALSE);
      if (gUseParticles) gInTree->SetBranchAddress("NeutralHypo__X4_Measured",&(inNeutralHypo__X4_Measured));
  Float_t inNeutralHypo__ShowerQuality[MAXNEUTRALS] = {};
      if (gUseParticles) gInTree->SetBranchAddress("NeutralHypo__ShowerQuality", inNeutralHypo__ShowerQuality);


        // ************************************
        // ***** 4C. COMBO-DEPENDENT DATA *****
        // ************************************

        //   *** Particle-Independent Data (indexed by combo) ***

  Float_t inRFTime_Measured[MAXCOMBOS] = {};
      if (gUseParticles) gInTree->SetBranchAddress("RFTime_Measured", inRFTime_Measured);
  //Float_t inRFTime_KinFit[MAXCOMBOS] = {};
  //    if (gUseParticles && gUseKinFit) gInTree->SetBranchAddress("RFTime_KinFit", inRFTime_KinFit);
  Float_t inChiSq_KinFit[MAXCOMBOS] = {};
      if (gUseParticles && gUseKinFit) gInTree->SetBranchAddress("ChiSq_KinFit", inChiSq_KinFit);
  UInt_t inNDF_KinFit[MAXCOMBOS] = {};
      if (gUseParticles && gUseKinFit) gInTree->SetBranchAddress("NDF_KinFit", inNDF_KinFit);
  Float_t inEnergy_Unused[MAXCOMBOS] = {};
      if (gUseParticles) gInTree->SetBranchAddress("Energy_UnusedShowers", inEnergy_Unused);


        //   *** Combo Beam Particles (indexed by combo) ***

  Int_t inBeamIndex[MAXCOMBOS] = {};
      if (gUseParticles) gInTree->SetBranchAddress("ComboBeam__BeamIndex", inBeamIndex);
  TClonesArray *inBeam__P4_KinFit;
      if (gUseParticles && gUseKinFit) inBeam__P4_KinFit = new TClonesArray("TLorentzVector",MAXCOMBOS);
      if (gUseParticles && gUseKinFit) gInTree->GetBranch       ("ComboBeam__P4_KinFit")->SetAutoDelete(kFALSE);
      if (gUseParticles && gUseKinFit) gInTree->SetBranchAddress("ComboBeam__P4_KinFit", &(inBeam__P4_KinFit));
  TClonesArray *inBeam__X4_KinFit;
      if (gUseParticles && gUseKinFitVtx) inBeam__X4_KinFit = new TClonesArray("TLorentzVector",MAXCOMBOS);
      if (gUseParticles && gUseKinFitVtx) gInTree->GetBranch       ("ComboBeam__X4_KinFit")->SetAutoDelete(kFALSE);
      if (gUseParticles && gUseKinFitVtx) gInTree->SetBranchAddress("ComboBeam__X4_KinFit", &(inBeam__X4_KinFit));


        //   *** Combo Tracks ***
        //   *** Combo Neutrals ***
        //   *** Combo Decaying Particles ***
        //     (all indexed by particleIndex and combo)

  TClonesArray *inP4_KinFit[MAXPARTICLES] = {};
  TClonesArray *inX4_KinFit[MAXPARTICLES] = {};
  Int_t inChargedIndex[MAXPARTICLES][MAXCOMBOS] = {};
  Float_t inBeta_Timing[MAXPARTICLES][MAXCOMBOS] = {};
  Float_t inChiSq_Timing[MAXPARTICLES][MAXCOMBOS] = {};
  Int_t inNeutralIndex[MAXPARTICLES][MAXCOMBOS] = {};
  TClonesArray *inX4[MAXPARTICLES] = {};
  Float_t inPathLengthSigma[MAXPARTICLES][MAXCOMBOS] = {};
  {
    for (unsigned int im = 0; im < gOrderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < gOrderedParticleNames[im].size(); id++){
      TString name = gOrderedParticleNames[im][id];
      int pIndex = gMapGlueXNameToParticleIndex[name];

        //   *** Combo Tracks ***

      if (GlueXParticleClass(name) == "Charged"){
        TString var_P4_KinFit(name); var_P4_KinFit += "__P4_KinFit";
            if (gUseParticles && gUseKinFit) inP4_KinFit[pIndex] = new TClonesArray("TLorentzVector",MAXCOMBOS);
            if (gUseParticles && gUseKinFit) gInTree->GetBranch       (var_P4_KinFit)->SetAutoDelete(kFALSE);
            if (gUseParticles && gUseKinFit) gInTree->SetBranchAddress(var_P4_KinFit,&(inP4_KinFit[pIndex]));
        TString var_X4_KinFit(name); var_X4_KinFit += "__X4_KinFit";
            if (gUseParticles && gUseKinFitVtx) inX4_KinFit[pIndex] = new TClonesArray("TLorentzVector",MAXCOMBOS);
            if (gUseParticles && gUseKinFitVtx) gInTree->GetBranch       (var_X4_KinFit)->SetAutoDelete(kFALSE);
            if (gUseParticles && gUseKinFitVtx) gInTree->SetBranchAddress(var_X4_KinFit,&(inX4_KinFit[pIndex]));
        TString var_ChargedIndex(name);  var_ChargedIndex += "__ChargedIndex";
            if (gUseParticles) gInTree->SetBranchAddress(var_ChargedIndex,inChargedIndex[pIndex]);
        TString var_Beta_Timing(name);
            if (gUseParticles && !gUseKinFit) var_Beta_Timing += "__Beta_Timing_Measured";
            if (gUseParticles &&  gUseKinFit) var_Beta_Timing += "__Beta_Timing_KinFit";
            if (gUseParticles && gAddPID) gInTree->SetBranchAddress(var_Beta_Timing,inBeta_Timing[pIndex]);
        TString var_ChiSq_Timing(name);
            if (gUseParticles && !gUseKinFit) var_ChiSq_Timing += "__ChiSq_Timing_Measured";
            if (gUseParticles &&  gUseKinFit) var_ChiSq_Timing += "__ChiSq_Timing_KinFit";
            if (gUseParticles && gAddPID) gInTree->SetBranchAddress(var_ChiSq_Timing,inChiSq_Timing[pIndex]);
      }

        //   *** Combo Neutrals ***

      if (GlueXParticleClass(name) == "Neutral"){
        TString var_P4_KinFit(name); var_P4_KinFit += "__P4_KinFit";
            if (gUseParticles && gUseKinFit) inP4_KinFit[pIndex] = new TClonesArray("TLorentzVector",MAXCOMBOS);
            if (gUseParticles && gUseKinFit) gInTree->GetBranch       (var_P4_KinFit)->SetAutoDelete(kFALSE);
            if (gUseParticles && gUseKinFit) gInTree->SetBranchAddress(var_P4_KinFit,&(inP4_KinFit[pIndex]));
        TString var_X4_KinFit(name); var_X4_KinFit += "__X4_KinFit";
            if (gUseParticles && gUseKinFitVtx) inX4_KinFit[pIndex] = new TClonesArray("TLorentzVector",MAXCOMBOS);
            if (gUseParticles && gUseKinFitVtx) gInTree->GetBranch       (var_X4_KinFit)->SetAutoDelete(kFALSE);
            if (gUseParticles && gUseKinFitVtx) gInTree->SetBranchAddress(var_X4_KinFit,&(inX4_KinFit[pIndex]));
        TString var_NeutralIndex(name);  var_NeutralIndex += "__NeutralIndex";
            if (gUseParticles) gInTree->SetBranchAddress(var_NeutralIndex,inNeutralIndex[pIndex]);
      }

        //   *** Combo Decaying Particles ***

      if (GlueXParticleClass(name).Contains("Decaying")){
      }

      if (GlueXParticleClass(name) == "DecayingToCharged"){
        TString var_X4(name); var_X4 += "__X4";
            if (gUseParticles && gUseKinFitVtx) inX4[pIndex] = new TClonesArray("TLorentzVector",MAXCOMBOS);
            if (gUseParticles && gUseKinFitVtx) gInTree->GetBranch       (var_X4)->SetAutoDelete(kFALSE);
            if (gUseParticles && gUseKinFitVtx) gInTree->SetBranchAddress(var_X4,&(inX4[pIndex]));
        TString var_PathLengthSigma(name);  var_PathLengthSigma += "__PathLengthSigma";
            if (gUseParticles && gUseKinFitVtx) gInTree->SetBranchAddress(var_PathLengthSigma,inPathLengthSigma[pIndex]);
      }

    }
    }
  }


   // **********************************************************************
   // STEP 5:  SET UP THE OUTPUT TREE INFORMATION (IN FSROOT FORMAT)
   // **********************************************************************

  gOutputFile->cd();
  TTree *gOutTree = new TTree(gOutTreeName, gOutTreeName);
  if (!gOutTree){
    cout << "ERROR:  problem with the output tree." << endl;
    exit(0);
  }

    // non-particle information

  double outRunNumber;                           gOutTree->Branch("Run",             &outRunNumber);
  double outEventNumber;                         gOutTree->Branch("Event",           &outEventNumber);
  double outChi2;             if (gUseParticles && gUseKinFit)
                                                 gOutTree->Branch("Chi2",            &outChi2);
  double outChi2DOF;          if (gUseParticles && gUseKinFit)
                                                 gOutTree->Branch("Chi2DOF",         &outChi2DOF);
  double outRFTime;           if (gUseParticles) gOutTree->Branch("RFTime",          &outRFTime);
  double outRFDeltaT;         if (gUseParticles) gOutTree->Branch("RFDeltaT",        &outRFDeltaT);
  double outEnUnusedSh;       if (gUseParticles) gOutTree->Branch("EnUnusedSh",      &outEnUnusedSh);
  double outNumUnusedTracks;  if (gUseParticles) gOutTree->Branch("NumUnusedTracks", &outNumUnusedTracks);
  double outNumNeutralHypos;  if (gUseParticles) gOutTree->Branch("NumNeutralHypos", &outNumNeutralHypos);
  double outPolarization;     if (gUsePolarization) gOutTree->Branch("PolarizationAngle", &outPolarization);
  double outNumBeam;          if (gUseParticles) gOutTree->Branch("NumBeam",         &outNumBeam);
  double outNumCombos;        if (gUseParticles) gOutTree->Branch("NumCombos",       &outNumCombos);
  double outProdVx;           if (gUseParticles) gOutTree->Branch("ProdVx",          &outProdVx);
  double outProdVy;           if (gUseParticles) gOutTree->Branch("ProdVy",          &outProdVy);
  double outProdVz;           if (gUseParticles) gOutTree->Branch("ProdVz",          &outProdVz);
  double outProdVt;           if (gUseParticles) gOutTree->Branch("ProdVt",          &outProdVt);
  double outPxPB;             if (gUseParticles && gUseKinFit)
                                                 gOutTree->Branch("PxPB",            &outPxPB);
  double outPyPB;             if (gUseParticles && gUseKinFit)
                                                 gOutTree->Branch("PyPB",            &outPyPB);
  double outPzPB;             if (gUseParticles && gUseKinFit)
                                                 gOutTree->Branch("PzPB",            &outPzPB);
  double outEnPB;             if (gUseParticles && gUseKinFit)
                                                 gOutTree->Branch("EnPB",            &outEnPB);
  double outVxPB;             if (gUseParticles && gUseKinFitVtx)
                                                 gOutTree->Branch("VxPB",            &outVxPB);
  double outVyPB;             if (gUseParticles && gUseKinFitVtx)
                                                 gOutTree->Branch("VyPB",            &outVyPB);
  double outVzPB;             if (gUseParticles && gUseKinFitVtx)
                                                 gOutTree->Branch("VzPB",            &outVzPB);
  double outRPxPB;            if (gUseParticles) gOutTree->Branch("RPxPB",           &outRPxPB);
  double outRPyPB;            if (gUseParticles) gOutTree->Branch("RPyPB",           &outRPyPB);
  double outRPzPB;            if (gUseParticles) gOutTree->Branch("RPzPB",           &outRPzPB);
  double outREnPB;            if (gUseParticles) gOutTree->Branch("REnPB",           &outREnPB);
  double outRVxPB;            if (gUseParticles) gOutTree->Branch("RVxPB",           &outRVxPB);
  double outRVyPB;            if (gUseParticles) gOutTree->Branch("RVyPB",           &outRVyPB);
  double outRVzPB;            if (gUseParticles) gOutTree->Branch("RVzPB",           &outRVzPB);

    // MC information

  double outMCPxPB;        if (gUseMCParticles) gOutTree->Branch("MCPxPB", &outMCPxPB);
  double outMCPyPB;        if (gUseMCParticles) gOutTree->Branch("MCPyPB", &outMCPyPB);
  double outMCPzPB;        if (gUseMCParticles) gOutTree->Branch("MCPzPB", &outMCPzPB);
  double outMCEnPB;        if (gUseMCInfo) gOutTree->Branch("MCEnPB",      &outMCEnPB);
  double outMCVxPB;        if (gUseMCParticles) gOutTree->Branch("MCVxPB", &outMCVxPB);
  double outMCVyPB;        if (gUseMCParticles) gOutTree->Branch("MCVyPB", &outMCVyPB);
  double outMCVzPB;        if (gUseMCParticles) gOutTree->Branch("MCVzPB", &outMCVzPB);
  double outMCDecayCode1;  if (gUseMCInfo) gOutTree->Branch("MCDecayCode1",&outMCDecayCode1);
  double outMCDecayCode2;  if (gUseMCInfo) gOutTree->Branch("MCDecayCode2",&outMCDecayCode2);
  double outMCExtras;      if (gUseMCInfo) gOutTree->Branch("MCExtras",    &outMCExtras);
  double outMCSignal;      if (gUseMCParticles&&gUseParticles) gOutTree->Branch("MCSignal", &outMCSignal);
  double outMCDecayParticle1;   double outMCDecayParticle2;   double outMCDecayParticle3;
  double outMCDecayParticle4;   double outMCDecayParticle5;   double outMCDecayParticle6;
  if (gUseMCInfo) gOutTree->Branch("MCDecayParticle1",&outMCDecayParticle1);
  if (gUseMCInfo) gOutTree->Branch("MCDecayParticle2",&outMCDecayParticle2);
  if (gUseMCInfo) gOutTree->Branch("MCDecayParticle3",&outMCDecayParticle3);
  if (gUseMCInfo) gOutTree->Branch("MCDecayParticle4",&outMCDecayParticle4);
  if (gUseMCInfo) gOutTree->Branch("MCDecayParticle5",&outMCDecayParticle5);
  if (gUseMCInfo) gOutTree->Branch("MCDecayParticle6",&outMCDecayParticle6);

    // particle information

  double   outPx[MAXPARTICLES]={},   outPy[MAXPARTICLES]={},   outPz[MAXPARTICLES]={},   outEn[MAXPARTICLES]={};
  double   outVx[MAXPARTICLES]={},   outVy[MAXPARTICLES]={},   outVz[MAXPARTICLES]={};
  double  outRPx[MAXPARTICLES]={},  outRPy[MAXPARTICLES]={},  outRPz[MAXPARTICLES]={},  outREn[MAXPARTICLES]={};
  double  outRVx[MAXPARTICLES]={},  outRVy[MAXPARTICLES]={},  outRVz[MAXPARTICLES]={};
  double outMCPx[MAXPARTICLES]={}, outMCPy[MAXPARTICLES]={}, outMCPz[MAXPARTICLES]={}, outMCEn[MAXPARTICLES]={};
  double outMCVx[MAXPARTICLES]={}, outMCVy[MAXPARTICLES]={}, outMCVz[MAXPARTICLES]={};
  double outPxUN[MAXPARTICLES]={}, outPyUN[MAXPARTICLES]={}, outPzUN[MAXPARTICLES]={}, outEnUN[MAXPARTICLES]={};
  double outVeeL[MAXPARTICLES]={}, outVeeLSigma[MAXPARTICLES]={};
  double outTkChi2[MAXPARTICLES]={}, outTkNDF[MAXPARTICLES]={};
  double outTkDEDXChi2[MAXPARTICLES]={}, outTkDEDXNDF[MAXPARTICLES]={};
  double outTkDEDXCDC[MAXPARTICLES]={}, outTkDEDXFDC[MAXPARTICLES]={};
  double outTkTOFBeta[MAXPARTICLES]={}, outTkTOFChi2[MAXPARTICLES]={};
  double outShQuality[MAXPARTICLES]={};
  double outLpiDIRC[MAXPARTICLES]={};
  double outLpDIRC[MAXPARTICLES]={};
  double outLkDIRC[MAXPARTICLES]={};
  double outLeleDIRC[MAXPARTICLES]={};
  Int_t outNumPhotonsDIRC[MAXPARTICLES]={};
  double outXDIRC[MAXPARTICLES]={};
  double outYDIRC[MAXPARTICLES]={};
  {
    for (unsigned int im = 0; im < gOrderedParticleNames.size(); im++){
    for (unsigned int id = 0; id < gOrderedParticleNames[im].size(); id++){
      TString name = gOrderedParticleNames[im][id];
      int pIndex = gMapGlueXNameToParticleIndex[name];
      TString fsIndex = gMapGlueXNameToFSIndex[name];
      if (gUseParticles && gUseKinFit){
        TString vPx("PxP");   vPx  += fsIndex; gOutTree->Branch(vPx, &outPx [pIndex]);
        TString vPy("PyP");   vPy  += fsIndex; gOutTree->Branch(vPy, &outPy [pIndex]);
        TString vPz("PzP");   vPz  += fsIndex; gOutTree->Branch(vPz, &outPz [pIndex]);
        TString vEn("EnP");   vEn  += fsIndex; gOutTree->Branch(vEn, &outEn [pIndex]);
      }
      if (gUseParticles && gUseKinFitVtx && (GlueXParticleClass(name) == "Charged" ||
                                             GlueXParticleClass(name) == "Neutral")){
        TString vVx("VxP");   vVx  += fsIndex; gOutTree->Branch(vVx, &outVx [pIndex]);
        TString vVy("VyP");   vVy  += fsIndex; gOutTree->Branch(vVy, &outVy [pIndex]);
        TString vVz("VzP");   vVz  += fsIndex; gOutTree->Branch(vVz, &outVz [pIndex]);
      }
      if (gUseParticles){
        TString vRPx("RPxP"); vRPx += fsIndex; gOutTree->Branch(vRPx,&outRPx[pIndex]);
        TString vRPy("RPyP"); vRPy += fsIndex; gOutTree->Branch(vRPy,&outRPy[pIndex]);
        TString vRPz("RPzP"); vRPz += fsIndex; gOutTree->Branch(vRPz,&outRPz[pIndex]);
        TString vREn("REnP"); vREn += fsIndex; gOutTree->Branch(vREn,&outREn[pIndex]);
        if (GlueXParticleClass(name) == "Charged" || GlueXParticleClass(name) == "Neutral"){
          TString vRVx("RVxP"); vRVx += fsIndex; gOutTree->Branch(vRVx,&outRVx[pIndex]);
          TString vRVy("RVyP"); vRVy += fsIndex; gOutTree->Branch(vRVy,&outRVy[pIndex]);
          TString vRVz("RVzP"); vRVz += fsIndex; gOutTree->Branch(vRVz,&outRVz[pIndex]);
        }
        if (GlueXParticleClass(name) == "Charged"){
          TString vTkNDF("TkNDFP");   vTkNDF  += fsIndex; gOutTree->Branch(vTkNDF, &outTkNDF [pIndex]);
          TString vTkChi2("TkChi2P"); vTkChi2 += fsIndex; gOutTree->Branch(vTkChi2,&outTkChi2[pIndex]);
        }
        if (gAddPID && GlueXParticleClass(name) == "Charged"){
          TString vTkTOFBeta ("TkTOFBetaP");  vTkTOFBeta  += fsIndex; gOutTree->Branch(vTkTOFBeta, &outTkTOFBeta[pIndex]);
          TString vTkTOFChi2 ("TkTOFChi2P");  vTkTOFChi2  += fsIndex; gOutTree->Branch(vTkTOFChi2, &outTkTOFChi2[pIndex]);
          TString vTkDEDXChi2("TkDEDXChi2P"); vTkDEDXChi2 += fsIndex; gOutTree->Branch(vTkDEDXChi2,&outTkDEDXChi2[pIndex]);
          TString vTkDEDXNDF("TkDEDXNDFP");   vTkDEDXNDF  += fsIndex; gOutTree->Branch(vTkDEDXNDF, &outTkDEDXNDF [pIndex]);
          TString vTkDEDXCDC("TkDEDXCDCP");   vTkDEDXCDC  += fsIndex; gOutTree->Branch(vTkDEDXCDC, &outTkDEDXCDC [pIndex]);
          TString vTkDEDXFDC("TkDEDXFDCP");   vTkDEDXFDC  += fsIndex; gOutTree->Branch(vTkDEDXFDC, &outTkDEDXFDC [pIndex]);
        }
        if(gUseDIRC && GlueXParticleClass(name)=="Charged"){
          TString vLpiDIRC("TkLpiDIRCP");        vLpiDIRC    += fsIndex; gOutTree->Branch(vLpiDIRC,   &outLpiDIRC[pIndex]);
          TString vLpDIRC("TkLpDIRCP");          vLpDIRC     += fsIndex; gOutTree->Branch(vLpDIRC,    &outLpDIRC[pIndex]);
          TString vLkDIRC("TkLkDIRCP");         vLkDIRC     += fsIndex; gOutTree->Branch(vLkDIRC,    &outLkDIRC[pIndex]);
          TString vLeleDIRC("TkLeleDIRCP");        vLeleDIRC   += fsIndex; gOutTree->Branch(vLeleDIRC,  &outLeleDIRC[pIndex]);
          TString vNumPhotonsDIRC("TkNumPhotonsDIRCP"); vNumPhotonsDIRC += fsIndex; gOutTree->Branch(vNumPhotonsDIRC,   &outNumPhotonsDIRC[pIndex]);
          TString vXDIRC("TkXDIRCP"); vXDIRC+=fsIndex; gOutTree->Branch(vXDIRC, &outXDIRC[pIndex]);
          TString vYDIRC("TkYDIRCP"); vYDIRC+=fsIndex; gOutTree->Branch(vYDIRC, &outYDIRC[pIndex]);
        }
        if (GlueXParticleClass(name) == "Neutral"){
          TString vQual("ShQualityP"); vQual += fsIndex; gOutTree->Branch(vQual, &outShQuality[pIndex]);
        }
        for (unsigned int iun = 0; iun < gAddUnusedNeutrals; iun++){
          TString siun(""); siun += (iun+1); 
          TString vPx("PxPUN");   vPx += siun;  gOutTree->Branch(vPx, &outPxUN [iun]);
          TString vPy("PyPUN");   vPy += siun;  gOutTree->Branch(vPy, &outPyUN [iun]);
          TString vPz("PzPUN");   vPz += siun;  gOutTree->Branch(vPz, &outPzUN [iun]);
          TString vEn("EnPUN");   vEn += siun;  gOutTree->Branch(vEn, &outEnUN [iun]);
        }
      }
      if (gUseMCParticles){
        TString vMCPx("MCPxP"); vMCPx += fsIndex; gOutTree->Branch(vMCPx,&outMCPx[pIndex]);
        TString vMCPy("MCPyP"); vMCPy += fsIndex; gOutTree->Branch(vMCPy,&outMCPy[pIndex]);
        TString vMCPz("MCPzP"); vMCPz += fsIndex; gOutTree->Branch(vMCPz,&outMCPz[pIndex]);
        TString vMCEn("MCEnP"); vMCEn += fsIndex; gOutTree->Branch(vMCEn,&outMCEn[pIndex]);
        if (GlueXParticleClass(name) == "Charged" || GlueXParticleClass(name) == "Neutral"){
          TString vMCVx("MCVxP"); vMCVx += fsIndex; gOutTree->Branch(vMCVx,&outMCVx[pIndex]);
          TString vMCVy("MCVyP"); vMCVy += fsIndex; gOutTree->Branch(vMCVy,&outMCVy[pIndex]);
          TString vMCVz("MCVzP"); vMCVz += fsIndex; gOutTree->Branch(vMCVz,&outMCVz[pIndex]);
        }
      }
      if (GlueXParticleClass(name) == "DecayingToCharged"){
        if (gUseParticles && gUseKinFitVtx){
          TString vVeeL      ("VeeLP");       vVeeL       += fsIndex; gOutTree->Branch(vVeeL,      &outVeeL      [pIndex]);
          TString vVeeLSigma ("VeeLSigmaP");  vVeeLSigma  += fsIndex; gOutTree->Branch(vVeeLSigma, &outVeeLSigma [pIndex]);
        }
      }
    }
    }
  }



   // **********************************************************************
   // STEP 6:  DO THE CONVERSION
   // **********************************************************************

  cout << "STARTING THE CONVERSION: " << endl << endl;


    // loop over the input tree

  Long64_t gInNEntries = gInTree->GetEntries();
  TString gInFileName("");
  int currPol; // hold polarization value for the entire run (assuming one run per tree!)
  cout << "LOOPING OVER " << gInNEntries << " ENTRIES..." << endl;
  for (Long64_t iEntry = 0; iEntry < gInNEntries; iEntry++){
    if ((iEntry+1) % 10000 == 0) cout << "entry = " << iEntry+1 << "  (" << (100.0*(iEntry+1))/gInNEntries << " percent)" << endl;

      // clear arrays (from ROOT documentation, see $ROOTSYS/tutorials/tree/tcl.C, also for SetAutoDelete, etc.)

    if (gUseMCParticles) inThrown__P4->Clear();
    if (gUseMCParticles) inThrown__X4->Clear();
    if (gUseParticles) inBeam__P4_Measured->Clear();
    if (gUseParticles) inBeam__X4_Measured->Clear();
    if (gUseParticles) inChargedHypo__P4_Measured->Clear();
    if (gUseParticles) inChargedHypo__X4_Measured->Clear();
    if (gUseParticles) inNeutralHypo__P4_Measured->Clear();
    if (gUseParticles) inNeutralHypo__X4_Measured->Clear();
    if (gUseParticles && gUseKinFit) inBeam__P4_KinFit->Clear();
    if (gUseParticles && gUseKinFitVtx) inBeam__X4_KinFit->Clear();
    for (unsigned int i = 0; i < MAXPARTICLES; i++){ if (inP4_KinFit[i]) inP4_KinFit[i]->Clear(); }
    for (unsigned int i = 0; i < MAXPARTICLES; i++){ if (inX4_KinFit[i]) inX4_KinFit[i]->Clear(); }
    for (unsigned int i = 0; i < MAXPARTICLES; i++){ if (inX4[i]) inX4[i]->Clear(); }


      // if running in safe mode, first check array sizes

    if (gSafe){
      Long64_t localEntry = gInTree->LoadTree(iEntry);
      TString fileName = gInTree->GetFile()->GetName();
      if (fileName != gInFileName){
        gInFileName = fileName;
        cout << "Starting file: " << gInFileName << endl;
      }
      if (localEntry < 0){
        cout << "WARNING: Problem reading this event!  Skipping!" << endl;
        continue;
      }
      brRunNumber->GetEntry(localEntry);
      brEventNumber->GetEntry(localEntry);
      if (gUseMCInfo) brNumThrown->GetEntry(localEntry);
      if (gUseParticles) brNumBeam->GetEntry(localEntry);
      if (gUseParticles) brNumChargedHypos->GetEntry(localEntry);
      if (gUseParticles) brNumNeutralHypos->GetEntry(localEntry);
      if (gUseParticles) brNumCombos->GetEntry(localEntry);
      if (gUseParticles) brNumUnusedTracks->GetEntry(localEntry);
      int numUnusedNeutrals = inNumNeutralHypos - gNumFSNeutrals;
      if (gUseParticles){
        if ((gNumUnusedTracksCut   >= 0) && (inNumUnusedTracks   > gNumUnusedTracksCut)) continue;
        if ((gNumUnusedNeutralsCut >= 0) && (  numUnusedNeutrals > gNumUnusedNeutralsCut)) continue;
        if ((gNumNeutralHyposCut   >= 0) && (inNumNeutralHypos   > gNumNeutralHyposCut)) continue;
      }
      if ((inNumThrown > MAXTHROWN) ||
          (inNumBeam > MAXBEAM) ||
          (inNumChargedHypos > MAXTRACKS) ||
          (inNumNeutralHypos > MAXNEUTRALS) ||
          (inNumCombos > MAXCOMBOS)){
        cout << "WARNING:  Array sizes will be exceeded!  Skipping event! (for details, use -print 1)" << endl;
        if (gPrint > 0){
          cout << "   Entry           = " << iEntry << endl;
          cout << "   Run             = " << inRunNumber << endl;
          cout << "   Event           = " << inEventNumber << endl;
          cout << "   NumThrown       = " << inNumThrown << endl;
          cout << "   NumBeam         = " << inNumBeam << endl;
          cout << "   NumChargedHypos = " << inNumChargedHypos << endl;
          cout << "   NumNeutralHypos = " << inNumNeutralHypos << endl;
          cout << "   NumCombos       = " << inNumCombos << endl;
          cout << endl;
        }
        continue;
      }
    }


      // get entries from the input tree

    gInTree->GetEntry(iEntry);


     // print some information (for debugging only)

    if ((iEntry+1 == 1) && (gPrint == 1)){
      cout << endl << "PRINTING TEST INFORMATION FOR FIVE EVENTS..." << endl << endl;
    }
    if (((iEntry < 5) && (gPrint == 1)) || (gPrint == 2)){
      cout << endl << endl;
      cout << "  ***************************" << endl;
      cout << "  ******* NEW EVENT " << iEntry+1 << " *******" << endl;
      cout << "  ***************************" << endl;
      cout << endl << endl;
    }


      // if MC, start parsing truth information

    vector< vector<int> > orderedThrownIndices;
    vector< vector<int> > orderedThrownPDGNumbers;

    if (gUseMCInfo){
        // set indices
      orderedThrownIndices = OrderedThrownIndices(inNumThrown,inThrown__PID,inThrown__ParentIndex);
      orderedThrownPDGNumbers = orderedThrownIndices;
      for (unsigned int i = 0; i < orderedThrownPDGNumbers.size(); i++){
      for (unsigned int j = 0; j < orderedThrownPDGNumbers[i].size(); j++){
        orderedThrownPDGNumbers[i][j] = inThrown__PID[orderedThrownIndices[i][j]];
      }}
        // set output information
      pair<int,int> fsCode = FSCode(orderedThrownPDGNumbers);
      outMCDecayCode1 = fsCode.first;
      outMCDecayCode2 = fsCode.second;
      outMCExtras = FSMCExtras(inNumThrown,inThrown__PID);
        // set MCDecayParticle information
      vector<int> mcDecayParticles = MCDecayParticles(inNumThrown,inThrown__PID,inThrown__ParentIndex);
      outMCDecayParticle1 = 0;  if (mcDecayParticles.size() >= 1) outMCDecayParticle1 = mcDecayParticles[1-1];
      outMCDecayParticle2 = 0;  if (mcDecayParticles.size() >= 2) outMCDecayParticle2 = mcDecayParticles[2-1];
      outMCDecayParticle3 = 0;  if (mcDecayParticles.size() >= 3) outMCDecayParticle3 = mcDecayParticles[3-1];
      outMCDecayParticle4 = 0;  if (mcDecayParticles.size() >= 4) outMCDecayParticle4 = mcDecayParticles[4-1];
      outMCDecayParticle5 = 0;  if (mcDecayParticles.size() >= 5) outMCDecayParticle5 = mcDecayParticles[5-1];
      outMCDecayParticle6 = 0;  if (mcDecayParticles.size() >= 6) outMCDecayParticle6 = mcDecayParticles[6-1];
        // select a specific final state
      if (gMCTag != ""){
        if (gMCTagExtras.Atoi() != (int) outMCExtras) continue;
        if (gMCTagDecayCode1.Atoi() != (int) outMCDecayCode1) continue;
        if (gMCTagDecayCode2.Atoi() != (int) outMCDecayCode2) continue;
      }
        // check that the generated final state matches the reconstructed final state
      if (gUseMCParticles){
        outMCSignal = 0;
        if ((gReconstructedFSCode.first == fsCode.first) &&
            (gReconstructedFSCode.second == fsCode.second) &&
            (outMCExtras < 0.1)) outMCSignal = 1;
      }
        // do some checks on the MC information
      bool mcError = false;  bool mcWarning = false;
      if (gUseMCParticles && outMCSignal > 0.1){
          // check orderedThrownIndices
        if (orderedThrownIndices.size() != gOrderedParticleNames.size()){
          cout << "ERROR: problem with size of orderedThrownIndices" << endl;
          mcError = true;
        }
        for (unsigned int i = 0; i < orderedThrownIndices.size(); i++){
          if (orderedThrownIndices[i].size() != gOrderedParticleNames[i].size()){
            cout << "ERROR: problem with size of orderedThrownIndices" << endl;
            mcError = true;
          }
        }
      }
     if (gMCChecks && BaryonNumber((int)outMCDecayCode1,(int)outMCDecayCode2,(int)outMCExtras) != 1)
       { mcWarning = true; cout << "WARNING: problem with baryon number in MC (for details, use -print 1)" << endl; }
     if (gMCChecks && Charge((int)outMCDecayCode1,(int)outMCDecayCode2,(int)outMCExtras) != 1)
       { mcWarning = true; cout << "WARNING: problem with electric charge in MC (for details, use -print 1)" << endl; }
      //if (((outMCSignal > 0.1)&&!inIsThrownTopology) ||
      //    ((outMCSignal < 0.1)&& inIsThrownTopology)){
      //  cout << "ERROR: MCSignal does not match IsThrownTopology" << endl;
      //  mcWarning = true;
      //}
        // print a few events to make sure MC makes sense
      if (((iEntry < 5) && gPrint == 1) || (mcError) || (mcWarning && gPrint > 0) || (gPrint == 2)){
      //if (isMC && (iEntry < 5||inIsThrownTopology)){
        cout << endl << endl;
        if (mcError||mcWarning) cout << "WARNING: problems with the truth parsing (see below)..." << endl;
        cout << "  **** TRUTH INFO STUDY FOR EVENT " << iEntry+1 << " ****" << endl;
        cout << "  NumThrown = " << inNumThrown << endl;
        cout << "  GeneratedEnergy = " << inThrownBeam__GeneratedEnergy << endl;
        cout << "  FSCode = " << (int)outMCDecayCode2 << "_" << (int)outMCDecayCode1 << endl;
        cout << "  MCExtras = " << outMCExtras << endl;
        cout << "  IsThrownTopology = " << inIsThrownTopology << endl;
        DisplayMCThrown(inNumThrown,inThrown__PID,inThrown__ParentIndex,inThrown__P4);
        cout << endl << endl;
      }
      if (mcError){
        cout << "ERROR: problem with MC truth parsing" << endl;
        exit(0);
      }
    }



      // loop over combos

    vector<double> vChi2Check;
    if (gUseMCInfo && !gUseParticles) inNumCombos = 1;
    for (UInt_t ic = 0; ic < inNumCombos; ic++){

        // non-particle information

      TLorentzVector *p4, *p4a, *p4b, *x4, *x4a, *x4b;
      outRunNumber       = inRunNumber;
      outEventNumber     = inEventNumber;
      if(gUsePolarization) {
        if(iEntry==0) {
          if(GetPolarizationAngle(inRunNumber, currPol)) {
            outPolarization = currPol;
          } else {
            outPolarization = -1;
          }
        } else {
          outPolarization = currPol;
        }
      }
      if (gUseParticles){
        outNumUnusedTracks = inNumUnusedTracks;
        outNumNeutralHypos = inNumNeutralHypos;
        outNumBeam         = inNumBeam;
        outNumCombos       = inNumCombos;
        if (gUseKinFit){
          outChi2        = inChiSq_KinFit[ic];
          outChi2DOF     = -1; if (inNDF_KinFit[ic]>0.0) outChi2DOF = outChi2/inNDF_KinFit[ic];
          //outRFTime      = inRFTime_KinFit[ic];
        }
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
        if (gUseKinFit){
              p4 = (TLorentzVector*)inBeam__P4_KinFit->At(ic);
          outPxPB = p4->Px();
          outPyPB = p4->Py();
          outPzPB = p4->Pz();
          outEnPB = p4->E();
        }
        if (gUseKinFitVtx){
              x4 = (TLorentzVector*)inBeam__X4_KinFit->At(ic);
          outVxPB = x4->X();
          outVyPB = x4->Y();
          outVzPB = x4->Z();
        }
            p4 = (TLorentzVector*)inBeam__P4_Measured->At(inBeamIndex[ic]);
        outRPxPB = p4->Px();
        outRPyPB = p4->Py();
        outRPzPB = p4->Pz();
        outREnPB = p4->E();
            x4 = (TLorentzVector*)inBeam__X4_Measured->At(inBeamIndex[ic]);
        outRVxPB = x4->X();
        outRVyPB = x4->Y();
        outRVzPB = x4->Z();
      }
      if (gUseMCInfo){
        outMCEnPB = inThrownBeam__GeneratedEnergy;
      }
      if (gUseMCParticles){
        outMCPxPB = inThrownBeam__P4->Px();
        outMCPyPB = inThrownBeam__P4->Py();
        outMCPzPB = inThrownBeam__P4->Pz();
        outMCEnPB = inThrownBeam__P4->E();
        outMCVxPB = inThrownBeam__X4->X();
        outMCVyPB = inThrownBeam__X4->Y();
        outMCVzPB = inThrownBeam__X4->Z();
      }

        // particle information

      bool cutDueToParticleInfo = false;
      vector<int> vUsedNeutralIndices;
      for (unsigned int im = 0; im < gOrderedParticleNames.size(); im++){
      for (unsigned int id = 0; id < gOrderedParticleNames[im].size(); id++){
        TString name = gOrderedParticleNames[im][id];
        int pIndex = gMapGlueXNameToParticleIndex[name];
        int tIndex; if (gUseMCParticles && outMCSignal > 0.1) tIndex = orderedThrownIndices[im][id];

          // charged tracks

        if (GlueXParticleClass(name) == "Charged"){
          if (gUseParticles && gUseKinFit){
            p4 = (TLorentzVector*)inP4_KinFit[pIndex]->At(ic);
              outPx[pIndex] = p4->Px();
              outPy[pIndex] = p4->Py();
              outPz[pIndex] = p4->Pz();
              outEn[pIndex] = p4->E();
          }
          if (gUseParticles && gUseKinFitVtx){
            x4 = (TLorentzVector*)inX4_KinFit[pIndex]->At(ic);
              outVx[pIndex] = x4->X();
              outVy[pIndex] = x4->Y();
              outVz[pIndex] = x4->Z();
          }
          if (gUseParticles){
            p4 = (TLorentzVector*)inChargedHypo__P4_Measured->At(inChargedIndex[pIndex][ic]);
              outRPx[pIndex] = p4->Px();
              outRPy[pIndex] = p4->Py();
              outRPz[pIndex] = p4->Pz();
              outREn[pIndex] = p4->E();
              outTkNDF [pIndex] = inChargedHypo__NDF_Tracking  [(inChargedIndex[pIndex][ic])];
              outTkChi2[pIndex] = inChargedHypo__ChiSq_Tracking[(inChargedIndex[pIndex][ic])];
            x4 = (TLorentzVector*)inChargedHypo__X4_Measured->At(inChargedIndex[pIndex][ic]);
              outRVx[pIndex] = x4->X();
              outRVy[pIndex] = x4->Y();
              outRVz[pIndex] = x4->Z();
            if (gAddPID){
              outTkTOFBeta[pIndex] = inBeta_Timing[pIndex][ic];
              outTkTOFChi2[pIndex] = inChiSq_Timing[pIndex][ic];
              outTkDEDXChi2[pIndex] = inChargedHypo__ChiSq_DCdEdx[(inChargedIndex[pIndex][ic])];
              outTkDEDXNDF [pIndex] = inChargedHypo__NDF_DCdEdx  [(inChargedIndex[pIndex][ic])];
              outTkDEDXCDC [pIndex] = inChargedHypo__dEdx_CDC    [(inChargedIndex[pIndex][ic])];
              outTkDEDXFDC [pIndex] = inChargedHypo__dEdx_FDC    [(inChargedIndex[pIndex][ic])];
            }
            if(gUseDIRC){
              outLkDIRC[pIndex]     = inChargedHypo__Lk_DIRC     [(inChargedIndex[pIndex][ic])];
              outLpiDIRC[pIndex]    = inChargedHypo__Lpi_DIRC    [(inChargedIndex[pIndex][ic])];
              outLpDIRC[pIndex]     = inChargedHypo__Lp_DIRC     [(inChargedIndex[pIndex][ic])];
              outLeleDIRC[pIndex]   = inChargedHypo__Lele_DIRC   [(inChargedIndex[pIndex][ic])];
              outNumPhotonsDIRC[pIndex]     = inChargedHypo__NumPhotons_DIRC[(inChargedIndex[pIndex][ic])];
              outXDIRC[pIndex]      = inChargedHypo__x_DIRC[(inChargedIndex[pIndex][ic])];
              outYDIRC[pIndex]      = inChargedHypo__y_DIRC[(inChargedIndex[pIndex][ic])];
            }
          }
          if (gUseMCParticles && outMCSignal > 0.1){
            p4 = (TLorentzVector*)inThrown__P4->At(tIndex);
              outMCPx[pIndex] = p4->Px();
              outMCPy[pIndex] = p4->Py();
              outMCPz[pIndex] = p4->Pz();
              outMCEn[pIndex] = p4->E();
            x4 = (TLorentzVector*)inThrown__X4->At(tIndex);
              outMCVx[pIndex] = x4->X();
              outMCVy[pIndex] = x4->Y();
              outMCVz[pIndex] = x4->Z();
          }
        }

          // neutral particles

        if (GlueXParticleClass(name) == "Neutral"){
          if (gUseParticles && gUseKinFit){
            p4 = (TLorentzVector*)inP4_KinFit[pIndex]->At(ic);
              outPx[pIndex] = p4->Px();
              outPy[pIndex] = p4->Py();
              outPz[pIndex] = p4->Pz();
              outEn[pIndex] = p4->E();
          }
          if (gUseParticles && gUseKinFitVtx){
            x4 = (TLorentzVector*)inX4_KinFit[pIndex]->At(ic);
              outVx[pIndex] = x4->X();
              outVy[pIndex] = x4->Y();
              outVz[pIndex] = x4->Z();
          }
          if (gUseParticles){
            p4 = (TLorentzVector*)inNeutralHypo__P4_Measured->At(inNeutralIndex[pIndex][ic]);
              outRPx[pIndex] = p4->Px();
              outRPy[pIndex] = p4->Py();
              outRPz[pIndex] = p4->Pz();
              outREn[pIndex] = p4->E();
            x4 = (TLorentzVector*)inNeutralHypo__X4_Measured->At(inNeutralIndex[pIndex][ic]);
              outRVx[pIndex] = x4->X();
              outRVy[pIndex] = x4->Y();
              outRVz[pIndex] = x4->Z();
            outShQuality[pIndex] = inNeutralHypo__ShowerQuality[(inNeutralIndex[pIndex][ic])];
            if (outShQuality[pIndex] < gShQualityCut) cutDueToParticleInfo = true;
            vUsedNeutralIndices.push_back(inNeutralIndex[pIndex][ic]);
          }
          if (gUseMCParticles && outMCSignal > 0.1){
            p4 = (TLorentzVector*)inThrown__P4->At(tIndex);
              outMCPx[pIndex] = p4->Px();
              outMCPy[pIndex] = p4->Py();
              outMCPz[pIndex] = p4->Pz();
              outMCEn[pIndex] = p4->E();
            x4 = (TLorentzVector*)inThrown__X4->At(tIndex);
              outMCVx[pIndex] = x4->X();
              outMCVy[pIndex] = x4->Y();
              outMCVz[pIndex] = x4->Z();
          }
        }

        // vertex information

        if (gUseKinFitVtx){
        }


          // decaying to charged tracks

        if (GlueXParticleClass(name) == "DecayingToCharged"){
          int pIndex1 = gMapGlueXNameToParticleIndex[gOrderedParticleNames[im][1]];
          int pIndex2 = gMapGlueXNameToParticleIndex[gOrderedParticleNames[im][2]];
          //int tIndex0;  if (gUseMCParticles && outMCSignal > 0.1) tIndex0 = orderedThrownIndices[im][0];
          int tIndex1;  if (gUseMCParticles && outMCSignal > 0.1) tIndex1 = orderedThrownIndices[im][1];
          int tIndex2;  if (gUseMCParticles && outMCSignal > 0.1) tIndex2 = orderedThrownIndices[im][2];
          if (gUseParticles && gUseKinFitVtx){
            p4a = (TLorentzVector*)inP4_KinFit[pIndex1]->At(ic);
            p4b = (TLorentzVector*)inP4_KinFit[pIndex2]->At(ic);
            TLorentzVector* p4 = new TLorentzVector(0,0,0,0);
            *p4 = *p4a + *p4b;
              outPx[pIndex] = p4a->Px() + p4b->Px();
              outPy[pIndex] = p4a->Py() + p4b->Py();
              outPz[pIndex] = p4a->Pz() + p4b->Pz();
              outEn[pIndex] = p4a->E()  + p4b->E();
            x4a = (TLorentzVector*)inBeam__X4_KinFit->At(ic);
            x4b = (TLorentzVector*)inX4[pIndex]->At(ic);
            TLorentzVector* x4 = new TLorentzVector(0,0,0,0);
            *x4 = *x4b - *x4a;
            outVeeL[pIndex] = (x4->Vect()).Mag();
            if ( (x4->Angle(p4->Vect()))/TMath::Pi() > 0.5 )
              outVeeL[pIndex] = -outVeeL[pIndex];
            if ( inPathLengthSigma[pIndex][ic] < 1.0e-6 )
              outVeeLSigma[pIndex] = -10000.;
            else
              outVeeLSigma[pIndex] = outVeeL[pIndex]/inPathLengthSigma[pIndex][ic];
            delete p4;
            delete x4;
          }
          if (gUseParticles){
            p4a = (TLorentzVector*)inChargedHypo__P4_Measured->At(inChargedIndex[pIndex1][ic]);
            p4b = (TLorentzVector*)inChargedHypo__P4_Measured->At(inChargedIndex[pIndex2][ic]);
              outRPx[pIndex] = p4a->Px() + p4b->Px();
              outRPy[pIndex] = p4a->Py() + p4b->Py();
              outRPz[pIndex] = p4a->Pz() + p4b->Pz();
              outREn[pIndex] = p4a->E()  + p4b->E();
            if (gMassWindows > 0){
              double mass = sqrt(pow(outREn[pIndex],2)-pow(outRPx[pIndex],2)-
                                 pow(outRPy[pIndex],2)-pow(outRPz[pIndex],2));
              if (gUseKinFit)
                     mass = sqrt(pow(outEn[pIndex],2)-pow(outPx[pIndex],2)-
                                 pow(outPy[pIndex],2)-pow(outPz[pIndex],2));
              if ((FSParticleType(name) == "Lambda") || (FSParticleType(name) == "ALambda"))
                if (abs(mass-1.115683) > gMassWindows/2.0) cutDueToParticleInfo = true;
              if (FSParticleType(name) == "Ks")
                if (abs(mass-0.497611) > gMassWindows/2.0) cutDueToParticleInfo = true;
            }
          }
          if (gUseMCParticles && outMCSignal > 0.1){
            //p4  = (TLorentzVector*)inThrown__P4->At(tIndex0);
            p4a = (TLorentzVector*)inThrown__P4->At(tIndex1);
            p4b = (TLorentzVector*)inThrown__P4->At(tIndex2);
              //outMCPx[pIndex] = p4->Px();
              //outMCPy[pIndex] = p4->Py();
              //outMCPz[pIndex] = p4->Pz();
              //outMCEn[pIndex] = p4->E();
              outMCPx[pIndex] = p4a->Px() + p4b->Px();
              outMCPy[pIndex] = p4a->Py() + p4b->Py();
              outMCPz[pIndex] = p4a->Pz() + p4b->Pz();
              outMCEn[pIndex] = p4a->E()  + p4b->E();
          }
        }

          // decaying to neutral particles

        if (GlueXParticleClass(name) == "DecayingToNeutral"){
          int pIndex1 = gMapGlueXNameToParticleIndex[gOrderedParticleNames[im][1]];
          int pIndex2 = gMapGlueXNameToParticleIndex[gOrderedParticleNames[im][2]];
          int tIndex1;  if (gUseMCParticles && outMCSignal > 0.1) tIndex1 = orderedThrownIndices[im][1];
          int tIndex2;  if (gUseMCParticles && outMCSignal > 0.1) tIndex2 = orderedThrownIndices[im][2];
          if (gUseParticles && gUseKinFit){
            p4a = (TLorentzVector*)inP4_KinFit[pIndex1]->At(ic);
            p4b = (TLorentzVector*)inP4_KinFit[pIndex2]->At(ic);
              outPx[pIndex] = p4a->Px() + p4b->Px();
              outPy[pIndex] = p4a->Py() + p4b->Py();
              outPz[pIndex] = p4a->Pz() + p4b->Pz();
              outEn[pIndex] = p4a->E()  + p4b->E();
          }
          if (gUseParticles){
            p4a = (TLorentzVector*)inNeutralHypo__P4_Measured->At(inNeutralIndex[pIndex1][ic]);
            p4b = (TLorentzVector*)inNeutralHypo__P4_Measured->At(inNeutralIndex[pIndex2][ic]);
            vUsedNeutralIndices.push_back(inNeutralIndex[pIndex1][ic]);
            vUsedNeutralIndices.push_back(inNeutralIndex[pIndex2][ic]);
              outRPx[pIndex] = p4a->Px() + p4b->Px();
              outRPy[pIndex] = p4a->Py() + p4b->Py();
              outRPz[pIndex] = p4a->Pz() + p4b->Pz();
              outREn[pIndex] = p4a->E()  + p4b->E();
            if (gMassWindows > 0){
              double mass = sqrt(pow(outREn[pIndex],2)-pow(outRPx[pIndex],2)-
                                 pow(outRPy[pIndex],2)-pow(outRPz[pIndex],2));
              if (gUseKinFit)
                     mass = sqrt(pow(outEn[pIndex],2)-pow(outPx[pIndex],2)-
                                 pow(outPy[pIndex],2)-pow(outPz[pIndex],2));
              if (FSParticleType(name) == "eta")
                if (abs(mass-0.547862) > gMassWindows/2.0) cutDueToParticleInfo = true;
              if (FSParticleType(name) == "pi0")
                if (abs(mass-0.134977) > gMassWindows/2.0) cutDueToParticleInfo = true;
            }
          }
          if (gUseMCParticles && outMCSignal > 0.1){
            p4a = (TLorentzVector*)inThrown__P4->At(tIndex1);
            p4b = (TLorentzVector*)inThrown__P4->At(tIndex2);
              outMCPx[pIndex] = p4a->Px() + p4b->Px();
              outMCPy[pIndex] = p4a->Py() + p4b->Py();
              outMCPz[pIndex] = p4a->Pz() + p4b->Pz();
              outMCEn[pIndex] = p4a->E()  + p4b->E();
          }
        }

      }}

      // print some information (for debugging only)

      if (((iEntry < 5) && (gPrint == 1) && (gUseParticles)) || ((gPrint == 2) && (gUseParticles))){
        cout << "  *******************************" << endl;
        cout << "  **** INFO FOR EVENT " << iEntry+1 << " ****" << endl;
        cout << "  *******************************" << endl;
        cout << "EVENT: " << inEventNumber << " (combo no. " << ic+1 << ")" << endl;
        cout << "  NumThrown = " << inNumThrown << endl;
        cout << "  NumChargedHypos = " << inNumChargedHypos << endl;
        cout << "  NumNeutralHypos = " << inNumNeutralHypos << endl;
        cout << "  NumBeam   = " << inNumBeam << endl;
        cout << "  NumCombos = " << inNumCombos << endl;
        cout << "  ProdVertex = " << inX4_Production->X() << " "
                                  << inX4_Production->Y() << " "
                                  << inX4_Production->Z() << " "
                                  << inX4_Production->T() << endl;
        for (unsigned int im = 0; im < gOrderedParticleNames.size(); im++){
        for (unsigned int id = 0; id < gOrderedParticleNames[im].size(); id++){
          TString name = gOrderedParticleNames[im][id];
          int pIndex = gMapGlueXNameToParticleIndex[name];
          TString fsIndex = gMapGlueXNameToFSIndex[name];
          double px = outRPx[pIndex];  if (gUseKinFit) px = outPx[pIndex];
          double py = outRPy[pIndex];  if (gUseKinFit) py = outPy[pIndex];
          double pz = outRPz[pIndex];  if (gUseKinFit) pz = outPz[pIndex];
          double en = outREn[pIndex];  if (gUseKinFit) en = outEn[pIndex];
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
      if (iEntry+1 == 5 && ic+1 == inNumCombos && (gPrint == 1) && gUseParticles){
        cout << endl << endl << "DONE PRINTING TEST INFORMATION FOR FIVE EVENTS" << endl << endl;
        cout << "CONTINUING THE CONVERSION... " << endl << endl;
      }


        // make cuts

      if (gUseParticles){
        if (cutDueToParticleInfo) continue;
        if (gUseKinFit && outChi2DOF > gChi2DOFCut) continue;
        if (gRFDeltaTCut > 0.0 && abs(outRFDeltaT) > gRFDeltaTCut) continue;
        int numUnusedNeutrals = inNumNeutralHypos - gNumFSNeutrals;
        if ((gNumUnusedTracksCut   >= 0) && (outNumUnusedTracks   > gNumUnusedTracksCut)) continue;
        if ((gNumUnusedNeutralsCut >= 0) && (   numUnusedNeutrals > gNumUnusedNeutralsCut)) continue;
        if ((gNumNeutralHyposCut   >= 0) && (outNumNeutralHypos   > gNumNeutralHyposCut)) continue;
      }


        // check for combos with the same chi2

      if (gUseParticles && gUseKinFit && gCombos != 2 && outChi2DOF <= gChi2DOFCut){
        bool foundChi2 = false;
        for (unsigned int icheck = 0; icheck < vChi2Check.size(); icheck++){
          if (fabs(vChi2Check[icheck] - outChi2) < 1.0e-8) { foundChi2 = true; break; } }
        if (!foundChi2) vChi2Check.push_back(outChi2);
        if (foundChi2 && gCombos == 0)
          cout << "WARNING:  multiple combos in (run,event) = (" << outRunNumber << "," << outEventNumber
               << ") with the same chi2 = " << outChi2 << endl;
        if (foundChi2 && gCombos == 1) continue;
      }

        // add four-momentum for unused neutrals
      {
        int numWritten = 0;
        for (unsigned int iun = 0; iun < inNumNeutralHypos && numWritten < gAddUnusedNeutrals; iun++){
          bool used = false;
          for (unsigned int iused = 0; iused < vUsedNeutralIndices.size(); iused++){
            if (vUsedNeutralIndices[iused] == iun){ used = true; break; }
          }
          if (!used){
            p4 = (TLorentzVector*)inNeutralHypo__P4_Measured->At(iun);
              outPxUN[numWritten] = p4->Px();
              outPyUN[numWritten] = p4->Py();
              outPzUN[numWritten] = p4->Pz();
              outEnUN[numWritten] = p4->E();
            numWritten++;
          }
        }
      }


        // fill the tree

      gOutTree->Fill();

    }

  }

    // write output information

  cout << endl << endl << "WRITING THE OUTPUT TREE..." << endl;
  gOutputFile->cd();
  gOutTree->Write();
  cout << endl;
  cout << "**************" << endl;
  cout << "FINISHED" << endl;
  cout << "**************" << endl;
  cout << "  total events = " << gInNEntries << endl;
  cout << "  kept entries = " << gOutTree->GetEntriesFast() << endl;
  cout << "  fraction = ";
  if (gInNEntries > 0){ cout << (double)gOutTree->GetEntriesFast()/gInNEntries << endl; }
  else { cout << " undefined" << endl; }
  cout << endl;
  gOutputFile->Close();

    // clean memory (or try to, delete doesn't work here)
    //    * all seems fine in top without deletes or Clears,
    //        even for large numbers of files
    //    * not sure how ROOT handles these, ignore for now

  //if (inThrown__P4)  inThrown__P4->Clear();
  //if (inBeam__P4_Measured)  inBeam__P4_Measured->Clear();
  //if (inBeam__X4_Measured)  inBeam__X4_Measured->Clear();
  //if (inChargedHypo__P4_Measured)  inChargedHypo__P4_Measured->Clear();
  //if (inNeutralHypo__P4_Measured)  inNeutralHypo__P4_Measured->Clear();
  //if (inBeam__P4_KinFit)  inBeam__P4_KinFit->Clear();
  //for (unsigned int i = 0; i < MAXPARTICLES; i++){ if (inP4_KinFit[i]) inP4_KinFit[i]->Clear(); }

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
static const int kpdgSigmap     = 3222;
static const int kpdgSigma0     = 3212;
static const int kpdgSigmam     = 3112;
static const int kpdgASigmam    = -3222;
static const int kpdgASigma0    = -3212;
static const int kpdgASigmap    = -3112;
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
  else if (id == kpdgSigmap)     name = "Sigma+";
  else if (id == kpdgSigma0)     name = "Sigma0";
  else if (id == kpdgSigmam)     name = "Sigma-";
  else if (id == kpdgASigmam)    name = "ASigma-";
  else if (id == kpdgASigma0)    name = "ASigma0";
  else if (id == kpdgASigmap)    name = "ASigma+";
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

void DisplayMCThrown(int numThrown, int pids[], int parentIndices[], TClonesArray* p4s){
  cout << "  LIST OF THROWN PARTICLES (STRAIGHT FROM ANALYSIS TREE): " << endl;
  TLorentzVector *p4;
  for (int i = 0; i < numThrown; i++){
    p4 = (TLorentzVector*)p4s->At(i);
    cout << "    THROWN INDEX = " << i << endl;
    cout << "      PID = " << pids[i] << endl;
    cout << "      PDG Name = " << PDGReadableName(pids[i]) << endl;
    cout << "      Parent Index = " << parentIndices[i] << endl;
    cout << "      Mass = " << p4->M() << endl;
    cout << "      Energy = " << p4->E() << endl;
  }
  vector< pair<int,int> > firstList;
  for (int index = 0; index < numThrown; index++){
    if (parentIndices[index] == -1) firstList.push_back(pair<int,int>(index,0));
  }
  vector< pair<int,int> > thrownList = firstList;
  for (int iter = 1; iter <= 5; iter++){
    vector< pair<int,int> > tempList = thrownList;  thrownList.clear();
    for (unsigned int i = 0; i < tempList.size(); i++){
      thrownList.push_back(tempList[i]);
      if (tempList[i].second == iter-1){
        for (int index = 0; index < numThrown; index++){
          if (parentIndices[index] == tempList[i].first)
            thrownList.push_back(pair<int,int>(index,iter));
        }
      }
    }
  }
  cout << "  ORDERED LIST OF THROWN PARTICLES" << endl;
  for (unsigned int i = 0; i < thrownList.size(); i++){
    int iter = thrownList[i].second;
    for (int j = 0; j < iter+1; j++){ cout << "   "; }
    cout << PDGReadableName(pids[thrownList[i].first]);
    cout << " (id = " << pids[thrownList[i].first] << ")";
    cout << " (index = " << thrownList[i].first << ")" << endl;
  }
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


vector<int> MCDecayParticles(int numThrown, int pids[], int parentIndices[]){
  vector<int> mcDecayParticles;
  for (int index = 0; index < numThrown; index++){
    if (parentIndices[index] == -1) mcDecayParticles.push_back(pids[index]);
  }
  if (mcDecayParticles.size() < 2) return mcDecayParticles;
  for (unsigned int i = 0; i < mcDecayParticles.size()-1; i++){
  for (unsigned int j = i+1; j < mcDecayParticles.size(); j++){
    if (mcDecayParticles[i] > mcDecayParticles[j]){
      int temp = mcDecayParticles[i];
      mcDecayParticles[i] = mcDecayParticles[j];
      mcDecayParticles[j] = temp;
    }
  }}
  return mcDecayParticles;
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

int BaryonNumber(int fsCode1, int fsCode2, int mcExtras){
  int baryonNumber = 0;
  baryonNumber += (((mcExtras%100)-(mcExtras%10))/10); // neutron
  baryonNumber -= (((mcExtras%10)-(mcExtras%1))/1); // anti-neutron
  baryonNumber += (((fsCode2%1000000000)-(fsCode2%100000000))/100000000); // Lambda
  baryonNumber -= (((fsCode2%100000000)-(fsCode2%10000000))/10000000); // anti-Lambda
  baryonNumber += (((fsCode2%1000)-(fsCode2%100))/100); // proton
  baryonNumber -= (((fsCode2%100)-(fsCode2%10))/10); // anti-proton
  baryonNumber += fsCode1*0;
  return baryonNumber;
}

int Charge(int fsCode1, int fsCode2, int mcExtras){
  int charge = 0;
  charge += (((fsCode2%10000000)-(fsCode2%1000000))/1000000);  // e+
  charge -= (((fsCode2%1000000)-(fsCode2%100000))/100000);  // e-
  charge += (((fsCode2%100000)-(fsCode2%10000))/10000);  // mu+
  charge -= (((fsCode2%10000)-(fsCode2%1000))/1000);  // mu-
  charge += (((fsCode2%1000)-(fsCode2%100))/100);  // p+
  charge -= (((fsCode2%100)-(fsCode2%10))/10);  // p-
  charge += (((fsCode1%1000000)-(fsCode1%100000))/100000);  // K+
  charge -= (((fsCode1%100000)-(fsCode1%10000))/10000);  // K-
  charge += (((fsCode1%1000)-(fsCode1%100))/100);  // pi+
  charge -= (((fsCode1%100)-(fsCode1%10))/10);  // pi-
  return charge;
}


TString FSParticleType(TString glueXParticleType){
  if (glueXParticleType.Contains("Missing"))     return TString("--");
  if (glueXParticleType.Contains("AntiLambda"))  return TString("ALambda");
  if (glueXParticleType.Contains("Lambda"))      return TString("Lambda");
  if (glueXParticleType.Contains("Positron"))    return TString("e+");
  if (glueXParticleType.Contains("Electron"))    return TString("e-");
  if (glueXParticleType.Contains("MuonPlus"))    return TString("mu+");
  if (glueXParticleType.Contains("MuonMinus"))   return TString("mu-");
  if (glueXParticleType.Contains("AntiProton"))  return TString("p-");
  if (glueXParticleType.Contains("Proton"))      return TString("p+");
  if (glueXParticleType.Contains("Eta") &&
     !glueXParticleType.Contains("EtaPrime"))    return TString("eta");
  if (glueXParticleType.Contains("Photon"))      return TString("gamma");
  if (glueXParticleType.Contains("KPlus"))       return TString("K+");
  if (glueXParticleType.Contains("KMinus"))      return TString("K-");
  if (glueXParticleType.Contains("KShort"))      return TString("Ks");
  if (glueXParticleType.Contains("PiPlus"))      return TString("pi+");
  if (glueXParticleType.Contains("PiMinus"))     return TString("pi-");
  if (glueXParticleType.Contains("Pi0"))         return TString("pi0");
  return TString("--");
}


TString GlueXParticleClass(TString glueXParticleType){
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

int GlueXNeutralsCounter(TString glueXParticleType){
  if (glueXParticleType.Contains("AntiLambda"))  return 0;
  if (glueXParticleType.Contains("Lambda"))      return 0;
  if (glueXParticleType.Contains("Positron"))    return 0;
  if (glueXParticleType.Contains("Electron"))    return 0;
  if (glueXParticleType.Contains("MuonPlus"))    return 0;
  if (glueXParticleType.Contains("MuonMinus"))   return 0;
  if (glueXParticleType.Contains("AntiProton"))  return 0;
  if (glueXParticleType.Contains("Proton"))      return 0;
  if (glueXParticleType.Contains("Eta"))         return 2;
  if (glueXParticleType.Contains("Photon"))      return 1;
  if (glueXParticleType.Contains("KPlus"))       return 0;
  if (glueXParticleType.Contains("KMinus"))      return 0;
  if (glueXParticleType.Contains("KShort"))      return 0;
  if (glueXParticleType.Contains("PiPlus"))      return 0;
  if (glueXParticleType.Contains("PiMinus"))     return 0;
  if (glueXParticleType.Contains("Pi0"))         return 2;
  return 0;
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


map<TString, vector<TString> > GlueXDecayProductMap(int fsCode1, int fsCode2){
  map<TString, vector<TString> > gluexMap;
  int n[16];
  n[ 0] =   (fsCode1%10);
  n[ 1] = (((fsCode1%100)-(fsCode1%10))/10);
  n[ 2] = (((fsCode1%1000)-(fsCode1%100))/100);
  n[ 3] = (((fsCode1%10000)-(fsCode1%1000))/1000);
  n[ 4] = (((fsCode1%100000)-(fsCode1%10000))/10000);
  n[ 5] = (((fsCode1%1000000)-(fsCode1%100000))/100000);
  n[ 6] = (((fsCode1%10000000)-(fsCode1%1000000))/1000000);
  n[ 7] =   (fsCode2%10);
  n[ 8] = (((fsCode2%100)-(fsCode2%10))/10);
  n[ 9] = (((fsCode2%1000)-(fsCode2%100))/100);
  n[10] = (((fsCode2%10000)-(fsCode2%1000))/1000);
  n[11] = (((fsCode2%100000)-(fsCode2%10000))/10000);
  n[12] = (((fsCode2%1000000)-(fsCode2%100000))/100000);
  n[13] = (((fsCode2%10000000)-(fsCode2%1000000))/1000000);
  n[14] = (((fsCode2%100000000)-(fsCode2%10000000))/10000000);
  n[15] = (((fsCode2%1000000000)-(fsCode2%100000000))/100000000);
  int pNumber = 0;
  for (int i = 0; i < 16; i++){
    for (int j = 0; j < n[i]; j++){
      TString name("");
      if (i == 15) name = "Lambda";
      if (i == 14) name = "AntiLambda";
      if (i == 13) name = "Positron";
      if (i == 12) name = "Electron";
      if (i == 11) name = "MuonPlus";
      if (i == 10) name = "MuonMinus";
      if (i ==  9) name = "Proton";
      if (i ==  8) name = "AntiProton";
      if (i ==  7) name = "Eta";
      if (i ==  6) name = "Photon";
      if (i ==  5) name = "KPlus";
      if (i ==  4) name = "KMinus";
      if (i ==  3) name = "KShort";
      if (i ==  2) name = "PiPlus";
      if (i ==  1) name = "PiMinus";
      if (i ==  0) name = "Pi0";
      name += (pNumber++);
      TString tmp("");
      vector<TString> names;
      if (name.Contains("Pi0")){ tmp = "Photon"; tmp += (pNumber++); names.push_back(tmp);
                                 tmp = "Photon"; tmp += (pNumber++); names.push_back(tmp); }
      if (name.Contains("Eta")){ tmp = "Photon"; tmp += (pNumber++); names.push_back(tmp);
                                 tmp = "Photon"; tmp += (pNumber++); names.push_back(tmp); }
      if (name.Contains("KShort")){ tmp = "PiPlus";  tmp += (pNumber++); names.push_back(tmp);
                                    tmp = "PiMinus"; tmp += (pNumber++); names.push_back(tmp); }
      if (name.Contains("Lambda")&&!name.Contains("AntiLambda"))
                                  { tmp = "Proton";  tmp += (pNumber++); names.push_back(tmp);
                                    tmp = "PiMinus"; tmp += (pNumber++); names.push_back(tmp); }
      if (name.Contains("AntiLambda")){ tmp = "AntiProton";  tmp += (pNumber++); names.push_back(tmp);
                                        tmp = "PiPlus";      tmp += (pNumber++); names.push_back(tmp); }
      gluexMap[name] = names;
    }
  }
  return gluexMap;
}

bool GetPolarizationAngle(int runNumber, int& polarizationAngle)
{
  //RCDB environment must be setup!!

  //Pipe the current constant into this function
  ostringstream locCommandStream;
  locCommandStream << "rcnd " << runNumber << " polarization_angle";
  FILE* locInputFile = gSystem->OpenPipe(locCommandStream.str().c_str(), "r");
  if(locInputFile == NULL)
    return false;

  //get the first line
  char buff[1024];
  if(fgets(buff, sizeof(buff), locInputFile) == NULL)
    return 0;
  istringstream locStringStream(buff);

  //Close the pipe
  gSystem->ClosePipe(locInputFile);

  //extract it
  string locPolarizationAngleString;
  if(!(locStringStream >> locPolarizationAngleString))
    return false;

  // convert string to integer
  polarizationAngle = atoi(locPolarizationAngleString.c_str());
  // amorphous runs have the value -1
  if (polarizationAngle == -1)
    return false;

  return true;
}
