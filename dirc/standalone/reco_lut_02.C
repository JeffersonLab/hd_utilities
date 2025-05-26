#define glx__sim

#include <iostream>
#include <fstream>
using namespace std;

#include "DrcHit.h"
#include "DrcEvent.h"
#include "DrcLutNode.h"
#include "glxtools.C"
#include <TVirtualFitter.h>
#include <TArc.h>
#include <TRotation.h>
#include <TFitResult.h>
#include <TCut.h>

TGraph gg_gr;
void circleFcn(int &, double *, double &f, double *par, int) {
  f = 0;
  double *x = gg_gr.GetX();
  double *y = gg_gr.GetY();
  int np = gg_gr.GetN();
  for (int i = 0; i < np; i++) {
    double u = x[i] - par[0];
    double v = y[i] - par[1];
    double d = fabs(par[2] - TMath::Sqrt(u * u + v * v));
    double e = sqrt(d);
    f += d;
  }
  // f *= f;
}

double mangle(int pid, double m) {
  return acos(sqrt(m * m + glx_mass[pid] * glx_mass[pid]) / m / 1.4738); // 1.4738
}

void FitRing(double &x0, double &y0, double &theta, TGraph gr) {

  double *x = gr.GetX();
  double *y = gr.GetY();
  gg_gr = TGraph();
  for (int i = 0; i < gr.GetN(); i++) {
    if (fabs(theta - TMath::Sqrt(x[i] * x[i] + y[i] * y[i])) < 0.007) {
      gg_gr.SetPoint(gg_gr.GetN(), x[i], y[i]);
    }
  }

  // Fit a circle to the graph points
  TVirtualFitter::SetDefaultFitter("Minuit"); // default is Minuit
  TVirtualFitter *fitter = TVirtualFitter::Fitter(0, 3);
  fitter->SetPrecision(0.00000001);
  fitter->SetMaxIterations(1000);

  fitter->SetFCN(circleFcn);
  fitter->SetParameter(0, "x0", 0, 0.001, -0.15, 0.15);
  fitter->SetParameter(1, "y0", 0, 0.001, -0.15, 0.15);
  fitter->SetParameter(2, "R", theta, 0.001, theta - 0.04, theta + 0.04);

  fitter->FixParameter(2);
  double arglist[1] = {0};
  fitter->ExecuteCommand("MINIMIZE", arglist, 0);

  x0 = fitter->GetParameter(0);
  y0 = fitter->GetParameter(1);
  theta = fitter->GetParameter(2);
}

// 
void reco_lut_02(TString infile = "pip_p3_theta4.root", TString inlut = "/work/halld/home/gxproj7/RunPeriod-2019-11/dircsim-2019_11-ver03/lut/lut_all_avr.root",
                 int ibar = 7, int ibin = -1, double moms = 3.0, double scan = 0, double dx = 0.0,
                 double dy = 0, int vcorr = 0) {

  int indd = -1;
  const int nodes = 5184; //glx_maxch;
  int glx_nch = 5184;
  const int luts = 24;

  gSystem->Load("DrcEvent_cc.so");
  gSystem->Load("DrcHit_cc.so");
  gSystem->Load("DrcLutNode_cc.so");

  bool sim = true; //false;
  TString outfile = infile;
  TSystemDirectory directory(infile, infile);
  TList *files=directory.GetListOfFiles();
  if (infile.Contains("hd_root_gen")) sim = true;
  if (files) {
    infile = infile+"*.root";
    outfile = "hd_root";
  }
  cout<<infile.Data()<<" "<<outfile.Data()<<endl;

  TFile *fLut = new TFile(inlut);
  TTree *tLut = (TTree *)fLut->Get("lut_dirc");
  TClonesArray *cLut[luts];
  for (int l = 0; l < luts; l++) {
    cLut[l] = new TClonesArray("DrcLutNode");
    tLut->SetBranchAddress(Form("LUT_%d", l), &cLut[l]);
  }
  tLut->GetEntry(0);
  
  DrcLutNode *lutNode[luts][nodes];
  for (int l = 0; l < luts; l++) {
	  for (int i = 0; i < nodes; i++) {
		  if(l<24) lutNode[l][i] = (DrcLutNode *)cLut[l]->At(i);
		  else lutNode[l][i] = (DrcLutNode *)cLut[l]->At(i+nodes);
	  }
  }
  TGaxis::SetMaxDigits(4);
  //*/
  TVector3 fnX1 = TVector3(1, 0, 0);
  TVector3 fnY1 = TVector3(0, 1, 0);
  TVector3 fnZ1 = TVector3(0, 0, 1);
  TVector3 cz;
  const int nbins = 10;

  vector<vector<vector<TH1F *>>> hCorrAD(48,
                                         vector<vector<TH1F *>>(nbins, vector<TH1F *>(glx_npmt)));
  vector<vector<vector<TH1F *>>> hCorrAR(48,
                                         vector<vector<TH1F *>>(nbins, vector<TH1F *>(glx_npmt)));
  vector<vector<vector<TH1F *>>> hCorrTD(48,
                                         vector<vector<TH1F *>>(nbins, vector<TH1F *>(glx_npmt)));
  vector<vector<vector<TH1F *>>> hCorrTR(48,
                                         vector<vector<TH1F *>>(nbins, vector<TH1F *>(glx_npmt)));

  TString stdiff = ";t_{measured}-t_{calculated} [ns];entries [#]";
  TString scdiff = ";#theta_{C reco} - #theta_{C expected} [mrad];entries [#]";

  for (int b = 0; b < 48; b++) {
    for (int x = 0; x < nbins; x++) {
      for (int p = 0; p < glx_npmt; p++) {
        hCorrAD[b][x][p] = new TH1F(Form("hCorrAD_%d_%d_%d", b, x, p), scdiff, 60, -50, 50);
        hCorrAR[b][x][p] = new TH1F(Form("hCorrAR_%d_%d_%d", b, x, p), scdiff, 60, -50, 50);
        hCorrTD[b][x][p] = new TH1F(Form("hCorrTD_%d_%d_%d", b, x, p), stdiff, 100, -5, 5);
        hCorrTR[b][x][p] = new TH1F(Form("hCorrTR_%d_%d_%d", b, x, p), stdiff, 100, -5, 5);
      }
    }
  }

  const int nphi = 80, ntheta = 40;
  vector<vector<TH1F *>> hCorrLut(nphi, vector<TH1F *>(ntheta));
  for (int b = 0; b < nphi; b++) {
    for (int p = 0; p < ntheta; p++) {
      hCorrLut[b][p] = new TH1F(Form("hCorrLut_p%d_t%d", b, p), stdiff, 100, -20, 20);
    }
  }

  double radiatorL = 489.712;    // 4*122.5;
  double barend = -294.022 + dx; // 4*1225-1960; -294.022

  double minChangle = 0.6;
  double maxChangle = 0.9;
  double sum1, sum2, noise = 0.2; // 0.4
  // cuts
  int anglecorr = (vcorr); // 2-apply
  bool bfitcorr = (0);
  double cut_cangle = 3.5 * 0.008; // 3.5
  double cut_tdiff = 0.5;

  /*
  // Roman's per-bar corrections (not sure of source)
  double bar_corr_x[] = {
    0.000,  -0.002, -0.004, -0.004, -0.004, -0.003, -0.003, -0.003, -0.004, -0.004, -0.003, -0.004,
    -0.004, -0.005, -0.005, -0.004, -0.003, -0.004, -0.003, -0.004, -0.005, -0.005, -0.005, -0.005,

    -0.009, -0.002, -0.003, -0.002, -0.003, -0.002, -0.001, -0.001, 0.000,  0.000,  0.000,  0.000,
    0.000,  -0.001, -0.001, 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000};
  double bar_corr_y[] = {
    0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002,
    0.003, 0.003, 0.003, 0.003, 0.003, 0.003, 0.003, 0.003, 0.003, 0.003, 0.003, 0.003,

    0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002,
    0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002};
  */

  // bar box rotations from CCDB table DIRC/bar_rotation 
  double bar_corr_x[4] = {-0.2134, -0.0791, -0.15384, -0.18363};
  double bar_corr_y[4] = {0.0963, 0.1003, 0.08365, 0.09568};
  double bar_corr_z[4] = {-0.0355, -0.0381, 0.00773, 0.01461};
  double sigma[] = {0.01, 0.01, 0.0073, 0.0073, 0.01};

  double acorrAD[48][nbins][glx_npmt] = {{{0}}};
  double acorrAR[48][nbins][glx_npmt] = {{{0}}};
  double acorrTD[48][nbins][glx_npmt] = {{{0}}};
  double acorrTR[48][nbins][glx_npmt] = {{{0}}};
  double asigmaAD[48][nbins][glx_npmt] = {{{0}}};
  double asigmaAR[48][nbins][glx_npmt] = {{{0}}};
  double asigmaTD[48][nbins][glx_npmt] = {{{0}}};
  double asigmaTR[48][nbins][glx_npmt] = {{{0}}};
  double acorr3AD[48][nphi][ntheta] = {{{0}}};
  double acorr3AR[48][nphi][ntheta] = {{{0}}};
  double corrAD, corrAR, sigmaAD, corrTD, corrTR, sigmaTD, sigmaTR, fracAR, fracAD;
  int cor_level = 0, tb, tp, tt, tbin, level;

  TString corrfile = infile + ".corr.root";
  if (ibar > -1 && ibin > -1) corrfile = infile + Form(".corr_%02d_%02d.root", ibar, ibin);

  if (anglecorr == 2) {
    // read per pmt corrections
    if (!gSystem->AccessPathName(corrfile)) {
      std::cout << "--- reading corrections from " << corrfile << std::endl;
      TChain ch;
      ch.SetName("corr");
      ch.Add(corrfile);
      ch.SetBranchAddress("bar", &tb);
      ch.SetBranchAddress("pmt", &tp);
      ch.SetBranchAddress("bin", &tbin);
      ch.SetBranchAddress("level", &level);
      ch.SetBranchAddress("zcorrAD", &corrAD);
      ch.SetBranchAddress("zcorrAR", &corrAR);
      ch.SetBranchAddress("zcorrTD", &corrTD);
      ch.SetBranchAddress("zcorrTR", &corrTR);
      ch.SetBranchAddress("zsigmaTD", &sigmaTD);
      ch.SetBranchAddress("zsigmaTR", &sigmaTR);
      ch.SetBranchAddress("zsigmaAD", &sigmaAD);
      ch.SetBranchAddress("zfracAD", &fracAD);
      ch.SetBranchAddress("zfracAR", &fracAR);

      for (int i = 0; i < ch.GetEntries(); i++) {
        ch.GetEvent(i);
        cor_level = level;
	asigmaTD[tb][tbin][tp] = sigmaTD;
	asigmaTR[tb][tbin][tp] = sigmaTR;
	if (sigmaTD < 2.0 && sigmaTD > 0.25) {
		acorrTD[tb][tbin][tp] = corrTD;
		if (fabs(corrAD) < 9.0 && fracAD > 0.2 && fracAD < 2.0)
			acorrAD[tb][tbin][tp] = 0.001 * corrAD;
	}
	if (sigmaTR < 3.0 && sigmaTR > 0.3) {
		acorrTR[tb][tbin][tp] = corrTR;
		if (fabs(corrAR) < 9.0 && fracAR > 0.2 && fracAR < 2.0) 
			acorrAR[tb][tbin][tp] = 0.001 * corrAR;
	}

        std::cout << "L " << cor_level << " bar = " << tb << " bin = " << tbin << " pmt = " << tp
                  << Form(" ad %-8.5f ar %-8.5f", acorrAD[tb][tbin][tp], acorrAR[tb][tbin][tp])
                  << Form(" td %-8.5f tr %-8.5f", acorrTD[tb][tbin][tp], acorrTR[tb][tbin][tp])
                  << std::endl;
      }
    } else {
      cor_level = 0;
      std::cout << "--- corr file not found  " << corrfile << std::endl;
    }

    if (cor_level == 0) cut_tdiff = 2.0;
    if (cor_level == 1) cut_tdiff = 0.5;
  }

  /*
  // bar box survey rotations in degrees (used in halld_recon reconstruction)
  double bar_corr_z[48] = {0};
  if (true) { //cor_level < 2) {  
	  for(int ibar=0; ibar<48; ibar++) {
		  if(ibar<12) {
			  bar_corr_x[ibar] = -0.2134 * TMath::Pi()/180.;
			  bar_corr_y[ibar] = 0.0963 * TMath::Pi()/180.;
			  bar_corr_z[ibar] = -0.0355 * TMath::Pi()/180.;
		  }
		  else if(ibar<24) {
			  bar_corr_x[ibar] = -0.0791 * TMath::Pi()/180.;
			  bar_corr_y[ibar] = 0.1003 * TMath::Pi()/180.;
			  bar_corr_z[ibar] = -0.0381 * TMath::Pi()/180.;	
		  }
		  else if(ibar<36) {
			  bar_corr_x[ibar] = -0.15384 * TMath::Pi()/180.;
			  bar_corr_y[ibar] = 0.08365 * TMath::Pi()/180.;
			  bar_corr_z[ibar] = 0.00773 * TMath::Pi()/180.;		
		  }
		  else {
			  bar_corr_x[ibar] = -0.18363 * TMath::Pi()/180.;
			  bar_corr_y[ibar] = 0.09568 * TMath::Pi()/180.;
			  bar_corr_z[ibar] = 0.01461 * TMath::Pi()/180.;		
		  }
	  }
  }
  */

  if (anglecorr == 3) {
    corrfile = outfile + ".corr3.root";
    std::cout << "======= reading corrections from " << corrfile << std::endl;
    TChain ch;
    ch.SetName("corr");
    ch.Add(corrfile);
    ch.SetBranchAddress("bar", &tb);
    ch.SetBranchAddress("itheta", &tt);
    ch.SetBranchAddress("iphi", &tp);
    ch.SetBranchAddress("zcorrAD", &corrAD);
    ch.SetBranchAddress("zcorrAR", &corrAR);
    ch.SetBranchAddress("zsigmaAD", &sigmaAD);

    for (int i = 0; i < ch.GetEntries(); i++) {
      ch.GetEvent(i);
      if (fabs(corrAD) < 6) {
        acorr3AD[tb][tp][tt] = 0.001 * corrAD;
        acorr3AR[tb][tp][tt] = 0.001 * corrAR;
      }
      std::cout << "i " << i << " " << tb << "-" << tp << " " << tt << " " << acorr3AD[tb][tp][tt]
                << std::endl;
    }
  }

  double criticalAngle = asin(1.00028 / 1.47125); // n_quarzt = 1.47125; //(1.47125 <==> 390nm)
  double evtime, luttheta, tangle, lenx;
  int64_t pathid;
  TVector3 posInBar, posInBar_true, momInBar, dir, dird, ldir;
  double cherenkovreco[5], cherenkovreco_err[5], spr[5];

  TF1 *fit = new TF1("fgaus", "[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2]) +x*[3]+[4]", minChangle, maxChangle);
  TF1 *gaus = new TF1("gaus_for_integral", "[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2])");
  TSpectrum *spect = new TSpectrum(10);
  TH1F *hAngle[5], *hAngleDiff[5], *hLnDiff[5], *hNph[5];
  TH1F *hAngleU[5];
  TF1 *fAngle[5];
  double mAngle[5];
  TH1F *hDiff = new TH1F("hDiff", stdiff, 400, -10, 10);
  TH1F *hDiffT = new TH1F("hDiffT", stdiff, 400, -10, 10);
  TH1F *hDiffD = new TH1F("hDiffD", stdiff, 400, -10, 10);
  TH1F *hDiffR = new TH1F("hDiffR", stdiff, 400, -10, 10);
  TH1F *hTime = new TH1F("hTime", ";propagation time [ns];entries [#]", 1000, 0, 200);
  TH1F *hCalc = new TH1F("hCalc", ";calculated time [ns];entries [#]", 1000, 0, 200);
  TH1F *hNphC = new TH1F("hNphC", ";detected photons [#];entries [#]", 150, 0, 150);
  TH2F *hCMom[5];
  TH2F *hRing = new TH2F("hRing", ";#theta_{c}sin(#varphi_{c});#theta_{c}cos(#varphi_{c})", 500, -1,
                         1, 500, -1, 1);
  TH2F *h2Time = new TH2F("h2Time", ";propagation time [ns];t_{measured}-t_{calculated} [ns];", 500,
                          0, 100, 200, -5, 5);
  TH2F *hWall = new TH2F("hWall", ";x [cm];y [cm]", 400, -100, 100, 400, -100, 100);
  TH2F *hWallPos = new TH2F("hWalPos", ";x [cm];y [cm]", 50, -100, 100, 50, -100, 100);
  int wallb[5000] = {0};
  TH2F *hSpr = new TH2F("hSpr", ";x [cm];y [cm]", 400, -100, 100, 400, -100, 100);
  TH2F *hChrom = new TH2F("hChrom", ";t_{measured}-t_{calculated} [ns];#theta_{C} [mrad]", 100, -2,
                          2, 60, -30, 30);
  TH1F *hMult = new TH1F("hmult", "; track multiplicity [#];entries [#]", 8, 0, 8);
  TH2F *hLutCorr =
    new TH2F("hLutCorr", "hLutCorr", nphi, 0, TMath::TwoPi(), ntheta, 0, TMath::PiOver2());
  TH2F *hLutCorr1 =
    new TH2F("hLutCorr1", "hLutCorr", nphi, -TMath::Pi(), TMath::Pi(), ntheta, 0, TMath::PiOver2());

  hDiff->SetMinimum(0);
  TGaxis::SetMaxDigits(3);

  for (int i = 0; i < 5; i++) {
    double momentum = 4;
    hAngle[i] =
      new TH1F(Form("hAngle_%d", i), ";#theta_{C} [rad];entries/N_{max} [#]", 150, 0.6, 0.9);
    hAngleDiff[i] =
      new TH1F(Form("hAngleDiff_%d", i), ";#Delta#theta_{C} [rad];entries/N_{max} [#]", 150, -0.15, 0.15);
    hAngleU[i] =
      new TH1F(Form("hAngleu_%d", i), ";#theta_{C} [rad];entries/N_{max} [#]", 150, 0.6, 0.9);
    hCMom[i] = new TH2F(Form("cmom_%d", i), "hcmom", 1000, 0, 10, 500, 0.6, 0.9);
    hNph[i] = new TH1F(Form("hNph_%d", i), ";detected photons [#];entries [#]", 80, 0, 80);
    mAngle[i] =
      acos(sqrt(momentum * momentum + glx_mass[i] * glx_mass[i]) / momentum / 1.473); // 1.4738
    fAngle[i] = new TF1(Form("fAngle_%d", i), "[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2])", 0.7, 0.9);
    fAngle[i]->SetParameter(0, 1);         // const
    fAngle[i]->SetParameter(1, mAngle[i]); // mean
    fAngle[i]->SetParameter(2, sigma[i]);  // sigma
    hAngle[i]->SetMarkerStyle(20); hAngleDiff[i]->SetMarkerStyle(20);
    hAngle[i]->SetMarkerSize(0.8); hAngleDiff[i]->SetMarkerSize(0.8);
    if (moms < 4)
      hLnDiff[i] =
        new TH1F(Form("hLnDiff_%d", i), ";ln L(#pi) - ln L(K);entries [#]", 80, -160, 160);
    else
      hLnDiff[i] =
        new TH1F(Form("hLnDiff_%d", i), ";ln L(#pi) - ln L(K);entries [#]", 80, -100, 100);
  }

  hAngle[2]->SetLineColor(4); hAngleDiff[2]->SetLineColor(4);
  hAngle[3]->SetLineColor(2); hAngleDiff[2]->SetLineColor(2);
  hAngle[2]->SetMarkerColor(kBlue + 1); hAngleDiff[2]->SetMarkerColor(kBlue + 1);
  hAngle[3]->SetMarkerColor(kRed + 1); hAngleDiff[3]->SetMarkerColor(kRed + 1);
  fAngle[2]->SetLineColor(4);
  fAngle[3]->SetLineColor(2);

  hLnDiff[2]->SetLineColor(4);
  hLnDiff[3]->SetLineColor(2);
  int evtcount = 0, count[5] = {0};
  bool debug = false;
  TCanvas *cc;
  if (debug) cc = new TCanvas("cc", "cc", 800, 800);
  TLine *gLine = new TLine();
  TH1F *hphi = new TH1F("hphi", "hphi;[GeV/c];events [#]", 5000, 0, 1.5);
  TH1F *hrho = new TH1F("hrho", "hphi;[GeV/c];events [#]", 5000, 0, 1.5);

  TGraph cagr;
  double bartime, luttime, diftime, adiff, len, leny, lenz;
  //DrcHit hit;
  double dibin = -100 + ibin * 20 + 10;

  TCut cut = "";
  if (ibar > -1) cut += Form("(DrcEvent.fId == %d)", ibar);
  //if (ibar > -1) cut += Form("(fabs(DrcEvent.fId -%d) < 2)", ibar);

  // cut += "fabs(DrcEvent.fPosition.fX-0)<5"; // 10
  if (ibin > -1) cut += Form("fabs(DrcEvent.fPosition.fX-%f)<10", dibin); // 10

  if (cor_level < 2 && anglecorr > 0) cut += Form("fabs(DrcEvent.fMomentum.Mag()-%2.4f)<1", moms);
  //if (cor_level < 2 && anglecorr > 0) cut += "fabs(DrcEvent.fMomentum.Mag())>2.5";  
  else cut += Form("fabs(DrcEvent.fMomentum.Mag()-%2.4f)<0.2", moms);

  //cut += "DrcEvent.fPdg > 0";

  // up
  // cut += "(DrcEvent.fId >= 31)";
  // cut += "(DrcEvent.fId <= 35)";

  // down
  // cut += "(DrcEvent.fId >= 7)";
  // cut += "(DrcEvent.fId <= 11)";

  //cut += "fabs(DrcEvent.fPosition.fX-0)<20";
  cut.Print();

  //if (!glx_initc(infile, cut)) return;
  if(!glx_initc(infile,1,"data/reco_lut")) return;

/*
  for (int e = 0; e < glx_elist->GetN() && e < glx_ch->GetEntries(); e++) {
    glx_ch->GetEntry(glx_elist->GetEntry(e));

    if (glx_events->GetEntriesFast() > 1) continue;
    hMult->Fill(glx_events->GetEntriesFast());

    for (int t = 0; t < glx_events->GetEntriesFast(); t++) {
      glx_nextEventc(e, t, 1000);
*/
  for (int e = 0; e < glx_ch->GetEntries(); e++){
    glx_ch->GetEntry(e);
    cout<<"event: "<<e<<endl;

    for (int t = 0; t < glx_events->GetEntriesFast(); t++){
      glx_nextEventc(e,t,1000);
      cout<<"track: "<<t<<endl;

      if(e%1000 == 0)
	      cout<<"Particle count "<<count[2]<<" "<<count[3]<<endl;

      // if (glx_event->GetPdg() > 0) continue;
      if (glx_event->GetType() > 0 && !sim) { // beam
        // if(glx_event->GetType()!=2) continue; // 1-LED 2-beam 0-rest
        // if(glx_event->GetDcHits()<25) continue;
        // if (glx_event->GetTofTrackDist() > 1.5) continue;
        if (fabs(glx_event->GetPdg()) == 211) {
          if (glx_event->GetChiSq() > 10) continue;
          if (fabs(glx_event->GetInvMass() - 0.75) > 0.15) continue;     // 0.05
          if (fabs(glx_event->GetMissMass()) > 0.05) continue; // 0.001
        } else if (fabs(glx_event->GetPdg()) == 321) {
	  if (glx_event->GetChiSq() > 10) continue;
          if (fabs(glx_event->GetInvMass() - 1.02) > 0.02) continue;     // 0.02
          if (fabs(glx_event->GetMissMass()) > 0.05) continue; // 0.007
        } else continue;
      } else { // geant
        noise = 0.2;
      }

      int pdgId = glx_findPdgId(glx_event->GetPdg());
      int bar = glx_event->GetId();
      int lid = bar;
      int opbox = 0;
      double time0 = glx_event->GetTime();

      if (glx_event->GetPdg() == 321) hphi->Fill(glx_event->GetInvMass());
      if (glx_event->GetPdg() == 211) hrho->Fill(glx_event->GetInvMass());

      posInBar = glx_event->GetPosition();
      posInBar_true = glx_event->GetPosition_Truth();

      momInBar = glx_event->GetMomentum();
      double momentum = momInBar.Mag();
      int bin = (100 + posInBar.X()) / 200. * nbins;
      
      // selection
      if (bar != ibar && ibar > -1) continue;
      //if (fabs(bar - ibar) > 1 && ibar > -1) continue;

      if (fabs(posInBar.X() - dibin) > 10 && ibin > -1) continue;

      if (cor_level < 2 && anglecorr > 0) {
	if (momentum < 2.5) continue;
        //if (fabs(momentum - moms) > 1) continue;
      } else {

	// select tracks close to the center of the bar
        if (bar == 25 && fabs(posInBar.Y() - 14.25) > 1.25) continue;
        if (bar == 26 && fabs(posInBar.Y() - 17.75) > 1.25) continue;
        if (bar == 27 && fabs(posInBar.Y() - 21.25) > 1.25) continue;
        if (bar == 28 && fabs(posInBar.Y() - 24.75) > 1.25) continue;
        if (bar == 29 && fabs(posInBar.Y() - 28.25) > 1.25) continue;
        if (bar == 30 && fabs(posInBar.Y() - 31.75) > 1.25) continue;

        if (bar == 31 && fabs(posInBar.Y() - 35.25) > 1.25) continue;
        if (bar == 32 && fabs(posInBar.Y() - 38.75) > 1.25) continue;
        if (bar == 33 && fabs(posInBar.Y() - 42.25) > 1.25) continue;
        if (bar == 34 && fabs(posInBar.Y() - 45.75) > 1.25) continue;
        if (bar == 35 && fabs(posInBar.Y() - 49.25) > 1.25) continue;

        // if (pdgId == 2 && count[2] > 1000) continue;
        // if (pdgId == 3 && count[3] > 1000) continue;
	if (fabs(momentum - moms) > 0.2) continue;
      }

      if (bar > 23) {
        opbox = 1;
        barend = 294.022;
      }

      // if(glx_event->GetParent()>0) continue;
      // track correction
      // momInBar.RotateX(dx*TMath::DegToRad());
      // momInBar.RotateY(dy*TMath::DegToRad());

      hWall->Fill(posInBar.X(), posInBar.Y());
      // double wdx = posInBar.X()-posInBar_true.X();
      // if(fabs(wdx)>2) continue;
      // hWallPos->Fill(posInBar.X(),posInBar.Y(),wdx);
      // int wx=hWallPos->FindBin(posInBar.X(),posInBar.Y());
      // wallb[wx]++;

      if (glx_event->GetType() > 0 && !sim) { // data
        momInBar.RotateX(bar_corr_x[bar/12]);
        momInBar.RotateY(bar_corr_y[bar/12]);
	momInBar.RotateZ(bar_corr_z[bar/12]);
      } else { // sim
        cz = momInBar;
        momInBar.RotateX(gRandom->Gaus(0, 0.002));
        momInBar.RotateY(gRandom->Gaus(0, 0.002));
	momInBar.RotateZ(gRandom->Gaus(0, 0.002));
        // momInBar.RotateX(gRandom->Gaus(0, 0.0025));
        // momInBar.Rotate(gRandom->Uniform(0, TMath::PiOver2()), cz);
        // time0 += gRandom->Gaus(0, 0.25);
      }

      cz = momInBar.Unit();
      
      for (int p = 0; p < 5; p++) {
        mAngle[p] = mangle(p, momentum);
        fAngle[p]->SetParameter(1, mAngle[p]); // mean
        fAngle[p]->SetParameter(2, sigma[p]);
      }

      if (bfitcorr) {
        fAngle[2]->SetParameter(2, 0.0067);
        fAngle[3]->SetParameter(2, 0.0067);

        TGraph cgr;
        sum1 = 0;
        sum2 = 0;
        for (int h = 0; h < glx_event->GetHitSize(); h++) {
          DrcHit hit = glx_event->GetHit(h);
          int ch = hit.GetChannel();
          int pmt = hit.GetPmtId();
          int pix = hit.GetPixelId();

          if (opbox == 1) {
            pmt -= 108;
            ch -= 108 * 64;
          }

          double hitTime = hit.GetLeadTime() - time0;
          if (pmt <= 10 || (pmt >= 90 && pmt <= 96)) continue; // dummy pmts
          if (ch > glx_nch) continue;

          bool reflected = hitTime > 44;

          lenx = fabs(barend - posInBar.X());
          double rlenx = 2 * radiatorL - lenx;
          double dlenx = lenx;
          if (reflected) lenx = 2 * radiatorL - lenx;
          double p1, p2;

#if 0
          for (int i = 0; i < lutNode[lid][ch]->Entries(); i++) {
            dird = lutNode[lid][ch]->GetEntry(i);
            evtime = lutNode[lid][ch]->GetTime(i);
            pathid = lutNode[lid][ch]->GetPathId(i);
            int nrefl = lutNode[lid][ch]->GetNRefl(i);
            double weight = lutNode[lid][ch]->GetWeight(i);

            for (int r = 0; r < 2; r++) {
              if (!reflected && r == 1) continue;

              if (r) lenx = rlenx;
              else lenx = dlenx;

              for (int u = 0; u < 4; u++) {
                if (u == 0) dir = dird;
                if (u == 1) dir.SetXYZ(dird.X(), -dird.Y(), dird.Z());
                if (u == 2) dir.SetXYZ(dird.X(), dird.Y(), -dird.Z());
                if (u == 3) dir.SetXYZ(dird.X(), -dird.Y(), -dird.Z());
                if (r) dir.SetXYZ(-dir.X(), dir.Y(), dir.Z());
                if (dir.Angle(fnY1) < criticalAngle || dir.Angle(fnZ1) < criticalAngle) continue;
                dir = dir.Unit();
                luttheta = dir.Angle(TVector3(-1, 0, 0));

                if (opbox == 1) {
                  dir.RotateZ(TMath::Pi());
                  luttheta = dir.Angle(TVector3(1, 0, 0));
                }
                if (r) luttheta = TMath::Pi() - luttheta;
                tangle = momInBar.Angle(dir);

                bartime = lenx / cos(luttheta) / 19.65;
                luttime = bartime + evtime;

		if (reflected) luttime -= acorrTR[bar][bin][pmt];
		else luttime -= acorrTD[bar][bin][pmt];

                diftime = luttime - hitTime;

                if (r) tangle += acorrAR[bar][bin][pmt]; // per PMT corr
                else tangle += acorrAD[bar][bin][pmt];

                if (fabs(diftime) < 2.0 && cor_level > 0) tangle -= 0.0025 * diftime; // chrom corr

                if (fabs(diftime) > 0.5 + luttime * 0.03) continue;
                if (fabs(tangle - mAngle[2]) > 0.015 && fabs(tangle - mAngle[3]) > 0.015) continue;

                sum1 += TMath::Log(fAngle[2]->Eval(tangle) + noise);
                sum2 += TMath::Log(fAngle[3]->Eval(tangle) + noise);

                TVector3 rdir = TVector3(-dir.Y(), -dir.X(), dir.Z());
                cz = momInBar.Unit();
                rdir.RotateUz(cz);
                double lphi = rdir.Phi();
                double tx = tangle * TMath::Sin(lphi);
                double ty = tangle * TMath::Cos(lphi);
                hRing->Fill(tx, ty);
                cgr.SetPoint(cgr.GetN(), tx, ty);
              }
            }
          }
#endif
        }
        double theta = mAngle[3];
        if (sum1 > sum2) theta = mAngle[2];
        // theta = 0.5 * (mAngle[2] + mAngle[3]);

        {
          double tx0(0), ty0(0);
          FitRing(tx0, ty0, theta, cgr);
          TVector3 rcorr(tx0, ty0, 1 - TMath::Sqrt(tx0 * tx0 + ty0 * ty0));
          TVector3 oo = momInBar;
          momInBar.RotateX(2.5 * rcorr.Theta());
          momInBar.Rotate(rcorr.Phi(), oo);
          std::cout << "====== " << rcorr.Theta() << " " << rcorr.Phi() << std::endl;

          if (debug) {
            cc->cd();
            hRing->Draw("colz");
            gStyle->SetOptStat(0);
            TLegend *legr = new TLegend(0.25, 0.4, 0.65, 0.6);
            legr->SetFillStyle(0);
            legr->SetBorderSize(0);
            legr->AddEntry((TObject *)0, Form("Entries %0.0f", hRing->GetEntries()), "");
            legr->AddEntry((TObject *)0, Form("#Delta#theta_{c} %f [mrad]", rcorr.Theta() * 1000),
                           "");
            legr->AddEntry((TObject *)0, Form("#Delta#varphi_{c} %f [mrad]", rcorr.Phi()), "");
            legr->Draw();
            TArc *arc = new TArc(tx0, ty0, theta);
            arc->SetLineColor(kRed);
            arc->SetLineWidth(2);
            arc->SetFillStyle(0);
            arc->Draw();
            TArc *arcI = new TArc(0, 0, theta);
            arcI->SetLineColor(kGreen);
            arcI->SetLineWidth(2);
            arcI->SetFillStyle(0);
            arcI->Draw();

            cc->Update();
            cc->WaitPrimitive();
            // cc->SetName(Form("hRing_e%d",++indd));
            // glx_canvasAdd(cc);
            // glx_canvasSave(1);
            // glx_canvasDel(Form("hRing_e%d",indd));
            hRing->Reset();
          }
        }
      }

      cout<<"test 1"<<endl;
      
      sum1 = 0;
      sum2 = 0;
      int nph = 0, nphc = 0;
      // hNphC->Fill(glx_event->GetHitSize());
      bool goodevt = 0;

      for (int h = 0; h < glx_event->GetHitSize(); h++) {
        DrcHit hit = glx_event->GetHit(h);
        int ch = hit.GetChannel();
        int pmt = hit.GetPmtId();
        int pix = hit.GetPixelId();

        if (opbox == 0) {
          if (ch > glx_nch) continue;
        } else {
          if (ch < glx_nch) continue;
          pmt -= glx_npmt;
          ch -= glx_nch;
        }

        if (pmt <= 10 || (pmt >= 90 && pmt <= 96)) continue; // dummy pmts

        double hitTime = hit.GetLeadTime() - time0;
        if (sim) {
          if (gRandom->Uniform(0, 1) < 0.36) continue;
        }

        nphc++;
        bool reflected = hitTime > 44;

        // if(!reflected) continue;
        lenx = fabs(barend - posInBar.X());
        double rlenx = 2 * radiatorL - lenx;
        double dlenx = lenx;
        if (reflected) lenx = 2 * radiatorL - lenx;

        bool isGood(false);
        double p1, p2;

#if 1
        for (int i = 0; i < lutNode[lid][ch]->Entries(); i++) {
          dird = lutNode[lid][ch]->GetEntry(i);
          evtime = lutNode[lid][ch]->GetTime(i);
          pathid = lutNode[lid][ch]->GetPathId(i);
          int nrefl = lutNode[lid][ch]->GetNRefl(i);
          double weight = 1.5; // 20* lutNode[lid][ch]->GetWeight(i);
                               // if(weight<0.02*20) continue;

          TString spath = Form("%ld", pathid);
          // // if (!spath.BeginsWith("")) continue;
          if (spath.Contains("8")) continue;
          if (spath.Contains("7")) continue;

          // if (!spath.Contains("92")) continue;
          // if(!spath.BeginsWith("42920")) continue;
          // if(!spath.EqualTo("42920")) continue;
          // if(!spath.EqualTo("32412930")) continue;
          // if (nrefl > 3) continue;

          bool samepath(false);
          if (fabs(pathid - hit.GetPathId()) < 0.0001) samepath = true;
          p1 = hit.GetPathId();
          if (samepath) p2 = pathid;
          // if(!samepath) continue;

          if (opbox == 1) {
            ldir = dird;
            ldir.RotateY(-TMath::PiOver2());
          } else {
            ldir = dird;
            ldir.RotateY(TMath::PiOver2());
            ldir.RotateX(-TMath::Pi());
          }

          // int iphi = nphi*(ldir.Phi()+TMath::Pi())/TMath::TwoPi();
          // int itheta = ntheta*(ldir.Theta())/TMath::PiOver2();

          double lphi = ldir.Phi();
          double ltheta = ldir.Theta();
          if (lphi < 0) lphi = TMath::TwoPi() + lphi;
          if (ltheta > TMath::PiOver2()) ltheta = TMath::Pi() - ltheta;

          int iphi = nphi * (lphi) / TMath::TwoPi();
          int itheta = ntheta * (ltheta) / TMath::PiOver2();

          for (int r = 0; r < 2; r++) {
            if (!reflected && r == 1) continue;
            if (r) lenx = rlenx;
            else lenx = dlenx;

            for (int u = 0; u < 4; u++) {
              if (u == 0) dir = dird;
              if (u == 1) dir.SetXYZ(dird.X(), -dird.Y(), dird.Z());
              if (u == 2) dir.SetXYZ(dird.X(), dird.Y(), -dird.Z());
              if (u == 3) dir.SetXYZ(dird.X(), -dird.Y(), -dird.Z());
              if (r) dir.SetXYZ(-dir.X(), dir.Y(), dir.Z());
              if (dir.Angle(fnY1) < criticalAngle || dir.Angle(fnZ1) < criticalAngle) continue;
              dir = dir.Unit();

              //if (opbox == 0) luttheta = dir.Angle(TVector3(-1, 0, 0));
              //else luttheta = dir.Angle(TVector3(1, 0, 0));
              //if (r) luttheta = TMath::Pi() - luttheta;
	      
	      luttheta = dir.Angle(TVector3(-1,0,0));
	      if(luttheta > TMath::PiOver2()) luttheta = TMath::Pi()-luttheta;

              len = fabs(lenx / cos(luttheta));
              lenz = fabs(len * dir.Z());
              leny = fabs(len * dir.Y());
              bartime = lenx / cos(luttheta) / 19.65; // 19.7 203.767 for 1.47125
              luttime = bartime + evtime;

	      if (reflected) luttime -= acorrTR[bar][bin][pmt];
	      else luttime -= acorrTD[bar][bin][pmt];

              diftime = luttime - hitTime;

              tangle = momInBar.Angle(dir);
              // if (spath.BeginsWith("32")) tangle += -0.0015;
              // if (spath.BeginsWith("42")) tangle += -0.002;
              // if (spath.BeginsWith("31")) tangle += 0.0005;
              // if (spath.BeginsWith("41")) tangle += 0.0005;

              if (r) tangle += acorrAR[bar][bin][pmt]; //  per PMT corr
              else tangle += acorrAD[bar][bin][pmt];

              if (fabs(diftime) < 2.0 && cor_level > 0) tangle -= 0.0025 * diftime; // chrom corr

              hTime->Fill(hitTime);
              hCalc->Fill(luttime);
              // if(dir.Theta()>TMath::PiOver2()) continue;
              // if(fabs(tangle-mAngle[2])>0.02) continue;
              // std::cout<<Form(" %d  %5d  %9.2f   %9.2f ",nz,ny,2*lenx/nz,2*lenx/ny)<<std::endl;
              // if(samepath)
              // if(tangle-mAngle[2]<cut_cangle && tangle-mAngle[3]<cut_cangle)
              if (fabs(tangle - mAngle[2]) < cut_cangle || fabs(tangle - mAngle[3]) < cut_cangle) {
                hDiff->Fill(diftime);
                hDiffT->Fill(diftime);
                if (r) hDiffR->Fill(diftime);
                else hDiffD->Fill(diftime);
              }

              if (fabs(diftime) > cut_tdiff + luttime * 0.025) continue;

              adiff = (tangle - mAngle[pdgId]) * 1000;
              if (pdgId == 2 && fabs(adiff) < 50) {
                hLutCorr->Fill(lphi, ltheta);
                hCorrLut[iphi][itheta]->Fill(adiff);

                if (r) {
                  if (fabs(diftime) < 2) hCorrAR[bar][bin][pmt]->Fill(adiff);
                  if (adiff < 20) hCorrTR[bar][bin][pmt]->Fill(diftime);
                } else {
                  if (fabs(diftime) < 1) hCorrAD[bar][bin][pmt]->Fill(adiff);
                  if (adiff < 20) hCorrTD[bar][bin][pmt]->Fill(diftime);
                }
              }

              hChrom->Fill(diftime, adiff);

              if (pdgId == 2) hAngleU[u]->Fill(tangle, weight);
              hAngle[pdgId]->Fill(tangle, weight);
	      hAngleDiff[pdgId]->Fill(adiff/1000., weight);

              if (fabs(tangle - mAngle[2]) > 1 * cut_cangle &&
                  fabs(tangle - mAngle[3]) > 1 * cut_cangle)
                continue; // 8

              if (1 && pdgId == 2) {
                TVector3 rdir = TVector3(dir.Y(), dir.X(), dir.Z());
                cz = momInBar.Unit();
                rdir.RotateUz(cz);

                double rangle = tangle - mangle(2, momentum) + mangle(2, 3);
                double lphi = rdir.Phi();
                double tx = rangle * TMath::Sin(lphi); // rdir.Theta();
                double ty = rangle * TMath::Cos(lphi);
                double tp = asin(ty / rangle);
                if (tx < 0) tp = -asin(ty / rangle) + TMath::Pi();
                tp *= TMath::RadToDeg();
                tp += 90;

                // if(fabs(tp-300)<5) continue;
                // double tt = rdir.Theta();
                if (fabs(tangle - mAngle[2]) < 0.012) hRing->Fill(tx, ty);

                // for cherenckov circle fit
                cagr.SetPoint(cagr.GetN(), tx, ty);
              }

              isGood = true;

              // if(r && pdgId==2)
              h2Time->Fill(hitTime, diftime);
              // hTime->Fill(hitTime);
              // hCalc->Fill(luttime);

              sum1 += weight * TMath::Log(fAngle[2]->Eval(tangle) + noise);
              sum2 += weight * TMath::Log(fAngle[3]->Eval(tangle) + noise);

              if (0) {
                TString x = (sum1 > sum2) ? " <====== PION" : "";
                std::cout << Form("%1.6f  %1.6f | %1.6f  %1.6f        pid %d",
                                  TMath::Log(fAngle[2]->Eval(tangle) + noise),
                                  TMath::Log(fAngle[3]->Eval(tangle) + noise), sum1, sum2, pdgId)
                          << "  " << std::endl;

                cc->cd();
                fAngle[2]->Draw("");
                fAngle[3]->Draw("same");

                cc->Update();
                gLine->SetLineWidth(2);
                gLine->SetX1(tangle);
                gLine->SetX2(tangle);
                gLine->SetY1(cc->GetUymin());
                gLine->SetY2(cc->GetUymax());
                gLine->Draw();
                cc->Update();
                cc->WaitPrimitive();
              }
            }
          }
        }

        if (isGood) {
          nph++;
          if (pmt < 108) {
            glx_hdigi[pmt]->Fill(pix % 8, pix / 8);
            goodevt = 1;
          }
        }
#endif
      }

      if (goodevt) evtcount++;

      if (nph < 10) continue;
      hNph[pdgId]->Fill(nph);
      hNphC->Fill(nphc);

      if (0 && pdgId == 2) {
        // double xangle = glx_fit(hAngle[pdgId],0.02,10,0.008,1,"Q0").X();
        // if(xangle>0.84) continue;
        // hCMom[pdgId]->Fill(momentum,xangle);
        // hAngle[pdgId]->Reset();

        //double sigmat = glx_fit(hAngle[pdgId], 0.02, 10, 0.008, 1, "Q0").Y();
        // hSpr->Fill(x,y,sigmat);
        hAngle[pdgId]->Reset();
      }

      // auto cdigi = glx_drawDigi();
      // cdigi->SetName(Form("hp_k_%d", evtcount));
      // glx_canvasAdd(cdigi);
      // glx_canvasSave("data/reco_lut_02_scan_sel_02_single", 2);
      // glx_resetDigi();

      double sum = sum1 - sum2;
      hLnDiff[pdgId]->Fill(sum);

      count[pdgId]++;

      if (0) {
        //	if(!cc)
        TString x = (sum1 > sum2) ? " <====== Pion" : "";
        // std::cout<<Form("f %1.6f s %1.6f mcp %d pix %d   pid %d",aminf,amins,mcp,pix
        // ,prt_particle)<<"  "<<x <<std::endl;

        std::cout << "PID " << glx_event->GetPdg() << " sum1 " << sum1 << " sum2 " << sum2
                  << " sum " << sum << " " << x << std::endl;

        cc->cd();

        // if(hAngle[pdgId]->GetMaximum()>0) hAngle[pdgId]->Scale(1/hAngle[pdgId]->GetMaximum());

        hAngle[pdgId]->Draw("");
        fAngle[2]->Draw("same");
        fAngle[3]->Draw("same");

        cc->Update();
        TLine *line = new TLine(0, 0, 0, 1000);
        line->SetX1(mAngle[3]);
        line->SetX2(mAngle[3]);
        line->SetY1(cc->GetUymin());
        line->SetY2(cc->GetUymax());
        line->SetLineColor(kRed);
        line->Draw();

        TLine *line2 = new TLine(0, 0, 0, 1000);
        line2->SetX1(mAngle[2]);
        line2->SetX2(mAngle[2]);
        line2->SetY1(cc->GetUymin());
        line2->SetY2(cc->GetUymax());
        line2->SetLineColor(kBlue);
        line2->Draw();

        cc->Update();
        cc->WaitPrimitive();
      }
    }
  }

  cout<<evtcount<<" events"<<endl;
  
#if 1
  if (evtcount > 0) {
    for (int i = 0; i < glx_nch; i++) {
      int pmt = i / 64;
      int pix = i % 64;
      double rel = glx_hdigi[pmt]->GetBinContent(pix % 8 + 1, pix / 8 + 1) / (double)evtcount;
      glx_hdigi[pmt]->SetBinContent(pix % 8 + 1, pix / 8 + 1, rel);
    }
  }

  //  TString nid = Form("_%d_%d_%2.1f_%2.4f_%2.3f_%2.3f", ibar, ibin, moms, scan, dx, d);
  TString nid = Form("_%d_%d_%2.1f_%d", ibar, ibin, moms, cor_level);
  if (sim) nid = "_simM" + nid;
  nid.ReplaceAll("-", "m");
  nid.ReplaceAll(".", "d");

  double nph = 0, nphm = 0, maxTD, maxTR, maxTT, sep = 0, esep = 0;

  if (anglecorr == 2 && cor_level < 2) { // per pmt correction

    TCanvas *canv_angle, *canv_time;
    canv_angle = new TCanvas("canv_angle","canv_angle");
    canv_time = new TCanvas("canv_time","canv_time");
    double zcorrAD, zsigmaAD, zcorrAR, zsigmaAR, zcorrTD, zsigmaTD, zcorrTR, zsigmaTR, zfracAD, zfracAR;
    int bar, bin, pmt, level;
    TFile fc(corrfile, "recreate");
    TTree *tc = new TTree("corr", "corr");
    tc->Branch("zcorrAD", &zcorrAD, "zcorrAD/D");
    tc->Branch("zcorrAR", &zcorrAR, "zcorrAR/D");
    tc->Branch("zcorrTD", &zcorrTD, "zcorrTD/D");
    tc->Branch("zcorrTR", &zcorrTR, "zcorrTR/D");
    tc->Branch("zsigmaAD", &zsigmaAD, "zsigmaAD/D");
    tc->Branch("zsigmaAR", &zsigmaAR, "zsigmaAR/D");
    tc->Branch("zsigmaTD", &zsigmaTD, "zsigmaTD/D");
    tc->Branch("zsigmaTR", &zsigmaTR, "zsigmaTR/D");
    tc->Branch("zfracAD", &zfracAD, "zfracAD/D");
    tc->Branch("zfracAR", &zfracAR, "zfracAR/D");
    tc->Branch("bar", &bar, "bar/I");
    tc->Branch("bin", &bin, "bin/I");
    tc->Branch("pmt", &pmt, "pmt/I");
    tc->Branch("level", &level, "level/I");

    for (bar = 0; bar < 48; bar++) {
      for (bin = 0; bin < nbins; bin++) {
	if(bin == ibin && bar == ibar) {
	  if(cor_level==1) canv_angle->Print("data/anglepmt"+nid+".pdf[");
	  if(cor_level==0) canv_time->Print("data/timepmt"+nid+".pdf[");
	}

        for (pmt = 0; pmt < glx_npmt; pmt++) {
          zcorrAD = 0;
          zcorrTD = 0;
          zsigmaAD = 0;
          zsigmaTD = 0;
          zcorrAR = 0;
          zcorrTR = 0;
          zsigmaAR = 0;
          zsigmaTR = 0;
	  zfracAD = 0;
	  zfracAR = 0;
          double xmax = 0;

          if (hCorrAD[bar][bin][pmt]->GetEntries() < 200 && hCorrAR[bar][bin][pmt]->GetEntries() < 200) continue;

          glx_normalize(hCorrAD[bar][bin][pmt], hCorrAR[bar][bin][pmt]);
          glx_normalize(hCorrTD[bar][bin][pmt], hCorrTR[bar][bin][pmt]);

          if (cor_level == 1) {
	    if(bin == ibin && bar == ibar) {
	      canv_angle->cd();
	      hCorrAD[bar][bin][pmt]->Draw();
	      hCorrAR[bar][bin][pmt]->SetLineColor(kRed);
	      hCorrAR[bar][bin][pmt]->Draw("same");
	    }

            fit->SetParLimits(0, 0, 1000000000);
            fit->SetParameter(1, xmax);
            fit->SetParLimits(1, xmax - 10, xmax + 10);
            fit->SetParLimits(2, 5, 8);

            if (hCorrAD[bar][bin][pmt]->GetEntries() > 200) {
	      fit->SetLineColor(kBlack);
              hCorrAD[bar][bin][pmt]->Fit("fgaus", "Q", "", xmax - 25, xmax + 25);
              zcorrAD = -fit->GetParameter(1);
              zsigmaAD = fit->GetParameter(2);

	      for(int p=0;p<3;p++) gaus->SetParameter(p, fit->GetParameter(p)); 
	      zfracAD = gaus->Integral(xmax-25, xmax+25)/fit->Integral(xmax-25, xmax+25);
            }
            if (hCorrAR[bar][bin][pmt]->GetEntries() > 200) {
	      fit->SetLineColor(kRed);
              hCorrAR[bar][bin][pmt]->Fit("fgaus", "Q", "", xmax - 25, xmax + 25);
              zcorrAR = -fit->GetParameter(1);
              zsigmaAR = fit->GetParameter(2);

	      for(int p=0;p<3;p++) gaus->SetParameter(p, fit->GetParameter(p)); 
	      zfracAR = gaus->Integral(xmax-25, xmax+25)/fit->Integral(xmax-25, xmax+25);
            }
            level = 2;
	    if(bin==ibin && bar==ibar) 
	      canv_angle->Print("data/anglepmt"+nid+".pdf");
          }

          if (cor_level == 0) {
	    if(bin == ibin && bar == ibar) {
	      canv_time->cd();
	      hCorrTD[bar][bin][pmt]->Draw();
	      hCorrTR[bar][bin][pmt]->SetLineColor(kRed);
	      hCorrTR[bar][bin][pmt]->Draw("same");
	    }

	    fit->SetParLimits(0, 0, 1000000000);
            fit->SetParameter(1, xmax);
            fit->SetParLimits(1, xmax - 4, xmax + 4);
            fit->SetParameter(2, 0.5);
            fit->SetParLimits(2, 0.2, 10.0); // width
            //fit->FixParameter(3, 0);
            //fit->FixParameter(4, 0);
            if (hCorrTD[bar][bin][pmt]->GetEntries() > 200) {
	      fit->SetParLimits(1, xmax - 2, xmax + 2);
	      fit->SetLineColor(kBlack);
              auto ff = hCorrTD[bar][bin][pmt]->Fit("fgaus", "SQ", "", -2, 2);
              zcorrTD = -ff->Parameter(1);
              zsigmaTD = ff->Parameter(2);
            }
            if (hCorrTR[bar][bin][pmt]->GetEntries() > 200) {
	      fit->SetParLimits(1, xmax - 3, xmax + 3);
	      fit->SetLineColor(kRed);
              auto ff = hCorrTR[bar][bin][pmt]->Fit("fgaus", "SQ", "", -3, 3);
              zcorrTR = -ff->Parameter(1);
              zsigmaTR = ff->Parameter(2);
            }
            level = 1;
	    if(bin==ibin && bar==ibar) 
	      canv_time->Print("data/timepmt"+nid+".pdf");
          } else {
            zcorrTD = acorrTD[bar][bin][pmt];
            zcorrTR = acorrTR[bar][bin][pmt];
	    zsigmaTD = asigmaTD[bar][bin][pmt];
            zsigmaTR = asigmaTR[bar][bin][pmt];
          }

          std::cout << "L " << cor_level << " bar = " << bar << " bin = " << bin << " pmt = " << pmt
                    << Form(" ad %-8.5f ar %-8.5f", zcorrAD, zcorrAR)
                    << Form(" td %-8.5f tr %-8.5f", zcorrTD, zcorrTR) << std::endl;

          if (0) {
            canv_angle = glx_canvasAddOrGet("canv_angle");
            canv_angle->cd();
            hCorrAD[bar][bin][pmt]->Fit("fgaus", "Q", "", xmax - 30, xmax + 30);
            hCorrAR[bar][bin][pmt]->Fit("fgaus", "Q", "", xmax - 30, xmax + 30);
            gStyle->SetOptStat(1);
            gStyle->SetOptFit(1);
            gStyle->SetOptTitle(1);
            hCorrAD[bar][bin][pmt]->Draw();
            hCorrAR[bar][bin][pmt]->SetLineColor(kRed);
            hCorrAR[bar][bin][pmt]->Draw("same");
            gPad->Modified();
            gPad->Update();

            canv_time = glx_canvasAddOrGet("canv_time");
            canv_time->cd();
            hCorrTD[bar][bin][pmt]->Fit("gaus", "Q", "", -2, 2);
            hCorrTR[bar][bin][pmt]->Fit("gaus", "Q", "", -3, 3);
            hCorrTD[bar][bin][pmt]->Draw();
            hCorrTR[bar][bin][pmt]->SetLineColor(kRed);
            hCorrTR[bar][bin][pmt]->Draw("same");
            // cc->Print(Form("data/corr_pmt/corr_pmt_%d_%d.png",bar,pmt));
            gPad->Update();
            gPad->WaitPrimitive();
          }
          tc->Fill();
        }

        if(bin == ibin && bar == ibar) {
	  if(cor_level==1) canv_angle->Print("data/anglepmt"+nid+".pdf]");
	  if(cor_level==0) canv_time->Print("data/timepmt"+nid+".pdf]");
	}
      }
    }

    std::cout << "--- writing " << corrfile << std::endl;
    tc->Write();
    fc.Write();
    fc.Close();

    // copy individual level correction file
    TString corrfilelevel = corrfile; 
    corrfilelevel.Remove(corrfilelevel.Length() - 5);
    corrfilelevel += Form("_level%d.root", cor_level+1);
    gSystem->CopyFile(corrfile.Data(), corrfilelevel.Data(), true); 
  }

  {
    // lut correction
    // for(int b=0; b<nphi; b++){
    //   for(int p=0; p<ntheta; p++){
    // 	if(hCorrLut[b][p]->GetEntries()<100) continue;

    // 	//glx_canvasAdd(hCorrLut[b][p]->GetName(),800,800);

    // 	double xmax = 0;
    // 	fit->SetParLimits(0,0,1000000);
    // 	fit->SetParameter(1,xmax);
    // 	fit->SetParLimits(1,xmax-10,xmax+10);
    // 	fit->SetParLimits(2,5,8); // width

    // 	hCorrLut[b][p]->Fit("fgaus","M","",-30,30);
    // 	hLutCorr1->SetBinContent(b+1, p+1, fit->GetParameter(1));
    // 	  //hCorrLut[b][p]->Draw();

    // 	// gPad->Update();
    //     // gPad->WaitPrimitive();
    //   }
    // }

    // glx_canvasAdd("lutcorr"+nid,800,400);
    // hLutCorr->Draw("colz");

    // glx_canvasAdd("lutcorr1",800,400);
    // hLutCorr1->Draw("colz");
  }

  { // ring
    glx_canvasAdd("ring" + nid, 800, 800);
    hRing->Draw("colz");

    double x0(0), y0(0), theta(mangle(2, 3));
    FitRing(x0, y0, theta, cagr);

    TVector3 rcorr(x0, y0, 1 - TMath::Sqrt(x0 * x0 + y0 * y0));
    std::cout << "Tcorr " << rcorr.Theta() << "  Pcorr " << rcorr.Phi() << std::endl;
    TLegend *legr = new TLegend(0.25, 0.4, 0.65, 0.6);
    legr->SetFillStyle(0);
    legr->SetBorderSize(0);
    legr->AddEntry((TObject *)0, Form("Entries %0.0f", hRing->GetEntries()), "");
    legr->AddEntry((TObject *)0, Form("#Delta#theta_{c} %f [mrad]", rcorr.Theta() * 1000), "");
    legr->AddEntry((TObject *)0, Form("#varphi_{c} %f [rad]", rcorr.Phi()), "");
    legr->Draw();

    TArc *arc = new TArc(x0, y0, theta);
    std::cout << "XXXXXXXXXXXXXXX x0 " << x0 << " " << y0 << std::endl;

    arc->SetLineColor(kBlack);
    arc->SetLineWidth(2);
    arc->SetFillStyle(0);
    arc->Draw();
    TArc *arcI = new TArc(0, 0, theta);
    arcI->SetLineColor(kGreen);
    arcI->SetLineWidth(2);
    arcI->SetFillStyle(0);
    // arcI->Draw();
  }

  { // hp
    auto cdigi = glx_drawDigi();
    cdigi->SetName("hp" + nid);
    glx_canvasAdd(cdigi);
  }

  { // angle
    glx_canvasAdd("angle" + nid, 800, 400);

    if (hAngle[2]->GetMaximum() > 0) hAngle[2]->Scale(1 / hAngle[2]->GetMaximum());
    if (hAngle[3]->GetMaximum() > 0) hAngle[3]->Scale(1 / hAngle[3]->GetMaximum());

    for (int i = 0; i < 5; i++) {
      if (hAngle[i]->GetEntries() < 20) continue;

      int nfound = spect->Search(hAngle[i], 1, "goff", 0.9);
      if (nfound > 0) cherenkovreco[i] = spect->GetPositionX()[0];
      else cherenkovreco[i] = hAngle[i]->GetXaxis()->GetBinCenter(hAngle[i]->GetMaximumBin());
      if (cherenkovreco[i] > 0.85) cherenkovreco[i] = 0.82;

      if (i == 2) fit->SetLineColor(kBlue);
      if (i == 3) fit->SetLineColor(kRed);
      fit->SetParameters(100, cherenkovreco[i], 0.010, 10);
      fit->SetParNames("p0", "#theta_{c}", "#sigma_{c}", "p3", "p4");
      fit->SetParLimits(0, 0.1, 1E6);
      double frange = 3.5 * 0.008;
      fit->SetParLimits(1, cherenkovreco[i] - frange, cherenkovreco[i] + frange);
      fit->SetParLimits(2, 0.004, 0.030); // width
      hAngle[i]->Fit("fgaus", "I", "", cherenkovreco[i] - frange, cherenkovreco[i] + frange);
      hAngle[i]->Fit("fgaus", "M", "", cherenkovreco[i] - frange, cherenkovreco[i] + frange);

      cherenkovreco[i] = fit->GetParameter(1);
      cherenkovreco_err[i] = fit->GetParError(1);
      spr[i] = fit->GetParameter(2);
    }

    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);

    hAngle[2]->GetXaxis()->SetRangeUser(0.7, 0.9);
    // hAngle[2]->GetXaxis()->SetRangeUser(0.72,0.86);
    hAngle[2]->GetYaxis()->SetRangeUser(0, 1.2);
    hAngle[2]->Draw("");
    hAngle[3]->Draw("same");

    hAngleU[0]->SetLineColor(kRed + 2);
    hAngleU[1]->SetLineColor(kYellow + 1);
    hAngleU[2]->SetLineColor(kGreen + 1);
    hAngleU[3]->SetLineColor(kOrange + 1);

    // for(auto i=0; i<4; i++) {
    //   if(hAngleU[i]->GetMaximum()>0) hAngleU[i]->Scale(1/hAngleU[i]->GetMaximum());
    //   hAngleU[i]->Draw("same");
    // }

    // fAngle[3]->Draw("same");
    // fAngle[2]->Draw("same");

    double mm = moms;
    int c[5] = {0, 0, kBlue, kRed, 0};
    for (int i : {2, 3}) {
      mAngle[i] = mangle(i, mm);
      auto l = new TLine(0, 0, 0, 1000);
      l->SetX1(mAngle[i]);
      l->SetX2(mAngle[i]);
      l->SetY1(0);
      l->SetY2(1.2);
      l->SetLineColor(c[i]);
      l->Draw();

      // auto lr = new TLine(0, 0, 0, 1000);
      // lr->SetX1(cherenkovreco[i]);
      // lr->SetX2(cherenkovreco[i]);
      // lr->SetY1(0);
      // lr->SetY2(1.2);
      // lr->SetLineStyle(2);
      // lr->SetLineColor(c[i]);
      // lr->Draw();
    }

    // TLine *line3 = new TLine(0,0,0,1000);
    // line3->SetLineStyle(2);
    // line3->SetX1(mAngle[2]+cut_cangle);
    // line3->SetX2(mAngle[2]+cut_cangle);
    // line3->SetY1(0);
    // line3->SetY2(1.2);
    // line3->SetLineColor(1);
    // line3->Draw();

    // TLine *line4 = new TLine(0,0,0,1000);
    // line4->SetLineStyle(2);
    // line4->SetX1(mAngle[3]-cut_cangle);
    // line4->SetX2(mAngle[3]-cut_cangle);
    // line4->SetY1(0);
    // line4->SetY2(1.2);
    // line4->SetLineColor(1);
    // line4->Draw();

    TLegend *leg = new TLegend(0.1, 0.5, 0.4, 0.85);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hAngle[2], Form("#theta_{c}^{#pi} = %2.4f rad", cherenkovreco[2]), "");
    leg->AddEntry(hAngle[3], Form("#theta_{c}^{K} = %2.4f rad", cherenkovreco[3]), "");
    leg->AddEntry(hAngle[2], Form("#sigma_{c}^{#pi} = %2.1f mrad", spr[2] * 1000), "");
    leg->AddEntry(hAngle[3], Form("#sigma_{c}^{K} = %2.1f mrad", spr[3] * 1000), "");
    leg->Draw();

    TLegend *lnpa = new TLegend(0.7, 0.67, 0.9, 0.85);
    lnpa->SetFillColor(0);
    lnpa->SetFillStyle(0);
    lnpa->SetBorderSize(0);
    lnpa->SetFillStyle(0);
    lnpa->AddEntry(hAngle[2], "pions", "lp");
    lnpa->AddEntry(hAngle[3], "kaons", "lp");
    lnpa->Draw();

    // fAngle[2]->Draw("same");
    // fAngle[3]->Draw("same");
  }

  { // angle diff
    glx_canvasAdd("angle_diff" + nid, 800, 400);

    if (hAngleDiff[2]->GetMaximum() > 0) hAngleDiff[2]->Scale(1 / hAngleDiff[2]->GetMaximum());
    if (hAngleDiff[3]->GetMaximum() > 0) hAngleDiff[3]->Scale(1 / hAngleDiff[3]->GetMaximum());

    for (int i = 0; i < 5; i++) {
      if (hAngleDiff[i]->GetEntries() < 20) continue;

      int nfound = spect->Search(hAngleDiff[i], 1, "goff", 0.9);
      if (nfound > 0) cherenkovreco[i] = spect->GetPositionX()[0];
      else cherenkovreco[i] = hAngleDiff[i]->GetXaxis()->GetBinCenter(hAngleDiff[i]->GetMaximumBin());

      if (i == 2) fit->SetLineColor(kBlue);
      if (i == 3) fit->SetLineColor(kRed);
      fit->SetParameters(100, cherenkovreco[i], 0.008, 10);
      fit->SetParNames("p0", "#theta_{c}", "#sigma_{c}", "p3", "p4");
      fit->SetParLimits(0, 0.1, 1E6);
      double frange = 3.5 * 0.008;
      fit->SetParLimits(1, cherenkovreco[i] - frange, cherenkovreco[i] + frange);
      fit->SetParLimits(2, 0.004, 0.030); // width
      hAngleDiff[i]->Fit("fgaus", "I", "", cherenkovreco[i] - frange, cherenkovreco[i] + frange);
      hAngleDiff[i]->Fit("fgaus", "M", "", cherenkovreco[i] - frange, cherenkovreco[i] + frange);

      cherenkovreco[i] = fit->GetParameter(1);
      cherenkovreco_err[i] = fit->GetParError(1);
      spr[i] = fit->GetParameter(2);
    }

    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);

    hAngleDiff[2]->GetXaxis()->SetRangeUser(-0.1, 0.1);
    hAngleDiff[2]->GetYaxis()->SetRangeUser(0, 1.2);
    hAngleDiff[2]->Draw("");
    hAngleDiff[3]->Draw("same");

    double mm = moms;
    int c[5] = {0, 0, kBlue, kRed, 0};
    for (int i : {2, 3}) {
      mAngle[i] = mangle(i, mm);
      auto l = new TLine(0, 0, 0, 1000);
      l->SetX1(mAngle[i]);
      l->SetX2(mAngle[i]);
      l->SetY1(0);
      l->SetY2(1.2);
      l->SetLineColor(c[i]);
      l->Draw();
    }
 
    TLegend *leg = new TLegend(0.1, 0.5, 0.4, 0.85);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hAngleDiff[2], Form("#theta_{c}^{#pi} = %2.4f rad", cherenkovreco[2]), "");
    leg->AddEntry(hAngleDiff[3], Form("#theta_{c}^{K} = %2.4f rad", cherenkovreco[3]), "");
    leg->AddEntry(hAngleDiff[2], Form("#sigma_{c}^{#pi} = %2.1f mrad", spr[2] * 1000), "");
    leg->AddEntry(hAngleDiff[3], Form("#sigma_{c}^{K} = %2.1f mrad", spr[3] * 1000), "");
    leg->Draw();

    TLegend *lnpa = new TLegend(0.7, 0.67, 0.9, 0.85);
    lnpa->SetFillColor(0);
    lnpa->SetFillStyle(0);
    lnpa->SetBorderSize(0);
    lnpa->SetFillStyle(0);
    lnpa->AddEntry(hAngleDiff[2], "pions", "lp");
    lnpa->AddEntry(hAngleDiff[3], "kaons", "lp");
    lnpa->Draw();
  }

  { // time
    glx_canvasAdd("time_2d" + nid, 800, 400);
    h2Time->Draw("colz");

    glx_canvasAdd("time" + nid, 800, 400);
    hTime->Draw();
    hCalc->SetLineColor(2);
    hCalc->Draw("same");
    TLegend *leg1 = new TLegend(0.5, 0.6, 0.85, 0.80);
    leg1->SetFillColor(0);
    leg1->SetFillStyle(0);
    leg1->SetBorderSize(0);
    leg1->SetFillStyle(0);
    leg1->AddEntry(hTime, "measured", "lp");
    leg1->AddEntry(hCalc, "calculated", "lp");
    leg1->Draw();

    glx_canvasAdd("time_diff" + nid, 800, 400);
    hDiff->SetLineColor(kBlack);
    hDiff->Draw();

    // hDiffT->SetLineColor(kRed+1);
    // hDiffT->Draw("same");
    hDiffD->SetLineColor(kGreen + 2);
    hDiffD->Draw("same");
    hDiffR->SetLineColor(kBlue + 1);
    hDiffR->Draw("same");

    maxTD = hDiffD->GetXaxis()->GetBinCenter(hDiffD->GetMaximumBin());
    maxTR = hDiffR->GetXaxis()->GetBinCenter(hDiffR->GetMaximumBin());
    maxTT = hTime->GetXaxis()->GetBinCenter(hTime->GetMaximumBin());

    // maxTD = glx_fit(hDiffD, 2.2).X();
    // maxTR = glx_fit(hDiffR, 2.2).X();

    TLine *line = new TLine(0, 0, 0, 1000);
    line->SetLineStyle(2);
    line->SetX1(-cut_tdiff);
    line->SetX2(-cut_tdiff);
    line->SetY1(0);
    line->SetY2(hDiff->GetMaximum() + 0.05 * hDiff->GetMaximum());
    line->SetLineColor(1);
    // line->Draw();

    TLine *line2 = new TLine(0, 0, 0, 1000);
    line2->SetLineStyle(2);
    line2->SetX1(cut_tdiff);
    line2->SetX2(cut_tdiff);
    line2->SetY1(0);
    line2->SetY2(hDiff->GetMaximum() + 0.05 * hDiff->GetMaximum());
    line2->SetLineColor(1);
    // line2->Draw();

    TLegend *leg2 = new TLegend(0.6, 0.57, 0.9, 0.85);
    leg2->SetFillColor(0);
    leg2->SetFillStyle(0);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->AddEntry(hDiff, "all", "lp");
    // leg2->AddEntry(hDiffT,"MC path in EV","lp");
    // leg2->AddEntry(hDiffD,"MC path in EV for direct photons","lp");
    // leg2->AddEntry(hDiffR,"MC path in EV for reflected photons","lp");
    leg2->AddEntry(hDiffD, "direct photons", "lp");
    leg2->AddEntry(hDiffR, "reflected photons", "lp");

    leg2->Draw();
  }

  { // yield
    glx_canvasAdd("nph" + nid, 800, 400);

    if (hNph[2]->GetEntries() > 50) {
      nph = glx_fit(hNph[2], 50, 30, 50).X();
      nphm = hNph[2]->GetMean();
      auto rfit = hNph[2]->GetFunction("glx_gaust");
      if (rfit) rfit->SetLineColor(kBlue + 1);
      hNph[2]->SetLineColor(kBlue);

      glx_fit(hNph[3], 40, 100, 40).X();
      rfit = hNph[3]->GetFunction("glx_gaust");
      if (rfit) rfit->SetLineColor(kRed + 1);
      hNph[3]->SetLineColor(kRed);

      hNph[2]->Draw();
      hNph[3]->Draw("same");
    }
    // hNphC->SetLineColor(kBlack);
    // hNphC->Draw("same");

    TLegend *lnph = new TLegend(0.6, 0.65, 0.9, 0.85);
    lnph->SetFillColor(0);
    lnph->SetFillStyle(0);
    lnph->SetBorderSize(0);
    lnph->SetFillStyle(0);
    // lnph->AddEntry(hNphC,"simulated","lp");
    lnph->AddEntry(hNph[2], "pions", "lp");
    lnph->AddEntry(hNph[3], "kaons", "lp");
    lnph->Draw();
  }

  { // LH
    glx_canvasAdd("lndiff" + nid, 800, 400);
    TF1 *ff;
    double m1 = 0, m2 = 0, s1 = 0, s2 = 0;
    if (hLnDiff[3]->GetEntries() > 20) {
      hLnDiff[3]->Fit("gaus", "S");
      ff = hLnDiff[3]->GetFunction("gaus");
      ff->SetLineColor(1);
      m1 = ff->GetParameter(1);
      s1 = ff->GetParameter(2);
    }
    if (hLnDiff[2]->GetEntries() > 20) {
      hLnDiff[2]->Fit("gaus", "S");
      ff = hLnDiff[2]->GetFunction("gaus");
      ff->SetLineColor(1);
      m2 = ff->GetParameter(1);
      s2 = ff->GetParameter(2);
    }
    if (s1 > 0 && s2 > 0) sep = (fabs(m2 - m1)) / (0.5 * (s1 + s2));

    hLnDiff[2]->SetTitle(Form("sep = %2.2f s.d.", sep));
    hLnDiff[2]->Draw();
    hLnDiff[3]->Draw("same");

    gStyle->SetOptTitle(1);
    TLegend *lnpl = new TLegend(0.7, 0.67, 0.9, 0.85);
    lnpl->SetFillColor(0);
    lnpl->SetFillStyle(0);
    lnpl->SetBorderSize(0);
    lnpl->SetFillStyle(0);
    lnpl->AddEntry(hLnDiff[2], "pions", "lp");
    lnpl->AddEntry(hLnDiff[3], "kaons", "lp");
    lnpl->Draw();

    std::cout << "separation = " << sep << "  nph = " << nph << std::endl;
    std::cout << "maxTD " << maxTD << "  maxTR " << maxTR << std::endl;
    glx_separation(hLnDiff[2], hLnDiff[3]);
  }

  {
    // kinematics

    glx_canvasAdd("hKin"+nid,800,400);
    hrho->Draw();
    hphi->SetLineColor(kRed);
    hphi->Draw("same");

    //glx_canvasAdd("hCMom"+nid,800,400);
    //hCMom[2]->SetMarkerColor(kBlue);
    //hCMom[2]->Draw();
    //hCMom[3]->SetMarkerColor(kRed);
    //hCMom[3]->Draw("same");
    //hCMom[4]->Draw("colz same");
  }

  { // wall
    glx_canvasAdd("wall" + nid, 800, 800);
    hWall->Draw("colz");
    double pos[] = {0.382, 0.185, 0.618, 0.815};
    double w = 0.085;
    TBox *pbox[4];
    for (int i = 0; i < 4; i++) {
      pbox[i] = new TBox(0.1, pos[i] - w, 0.9, pos[i] + w);
      pbox[i]->Draw();
    }
  }

  {
    // wall
    // glx_canvasAdd("wall_pos",800,800);
    // for(int i=0; i<=(hWallPos->GetNbinsX()+1)*(hWallPos->GetNbinsY()+1); i++){
    //   double c = hWallPos->GetBinContent(i);
    //   if(c!=0 && wallb[i]!=0) hWallPos->SetBinContent(i,c/wallb[i]);
    //   else  hWallPos->SetBinContent(i,-100);
    // }
    // hWallPos->SetMaximum(1);
    // hWallPos->SetMinimum(-1);
    // hWallPos->Draw("colz");
  }

  { // chromatic corrections
    glx_canvasAdd("chrom" + nid, 800, 400);
    hChrom->Draw("colz");
  }

  { // track multiplicity
    glx_canvasAdd("mult" + nid, 800, 400);
    hMult->Draw();
  }

  glx_canvasSave("data/reco_lut_02_scan_sel_05_barscan_lr", 2);

  { // tree
    TString out = glx_savepath + "/res" + nid + ".root";
    TFile fc(out, "recreate");
    TTree *tc = new TTree("reco", "reco");
    // tc->Branch("theta",&theta,"theta/D");
    // tc->Branch("phi",&phi,"prt_phi/D");
    tc->Branch("sep", &sep, "sep/D");
    tc->Branch("esep", &esep, "esep/D");
    tc->Branch("moms", &moms, "prt_mom/D");
    tc->Branch("bar", &ibar, "ibar/I");
    tc->Branch("bin", &ibin, "ibin/I");
    tc->Branch("nph", &nph, "nph/D");
    tc->Branch("nphm", &nphm, "nphm/D");
    tc->Branch("spr", &spr[3], "spr/D");
    tc->Branch("maxTD", &maxTD, "maxTD/D");
    tc->Branch("maxTR", &maxTR, "maxTR/D");
    tc->Branch("maxTT", &maxTT, "maxTT/D");
    tc->Branch("scan", &scan, "scan/D");
    tc->Branch("dx", &dx, "dx/D");
    tc->Branch("dy", &dy, "dy/D");

    tc->Branch("cangle2", &cherenkovreco[2], "cangle2/D");
    tc->Branch("cangle3", &cherenkovreco[3], "cangle3/D");
    tc->Branch("cangle4", &cherenkovreco[4], "cangle4/D");
    tc->Branch("cangle2_err", &cherenkovreco_err[2], "cangle2_err/D");
    tc->Branch("cangle3_err", &cherenkovreco_err[3], "cangle3_err/D");
    tc->Branch("cangle4_err", &cherenkovreco_err[4], "cangle4_err/D");

    tc->Branch("spr2", &spr[2], "spr2/D");
    tc->Branch("spr3", &spr[3], "spr3/D");
    tc->Branch("spr4", &spr[4], "spr4/D");

    tc->Fill();
    tc->Write();
    fc.Write();
    fc.Close();

    std::cout << "tree saved in " << out << std::endl;
  }
#endif
}

int main(int nargs, char* argv[]) {
  reco_lut_02("/volatile/halld/home/jrsteven/RunPeriod-2019-11/recon/ver01_pass03/merged/hd_root/hd_root_072646.root","/work/halld/home/gxproj7/RunPeriod-2019-11/dircsim-2019_11-ver03/lut/hd_root.root", 32, 3, 3.5, 0, 0.0, 0, 0);
}
