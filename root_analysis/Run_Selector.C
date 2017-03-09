#include <string>
#include <iostream>
#include "TROOT.h"
#include "TTree.h"
#include "TDirectory.h"

R__LOAD_LIBRARY(libDSelector)

using namespace std;

int Run_Selector(string locTreeName, string locSelectorName, unsigned int locNThreads)
{
	//tell it to compile selector (if user did not)
	if(locSelectorName[locSelectorName.size() - 1] != '+')
		locSelectorName += '+';

	//process tree
	cout << "tree name, selector name, #threads = " << locTreeName << ", " << locSelectorName << ", " << locNThreads << endl;
	TTree* locTree = (TTree*)gDirectory->Get(locTreeName.c_str());

	if(locNThreads == 1) //process tree directly
	{
		Long64_t locStatus = locTree->Process(locSelectorName.c_str());
		return ((locStatus >= Long64_t(0)) ? 0 : 999); //0 = success
	}

	//Use PROOF
	return (DPROOFLiteManager::Process_Tree(locTree, locSelectorName, locNThreads) ? 0 : 999); //0 = success
}

