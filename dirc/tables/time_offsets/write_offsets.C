
void write_offsets(int run = 70080, TString box = "SouthLower") {

	//TFile *f = TFile::Open(Form("/volatile/halld/home/jrsteven/RunPeriod-2019-01/dirc_monitoring/ver07/merged/hd_root_0%d.root", run));
	TFile *f = TFile::Open(Form("/work/halld2/home/jrsteven/2018-dirc/hd_root_%d.root", run));
	TH2I *h2 = (TH2I*)f->Get(Form("DIRC_online/Hit/%sBox/Hit_LEDTimeDiffVsChannel", box.Data()));
	h2->Draw("colz");

	TH1I *hFiber1TimeMean = new TH1I("hFiber1TimeMean", "Fiber 1 mean time; time (ns)", 200, -10, 30);
	TH1I *hFiber2TimeMean = new TH1I("hFiber2TimeMean", "Fiber 2 mean time; time (ns)", 200, -10, 30);
	TH1I *hFiber3TimeMean = new TH1I("hFiber3TimeMean", "Fiber 3 mean time; time (ns)", 200, -10, 30);
	TH1I *hFiber1TimeSigma = new TH1I("hFiber1TimeSigma", "Fiber 1 time sigma; time (ns)", 100, 0, 3);
        TH1I *hFiber2TimeSigma = new TH1I("hFiber2TimeSigma", "Fiber 2 time sigma; time (ns)", 100, 0, 3);
        TH1I *hFiber3TimeSigma = new TH1I("hFiber3TimeSigma", "Fiber 3 time sigma; time (ns)", 100, 0, 3);

	ofstream offsetfile;
        offsetfile.open("time_offsets.txt");

	// loop over channels and fit with gaussian to get time offsets and widths
	for(uint i=0; i<h2->GetNbinsX(); i++) {
		TH1F *h = (TH1F*)h2->ProjectionY(Form("channel_%d",i),i+1,i+1);
		if(h->Integral() < 1) {
			offsetfile<<0<<endl;
			continue;
		}
		h->Draw();

		TF1 *f = new TF1("f1", "gaus", -10, 30);

		int pmtid = i/64;
		int pmtrow = pmtid%18;
		if(pmtrow < 6) {
			h->Fit(f, "Q", "", -8, 8);
	                double mean = f->GetParameter(1);
        	        double sigma = f->GetParameter(2);
			hFiber1TimeMean->Fill(mean);
			hFiber1TimeSigma->Fill(sigma);
			offsetfile<<mean<<endl;
		}
		else if(pmtrow < 12) {
			h->Fit(f, "Q", "", 2, 18);
        	        double mean = f->GetParameter(1);
	                double sigma = f->GetParameter(2);
			hFiber2TimeMean->Fill(mean);
			hFiber2TimeSigma->Fill(sigma);
			offsetfile<<mean-10.<<endl;
		}
		else {
			h->Fit(f, "Q", "", 12, 28);
	                double mean = f->GetParameter(1);
        	        double sigma = f->GetParameter(2);
			hFiber3TimeMean->Fill(mean);
			hFiber3TimeSigma->Fill(sigma);
			offsetfile<<mean-20.<<endl;
		}
	}	

	TCanvas *cc = new TCanvas("cc", "cc", 900, 300);
	cc->Divide(3,1);
	cc->cd(1); hFiber1TimeMean->Draw();
	cc->cd(2); hFiber2TimeMean->Draw();
	cc->cd(3); hFiber3TimeMean->Draw();

	TCanvas *dd = new TCanvas("dd", "dd", 900, 300);
        dd->Divide(3,1);
        dd->cd(1); hFiber1TimeSigma->Draw();
        dd->cd(2); hFiber2TimeSigma->Draw();
        dd->cd(3); hFiber3TimeSigma->Draw();

	offsetfile.close();

	return;
}
