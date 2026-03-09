#include <iostream>
#include <vector>
#include <math.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> 
#include <unistd.h>

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TStyle.h"
#include "TTree.h"
#include "TProfile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2D.h"
#include "TFile.h"
#include "TPolyLine.h"

#include "TImage.h"

using namespace std;


void tw_tagm(char *file_name, int run_numb, int ch_min = 1, int ch_max = 102, int debug_counter_fit = 0){

  // 11/10/2020 A.S.
  // Simple script to determine TW corrections (checked with PrimEx data)
  // Use histograms filled in the TAGM TW plugin. Use Alex's B. fit function.
  // Not very robust fit, but seems to be Ok.

  int save_plot = 1;
  
  bool make_plots = false;

  // Use histograms from monitoring 
  //char file_name[300];
  //sprintf(file_name,"/work/halld/data_monitoring/RunPeriod-2019-01/mon_ver13/rootfiles/hd_root_0%d.root",run_numb);  
  TFile *file = new TFile (file_name,"R"); 


  char out_file[300];
  char image_file[300];
  char file_tmp[300];


  // Need to set up output dirs

  char period[20] = "empty4";

  //sprintf(out_file,"tagm_tw_new/%s/tagm_tw_new_%d.txt",period,run_numb);
  sprintf(image_file,"tagm_tw_new/%s/c4_%d.png",period,run_numb);  
  sprintf(file_tmp,"tagm_tw_new/%s/fit_%d.pdf",period,run_numb);
  sprintf(out_file,"tw-corr-%d.txt",run_numb);          // change to output of Alex Barnes' script
  sprintf(image_file,"c4_%d.png",run_numb);  
  sprintf(file_tmp,"fit_%d.pdf",run_numb);

  gStyle->SetOptFit(10);


  TGraph *gr1[103]={NULL};

  double cc0[103], cc1[103], cc2[103], cc3[103];

  double ecc0[103], ecc1[103], ecc2[103], ecc3[103];


  double cnt[103];
  double ecnt[103];

  const int PBIN = 125;

  double x[PBIN];
  double y[PBIN];

  double ex[PBIN];
  double ey[PBIN];

  double res[PBIN];
  double eres[PBIN];


  memset(cc0,0,sizeof(cc0)); memset(cc1,0,sizeof(cc1)); memset(cc2,0,sizeof(cc2)); memset(cc3,0,sizeof(cc3));

  memset(ecc0,0,sizeof(ecc0)); memset(ecc1,0,sizeof(ecc1)); memset(ecc2,0,sizeof(ecc2)); memset(ecc3,0,sizeof(ecc3));

  memset(cnt,0,sizeof(cnt));
  memset(ecnt,0,sizeof(ecnt));


  TH2I *hdt_ch[110];  
  
  Int_t ii = 0;


  
  TCanvas *c1 = new TCanvas("c1","c1", 200, 10, 450, 450);
  c1->Divide(1,2);
  
  TCanvas *c2 = new TCanvas("c2","c2", 700, 10, 450, 450);
  
  TCanvas *c3 = new TCanvas("c3","c3", 200, 610, 450, 450);
  

  for(int ch = ch_min; ch <= ch_max; ch++){

    char title[30];

    //  h_dt_vs_pp_tdc is a histogram with no tw corrections applied

    //    sprintf(title,"TAGM_TW/t-rf/h_dt_vs_pp_%d",ch);
    
    sprintf(title,"TAGM_TW/tdc-rf/h_dt_vs_pp_tdc_%d",ch);

    cnt[ch] = double(ch);

    cout << " COUNTER = " << ch << endl;

    int npoint = 0;

    memset(x,0,sizeof(x));
    memset(y,0,sizeof(y));
    memset(ex,0,sizeof(ex));
    memset(ey,0,sizeof(ey));

    cc0[ch] = -100.;
    
    hdt_ch[ch] = (TH2I*)file->Get(title);
    

    if(debug_counter_fit){
      c1->cd(1);
      hdt_ch[ch]->Draw();
    }

    double adc_min = -1;
    double adc_max = -1;


    int max_bin_scan = 1900;
    int int_win = 40;
    double tmin = 20;
    double tmax = 60;


    for(int slice = 0; slice < 125; slice++){

      TH1D *h1 = hdt_ch[ch]->ProjectionY("test",slice,slice);
      
      int ent = h1->GetEntries();
      
      // Not enough entries in the histogram to fit

      if(ent < 400) continue;

      if(debug_counter_fit)
	cout << h1->GetEntries() << " X bins " << h1->GetXaxis()->GetNbins()  << "  " <<  
	  h1->GetXaxis()->GetXmin() << endl;

      
      int sum_max = 0;
      
      double x_min = -100;
      double x_max = -100;
      
      int bin_min  = -100;
      
      //  Run sliding window to determine the peak to fit. Multiple peaks are possible if the bunch structure 
      //  is not resolved (ADC time is not well aligned). The RF is selected based on the ADC time. PS time can 
      //  be used instead. 

      for(int bin = 0; bin < max_bin_scan; bin++){

	int sum = 0;
	
	// bin size 0.05 ns
       
	  for(int ii = 0; ii < int_win; ii++){

	  int tmp = bin + ii;
	  
	  sum += h1->GetBinContent(tmp);
	
	}
        
	if(sum > sum_max){
	  sum_max = sum; 
	  bin_min = bin;
	}
	
      }
      

      x_min = h1->GetXaxis()->GetBinCenter(bin_min-tmin);
      x_max = h1->GetXaxis()->GetBinCenter(bin_min+tmax);
   

      if(debug_counter_fit)
	cout << " X min = " << x_min << " X max = " << x_max << " Bin min " << bin_min << endl;
            
      c1->cd(2);


      TF1 *f1 = new TF1("f1","gaus",-10.,10.);

      f1->SetParameter(0,sum_max);
      f1->SetParameter(1,0);
      f1->SetParameter(2,0.25);

      if(debug_counter_fit)
	h1->Fit("f1","","",x_min,x_max);
      else 
	h1->Fit("f1","Q","",x_min,x_max);


      
      // Restrict fitting range, the maximum allowed time shift is between [-1, 2] ns
      if(f1->GetParameter(1) < 2. && f1->GetParameter(1) > -1.){

	if(adc_min < 0) adc_min =  hdt_ch[ch]->GetXaxis()->GetBinCenter(slice);
	adc_max = hdt_ch[ch]->GetXaxis()->GetBinCenter(slice);

	x[npoint]  = hdt_ch[ch]->GetXaxis()->GetBinCenter(slice);
	y[npoint]  = f1->GetParameter(1);
	ey[npoint] = f1->GetParError(1);

	res[npoint]  = f1->GetParameter(2);
	eres[npoint] = f1->GetParError(2);

	npoint++;
      }


      if(debug_counter_fit){
	h1->Draw();
	c1->Update();
      }

      //      cout << "Slice = " << slice << "  X =  " << hdt_ch[ch]->GetXaxis()->GetBinCenter(slice) <<   endl;
      //      getchar();
      
    }  // Loop over slices



    if(npoint == 0) continue;

    gr1[ch]  = new TGraphErrors(npoint,x,y,ex,ey);

    if(make_plots) {
	    c2->cd();

	    gr1[ch]->SetMarkerStyle(20);
	    gr1[ch]->SetMarkerSize(0.6);
	    gr1[ch]->SetMarkerColor(4);
	    
	    gr1[ch]->SetMaximum(6);
	    gr1[ch]->SetMinimum(-3);
	    
	    gr1[ch]->SetTitle("");
	    gr1[ch]->GetXaxis()->SetTitle("ADC peak amplitude (count)");
	    gr1[ch]->GetYaxis()->SetTitle("T_{TAGM} - T_{RF}");
	    
	    char gr_title[30];
	    sprintf(gr_title,"TAGM channel %d",ch);
	    
	    gr1[ch]->SetTitle(gr_title);
	    

	    gr1[ch]->Draw("AP");
	    
	    // Not easy to fit with the power functuin as a free parameter, fix it 
	    // Fit results seem to be satisfactory.     
    }	    

    TF1 *func5 = new TF1("func5","[0]+[1]*(1/(x+[3]) )**[2]",200,2000);


    func5->SetParameter(0,-2.5);
    func5->SetParameter(1,100);
    func5->SetParameter(2,0.6);
    func5->SetParameter(3,-400);

    // Fit range has to be checked and adjusted for some channels, where ADC time is not well calibrated
    // In most cases it's not needed

    gr1[ch]->Fit("func5","EQ","",adc_min+60,adc_max-20);

    func5->FixParameter(2,func5->GetParameter(2));

    gr1[ch]->Fit("func5","EQ","",adc_min+60,adc_max-20);


    cc0[ch] = func5->GetParameter(0);
    cc1[ch] = func5->GetParameter(1);  
    cc2[ch] = func5->GetParameter(2);
    cc3[ch] = func5->GetParameter(3);

    ecc0[ch]  = func5->GetParameter(0);
    ecc1[ch]  = func5->GetParameter(1);
    ecc2[ch]  = func5->GetParameter(2);
    ecc3[ch]  = func5->GetParameter(3);

    if(make_plots) {
	    c2->Update();

    //    TF1 *func6 = new TF1("func6","-3.77616476483+1743.38747232*(1/(x+539.611427423))**0.821364275702",400,2000);
    //    func6->SetLineColor(1);
    //    func6->SetLineStyle(2);    
    //    func6->Draw("same");

	    
	    c3->cd();
	    

	    TGraphErrors *gr3  = new TGraphErrors(npoint,x,res,ex,eres);
	    
	    gr3->SetMarkerStyle(20);
	    gr3->SetMarkerSize(0.6);
	    gr3->SetMarkerColor(4);
	    
	    gr3->SetMaximum(0.9);
	    gr3->SetMinimum(0.);
	    
	    gr3->SetTitle("");
	    gr3->GetXaxis()->SetTitle("ADC peak amplitude");
	    gr3->GetYaxis()->SetTitle("#sigma_{T}  (ns)");
	    
	    gr3->SetTitle("");

	    //    TF1 *func10 = new TF1("func10","[0]/sqrt(x-[1])+[2]",600,2000);
	    //    gr3->Fit("func10","","",600,1600);


	    gr3->Draw("AP");
    }

  }   // Loop over TAGM counters
  


  // Print suspicious TAGM channels with the large value of C0
  for(int kk = 0; kk < 102; kk++){
    
    if(fabs(cc0[kk]) > 10)      
      cout << " COUNTER = " << kk <<  "  CC0 = " <<  cc0[kk]  << endl;    
    
  }
  



  // Draw some fit parameters

  TCanvas *c4 = new TCanvas("c4","c4", 700, 610, 450, 450);
    if(make_plots) {

	    c4->Divide(1,2);

	    c4->cd(1);

	    TGraphErrors *gr100  = new TGraphErrors(102,cnt,cc0,ecnt,ecc0);

	    gr100->SetMarkerStyle(20);
	    gr100->SetMarkerSize(0.6);
	    gr100->SetMarkerColor(4);
	    
	    gr100->SetMaximum(4);
	    gr100->SetMinimum(-4);
	    
	    gr100->SetTitle("");
	    gr100->GetXaxis()->SetTitle("ADC peak amplitude (count)");
	    gr100->GetYaxis()->SetTitle("T_{TAGM} - T_{RF}");
	    
	    gr100->SetTitle("");
	    
	    gr100->Draw("AP");
  
  

	    TGraphErrors *gr101  = new TGraphErrors(102,cnt,cc2,ecnt,ecc2);

	    c4->cd(2);
	    
	    gr101->SetMarkerStyle(20);
	    gr101->SetMarkerSize(0.6);
	    gr101->SetMarkerColor(4);
	    
	    gr101->SetTitle("");
	    gr101->GetXaxis()->SetTitle("ADC peak amplitude (count)");
	    gr101->GetYaxis()->SetTitle("T_{TAGM} - T_{RF}");
	    
	    gr101->SetTitle("");

	    gr101->Draw("AP");
    }

  // Write results
  
  FILE *data_file = fopen(out_file,"w");
  
  for(int ch = 1; ch <= 102; ch++){
    fprintf(data_file, "%4d  %4d  %14.7f  %14.7f  %14.7f %14.7f %14.7f\n",0, ch, cc0[ch], cc1[ch], cc2[ch], cc3[ch], 0. );
    
    if((ch == 9) || (ch == 27) || (ch == 81) || (ch == 99)){
      for(int single = 1; single < 6; single++){
	fprintf(data_file, "%4d  %4d  %14.7f  %14.7f  %14.7f %14.7f %14.7f\n",single,ch, cc0[ch], cc1[ch], cc2[ch], cc3[ch], 0. );
      }      
    }
    
    
  }
   
  fclose(data_file);
  
  
  if(save_plot){

    c4->SaveAs(image_file);


    TCanvas *c10 = new TCanvas("c10","c10", 10, 200, 600, 600);

    c10->Divide(5,5);
    
    int plot = 1;
    
    char tmp[300], tmp1[300];
    
    strcpy(tmp,file_tmp);  
    strcpy(tmp1,file_tmp);
    
    strcat(tmp,"(");   strcat(tmp1,")");
    
    for(int ch = 1; ch <= 102; ch++){
      
      c10->cd(plot);
      
      if(gr1[ch] != NULL){
	gr1[ch]->SetMarkerSize(0.25);
	gr1[ch]->Draw("AP");
      }
      
      if(ch == 25){
	c10->Print(tmp,"pdf");
	c10->Clear(); c10->Divide(5,5);
	plot = 0;
      }
      if(ch == 50){
	c10->Print(file_tmp,"pdf");
	c10->Clear(); c10->Divide(5,5);
	plot = 0;
      }
      if(ch == 75){
	c10->Print(file_tmp,"pdf");
	c10->Clear(); c10->Divide(5,5);
	plot = 0;
      }  
      if(ch == 100){
	c10->Print(file_tmp,"pdf");
	c10->Clear(); c10->Divide(5,5);
	plot = 0;
	
      }  
      
      plot++;
    }
    c10->Print(tmp1,"pdf");
        
  }  // End of save plots


  
}
