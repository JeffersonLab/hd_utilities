#ifndef CHAINIFY_HPP
#define CHAINIFY_HPP

#include "doubleratio.hpp"

#include <TFile.h>

#include <string>
#include <unordered_set>
#include <vector>

struct chaindata {
  Int_t chainid = 0;
  Int_t chaincount = 0;
  Int_t ratiocounts = 0;
  Float_t chainsum = 0;
  Float_t chainrmssum = 0;
  Float_t chainvalue = 0;
  Float_t chainrms = 0;
};

typedef std::vector<struct chaindata> chainvec;

class Chainify {
public:
  Chainify(const char *path, const char * out, Int_t num, Int_t min, Int_t max);
  virtual ~Chainify();

  void MakeChains();
private:
  const std::string intreename;
  const std::string outtreename;

  const std::string fname;
  const Int_t runperchain;
  const Int_t minrun;
  const Int_t maxrun;

  TFile *myfile;
  TFile *outfile;

  std::vector<ratiovec> dratios;
  std::vector<chainvec> chains;
  std::unordered_set<Int_t> chainids;

  void ReadRatios();
  void ChainifyRatios();
  void WriteChains();
};

#endif
