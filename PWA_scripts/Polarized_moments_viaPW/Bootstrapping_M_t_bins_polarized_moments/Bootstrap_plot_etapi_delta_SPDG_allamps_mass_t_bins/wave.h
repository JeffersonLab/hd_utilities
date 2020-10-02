/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   wave.h
 * Author: marianak
 *
 * Created on August 27, 2020, 1:13 PM
 */

#ifndef WAVE_H
#define WAVE_H

#include <complex>
#include <vector>
#include <map>
#include <iostream>

#include "TH1.h"

using namespace std;

struct wave {
  string name;
  double l, m;
  size_t idx;   // Index into the fit variables.
  bool phaseLocked;

  wave(const char* name_, double ll, double mm)
  : name(name_), l(ll), m(mm), phaseLocked(false) { }
  wave(const char* name_, double ll, double mm, int nBins, double lower, double upper, bool phaseLocked_ = false)
  : name(name_), l(ll), m(mm), phaseLocked(phaseLocked_) { }
  wave(const wave& o);

  ~wave() {} // histograms are owned by ROOT

  void setIndex(int idx_) { idx = idx_; }
  size_t getIndex() const { return idx; }

  const string& getName() const { return name; }
  double getL() const { return l; }
  double getM() const { return m; }

};

inline bool operator<(const wave& c, const wave &o){
    return (c.l < o.l || ( c.l==o.l && c.m < o.m ) );
  }


struct coherent_waves {
  int reflectivity;
  int spinflip;
  std::vector<wave> waves;

  coherent_waves() {};
  coherent_waves(const coherent_waves& o) { reflectivity = o.reflectivity; spinflip = o.spinflip; waves = o.waves; }

  std::vector<wave>& getWaves() { return waves; }
  const std::vector<wave>& getWaves() const { return waves; }
  size_t getNwaves() const { return waves.size(); }

  void print() { cout << "| ";for (size_t i = 0; i < waves.size(); i++) { cout << waves[i].getName() << " "; } cout << endl; }

};

struct waveset : public std::vector<coherent_waves> {
public:
  waveset();

  size_t getNwaves() const {
    size_t count = 0;
    for (size_t i = 0; i < this->size(); i++)
      count += (*this)[i].waves.size();
    return count;
  }

  size_t getNparams() const {
    size_t count = 0;
    for (size_t i = 0; i < this->size(); i++)
      {
	const vector<wave>& w = (*this)[i].waves;
	for (size_t j = 0; j < w.size(); j++)
	  {
	    if (w[j].phaseLocked)
	      count += 1;
	    else
	      count += 2;
	  }
      }
    return count;
  }

};

#endif /* WAVE_H */

