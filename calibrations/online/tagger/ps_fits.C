using namespace RooFit;
TH1* GetHistogram(TFile *f, TString hname, TString htype, int counter) {
    stringstream ss; ss << counter;
    TH2I *h2 = (TH2I*)f->Get("PS_timing/"+hname);
    if (htype != "PSC" && htype != "PS" && htype != "TDCADC") {
        cerr << "Unsupported histogram type: Choose 'PS', 'PSC', or 'TDCADC'" << endl;
    }
    TH1 *h = h2->ProjectionY(TString(h2->GetName())+"_"+TString(ss.str()),counter,counter);
    TString title = (htype == "TDCADC") ? "PSC counter "+TString(ss.str()) : htype+" counter "+TString(ss.str());
    h->SetTitle(title);
    h->GetXaxis()->SetTitle(h2->GetYaxis()->GetTitle());
    return h;
}
TH1* GetTaggerHistogram(TFile *f) {
    TH2I *h2 = (TH2I*)f->Get("PS_timing/TAGHRF_tdcTimeDiffVsID");
    const int N = 127; // Use upstream counters (relative to microscope)
    TH1 *h = h2->ProjectionY(TString(h2->GetName()),1,N+1);
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
void WriteFitResults(ofstream &fout, TH1 *h, TString htype, int counter) {
    TString sep = ",";
    double max = h->GetBinContent(h->GetMaximumBin());
    double mode = GetMode(h);
    if (h->GetEntries() < 100.0 || mode == 999.0) {
        fout << counter << sep << max << sep << 0.0 << sep << 0.0 << sep << 0.0 << endl;
        return;
    }
    double w = 0.75;
    double xlow = mode - w; double xhigh = mode + w;
    if (htype == "TDCADC") {
        xlow = h->GetBinCenter(h->FindFirstBinAbove(0.5*max)) - w;
        xhigh = h->GetBinCenter(h->FindLastBinAbove(0.5*max)) + w;
    }
    RooRealVar x("TDC time difference","TDC time difference [ns]",xlow,xhigh);
    RooDataHist data("data","data",RooArgList(x),h);
    // model: gaussian
    RooRealVar mean("mean","mean",mode,xlow,xhigh);
    RooRealVar sigma("sigma","sigma",0.2,0.01,0.5);
    RooGaussian gauss("gauss","gauss",x,mean,sigma);
    gauss.fitTo(data,PrintLevel(-1));
    // Make plot
    TCanvas *canvas = new TCanvas("c","c",800,500);
    RooPlot *plot = x.frame();
    plot->SetTitle(h->GetTitle());
    plot->SetXTitle(h->GetXaxis()->GetTitle());
    TString ytitle = (htype == "PS") ? "PS hits" : "PSC hits";
    plot->SetYTitle(ytitle);
    plot->SetTitleSize(0.0474,"XY");
    data.plotOn(plot);
    gauss.plotOn(plot,LineColor(kRed));
    plot->Draw();
    gauss.paramOn(plot,Layout(0.58,0.9,0.9),Format("NEU",AutoPrecision(1)));
    plot->Draw();
    stringstream ss; ss << counter;
    system(TString("mkdir -p fits_"+htype).Data());
    canvas->Print("fits_"+htype+"/counter_"+TString(ss.str())+".gif");
    delete canvas; delete plot;
    fout << counter << sep << max << sep << mean.getVal() << sep << mean.getError() << sep << sigma.getVal() << endl;
}
void WriteFitResults2(ofstream &fout, TH1 *h, TString htype, int counter) {
    TString sep = ",";
    double max = h->GetBinContent(h->GetMaximumBin());
    double mode = GetMode(h);
    if (h->GetEntries() < 100.0 || mode == 999.0) {
        if (counter > 0) fout << counter << sep << max << sep << 0.0 << sep << 0.0 << sep << 0.0 << endl;
        return;
    }
    double w = (htype == "TAGH") ? 1.0 : 0.75;
    double xlow = mode - w;
    double xhigh = mode + w;
    RooRealVar x("TDC time difference","TDC time difference [ns]",xlow,xhigh);
    RooDataHist data("data","data",RooArgList(x),h);
    // model: add a narrow and wide gaussian with same mean
    RooRealVar mean("mean","mean",mode,xlow,xhigh);
    RooRealVar sigma1("sigma1","sigma1",0.2,0.01,0.4);//0.01,0.6
    RooGaussian gauss1("gauss1","gauss1",x,mean,sigma1);
    RooRealVar sigma2("sigma2","sigma2",0.7,0.3,2.5);//0.1,2.5
    RooGaussian gauss2("gauss2","gauss2",x,mean,sigma2);
    // f1: fraction of entries in first gaussian
    RooRealVar f1("f1","f1",0.75,0.01,1.);
    RooAddPdf doubleGauss("doubleGauss","doubleGauss",RooArgList(gauss1,gauss2),RooArgList(f1));
    doubleGauss.fitTo(data,PrintLevel(-1));
    // Make plot
    TCanvas *canvas = new TCanvas("c","c",800,500);
    RooPlot *plot = x.frame();
    plot->SetTitle(h->GetTitle());
    plot->SetXTitle(h->GetXaxis()->GetTitle());
    TString ytitle = (htype == "PS") ? "PS hits" : "PSC hits";
    if (htype == "TAGH") ytitle = "TAGH hits";
    plot->SetYTitle(ytitle);
    plot->SetTitleSize(0.0474,"XY");
    data.plotOn(plot);
    doubleGauss.plotOn(plot,LineColor(kRed));
    plot->Draw();
    doubleGauss.plotOn(plot,Components("gauss1"),LineColor(kBlue),LineStyle(kDashed));
    doubleGauss.plotOn(plot,Components("gauss2"),LineColor(kGreen),LineStyle(kDashed));
    doubleGauss.paramOn(plot,Layout(0.15,0.45,0.9),Format("NEU",AutoPrecision(1)));
    plot->Draw();
    stringstream ss; ss << counter;
    system(TString("mkdir -p fits_"+htype).Data());
    canvas->Print("fits_"+htype+"/counter_"+TString(ss.str())+".gif");
    delete canvas; delete plot;
    if (counter > 0) fout << counter << sep << max << sep << mean.getVal() << sep << mean.getError() << sep << sigma1.getVal() << endl;
}
int ps_fits(TString rootFile, bool doAllFits) {
    TFile *f = new TFile(rootFile,"read");
    TString hnames[] = {"PSC_tdcadcTimeDiffVsID","PSCRF_tdcTimeDiffVsID","PSRF_adcTimeDiffVsID"};
    TString htypes[] = {"TDCADC","PSC","PS"};
    int N_htypes = 2;
    if (doAllFits) N_htypes = 3;
    for (int htype = 0; htype < N_htypes; htype++) {
        system("mkdir -p fits-csv");
        ofstream fout; fout.open("fits-csv/results_"+htypes[htype]+".txt");
        const int N = (htypes[htype] == "PS") ? 290 : 16;
        for (int i = 1; i <= N; i++) { // counters
            TH1 *h = GetHistogram(f,hnames[htype],htypes[htype],i);
            if (htypes[htype] != "PSC") WriteFitResults(fout,h,htypes[htype],i);
            else WriteFitResults2(fout,h,htypes[htype],i);
        }
        fout.close();
    }
    ofstream fout; fout.open("fits-csv/results_base.txt");
    TH1 *h = GetTaggerHistogram(f);
    WriteFitResults2(fout,h,"TAGH",1);
    fout.close();
    return 0;
}
