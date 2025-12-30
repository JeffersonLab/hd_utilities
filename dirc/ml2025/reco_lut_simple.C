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

TF1 *n_fused_silica_nm = new TF1(
    "n_fused_silica_nm",
    "sqrt(1"
    "+ (0.6961663*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.0684043*0.0684043)"
    "+ (0.4079426*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.1162414*0.1162414)"
    "+ (0.8974794*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 9.896161*9.896161))",
    200.0, 800.0
);

TF1 *ng_fused_silica_nm = new TF1(
  "ng_fused_silica_nm",

  // n_g = n - x * dn/dx   (x in nm)
  "("
    // n(x)
    "sqrt(1"
      "+ (0.6961663*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.0684043*0.0684043)"
      "+ (0.4079426*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.1162414*0.1162414)"
      "+ (0.8974794*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 9.896161*9.896161)"
    ")"

    // - x * dn/dx
    " - x * ("

      // dn/dx = (1/(2*n)) * d(n^2)/dx
      // d(n^2)/dx = (1/1000) * d(n^2)/dλ_um
      // and d/dλ_um [ B*λ^2/(λ^2-C) ] = -2*B*C*λ / (λ^2 - C)^2
      "("
        "(-2*0.6961663*(0.0684043*0.0684043)*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 0.0684043*0.0684043, 2)"
        "+(-2*0.4079426*(0.1162414*0.1162414)*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 0.1162414*0.1162414, 2)"
        "+(-2*0.8974794*(9.896161*9.896161)*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 9.896161*9.896161, 2)"
      ")"
      " / (2*1000.0*sqrt(1"
        "+ (0.6961663*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.0684043*0.0684043)"
        "+ (0.4079426*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.1162414*0.1162414)"
        "+ (0.8974794*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 9.896161*9.896161)"
      "))"

    ")"
  ")",
  200.0, 800.0
);

TF1 *ng_water_nm = new TF1(
  "ng_water_nm",

  "("
    // n(x)
    "sqrt(1"
      "+ (0.5666959820*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.005084151894)"
      "+ (0.1731900098*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.01818488474)"
      "+ (0.02095951857*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.02625439472)"
      "+ (0.1125228406*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 10.73842352)"
    ")"

    // − x * dn/dx
    " - x * ("
      "("
        "(-2*0.5666959820*0.005084151894*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 0.005084151894, 2)"
        "+(-2*0.1731900098*0.01818488474*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 0.01818488474, 2)"
        "+(-2*0.02095951857*0.02625439472*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 0.02625439472, 2)"
        "+(-2*0.1125228406*10.73842352*(x/1000.0)) / pow((x/1000.0)*(x/1000.0) - 10.73842352, 2)"
      ")"
      " / (2*1000.0*sqrt(1"
        "+ (0.5666959820*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.005084151894)"
        "+ (0.1731900098*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.01818488474)"
        "+ (0.02095951857*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 0.02625439472)"
        "+ (0.1125228406*(x/1000.0)*(x/1000.0))/((x/1000.0)*(x/1000.0) - 10.73842352)"
      "))"
    ")"
  ")",
  200.0, 800.0
);

double mangle(int pid, double m, double nindex) {
  return acos(sqrt(m * m + glx_mass[pid] * glx_mass[pid]) / m / nindex);
}

void reco_lut_simple(TString infile = "pip_p3_theta4.root", TString inlut = "/work/halld/home/jrsteven/RunPeriod-2019-11/dircsim-2019_11-ver05/lut/lut_all_avr.root",
                 int ibar = 7, int ibin = -1, double moms = 3.0, int ivar=0) {

  int indd = -1;
  const int nodes = 5184;
  int glx_nch = 5184;
  const int luts = 24;

  double c = 29.9792458; // speed of light in vacuum (cm/ns)
  double average_wavelength = 394; // nm : 412 is average, 394 gives better performance?
  double nindex = n_fused_silica_nm->Eval(average_wavelength); // fused silica refractive index
  double vg_light_fused_silica = c / ng_fused_silica_nm->Eval(average_wavelength); // group velocity in fused silica (cm/ns)

  // use truth hit information (wavelength, pathId, nreflections)
  bool truthHitSim = false;
  
  // apply correction factors to simulation (e.g. hit detection efficiency)
  bool sim = true;

  // only use direct or reflected photons
  bool onlyDirect = false;
  bool onlyReflected = false;

  // only use same pathid from LUT for simulation
  bool onlySamePath = false;

  TSystemDirectory directory(infile, infile);
  TList *files=directory.GetListOfFiles();
  if (infile.Contains("hd_root_gen")) sim = true;
  if (files) infile = infile+"*.root";
  cout<<infile.Data()<<endl;

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

  TString stdiff = ";t_{measured}-t_{calculated} [ns];entries [#]";
  TString scdiff = ";#theta_{C reco} - #theta_{C expected} [mrad];entries [#]";

  double radiatorL = 489.8; // nominal = 4*122.5;
  double barend = -292.96; // nominal = 4*122.5-196.0;

  double minChangle = 0.6;
  double maxChangle = 0.9;
  double sum1, sum2, noise = 0.2; // 0.4
  // cuts
  double cut_cangle = 3.5 * 0.008; // 3.5
  double cut_tdiff = 0.5;

  double sigma[] = {0.01, 0.01, 0.0073, 0.0073, 0.01};

  double criticalAngle = asin(1.00028 / nindex);
  double evtime, luttheta, tangle, lenx;
  int64_t pathid;
  TVector3 posInBar, posInBar_true, momInBar, dir, dird, ldir;

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
  TH1F *hLambda = new TH1F("hLambda", ";wavelength [nm];entries [#]", 300, 200, 800);
  TH1F *hLambdaD = new TH1F("hLambdaD", ";wavelength [nm];entries [#]", 300, 200, 800);
  TH1F *hLambdaR = new TH1F("hLambdaR", ";wavelength [nm];entries [#]", 300, 200, 800);
  TH2F *hDiffDVsLambda = new TH2F("hDiffDVsLambda",";wavelength [nm]; t_{measured}-t_{calculated} [ns]", 300, 200, 800, 400, -10, 10);
  TH2F *hDiffRVsLambda = new TH2F("hDiffRVsLambda",";wavelength [nm]; t_{measured}-t_{calculated} [ns]", 300, 200, 800, 400, -10, 10);
  TH2F *hDiffDVsNreflections = new TH2F("hDiffDVsNreflections","; # reflections; t_{measured}-t_{calculated} [ns]", 20, 0, 20, 400, -10, 10);
  TH2F *hDiffRVsNreflections = new TH2F("hDiffRVsNreflections","; # reflections; t_{measured}-t_{calculated} [ns]", 20, 0, 20, 400, -10, 10);
  TH1F *hTime = new TH1F("hTime", ";propagation time [ns];entries [#]", 2000, 0, 200);
  TH1F *hCalc = new TH1F("hCalc", ";calculated time [ns];entries [#]", 2000, 0, 200);
  TH1F *hCalcBar = new TH1F("hCalcBar", ";propagation time in bar [ns];entries [#]", 2000, 0, 200);
  TH1F *hNphC = new TH1F("hNphC", ";detected photons [#];entries [#]", 150, 0, 150);
  TH2F *hCMom[5];
  TH2F *h2Time = new TH2F("h2Time", ";propagation time [ns];t_{measured}-t_{calculated} [ns];", 500, 0, 100, 200, -10, 10);
  TH2F *hWall = new TH2F("hWall", ";x [cm];y [cm]", 400, -100, 100, 400, -100, 100);
  TH2F *hWallPos = new TH2F("hWalPos", ";x [cm];y [cm]", 50, -100, 100, 50, -100, 100);
  int wallb[5000] = {0};
  TH2F *hSpr = new TH2F("hSpr", ";x [cm];y [cm]", 400, -100, 100, 400, -100, 100);
  TH2F *hChrom = new TH2F("hChrom", ";t_{measured}-t_{calculated} [ns]; #Delta#theta_{C} [mrad]", 100, -2, 2, 60, -30, 30);
  TH2F *hChromLambda = new TH2F("hChromLambda", ";wavelength [nm]; #Delta#theta_{C} [mrad]", 300, 200, 800, 60, -30, 30);
  TH1F *hMult = new TH1F("hmult", "; track multiplicity [#];entries [#]", 8, 0, 8);
  TH1I *hPathIdD = new TH1I("hPathIdD", ";PathId; entries [#]", 100000, -0.5, 100000);
  TH1I *hPathIdR = new TH1I("hPathIdR", ";PathId; entries [#]", 100000, -0.5, 100000);

  hDiff->SetMinimum(0);
  TGaxis::SetMaxDigits(3);

  for (int i = 0; i < 5; i++) {
    double momentum = 4;
    hAngle[i] = new TH1F(Form("hAngle_%d", i), ";#theta_{C} [rad];entries/N_{max} [#]", 150, 0.6, 0.9);
    hAngleDiff[i] = new TH1F(Form("hAngleDiff_%d", i), ";#Delta#theta_{C} [rad];entries/N_{max} [#]", 150, -0.15, 0.15);
    hAngleU[i] = new TH1F(Form("hAngleu_%d", i), ";#theta_{C} [rad];entries/N_{max} [#]", 150, 0.6, 0.9);
    hCMom[i] = new TH2F(Form("cmom_%d", i), "hcmom", 1000, 0, 10, 500, 0.6, 0.9);
    hNph[i] = new TH1F(Form("hNph_%d", i), ";detected photons [#];entries [#]", 80, 0, 80);
    mAngle[i] = acos(sqrt(momentum * momentum + glx_mass[i] * glx_mass[i]) / momentum / nindex);
    fAngle[i] = new TF1(Form("fAngle_%d", i), "[0]*exp(-0.5*((x-[1])/[2])*(x-[1])/[2])", 0.7, 0.9);
    fAngle[i]->SetParameter(0, 1);         // const
    fAngle[i]->SetParameter(1, mAngle[i]); // mean
    fAngle[i]->SetParameter(2, sigma[i]);  // sigma
    hAngle[i]->SetMarkerStyle(20); hAngleDiff[i]->SetMarkerStyle(20);
    hAngle[i]->SetMarkerSize(0.8); hAngleDiff[i]->SetMarkerSize(0.8);
    if (moms < 4)
      hLnDiff[i] = new TH1F(Form("hLnDiff_%d", i), ";ln L(#pi) - ln L(K);entries [#]", 80, -160, 160);
    else
      hLnDiff[i] = new TH1F(Form("hLnDiff_%d", i), ";ln L(#pi) - ln L(K);entries [#]", 80, -100, 100);
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
  TH1F *hphi = new TH1F("hphi", "hphi; Mass [GeV/c];events [#]", 5000, 0, 1.5);
  TH1F *hrho = new TH1F("hrho", "hphi; Mass [GeV/c];events [#]", 5000, 0, 1.5);

  TGraph cagr;
  double bartime, luttime, diftime, adiff, len, leny, lenz;
  double dibin = -100 + ibin * 20 + 10;

  TCut cut = "";
  if (ibar > -1) cut += Form("(DrcEvent.fId == %d)", ibar);
  if (ibin > -1) cut += Form("fabs(DrcEvent.fPosition.fX-%f)<10", dibin); // 10
  cut.Print();

  if(!glx_initc(infile,1,"data/reco_lut")) return;

  for (int e = 0; e < glx_ch->GetEntries(); e++){
    glx_ch->GetEntry(e);
    //cout<<"event: "<<e<<endl;
    
    for (int t = 0; t < glx_events->GetEntriesFast(); t++){
      glx_nextEventc(e,t,1000);

      if(e%1000 == 0)
	      cout<<"Particle count "<<count[2]<<" "<<count[3]<<endl;

      int pdgId = glx_findPdgId(glx_event->GetPdg());
      int bar = glx_event->GetId();
      int lid = bar;
      int opbox = 0;
      double time0 = glx_event->GetTime();

      posInBar = glx_event->GetPosition();
      posInBar_true = glx_event->GetPosition_Truth();

      momInBar = glx_event->GetMomentum();
      double momentum = momInBar.Mag();
      int bin = (100 + posInBar.X()) / 200. * nbins;
     
      // selection
      if (bar != ibar && ibar > -1) continue;
      if (fabs(posInBar.X() - dibin) > 10 && ibin > -1) continue;

      if (bar > 23) {
        opbox = 1;
        barend *= -1;
      }

      hWall->Fill(posInBar.X(), posInBar.Y());

      cz = momInBar.Unit();
      
      for (int p = 0; p < 5; p++) {
        mAngle[p] = mangle(p, momentum, nindex);
        fAngle[p]->SetParameter(1, mAngle[p]); // mean
        fAngle[p]->SetParameter(2, sigma[p]);
      }

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
        
        double hitWavelength = average_wavelength; // nm
        int hitNreflections = -1;
        int hitPathId = -1; 
        if(truthHitSim) { // hit information only available in simulation
          double hitEnergy = hit.GetEnergy(); // GeV
          hitWavelength = 1239.84 / (hitEnergy * 1e9); // nm
          hitNreflections = hit.GetNreflections();
          hitPathId = hit.GetPathId();

          //if(hitNreflections != 3 || hitPathId != 4920) continue; // 4910 4920
          //cout<<"wavelength: "<<hitWavelength<<" nindex: "<<n_fused_silica_nm->Eval(hitWavelength)<<" vlight: "<<vlight<<" Nreflections: "<<hitNreflections<<" PathId: "<<hitPathId<<endl;
        
          // apply wavelength dependent refractive index and group velocity
          nindex = n_fused_silica_nm->Eval(hitWavelength); // fused silica refractive index
          vg_light_fused_silica = c / ng_fused_silica_nm->Eval(hitWavelength); // cm/ns
        }

        nphc++;
        bool reflected = hitTime > 44;

        if(onlyDirect && reflected) continue;
        if(onlyReflected && !reflected) continue;

        lenx = fabs(barend - posInBar.X());
        double rlenx = 2 * radiatorL - lenx;
        double dlenx = lenx;
        if (reflected) lenx = 2 * radiatorL - lenx;

        bool isGood(false);
        double p1, p2;

        for (int i = 0; i < lutNode[lid][ch]->Entries(); i++) {
          dird = lutNode[lid][ch]->GetEntry(i);
          evtime = lutNode[lid][ch]->GetTime(i);
          pathid = lutNode[lid][ch]->GetPathId(i);
          int nrefl = lutNode[lid][ch]->GetNRefl(i);
          double weight = 1.5; // lutNode[lid][ch]->GetWeight(i);

          TString spath = Form("%ld", pathid);
          // // if (!spath.BeginsWith("")) continue;
          if (spath.Contains("8")) continue;
          if (spath.Contains("7")) continue;

          bool samepath(false);
          if (fabs(pathid - hit.GetPathId()) < 0.0001) samepath = true;
          p1 = hit.GetPathId();
          if (samepath) p2 = pathid;

          if(onlySamePath && !samepath) continue; // only use same pathid from LUT for simulation

          if (opbox == 1) {
            ldir = dird;
            ldir.RotateY(-TMath::PiOver2());
          } else {
            ldir = dird;
            ldir.RotateY(TMath::PiOver2());
            ldir.RotateX(-TMath::Pi());
          }

          double lphi = ldir.Phi();
          double ltheta = ldir.Theta();
          if (lphi < 0) lphi = TMath::TwoPi() + lphi;
          if (ltheta > TMath::PiOver2()) ltheta = TMath::Pi() - ltheta;

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
              bartime = lenx / cos(luttheta) / vg_light_fused_silica;
              double obtime = evtime;
              luttime = bartime + obtime;

              diftime = hitTime - luttime;

              tangle = momInBar.Angle(dir);

              if (fabs(tangle - mAngle[2]) < cut_cangle || fabs(tangle - mAngle[3]) < cut_cangle) {
		            hTime->Fill(hitTime);
		            hCalc->Fill(luttime);
                hCalcBar->Fill(bartime);
                hDiff->Fill(diftime);
                hDiffT->Fill(diftime);
                if (r) hDiffR->Fill(diftime);
                else hDiffD->Fill(diftime);
                
                if(truthHitSim) {
                  hLambda->Fill(hitWavelength);
                  if (r) {
                    hLambdaR->Fill(hitWavelength);
                    hDiffRVsLambda->Fill(hitWavelength, diftime);
                    hDiffRVsNreflections->Fill(hitNreflections, diftime);
                    hPathIdR->Fill(hitPathId);
                  }
                  else {
                    hLambdaD->Fill(hitWavelength);
                    hDiffDVsLambda->Fill(hitWavelength, diftime);
                    hDiffDVsNreflections->Fill(hitNreflections, diftime);
                    hPathIdD->Fill(hitPathId);
                  }
                }
              }

              if (fabs(diftime) > cut_tdiff + luttime * 0.025) continue;

              adiff = (tangle - mangle(pdgId, momentum, nindex)) * 1000;

              hChrom->Fill(diftime, adiff);
              hChromLambda->Fill(hitWavelength, adiff);

              if (pdgId == 2) hAngleU[u]->Fill(tangle, weight);
              hAngle[pdgId]->Fill(tangle, weight);
	            hAngleDiff[pdgId]->Fill(adiff/1000., weight);

              if (fabs(tangle - mAngle[2]) > 1 * cut_cangle &&
                  fabs(tangle - mAngle[3]) > 1 * cut_cangle)
                continue; // 8

              isGood = true;

              h2Time->Fill(hitTime, diftime);

              sum1 += weight * TMath::Log(fAngle[2]->Eval(tangle) + noise);
              sum2 += weight * TMath::Log(fAngle[3]->Eval(tangle) + noise);
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
      }

      if (goodevt) evtcount++;

      if (nph < 10) continue;
      hNph[pdgId]->Fill(nph);
      hNphC->Fill(nphc);

      double sum = sum1 - sum2;
      hLnDiff[pdgId]->Fill(sum);

      count[pdgId]++;
    }

    // reset nindex to average value after each track if recalculating per-hit nindex
    if(truthHitSim) nindex = n_fused_silica_nm->Eval(average_wavelength);
  }

  cout<<evtcount<<" events"<<endl;

  if (evtcount > 0) {
    for (int i = 0; i < glx_nch; i++) {
      int pmt = i / 64;
      int pix = i % 64;
      double rel = glx_hdigi[pmt]->GetBinContent(pix % 8 + 1, pix / 8 + 1) / (double)evtcount;
      glx_hdigi[pmt]->SetBinContent(pix % 8 + 1, pix / 8 + 1, rel);
    }
  }

  TString nid = Form("_%d_%d_%2.1f", ibar, ibin, moms);
  if (sim) nid = "_simM" + nid;
  nid.ReplaceAll("-", "m");
  nid.ReplaceAll(".", "d");

  // summary information
  double maxTD, maxTR, maxTT, sep = 0, esep = 0;
  double nph[5], nphm[5], cherenkovreco[5], cherenkovreco_err[5], spr[5], spr_err[5];

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
      spr_err[i] = fit->GetParError(2);
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

    double mm = moms;
    int c[5] = {0, 0, kBlue, kRed, 0};
    for (int i : {2, 3}) {
      mAngle[i] = mangle(i, mm, nindex);
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
      spr_err[i] = fit->GetParError(2);
    }

    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);

    hAngleDiff[2]->GetXaxis()->SetRangeUser(-0.1, 0.1);
    hAngleDiff[2]->GetYaxis()->SetRangeUser(0, 1.2);
    hAngleDiff[2]->Draw("");
    hAngleDiff[3]->Draw("same");
 
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
    //hCalcBar->SetLineColor(4);
    //hCalcBar->Draw("same");
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
      nph[2] = glx_fit(hNph[2], 50, 20, 50).X();
      nphm[2] = hNph[2]->GetMean();
      auto rfit = hNph[2]->GetFunction("glx_gaust");
      if (rfit) rfit->SetLineColor(kBlue + 1);
      hNph[2]->SetLineColor(kBlue);

      nph[3] = glx_fit(hNph[3], 50, 20, 50).X();
      nphm[3] = hNph[3]->GetMean();
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
    lnph->AddEntry(hNph[2], Form("pions: N=%.1f", nphm[2]), "lp");
    lnph->AddEntry(hNph[3], Form("kaons: N=%.1f", nphm[3]), "lp");
    lnph->Draw();
  }

  { // LH
    glx_canvasAdd("lndiff" + nid, 800, 400);
    TF1 *ff;
    double m1 = 0, m2 = 0, s1 = 0, s2 = 0;
    double m1_err = 0, m2_err = 0, s1_err = 0, s2_err = 0;
    if (hLnDiff[3]->GetEntries() > 20) {
      hLnDiff[3]->Fit("gaus", "S");
      ff = hLnDiff[3]->GetFunction("gaus");
      ff->SetLineColor(1);
      m1 = ff->GetParameter(1);
      s1 = ff->GetParameter(2);
      m1_err = ff->GetParError(1);
      s1_err = ff->GetParError(2);
    }
    if (hLnDiff[2]->GetEntries() > 20) {
      hLnDiff[2]->Fit("gaus", "S");
      ff = hLnDiff[2]->GetFunction("gaus");
      ff->SetLineColor(1);
      m2 = ff->GetParameter(1);
      s2 = ff->GetParameter(2);
      m2_err = ff->GetParError(1);
      s2_err = ff->GetParError(2);
    }
    if (s1 > 0 && s2 > 0) {
      sep = (fabs(m2 - m1)) / (0.5 * (s1 + s2));
      esep = sqrt(m1_err * m1_err + m2_err * m2_err) / (0.5 * (s1 + s2)) +
             (fabs(m2 - m1) / (0.5 * (s1 + s2) * (0.5 * (s1 + s2)))) *
                 sqrt(s1_err * s1_err + s2_err * s2_err);
    }

    hLnDiff[2]->SetTitle(Form("sep = %2.2f +/- %2.2f", sep, esep));
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

    std::cout << "separation = " << sep << "  nph = " << nphm[2] << ", " << nphm[3] << std::endl;
    std::cout << "maxTD " << maxTD << "  maxTR " << maxTR << std::endl;
    glx_separation(hLnDiff[2], hLnDiff[3]);
  }

  {
    // kinematics

    glx_canvasAdd("hKin"+nid,800,400);
    hrho->Draw();
    hphi->SetLineColor(kRed);
    hphi->Draw("same");
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

  { // chromatic corrections
    glx_canvasAdd("chrom" + nid, 800, 400);
    hChrom->Draw("colz");
    glx_canvasAdd("chromLambda" + nid, 800, 400);
    hChromLambda->Draw("colz");
  }

  if(truthHitSim)
  { // photon wavelength
    glx_canvasAdd("lambda" + nid, 800, 400);
    hLambda->Draw();
    hLambda->SetLineColor(kBlack);
    hLambda->Draw();

    hLambdaD->SetLineColor(kGreen + 2);
    hLambdaD->Draw("same");
    hLambdaR->SetLineColor(kBlue + 1);
    hLambdaR->Draw("same");
    cout<<"Mean lambda all: "<<hLambda->GetMean()<<endl;
    cout<<"Mean lambda direct: "<<hLambdaD->GetMean()<<endl;
    cout<<"Mean lambda reflected: "<<hLambdaR->GetMean()<<endl;

    glx_canvasAdd("lambda_time_diff" + nid, 800, 400);
    hDiffRVsLambda->Draw("colz");
    glx_canvasAdd("nreflections_time_diff" + nid, 800, 400);
    hDiffRVsNreflections->Draw("colz");
    glx_canvasAdd("pathid" + nid, 800, 400);
    hPathIdR->Draw();
  }

  glx_canvasSave("data/reco_lut_02_scan_sel_05_barscan_lr", 2);

  // write summary information to text file
  ofstream outfile;
  outfile.open("summary.txt", ios::out);
  outfile << "ibar: " << ibar << "\n";
  outfile << "ibin: " << ibin << "\n";
  outfile << "momentum: " << moms << " GeV/c\n";
  outfile << "nph pi mean: " << nphm[2] << "\n";
  outfile << "nph K mean: " << nphm[3] << "\n";
  outfile << "spr pi (mrad): " << spr[2] * 1000 << "\n";
  outfile << "spr pi error (mrad): " << spr_err[2] * 1000 << "\n";
  outfile << "spr K (mrad): " << spr[3] * 1000 << "\n";
  outfile << "spr K error (mrad): " << spr_err[3] * 1000 << "\n";
  outfile << "separation (sigma): " << sep << "\n";
  outfile << "separation error (sigma): " << esep << "\n";
  outfile.close();

  // write summary information to CSV file (append mode for many variations)
  ofstream csvfile;
  csvfile.open("summary_results.csv", ios::app);
  // Check if file is empty to write header
  ifstream check("summary_results.csv");
  bool is_empty = check.peek() == EOF;
  check.close();
  if (is_empty) {
    csvfile << "ivar,ibar,ibin,momentum_GeV,nph_pi,nph_K,spr_pi,spr_pi_err,spr_K,spr_K_err,sep,sep_err\n";
  }
  csvfile << ivar << "," << ibar << "," << ibin << "," << moms << "," << nphm[2] << "," << nphm[3] << ","
          << spr[2] * 1000 << "," << spr_err[2] * 1000 << "," << spr[3] * 1000 << ","
          << spr_err[3] * 1000 << "," << sep << "," << esep << "\n";
  csvfile.close();
}
