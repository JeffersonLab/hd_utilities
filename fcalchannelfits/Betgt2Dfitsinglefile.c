//
//  Betgt2Dfitsinglefile.c
//  
//
//  Created by Chandrasekhar Akondi on 3/23/20.
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
#include<TLatex.h>
#include<TF1.h>
#include "TRandom2.h"
#include "TF3.h"
#include "TError.h"
#include "Fit/BinData.h"
#include "Fit/Fitter.h"
#include "Math/WrappedMultiTF1.h"
#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "Fit/Chi2FCN.h"


/////////////////************  imgg innner  5 squares ******* SET - I      ********/////////////// START

void imggset1(int& numch, int *charry, ofstream& fitvaluefile, ofstream& lowstatfile, ofstream& badfitfile, double& pi0mass)
     {/////imggset  1

             double currentgain[2801];
            //// TFile *t=new TFile("BetargetAll.root");
         
                                      TFile *t=new TFile("Betgtite1.root");
    TProfile *hCurrentGainConstants = (TProfile *) gDirectory->Get("FCAL_Pi0TOF/CurrentGainConstants");
   
         for(int i=0;i<2800;i++)
         {
             
              cout<<i+1<<"\t\t"<<hCurrentGainConstants->GetBinContent(i+1)<<endl;
         }
                    TCanvas  *Canvas[6];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    //TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;  double sum=0; int sumcnt=0;  int count=0;/////  DONT REMOVE IT
                    gStyle->SetLineWidth(1);
                    gStyle->SetOptTitle(kFALSE);
         for(int i=0;i<=4;i++)
         {//// canvas loop

                 Canvas[i]=new TCanvas(Form("Canvas%d",i));
                 Canvas[i]->Divide(8,4);
                 stringstream a;a<<i+1;
                 outname=a.str();
  
             for(int j=0;j<32;j++)
              {////// each pad
                  
                                 if (count>=numch) continue;/////  DONT REMOVE IT

                 int chnumber=(32*i+j);
                  
                  //  if(charry[chnumber]<0) continue;///// if the #canvas x pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
                  Canvas[i]->cd(j+1);
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  if(projY->GetEntries()<=5000)
                  {/////checking the #entries  <=12K
                      projY->Draw();
lowstatfile<<-0.25<<"\t"<<-0.25<<"\t"<<charry[chnumber]<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                  }/////checking the #entries  <=12K
                  else
                  {  /////checking the #entries  <=12K else
        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.04,0.29);
        fSpectrum->SetLineWidth(1.0);
        binmax[chnumber] = projY->GetMaximumBin();
        xvaluebinmax[chnumber] = projY->GetXaxis()->GetBinCenter(binmax[chnumber]);///// X-value of the max
        hightbinmax[chnumber] = projY->GetMaximum();
        const Int_t nparSpectrum=10;
      //  Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.00001, 3e4, 4e-2, 2e-2,1e-2,1e-4,1e-4,1e-6};/// working for 0.04,0.29 for 10 parameters
            Double_t SpectrumParameters[nparSpectrum]={hightbinmax[chnumber],xvaluebinmax[chnumber],0.01, 9e2, 1, 1,1,1,1,1};
                      
        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
        fSpectrum->SetParameters(SpectrumParameters);

                     
                                    fSpectrum->SetParLimits(0,50, 1e8);////working
                                    fSpectrum->SetParLimits(1,0.03, 0.25);////working
                                    fSpectrum->SetParLimits(2,0.001, 0.1);////working
                                    fSpectrum->SetParLimits(3,50, 1e8);////working
                                    fSpectrum->SetParLimits(4,0.0025, 0.30);////working
                                   // fSpectrum->SetParLimits(5,0.0015, 0.3);////working0.04,0.29
                                    fSpectrum->SetParLimits(5,0.0015, 0.3);////working
                                  //  fSpectrum->SetParLimits(6,0.0025, 0.30);////working
                                   // fSpectrum->SetParLimits(7,0.0025, 0.3);
                                    //fSpectrum->SetParLimits(8,0.0015, 0.3);
                     
                      
                   projY->Fit("fSpectrum","","",0.04,0.29);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.04,0.29);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.04,0.29);
                  
                      
                      
                      
                                     double ratio=pi0mass/(fSpectrum->GetParameter(1));
                                     double errratio =(pi0mass/((fSpectrum->GetParameter(1))*(fSpectrum->GetParameter(1))))*(fSpectrum->GetParError(1));
                                     double gain =hCurrentGainConstants->GetBinContent(charry[chnumber])*(pi0mass/(fSpectrum->GetParameter(1)));
                      double errgain = hCurrentGainConstants->GetBinContent(charry[chnumber])*(pi0mass/((fSpectrum->GetParameter(1))*(fSpectrum->GetParameter(1))))*(fSpectrum->GetParError(1));
                                     
                    // cout<<charry[chnumber]<<"\t"<<hCurrentGainConstants->GetBinContent(charry[chnumber])<<"\t"<< ratio<<endl;
                      
                                if ( (gain<=1.5) && (errgain <=0.01) )
                                             {
                                         sum=sum+gain;
                                         sumcnt++;
                                            fitvaluefile<<ratio<<"\t"<<gain<<"\t"<<charry[chnumber]<<"\t"<<errratio<<"\t"<<errgain<<"\t"<<0<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<0<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()<<endl;
                                                 
                                                projY->SetLineColor(kGreen+3);
                    
                                            }
                                        else{
                                            
                                            double ave =sum/sumcnt;
                                            
                                            badfitfile<<ratio<<"\t"<<gain<<"\t"<<charry[chnumber]<<"\t"<<errratio<<"\t"<<errgain<<"\t"<<0<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<0<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()<<endl;
                                            }
                                    
                     
                  }/////////checking the #entries  <=12K else
                  
                                      count++;/////  DONT REMOVE IT
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet1-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       

     }///////imggset 1///////////////************  imgg innner  5 squares ******* SET - I      ********/////////////// END

/////////////////****** ******* SET - II     ********/////////////// START


void imggset2(int& numch, int *charry, ofstream& fitvaluefile, ofstream& lowstatfile, ofstream& badfitfile, double& pi0mass)
   {/////imgginnnerset 2


                double currentgain[2801];
                                 //// TFile *t=new TFile("BetargetAll.root");
                                     
                                                                  TFile *t=new TFile("Betgtite1.root");
       TProfile *hCurrentGainConstants = (TProfile *) gDirectory->Get("FCAL_Pi0TOF/CurrentGainConstants");
       
                  TCanvas  *Canvas[60];///////2150 channels = 22 canvas x 100 per canvas
                  char nl[256];
                  char histoname[256];TH1F *h[numch+1];
                  int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
 
                  TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                  string outname; double sum=0; int sumcnt=0;  int count=1;/////  DONT REMOVE IT
                  gStyle->SetLineWidth(1);
                  gStyle->SetOptTitle(kFALSE);
       for(int i=0;i<=19;i++)////// 42 canvases
       {//// canvas loop

               Canvas[i]=new TCanvas(Form("Canvas%d",i));
               Canvas[i]->Divide(12,9);
               stringstream a;a<<i+1;
               outname=a.str();

           for(int j=0;j<108;j++)
            {////// each pad
                
                                 if (count>numch) continue;/////  DONT REMOVE IT
               
              // cout<<"count="<<count<<"\t"<<numch<<endl;
                
               
              
               int chnumber=(108*i+j);
            
                cout<<charry[chnumber]<<"\t"<<"count="<<count<<"\t"<<numch<<endl;
                
              
                
        //    if ( (charry[chnumber]<644)|| (charry[chnumber]>648)) continue;////testing purpose
                
         
                
                Canvas[i]->cd(j+1);
                     
                
  TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                

                
                if(projY->GetEntries()<=5000)
                {

                          
                       projY->Draw();
                       projY->GetXaxis()->SetRangeUser(0.03,0.3);
lowstatfile<<-0.25<<"\t"<<-0.25<<"\t"<<charry[chnumber]<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                   }////// inner loop if   <=20000
                    
                else{/////// else entries >20K
                    
            

      TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.03,0.3);
      fSpectrum->SetLineWidth(1.0);
      binmax[chnumber] = projY->GetMaximumBin();
      xvaluebinmax[chnumber] = projY->GetXaxis()->GetBinCenter(binmax[chnumber]);///// X-value of the max
      hightbinmax[chnumber] = projY->GetMaximum();
      const Int_t nparSpectrum=10;
    Double_t SpectrumParameters[nparSpectrum]={hightbinmax[chnumber],xvaluebinmax[chnumber],0.001, 9e2, 1, 1,1,1,1,1};
    fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
                               fSpectrum->SetParameters(SpectrumParameters);
                                              fSpectrum->SetParLimits(0,0,1e8);
                                              fSpectrum->SetParLimits(1,0.003,0.25);
                                               fSpectrum->SetParLimits(2,0.001,0.009);
                                               fSpectrum->SetParLimits(3,0,1e8);
                                               fSpectrum->SetParLimits(4,0.0024,0.250);
                                            fSpectrum->SetParLimits(5,0.001,0.6);
                 projY->Fit("fSpectrum","","",0.03,0.3);
                 TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.03,0.3);
                 auto result = projY->Fit("fSpectrum","SQR");
                 result->Print();
                 projY->GetXaxis()->SetRangeUser(0.03,0.3);
               
                                              
                                                   double ratio=pi0mass/(fSpectrum->GetParameter(1));
                                                   double errratio =(pi0mass/((fSpectrum->GetParameter(1))*(fSpectrum->GetParameter(1))))*(fSpectrum->GetParError(1));
                                                   double gain =hCurrentGainConstants->GetBinContent(charry[chnumber])*(pi0mass/(fSpectrum->GetParameter(1)));
                                    double errgain = hCurrentGainConstants->GetBinContent(charry[chnumber])*(pi0mass/((fSpectrum->GetParameter(1))*(fSpectrum->GetParameter(1))))*(fSpectrum->GetParError(1));
               if ( (gain<=1.5) && (errgain <=0.01) )
                            {
                        sum=sum+gain;
                        sumcnt++;
                           fitvaluefile<<ratio<<"\t"<<gain<<"\t"<<charry[chnumber]<<"\t"<<errratio<<"\t"<<errgain<<"\t"<<0<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<0<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()<<endl;
                               
                                projY->SetLineColor(kGreen+3);
   
                           }
                       else{
                           
                            double ave =sum/sumcnt;
                           
                           badfitfile<<ratio<<"\t"<<gain<<"\t"<<charry[chnumber]<<"\t"<<errratio<<"\t"<<errgain<<"\t"<<0<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<0<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()<<endl;
                           }
                   
                
                }////else entries > 20K
             
              
                count++;/////  DONT REMOVE IT
            }///// each  pad
            
           Canvas[i]->Print(Form("Betgt2DfitinnersquareSet2-%s.pdf",outname.c_str()));
           Canvas[i]->Update();
           
           
       }////////canvas loop
     


   }//////imggset 2 ///////////////////////////***** ******* SET - 2     ********/////////////// END


/////////********///###########********/////////******$$$$$$$$$$*******########//////***************$$$$$$$$$$$$***********

/////////////////****** ******* SET - III     ********/////////////// START


void imggset3(int& numch, int *charry, ofstream& fitvaluefile,ofstream& lowstatfile, ofstream& badfitfile,double& pi0mass)
   {/////imgginnnerset 3


                double currentgain[2801];
                                   //// TFile *t=new TFile("BetargetAll.root");
                                     
                                                                  TFile *t=new TFile("Betgtite1.root");
       TProfile *hCurrentGainConstants = (TProfile *) gDirectory->Get("FCAL_Pi0TOF/CurrentGainConstants");
       
                  TCanvas  *Canvas[6];///////496channels = 5canvas x 100 per canvas
                  char nl[256];
                  char histoname[256];TH1F *h[numch+1];
                  int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
         
                  TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
       string outname;   int count=0; double sum=0; int sumcnt=0;
                  gStyle->SetLineWidth(1);
                  gStyle->SetOptTitle(kFALSE);
       for(int i=0;i<=4;i++)////// 42 canvases
           
           
       {//// canvas loop

               Canvas[i]=new TCanvas(Form("Canvas%d",i));
               Canvas[i]->Divide(10,10);
                      //    Canvas[i]->Divide(6,6);
               stringstream a;a<<i+1;
               outname=a.str();

           for(int j=0;j<100;j++)////working
           
                  //  for(int j=0;j<36;j++)
               
            {////// each pad
                            if (count>=numch) continue;/////  DONT REMOVE IT
               int chnumber=(100*i+j);   ///working
                
                             //  int chnumber=(36*i+j);

                Canvas[i]->cd(j+1);
                     
                
  TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel

                if(projY->GetEntries()<=5000){ ///// entries <20K
                    projY->Draw();
                    projY->GetXaxis()->SetRangeUser(0.03,0.3);
                    lowstatfile<<-0.25<<"\t"<<-0.25<<"\t"<<charry[chnumber]<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                    }  ///// entries <20K
           
                 else{///////  entries >20K
                    
            

      TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.03,0.3);
      fSpectrum->SetLineWidth(1.0);
      binmax[chnumber] = projY->GetMaximumBin();
      xvaluebinmax[chnumber] = projY->GetXaxis()->GetBinCenter(binmax[chnumber]);///// X-value of the max
      hightbinmax[chnumber] = projY->GetMaximum();
      const Int_t nparSpectrum=10;
                    
                    
                               Double_t SpectrumParameters[nparSpectrum]={hightbinmax[chnumber],xvaluebinmax[chnumber],0.001, 9e2, 1, 1,1,1,1,1};
                                                                    
                        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
                               fSpectrum->SetParameters(SpectrumParameters);
                                                                    
                                            fSpectrum->SetParLimits(0,0, 1e8);////working
                                            fSpectrum->SetParLimits(1,0.003, 0.25);////working
                                            fSpectrum->SetParLimits(2,0.001, 0.009);////working
                                            fSpectrum->SetParLimits(3,0, 1e8);////working
                                            fSpectrum->SetParLimits(4,0.0024, 0.250);////working
                                            fSpectrum->SetParLimits(5,0.001, 0.6);////working
                 projY->Fit("fSpectrum","","",0.03,0.3);
                 TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.03,0.3);
                 auto result = projY->Fit("fSpectrum","SQR");
                 result->Print();
                 projY->GetXaxis()->SetRangeUser(0.03,0.3);
                   
                                                        
                                                               double ratio=pi0mass/(fSpectrum->GetParameter(1));
                                                               double errratio =(pi0mass/((fSpectrum->GetParameter(1))*(fSpectrum->GetParameter(1))))*(fSpectrum->GetParError(1));
                                                               double gain =hCurrentGainConstants->GetBinContent(charry[chnumber])*(pi0mass/(fSpectrum->GetParameter(1)));
                                                double errgain = hCurrentGainConstants->GetBinContent(charry[chnumber])*(pi0mass/((fSpectrum->GetParameter(1))*(fSpectrum->GetParameter(1))))*(fSpectrum->GetParError(1));
                                     
                    if ( (gain<=1.5) && (errgain <=0.01) )
                                             {
                                         
                                                 sum=sum+gain;
                                                 sumcnt++;
                                                 
                                                 
                                            fitvaluefile<<ratio<<"\t"<<gain<<"\t"<<charry[chnumber]<<"\t"<<errratio<<"\t"<<errgain<<"\t"<<0<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<0<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()<<endl;
                                    
                                                 projY->SetLineColor(kGreen+3);
                                            }
                     
                                        else{
                                             //  cout<<"sum=    "<<sum<<"sumcnt="<<sumcnt<<endl;
                                            double ave =sum/sumcnt;
                                            
                                            badfitfile<<ratio<<"\t"<<gain<<"\t"<<charry[chnumber]<<"\t"<<errratio<<"\t"<<errgain<<"\t"<<0<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<0<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()<<endl;
                                            
                                        // badfitfile<<ave<<"\t"<<ave<<"\t"<<charry[chnumber]<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<< 0<<endl;
                                            
                                            
                                            }
                  
                     
                }////else entries > 12K
                

                
                
                
                
                
                
               
                                count++;/////  DONT REMOVE IT
            }///// each  pad
            
           Canvas[i]->Print(Form("Betgt2DfitinnersquareSet3-%s.pdf",outname.c_str()));
           Canvas[i]->Update();
           
           
       }////////canvas loop
     


   }//////imggset  ///////////////////////////***** ******* SET - III    ********/////////////// END

/////////////////// plotting the Gains vs Channels and Sigma vs Channels

void plotGainsVsCh() ////// plotting the Gains vs Channels and Sigma vs Channels
{
    

    
    
    
    

       /////  ratio gain charry errratio errgain 0 fSpectrum->GetParameter(2) 0 fSpectrum->GetParError(2)  r
    
           TCanvas *GainVsCh=new TCanvas("GainVsCh","GainVsCh",600,600);
           TMultiGraph *mg = new TMultiGraph();
    TGraphErrors *hA= new TGraphErrors("goodchannelsfitparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
             hA->SetMarkerColor(kGreen);
             hA->SetLineColor(kGreen);
             hA->SetMarkerStyle(24);
             hA->SetMarkerSize(1.0);
             mg->Add(hA);
    TGraphErrors *hB= new TGraphErrors("fitbadparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
             hB->SetMarkerColor(kRed);
             hB->SetLineColor(kRed);
             hB->SetMarkerStyle(24);
             hB->SetMarkerSize(1.0);
             mg->Add(hB);
    TGraphErrors *hC= new TGraphErrors("lowstatchannelsfitparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
             hC->SetMarkerColor(kBlue);
             hC->SetLineColor(kBlue);
             hC->SetMarkerStyle(24);
             hC->SetMarkerSize(1.0);
             mg->Add(hC);
mg->GetXaxis()->SetTitle("new Gain = old gain #times (m_{#pi^{0}} / m_{2#gamma})");
mg->GetYaxis()->SetTitle("FCAL channels");
    mg->GetYaxis()->SetTitleOffset(1.0);
    mg->GetYaxis()->SetLabelSize(0.02);
                    mg->Draw("AP");
    
    GainVsCh->Print("GainVsCh.pdf");
    
    
    
             TCanvas *GainVsChgoodfit=new TCanvas("GainVsChgoodfit","GainVsChgoodfit",600,600);
             TGraphErrors *hgaingoodfit= new TGraphErrors("goodchannelsfitparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
             hgaingoodfit->SetMarkerColor(kGreen+2);
             hgaingoodfit->SetLineColor(kGreen+2);
             hgaingoodfit->SetMarkerStyle(24);
             hgaingoodfit->SetMarkerSize(1.0);
    hgaingoodfit->GetXaxis()->SetTitle("Gain = Oldgain #times (m_{#pi^{0}} / m_{2#gamma})");
    hgaingoodfit->GetYaxis()->SetTitle("FCAL channels");
    hgaingoodfit->GetYaxis()->SetTitleOffset(1.0);
    hgaingoodfit->GetYaxis()->SetLabelSize(0.02);
             hgaingoodfit->Draw("AP");
    GainVsChgoodfit->Print("GainVsChgoodfit.pdf");
    TCanvas *GainVsChbadfit=new TCanvas("GainVsChbadfit","GainVsChbadfit",600,600);
    TGraphErrors *hgainbadfit= new TGraphErrors("fitbadparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
    hgainbadfit->SetMarkerColor(kRed);
    hgainbadfit->SetLineColor(kRed);
    hgainbadfit->SetMarkerStyle(24);
    hgainbadfit->SetMarkerSize(1.0);
    hgainbadfit->GetXaxis()->SetTitle("Gain = Oldgain #times (m_{#pi^{0}} / m_{2#gamma})");
    hgainbadfit->GetYaxis()->SetTitle("FCAL channels");
    hgainbadfit->GetYaxis()->SetTitleOffset(1.0);
    hgainbadfit->GetYaxis()->SetLabelSize(0.02);
    hgainbadfit->Draw("AP");
        GainVsChbadfit->Print("GainVsChbadfit.pdf");
    
    
    
    
       
        
                TCanvas *RatioVsCh=new TCanvas("RatioVsCh","RatioVsCh",600,600);
        
               TMultiGraph *mg0 = new TMultiGraph();

        TGraphErrors *hA0= new TGraphErrors("goodchannelsfitparameters.txt","%lg%*lg%lg%lg%*lg%lg%*lg%*lg%*lg%*lg");
                 hA0->SetMarkerColor(kGreen);
                 hA0->SetLineColor(kGreen);
                 hA0->SetMarkerStyle(24);
                 hA0->SetMarkerSize(1.0);
                 mg0->Add(hA0);

        TGraphErrors *hB0= new TGraphErrors("fitbadparameters.txt","%lg%*lg%lg%lg%*lg%lg%*lg%*lg%*lg%*lg");
    
                 hB0->SetMarkerColor(kRed);
                 hB0->SetLineColor(kRed);
                 hB0->SetMarkerStyle(24);
                 hB0->SetMarkerSize(1.0);
                 mg0->Add(hB0);
        

        TGraphErrors *hC0= new TGraphErrors("lowstatchannelsfitparameters.txt","%lg%*lg%lg%lg%*lg%lg%*lg%*lg%*lg%*lg");
                 hC0->SetMarkerColor(kBlue);
                 hC0->SetLineColor(kBlue);
                 hC0->SetMarkerStyle(24);
                 hC0->SetMarkerSize(1.0);
                 mg0->Add(hC0);
        
                            
    mg0->GetXaxis()->SetTitle("Ratio = m_{#pi^{0}} / m_{2#gamma}");
    mg0->GetYaxis()->SetTitle("FCAL channels");
    mg0->GetYaxis()->SetTitleOffset(1.0);
    mg0->GetYaxis()->SetLabelSize(0.02);
                        mg0->Draw("AP");
        
        RatioVsCh->Print("RatioVsCh.pdf");
    
             TCanvas *ratioVsChgoodfit=new TCanvas("ratioVsChgoodfit","ratioVsChgoodfit",600,600);
             TGraphErrors *hratiogoodfit= new TGraphErrors("goodchannelsfitparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
             hratiogoodfit->SetMarkerColor(kGreen+2);
             hratiogoodfit->SetLineColor(kGreen+2);
             hratiogoodfit->SetMarkerStyle(24);
             hratiogoodfit->SetMarkerSize(1.0);
    hratiogoodfit->GetXaxis()->SetTitle("Ratio = m_{#pi^{0}} / m_{2#gamma}");
    hratiogoodfit->GetYaxis()->SetTitle("FCAL channels");
    hratiogoodfit->GetYaxis()->SetTitleOffset(1.0);
    hratiogoodfit->GetYaxis()->SetLabelSize(0.02);
             hratiogoodfit->Draw("AP");
    ratioVsChgoodfit->Print("ratioVsChgoodfit.pdf");
    TCanvas *ratioVsChbadfit=new TCanvas("ratioVsChbadfit","ratioVsChbadfit",600,600);
    TGraphErrors *hratiobadfit= new TGraphErrors("fitbadparameters.txt","%*lg%lg%lg%*lg%lg%lg%*lg%*lg%*lg%*lg");
    hratiobadfit->SetMarkerColor(kRed);
    hratiobadfit->SetLineColor(kRed);
    hratiobadfit->SetMarkerStyle(24);
    hratiobadfit->SetMarkerSize(1.0);
    hratiobadfit->GetXaxis()->SetTitle("Ratio = m_{#pi^{0}} / m_{2#gamma}");
    hratiobadfit->GetYaxis()->SetTitle("FCAL channels");
    hratiobadfit->GetYaxis()->SetTitleOffset(1.0);
    hratiobadfit->GetYaxis()->SetLabelSize(0.02);
    hratiobadfit->Draw("AP");
        ratioVsChbadfit->Print("ratioVsChbadfit.pdf");
    
   /////  ratio gain charry errratio errgain 0 fSpectrum->GetParameter(2) 0 fSpectrum->GetParError(2)  r
    
                TCanvas *SigmaVsCh=new TCanvas("SigmaVsCh","SigmaVsCh",600,600);
        
               TMultiGraph *mg1 = new TMultiGraph();

        TGraphErrors *hA1= new TGraphErrors("goodchannelsfitparameters.txt","%*lg%*lg%lg%*lg%*lg%*lg%lg%lg%lg%*lg");
                 hA1->SetMarkerColor(kGreen);
                 hA1->SetLineColor(kGreen);
                 hA1->SetMarkerStyle(24);
                 hA1->SetMarkerSize(1.0);
                 mg1->Add(hA1);
        
   
            TGraphErrors *hB1= new TGraphErrors("fitbadparameters.txt","%*lg%*lg%lg%*lg%*lg%*lg%lg%lg%lg%*lg");
                 hB1->SetMarkerColor(kRed);
                 hB1->SetLineColor(kRed);
                 hB1->SetMarkerStyle(24);
                 hB1->SetMarkerSize(1.0);
                 mg1->Add(hB1);
        TGraphErrors *hC1= new TGraphErrors("lowstatchannelsfitparameters.txt","%*lg%*lg%lg%*lg%*lg%*lg%lg%lg%lg%*lg");
                 hC1->SetMarkerColor(kBlue);
                 hC1->SetLineColor(kBlue);
                 hC1->SetMarkerStyle(24);
                 hC1->SetMarkerSize(1.0);
                 mg1->Add(hC1);
        
                            
    mg1->GetXaxis()->SetTitle("FCAL channels");
    mg1->GetYaxis()->SetTitle("Sigma (GeV)");
    mg1->GetYaxis()->SetTitleOffset(1.0);
   mg1->GetYaxis()->SetLabelSize(0.02);
                        mg1->Draw("AP");
        
        SigmaVsCh->Print("ChVsSigma.pdf");
    
    
    
    
             TCanvas *sigmaVsChgoodfit=new TCanvas("sigmaVsChgoodfit","sigmaVsChgoodfit",600,600);
             TGraphErrors *hsigmagoodfit= new TGraphErrors("goodchannelsfitparameters.txt","%*lg%*lg%lg%*lg%*lg%*lg%lg%lg%lg%*lg");
             hsigmagoodfit->SetMarkerColor(kGreen+2);
             hsigmagoodfit->SetLineColor(kGreen+2);
             hsigmagoodfit->SetMarkerStyle(24);
             hsigmagoodfit->SetMarkerSize(1.0);
    hsigmagoodfit->GetXaxis()->SetTitle("FCAL channels");
    hsigmagoodfit->GetYaxis()->SetTitle("Sigma (GeV)");
    hsigmagoodfit->GetYaxis()->SetTitleOffset(1.0);
    hsigmagoodfit->GetYaxis()->SetLabelSize(0.02);
             hsigmagoodfit->Draw("AP");
    sigmaVsChgoodfit->Print("sigmaVsChgoodfit.pdf");
    TCanvas *sigmaVsChbadfit=new TCanvas("sigmaVsChbadfit","sigmaVsChbadfit",600,600);
    TGraphErrors *hsigmabadfit= new TGraphErrors("fitbadparameters.txt","%*lg%*lg%lg%*lg%*lg%*lg%lg%lg%lg%*lg");
    hsigmabadfit->SetMarkerColor(kRed);
    hsigmabadfit->SetLineColor(kRed);
    hsigmabadfit->SetMarkerStyle(24);
    hsigmabadfit->SetMarkerSize(1.0);
    hsigmabadfit->GetXaxis()->SetTitle("FCAL channels");
    hsigmabadfit->GetYaxis()->SetTitle("Sigma (GeV)");
    hsigmabadfit->GetYaxis()->SetTitleOffset(1.0);
    hsigmabadfit->GetYaxis()->SetLabelSize(0.02);
    hsigmabadfit->Draw("AP");
        sigmaVsChbadfit->Print("sigmaVsChbadfit.pdf");

    
    
    
}


//////////////***************************************
void Betgt2Dfitsinglefile()
{//// main starting
    
    
    double PDGPi0Mass = 0.1349766;
      
          ////////   number of channels in each inner square

         ifstream f1;f1.open("FCALinnersquare1.txt",ios::in);
         int insquare1ch[20],inr1ch;
         ifstream f2;f2.open("FCALinnersquare2.txt",ios::in);
         int insquare2ch[30],inr2ch;
         ifstream f3;f3.open("FCALinnersquare3.txt",ios::in);
         int insquare3ch[40],inr3ch;
         ifstream f4;f4.open("FCALinnersquare4.txt",ios::in);
         int insquare4ch[50],inr4ch;
         ifstream f5;f5.open("FCALinnersquare5.txt",ios::in);
         int insquare5ch[50],inr5ch;
         ifstream f6;f6.open("FCALinnersquare6.txt",ios::in);
         int insquare6ch[260],inr6ch;
         ifstream f7;f7.open("FCALinnersquare7.txt",ios::in);
         int insquare7ch[270],inr7ch;
         ifstream f8;f8.open("FCALinnersquare8.txt",ios::in);
         int insquare8ch[280],inr8ch;
         ifstream f9;f9.open("FCALinnersquare9.txt",ios::in);
         int insquare9ch[290],inr9ch;
         ifstream f10;f10.open("FCALinnersquare10.txt",ios::in);
         int insquare10ch[300],inr10ch;
         ifstream f11;f11.open("FCALinnersquare11.txt",ios::in);
         int insquare11ch[250],inr11ch;
         ifstream f12;f12.open("FCALinnersquare12.txt",ios::in);
         int insquare12ch[250],inr12ch;
         ifstream f13;f13.open("FCALinnersquare13.txt",ios::in);
         int insquare13ch[250],inr13ch;
         ifstream f14;f14.open("FCALinnersquare14.txt",ios::in);
         int insquare14ch[250],inr14ch;
         ifstream f15;f15.open("FCALinnersquare15.txt",ios::in);
         int insquare15ch[250],inr15ch;
         ifstream f16;f16.open("FCALinnersquare16.txt",ios::in);
         int insquare16ch[250],inr16ch;
         ifstream f17;f17.open("FCALinnersquare17.txt",ios::in);
         int insquare17ch[250],inr17ch;
         ifstream f18;f18.open("FCALinnersquare18.txt",ios::in);
         int insquare18ch[260],inr18ch;
         ifstream f19;f19.open("FCALinnersquare19.txt",ios::in);
         int insquare19ch[270],inr19ch;
         ifstream f20;f20.open("FCALinnersquare20.txt",ios::in);
         int insquare20ch[280],inr20ch;
         ifstream f21;f21.open("FCALinnersquare21.txt",ios::in);
         int insquare21ch[270],inr21ch;
         ifstream f22;f22.open("FCALinnersquare22.txt",ios::in);
         int insquare22ch[260],inr22ch;
         ifstream f23;f23.open("FCALinnersquare23.txt",ios::in);
         int insquare23ch[250],inr23ch;
         ifstream f24;f24.open("FCALinnersquare24.txt",ios::in);
         int insquare24ch[240],inr24ch;
         ifstream f25;f25.open("FCALinnersquare25.txt",ios::in);
         int insquare25ch[220],inr25ch;
         ifstream f26;f26.open("FCALinnersquare26.txt",ios::in);
         int insquare26ch[210],inr26ch;
         ifstream f27;f27.open("FCALinnersquare27.txt",ios::in);
         int insquare27ch[190],inr27ch;
         ifstream f28;f28.open("FCALinnersquare28.txt",ios::in);
         int insquare28ch[170],inr28ch;

         
         ifstream fff;fff.open("secondsetchannels.txt",ios::in);
         int fffch[2200],inrfffch;
         
      int set1squares[300],set1squaresch,set1squareschcnt=0;////////   set -I   inner 5 rings
      int set2squares[2700],set2squaresch,set2squareschcnt=0;////////   set -II   rest of the channels
      int set3squares[500],set3squaresch,set3squareschcnt=0;////////  outer channels
    

   /////// SET -1     squares 1 to squares 5
      
                   while(f1>>inr1ch)
                   {////f1>>inr1ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr1ch==i)
                               {//////matching channels
                                 set1squares[set1squareschcnt]=inr1ch;
                                 set1squareschcnt++;
                                }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                    }/////f1>>inr1ch
               
                    while(f2>>inr2ch)
                    {////f2>>inr2ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<2800;i++)
                          if (inr2ch==i)
                                {//////matching channels
                                  set1squares[set1squareschcnt]=inr2ch;
                                   set1squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f2>>inr2ch
        
    while(f3>>inr3ch)
                      {////f3>>inr3ch
                      for(int i=0;i<2800;i++)
                          {////////for(int i=0;i<2800;i++)
                            if (inr3ch==i)
                                  {//////matching channels
                                   set1squares[set1squareschcnt]=inr3ch;
                                   set1squareschcnt++;
                                   }///// matching channels
                           }////////for(int i=0;i<2800;i++)
                       }/////f3>>inr3ch
        
     while(f4>>inr4ch)
                    {////f4>>inr4ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<2800;i++)
                          if (inr4ch==i)
                                {//////matching channels
                                  set1squares[set1squareschcnt]=inr4ch;
                                  set1squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f4>>inr4ch
        
    while(f5>>inr5ch)
                    {////f5>>inr5ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<2800;i++)
                          if (inr5ch==i)
                                {//////matching channels
                               set1squares[set1squareschcnt]=inr5ch;
                               set1squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f5>>inr5ch
    
    
/////// SET -2     Rest of thechannels
       

    
                       while(f6>>inr6ch)
                       {////f6>>inr6ch
                       for(int i=0;i<2800;i++)
                           {////////for(int i=0;i<2800;i++)
                             if (inr6ch==i)
                                   {//////matching channels
                                     set2squares[set2squareschcnt]=inr6ch;
                                     set2squareschcnt++;
                                    }///// matching channels
                             }////////for(int i=0;i<2800;i++)
                        }/////f6>>inr6ch
                   
        while(f7>>inr7ch)
                        {////f7>>inr7ch
                        for(int i=0;i<2800;i++)
                            {////////for(int i=0;i<2800;i++)
                              if (inr7ch==i)
                                    {//////matching channels
                                      set2squares[set2squareschcnt]=inr7ch;
                                       set2squareschcnt++;
                                     }///// matching channels
                             }////////for(int i=0;i<2800;i++)
                         }/////f7>>inr7ch
            
        while(f8>>inr8ch)
                          {////f8>>inr8ch
                          for(int i=0;i<2800;i++)
                              {////////for(int i=0;i<2800;i++)
                                if (inr8ch==i)
                                      {//////matching channels
                                       set2squares[set2squareschcnt]=inr8ch;
                                       set2squareschcnt++;
                                       }///// matching channels
                               }////////for(int i=0;i<2800;i++)
                           }/////f8>>inr8ch
            
         while(f9>>inr9ch)
                        {////f9>>inr9ch
                        for(int i=0;i<2800;i++)
                            {////////for(int i=0;i<2800;i++)
                              if (inr9ch==i)
                                    {//////matching channels
                                      set2squares[set2squareschcnt]=inr9ch;
                                      set2squareschcnt++;
                                     }///// matching channels
                             }////////for(int i=0;i<2800;i++)
                         }/////f9>>inr9ch
         

  
        while(f10>>inr10ch)
                        {////f10>>inr10ch
                        for(int i=0;i<2800;i++)
                            {////////for(int i=0;i<2800;i++)
                              if (inr10ch==i)
                                    {//////matching channels
                                   set2squares[set2squareschcnt]=inr10ch;
                                   set2squareschcnt++;
                                     }///// matching channels
                             }////////for(int i=0;i<2800;i++)
                         }/////f10>>inr10ch

    
        while(f11>>inr11ch)
                   {////f11>>inr11ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr11ch==i)
                               {//////matching channels
                                 set2squares[set2squareschcnt]=inr11ch;
                                 set2squareschcnt++;
                                }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                    }/////f11>>inr11ch
    
 
    
    while(f12>>inr12ch)
                    {////f12>>inr12ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<2800;i++)
                          if (inr12ch==i)
                                {//////matching channels
                                  set2squares[set2squareschcnt]=inr12ch;
                                   set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f12>>inr12ch
    
    while(f13>>inr13ch)
                      {////f13>>inr13ch
                      for(int i=0;i<2800;i++)
                          {////////for(int i=0;i<2800;i++)
                            if (inr13ch==i)
                                  {//////matching channels
                                   set2squares[set2squareschcnt]=inr13ch;
                                   set2squareschcnt++;
                                   }///// matching channels
                           }////////for(int i=0;i<2800;i++)
                       }/////f13>>inr13ch
   
   
    
     while(f14>>inr14ch)
                    {////f14>>inr14ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr14ch==i)
                                {//////matching channels
                                  set2squares[set2squareschcnt]=inr14ch;
                                  set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f14>>inr14ch
        
    while(f15>>inr15ch)
                    {////f15>>inr15ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr15ch==i)
                                {//////matching channels
                               set2squares[set2squareschcnt]=inr15ch;
                               set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f15>>inr15ch
  
                   while(f16>>inr16ch)
                   {////f16>>inr16ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<21300;i++)
                         if (inr16ch==i)
                               {//////matching channels
                                 set2squares[set2squareschcnt]=inr16ch;
                                 set2squareschcnt++;
                                }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                    }/////f16>>inr16ch
               
                    while(f17>>inr17ch)
                    {////f17>>inr17ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr17ch==i)
                                {//////matching channels
                                  set2squares[set2squareschcnt]=inr17ch;
                                   set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f17>>inr17ch
     
  
    
    while(f18>>inr18ch)
                      {////f18>>inr18ch
                      for(int i=0;i<2800;i++)
                          {////////for(int i=0;i<21300;i++)
                            if (inr18ch==i)
                                  {//////matching channels
                                   set2squares[set2squareschcnt]=inr18ch;
                                   set2squareschcnt++;
                                   }///// matching channels
                           }////////for(int i=0;i<2800;i++)
                       }/////f18>>inr18ch
        
     while(f19>>inr19ch)
                    {////f19>>inr19ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr19ch==i)
                                {//////matching channels
                                  set2squares[set2squareschcnt]=inr19ch;
                                  set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f19>>inr19ch
        
    while(f20>>inr20ch)
                    {////f20>>inr20ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr20ch==i)
                                {//////matching channels
                               set2squares[set2squareschcnt]=inr20ch;
                               set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f20>>inr20ch
    
    
                   while(f21>>inr21ch)
                   {////f21>>inr21ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<21300;i++)
                         if (inr21ch==i)
                               {//////matching channels
                                 set2squares[set2squareschcnt]=inr21ch;
                                 set2squareschcnt++;
                                }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                    }/////f21>>inr21ch
               
                    while(f22>>inr22ch)
                    {////f22>>inr22ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr22ch==i)
                                {//////matching channels
                                  set2squares[set2squareschcnt]=inr22ch;
                                   set2squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f22>>inr22ch
        
    while(f23>>inr23ch)
                      {////f23>>inr23ch
                      for(int i=0;i<2800;i++)
                          {////////for(int i=0;i<21300;i++)
                            if (inr23ch==i)
                                  {//////matching channels
                                   set2squares[set2squareschcnt]=inr23ch;
                                   set2squareschcnt++;
                                   }///// matching channels
                           }////////for(int i=0;i<2800;i++)
                       }/////f23>>inr23ch
    
  
    
    
    
    
    //////////////// OUTER CHANNELS /////////////////////////

    
     while(f24>>inr24ch)
                    {////f24>>inr24ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr24ch==i)
                                {//////matching channels
                                  set3squares[set3squareschcnt]=inr24ch;
                                  set3squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f24>>inr24ch

    while(f25>>inr25ch)
                    {////f25>>inr25ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr25ch==i)
                                {//////matching channels
                               set3squares[set3squareschcnt]=inr25ch;
                               set3squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f25>>inr25ch

                   while(f26>>inr26ch)
                   {////f26>>inr26ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<21300;i++)
                         if (inr26ch==i)
                               {//////matching channels
                                 set3squares[set3squareschcnt]=inr26ch;
                                 set3squareschcnt++;
                                }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                    }/////f26>>inr26ch
               
                    while(f27>>inr27ch)
                    {////f27>>inr27ch
                    for(int i=0;i<2800;i++)
                        {////////for(int i=0;i<21300;i++)
                          if (inr27ch==i)
                                {//////matching channels
                                  set3squares[set3squareschcnt]=inr27ch;
                                   set3squareschcnt++;
                                 }///// matching channels
                         }////////for(int i=0;i<2800;i++)
                     }/////f27>>inr27ch
        
    while(f28>>inr28ch)
                      {////f28>>inr28ch
                      for(int i=0;i<2800;i++)
                          {////////for(int i=0;i<21300;i++)
                            if (inr28ch==i)
                                  {//////matching channels
                                   set3squares[set3squareschcnt]=inr28ch;
                                   set3squareschcnt++;
                                   }///// matching channels
                           }////////for(int i=0;i<2800;i++)
                       }/////f28>>inr28ch
  
    
    
    // cout<<set1squareschcnt<<"\t"<<set2squareschcnt<<endl;
    
                                ofstream fitvaluefile,lowstatvaluefile,badvaluefile;
                                string fitfilename,lowstatfilename,badfitfilename;
                                fitfilename ="goodchannelsfitparameters.txt";
                                lowstatfilename ="lowstatchannelsfitparameters.txt";
                                badfitfilename ="fitbadparameters.txt";
                  
                  
                  
                  
    fitvaluefile.open(fitfilename.c_str(),ios::out) ;
                                lowstatvaluefile.open(lowstatfilename.c_str(),ios::out) ;
                                badvaluefile.open(badfitfilename.c_str(),ios::out) ;
    
              //cout<<set1squareschcnt<<endl;
             // cout<<set2squareschcnt<<endl;
            // cout<<set3squareschcnt<<endl;
    
    imggset1(set1squareschcnt , set1squares, fitvaluefile,lowstatvaluefile,badvaluefile, PDGPi0Mass);///// innermost 5 rings
    imggset2(set2squareschcnt , set2squares, fitvaluefile,lowstatvaluefile,badvaluefile, PDGPi0Mass);////rest of the channels
    imggset3(set3squareschcnt , set3squares, fitvaluefile,lowstatvaluefile,badvaluefile, PDGPi0Mass);////outer 5 ring channels


    
    fitvaluefile.close();lowstatvaluefile.close();badvaluefile.close();
    
    plotGainsVsCh(); ////// plotting the Gains vs Channels and Sigma vs Channels
    
    
    
    
    
    
    
    
}//////main ending













