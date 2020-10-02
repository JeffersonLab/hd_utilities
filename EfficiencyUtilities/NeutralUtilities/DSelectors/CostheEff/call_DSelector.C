void call_DSelector (TString file)
{
// issue the tree->Process, so that it can be run from the command line
//
cout << "call_DSelector: file=" << file << endl;
gROOT->LoadMacro("$ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C");
// pi0pippim__B4_Tree->Process(file);
pi0pippim__B4_Tree->Process("DSelector_pi0pippim__B4_ver21.C+");
}
