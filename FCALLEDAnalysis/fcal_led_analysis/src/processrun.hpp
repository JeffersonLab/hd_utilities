#ifndef processrun_hpp_
#define processrun_hpp_

#include "ledutility.hpp"

#include <TTree.h>
#include <TFile.h>
#include <TMath.h>

#include <map>
#include <utility>
#include <functional>
#include <vector>

struct ringdata {
  Int_t ringid = 0;
  Float_t ringvalue = 0.0f;
  Float_t ringrms = 0.0f;
  Int_t ringcount = 0;
  Float_t ringsum = 0.0;
  Float_t ringsqsum = 0.0;

  ringdata (Int_t i, Float_t a, Float_t b, Int_t c)
    : ringid(i), ringvalue(0), ringrms(0), ringcount(c), ringsum(a), ringsqsum(b)
  {}
  ringdata ()
    : ringid(0), ringvalue(0), ringrms(0), ringcount(0), ringsum(0), ringsqsum(0)
  {}
};

typedef std::vector<struct ringdata> ringvec;

class ProcessRun
{
public:
  ProcessRun(const char *t, const char *f);
  virtual ~ProcessRun();
  void Run();
private:
  const std::string fname;
  const std::string tree;
  const std::string otree;
  TFile *infile;
  TFile *outfile;

  TTree *intree;
  TTree *outtree;

  Int_t run;

  std::map<FCALLED::COLOR, ringvec> processed_ring_data;
  std::map<FCALLED::COLOR, struct ringdata> processed_norm_data;

  std::map<FCALLED::COLOR, Float_t *> esum;
  std::map<FCALLED::COLOR, Float_t *> esqsum;
  std::map<FCALLED::COLOR, Int_t *> ecnt;

  std::map<FCALLED::COLOR, Float_t> nsum;
  std::map<FCALLED::COLOR, Float_t> nsqsum;
  std::map<FCALLED::COLOR, Int_t> ncnt;

  Float_t *edata;
  Float_t *erms;
  Int_t *ecount;
  Int_t ringcount;
  Int_t *ringid;

  Float_t norm;
  Float_t nrms;
  Int_t ncount;

  void HandleFile();
  void GetAvgs();
  void WriteAvgs();
  void WriteAvg(FCALLED::COLOR col);
  TTree *GetTree(TFile *ofile);

  static const char * DataPath(FCALLED::COLOR c);
  static const char * DataPath(unsigned int c);
  static const char * LockPath(FCALLED::COLOR c);
  static const char * LockPath(unsigned int c);
};

#endif
