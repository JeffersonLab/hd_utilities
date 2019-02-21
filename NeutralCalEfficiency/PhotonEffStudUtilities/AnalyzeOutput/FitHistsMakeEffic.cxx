#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include "TLatex.h"
#include "TPaveStats.h"
#include "TGraphPainter.h"
#include "TString.h"
#include "TCollection.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2I.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMinuit.h"
#include "TKey.h"
#include "TCut.h"
#include "TDatime.h"
#include "TAxis.h"
#include "TLine.h"
#include "TTree.h"
#include "TBranch.h"
#include "TStyle.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TDirectory.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TRandom3.h"

#include <TROOT.h>
#include <TStyle.h>

Bool_t FCAL_STUDY=true; //true: look for one candidates in FCAL; false: look for one candidate in BCAL

Int_t N_E_BINS = 14; //FCAL
Int_t N_THETA_BINS = 40; //FCAL
// Int_t N_E_BINS = 20; //BCAL
// Int_t N_THETA_BINS = 11; //BCAL

//Exclusion region around phi
//Globals such that same values used in 2gaus and 3gaus fits
Double_t gexclude_lo = 0.95;
Double_t gexclude_hi = 1.07;

Int_t NFITS = 7;
Int_t MY_SEED = 5;

Double_t omega_mass_min = 0.71;
Double_t omega_mass_max = 0.85;
Double_t omega_width_min = 0.003;
Double_t omega_width_max = 0.07;

Double_t min_fit_range = 0.61;
Double_t max_fit_range = 1.35;

TString gPlotRelativeDirectory = ".plots/";
TString fit_options = "Q";

Bool_t save_hists = false;

Double_t jz_3gaus_2poly(Double_t* x, Double_t* parms) {
	Double_t amplitude1 = parms[0];
	Double_t mean1      = parms[1];
	Double_t sigma1     = parms[2];
	Double_t amplitude2 = parms[3];
	Double_t mean2      = parms[4];
	Double_t sigma2     = parms[5];
	Double_t amplitude3 = parms[6];
	Double_t mean3      = parms[7];
	Double_t sigma3     = parms[8];
	Double_t p0         = parms[9];
	Double_t p1         = parms[10];
	Double_t p2         = parms[11];

	if(gexclude_lo<x[0] and x[0]<gexclude_hi) {
		TF1::RejectPoint();
		return 0.;
	}

	Double_t gaus_return1 = parms[0]* (1/parms[2]) * (1/sqrt(2*3.14159) ) *exp(-(x[0]-parms[1])*(x[0]-parms[1])/(2*parms[2]*parms[2]) );
	Double_t gaus_return2 = parms[0+3]* (1/parms[2+3]) * (1/sqrt(2*3.14159) ) *exp(-(x[0]-parms[1+3])*(x[0]-parms[1+3])/(2*parms[2+3]*parms[2+3]) );
	Double_t gaus_return3 = parms[0+6]* (1/parms[2+6]) * (1/sqrt(2*3.14159) ) *exp(-(x[0]-parms[1+6])*(x[0]-parms[1+6])/(2*parms[2+6]*parms[2+6]) );
	
	return p0+x[0]*p1+x[0]*x[0]*p2+gaus_return1+gaus_return2+gaus_return3;
}

Double_t jz_2gaus_2poly(Double_t* x, Double_t* parms) {
	
	Double_t sig_yield    = parms[0];
	Double_t mean1        = parms[1];
	Double_t sigma1       = parms[2];
	Double_t rel_fraction = parms[3];
	Double_t mean2        = parms[4];
	Double_t sigma2       = parms[5];
	Double_t p0           = parms[6];
	Double_t p1           = parms[7];
	Double_t p2           = parms[8];
	
	if(gexclude_lo<x[0] and x[0]<gexclude_hi) {
		TF1::RejectPoint();
		return 0.;
	}

	Double_t gaus_return1 = parms[0]* (1/parms[2]) * (1/sqrt(2*3.14159) ) *exp(-(x[0]-parms[1])*(x[0]-parms[1])/(2*parms[2]*parms[2]) );
	Double_t gaus_return2 = parms[0+3]* (1/parms[2+3]) * (1/sqrt(2*3.14159) ) *exp(-(x[0]-parms[1+3])*(x[0]-parms[1+3])/(2*parms[2+3]*parms[2+3]) );
	
	return p0+x[0]*p1+x[0]*x[0]*p2+sig_yield*(rel_fraction*gaus_return1+(1-rel_fraction)*gaus_return2);
}



vector<Double_t> jz_fitandsave_omega_MM_3gaus(TH1F* hist, TString str_tag) {
	
	TCanvas* c1 = new TCanvas ("c1","c1",1200*1,900*1);
	TRandom3* myrand = new TRandom3();
	myrand->SetSeed(MY_SEED);
	
	vector<Double_t> return_vec;
	
	cout << "Fitting hist with tag: " << str_tag << endl;
	
	TF1* my_MMomega_fit = new TF1("my_MMomega_fit",jz_3gaus_2poly,min_fit_range,max_fit_range,12);
	
	my_MMomega_fit->SetParameters(200,0.782,0.007,200,0.782,omega_width_min,200,0.782,omega_width_max);
	my_MMomega_fit->SetParLimits(0,0,1000000); //Yeild must be positive
	my_MMomega_fit->SetParLimits(0+3,0,1000000); //Yeild must be positive
	my_MMomega_fit->SetParLimits(0+6,0,1000000); //Yeild must be positive
	my_MMomega_fit->SetParLimits(1,omega_mass_min,omega_mass_max); //Omega mean must be close at least
	my_MMomega_fit->SetParLimits(1+3,omega_mass_min,omega_mass_max); //Omega mean must be close at least
	my_MMomega_fit->SetParLimits(1+6,omega_mass_min,omega_mass_max); //Omega mean must be close at least
	my_MMomega_fit->SetParLimits(2,omega_width_min,omega_width_max); //Omega sigma must be reasonable
	my_MMomega_fit->SetParLimits(2+3,omega_width_min,omega_width_max); //Omega sigma must be reasonable
	my_MMomega_fit->SetParLimits(2+6,omega_width_min,omega_width_max); //Omega sigma must be reasonable
	//my_MMomega_fit->FixParameter(9,0.) //Linear background only
	//my_MMomega_fit->FixParameter(10,0.) //Linear background only
	//my_MMomega_fit->FixParameter(11,0.) //Linear background only
	my_MMomega_fit->SetNpx(1000);
	hist->GetXaxis()->SetRangeUser(0.4,1.4);
	my_MMomega_fit->SetParNames("Sig Yield 1","Mean 1","#sigma 1","Sig Yield 2","Mean 2","#sigma 2","Sig Yield 3","Mean 3","sigma 3");
	my_MMomega_fit->SetParName(9,"pol0");
	my_MMomega_fit->SetParName(10,"pol1");
	my_MMomega_fit->SetParName(11,"pol2");
	
	
	Double_t best_chisq = 1e9;
	Double_t my_yield = -1.;
	Double_t my_yield_err = -1;
	Double_t my_purity = -1;
	
	Double_t bin_width = hist->GetBinWidth(0);
	
 	for(int i =0; i<NFITS; ++i) {

		Double_t omega_mass_start1 = myrand->Uniform(omega_mass_min,omega_mass_max);
		Double_t omega_mass_start2 = myrand->Uniform(omega_mass_min,omega_mass_max);
		Double_t omega_width_start1 = myrand->Uniform(omega_width_min,omega_width_max);
		Double_t omega_width_start2 = myrand->Uniform(omega_width_min,omega_width_max);
		
		my_MMomega_fit->SetParameters(200,omega_mass_start1,omega_mass_start2,200,omega_width_start1,omega_width_start2,200,0.782,0.007);
		Int_t fit_result = -1234;
		if(hist->GetEntries()>500) fit_result = hist->Fit(my_MMomega_fit,fit_options,"",min_fit_range,max_fit_range);
		else fit_result =hist->Fit(my_MMomega_fit,"QL","",min_fit_range,max_fit_range);
		
		Double_t parms[12];
		const Double_t* parms_err;
		my_MMomega_fit->GetParameters(parms);
		parms_err = my_MMomega_fit->GetParErrors();
				
		if(my_MMomega_fit->GetChisquare()<best_chisq) { //New best chi square
			best_chisq = my_MMomega_fit->GetChisquare()/my_MMomega_fit->GetNDF();
			TF1* my_bkg = new TF1("my_bkg","[0]+x*[1]+x*x*[2]",min_fit_range,max_fit_range);
			
			my_bkg->SetParameters(parms[6+3],parms[7+3],parms[8+3]);
			my_bkg->SetLineColor(kBlue);
			my_bkg->SetLineStyle(kDashed);
			my_bkg->Draw("same");
			my_yield = (parms[0]+parms[3]+parms[6])/bin_width;
			Double_t tot_yield = my_MMomega_fit->Integral(0.77,0.79)/bin_width;
			Double_t bkg_yield = my_bkg->Integral(0.77,0.79)/bin_width;
			// Double_t my_yield_err = sqrt(parms_err[0]*parms_err[0]+parms_err[3]*parms_err[3]+parms_err[6]*parms_err[6])/bin_width; //BAD! Ignores correlations
			my_yield_err = 1.5*sqrt(my_yield); //Slightly better? Ballpark from sqrt(N) inflated by something motivated by double gaussian
			my_purity = (tot_yield-bkg_yield)/(tot_yield);
		}
		if(save_hists) c1->SaveAs(gPlotRelativeDirectory+"omegaMM_"+str_tag+".png");
		
		if(my_yield/hist->GetEntries() <0 || my_yield/hist->GetEntries() > 6. ) {
			my_yield = 0;
			my_yield_err = 0;
			my_purity = 0;
			best_chisq = 0;
		}
		
		
		return_vec.push_back(my_yield);
		return_vec.push_back(my_yield_err);
		return_vec.push_back(my_purity);
		return_vec.push_back(best_chisq);
		
	}	
	
	
	return return_vec;
}

vector<Double_t> jz_fitandsave_omega_MM_2gaus(TH1F* hist, TString str_tag) {
	
	TCanvas* c1 = new TCanvas ("c1","c1",1200*1,900*1);
	TRandom3* myrand = new TRandom3();
	myrand->SetSeed(2);
	
	vector<Double_t> return_vec;
	
	TF1* my_MMomega_fit = new TF1("my_MMomega_fit",jz_2gaus_2poly,min_fit_range,max_fit_range,9);
	
	my_MMomega_fit->SetParameters(200,0.782,0.007,0.5,0.782,omega_width_min,0,0,0);
	my_MMomega_fit->SetParLimits(0,0,1000000); //Yeild must be positive
	my_MMomega_fit->SetParLimits(0+3,0,1); //Fraction between 0 and 1
	my_MMomega_fit->SetParLimits(1,omega_mass_min,omega_mass_max); //Omega mean must be close at least
	my_MMomega_fit->SetParLimits(1+3,omega_mass_min,omega_mass_max); //Omega mean must be close at least
	my_MMomega_fit->SetParLimits(2,omega_width_min,omega_width_max); //Omega sigma must be reasonable
	my_MMomega_fit->SetParLimits(2+3,omega_width_min,omega_width_max); //Omega sigma must be reasonable
	//my_MMomega_fit->FixParameter(9,0.) //Linear background only
	//my_MMomega_fit->FixParameter(10,0.) //Linear background only
	//my_MMomega_fit->FixParameter(11,0.) //Linear background only
	my_MMomega_fit->SetNpx(1000);
	hist->GetXaxis()->SetRangeUser(0.4,1.4);
	my_MMomega_fit->SetParNames("SigYield","Mean 1","#sigma 1","gaus_frac","Mean 2","#sigma 2","pol0","pol1","pol2");
	
	Double_t best_chisq = 1e9;
	Double_t my_yield = -1.;
	Double_t my_yield_err = -1;
	Double_t my_purity = -1;
	
	Double_t bin_width = hist->GetBinWidth(0);
	
 	for(int i =0; i<NFITS; ++i) {

		Double_t omega_mass_start1 = myrand->Uniform(omega_mass_min,omega_mass_max);
		Double_t omega_mass_start2 = myrand->Uniform(omega_mass_min,omega_mass_max);
		Double_t omega_width_start1 = myrand->Uniform(omega_width_min,omega_width_max);
		Double_t omega_width_start2 = myrand->Uniform(omega_width_min,omega_width_max);
		
		my_MMomega_fit->SetParameters(200,omega_mass_start1,omega_mass_start2,0.5,omega_width_start1,omega_width_start2,0,0,0);
		Int_t fit_result = -1234;
		if(hist->GetEntries()>1000) fit_result = hist->Fit(my_MMomega_fit,fit_options,"",min_fit_range,max_fit_range);
		else {
			my_MMomega_fit->SetParameter(0,0);
			my_MMomega_fit->SetParameter(3,0);
			my_MMomega_fit->SetParameter(6,0);
			my_MMomega_fit->SetParLimits(0,0,100);
			my_MMomega_fit->SetParLimits(3,0,100);
			my_MMomega_fit->SetParLimits(6,0,100);
			fit_result =hist->Fit(my_MMomega_fit,"Q","",min_fit_range,max_fit_range);
		}
		
		Double_t parms[9];
		const Double_t* parms_err;
		my_MMomega_fit->GetParameters(parms);
		parms_err = my_MMomega_fit->GetParErrors();
				
		if(my_MMomega_fit->GetChisquare()<best_chisq) { //New best chi square
			best_chisq = my_MMomega_fit->GetChisquare()/my_MMomega_fit->GetNDF();
			TF1* my_bkg = new TF1("my_bkg","[0]+x*[1]+x*x*[2]",min_fit_range,max_fit_range);
			
			my_bkg->SetParameters(parms[6],parms[7],parms[8]);
			my_bkg->SetLineColor(kBlue);
			my_bkg->SetLineStyle(kDashed);
			my_bkg->Draw("same");
			my_yield = (parms[0])/bin_width;
			Double_t tot_yield = my_MMomega_fit->Integral(0.77,0.79)/bin_width;
			Double_t bkg_yield = my_bkg->Integral(0.77,0.79)/bin_width;
			// Double_t my_yield_err = sqrt(parms_err[0]*parms_err[0]+parms_err[3]*parms_err[3]+parms_err[6]*parms_err[6])/bin_width; //BAD! Ignores correlations
			my_yield_err = parms_err[0]; //Slightly better? Ballpark from sqrt(N) inflated by something motivated by double gaussian
			my_purity = (tot_yield-bkg_yield)/(tot_yield);
		}
		if(save_hists) c1->SaveAs(gPlotRelativeDirectory+"omegaInv_"+str_tag+".png");
		
		if(my_yield/hist->GetEntries() <0 || my_yield/hist->GetEntries() > 1.3 ) {
			my_yield = 0;
			my_yield_err = 0;
			my_purity = 0;
			best_chisq = 0;
			}
		
		return_vec.push_back(my_yield);
		return_vec.push_back(my_yield_err);
		return_vec.push_back(my_purity);
		return_vec.push_back(best_chisq);
		
	}	
	
	
	return return_vec;
}



Int_t FitOmegaHists(TString str_infile, TString str_outfile) {

	TFile* f_in = new TFile(str_infile);

	// TH1F* test_hist = (TH1F*)f_in->FindObjectAny("h_m1num_miss_omega_accidsub");
	// vector<Double_t> test_vec = jz_fitandsave_omega_MM_3gaus(test_hist,"TESTING");
	
	gStyle->SetOptFit(1111);
	gStyle->SetOptStat(0);
	
	TH1F* h_m1num_miss_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m1den_miss_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m1num_miss_omega_Thetabins_accidsub[N_THETA_BINS];
	TH1F* h_m1den_miss_omega_Thetabins_accidsub[N_THETA_BINS];
	
	TH1F* h_m2num_inv_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m2denineff_miss_omega_Ebins_accidsub[N_E_BINS];
	TH1F* h_m2num_inv_omega_Thetabins_accidsub[N_THETA_BINS];
	TH1F* h_m2denineff_miss_omega_Thetabins_accidsub[N_THETA_BINS];
	
 	for(int i =0; i<N_E_BINS; ++i) {
		char asdfasdf[20];
		sprintf(asdfasdf,"%d",i);
		TString index = asdfasdf;

		h_m1num_miss_omega_Ebins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m1num_miss_omega_Ebins_accidsub"+index);
		h_m1den_miss_omega_Ebins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m1den_miss_omega_Ebins_accidsub"+index);
		h_m2num_inv_omega_Ebins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m2num_inv_omega_Ebins_accidsub"+index);
		h_m2denineff_miss_omega_Ebins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m2denineff_miss_omega_Ebins_accidsub"+index);
	}		
 	for(int i =0; i<N_THETA_BINS; ++i) {
		char asdfasdf[20];
		sprintf(asdfasdf,"%d",i);
		TString index = asdfasdf;
		
		h_m1num_miss_omega_Thetabins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m1num_miss_omega_Thetabins_accidsub"+index);
		h_m1den_miss_omega_Thetabins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m1den_miss_omega_Thetabins_accidsub"+index);
		h_m2num_inv_omega_Thetabins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m2num_inv_omega_Thetabins_accidsub"+index);
		h_m2denineff_miss_omega_Thetabins_accidsub[i] = 	(TH1F*)f_in->FindObjectAny("h_m2denineff_miss_omega_Thetabins_accidsub"+index);
	}			
	
	
	
	Double_t effic_m1_Ebins[N_E_BINS];
	Double_t effic_err_m1_Ebins[N_E_BINS];
	Double_t effic_m2_Ebins[N_E_BINS];
	Double_t effic_err_m2_Ebins[N_E_BINS];
	Double_t effic_m1_Thetabins[N_THETA_BINS];
	Double_t effic_err_m1_Thetabins[N_THETA_BINS];
	Double_t effic_m2_Thetabins[N_THETA_BINS];
	Double_t effic_err_m2_Thetabins[N_THETA_BINS];
	
	Double_t E_arr[N_E_BINS];
	Double_t E_arr_err[N_E_BINS];
	Double_t Theta_arr[N_THETA_BINS];
	Double_t Theta_arr_err[N_THETA_BINS];
	
	Double_t chi2_m1_num_Ebins_arr[N_E_BINS];
	Double_t chi2_m1_den_Ebins_arr[N_E_BINS];
	Double_t chi2_m2_inv_Ebins_arr[N_E_BINS];
	Double_t chi2_m2_ineff_Ebins_arr[N_E_BINS];
	Double_t chi2_m1_num_Thetabins_arr[N_THETA_BINS];
	Double_t chi2_m1_den_Thetabins_arr[N_THETA_BINS];
	Double_t chi2_m2_inv_Thetabins_arr[N_THETA_BINS];
	Double_t chi2_m2_ineff_Thetabins_arr[N_THETA_BINS];
	
	
	//Fit histograms
	
 	for(int i =0; i<N_E_BINS; ++i) {
		char asdfasdf[20];
		sprintf(asdfasdf,"%d",i);
		TString index = asdfasdf;
		
		if(FCAL_STUDY) E_arr[i] = 0.16666666 + 0.333333*i;
		else E_arr[i] = 0.05 + 0.1*i;
		E_arr_err[i] = 0.;
		
		vector<Double_t> m1_num_info = jz_fitandsave_omega_MM_3gaus(h_m1num_miss_omega_Ebins_accidsub[i],"m1_num_Ebin_"+index);
		vector<Double_t> m1_den_info = jz_fitandsave_omega_MM_3gaus(h_m1den_miss_omega_Ebins_accidsub[i],"m1_den_Ebin_"+index);
		// vector<Double_t> m2_inv_info = jz_fitandsave_omega_MM_2gaus(h_m2num_inv_omega_Ebins_accidsub[i],"m2_inv_Ebin_"+index);
		vector<Double_t> m2_inv_info = jz_fitandsave_omega_MM_3gaus(h_m2num_inv_omega_Ebins_accidsub[i],"m2_inv_Ebin_"+index);
		vector<Double_t> m2_ineff_info = jz_fitandsave_omega_MM_3gaus(h_m2denineff_miss_omega_Ebins_accidsub[i],"m2_ineff_Ebin_"+index);
	
		effic_m1_Ebins[i] = m1_num_info[0]/m1_den_info[0];
		effic_err_m1_Ebins[i] = effic_m1_Ebins[i]*sqrt( (m1_num_info[1]/m1_num_info[0])*(m1_num_info[1]/m1_num_info[0]) +(m1_den_info[1]/m1_den_info[0])*(m1_den_info[1]/m1_den_info[0])  );
		
		Double_t m2_den = (m2_inv_info[0]+m2_ineff_info[0]);
		effic_m2_Ebins[i] = m2_inv_info[0]/ m2_den;
		//Wrong!!! But I'm rushed right now, should fix to have proper cov stuff later
		effic_err_m2_Ebins[i] = effic_m2_Ebins[i]*sqrt( (m2_inv_info[1]/m2_inv_info[0])*(m2_inv_info[1]/m2_inv_info[0]) + 1/m2_den  );
	
		chi2_m1_num_Ebins_arr[i] = m1_num_info[3];
		chi2_m1_den_Ebins_arr[i] = m1_den_info[3];
		chi2_m2_inv_Ebins_arr[i] = m2_inv_info[3];
		chi2_m2_ineff_Ebins_arr[i] = m2_ineff_info[3];
		
	}
	
	
 	for(int i =0; i<N_THETA_BINS; ++i) {
		char asdfasdf[20];
		sprintf(asdfasdf,"%d",i);
		TString index = asdfasdf;
		
		if(FCAL_STUDY) Theta_arr[i] = 0.25 + 0.5*i;
		else Theta_arr[i] = 9.5 + 3.*i;
		Theta_arr_err[i] = 0.;
		
		
		if(h_m1num_miss_omega_Thetabins_accidsub[i]->GetEntries()<50 || h_m1den_miss_omega_Thetabins_accidsub[i]->GetEntries()<50 || h_m2num_inv_omega_Thetabins_accidsub[i]->GetEntries()<50 || h_m2denineff_miss_omega_Thetabins_accidsub[i]->GetEntries()<50) {
			effic_m1_Thetabins[i] = 0;
			effic_err_m1_Thetabins[i] = 0;
			effic_m2_Thetabins[i] = 0;
			effic_err_m2_Thetabins[i] = 0;
			chi2_m1_num_Thetabins_arr[i] = 0;
			chi2_m1_den_Thetabins_arr[i] = 0;
			chi2_m2_inv_Thetabins_arr[i] = 0;
			chi2_m2_ineff_Thetabins_arr[i] = 0;
			continue;
		}
		
		
		vector<Double_t> m1_num_info = jz_fitandsave_omega_MM_3gaus(h_m1num_miss_omega_Thetabins_accidsub[i],"m1_num_Thetabin_"+index);
		vector<Double_t> m1_den_info = jz_fitandsave_omega_MM_3gaus(h_m1den_miss_omega_Thetabins_accidsub[i],"m1_den_Thetabin_"+index);
		// vector<Double_t> m2_inv_info = jz_fitandsave_omega_MM_2gaus(h_m2num_inv_omega_Thetabins_accidsub[i],"m2_inv_Thetabin_"+index);
		vector<Double_t> m2_inv_info = jz_fitandsave_omega_MM_3gaus(h_m2num_inv_omega_Thetabins_accidsub[i],"m2_inv_Thetabin_"+index);
		vector<Double_t> m2_ineff_info = jz_fitandsave_omega_MM_3gaus(h_m2denineff_miss_omega_Thetabins_accidsub[i],"m2_ineff_Thetabin_"+index);
	
	
		if(m1_num_info[0]==0||m1_den_info[0]==0) {
			effic_m1_Thetabins[i]=0;
			effic_err_m1_Thetabins[i]=0;
		}
		if(m2_inv_info[0]==0||m2_ineff_info[0]==0) {
			effic_m2_Thetabins[i]=0;
			effic_err_m2_Thetabins[i]=0;
		}
		
	
		effic_m1_Thetabins[i] = m1_num_info[0]/m1_den_info[0];
		effic_err_m1_Thetabins[i] = effic_m1_Thetabins[i]*sqrt( (m1_num_info[1]/m1_num_info[0])*(m1_num_info[1]/m1_num_info[0]) +(m1_den_info[1]/m1_den_info[0])*(m1_den_info[1]/m1_den_info[0])  );
		
		Double_t m2_den = (m2_inv_info[0]+m2_ineff_info[0]);
		effic_m2_Thetabins[i] = m2_inv_info[0]/ m2_den;
		//Wrong!!! But I'm rushed right now, should fix to have proper cov stuff later
		effic_err_m2_Thetabins[i] = effic_m2_Thetabins[i]*sqrt( (m2_inv_info[1]/m2_inv_info[0])*(m2_inv_info[1]/m2_inv_info[0]) + 1/m2_den  );
	
		chi2_m1_num_Thetabins_arr[i] = m1_num_info[3];
		chi2_m1_den_Thetabins_arr[i] = m1_den_info[3];
		chi2_m2_inv_Thetabins_arr[i] = m2_inv_info[3];
		chi2_m2_ineff_Thetabins_arr[i] = m2_ineff_info[3];
		
		if(effic_m1_Thetabins[i] > 1.3) {
			effic_m1_Thetabins[i] = 0;
			effic_err_m1_Thetabins[i] = 0;
		}
		if(effic_m2_Thetabins[i] > 1.3) {
			effic_m2_Thetabins[i] = 0;
			effic_err_m2_Thetabins[i] = 0;
		}
		
		// cout << "m1_num yield: " << m1_num_info[0] << endl;
		// cout << "m1_den yield: " << m1_den_info[0] << endl;
		// cout << "m1 efficiency: " << effic_m1_Thetabins[i] << endl;
		// cout << "m2_inv yield: " << m2_inv_info[0] << endl;
		// cout << "m2_ineff yield: " << m2_ineff_info[0] << endl;
		// cout << "m2 efficiency: " << effic_m2_Thetabins[i] << endl;
		
		// cout << "Signal yield / total m1_num: " << m1_num_info[0]/h_m1num_miss_omega_Thetabins_accidsub[i]->GetEntries() << endl;
		// cout << "Signal yield / total m1_den: " << m1_den_info[0]/h_m1den_miss_omega_Thetabins_accidsub[i]->GetEntries() << endl;
		// cout << "Signal yield / total m2_inv: " << m2_inv_info[0]/h_m2num_inv_omega_Thetabins_accidsub[i]->GetEntries() << endl;
		// cout << "Signal yield / total m2_ineff: " << m2_ineff_info[0]/h_m2denineff_miss_omega_Thetabins_accidsub[i]->GetEntries() << endl;
	
		// cout << "chi2 m1_num " << m1_num_info[3] << endl;
		// cout << "chi2 m1_den " << m1_den_info[3] << endl;
		// cout << "chi2 m2_inv " << m2_inv_info[3] << endl;
		// cout << "chi2 m2_ineff " << m2_ineff_info[3] << endl;
		
		cout << "Done with fitting for bin: " << i << endl;
	}
	
	for(int i =0; i<N_E_BINS; ++i) {
		cout << "E bin: " << i << endl;
		cout << "Efficiency method 1: " << effic_m1_Ebins[i] << " +- " << effic_err_m1_Ebins[i] << endl;
		cout << "Efficiency method 2: " << effic_m2_Ebins[i] << " +- " << effic_err_m2_Ebins[i] << endl;
	}
	for(int i =0; i<N_THETA_BINS; ++i) {
		cout << "Theta bin: " << i << endl;
		cout << "Efficiency method 1: " << effic_m1_Thetabins[i] << " +- " << effic_err_m1_Thetabins[i] << endl;
		cout << "Efficiency method 2: " << effic_m2_Thetabins[i] << " +- " << effic_err_m2_Thetabins[i] << endl;
	}
	
	TGraphErrors* gr_m1_effic_Ebins = new TGraphErrors(N_E_BINS,E_arr,effic_m1_Ebins,E_arr_err,effic_err_m1_Ebins);
	gr_m1_effic_Ebins->SetName("gr_m1_effic_Ebins");
	TGraphErrors* gr_m1_effic_Thetabins = new TGraphErrors(N_THETA_BINS,Theta_arr,effic_m1_Thetabins,Theta_arr_err,effic_err_m1_Thetabins);
	gr_m1_effic_Thetabins->SetName("gr_m1_effic_Thetabins");
	TGraphErrors* gr_m2_effic_Ebins = new TGraphErrors(N_E_BINS,E_arr,effic_m2_Ebins,E_arr_err,effic_err_m2_Ebins);
	gr_m2_effic_Ebins->SetName("gr_m2_effic_Ebins");
	TGraphErrors* gr_m2_effic_Thetabins = new TGraphErrors(N_THETA_BINS,Theta_arr,effic_m2_Thetabins,Theta_arr_err,effic_err_m2_Thetabins);
	gr_m2_effic_Thetabins->SetName("gr_m2_effic_Thetabins");
	
	TGraph* gr_m1_num_chi2_Ebins = new TGraph(N_E_BINS,E_arr,chi2_m1_num_Ebins_arr);
	TGraph* gr_m1_den_chi2_Ebins = new TGraph(N_E_BINS,E_arr,chi2_m1_den_Ebins_arr);
	TGraph* gr_m2_inv_chi2_Ebins = new TGraph(N_E_BINS,E_arr,chi2_m2_inv_Ebins_arr);
	TGraph* gr_m2_ineff_chi2_Ebins = new TGraph(N_E_BINS,E_arr,chi2_m2_ineff_Ebins_arr);
	gr_m1_num_chi2_Ebins->SetName("gr_m1_num_chi2_Ebins");
	gr_m1_den_chi2_Ebins->SetName("gr_m1_den_chi2_Ebins");
	gr_m2_inv_chi2_Ebins->SetName("gr_m2_inv_chi2_Ebins");
	gr_m2_ineff_chi2_Ebins->SetName("gr_m2_ineff_chi2_Ebins");
	TGraph* gr_m1_num_chi2_Thetabins = new TGraph(N_THETA_BINS,Theta_arr,chi2_m1_num_Thetabins_arr);
	TGraph* gr_m1_den_chi2_Thetabins = new TGraph(N_THETA_BINS,Theta_arr,chi2_m1_den_Thetabins_arr);
	TGraph* gr_m2_inv_chi2_Thetabins = new TGraph(N_THETA_BINS,Theta_arr,chi2_m2_inv_Thetabins_arr);
	TGraph* gr_m2_ineff_chi2_Thetabins = new TGraph(N_THETA_BINS,Theta_arr,chi2_m2_ineff_Thetabins_arr);
	gr_m1_num_chi2_Thetabins->SetName("gr_m1_num_chi2_Thetabins");
	gr_m1_den_chi2_Thetabins->SetName("gr_m1_den_chi2_Thetabins");
	gr_m2_inv_chi2_Thetabins->SetName("gr_m2_inv_chi2_Thetabins");
	gr_m2_ineff_chi2_Thetabins->SetName("gr_m2_ineff_chi2_Thetabins");
	
	TFile* outfile = new TFile( str_outfile, "RECREATE" );
	outfile->cd();
	gr_m1_effic_Ebins->Write();
	gr_m2_effic_Ebins->Write();
	gr_m1_effic_Thetabins->Write();
	gr_m2_effic_Thetabins->Write();
	gr_m1_num_chi2_Ebins->Write();
	gr_m1_den_chi2_Ebins->Write();
	gr_m2_inv_chi2_Ebins->Write();
	gr_m2_ineff_chi2_Ebins->Write();
	gr_m1_num_chi2_Thetabins->Write();
	gr_m1_den_chi2_Thetabins->Write();
	gr_m2_inv_chi2_Thetabins->Write();
	gr_m2_ineff_chi2_Thetabins->Write();
	outfile->Close();
	
	return 0;
}

