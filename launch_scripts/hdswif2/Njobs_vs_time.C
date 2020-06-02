
#include "StandardLabels.C"
#include "njobs.h"

void Njobs_vs_time(const char *start_tstr, int RunMin=0, int RunMax=999999, const char *site="")
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

	auto selector = (njobs*)TSelector::GetSelector("njobs.C");
	selector->SetRunRange( RunMin, RunMax );
	t->Process(selector);
	TH1D *njobs_vs_time = (TH1D*)gROOT->FindObject("njobs_vs_time");
	TH1D *nqueued_vs_time = (TH1D*)gROOT->FindObject("nqueued_vs_time");
	
	// Find maximum of tsubmit, tstart, and tend since that should indicate the
	// end of our time region.
	double tsubmit_max = t->GetMaximum("tsubmit");
	double tstart_max  = t->GetMaximum("tstart");
	double tend_max    = t->GetMaximum("tend");
	double tmax = tsubmit_max;
	if(tstart_max > tmax) tmax = tstart_max;
	if(tend_max   > tmax) tmax = tend_max;

	double ymax = 1.10*nqueued_vs_time->GetMaximum();
	if( njobs_vs_time->GetMaximum() > nqueued_vs_time->GetMaximum() ) ymax = 1.10*njobs_vs_time->GetMaximum();
	string title = string(site) + " Instantaneous Num. Jobs vs. Time;;Number of jobs";
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
	njobs_vs_time->SetLineColor(kRed);
	njobs_vs_time->SetLineWidth(5);
	njobs_vs_time->Draw("same");

	nqueued_vs_time->SetLineColor(kBlue);
	nqueued_vs_time->SetLineWidth(5);
	nqueued_vs_time->Draw("same");
	
	// Draw box shading unreliable region
	TBox *b = new TBox(tmax - 4.5*3600.0,0.0, tmax, axes->GetYaxis()->GetXmax());
	b->SetFillColor(kGray);
	b->SetFillStyle(3001);
	b->Draw();
	
	TLegend *leg1 = new TLegend(0.10, 0.85, 0.30, 0.9);
	leg1->AddEntry(nqueued_vs_time, "Queued Jobs");
	leg1->Draw();
	TLegend *leg2 = new TLegend(0.30, 0.85, 0.50, 0.9);
	leg2->AddEntry(njobs_vs_time, "Running Jobs");
	leg2->Draw();
	
	StandardLabels(axes);

	c1->SaveAs("Njobs_vs_time.png");
	c1->SaveAs("Njobs_vs_time.pdf");
}


