#include <string>
#include <iostream>
#include "TROOT.h"
#include "TTree.h"
#include "TDirectory.h"

using namespace std;

void Run_Selector(string locTreeName, string locSelectorName)
{
	//load libraries, etc.
	Long_t locResult = gROOT->ProcessLine(".x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C");

	//tell it to compile selector (if user did not)
	if(locSelectorName[locSelectorName.size() - 1] != '+')
		locSelectorName += '+';

	//process tree
	cout << "tree name, selector name = " << locTreeName << ", " << locSelectorName << endl;
	TTree* locTree = (TTree*)gDirectory->Get(locTreeName.c_str());
	locTree->Process(locSelectorName.c_str());
}

