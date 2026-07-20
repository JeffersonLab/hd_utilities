void GetData(TString fname, double *y, const int N) {
    ifstream fin(fname);
    double prod = 0.0; double sum = 0.0;
    for (int i = 0; i < N; i++) {
        char sep; int n; double w, t, dt, sigma;
        fin >> n >> sep >> w >> sep >> t >> sep >> dt >> sep >> sigma;
        y[i] = t;
        if (sigma < 0.06 && t != 0.0) {
            y[i] = 0.0;
            cout << "Bad fit: counter " << n << endl;
        }
    }
    fin.close();
}
void GetCCDBOffsetsBase(double &adc_offset, double &tdc_offset) {
    ifstream fin("offsets/base_time_offset_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    fin >> adc_offset >> tdc_offset;
    fin.close();
}
void GetCCDBOffsetsTDC(double* tdc_offsets, const int N) {
    ifstream fin("offsets/tdc_time_offsets_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int n;
    for (int i = 0; i < N; i++) {
        fin >> n >> tdc_offsets[i];
    }
    fin.close();
}
void GetCCDBOffsetsADC(double* adc_offsets, const int N) {
    ifstream fin("offsets/fadc_time_offsets_ccdb.txt");
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int n;
    for (int i = 0; i < N; i++) {
        fin >> n >> adc_offsets[i];
    }
    fin.close();
}
void WriteBaseOffsets(double c1_adc, double c1_tdc) {
    double adc_ccdb; double tdc_ccdb;
    GetCCDBOffsetsBase(adc_ccdb,tdc_ccdb);
    ofstream fout; fout.open("offsets/base_time_offset.txt");
    TString sep = "        ";
    fout << adc_ccdb - c1_adc << sep << tdc_ccdb - c1_tdc << endl;
    fout.close();
}
void WriteTAGHTDCOffsets(double *y, const int N, double &c1_tdc) {
    double y_ccdb[N];
    GetCCDBOffsetsTDC(y_ccdb,N);
    ofstream fout; fout.open("offsets/tdc_time_offsets.txt");
    TString sep = "        ";
    for (int i = 0; i < N; i++) {
        double offset = y_ccdb[i] + y[i];
	/*
        if (i == 0) c1_tdc = offset;
        if (y_ccdb[i] == 0.0 && y[i] == 0.0) {
            offset = 0.0;
        } else {
            offset -= c1_tdc;
        }
	*/
        fout << i + 1 << sep << offset << endl;
    }
    fout.close();
    fout.open("offsets/tdc_time_offsets_diff.txt");
    for (int i = 0; i < N; i++) fout << i + 1 << sep << y[i] << endl;
    fout.close();
}
void WriteTAGHADCOffsets(double *y_tdcadc, double *y, const int N, double &c1_adc) {
    double y_ccdb[N];
    GetCCDBOffsetsADC(y_ccdb,N);
    ofstream fout; fout.open("offsets/fadc_time_offsets.txt");
    TString sep = "        ";
    for (int i = 0; i < N; i++) {
        double offset = y_ccdb[i] + y[i] - y_tdcadc[i];
	/*
        if (i == 0) c1_adc = offset;
        if (y_ccdb[i] == 0.0 && y[i] == 0.0) {
            offset = 0.0;
        } else {
            offset -= c1_adc;
        }
	*/
        fout << i + 1 << sep << offset << endl;
    }
    fout.close();
    fout.open("offsets/fadc_time_offsets_diff.txt");
    for (int i = 0; i < N; i++) fout << i + 1 << sep << y[i] - y_tdcadc[i] << endl;
    fout.close();
}
int tagh_offsets(TString dir) {
    system("mkdir -p offsets");
    // TAGH offsets
    const int N = 274; // counters
    double y[N]; double c1_tdc = 0.0; double c1_adc = 0.0;
    GetData(dir+"/results_TAGH.txt",y,N);
    WriteTAGHTDCOffsets(y,N,c1_tdc);
    double y_tdcadc[N];
    GetData(dir+"/results_TDCADC.txt",y_tdcadc,N);
    WriteTAGHADCOffsets(y_tdcadc,y,N,c1_adc);
    WriteBaseOffsets(c1_adc,c1_tdc);
    return 0;
}
