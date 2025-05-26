// plot LUT corrections and differences
void plotCorr() {

	TH1F *hAD1 = new TH1F("hAD1","hAD1",200,-10,10);
	TH1F *hAD2 = new TH1F("hAD2","hAD2",200,-10,10);
	TH1F *hAR1 = new TH1F("hAR1","hAR1",200,-10,10);
	TH1F *hAR2 = new TH1F("hAR2","hAR2",200,-10,10);
	TH1F *hDiffAD = new TH1F("hDiffAD","hDiffAD",200,-20,20);
	TH1F *hDiffAR = new TH1F("hDiffAR","hDiffAR",200,-20,20);
	TH2F *hCorrAD = new TH2F("hCorrAD","hCorrAD",100,-10,10,100,-10,10);
	TH2F *hCorrAR = new TH2F("hCorrAR","hCorrAR",100,-10,10,100,-10,10);

	TH1F *hTD1 = new TH1F("hTD1","hTD1",200,-5,5);
	TH1F *hTD2 = new TH1F("hTD2","hTD2",200,-5,5);
	TH1F *hTR1 = new TH1F("hTR1","hTR1",200,-5,5);
	TH1F *hTR2 = new TH1F("hTR2","hTR2",200,-5,5);
	TH1F *hDiffTD = new TH1F("hDiffTD","hDiffTD",200,-5,5);
	TH1F *hDiffTR = new TH1F("hDiffTR","hDiffTR",200,-5,5);
	TH2F *hCorrTD = new TH2F("hCorrTD","hCorrTD",100,-5,5,100,-5,5);
	TH2F *hCorrTR = new TH2F("hCorrTR","hCorrTR",100,-5,5,100,-5,5);

	TFile *fLutCorr[2]; 
	TTree *tLutCorr[2];
	fLutCorr[0] = TFile::Open("allbars_ver0.9.corr.root");
	tLutCorr[0]=(TTree*) fLutCorr[0]->Get("corr");
	fLutCorr[1] = TFile::Open("all_m2_allbars_sel.root.corr.root");
	tLutCorr[1]=(TTree*) fLutCorr[1]->Get("corr");

	const int nbins = tLutCorr[0]->GetMaximum("bin")+1;
	double corrAD, corrAR, sigmaAD, sigmaAR, corrTD, corrTR, sigmaTD, sigmaTR, fracAD, fracAR;
	int tb, tp, tbin;

	vector<double> lutCorrAngleDirect[48][108][2];
	vector<double> lutCorrAngleReflected[48][108][2];
	vector<double> lutCorrTimeDirect[48][108][2];
	vector<double> lutCorrTimeReflected[48][108][2];

	// clear arrays to fill from TTree
	for(int ifile=0; ifile<2; ifile++) {
		tLutCorr[ifile]->SetBranchAddress("bar", &tb);
		tLutCorr[ifile]->SetBranchAddress("pmt", &tp);
		tLutCorr[ifile]->SetBranchAddress("bin", &tbin);
		tLutCorr[ifile]->SetBranchAddress("zcorrAD", &corrAD);
		tLutCorr[ifile]->SetBranchAddress("zcorrAR", &corrAR);
		tLutCorr[ifile]->SetBranchAddress("zcorrTD", &corrTD);
		tLutCorr[ifile]->SetBranchAddress("zcorrTR", &corrTR);
		tLutCorr[ifile]->SetBranchAddress("zsigmaTD", &sigmaTD);
		tLutCorr[ifile]->SetBranchAddress("zsigmaTR", &sigmaTR);
		tLutCorr[ifile]->SetBranchAddress("zsigmaAD", &sigmaAD);
		tLutCorr[ifile]->SetBranchAddress("zsigmaAR", &sigmaAR);
		tLutCorr[ifile]->SetBranchAddress("zfracAD", &fracAD);
		tLutCorr[ifile]->SetBranchAddress("zfracAR", &fracAR);

		for(int bar=0; bar<48; bar++){
			for(int pmt=0; pmt<108; pmt++){
				for(int bin=0; bin<nbins; bin++){
					lutCorrAngleDirect[bar][pmt][ifile].push_back(0);
					lutCorrAngleReflected[bar][pmt][ifile].push_back(0);
					lutCorrTimeDirect[bar][pmt][ifile].push_back(0);
					lutCorrTimeReflected[bar][pmt][ifile].push_back(0);
				}
			}
		}
	}

	// fill arrays with per-PMT corrections
	for(int ifile=0; ifile<2; ifile++) {
		for (int i = 0; i < tLutCorr[ifile]->GetEntries(); i++) {
			tLutCorr[ifile]->GetEvent(i);
			if(ifile==0 && fabs(corrTD) < 1.9 && sigmaTD < 2.0 && sigmaTD > 0.25) {
				lutCorrTimeDirect[tb][tp][ifile].at(tbin) = corrTD;
				if(fabs(corrAD) < 9.0 && fracAD > 0.2)
					lutCorrAngleDirect[tb][tp][ifile].at(tbin) = corrAD;
			}
			if(ifile==1 && fabs(corrTD) < 1.9) {
			        lutCorrTimeDirect[tb][tp][ifile].at(tbin) = corrTD;
				if(fabs(corrAD) < 9.0)
					lutCorrAngleDirect[tb][tp][ifile].at(tbin) = corrAD;
			}
			
			if(ifile==0 && fabs(corrTR) < 2.9 && sigmaTR < 3.0 && sigmaTR > 0.3) {
				lutCorrTimeReflected[tb][tp][ifile].at(tbin) = corrTR;
				if(fabs(corrAR) < 9.0 && fracAR > 0.2)
					lutCorrAngleReflected[tb][tp][ifile].at(tbin) = corrAR;
			}
			if(ifile==1 && fabs(corrTR) < 2.9) {
				lutCorrTimeReflected[tb][tp][ifile].at(tbin) = corrTR;
				if(fabs(corrAR) < 9.0)
					lutCorrAngleReflected[tb][tp][ifile].at(tbin) = corrAR;
			}
		}
	}					

	for(int ibar=0; ibar<48; ibar++) {
		for(int ibin=0; ibin<10; ibin++) {
			for(int ipmt=0; ipmt<108; ipmt++) {
				if(lutCorrAngleDirect[ibar][ipmt][0].at(ibin)!=0.0 && lutCorrAngleDirect[ibar][ipmt][1].at(ibin)!=0.0) {
					hAD1->Fill(lutCorrAngleDirect[ibar][ipmt][0].at(ibin));
					hAD2->Fill(lutCorrAngleDirect[ibar][ipmt][1].at(ibin));
					hDiffAD->Fill(lutCorrAngleDirect[ibar][ipmt][0].at(ibin) - lutCorrAngleDirect[ibar][ipmt][1].at(ibin));
					hCorrAD->Fill(lutCorrAngleDirect[ibar][ipmt][0].at(ibin), lutCorrAngleDirect[ibar][ipmt][1].at(ibin));
				}
					
				
				if(lutCorrAngleReflected[ibar][ipmt][0].at(ibin)!=0.0 || lutCorrAngleReflected[ibar][ipmt][1].at(ibin)!=0.0) {
					hAR1->Fill(lutCorrAngleReflected[ibar][ipmt][0].at(ibin));
					hAR2->Fill(lutCorrAngleReflected[ibar][ipmt][1].at(ibin));
					hDiffAR->Fill(lutCorrAngleReflected[ibar][ipmt][0].at(ibin) - lutCorrAngleReflected[ibar][ipmt][1].at(ibin));
					hCorrAR->Fill(lutCorrAngleReflected[ibar][ipmt][0].at(ibin), lutCorrAngleReflected[ibar][ipmt][1].at(ibin));	
				}

				if(lutCorrTimeDirect[ibar][ipmt][0].at(ibin)!=0.0 && lutCorrTimeDirect[ibar][ipmt][1].at(ibin)!=0.0) {
					hTD1->Fill(lutCorrTimeDirect[ibar][ipmt][0].at(ibin));
					hTD2->Fill(lutCorrTimeDirect[ibar][ipmt][1].at(ibin));
					hDiffTD->Fill(lutCorrTimeDirect[ibar][ipmt][0].at(ibin) - lutCorrTimeDirect[ibar][ipmt][1].at(ibin));
					hCorrTD->Fill(lutCorrTimeDirect[ibar][ipmt][0].at(ibin), lutCorrTimeDirect[ibar][ipmt][1].at(ibin));
				}
				
				if(lutCorrTimeReflected[ibar][ipmt][0].at(ibin)!=0.0 && lutCorrTimeReflected[ibar][ipmt][1].at(ibin)!=0.0) {
					hTR1->Fill(lutCorrTimeReflected[ibar][ipmt][0].at(ibin));
					hTR2->Fill(lutCorrTimeReflected[ibar][ipmt][1].at(ibin));
					hDiffTR->Fill(lutCorrTimeReflected[ibar][ipmt][0].at(ibin) - lutCorrTimeReflected[ibar][ipmt][1].at(ibin));
					hCorrTR->Fill(lutCorrTimeReflected[ibar][ipmt][0].at(ibin), lutCorrTimeReflected[ibar][ipmt][1].at(ibin));
				}
			}
		}
	}

	// close LUT correction file
	fLutCorr[0]->Close();
	fLutCorr[1]->Close();

	TCanvas *cc = new TCanvas("cc","cc",800,600);
	cc->Divide(2,2);
	cc->cd(1);
	hAD1->Draw();
	cc->cd(2);
	hAD2->Draw();
	cc->cd(3);
	hDiffAD->Draw();
	cc->cd(4);
	hCorrAD->Draw("colz");

	TCanvas *ccR = new TCanvas("ccR","ccR",800,600);
	ccR->Divide(2,2);
	ccR->cd(1);
	hAR1->Draw();
	ccR->cd(2);
	hAR2->Draw();
	ccR->cd(3);
	hDiffAR->Draw();
	ccR->cd(4);
	hCorrAR->Draw("colz");

	TCanvas *dd = new TCanvas("dd","dd",800,600);
	dd->Divide(2,2);
	dd->cd(1);
	hTD1->Draw();
	dd->cd(2);
	hTD2->Draw();
	dd->cd(3);
	hDiffTD->Draw();
	dd->cd(4);
	hCorrTD->Draw("colz");

	TCanvas *ddR = new TCanvas("ddR","ddR",800,600);
	ddR->Divide(2,2);
	ddR->cd(1);
	hTR1->Draw();
	ddR->cd(2);
	hTR2->Draw();
	ddR->cd(3);
	hDiffTR->Draw();
	ddR->cd(4);
	hCorrTR->Draw("colz");
}
