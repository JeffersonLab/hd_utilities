#include "doubleratio.hpp"
#include "ledutility.hpp"

#include <TMath.h>

#include <cstring>
#include <stdexcept>
#include <iostream>

DoubleRatio::DoubleRatio(
    const char *in,
    const char *out
  ) : intree("fcalRings"), outtree("fcalRingRatios")
{
  if (strcmp(in, out) == 0) {
    this->myfile = new TFile(in, "UPDATE");
    if (!this->myfile) {
      throw std::runtime_error("Bad file!");
    }

    this->outfile = NULL;
  } else {
    this->myfile = new TFile(in);
    if (!this->myfile) {
      throw std::runtime_error("Bad file!");
    }

    this->outfile = new TFile(out, "UPDATE");
    if (!this->outfile) {
      throw std::runtime_error("Bad file!");
    }
  }

  this->processed_ratio_data = std::map<Int_t, ratiovec>();
  this->processed_dratio_data = std::map<Int_t, ratiovec>();
  this->runs = std::unordered_set<Int_t>();

  this->rringid = new Int_t[FCALLED::numrings];
  this->nringid = new Int_t[FCALLED::numrings];
  this->nrings = new Float_t[FCALLED::numrings];
  this->rrings = new Float_t[FCALLED::numrings];
  this->nringsrms = new Float_t[FCALLED::numrings];
  this->rringsrms = new Float_t[FCALLED::numrings];
  this->ncount = new Int_t[FCALLED::numrings];
  this->rcount = new Int_t[FCALLED::numrings];
}

DoubleRatio::~DoubleRatio()
{
  if (this->outfile) {
    this->outfile->Close();
  }

  this->myfile->Close();

  delete[] this->nrings;
  delete[] this->rrings;
  delete[] this->rringid;
  delete[] this->nringid;
  delete[] this->nringsrms;
  delete[] this->rringsrms;
  delete[] this->ncount;
  delete[] this->rcount;
}

void DoubleRatio::GetRatios()
{
  this->ReadFile();
  this->GetDRatio();
  this->UpdateFile();
}

void DoubleRatio::ReadFile()
{
  Int_t run = 0;
  Float_t inrings[FCALLED::numrings];
  Float_t inringsrms[FCALLED::numrings];
  Int_t inringscnt[FCALLED::numrings];
  Int_t ringid[FCALLED::numrings];
  Float_t innorm = 0.0;
  Float_t innormrms = 0.0;
  Int_t innormcnt = 0;
  Int_t ringcount = 0;

  TTree *itree = (TTree *)this->myfile->Get(this->intree.c_str());
  if (!itree) {
    throw std::runtime_error("Bad in tree!");
  }

  itree->SetBranchAddress("run", &run);
  itree->SetBranchAddress("norm", &innorm);
  itree->SetBranchAddress("nrms", &innormrms);
  itree->SetBranchAddress("ncount", &innormcnt);
  itree->SetBranchAddress("rings", &ringcount);
  itree->SetBranchAddress("ringavg", &inrings[0]);
  itree->SetBranchAddress("ringrms", &inringsrms[0]);
  itree->SetBranchAddress("ringcnt", &inringscnt[0]);
  itree->SetBranchAddress("ringid", &ringid[0]);

  Long64_t entr = itree->GetEntries();
  if (entr <= 0) {
    return;
  }

  for (Long64_t r=0; r < entr; ++r) {
    itree->GetEntry(r);
    this->runs.insert(run);

    // Guarenteed to have at least a norm value and a ring value

    struct ratiodata ratdata{};

    for (unsigned int i=0; i < static_cast<unsigned int>(ringcount); ++i) {
      // Guarenteed that all the rings in ringcount are non-zero
      // And that all rings in a single entry have an unique id
      ratdata.ratioid = ringid[i];
      ratdata.ratiovalue = inrings[i] / innorm;
      ratdata.ratiorms = TMath::Sqrt(
        TMath::Sq(inringsrms[i] / inrings[i])
        + TMath::Sq(innormrms / innorm)
      ) * ratdata.ratiovalue;
      ratdata.runid = run;
      ratdata.ratiocount = inringscnt[i] + innormcnt;

      if (this->processed_ratio_data.count(ratdata.ratioid) == 0) {
        this->processed_ratio_data[ratdata.ratioid] =
          std::vector<struct ratiodata>();
      }

      this->processed_ratio_data[ratdata.ratioid].push_back(ratdata);
    }
  }
}

void DoubleRatio::GetDRatio()
{
  // Iter over (ring, data)
  for (auto& mappair : this->processed_ratio_data) {
    this->processed_dratio_data[mappair.first] = std::vector<struct ratiodata>();

    std::vector<struct ratiodata> sorted{mappair.second.cbegin(), mappair.second.cend()};

    // Put runs in order
    std::sort(
      sorted.begin(),
      sorted.end(),
      [] (const struct ratiodata& lhs, const struct ratiodata& rhs) {
        return lhs.runid < rhs.runid;
      }
    );

    // Each section of the data gets its own start
    // Int_t is an index into the sorted vector
    struct ratiodata initialruns[FCALLED::numbranchpoints] {};

    for (unsigned int i=0; i < FCALLED::numbranchpoints + 1; ++i) {
      // Since data is sorted first run to be in a branch range
      // is that branch range's initial run
      auto initialdata = std::find_if(
        sorted.cbegin(),
        sorted.cend(),
        [i] (const struct ratiodata& r) {
          return i == FCALLED::GetNormSection(r.runid, r.ratioid);
        }
      );

      // Search will only fail when there are no runs in the branch range.
      // In that case, we dont have any runs that need the initialruns value anyway!
      if (initialdata != sorted.cend()) {
        initialruns[i].ratiovalue = initialdata->ratiovalue;
        initialruns[i].ratiorms = initialdata->ratiorms;
      }
    }

    struct ratiodata dratdata{};
    for (auto& rdata : mappair.second) {
      dratdata.runid = rdata.runid;
      dratdata.ratioid = rdata.ratioid;
      dratdata.ratiocount = rdata.ratiocount;

      Int_t bnum = FCALLED::GetNormSection(rdata.runid, rdata.ratioid);
      const struct ratiodata& compareto = initialruns[bnum];

      dratdata.ratiovalue = rdata.ratiovalue / compareto.ratiovalue;
      dratdata.ratiorms = TMath::Sqrt(
        TMath::Sq(rdata.ratiorms / rdata.ratiovalue) + TMath::Sq(compareto.ratiorms / compareto.ratiovalue)
      )  * dratdata.ratiovalue;
      this->processed_dratio_data[mappair.first].push_back(dratdata);
    }

  }
}

void DoubleRatio::UpdateFile()
{
  TTree *otree = new TTree(this->outtree.c_str(), this->outtree.c_str());
  Int_t run;
  std::string title("");

  otree->Branch("run", &run, "run/I");
  otree->Branch("rationum", &this->nringnum, "rationum/I");
  otree->Branch("drationum", &this->rringnum, "drationum/B");
  otree->Branch("ratioid", &this->nringid[0], "ratioid[rationum]/I");
  otree->Branch("dratioid", &this->rringid[0], "dratioid[drationum]/I");
  otree->Branch("ratio", &this->nrings[0], "ratio[rationum]/F");
  otree->Branch("dratio", &this->rrings[0], "dratio[drationum]/F");
  otree->Branch("ratiorms", &this->nringsrms[0], "ratiorms[rationum]/F");
  otree->Branch("dratiorms", &this->rringsrms[0], "dratiorms[drationum]/F");
  otree->Branch("ratiocounts", &this->ncount[0], "ratiocounts[rationum]/I");
  otree->Branch("dratiocounts", &this->rcount[0], "dratiocounts[drationum]/I");

  for (auto& findrun : this->runs) {
    run = findrun;

    for (unsigned int i=0; i < FCALLED::numrings; ++i) {
      this->nringnum = 0;
      this->rringnum = 0;
    }

    for (unsigned int i=0; i < FCALLED::numrings; ++i) {
      Int_t mapkey = (Int_t)i;
      auto thisring = std::find_if(
        this->processed_ratio_data[mapkey].cbegin(),
        this->processed_ratio_data[mapkey].cend(),
        [findrun] (const struct ratiodata& rd) {
          return rd.runid == findrun;
        }
      );

      if (thisring != this->processed_ratio_data[mapkey].cend()) {
        size_t rpos = static_cast<size_t>(this->nringnum);

        this->nrings[rpos] = thisring->ratiovalue;
        this->nringsrms[rpos] = thisring->ratiorms;
        this->nringid[rpos] = thisring->ratioid;
        this->ncount[rpos] = thisring->ratiocount;
        ++this->nringnum;
      }

      auto thisdring = std::find_if(
        this->processed_dratio_data[mapkey].cbegin(),
        this->processed_dratio_data[mapkey].cend(),
        [findrun] (const struct ratiodata& rd) {
          return rd.runid == findrun;
        }
      );

      if (thisdring != this->processed_dratio_data[mapkey].cend()) {
        size_t rpos = static_cast<size_t>(this->rringnum);

        this->rrings[rpos] = thisdring->ratiovalue;
        this->rringsrms[rpos] = thisdring->ratiorms;
        this->rringid[rpos] = thisdring->ratioid;
        this->rcount[rpos] = thisdring->ratiocount;
        ++this->rringnum;
      }

      if (this->nringnum && this->rringnum) {
        otree->Fill();
      }
    }
  }

  if (this->outfile) {
    this->outfile->cd();
  } else {
    this->myfile->cd();
  }
  otree->Write();
}
