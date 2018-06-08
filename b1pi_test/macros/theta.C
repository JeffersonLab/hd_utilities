//
// July 29, 2010  David Lawrence
//
// This macro will produce 4 plots (on one canvas) showing
// the theta angle for both thrown and reconstructed
// particles. The 4 plots correspond to the different particle
// types: p pi+ pi- gamma.
//
// A fiducial cut is applied on the total momentum of the thrown
// particles. For the reconstructed particles, the same
// cut is applied to the thrown particle that was determined
// to correspond to the reconstructed particle. To adjust
// the cuts, look for the p_min and p_max
// variables in this file.
// 
// This macro will open the file "hd_root.root" in the local
// directory. This macro takes no arguments.
//
// e.g.  From the unix command line:
//
//   root -q -b theta.C
//
//
//  From the ROOT prompt:
//
//  root [0] .x theta.C
//

//-----------------
// theta
//-----------------
void theta(void)
{
	gROOT->Reset();
	TColor::CreateColorWheel();
	
	// Open ROOT file and get pointers to thrown and recon trees
	TFile *f = new TFile("hd_root.root");
	f->cd("PHYSICS");
	TTree *thrown = (TTree*)gROOT->FindObject("thrown");
	TTree *recon = (TTree*)gROOT->FindObject("recon");

	// Create canvas to draw on and divide into 4 sections
	TCanvas *c1 = new TCanvas("c1");
	c1->Divide(2,2);
	
	// pip
	c1->cd(1);
	PlotParticle(thrown, recon, "pip", "#pi^{+}");

	// pim
	c1->cd(2);
	PlotParticle(thrown, recon, "pim", "#pi^{-}");

	// proton
	c1->cd(3);
	PlotParticle(thrown, recon, "proton", "proton");

	// photon
	c1->cd(4);
	PlotParticle(thrown, recon, "photon", "#gamma");
	
	// Save plot to output files
	c1->SaveAs("theta.pdf");
	c1->SaveAs("theta.png");
}

//-----------------
// PlotParticle
//-----------------
void PlotParticle(TTree *thrown, TTree *recon, const char *pname, const char *title)
{
	// This routine will draw the theta distribution for
	// thrown and reconstructed particles onto the current
	// canvas/pad for the particle given by pname. The value
	// of pname should be one of:"pip", "pim", "proton", "gamma"
	
	// Set the current pad to draw ticks on the right-side and
	// top-side axes as well as gridlines on the plot.
	gPad->SetTicks();
	gPad->SetGrid();
	
	// Generate unique histogram names based on pname
	char hname_thrown[256];
	char hname_recon[256];
	sprintf(hname_thrown, "thrown_%s", pname);
	sprintf(hname_recon, "recon_%s", pname);

	// Create histograms and set drawing attributes
	TH1D *hthrown = new TH1D(hname_thrown, title, 50, 0.0, 140.0);
	hthrown->SetLineWidth(3);
	hthrown->SetLineColor(kBlue);
	hthrown->SetXTitle("#theta (degrees)");
	hthrown->SetStats(0); // supress stats box when plotting
	TH1D *hrecon = (TH1D*)hthrown->Clone(hname_recon);
	hrecon->SetLineColor(kRed);
	
	// Set rough fiducial cut on theta based on detector geometry
	double p_min = 0.35; // GeV/c
	double p_max = 12.0; // GeV/c
	char cut_thrown[256];	
	char cut_recon[256];
	sprintf(cut_thrown, "T.%s.p.P()>%f && T.%s.p.P()<%f", pname, p_min, pname, p_max);
	sprintf(cut_recon, "R.%s_match.p.P()>%f && R.%s_match.p.P()<%f", pname, p_min, pname, p_max);
	
	// Create varexp based on particle type (pname) and project onto histos
	char varexp_thrown[256];
	char varexp_recon[256];
	sprintf(varexp_thrown, "T.%s.p.Theta()*TMath::RadToDeg()", pname);
	sprintf(varexp_recon, "R.%s.p.Theta()*TMath::RadToDeg()", pname);
	thrown->Project(hname_thrown, varexp_thrown, cut_thrown);
	recon->Project(hname_recon, varexp_recon, cut_recon);
	
	// Draw histos, overlaying one with smaller maximum on one with bigger
	if(hthrown->GetMaximum() > hrecon->GetMaximum()){
		hthrown->Draw();
		hrecon->Draw("same");
	}else{
		hrecon->Draw();
		hthrown->Draw("same");
	}
	
	// Draw Legend
	TLegend *leg = new TLegend(0.6, 0.7, 0.89, 0.89);
	leg->SetFillColor(kWhite);
	leg->AddEntry(hthrown, "Generated");
	leg->AddEntry(hrecon, "Reconstructed");
	leg->Draw();

	// Update the current TPad so the GetY2 method will work properly below
	gPad->Update();

	// Add Label with momentum cuts
	char str[256];
	sprintf(str, "%2.1fGeV/c #leq p #leq %2.0fGeV/c", p_min, p_max);
	TLatex *lab = new TLatex(87.0, 0.955*gPad->GetY2(), str);
	lab->SetTextAlign(21);
	lab->SetTextSize(0.035);
	lab->Draw();

	// Add Label with current user
	TLatex *lab = new TLatex(53.0, 0.91*gPad->GetY2(), getenv("USER"));
	lab->SetTextAlign(31);
	lab->SetTextSize(0.05);
	lab->Draw();

	// Add Label with current host
	TLatex *lab = new TLatex(56.0, 0.91*gPad->GetY2(), getenv("HOST"));
	lab->SetTextAlign(11);
	lab->SetTextSize(0.04);
	lab->Draw();
	
	// Add Label with current date/time
	time_t t = time(NULL);
	TLatex *lab = new TLatex(155.0, 0.91*gPad->GetY2(), ctime(&t));
	lab->SetTextAlign(31);
	lab->SetTextSize(0.05);
	lab->Draw();
}


