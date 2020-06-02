
#include "StandardLabels.C"

void cpu_vs_time(const char *start_tstr, int RunMin=0, int RunMax=999999, const char *site="")
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

	
	char run_cut_str[256];
	sprintf(run_cut_str, "(run>=%d) && (run<=%d)", RunMin, RunMax);

	TTree *t = new TTree("slurminfo", "SLURM Info.");
	t->ReadFile("slurm.csv", "tsubmit/F:tstart:tend:cpu:latency:ncpus:run/I:file");

	TCanvas *c1 = new TCanvas("c1", "", 1600,600);
	c1->SetGrid();
	c1->SetTicks();
	gPad->SetBottomMargin(0.3);
	gPad->SetRightMargin(0.02);

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

	double ymax = 10.0; // hrs
	string title = string(site) + " Wall time vs. Start Time (completed jobs only);;Wall time per job (hrs)";
	TH2D *axes = new TH2D("axes", title.c_str(), 100, 0.0, tmax, 100, 0.0, ymax);
	axes->SetStats(0);
	axes->GetYaxis()->SetTitleSize(0.05);
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

	t->SetMarkerStyle(20);
	t->SetMarkerColor(kMagenta+3);
	t->Draw("cpu/ncpus/3600.0:tstart", run_cut_str, "same");

	// Draw box shading unreliable region
	TBox *b = new TBox(tmax - 4.5*3600.0,0.0, tmax, axes->GetYaxis()->GetXmax());
	b->SetFillColor(kGray);
	b->SetFillStyle(3001);
	b->Draw();
	
	StandardLabels(axes);

	c1->SaveAs("cpu_vs_time.png");
	c1->SaveAs("cpu_vs_time.pdf");
}


