using namespace RooFit;
TH1* GetHistogram(TFile *f, int counter, int ph_bin) {
    stringstream ss_c; ss_c << counter;
    TH2I *h2 = (TH2I*)f->Get("TAGH_timewalk/Timewalk/TAGHRF_tdcTimeDiffVsPulseHeight_"+TString(ss_c.str()));
    stringstream ss_ph; ss_ph << ph_bin;
    TH1 *h;
    if (counter == 0 && ph_bin == 0) {
        const int N = 40;
        h = h2->ProjectionY(TString(h2->GetName())+"_"+TString(ss_ph.str()),1,N+1);
        h->SetTitle("TAGH TDC timing - relative to RF, all counters");
    }
    else {
        h = h2->ProjectionY(TString(h2->GetName())+"_"+TString(ss_ph.str()),ph_bin,ph_bin);
        h->SetTitle("TAGH counter "+TString(ss_c.str())+", Pulse-height bin "+TString(ss_ph.str()));
    }
    h->GetXaxis()->SetTitle("time(TDC) - time(RF) [ns]");
    return h;
}
double GetMode(TH1 *h) {
    if (h->GetEntries() == 0.0) return 999.0;
    int max_bin = h->GetMaximumBin();
    double max = h->GetBinContent(max_bin);
    if (max < 7.0) return 999.0;
    return h->GetBinCenter(max_bin);
}
void WriteGaussianFitResults(ofstream &fout, TH1 *h, int counter, int ph_bin) {
    TString sep = ",";
    double mode = GetMode(h);
    if (h->GetEntries() < 100.0 || mode == 999.0) {
        if (counter > 0) fout << counter << sep << ph_bin << sep << h->GetEntries() << sep << 0.0 << sep << 0.0 << sep << 0.0 << endl;
        return;
    }
    double xlow = mode - 1.0;
    double xhigh = mode + 1.0;
    RooRealVar x("TDC time difference","TDC time difference [ns]",xlow,xhigh);
    RooDataHist data("data","data",RooArgList(x),h);
    // model: add a narrow and wide gaussian with same mean
    RooRealVar mean("mean","mean",mode,xlow,xhigh);
    RooRealVar sigma1("sigma1","sigma1",0.2,0.01,0.4);//0.01,0.6
    RooGaussian gauss1("gauss1","gauss1",x,mean,sigma1);
    RooRealVar sigma2("sigma2","sigma2",0.7,0.3,3.0);//0.3,2.5
    RooGaussian gauss2("gauss2","gauss2",x,mean,sigma2);
    // f1: fraction of entries in first gaussian
    RooRealVar f1("f1","f1",0.75,0.01,1.);
    RooAddPdf doubleGauss("doubleGauss","doubleGauss",RooArgList(gauss1,gauss2),RooArgList(f1));
    doubleGauss.fitTo(data,PrintLevel(-1));
    // Make plot
    TCanvas *canvas = new TCanvas("c","c",800,500);
    canvas->SetBatch(kTRUE);
    RooPlot *plot = x.frame();
    plot->SetTitle(h->GetTitle());
    plot->SetXTitle(h->GetXaxis()->GetTitle());
    plot->SetYTitle("TAGH hits");
    plot->SetTitleOffset(1.1,"Y");
    data.plotOn(plot);
    doubleGauss.plotOn(plot,LineColor(kRed));
    plot->Draw();
    doubleGauss.plotOn(plot,Components("gauss1"),LineColor(kBlue),LineStyle(kDashed));
    doubleGauss.plotOn(plot,Components("gauss2"),LineColor(kGreen),LineStyle(kDashed));
    if (counter == 0)
        doubleGauss.paramOn(plot,Layout(0.56,0.9,0.9),Format("NEU",AutoPrecision(1)));
    else
        doubleGauss.paramOn(plot,Layout(0.15,0.45,0.9),Format("NEU",AutoPrecision(1)));
    plot->Draw();
    if (counter == 0) {
        canvas->Print("overall_gaussian_fit.gif");
    }
    else {
        stringstream ss; ss << counter;
        system(TString("mkdir -p fits_gaussian/counter_"+ss.str()).Data());
        canvas->Print("fits_gaussian/counter_"+TString(ss.str())+"/"+TString(h->GetName())+".gif");
    }
    delete canvas; delete plot;
    if (counter > 0) fout << counter << sep << ph_bin << sep << h->GetEntries() << sep << mean.getVal() << sep << mean.getError() << sep << sigma1.getVal() << endl;
}
int gaussian_fits(TString rootFile, bool doAllFits) {
    TFile *f = new TFile(rootFile,"read");
    if (doAllFits) {
        system("mkdir -p gaussian-fits-csv");
        for (int i = 1; i <= 274; i++) { // counters
            stringstream ss; ss << i;
            ofstream fout; fout.open("gaussian-fits-csv/counter_"+TString(ss.str())+".txt");
            for (int j = 1; j <= 41; j++) { // pulse-height bins
                TH1 *h = GetHistogram(f,i,j);
                WriteGaussianFitResults(fout,h,i,j);
            }
            fout.close();
        }
    }
    // Do overall fit
    ofstream fout; TH1 *h_overall = GetHistogram(f,0,0); WriteGaussianFitResults(fout,h_overall,0,0);
    return 0;
}
