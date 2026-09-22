void GetData(TString fname, double *y, double *dy, double &min, double &max) {
    ifstream fin(fname);
    const int N = 41; // pulse-height bins
    for (int i = 0; i < N; i++) {
        char sep; int n; double stats, t, dt, sigma;
        fin >> n >> sep >> n >> sep >> stats >> sep >> t >> sep >> dt >> sep >> sigma;
        if (dt < 0.01) dt = 0.01;
        y[i] = t; dy[i] = dt;
        if (t > max) max = t;
        if (t < min) min = t;
        if (sigma < 0.06) {y[i] = 0.0; dy[i] = 0.0;} // remove bad Gaussian-fits
        if (i < 5 && t < 0.1) {y[i] = 0.0; dy[i] = 0.0;} // ignore outliers
    }
    fin.close();
}
double func(double *x, double *par) {
    if (x[0] < par[3] && x[0] >= 200.0)
        return par[0]+par[1]/pow(x[0],par[2]);
    else if (x[0] >= par[3])
        return par[0]+par[1]*(1.0+par[2])/pow(par[3],par[2])-x[0]*par[1]*par[2]/pow(par[3],par[2]+1.0);
    else return 999.0;
}
void WriteNA(ofstream &fout, ofstream &fout_ccdb, int counter) {
    double na = 0.0; TString sep = ",";
    fout << counter << sep << na << sep << na << sep << na;
    fout << sep << na << sep << na << sep << na;
    fout << sep << na << sep << na << sep << na << sep << na << endl; sep = "        ";
    fout_ccdb << counter << sep << 0.0 << sep << 0.0 << sep << 0.0 << sep << 0.0 << endl;
}
void WriteTimewalkFitResults(ofstream &fout, ofstream &fout_ccdb, int counter, double *y, double *dy, double min, double max) {
    if (min == 0.0 && max == 0.0) {WriteNA(fout,fout_ccdb,counter); return;}
    TString sep = ",";
    stringstream ss; ss << counter;
    TCanvas c("c","c",800,500);
    const int N = 41; // pulse-height bins
    double x[N]; double dx[N];
    for (int i = 0; i < N; i++) {
        x[i] = 50.0 + i*100.0;
        dx[i] = 0.0;
    }
    TGraphErrors gr(N,x,y,dx,dy);
    TF1 f("f",func,200.0,4000.0,4);
    f.SetParameter(0,min); f.SetParLimits(0,min-10.0,0.0);
    f.SetParameter(1,25.0); f.SetParLimits(1,1.0,90.0);
    f.SetParameter(2,0.5); f.SetParLimits(2,0.02,0.85);
    f.SetParameter(3,2250.0); f.SetParLimits(3,1400.0,4000.0);
    gr.SetTitle("TAGH counter "+TString(ss.str()));
    gr.GetXaxis()->SetTitle("pulse-height [fADC counts]");
    gr.GetYaxis()->SetTitle("Gaussian mean of time(TDC) - time(RF) [ns]");
    gr.Fit("f","eir");
    gr.GetYaxis()->SetRangeUser(min-0.2,max+0.2);
    gr.Draw("AP");
    //if (f.GetNDF() == 0 || f.GetChisquare()/f.GetNDF() > 15.0) {
    if (f.GetNDF() == 0 || f.GetChisquare()/f.GetNDF() > 30.0) {
        WriteNA(fout,fout_ccdb,counter);
        return;
    }
    system("mkdir -p fits_timewalk");
    c.Print("fits_timewalk/counter_"+TString(ss.str())+".gif");
    c.Clear();
    fout << counter << sep << f.GetNDF() << sep << f.GetChisquare() << sep << f.GetParameter(0);
    fout << sep << f.GetParameter(1) << sep <<  f.GetParameter(2) << sep << f.GetParameter(3);
    fout << sep << f.GetParError(0) << sep << f.GetParError(1) << sep << f.GetParError(2) << sep << f.GetParError(3) << endl; sep = "        ";
    fout_ccdb << counter << sep << f.GetParameter(0) << sep << f.GetParameter(1) << sep << f.GetParameter(2) << sep << f.GetParameter(3) << endl;
}
void PrintHistogram(TH1D h) {
    gStyle->SetOptStat("eimr");
    TCanvas c("c","c",800,500);
    h.Draw();
    system("mkdir -p parms_timewalk");
    c.Print("parms_timewalk/"+TString(h.GetName())+".gif");
}
void PrintHistograms(TString fname) {
    TH1D *h[4]; TH1D *he[4];
    double min[] = {-8.0,1.0,0.0,1400.0}; double max[] = {2.0,91.0,1.0,4000.0};
    double emin[] = {0.0,0.0,0.0,0.0}; double emax[] = {0.5,10.0,0.05,100.0};
    for (int i = 0; i < 4; i++) {
        stringstream ss; ss << i; TString name = "c" + TString(ss.str());
        h[i] = new TH1D(name,"TAGH timewalk: "+name+";"+name+";counters",100,min[i],max[i]);
        name = "e_c" + TString(ss.str());
        he[i] = new TH1D(name,"TAGH timewalk: "+name+";"+name+";counters"+";"+name+";counters",100,emin[i],emax[i]);
    }
    TH1D hChisq("Chisq","TAGH timewalk: chi-square / Ndf;chi-square / Ndf;counters",150,0.0,15.0);
    ifstream fin(fname);
    for (int i = 0; i < 274; i++) { // counters
        char sep; int n, ndf; double chisq, c[4], dc[4];
        fin >> n >> sep >> ndf >> sep >> chisq;
        for (int j = 0; j < 4; j++) fin >> sep >> c[j];
        for (int j = 0; j < 4; j++) fin >> sep >> dc[j];
        if (chisq == 0.0) continue;
        for (int j = 0; j < 4; j++) h[j]->Fill(c[j]);
        for (int j = 0; j < 4; j++) he[j]->Fill(dc[j]);
        hChisq.Fill(chisq/ndf);
    }
    fin.close();
    for (int i = 0; i < 4; i++) {
        PrintHistogram(*h[i]); PrintHistogram(*he[i]);
        delete h[i]; delete he[i];
    }
    PrintHistogram(hChisq);
}
int timewalk_fits(TString dir) {
    gStyle->SetOptFit(111);
    TString fname = "timewalk-fits.txt";
    ofstream fout; fout.open(fname);
    ofstream fout_ccdb; fout_ccdb.open("tdc_timewalk.txt");
    for (int i = 1; i <= 274; i++) { // counters
        stringstream ss; ss << i;
        const int N = 41; // pulse-height bins
        double y[N]; double dy[N];
        double max = -10.0; double min = 10.0;
        GetData(dir+"/counter_"+TString(ss.str())+".txt",y,dy,min,max);
        WriteTimewalkFitResults(fout,fout_ccdb,i,y,dy,min,max);
    }
    fout.close(); fout_ccdb.close();
    PrintHistograms(fname);
    return 0;
}
