#include "chainify.hpp"
#include "ledutility.hpp"

#include <TMath.h>

#include <cstring>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <climits>

Chainify::Chainify(
    const char *path,
    const char *out,
    Int_t num,
    Int_t min,
    Int_t max
  ) : intreename("fcalRingRatios"), outtreename("fcalRingChains"),
    fname(path), runperchain(num), minrun(min), maxrun(max)
{
  if (strcmp(path, out) == 0) {
    this->myfile = new TFile(path, "UPDATE");
    if (!this->myfile) {
      throw std::runtime_error("Bad file!");
    }
    this->outfile = NULL;
  } else {
    this->myfile = new TFile(path);
    if (!this->myfile) {
      throw std::runtime_error("Bad file!");
    }

    this->outfile = new TFile(out, "UPDATE");
    if (!this->outfile) {
      throw std::runtime_error("Bad file!");
    }
  }


  this->dratios = std::vector<ratiovec>(FCALLED::numrings);
  this->chains = std::vector<chainvec>(FCALLED::numrings);
  this->chainids = std::unordered_set<Int_t>();
}

Chainify::~Chainify()
{
  this->myfile->Close();
  if (this->outfile) {
    this->outfile->Close();
  }
}

void Chainify::MakeChains()
{
  this->ReadRatios();
  this->ChainifyRatios();
  this->WriteChains();
}

void Chainify::ReadRatios()
{
  TTree *t = (TTree *)this->myfile->Get(this->intreename.c_str());
  if (!t) {
    throw std::runtime_error("Bad tree!");
  }

  Int_t rnum = 0;
  Int_t rids[10] {0};
  Int_t run = 0;
  Float_t rat[10] {0};
  Float_t ratrms[10] {0};
  Int_t rcount[10] {0};

  t->SetBranchAddress("run", &run);
  t->SetBranchAddress("drationum", &rnum);
  t->SetBranchAddress("dratioid", &rids[0]);
  t->SetBranchAddress("dratio", &rat[0]);
  t->SetBranchAddress("dratiocounts", &rcount[0]);
  t->SetBranchAddress("dratiorms", &ratrms[0]);

  Long64_t entr = t->GetEntries();
  if (entr < 0) {
    throw std::runtime_error("Bad entries!");
  }

  struct ratiodata tmp{};
  for (Long64_t i=0; i < entr; ++i) {
    t->GetEntry(i);
    if (run < this->minrun || run > this->maxrun) {
      continue;
    }

    tmp.runid = run;
    for (Int_t r=0; r < rnum; ++r) {
      tmp.ratiovalue = rat[r];
      tmp.ratiocount = rcount[r];
      tmp.ratiorms = ratrms[r];
      tmp.ratioid = rids[r];
      this->dratios[rids[r]].push_back(tmp);
    }
  }
}

void Chainify::ChainifyRatios()
{
  for (Int_t ring=0; ring < 10; ++ring) {
    std::vector<struct ratiodata> sorted{this->dratios[ring].cbegin(), this->dratios[ring].cend()};
    std::sort(
      sorted.begin(),
      sorted.end(),
      [] (const struct ratiodata& lhs, const struct ratiodata& rhs) {
        return lhs.runid < rhs.runid;
      }
    );

    Int_t chainid = 1;
    Int_t chaincounter = 0;
    struct chaindata tmp{};

    size_t posptr = 0;
    for (const auto& rdata: sorted) {
      ++posptr;
      if (chaincounter == 0) {
        tmp.chainid = rdata.runid;
        tmp.chaincount = 1;
        tmp.ratiocounts = rdata.ratiocount;
        tmp.chainsum = rdata.ratiovalue;
        tmp.chainrmssum = TMath::Sq(rdata.ratiorms);
        this->chainids.emplace(rdata.runid);
        this->chains[ring].push_back(tmp);
      } else {
        this->chains[ring][chainid - 1].ratiocounts += rdata.ratiocount;
        this->chains[ring][chainid - 1].chainsum += rdata.ratiovalue;
        this->chains[ring][chainid - 1].chainrmssum += TMath::Sq(rdata.ratiorms);
        ++this->chains[ring][chainid - 1].chaincount;
      }

      ++chaincounter;
      if (chaincounter == this->runperchain) {
        chaincounter = 0;
        ++chainid;
      }

      if (chaincounter != 0
          && posptr != sorted.size()
          && FCALLED::GetNormSection(rdata.runid, rdata.ratioid) !=
            FCALLED::GetNormSection(sorted[posptr].runid, sorted[posptr].ratioid))
      {
        chaincounter = 0;
        ++chainid;
      }
    }

    for (auto& cdata: this->chains[ring]) {
      cdata.chainvalue = cdata.chainsum / static_cast<Float_t>(cdata.chaincount);
      cdata.chainrms = TMath::Sqrt(cdata.chainrmssum);
    }
  }
}

void Chainify::WriteChains()
{
  TTree *t = new TTree(this->outtreename.c_str(), this->outtreename.c_str());

  Int_t chainid = 0;
  Int_t chainnum = 0;
  Int_t rids[10] {0};
  Float_t chainval[10] {0};
  Float_t chainrms[10] {0};
  Int_t chaincount[10] {0};

  std::string title("");
  t->Branch("chain", &chainid, "chain/I");
  t->Branch("ringnum", &chainnum, "ringnum/I");
  t->Branch("ringavg", &chainval[0], "ringavg[ringnum]/F");
  t->Branch("ringcount", &chaincount[0], "ringcount[ringnum]/I");
  t->Branch("ringrms", &chainrms[0], "ringrms[ringnum]/F");
  t->Branch("ringid", &rids[0], "ringid[ringnum]/I");

  for (auto& cnum : this->chainids) {
    chainid = cnum;
    chainnum = 0;
    for (unsigned int r=0; r < FCALLED::numrings; ++r) {
      const auto chainelem = std::find_if(
        this->chains[r].cbegin(),
        this->chains[r].cend(),
        [cnum] (const struct chaindata& ele) {
          return ele.chainid == cnum;
        }
      );

      if (chainelem == this->chains[r].cend()) {
        continue;
      }

      rids[chainnum] = r;
      chainval[chainnum] = chainelem->chainvalue;
      chainrms[chainnum] = chainelem->chainrms;
      chaincount[chainnum] = chainelem->ratiocounts;
      ++chainnum;
    }

    if (chainnum > 0) {
      t->Fill();
    }
  }

  if (this->outfile) {
    this->outfile->cd();
  } else {
    this->myfile->cd();
  }

  t->Write();
}
