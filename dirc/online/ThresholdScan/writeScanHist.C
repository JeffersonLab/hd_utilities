// Create histogram file from threshold scan TTree

void writeScanHist(int run) {

	TFile *f = TFile::Open(Form("dirc_scan_%d.root",run));
	TTree *T = (TTree*)f->Get("T");
	T->Process("thresholdScan.C");

	return;
}
