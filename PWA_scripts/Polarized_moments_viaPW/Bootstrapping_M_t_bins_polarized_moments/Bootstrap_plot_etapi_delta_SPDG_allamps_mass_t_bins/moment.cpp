/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <iostream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <unistd.h>
#include <complex>
#include <time.h>
#include "IUAmpTools/FitResults.h"
#include "TFile.h"
#include "Math/SpecFunc.h"
#include "wave.h"
#include "moment.h"


//using namespace std;

void Biggest_lm(const waveset& ws, size_t* maxL, size_t* maxM)
{
  // Find maximum L, M.
  *maxL=0;
  *maxM=0;
  for (size_t iWs = 0; iWs < ws.size(); iWs++)
    {
      const vector<wave>& w = ws[iWs].waves;
      for (size_t iW = 0; iW < w.size(); iW++)
	{
	  *maxL = std::max(size_t(w[iW].l), *maxL);
	  *maxM = std::max(size_t(w[iW].m), *maxM);
	}
    }
}




int isfrac(double x){
  int val = 1;
  double intpart;
  if( modf(x, &intpart) == 0.0){ val = 0;}
  return val;
}


long int factorial(int n)
{
  if(n > 1)
    return n * factorial(n - 1);
  else
    return 1;
}





double clebsch(double j1, double j2, double j3, double m1, double m2, double m3){
  double cg = 0.0;
  // double m3 = m1+m2;
  

  if((m1+m2)!=m3) {return 0.;}

  // check input
  if (isfrac(j1+j2+j3) || isfrac(j1+m1) || isfrac(j2+m2) || isfrac(j3+m3) ||
      isfrac(j3-j1-m2) || isfrac(j3-j2+m1)) {
    return 0.0;
  }
  
  
  // Check for conditions that give CG = 0.
  if ( j3 < fabs(j1-j2) || j3 < fabs(j1-j2)||
      fabs(m1) > j1 || fabs(m2) > j2 || fabs(m3) > j3 ) {
    return 0.0;
  }
  
  // Compute the Clebsch-Gordan coefficient
  cg = sqrt( (2*j3+1)/ factorial( round(j1+j2+j3+1) ) );
  cg = cg * sqrt( factorial(round(j1+j2-j3))*factorial(round(j2+j3-j1))*factorial(round(j3+j1-j2))   );
  cg = cg * sqrt( factorial(round(j1+m1))*factorial(round(j1-m1)) );
  cg = cg * sqrt( factorial(round(j2+m2))*factorial(round(j2-m2)) );
  cg = cg * sqrt( factorial(round(j3+m3))*factorial(round(j3-m3)) );
  
  double sum = 0.0, term;
  for (int k = 0; k < 99; k++) {
    if (j1+j2-j3-k < 0) continue;
    if (j3-j1-m2+k < 0) continue;
    if (j3-j2+m1+k < 0) continue;
    if (j1-m1-k    < 0) continue;
    if (j2+m2-k    < 0) continue;
    term = factorial(round(j1+j2-j3-k)) * factorial(round(j3-j1-m2+k)) ;
    term = term * factorial(round(j3-j2+m1+k)) * factorial(round(j1-m1-k));
    term = term * factorial(round(j2+m2-k)) * factorial(k);
    term = term * pow(-1, k);
    sum = sum + 1./term;
  }
  
  cg = cg * sum;
  
  return cg;
}

















std::complex<double> 
decomposeMoment(int alpha ,double L, double M, const waveset& ws, const vector<double>& x)
{
  return decomposeMoment(alpha , L,M,  ws, &x[0]);
}




std::complex<double> decomposeMoment(int alpha ,double L, double M, const waveset& ws, const double* x)//alpha 0 for unpol. mom, and 1,2,3 for polarized mom.
{
  std::complex<double> result(0.,0.);

  for (size_t iWs = 0; iWs < ws.size(); iWs++)//looping through coherent wave groups ws[iWs] corresponding to diff. reflectivity
    {
      double eps = ws[iWs].reflectivity;

      const vector<wave>& w = ws[iWs].waves;
      for (size_t iW1 = 0; iW1 < w.size(); iW1++) //sum over l1,m1
	{
	  const wave& w1 = w[iW1];
	  for (size_t iW2 = 0; iW2 < w.size(); iW2++) //sum over l2,m2
	    {
	      const wave& w2 = w[iW2];
              std::complex<double> ui (0., 1.);
	      double w1_re,w1_im, w2_re,w2_im;
	      w1_re=x[w1.getIndex()];//x[w1.getIndex()] and x[w1.getIndex()+1] correspond to the real and imaginary components of of wave w1
	      w1_im=x[w1.getIndex()+1];
	      w2_re=x[w2.getIndex()];
	      w2_im=x[w2.getIndex()+1];
              //Coefficent that is common to all sums
              double com_coeff=sqrt((2.*w2.l+1.)/(2.*w1.l+1.))*clebsch(w2.l,L,w1.l,0.,0.,0.);// 6th argument m1=M+m2
	      std::complex<double> llprimeconj=w1_re*w2_re + w1_im*w2_im +ui*(w1_im*w2_re - w1_re*w2_im);//ll'*
	     

	      if (alpha==0){  //H0
		double coeff1 = clebsch(w2.l,L,w1.l,w2.m,M,w1.m); // 6th argument m1=M+m2
		double coeff2 = pow(-1.,w2.m-w1.m)*clebsch(w2.l,L,w1.l,-w2.m,M,-w1.m);
		result=result+llprimeconj*com_coeff*(coeff1+coeff2);
              }

	      else if (alpha==1){//H1 
		double coeff1 = (-eps)*pow(-1.,w1.m)*clebsch(w2.l,L,w1.l,w2.m,M,-w1.m); // 6th argument m1=M+m2
		double coeff2 = (-eps)*pow(-1.,w2.m)*clebsch(w2.l,L,w1.l,-w2.m,M,w1.m);
		result=result+llprimeconj*com_coeff*(coeff1+coeff2);
              }
              
	      else if (alpha==2){  //H2
		std::complex<double> coeff1 = (-ui*eps)*pow(-1.,-w1.m)*clebsch(w2.l,L,w1.l,w2.m,M,-w1.m); // 6th argument m1=M+m2
		std::complex<double> coeff2 = (ui*eps)*pow(-1.,-w2.m)*clebsch(w2.l,L,w1.l,-w2.m,M,w1.m);
		result=result+llprimeconj*com_coeff*(coeff1+coeff2);
              }

	      else      { //H3
		double coeff1 = clebsch(w2.l,L,w1.l,w2.m,M,w1.m); 
		double coeff2 = -pow(-1.,w2.m-w1.m)*clebsch(w2.l,L,w1.l,-w2.m,M,-w1.m); 
		result=result+llprimeconj*com_coeff*(coeff1+coeff2);
	      }

	    
	    }
	}
    }
  if(alpha>0)  result=-result;
  
  return result;
}













