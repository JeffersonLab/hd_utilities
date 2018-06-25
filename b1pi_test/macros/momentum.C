//
// July 29, 2010  David Lawrence
//
// This macro will produce 4 plots (on one canvas) showing
// the total momentum for both thrown and reconstructed
// particles. The 4 plots correspond to the different particle
// types: p pi+ pi- gamma.
//
// A fiducial cut is applied on the theta angle of the thrown
// particles. For the reconstructed particles, the same
// cut is applied to the thrown particle that was determined
// to correspond to the reconstructed particle. To adjust
// the theta cuts, look for the theta_min and theta_max
// variables in this file.
// 
// This macro will open the file "hd_root.root" in the local
// directory. This macro takes no arguments.
//
// e.g.  From the unix command line:
//
//   root -q -b momentum.C
//
//
//  From the ROOT prompt:
//
//  root [0] .x momentum.C
//

//-----------------
// momentum
//-----------------
void momentum(void)
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
	c1->SaveAs("momentum.pdf");
	c1->SaveAs("momentum.png");
}

//-----------------
// PlotParticle
//-----------------
void PlotParticle(TTree *thrown, TTree *recon, const char *pname, const char *title)
{
	// This routine will draw the momentum distribution for
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
	TH1D *hthrown = new TH1D(hname_thrown, title, 50, 0.0, 6.0);
	hthrown->SetLineWidth(3);
	hthrown->SetLineColor(kBlue);
	hthrown->SetXTitle("Total Momentum (GeV/c)");
	hthrown->SetStats(0); // supress stats box when plotting
	TH1D *hrecon = (TH1D*)hthrown->Clone(hname_recon);
	hrecon->SetLineColor(kRed);
	
	// Set rough fiducial cut on theta based on detector geometry
	double theta_min = 2.0; // degrees
	double theta_max = 120.0; // degrees
	char cut_thrown[256];	
	char cut_recon[256];
	sprintf(cut_thrown, "T.%s.p.Theta()*TMath::RadToDeg()>%f && T.%s.p.Theta()*TMath::RadToDeg()<%f", pname, theta_min, pname, theta_max);
	sprintf(cut_recon, "R.%s_match.p.Theta()*TMath::RadToDeg()>%f && R.%s_match.p.Theta()*TMath::RadToDeg()<%f", pname, theta_min, pname, theta_max);
	
	// Create varexp based on particle type (pname) and project onto histos
	char varexp_thrown[256];
	char varexp_recon[256];
	sprintf(varexp_thrown, "T.%s.p.P()", pname);
	sprintf(varexp_recon, "R.%s.p.P()", pname);
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

	// Add Label with theta cuts
	char str[256];
	sprintf(str, "%2.0f^{o} #leq #theta #leq %2.0f^{o}", theta_min, theta_max);
	TLatex *lab = new TLatex(3.1, 0.955*gPad->GetY2(), str);
	lab->SetTextAlign(21);
	lab->SetTextSize(0.035);
	lab->Draw();

	// Add Label with current user
	TLatex *lab = new TLatex(1.9, 0.91*gPad->GetY2(), getenv("USER"));
	lab->SetTextAlign(31);
	lab->SetTextSize(0.05);
	lab->Draw();

	// Add Label with current host
	TLatex *lab = new TLatex(2.0, 0.91*gPad->GetY2(), getenv("HOST"));
	lab->SetTextAlign(11);
	lab->SetTextSize(0.04);
	lab->Draw();
	
	// Add Label with current date/time
	time_t t = time(NULL);
	TLatex *lab = new TLatex(6.5, 0.91*gPad->GetY2(), ctime(&t));
	lab->SetTextAlign(31);
	lab->SetTextSize(0.05);
	lab->Draw();
}


