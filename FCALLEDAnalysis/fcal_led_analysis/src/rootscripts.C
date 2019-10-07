const Float_t ranges[5][2] = {
  {1000, 3225},
  {3225, 4500},
  {4500, 6250},
  {6250, 8375},
  {8375, 15000}
};

#include "DFCALGeometry.hpp"

const char *GetPath(Int_t col)
{
  switch(col) {
    case 0:
      return "data/chain_blue_low.root";
    case 1:
      return "data/chain_violet_low.root";
    case 2:
      return "data/chain_green.root";
    case 3:
      return "data/chain_violet_high.root";
    case 4:
      return "data/chain_blue_high.root";
    default:
      return "";
  }

  return "";
}

TH2F *DetectorGraph(const char *pth, Int_t col, Float_t min=0, Float_t max = 117)
{
  if (!pth || col < 0 || col > 5) {
    return nullptr;
  }

  TH2F *hist = new TH2F(
    "detectormap",
    "Detector Results over a Run",
    DFCALGeometry::kBlocksWide, 0, DFCALGeometry::kBlocksWide,
    DFCALGeometry::kBlocksTall, 0, DFCALGeometry::kBlocksTall
  );

  TFile *f = new TFile(pth);
  if (!f) {
    delete hist;
    return nullptr;
  }

  TTree *t = (TTree *)f->Get("fcalBlockHits");
  if (!t) {
    delete hist;
    f->Close();
    return nullptr;
  }

  Long64_t entr = t->GetEntries();

  Int_t chan[5000];
  Float_t etot;
  Int_t nhits;
  Float_t eval[5000];

  t->SetBranchAddress("nHits", &nhits);
  t->SetBranchAddress("eTot", &etot);
  t->SetBranchAddress("chan", &chan[0]);
  t->SetBranchAddress("E", &eval[0]);

  DFCALGeometry dfcal{};

  for (Long64_t i=0; i < entr; ++i) {
    t->GetEntry(i);
    if (etot < ranges[col][0] || etot > ranges[col][1]) {
      continue;
    }

    for (Int_t n=0; n < nhits; ++n) {
      Float_t dist = dfcal.positionOnFace((int)chan[n]).Mod();
      if (dist < min || dist > max) {
        continue;
      }

      Float_t row = dfcal.row((int)chan[n]);
      Float_t col = dfcal.column((int)chan[n]);
      hist->Fill(col, row, eval[n]);
    }
  }

  f->Close();
  hist->SetStats(kFALSE);
  return hist;
}

Bool_t RunExists(Int_t col, Int_t run, const char *tree="fcalRingRatios")
{
  if (col < 0 || col > 5) {
    return kFALSE;
  }

  const char *pth = GetPath(col);

  TFile *f = new TFile(pth);
  if (!f) {
    return kFALSE;
  }

  TTree *t = (TTree *)f->Get(tree);
  if (!t) {
    f->Close();
    return kFALSE;
  }

  Long64_t entr = t->GetEntries();

  Int_t r = 0;
  t->SetBranchAddress("run", &r);

  for (Long64_t i=0; i < entr; ++i) {
    t->GetEntry(i);
    if (r == run) {
      f->Close();
      return kTRUE;
    }
  }

  f->Close();
  return kFALSE;
}

TGraphErrors * MakeGraph(TFile *f, Int_t ring)
{
  if (f == nullptr)
    return nullptr;

  TTree *t = (TTree *)f->Get("fcalRingChains");
  if (t == nullptr) {
    return nullptr;
  }

  Int_t numrings;
  Int_t run;
  Int_t rid[10];
  Float_t rat[10];
  Float_t drms[10];

  t->SetBranchAddress("chain", &run);
  t->SetBranchAddress("ringnum", &numrings);
  t->SetBranchAddress("ringid", &rid[0]);
  t->SetBranchAddress("ringavg", &rat[0]);
  t->SetBranchAddress("ringrms", &drms[0]);

  std::vector<Double_t> x = std::vector<Double_t>(0);
  std::vector<Double_t> y = std::vector<Double_t>(0);
  std::vector<Double_t> rms = std::vector<Double_t>(0);
  std::vector<Double_t> zeros = std::vector<Double_t>(0);

  auto cap = t->GetEntries();
  x.reserve(cap);
  y.reserve(cap);

  for (Long64_t e = 0; e < cap; ++e) {
    t->GetEntry(e);
    for (Int_t i=0; i < numrings; ++i) {
      if (rid[i] == ring) {
        x.push_back(static_cast<Double_t>(run));
        y.push_back(static_cast<Double_t>(rat[i] * 100));
        rms.push_back(static_cast<Double_t>(drms[i] * 100));
        zeros.push_back(0);
      }
    }

  }

  auto tge = new TGraphErrors(x.size(), x.data(), y.data(), zeros.data(), rms.data());
  tge->SetMarkerStyle(kStar);
  return tge;
}

Bool_t WriteGraphs(const char *pth)
{
  TFile *f = new TFile(pth, "NEW");
  if (!f) {
    return kFALSE;
  }

  for (unsigned int i=0; i < 5; ++i) {
    TFile *colorf = new TFile(GetPath(i));
    if (!colorf) {
      return kFALSE;
    }

    for (unsigned int j=0; j < 10; ++j) {
      auto g = MakeGraph(colorf, j);
      if (!g) {
        continue;
      }
      string name("graph");
      name += to_string(i) + string("_") + to_string(j);
      string title("Graph Color ");
      title += to_string(i) + string(" ring ") + to_string(j);
      g->SetNameTitle(name.c_str(), title.c_str());
      g->SetDrawOption("A*");

      f->cd();
      g->Write();
      delete g;
    }

    colorf->Close();
  }

  f->Close();
  return kTRUE;
}

Bool_t WriteMultigraphs(const char *pth, Int_t ring)
{
  TFile *f = new TFile(pth, "UPDATE");
  if (!f) {
    return kFALSE;
  }

  string name("");
  name = string("graph2_") + to_string(ring);
  TGraphErrors *green = (TGraphErrors *)f->Get(name.c_str());

  name = string("graph3_") + to_string(ring);
  TGraphErrors *viohigh = (TGraphErrors *)f->Get(name.c_str());

  name = string("graph4_") + to_string(ring);
  TGraphErrors *bhigh = (TGraphErrors *)f->Get(name.c_str());

  if (green) {
    green->SetMarkerColor(kGreen);
    green->SetMarkerStyle(kFullCircle);
    green->SetName((string("green") + to_string(ring)).c_str());
    green->GetYaxis()->SetRangeUser(85, 105);
  }

  if (viohigh) {
    viohigh->SetMarkerColor(kViolet);
    viohigh->SetMarkerStyle(kFullCircle);
    viohigh->SetName((string("violethigh") + to_string(ring)).c_str());
    viohigh->GetYaxis()->SetRangeUser(85, 105);
  }

  if (bhigh) {
    bhigh->SetMarkerColor(kBlue);
    bhigh->SetMarkerStyle(kFullCircle);
    bhigh->SetName((string("bluehigh") + to_string(ring)).c_str());
    bhigh->GetYaxis()->SetRangeUser(85, 105);
  }

  name = string("cRing") + to_string(ring);
  TMultiGraph *mg = new TMultiGraph();
  mg->SetName(name.c_str());

  if (!mg) {
    f->Close();
    return kFALSE;
  }

  if (green) {
    mg->Add(green, "P");
    green->SetTitle("ring response to green LEDs");
  }

  if (bhigh) {
    mg->Add(bhigh, "P");
    bhigh->SetTitle("ring response to blue LEDs");
  }

  if (viohigh) {
    mg->Add(viohigh, "P");
    viohigh->SetTitle("ring response to violet LEDs");
  }

  string title = string("Ring ") + to_string(ring + 1) + string(" Response over Time");
  mg->SetTitle(title.c_str());
  mg->GetXaxis()->SetTitle("Run Number (increases with time)");
  mg->GetYaxis()->SetTitle("Ring Response to Light (as a percentage of initial response)");
  mg->GetYaxis()->SetRangeUser(85, 105);

  name += string("canvas");

  mg->Write();
  f->cd();
  mg->Write();
  f->Close();

  return kTRUE;
}

void WriteAllMultis(const char *pth)
{
  for (Int_t i=0; i < 10; ++i) {
    WriteMultigraphs(pth, i);
  }
}
