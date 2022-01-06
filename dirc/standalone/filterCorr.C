// filter LUT corrections for CCDB entry
void filterCorr(TString filename = "allbars_ver1.1.corr.root") {

	TFile *fLutCorr; 
	TTree *tLutCorr;
	fLutCorr = TFile::Open(filename);
	tLutCorr=(TTree*) fLutCorr->Get("corr");

	TString filenameFilt = filename.ReplaceAll(".corr.root",".corrfilt.root");
	TFile fLutCorrFilt(filenameFilt, "recreate");
	TTree *tLutCorrFilt = new TTree("corr", "corr");
	double zcorrAD, zsigmaAD, zcorrAR, zsigmaAR, zcorrTD, zsigmaTD, zcorrTR, zsigmaTR, zfracAD, zfracAR;
	int zbar, zbin, zpmt, zlevel;
	tLutCorrFilt->Branch("zcorrAD", &zcorrAD, "zcorrAD/D");
	tLutCorrFilt->Branch("zcorrAR", &zcorrAR, "zcorrAR/D");
	tLutCorrFilt->Branch("zcorrTD", &zcorrTD, "zcorrTD/D");
	tLutCorrFilt->Branch("zcorrTR", &zcorrTR, "zcorrTR/D");
	tLutCorrFilt->Branch("zsigmaAD", &zsigmaAD, "zsigmaAD/D");
	tLutCorrFilt->Branch("zsigmaAR", &zsigmaAR, "zsigmaAR/D");
	tLutCorrFilt->Branch("zsigmaTD", &zsigmaTD, "zsigmaTD/D");
	tLutCorrFilt->Branch("zsigmaTR", &zsigmaTR, "zsigmaTR/D");
	tLutCorrFilt->Branch("zfracAD", &zfracAD, "zfracAD/D");
	tLutCorrFilt->Branch("zfracAR", &zfracAR, "zfracAR/D");
	tLutCorrFilt->Branch("bar", &zbar, "bar/I");
	tLutCorrFilt->Branch("bin", &zbin, "bin/I");
	tLutCorrFilt->Branch("pmt", &zpmt, "pmt/I");
	tLutCorrFilt->Branch("level", &zlevel, "level/I");

	double corrAD, corrAR, sigmaAD, sigmaAR, corrTD, corrTR, sigmaTD, sigmaTR, fracAD, fracAR;
	int tb, tp, tbin, tlevel;
	tLutCorr->SetBranchAddress("bar", &tb);
	tLutCorr->SetBranchAddress("pmt", &tp);
	tLutCorr->SetBranchAddress("bin", &tbin);
	tLutCorr->SetBranchAddress("level", &tlevel);
	tLutCorr->SetBranchAddress("zcorrAD", &corrAD);
	tLutCorr->SetBranchAddress("zcorrAR", &corrAR);
	tLutCorr->SetBranchAddress("zcorrTD", &corrTD);
	tLutCorr->SetBranchAddress("zcorrTR", &corrTR);
	tLutCorr->SetBranchAddress("zsigmaTD", &sigmaTD);
	tLutCorr->SetBranchAddress("zsigmaTR", &sigmaTR);
	tLutCorr->SetBranchAddress("zsigmaAD", &sigmaAD);
	tLutCorr->SetBranchAddress("zsigmaAR", &sigmaAR);
	tLutCorr->SetBranchAddress("zfracAD", &fracAD);
	tLutCorr->SetBranchAddress("zfracAR", &fracAR);
	
	// fill arrays with per-PMT corrections
	bool fillEntry;
	for (int i = 0; i < tLutCorr->GetEntries(); i++) {
		tLutCorr->GetEvent(i);
		zbar = tb; zbin = tbin; zpmt = tp; zlevel = tlevel;
		zcorrTD = 0; zcorrTR = 0; zsigmaTD = 0; zsigmaTR = 0;
		zcorrAD = 0; zcorrAR = 0; zsigmaAD = 0; zsigmaAR = 0;
		zfracAD = 0; zfracAR = 0;
		fillEntry = false;

		if(fabs(corrTD) < 1.9 && sigmaTD < 2.0 && sigmaTD > 0.25) {
			fillEntry = true;
			zcorrTD = corrTD;
			zsigmaTD = sigmaTD;
			if(fracAD > 0.2 && fracAD < 2.0 && fabs(corrAD) < 9.0) {
				zcorrAD = corrAD;
				zsigmaAD = sigmaAD;
				zfracAD = fracAD;
			}
		}
		if(fabs(corrTR) < 2.9 && sigmaTR < 3.0 && sigmaTR > 0.3) {
			fillEntry = true;
			zcorrTR = corrTR;
			zsigmaTR = sigmaTR;
			if(fracAR > 0.2 && fracAR < 2.0  && fabs(corrAR) < 9.0) {
				zcorrAR = corrAR;
				zsigmaAR = sigmaAR;
				zfracAR = fracAR;
			}
		}
		
		if(fillEntry)
			tLutCorrFilt->Fill();
	}					


	// close LUT correction file
	fLutCorr->Close();
	tLutCorrFilt->Write();
	fLutCorrFilt.Close();
}

