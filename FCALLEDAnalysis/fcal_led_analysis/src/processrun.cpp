#include "processrun.hpp"
#include "DFCALGeometry.hpp"

#include <TDataType.h>
#include <TMath.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <cctype>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

ProcessRun::ProcessRun(
    const char *t,
    const char *f
  )
  : fname(f), tree(t), otree("fcalRings")
{
  this->infile = new TFile(f);
  if (!this->infile) {
    throw std::runtime_error("Bad file!");
  }
  this->intree = (TTree *)this->infile->Get(t);
  if (!this->intree) {
    throw std::runtime_error("Bad tree!");
  }

  this->edata = new Float_t[FCALLED::numrings];
  this->erms = new Float_t[FCALLED::numrings];
  this->ecount = new Int_t[FCALLED::numrings];
  this->ringid = new Int_t[FCALLED::numrings];
  this->norm = 0.0;
  this->nrms = 0.0;
  this->ncount = 0;

  this->processed_ring_data = std::map<FCALLED::COLOR, ringvec>();
  this->processed_norm_data = std::map<FCALLED::COLOR, struct ringdata>();

  std::string getnum("");
  Bool_t last = kTRUE;

  for (size_t i=0; i < this->fname.size(); ++i) {
    auto dig = isdigit(this->fname[i]);
    if (dig && last) {
      getnum += std::string(1, this->fname[i]);
      last = kTRUE;
    } else if (dig) {
      getnum = std::string(1, this->fname[i]);
      last = kTRUE;
    } else {
      last = kFALSE;
    }
  }

  if (getnum.size() == 0 || getnum[0] == '\0') {
    this->run = -1;
  } else {
    if (getnum[0] == '0') {
      getnum.erase(0, 1);
    }

    this->run = static_cast<Int_t>(std::stoi(getnum));
  }
}

ProcessRun::~ProcessRun()
{
  this->infile->Close();
  delete this->infile;
}

void ProcessRun::Run()
{
  this->HandleFile();
  this->GetAvgs();
  this->WriteAvgs();
}

void ProcessRun::HandleFile()
{
  Int_t nHits = 0;
  Int_t runr = 0;
  Float_t eTot = 0.0;
  Int_t *chan = new Int_t[FCALLED::bigbuffer];
  Float_t *E  = new Float_t[FCALLED::bigbuffer];

  this->intree->SetBranchAddress("run", &runr);
  this->intree->SetBranchAddress("eTot", &eTot);
  this->intree->SetBranchAddress("nHits", &nHits);
  this->intree->SetBranchAddress("E", &E[0]);
  this->intree->SetBranchAddress("chan", &chan[0]);

  for (unsigned int i=0; i < FCALLED::numcolors; ++i) {
    auto key = FCALLED::RangeColor(i);

    this->processed_ring_data[key] = std::vector<struct ringdata>();
    this->processed_norm_data[key] = {};
  }

  Long64_t entr = this->intree->GetEntries();
  if (entr <= 0) {
    return;
  }

  DFCALGeometry dfcal{};

  unsigned int col = 0;
  FCALLED::COLOR color = FCALLED::COLOR_GREEN;
  for (Long64_t i=0; i < entr; ++i) {
    this->intree->GetEntry(i);

    if (this->run == -1) {
      this->run = runr;
    }

    if (eTot < FCALLED::noise_cutoff) {
      continue;
    }

    Float_t runningsum[FCALLED::numrings] {0};
    Int_t runningcount[FCALLED::numrings] {0};

    Float_t runningnorm = 0.0;
    Int_t runningncount = 0;

    for (Int_t n=0; n < nHits; ++n) {
      Int_t thischan = chan[n];
      auto dist = dfcal.positionOnFace((int)thischan).Mod();
      auto ring = FCALLED::GetRing(dist);
      col = FCALLED::GetColor(eTot, this->run, ring);
      color = FCALLED::RangeColor(col);

      runningsum[ring] += E[n];
      ++runningcount[ring];

      if (dist >= FCALLED::innerdiam && dist <= FCALLED::outerdiam) {
        runningnorm += E[n];
        ++runningncount;
      }
    }

    if (runningncount >= FCALLED::norm_size / 2) {
      size_t numringsfound = 0;
      for (unsigned int i=0; i < FCALLED::numrings; ++i) {
        if (runningcount[i] >= FCALLED::ring_size[i] / 2) {
          ++numringsfound;
          auto vfind = std::find_if(
            this->processed_ring_data[color].begin(),
            this->processed_ring_data[color].end(),
            [i] (const struct ringdata& r) {
              return r.ringid == (Int_t)i;
            }
          );

          if (vfind != this->processed_ring_data[color].end()) {
            vfind->ringsum += (runningsum[i] / static_cast<Float_t>(runningcount[i]));
            vfind->ringsqsum += TMath::Sq(runningsum[i] / static_cast<Float_t>(runningcount[i]));
            ++vfind->ringcount;
          } else {
            struct ringdata pushdata{};
            pushdata.ringid = (Int_t)i;
            pushdata.ringsum = runningsum[i] / static_cast<Float_t>(runningcount[i]);
            pushdata.ringsqsum = TMath::Sq(runningsum[i] / static_cast<Float_t>(runningcount[i]));
            pushdata.ringcount = 1;
            this->processed_ring_data[color].push_back(pushdata);
          }
        }
      }

      if (!numringsfound) {
        continue;
      }

      ++this->processed_norm_data[color].ringcount;
      this->processed_norm_data[color].ringsum +=
        runningnorm / static_cast<Float_t>(runningncount);
      this->processed_norm_data[color].ringsqsum +=
        TMath::Sq(runningnorm / static_cast<Float_t>(runningncount));
    }
  }

  delete[] chan;
  delete[] E;
}

void ProcessRun::GetAvgs()
{
  for (unsigned int i=0; i < FCALLED::numcolors; ++i) {
    auto color = FCALLED::RangeColor(i);

    if (this->processed_norm_data[color].ringcount == 0) {
      continue;
    }

    this->processed_norm_data[color].ringvalue =
      this->processed_norm_data[color].ringsum /
      static_cast<Float_t>(this->processed_norm_data[color].ringcount);
    this->processed_norm_data[color].ringrms = TMath::Sqrt(TMath::Abs(
        this->processed_norm_data[color].ringsqsum / static_cast<Float_t>(this->processed_norm_data[color].ringcount)
        - TMath::Sq(this->processed_norm_data[color].ringvalue)
    ));

    for (auto& rvec : this->processed_ring_data[color]) {
      rvec.ringvalue = rvec.ringsum / static_cast<Float_t>(rvec.ringcount);
      rvec.ringrms = TMath::Sqrt(TMath::Abs(
          rvec.ringsqsum / static_cast<Float_t>(rvec.ringcount)
          - TMath::Sq(rvec.ringvalue)
      ));
    }
  }
}

void ProcessRun::WriteAvgs()
{
  mkdir("data", 0777);
  for (unsigned int i=0; i < FCALLED::numcolors; ++i) {
    this->WriteAvg(FCALLED::RangeColor(i));
  }
}

void ProcessRun::WriteAvg(FCALLED::COLOR col)
{
  struct ringdata& normdata = this->processed_norm_data[col];
  ringvec& rdata = this->processed_ring_data[col];

  if (normdata.ringcount == 0) {
    return;
  }

  if (rdata.size() == 0) {
    return;
  }

  auto lockfile = open(ProcessRun::LockPath(col), O_CREAT|O_RDWR, 0777);
  if (lockfile == -1) {
    throw std::runtime_error("Error getting lockfile!");
  }
  lockf(lockfile, F_LOCK, 0);

  auto ofile = new TFile(ProcessRun::DataPath(col), "UPDATE");
  TTree *tr = this->GetTree(ofile);

  this->norm = normdata.ringvalue;
  this->nrms = normdata.ringrms / TMath::Sqrt((Float_t)normdata.ringcount);
  this->ncount = normdata.ringcount;

  size_t pos = 0;
  for (auto& somering : rdata) {
    this->edata[pos] = somering.ringvalue;
    this->erms[pos] = somering.ringrms / TMath::Sqrt((Float_t)somering.ringcount);
    this->ecount[pos] = somering.ringcount;
    this->ringid[pos] = somering.ringid;
    ++pos;
  }

  this->ringcount = pos;
  tr->Fill();

  ofile->Write();
  ofile->Close();
  delete ofile;

  lockf(lockfile, F_ULOCK, 0);
  close(lockfile);
}

TTree *ProcessRun::GetTree(TFile *file)
{
  if (!file) {
    return nullptr;
  }

  TTree *t;

  if (file->Get(this->otree.c_str())) {
    t = (TTree *)file->Get(this->otree.c_str());
    t->SetBranchAddress("run", &this->run);
    t->SetBranchAddress("norm", &this->norm);
    t->SetBranchAddress("nrms", &this->nrms);
    t->SetBranchAddress("ncount", &this->ncount);
    t->SetBranchAddress("rings", &this->ringcount);
    t->SetBranchAddress("ringavg", &this->edata[0]);
    t->SetBranchAddress("ringrms", &this->erms[0]);
    t->SetBranchAddress("ringcnt", &this->ecount[0]);
    t->SetBranchAddress("ringid", &this->ringid[0]);
  } else {
    t = new TTree(this->otree.c_str(), this->otree.c_str());
    t->Branch("run", &this->run, "run/I");
    t->Branch("norm", &this->norm, "norm/F");
    t->Branch("nrms", &this->nrms, "nrms/F");
    t->Branch("ncount", &this->ncount, "ncount/I");
    t->Branch("rings", &this->ringcount, "rings/I");
    t->Branch("ringavg", &this->edata[0], "ringavg[rings]/F");
    t->Branch("ringrms", &this->erms[0], "ringrms[rings]/F");
    t->Branch("ringcnt", &this->ecount[0], "ringcnt[rings]/I");
    t->Branch("ringid", &this->ringid[0], "ringid[rings]/I");
  }

  return t;
}

const char * ProcessRun::DataPath(FCALLED::COLOR c)
{
  switch(c) {
    case FCALLED::COLOR_BLUE_LOW: return "data/data_blue_low.root";
    case FCALLED::COLOR_VIOLET_LOW: return "data/data_violet_low.root";
    case FCALLED::COLOR_GREEN: return "data/data_green.root";
    case FCALLED::COLOR_VIOLET_HIGH: return "data/data_violet_high.root";
    case FCALLED::COLOR_BLUE_HIGH: return "data/data_blue_high.root";
    case FCALLED::COLOR_INVALID:
    default: return nullptr;
  }
}

const char * ProcessRun::DataPath(unsigned int c)
{
  return ProcessRun::DataPath(FCALLED::RangeColor(c));
}

const char * ProcessRun::LockPath(FCALLED::COLOR c)
{
  switch (c) {
    case FCALLED::COLOR_BLUE_LOW: return "data/data_blue_low.lock";
    case FCALLED::COLOR_VIOLET_LOW: return "data/data_violet_low.lock";
    case FCALLED::COLOR_GREEN: return "data/data_green.lock";
    case FCALLED::COLOR_VIOLET_HIGH: return "data/data_violet_high.lock";
    case FCALLED::COLOR_BLUE_HIGH: return "data/data_blue_high.lock";
    case FCALLED::COLOR_INVALID:
    default: return nullptr;
  }
}

const char * ProcessRun::LockPath(unsigned int r)
{
  return ProcessRun::LockPath(FCALLED::RangeColor(r));
}
