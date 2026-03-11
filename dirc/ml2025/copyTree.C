void copyTree(TString inputDir = "/cache/halld/home/jrsteven/RunPeriod-2019-11/recon/ver01_pass05/hists/", 
              TString outputDir = "/volatile/halld/home/jrsteven/analysisGluexII/RunPeriod-2019-11/recon/ver01_pass05/") {
    gROOT->Reset();

    // Create output directory if it doesn't exist
    gSystem->Exec(Form("mkdir -p %s", outputDir.Data()));

    // Only process these specific bar numbers
    int barNumbers[] = {3, 7, 15, 19, 27, 31, 39, 43};
    int nBars = 8;
    
    // Get list of run number directories in inputDir
    void* dirp = gSystem->OpenDirectory(inputDir.Data());
    if (!dirp) {
        cout << "Error: cannot open directory " << inputDir << endl;
        return;
    }
    
    const char* entry;
    while ((entry = gSystem->GetDirEntry(dirp))) {
        TString runDir = entry;
        if (runDir == "." || runDir == "..") continue;
        
        if (!runDir.BeginsWith("0719")) continue;

        TString subdirPath = inputDir + "/" + runDir;
        if (gSystem->AccessPathName(subdirPath.Data()) == 0) { // path exists
            
            cout << "\n=== Processing run " << runDir << " ===" << endl;
            
            // Create TChain for this run number
            TChain *T = new TChain("dirc");
            
            // Add all .root files in this run directory
            TString pattern = subdirPath + "/*.root";
            int nFilesAdded = T->Add(pattern.Data());
            
            cout << "Added " << nFilesAdded << " files, Total entries: " << T->GetEntries() << endl;
            
            if (T->GetEntries() > 0) {
                // Process each bar
                for(int i = 0; i < nBars; i++){
                    int bar = barNumbers[i];
                    
                    TString newTreeFileName = Form("hd_root_%s_bar%02d.root", runDir.Data(), bar);
                    TString outputPath = outputDir + newTreeFileName;
                    
                    cout << "  Creating " << newTreeFileName << endl;
                    TFile f2(outputPath, "recreate");
                    TTree *T2 = T->CopyTree(Form("DrcEvent.fId == %d", bar));
                    T2->Write();
                    f2.Close();
                }
            }
            
            delete T;
        }
    }
    gSystem->FreeDirectory(dirp);
    
    cout << "\n=== All runs processed ===" << endl;
}
