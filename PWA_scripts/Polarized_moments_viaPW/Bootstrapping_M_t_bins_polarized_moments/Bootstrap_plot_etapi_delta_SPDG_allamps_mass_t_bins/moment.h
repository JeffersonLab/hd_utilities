/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   moment.h
 * Author: marianak
 *
 * Created on August 27, 2020, 1:51 PM
 */

#ifndef MOMENT_H
#define MOMENT_H

#include <vector>
#include <map>
#include <complex>

#include "wave.h"




void Biggest_lm(const waveset& ws, size_t* maxL, size_t* maxM);
double clebsch(double j1, double j2, double j3, double m1, double m2, double m3);
std::complex<double> decomposeMoment(int alpha,double L, double M, const waveset& ws, const double* x);
std::complex<double> decomposeMoment(int alpha ,double L, double M, const waveset& ws, const vector<double>& x);


#endif /* MOMENT_H */

