// root script to compare the number of entries in the original root tree with those processed by the DSelector, using the NumEventsSurvivedAction histogram
// This can be useful to guard against incomplete processing such as happens when one or more proof threads are marked bad but the job continues to run.
// Obviously the run number range, file paths and root tree names will need to be customised.
{

  int nfiles = 800; // max # files to check

  char filename[100];
  char treename[100];

  int run;
  int filesdone=0;

  TFile *f;
  TTree *t;
  TH1D *hevts;
  Long64_t entries_orig;
  Long64_t entries_proc;

  sprintf(treename, "kpkm__B4_Tree");


  for (int run=30274; run <= 31057; run++) {

    if (filesdone == nfiles) break;

    // get the #entries in the original root tree from the analysis launch  
    sprintf(filename, "/RunPeriod-2017-01/AnalysisTrees/ver39/tree_kpkm__B4/merged/tree_kpkm__B4_0%i.root",run);

    if (gSystem->AccessPathName(filename)) continue;
    filesdone++;
 
    f = new TFile(filename);
    t = (TTree*)f->Get(treename);
    if (!t) cout << "no tree found in file " << filename << endl;

    entries_orig = t->GetEntries();
    f->Close();

    // get the #entries processed by the DSelector

    sprintf(filename, "hist_%i.root",run);
 
    if (gSystem->AccessPathName(filename)) cout << "Missing file: " << filename << endl;
    if (gSystem->AccessPathName(filename)) continue;

    f = new TFile(filename);

    hevts = (TH1D*)gDirectory->Get("NumEventsSurvivedAction");

    entries_proc =  hevts->GetBinContent(1);

    if (entries_proc != entries_orig) cout << "Run " << run << " original tree events: " << entries_orig << " events processed: " << entries_proc << endl;

    f->Close();

  }
}
