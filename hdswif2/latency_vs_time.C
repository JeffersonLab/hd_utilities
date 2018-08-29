
#include "StandardLabels.C"

void latency_vs_time(void)
{
	// "Start" time of launch
	TDatime da(2018,8,21,14,00,00);
	auto da_sec = da.Convert();
	gStyle->SetTimeOffset(da_sec);


	TTree *t = new TTree("slurminfo", "SLURM Info.");
	t->ReadFile("slurm.csv", "tsubmit/F:tstart:tend:cpu:latency");

	TCanvas *c1 = new TCanvas("c1", "", 1600,600);
	c1->SetGrid();
	c1->SetTicks();
	gPad->SetBottomMargin(0.3);
	gPad->SetRightMargin(0.02);

	// Find maximum of tsubmit, tstart, and tend since that should indicate the
	// end of our time region.
	double tsubmit_max = t->GetMaximum("tsubmit");
	double tstart_max  = t->GetMaximum("tstart");
	double tend_max    = t->GetMaximum("tend");
	double tmax = tsubmit_max;
	if(tstart_max > tmax) tmax = tstart_max;
	if(tend_max   > tmax) tmax = tend_max;

	double ymax = 40.0;
	TH2D *axes = new TH2D("axes", "NERSC Job Start Latency vs. Submit Time (completed jobs only);;Latency (hrs)", 100, 0.0, tmax, 100, 0.0, ymax);
	axes->SetStats(0);
	axes->GetYaxis()->SetTitleSize(0.07);
	axes->GetYaxis()->SetTitleOffset(0.4);
	axes->GetXaxis()->SetTimeDisplay(1);
	axes->GetXaxis()->SetTimeFormat("%m/%d/%Y %H:%M");
	axes->GetXaxis()->SetLabelOffset(0.1);
	for(auto i=1; i<=40; i++) axes->GetXaxis()->ChangeLabel(i, 270.0);
	axes->Draw();
	
	// Draw lines at midnight to help visually distinguish days
	for(auto d=22; d< 31; d++){
		TDatime da(2018,8,d,0,0,00);
		double x = da.Convert() - da_sec;
		TLine *lin = new TLine(x, 0.0, x, ymax);
		lin->SetLineColor(kBlack);
		lin->SetLineWidth(2);
		lin->Draw();
	}

	t->SetMarkerStyle(20);
	t->SetMarkerColor(kBlue);
	t->Draw("latency/3600.0:tsubmit", "", "same");

	// Draw box shading unreliable region
	TBox *b = new TBox(tmax - 4.5*3600.0,0.0, tmax, axes->GetYaxis()->GetXmax());
	b->SetFillColor(kGray);
	b->SetFillStyle(3001);
	b->Draw();
	
	StandardLabels(axes);

	c1->SaveAs("latency_vs_time.png");
	c1->SaveAs("latency_vs_time.pdf");
}


