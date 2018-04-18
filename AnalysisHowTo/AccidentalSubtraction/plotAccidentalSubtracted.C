
void plotAccidentalSubtracted() {

	gStyle->SetOptStat(0);

	// get histograms from file
	TFile *f = TFile::Open("hist_gg.root");
	TH1F *locHist_BeamDeltaT = (TH1F*)f->Get("BeamDeltaT");
	TH1F *locHist_MissingMassSquared_weighted = (TH1F*)f->Get("MissingMassSquared_weighted");
	TH1F *locHist_MissingMassSquared_prompt = (TH1F*)f->Get("MissingMassSquared_prompt");
	TH1F *locHist_MissingMassSquared_accid = (TH1F*)f->Get("MissingMassSquared_accid");
       
	// IMPORTANT: Need to set Sumw2 to use weight squared in histogram for subtraction of prompt and accidental histograms.  If this is not done in DSelector when histogram is defined, do it here before subtraction.
	//locHist_MissingMassSquared_prompt->Sumw2();
	//locHist_MissingMassSquared_accid->Sumw2();

	// create accidental subtracted histograms from separate prompt and accidental histograms
	TH1F *locHist_MissingMassSquared_accidSub = (TH1F*)locHist_MissingMassSquared_prompt->Clone("MissingMassSquared_accidSum");
	locHist_MissingMassSquared_accidSub->Add(locHist_MissingMassSquared_accid, -1./6.);

	// set all histogram minima to zero so all have the same scale
	locHist_MissingMassSquared_weighted->SetMinimum(0);
	locHist_MissingMassSquared_accidSub->SetMinimum(0);
	locHist_MissingMassSquared_prompt->SetMinimum(0);
	
	TCanvas *cc = new TCanvas("cc", "cc", 1000, 700);
	cc->Divide(2,2);
	cc->cd(1);
	locHist_BeamDeltaT->Draw();

	// draw weighted histogram from DSelector (accidental subtraction already done with weights)
	cc->cd(2);
	
	locHist_MissingMassSquared_weighted->Draw();

	TLegend *leg2 = new TLegend(0.2, 0.7, 0.45, 0.85);
	leg2->SetBorderSize(0);
	leg2->AddEntry(locHist_MissingMassSquared_weighted, "Weighted", "l");
	leg2->Draw("same");

	// draw prompt and scaled accidental subtracted histograms to see what is being subtracted
	cc->cd(3);
	
	locHist_MissingMassSquared_prompt->SetLineColor(kRed);
	locHist_MissingMassSquared_accid->SetLineColor(kBlue);
	locHist_MissingMassSquared_accid->Scale(1./6.);
	locHist_MissingMassSquared_prompt->Draw();
	locHist_MissingMassSquared_accid->Draw("same");

	TLegend *leg3 = new TLegend(0.175, 0.7, 0.48, 0.9);
	leg3->SetBorderSize(0);
	leg3->AddEntry(locHist_MissingMassSquared_prompt, "|#Delta T| < 2.004 ns", "l");
	leg3->AddEntry(locHist_MissingMassSquared_accid, "|#Delta T| > 2.004 ns", "l");
	leg3->Draw();

	// draw accidental subtracted histogram from subtraction of 2 histograms done in this macro
	cc->cd(4);

	locHist_MissingMassSquared_accidSub->SetLineColor(kMagenta);
	locHist_MissingMassSquared_accidSub->Draw("e");

	TLegend *leg4 = new TLegend(0.2, 0.7, 0.45, 0.85);
	leg4->SetBorderSize(0);
	leg4->AddEntry(locHist_MissingMassSquared_accidSub, "Subtracted", "l");
	leg4->Draw();

	return;
}
