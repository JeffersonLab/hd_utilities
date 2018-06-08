
// 2010/09/02  David Lawrence
//
// This macro is used to generate an invariant mass plot of the X(2000) resonance
// and then fit it to a Voigt function before making a pretty plot.
//
// This relies on multiple macros to do various things. The bi_pi.C and b1_pi.h
// macros comprise an event selector class suitable for looping over all events
// creating and filling the invariant mass histogram. It does most of the work.
// the fitting to the Voigt function is done by the fit_voigt.C macro and finally
// some of the labels on the plot are placed there by the StandardLabels.C macro.
//
// To use this, one needs a hd_root.root that contains a tree made with the 
// phys_tree plugin in it. It should be in the current directory as should copies
// of all of the other macros mentioned above. Then one can generate the plot with:
//
// root -q -b mass_X.C
//

#include "StandardLabels.C"

#include "fit_voigt.C"

void mass_X(void)
{
// The following #if is here to make it easy to temporarily disable regenerating the
// the histograms by looping over all events which can be a little time consuming.
// Normally it should just be #if 1, but your tweaking this macro you may want to 
// make it #if 0 temporarily to speed things up a bit.
#if 1
	gROOT->Reset();
	
	TFile *f = new TFile("hd_root.root");
	f->cd("PHYSICS");
	TTree *recon = (TTree*)gROOT->FindObject("recon");
	
	recon->Process("b1_pi.C");
#endif	
	TH1D *mass_X_good = (TH1D*)gROOT->FindObject("mass_X_good");
	int Nfit_events = fit_voigt(mass_X_good, "0");
	TF1 *voigt = mass_X_good->GetFunction("voigt");
	
	TH1D *stats = (TH1D*)gROOT->FindObject("stats");
	double Ngood = stats->GetBinContent(1);
	double eff = Nfit_events/Ngood;
	
	TCanvas *c1 = new TCanvas("c1");
	c1->SetTicks();
	c1->SetGrid();
	
	mass_X_good->SetStats(0);
	mass_X_good->SetTitle("Reconstructed X(2000)#rightarrowb_{1}#pi");
	mass_X_good->SetXTitle("mass_{p#pi^{+}#pi^{+}#pi^{-}#pi^{-}#pi^{o}} (GeV/c^{2})");
	mass_X_good->Draw();
	voigt->Draw("same");
	
	double sf = mass_X_good->GetMaximum()*1.05/40.0;
	
	// Labels
	TPave *pave = new TPave(2.27, sf*19.52, 3.97, sf*28.38);
	pave->Draw();

	char str[256];
	sprintf(str,"Integral (curve): %3.1f events", Nfit_events);
	TLatex *lab = new TLatex(3.85, sf*27.0, str);
	lab->SetTextAlign(32);
	lab->SetTextSize(0.035);
	lab->Draw();
	
	sprintf(str,"Num. \"good\": %d events", (int)Ngood);
	TLatex *lab = new TLatex(3.85, sf*24.0, str);
	lab->SetTextAlign(32);
	lab->SetTextSize(0.035);
	lab->Draw();
	
	sprintf(str,"Efficiency: %3.1f%%", eff*100.0);
	TLatex *lab = new TLatex(3.85, sf*21.0, str);
	lab->SetTextAlign(32);
	lab->SetTextSize(0.035);
	lab->Draw();

	TLatex *lab = new TLatex(4.4, sf*38.0, "\"good\" = all final state particles (charged and neutral) are fiducial and");
	lab->SetTextAlign(13);
	lab->SetTextSize(0.025);
	lab->SetTextAngle(270);
	lab->Draw();
	TLatex *lab = new TLatex(4.3, sf*33.0, "only 2 photons reconstructed with m_{#gamma#gamma} within 200MeV of m_{#pi^{o}}");
	lab->SetTextAlign(13);
	lab->SetTextSize(0.025);
	lab->SetTextAngle(270);
	lab->Draw();

	StandardLabels1D(mass_X_good, "no background    
	#gammap#rightarrowpX#rightarrowpb_{1}#pi#rightarrowp#pi^{+}#pi^{+}#pi^{-}#pi^{-}#pi^{o}",
	"auto-generated");

	c1->SaveAs("mass_X.pdf");
	c1->SaveAs("mass_X.png");
}

