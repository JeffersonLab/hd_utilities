
#include "StandardLabels.C"

//----------------
// DrawValue
void DrawValue( double x, double y, double tsize, const char *lab, int val, int norm=0)
{	
	char str[256];
	if(norm==0){
		sprintf(str, "%5d", val);
	}else{
		sprintf(str, "%5d (%3.0f%%)", val, 100.0*(double)val/(double)norm);
	}
	
	TLatex latex;
	latex.SetTextSize(tsize);
	latex.SetTextAlign(31);
	latex.DrawLatex( x, y, lab);
	latex.SetTextAlign(11);
	latex.DrawLatex( 1.04*x, y, str);
}

//----------------
// iNjobs_vs_time
void iNjobs_vs_time(const char *start_tstr, int NJOBS_TOTAL=0, int NJOBS_SUCCEEDED=0, int RunMin=0, int RunMax=999999, const char *site="")
{
	// "Start" time of launch 
	// should match plot_start in regenerate_plots.csh for California
	// time or that +3hrs for Virginia time
	int y,m,d,h,M,s;
	sscanf(start_tstr, "%d-%d-%dT%d:%d:%d", &y, &m, &d, &h, &M, &s);
	TDatime da(y,m,d,h,M,s);
	//TDatime da(2018,9,20,16,20,00);
	auto da_sec = da.Convert() + 3*3600; // convert to seconds in Virginia time
	gStyle->SetTimeOffset(da_sec);


	TTree *t = new TTree("slurminfo", "SLURM Info.");
	t->ReadFile("slurm.csv", "tsubmit/F:tstart:tend:cpu:latency:ncpus:run/I:file");
	
	TCanvas *c1 = new TCanvas("c1", "", 1600,600);
	c1->SetGrid();
	c1->SetTicks();
	gPad->SetBottomMargin(0.3);
	gPad->SetRightMargin(0.02);
	
	char run_cut_str[256];
	sprintf(run_cut_str, "(run>=%d) && (run<=%d)", RunMin, RunMax);

	TH1D *nsubmitted_vs_time = new TH1D("nsubmitted_vs_time",   "", 5000E1, 0.0, 5000.0E3);
	TH1D *nstarted_vs_time = (TH1D*)nsubmitted_vs_time->Clone("nstarted_vs_time");
	TH1D *nended_vs_time = (TH1D*)nsubmitted_vs_time->Clone("nended_vs_time");
	t->Project("nsubmitted_vs_time", "tsubmit", run_cut_str);
	t->Project("nstarted_vs_time", "tstart", run_cut_str);
	t->Project("nended_vs_time", "tend", run_cut_str);
	for(int ibin=2; ibin<=nsubmitted_vs_time->GetNbinsX(); ibin++){
		double v0 = nsubmitted_vs_time->GetBinContent(ibin-1);
		double v1 = nsubmitted_vs_time->GetBinContent(ibin-0);
		nsubmitted_vs_time->SetBinContent(ibin, v0 + v1);

		v0 = nstarted_vs_time->GetBinContent(ibin-1);
		v1 = nstarted_vs_time->GetBinContent(ibin-0);
		nstarted_vs_time->SetBinContent(ibin, v0 + v1);

		v0 = nended_vs_time->GetBinContent(ibin-1);
		v1 = nended_vs_time->GetBinContent(ibin-0);
		nended_vs_time->SetBinContent(ibin, v0 + v1);
	}
	
	
	// Find maximum of tsubmit, tstart, and tend since that should indicate the
	// end of our time region.
	//
	// In order to restrict this to only entries for our run range
	// we have to make a TEventList and set this for the tree
	t->Draw(">>elist1", run_cut_str);
	TEventList *list = (TEventList*)gDirectory->Get("elist1");
	t->SetEventList(list);
	double tsubmit_max = t->GetMaximum("tsubmit");
	double tstart_max  = t->GetMaximum("tstart");
	double tend_max    = t->GetMaximum("tend");
	double tmax = tsubmit_max;
	if(tstart_max > tmax) tmax = tstart_max;
	if(tend_max   > tmax) tmax = tend_max;

	double ymax = NJOBS_TOTAL;
	if( nsubmitted_vs_time->GetMaximum() > ymax ) ymax = nsubmitted_vs_time->GetMaximum();
	if( nstarted_vs_time->GetMaximum() > ymax ) ymax = nstarted_vs_time->GetMaximum();
	ymax *= 1.10;
	string title = string(site) + " Integrated Num. Jobs vs. Time;;Number of jobs";
	TH2D *axes = new TH2D("axes", title.c_str(), 100, 0.0, tmax, 100, 0.0, ymax);
	axes->SetStats(0);
	axes->GetYaxis()->SetTitleSize(0.07);
	axes->GetYaxis()->SetTitleOffset(0.4);
	axes->GetXaxis()->SetTimeDisplay(1);
	axes->GetXaxis()->SetTimeFormat("%m/%d/%Y %H:%M");
	axes->GetXaxis()->SetLabelOffset(0.1);
	axes->GetXaxis()->SetNdivisions(610, kFALSE);
	for(auto i=1; i<=40; i++) axes->GetXaxis()->ChangeLabel(i, 270.0);
	axes->Draw();
	
	// Draw lines every 24hrs to help visually distinguish days
	for(uint64_t x=0; x<tmax; x+=24*3600){
		TLine *lin = new TLine(x, 0.0, x, ymax);
		lin->SetLineColor(kBlack);
		lin->SetLineWidth(2);
		lin->Draw();
	}

	// Draw Num. jobs
	nsubmitted_vs_time->SetLineColor(kRed);
	nsubmitted_vs_time->SetLineWidth(5);
	nsubmitted_vs_time->Draw("same");

	nstarted_vs_time->SetLineColor(kBlue);
	nstarted_vs_time->SetLineWidth(5);
	nstarted_vs_time->Draw("same");

	nended_vs_time->SetLineColor(kBlack);
	nended_vs_time->SetLineWidth(5);
	nended_vs_time->Draw("same");
	
	// Draw box shading unreliable region
	TBox *b = new TBox(tmax - 8.5*3600.0,0.0, tmax, axes->GetYaxis()->GetXmax());
	b->SetFillColor(kGray);
	b->SetFillStyle(3001);
	b->Draw();
	
	TLegend *leg1 = new TLegend(0.10, 0.85, 0.25, 0.9);
	leg1->AddEntry(nsubmitted_vs_time, "submitted");
	leg1->Draw();
	TLegend *leg2 = new TLegend(0.25, 0.85, 0.40, 0.9);
	leg2->AddEntry(nstarted_vs_time, "started");
	leg2->Draw();
	TLegend *leg3 = new TLegend(0.40, 0.85, 0.55, 0.9);
	leg3->AddEntry(nended_vs_time, "ended");
	leg3->Draw();
	
	// Add some statistics
		
	double x = 0.25*tmax;
	if(NJOBS_TOTAL>0){
		TLine *lin = new TLine(0.0, NJOBS_TOTAL, tmax, NJOBS_TOTAL);
		lin->SetLineColor(kMagenta);
		lin->SetLineStyle(2);
		lin->SetLineWidth(2);
		lin->Draw();
		DrawValue( x, 0.85*ymax, 0.04, "Jobs in workflow:", NJOBS_TOTAL);
	}
	DrawValue( x, 0.78*ymax, 0.04, "Jobs submitted:", nsubmitted_vs_time->GetMaximum(), NJOBS_TOTAL);
	DrawValue( x, 0.71*ymax, 0.04, "Jobs started:", nstarted_vs_time->GetMaximum(), NJOBS_TOTAL);
	DrawValue( x, 0.64*ymax, 0.04, "Jobs ended:", nended_vs_time->GetMaximum(), NJOBS_TOTAL);
	DrawValue( x, 0.57*ymax, 0.04, "Jobs succeeded:", NJOBS_SUCCEEDED, NJOBS_TOTAL);
	
	StandardLabels(axes);

	c1->SaveAs("iNjobs_vs_time.png");
	c1->SaveAs("iNjobs_vs_time.pdf");
}


