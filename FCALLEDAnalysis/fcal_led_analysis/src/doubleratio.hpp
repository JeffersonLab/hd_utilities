#ifndef DOUBLERATIO_HPP_
#define DOUBLERATIO_HPP_

#include "processrun.hpp"

#include <TFile.h>
#include <TTree.h>

#include <string>
#include <functional>
#include <utility>
#include <map>
#include <vector>
#include <unordered_set>

struct ratiodata {
  Float_t ratiovalue = 0.0;
  Int_t ratiocount = 0;
  Float_t ratiorms = 0.0;
  Int_t ratioid = 0;
  Int_t runid = 0;
};

typedef std::vector<struct ratiodata> ratiovec;

class DoubleRatio {
public:
  DoubleRatio(const char *in, const char *out);
  virtual ~DoubleRatio();
  void GetRatios();
private:
  const std::string intree;
  const std::string outtree;

  TFile *myfile;
  TFile *outfile;

  // Ring id to ratios
  std::map<Int_t, ratiovec> processed_ratio_data;
  std::map<Int_t, ratiovec> processed_dratio_data;

  std::unordered_set<Int_t> runs;

  Int_t nringnum;
  Int_t rringnum;
  Int_t *nringid;
  Int_t *rringid;
  Float_t *nrings;
  Float_t *rrings;
  Float_t *nringsrms;
  Float_t *rringsrms;
  Int_t *ncount;
  Int_t *rcount;

  void ReadFile();
  void GetDRatio();
  void UpdateFile();
};

#endif
