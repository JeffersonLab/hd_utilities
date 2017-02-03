#include <string>
#include <iostream>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"

R__LOAD_LIBRARY(libDSelector)

using namespace std;

void Run_Selector(string locTreeName, string locInputFile, string locOutputFile, string locSelectorName)
{
	//load libraries, etc.
	Long_t locResult = gROOT->ProcessLine(".x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C");
	bool proof = 1;
	int proof_Nthreads = 6; //make sure to request NCORES in the config!

	//tell it to compile selector (if user did not)
	if(locSelectorName[locSelectorName.size() - 1] != '+')
		locSelectorName += '+';

	//process tree
	cout << "tree name, selector name = " << locTreeName << ", " << locSelectorName << endl;

	if(proof) { // add TTree to chain and use PROOFLiteManager
	      	TChain *locChain = new TChain(locTreeName.c_str());
		locChain->Add(locInputFile.c_str());
	
		string outputHistFileName = locOutputFile;
		outputHistFileName += Form(".root");
	
		DPROOFLiteManager *dproof = new DPROOFLiteManager();
		dproof->Process_Chain(locChain, locSelectorName, outputHistFileName, "", "", proof_Nthreads);
	}
	else { // get TTree and use standard TTree::Process
	        TFile *f = TFile::Open(locInputFile.c_str());
		TTree* locTree = (TTree*)gDirectory->Get(locTreeName.c_str());
		locTree->Process(locSelectorName.c_str());
	}

}

