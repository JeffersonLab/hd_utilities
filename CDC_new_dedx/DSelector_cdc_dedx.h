#ifndef DSelector_cdc_dedx_h
#define DSelector_cdc_dedx_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_cdc_dedx : public DSelector
{
	public:

		DSelector_cdc_dedx(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_cdc_dedx(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
                Int_t Get_Hypo_Num(Particle_t, Int_t); //, DChargedTrackHypothesis*);
                Int_t Read_CDC_dEdx_amp_CorrectionFile(void);
                Int_t Read_CDC_dEdx_int_CorrectionFile(void);
                Float_t Get_CDC_dEdx_amp_CF(Float_t, Float_t);
                Float_t Get_CDC_dEdx_int_CF(Float_t, Float_t);
                Float_t Correct_CDC_dEdx_amp(Int_t, Float_t);
                Float_t Correct_CDC_dEdx_int(Int_t, Float_t);

		void Finalize(void);

                Bool_t IsMC; 

		// ANALYZE CUT ACTIONS
		// // Automatically makes mass histograms where one cut is missing
		//		DHistogramAction_AnalyzeCutActions* dAnalyzeCutActions;

		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS

		//Step 0
		DParticleComboStep* dStep0Wrapper;
		DBeamParticle* dComboBeamWrapper;
		DChargedTrackHypothesis* dKPlusWrapper;
		DChargedTrackHypothesis* dKMinusWrapper;
		DChargedTrackHypothesis* dProtonWrapper;


                vector<vector<Float_t>>CDC_DEDX_AMP_CORRECTION;
                Float_t cdc_min_theta, cdc_max_theta;
                Float_t cdc_min_dedx, cdc_max_dedx;
                Float_t cdc_theta_step, cdc_dedx_step; 
                Int_t cdc_npoints_theta, cdc_npoints_dedx;

                vector<vector<Float_t>>CDC_DEDX_INT_CORRECTION;
                Float_t cdc_min_theta_int, cdc_max_theta_int;
                Float_t cdc_min_dedx_int, cdc_max_dedx_int;
                Float_t cdc_theta_step_int, cdc_dedx_step_int; 
                Int_t cdc_npoints_theta_int, cdc_npoints_dedx_int;



		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:

          	TH2I* dHist_NewDedxProton;
         	TH2I* dHist_NewDedxKPlus;
	        TH2I* dHist_NewDedxKMinus;
	
          	TH2I* dHist_NewDedxIntProton;
         	TH2I* dHist_NewDedxIntKPlus;
	        TH2I* dHist_NewDedxIntKMinus;

	ClassDef(DSelector_cdc_dedx, 0);
};

void DSelector_cdc_dedx::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dKPlusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(0));
	dKMinusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(2));
}


Int_t DSelector_cdc_dedx::Read_CDC_dEdx_amp_CorrectionFile(void){

    FILE *dedxfile = fopen("/home/njarvis/phi/dsel/dedx_amp_corr.txt","r");
    if (!dedxfile) cout << "Could not open dedx_amp_corr.txt\n";
    if (!dedxfile) return 1;

    fscanf(dedxfile,"%i values of theta\n",&cdc_npoints_theta);
    fscanf(dedxfile,"%f min theta\n",&cdc_min_theta);
    fscanf(dedxfile,"%f max theta\n",&cdc_max_theta);
    fscanf(dedxfile,"%f theta step\n",&cdc_theta_step);

    fscanf(dedxfile,"%i values of dedx\n",&cdc_npoints_dedx);
    fscanf(dedxfile,"%f min dedx\n",&cdc_min_dedx);
    fscanf(dedxfile,"%f max dedx\n",&cdc_max_dedx);
    fscanf(dedxfile,"%f dedx step\n",&cdc_dedx_step);
    fscanf(dedxfile,"\n");

    vector<Float_t> dedx_cf_alltheta;
    Float_t dedx_cf;

    // Store the scaling factors in vector<vector<double>>CDC_DEDX_CORRECTION;
  
    for (Int_t ii =0; ii<cdc_npoints_dedx; ii++) {
      for (Int_t jj=0; jj<cdc_npoints_theta; jj++) {
        fscanf(dedxfile,"%f\n",&dedx_cf);
        dedx_cf_alltheta.push_back(dedx_cf);
      }
      CDC_DEDX_AMP_CORRECTION.push_back(dedx_cf_alltheta);
      dedx_cf_alltheta.clear();
    }
    fclose(dedxfile);

    return 0;

}

Int_t DSelector_cdc_dedx::Read_CDC_dEdx_int_CorrectionFile(void){

    FILE *dedxfile = fopen("/home/njarvis/phi/dsel/dedx_int_corr.txt","r");

    if (!dedxfile) cout << "Could not open dedx_int_corr.txt\n";
    if (!dedxfile) return 1;

    fscanf(dedxfile,"%i values of theta\n",&cdc_npoints_theta_int);
    fscanf(dedxfile,"%f min theta\n",&cdc_min_theta_int);
    fscanf(dedxfile,"%f max theta\n",&cdc_max_theta_int);
    fscanf(dedxfile,"%f theta step\n",&cdc_theta_step_int);

    fscanf(dedxfile,"%i values of dedx\n",&cdc_npoints_dedx_int);
    fscanf(dedxfile,"%f min dedx\n",&cdc_min_dedx_int);
    fscanf(dedxfile,"%f max dedx\n",&cdc_max_dedx_int);
    fscanf(dedxfile,"%f dedx step\n",&cdc_dedx_step_int);
    fscanf(dedxfile,"\n");

    vector<Float_t> dedx_cf_alltheta;
    Float_t dedx_cf;

    // Store the scaling factors in vector<vector<double>>CDC_DEDX_CORRECTION;
  
    for (Int_t ii =0; ii<cdc_npoints_dedx_int; ii++) {
      for (Int_t jj=0; jj<cdc_npoints_theta_int; jj++) {
        fscanf(dedxfile,"%f\n",&dedx_cf);
        dedx_cf_alltheta.push_back(dedx_cf);
      }
      CDC_DEDX_INT_CORRECTION.push_back(dedx_cf_alltheta);
      dedx_cf_alltheta.clear();
    }
    fclose(dedxfile);

    return 0;
}

Float_t DSelector_cdc_dedx::Get_CDC_dEdx_amp_CF(Float_t theta_deg, Float_t thisdedx){
            Int_t thetabin1, thetabin2, dedxbin1, dedxbin2;

            if (theta_deg <= cdc_min_theta) {
              thetabin1 = 0;
              thetabin2 = thetabin1;
            } else if (theta_deg >= cdc_max_theta) { 
              thetabin1 = cdc_npoints_theta - 1;
              thetabin2 = thetabin1;
            } else {
              thetabin1 = (Int_t)((theta_deg - cdc_min_theta)/cdc_theta_step);  
              thetabin2 = thetabin1 + 1;  
            }
  
            if (thisdedx <= cdc_min_dedx) {
              dedxbin1 = 0;
              dedxbin2 = dedxbin1;
            } else if (thisdedx >= cdc_max_dedx) { 
              dedxbin1 = cdc_npoints_dedx - 1;
              dedxbin2 = dedxbin1;
            } else {
              dedxbin1 = (Int_t)((thisdedx - cdc_min_dedx)/cdc_dedx_step);
              dedxbin2 = dedxbin1 + 1;
            }

           Float_t dedxcf;
  
            if ((thetabin1 == thetabin2) && (dedxbin1 == dedxbin2)) {
  
              dedxcf = CDC_DEDX_AMP_CORRECTION[dedxbin1][thetabin1];
  
  	    } else if (thetabin1 == thetabin2) {  // interp dedx only
  
              Float_t cf1 = CDC_DEDX_AMP_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_AMP_CORRECTION[dedxbin2][thetabin1];
  
              Float_t dedx1 = cdc_min_dedx + dedxbin1*cdc_dedx_step;
              Float_t dedx2 = dedx1 + cdc_dedx_step;
  
              dedxcf = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);

  	    } else if (dedxbin1 == dedxbin2) {  // interp theta only
  
              Float_t cf1 = CDC_DEDX_AMP_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_AMP_CORRECTION[dedxbin1][thetabin2];
  
              Float_t theta1 = cdc_min_theta + thetabin1*cdc_theta_step;
              Float_t theta2 = theta1 + cdc_theta_step;
  
              dedxcf = cf1 + (theta_deg - theta1)*(cf2 - cf1)/(theta2-theta1);

            } else {
  
              Float_t cf1 = CDC_DEDX_AMP_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_AMP_CORRECTION[dedxbin2][thetabin1];
  
              Float_t dedx1 = cdc_min_dedx + dedxbin1*cdc_dedx_step;
              Float_t dedx2 = dedx1 + cdc_dedx_step;
  
              Float_t cf3 = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);
  
              cf1 = CDC_DEDX_AMP_CORRECTION[dedxbin1][thetabin2];
              cf2 = CDC_DEDX_AMP_CORRECTION[dedxbin2][thetabin2];
  
              dedx1 = cdc_min_dedx + dedxbin1*cdc_dedx_step;
              dedx2 = dedx1 + cdc_dedx_step;
  
              Float_t cf4 = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);
  
              Float_t theta1 = cdc_min_theta + thetabin1*cdc_theta_step;
              Float_t theta2 = theta1 + cdc_theta_step;
  
              dedxcf = cf3 + (theta_deg - theta1)*(cf4 - cf3)/(theta2-theta1);
  
            }
  
            return dedxcf;
}

Float_t DSelector_cdc_dedx::Get_CDC_dEdx_int_CF(Float_t theta_deg, Float_t thisdedx){
            Int_t thetabin1, thetabin2, dedxbin1, dedxbin2;

            if (theta_deg <= cdc_min_theta_int) {
              thetabin1 = 0;
              thetabin2 = thetabin1;
            } else if (theta_deg >= cdc_max_theta_int) { 
              thetabin1 = cdc_npoints_theta_int - 1;
              thetabin2 = thetabin1;
            } else {
              thetabin1 = (Int_t)((theta_deg - cdc_min_theta_int)/cdc_theta_step_int);  
              thetabin2 = thetabin1 + 1;  
            }
  
            if (thisdedx <= cdc_min_dedx_int) {
              dedxbin1 = 0;
              dedxbin2 = dedxbin1;
            } else if (thisdedx >= cdc_max_dedx_int) { 
              dedxbin1 = cdc_npoints_dedx_int - 1;
              dedxbin2 = dedxbin1;
            } else {
              dedxbin1 = (Int_t)((thisdedx - cdc_min_dedx_int)/cdc_dedx_step_int);
              dedxbin2 = dedxbin1 + 1;
            }

           Float_t dedxcf;
  
            if ((thetabin1 == thetabin2) && (dedxbin1 == dedxbin2)) {
  
              dedxcf = CDC_DEDX_INT_CORRECTION[dedxbin1][thetabin1];
  
  	    } else if (thetabin1 == thetabin2) {  // interp dedx only
  
              Float_t cf1 = CDC_DEDX_INT_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_INT_CORRECTION[dedxbin2][thetabin1];
  
              Float_t dedx1 = cdc_min_dedx_int + dedxbin1*cdc_dedx_step_int;
              Float_t dedx2 = dedx1 + cdc_dedx_step_int;
  
              dedxcf = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);

  	    } else if (dedxbin1 == dedxbin2) {  // interp theta only
  
              Float_t cf1 = CDC_DEDX_INT_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_INT_CORRECTION[dedxbin1][thetabin2];
  
              Float_t theta1 = cdc_min_theta_int + thetabin1*cdc_theta_step_int;
              Float_t theta2 = theta1 + cdc_theta_step_int;
  
              dedxcf = cf1 + (theta_deg - theta1)*(cf2 - cf1)/(theta2-theta1);

            } else {
  
              Float_t cf1 = CDC_DEDX_INT_CORRECTION[dedxbin1][thetabin1];
              Float_t cf2 = CDC_DEDX_INT_CORRECTION[dedxbin2][thetabin1];
  
              Float_t dedx1 = cdc_min_dedx_int + dedxbin1*cdc_dedx_step_int;
              Float_t dedx2 = dedx1 + cdc_dedx_step_int;
  
              Float_t cf3 = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);
  
              cf1 = CDC_DEDX_INT_CORRECTION[dedxbin1][thetabin2];
              cf2 = CDC_DEDX_INT_CORRECTION[dedxbin2][thetabin2];
  
              dedx1 = cdc_min_dedx_int + dedxbin1*cdc_dedx_step_int;
              dedx2 = dedx1 + cdc_dedx_step_int;
  
              Float_t cf4 = cf1 + (thisdedx - dedx1)*(cf2 - cf1)/(dedx2-dedx1);
  
              Float_t theta1 = cdc_min_theta_int + thetabin1*cdc_theta_step_int;
              Float_t theta2 = theta1 + cdc_theta_step_int;
  
              dedxcf = cf3 + (theta_deg - theta1)*(cf4 - cf3)/(theta2-theta1);
  
            }
  
            return dedxcf;
}


Int_t DSelector_cdc_dedx::Get_Hypo_Num(Particle_t find_particle, Int_t find_trackid) { 

            // find which charged track hypo is the particle and trackid sought

            Int_t found_hypo = -1;

            for (UInt_t loc_hypo =0; loc_hypo < Get_NumChargedHypos(); loc_hypo++) {

                  dChargedHypoWrapper->Set_ArrayIndex(loc_hypo); 

                  Int_t loc_trackid = dChargedHypoWrapper->Get_ID();
                  Int_t loc_pid = dChargedHypoWrapper->Get_PID();
                 
                  if (loc_trackid == find_trackid && loc_pid == find_particle) found_hypo = (Int_t) loc_hypo;
            } 

            return found_hypo;
}

Float_t DSelector_cdc_dedx::Correct_CDC_dEdx_amp(Int_t loc_hypo, Float_t theta) {

            dChargedHypoWrapper->Set_ArrayIndex((UInt_t)loc_hypo); 

            Float_t thisdedx = 1.0e6 *dChargedHypoWrapper->Get_dEdx_CDC();
		  
            if (thisdedx == 0) return 0;
		    
            Float_t theta_deg = 180.0/3.14159 * theta;

            Float_t dedxcf = 1.0;
            if (!IsMC) dedxcf = Get_CDC_dEdx_amp_CF(theta_deg, thisdedx);

	    return 1.06*dedxcf*thisdedx;
  
}

Float_t DSelector_cdc_dedx::Correct_CDC_dEdx_int(Int_t loc_hypo, Float_t theta) {

            dChargedHypoWrapper->Set_ArrayIndex((UInt_t)loc_hypo); 

            Float_t thisdedx = 1.0e6 *dChargedHypoWrapper->Get_dEdx_CDC_integral();
		  
            if (thisdedx == 0) return 0;
		    
            Float_t theta_deg = 180.0/3.14159 * theta;

            Float_t dedxcf = 1.0;
            if (!IsMC) dedxcf = Get_CDC_dEdx_int_CF(theta_deg, thisdedx);

	    return 0.94*dedxcf*thisdedx;
  
}

#endif // DSelector_cdc_dedx_h
