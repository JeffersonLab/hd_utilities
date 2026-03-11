void copyTreeMC(TString treesDir = "trees") {
    gROOT->Reset();

    // Verify input trees directory exists
    if (gSystem->AccessPathName(treesDir.Data()) != 0) {
        cout << "Error: cannot access directory " << treesDir << endl;
        return;
    }

    // Only process the same specific bar IDs used in copyTree.C
    int barNumbers[] = {3, 7, 15, 19, 27, 31, 39, 43};
    const int nBars = 8;

    // Create bar subdirectories: trees/bar3, trees/bar7, ...
    for (int i = 0; i < nBars; ++i) {
        int bar = barNumbers[i];
        TString barDir = Form("%s/bar_%d", treesDir.Data(), bar);
        gSystem->Exec(Form("mkdir -p %s", barDir.Data()));
    }

    // Loop over ROOT files directly under trees/
    void* dirp = gSystem->OpenDirectory(treesDir.Data());
    if (!dirp) {
        cout << "Error: cannot open directory " << treesDir << endl;
        return;
    }

    const char* entry;
    int nInputFiles = 0;
    while ((entry = gSystem->GetDirEntry(dirp))) {
        TString fileName = entry;
        if (fileName == "." || fileName == "..") continue;
        if (!fileName.EndsWith(".root")) continue;

        TString inputPath = Form("%s/%s", treesDir.Data(), fileName.Data());

        // Skip directories or inaccessible entries
        if (gSystem->AccessPathName(inputPath.Data()) != 0) continue;

        nInputFiles++;
        cout << "\n=== Processing file " << fileName << " ===" << endl;

        TFile* inFile = TFile::Open(inputPath, "READ");
        if (!inFile || inFile->IsZombie()) {
            cout << "  Error: failed to open " << inputPath << endl;
            if (inFile) {
                inFile->Close();
                delete inFile;
            }
            continue;
        }

        TTree* inTree = (TTree*)inFile->Get("dirc");
        if (!inTree) {
            cout << "  Warning: tree 'dirc' not found in " << fileName << ", skipping" << endl;
            inFile->Close();
            delete inFile;
            continue;
        }

        cout << "  Input entries: " << inTree->GetEntries() << endl;

        for (int i = 0; i < nBars; ++i) {
            int bar = barNumbers[i];
            TString outDir = Form("%s/bar_%d", treesDir.Data(), bar);
            TString outPath = Form("%s/%s", outDir.Data(), fileName.Data());

            TFile outFile(outPath, "RECREATE");
            TTree* outTree = inTree->CopyTree(Form("DrcEvent.fId == %d", bar));
            if (outTree) {
                outTree->Write();
                cout << "  bar" << bar << ": " << outTree->GetEntries() << " entries -> " << outPath << endl;
            }
            outFile.Close();
        }

        inFile->Close();
        delete inFile;
    }

    gSystem->FreeDirectory(dirp);

    cout << "\n=== Done ===" << endl;
    cout << "Processed " << nInputFiles << " input ROOT files from " << treesDir << endl;
}
