void GetData(TString fname, double *y, double &mean, const int N) {
    ifstream fin(fname);
    double prod = 0.0; double sum = 0.0;
    for (int i = 0; i < N; i++) {
        char sep; int n; double w, t, dt, sigma;
        fin >> n >> sep >> w >> sep >> t >> sep >> dt >> sep >> sigma;
        y[i] = t;
        prod += w*t;
        sum += w;
    }
    if (sum > 0.0) {
        mean = prod/sum;
    }
    for (int i = 0; i < N; i++) {if (y[i] == 0.0) y[i] = mean;}
    fin.close();
}
double GetBaseOffset(TString fname) {
    ifstream fin(fname);
    char sep; int n; double w, t, dt, sigma;
    fin >> n >> sep >> w >> sep >> t >> sep >> dt >> sep >> sigma;
    fin.close();
    return t;
}
void GetCCDBOffsetsBase(double &adc_offset_psc, double &tdc_offset_psc, double &adc_offset_ps) {
    ifstream fin("ps_offsets/base_time_offset_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    fin >> adc_offset_psc >> tdc_offset_psc >> adc_offset_ps;
    fin.close();
}
void GetPSCCCDBOffsetsTDC(double* tdc_offsets, const int N) {
    ifstream fin("ps_offsets/tdc_timing_offsets_psc_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < N; i++) {
        fin >> tdc_offsets[i];
    }
    fin.close();
}
void GetPSCCCDBOffsetsADC(double* adc_offsets, const int N) {
    ifstream fin("ps_offsets/adc_timing_offsets_psc_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < N; i++) {
        fin >> adc_offsets[i];
    }
    fin.close();
}
void GetPSCCDBOffsetsADC(double* adc_offsets_l, double* adc_offsets_r, const int N) {
    ifstream fin("ps_offsets/adc_timing_offsets_ps_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (int i = 0; i < N; i++) {
        fin >> adc_offsets_l[i] >> adc_offsets_r[i];
    }
    fin.close();
}
void WriteBaseOffsets(double TAGH_mu, double tdc_mu, double tdcadc_mu, double ps_mu) {
    double adc_ccdb_psc; double tdc_ccdb_psc; double adc_ccdb_ps;
    GetCCDBOffsetsBase(adc_ccdb_psc,tdc_ccdb_psc,adc_ccdb_ps);
    ofstream fout; fout.open("ps_offsets/base_time_offset.txt");
    TString sep = "        ";
    double adc_mu = tdc_mu - tdcadc_mu; // mean PSC ADC time
    fout << adc_ccdb_psc - adc_mu + TAGH_mu << sep << tdc_ccdb_psc - tdc_mu + TAGH_mu << sep << adc_ccdb_ps - ps_mu + TAGH_mu << endl;
    fout.close();
}
void WritePSCTDCOffsets(double *y, double mean, const int N) {
    double y_ccdb[N];
    GetPSCCCDBOffsetsTDC(y_ccdb,N);
    ofstream fout; fout.open("ps_offsets/tdc_timing_offsets_psc.txt");
    TString sep = "        ";
    for (int i = 0; i < N; i++) fout << y_ccdb[i] + y[i] - mean << endl;
    fout.close();
    fout.open("ps_offsets/tdc_timing_offsets_psc_diff.txt");
    for (int i = 0; i < N; i++) fout << y[i] - mean << endl;
    fout.close();
}
void WritePSCADCOffsets(double *y_tdcadc, double mean_tdcadc, double *y, double mean, const int N) {
    double y_ccdb[N];
    GetPSCCCDBOffsetsADC(y_ccdb,N);
    ofstream fout; fout.open("ps_offsets/adc_timing_offsets_psc.txt");
    TString sep = "        ";
    for (int i = 0; i < N; i++) fout << y_ccdb[i] + y[i] - mean - (y_tdcadc[i] - mean_tdcadc) << endl;
    fout.close();
    fout.open("ps_offsets/adc_timing_offsets_psc_diff.txt");
    for (int i = 0; i < N; i++) fout << y[i] - mean - (y_tdcadc[i] - mean_tdcadc) << endl;
    fout.close();
}
void WritePSADCOffsets(double *y, double mean, const int N) {
    double yl_ccdb[int(N/2)]; double yr_ccdb[int(N/2)];
    GetPSCCDBOffsetsADC(yl_ccdb,yr_ccdb,int(N/2));
    ofstream fout; fout.open("ps_offsets/adc_timing_offsets_ps.txt");
    TString sep = "        ";
    for (int i = 0; i < int(N/2); i++) fout << yl_ccdb[i] + y[i] - mean << sep << yr_ccdb[i] + y[i+145] - mean << endl;
    fout.close();
    fout.open("ps_offsets/adc_timing_offsets_ps_diff.txt");
    for (int i = 0; i < int(N/2); i++) fout << y[i] - mean << sep << y[i+145] - mean << endl;
    fout.close();
}
int ps_offsets(TString dir) {
    system("mkdir -p offsets");
    // PSC offsets
    const int N = 16; // counters
    double y[N]; double mean_tdc = 0.0;
    GetData(dir+"/results_PSC.txt",y,mean_tdc,N);
    WritePSCTDCOffsets(y,mean_tdc,N);
    double y_tdcadc[N]; double mean_tdcadc = 0.0;
    GetData(dir+"/results_TDCADC.txt",y_tdcadc,mean_tdcadc,N);
    WritePSCADCOffsets(y_tdcadc,mean_tdcadc,y,mean_tdc,N);
    // PS offsets
    const int N_ps = 290; // counters
    double y_ps[N_ps]; double mean_ps = 0.0;
    GetData(dir+"/results_PS.txt",y_ps,mean_ps,N_ps);
    WritePSADCOffsets(y_ps,mean_ps,N_ps);
    // Base offsets
    double PSCTAGH_offset = GetBaseOffset(dir+"/results_base.txt");
    WriteBaseOffsets(PSCTAGH_offset,mean_tdc,mean_tdcadc,mean_ps);
    return 0;
}
