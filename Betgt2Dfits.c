//
//  Betgt2Dfits.c
//  
//
//  Created by Chandrasekhar Akondi on 3/21/20.
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
using namespace std;





/////////////////************  imgginnner  5 squares ******* SET - I      ********/////////////// START

void imgginnnerset1(int& numch, int *charry, ofstream& fitvaluefile, double& pi0mass)
     {/////imgginnnerset  1


         
                    TCanvas  *Canvas[38];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;   int canvaspad=1;
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
                 int chnumber=(32*i+j);
                  
                  //  if(charry[chnumber]<0) continue;///// if the #canvas x pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
                  Canvas[i]->cd(j+1);
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  if(projY->GetEntries()<=18000)
                  {
                fitvaluefile<<charry[chnumber]<<"\t"<<-1.0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                  }
                  else{
                      
                
           
                  
        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.045,0.28);
        fSpectrum->SetLineWidth(1.0);
        binmax[chnumber] = projY->GetMaximumBin();
        xvaluebinmax[chnumber] = projY->GetXaxis()->GetBinCenter(binmax[chnumber]);///// X-value of the max
        hightbinmax[chnumber] = projY->GetMaximum();
        const Int_t nparSpectrum=10;
                      

              
        Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.00001, 3e4, 4e-2, 2e-2,1e-2,1e-4,1e-4,1e-4};
        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
        fSpectrum->SetParameters(SpectrumParameters);
                                    fSpectrum->SetParLimits(0,50, 1e8);////working
                                    fSpectrum->SetParLimits(1,0.03, 0.25);////working
                                    fSpectrum->SetParLimits(2,0.001, 0.1);////working
                                    fSpectrum->SetParLimits(3,50, 1e8);////working
                                    fSpectrum->SetParLimits(4,0.0025, 0.30);////working
                                    fSpectrum->SetParLimits(5,0.0015, 0.3);////working
                                    fSpectrum->SetParLimits(6,0.0025, 0.30);////working
                                    fSpectrum->SetParLimits(7,0.0025, 0.3);
                                    fSpectrum->SetParLimits(8,0.0015, 0.3);
                      
                  
                      
                      
                      
                   projY->Fit("fSpectrum","","",0.045,0.28);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.045,0.28);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.045,0.28);
                  
            fitvaluefile<<charry[chnumber]<<"\t"<<fSpectrum->GetParameter(1)/pi0mass<<"\t"<<0<<"\t"<<fSpectrum->GetParError(1)/pi0mass<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()   <<endl;
                     
                  }////else
                      
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet1-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       

     }///////imgginnnerset 1///////////////************  imgginnner  5 squares ******* SET - I      ********/////////////// END


/////////********/////////**********/////////******$$$$$$$$$$***********/////////***************$$$$$$$$$$$$************/////////**************$$$$$$$*************/////////***************************/////////***************


/////////////////************  imgginnner  6 to 11  squares ******* SET - 2     ********/////////////// START

void imgginnnerset2(int& numch, int *charry, ofstream& fitvaluefile, double& pi0mass)
     {/////imgginnnerset 2



         
                    TCanvas  *Canvas[38];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;   int canvaspad=1;
                    gStyle->SetLineWidth(1);
                    gStyle->SetOptTitle(kFALSE);
         for(int i=0;i<=5;i++)
         {//// canvas loop

                 Canvas[i]=new TCanvas(Form("Canvas%d",i));
                 Canvas[i]->Divide(7,11);
                 stringstream a;a<<i+1;
                 outname=a.str();
  
             for(int j=0;j<77;j++)
              {////// each pad
                 int chnumber=(77*i+j);
                  
                 //   if(charry[chnumber]<0) continue;///// if the #canvas x pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
                  Canvas[i]->cd(j+1);
                       
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  if(projY->GetEntries()<=18000)
                  {
                    fitvaluefile<<charry[chnumber]<<"\t"<<-1.0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                  }
                  else{
                      
                 

        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.045,0.27);
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
                      
                      
                      /*
                      
        Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.00001, 3e4, 4e-2, 2e-2,1e-2,1e-4,1e-4,1e-4};
        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
        fSpectrum->SetParameters(SpectrumParameters);
                                    fSpectrum->SetParLimits(0,50, 1e8);////working
                                    fSpectrum->SetParLimits(1,0.03, 0.25);////working
                                    fSpectrum->SetParLimits(2,0.001, 0.1);////working
                                    fSpectrum->SetParLimits(3,50, 1e8);////working
                                    fSpectrum->SetParLimits(4,0.0025, 0.30);////working
                                    fSpectrum->SetParLimits(5,0.0015, 0.3);////working
                                    fSpectrum->SetParLimits(6,0.0015, 0.40);////working
                                   // fSpectrum->SetParLimits(7,0.0025, 0.4);
                                   // fSpectrum->SetParLimits(8,0.0015, 0.3);
                                  //  fSpectrum->SetParLimits(9,0.0015, 0.3);
                      
                 */
                      
                      
                      
                      
                   projY->Fit("fSpectrum","","",0.045,0.27);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.045,0.27);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.045,0.27);
                  
            fitvaluefile<<charry[chnumber]<<"\t"<<fSpectrum->GetParameter(1)/pi0mass<<"\t"<<0<<"\t"<<fSpectrum->GetParError(1)/pi0mass<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()   <<endl;
                
                  }////else
                  
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet2-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       


     }//////imgginnnerset 2 ///////////////////////////************  imgginnner 6 to 11 squares ******* SET - 2     ********/////////////// END

/////////********/////////**********/////////******$$$$$$$$$$***********/////////***************$$$$$$$$$$$$************/////////**************$$$$$$$*************/////////***************************/////////***************


/////////////////************  imgginnner  12 to 17  squares ******* SET - 3     ********/////////////// START

void imgginnnerset3(int& numch, int *charry, ofstream& fitvaluefile, double& pi0mass)
     {/////imgginnnerset 3

                    TCanvas  *Canvas[38];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;   int canvaspad=1;
                    gStyle->SetLineWidth(1);
                    gStyle->SetOptTitle(kFALSE);
         for(int i=0;i<=5;i++)
         {//// canvas loop
                 Canvas[i]=new TCanvas(Form("Canvas%d",i));
                 Canvas[i]->Divide(13,10);
                 stringstream a;a<<i+1;
                 outname=a.str();
             for(int j=0;j<130;j++)
              {////// each pad
                 int chnumber=(130*i+j);
             //  if(charry[chnumber]<0) continue;///// if the #canvas x pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
             Canvas[i]->cd(j+1);
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  
               
                  if(projY->GetEntries()<=18000)
                  {
                  fitvaluefile<<charry[chnumber]<<"\t"<<-1.0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                  }
                  else{
                      
                 

        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.045,0.27);
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
                      
                      
           /*
                      
        Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.00001, 3e4, 4e-2, 2e-2,1e-2,1e-4,1e-4,1e-4};
        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
        fSpectrum->SetParameters(SpectrumParameters);
                                    fSpectrum->SetParLimits(0,50, 1e8);////working
                                    fSpectrum->SetParLimits(1,0.03, 0.25);////working
                                    fSpectrum->SetParLimits(2,0.001, 0.1);////working
                                    fSpectrum->SetParLimits(3,50, 1e8);////working
                                    fSpectrum->SetParLimits(4,0.0025, 0.30);////working
                                    fSpectrum->SetParLimits(5,0.0025, 0.5);////working
                                 //   fSpectrum->SetParLimits(6,0.0015, 0.40);////working
                                   // fSpectrum->SetParLimits(7,0.0025, 0.4);
                                   // fSpectrum->SetParLimits(8,0.0015, 0.3);
                                  //  fSpectrum->SetParLimits(9,0.0015, 0.3);
       
                                    
                 */
                      
                      
                      
                      
                      
                   projY->Fit("fSpectrum","","",0.045,0.27);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.045,0.27);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.045,0.27);
                  
            fitvaluefile<<charry[chnumber]<<"\t"<<fSpectrum->GetParameter(1)/pi0mass<<"\t"<<0<<"\t"<<fSpectrum->GetParError(1)/pi0mass<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()   <<endl;
                
                  }////else
                  
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet3-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       


     }//////imgginnnerset 3 ///////////////////////////************  imgginnner 12 to 17 squares ******* SET - 3     ********/////////////// END

/////////********/////////**********/////////******$$$$$$$$$$***********/////////***************$$$$$$$$$$$$************/////////**************$$$$$$$*************/////////***************************/////////***************


/////////////////************  imgginnner  18 squares ******* SET - 4     ********/////////////// START

void imgginnnerset4(int& numch, int *charry, ofstream& fitvaluefile, double& pi0mass)
     {/////imgginnnerset 4

                    TCanvas  *Canvas[38];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;   int canvaspad=1;
                    gStyle->SetLineWidth(1);
                    gStyle->SetOptTitle(kFALSE);
         for(int i=0;i<1;i++)
         {//// canvas loop
             
            // if(i>0)continue;
             
             
                 Canvas[i]=new TCanvas(Form("Canvas%d",i));
                 Canvas[i]->Divide(12,13);
                 stringstream a;a<<i+1;
                 outname=a.str();
             for(int j=0;j<156;j++)
              {////// each pad
                 int chnumber=(156*i+j);
              //  if(charry[chnumber]<0) continue;///// if the #canvas times pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
                  
                    Canvas[i]->cd(j+1);
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  
               
                  if(projY->GetEntries()<=18000)
                  {
                        fitvaluefile<<charry[chnumber]<<"\t"<<-1.0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                  }
                  else{
                      
               

                  

                  
        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.045,0.27);
                  
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
                      

                              
                
                      
                      
                      
                      
                      
        /*
        Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.001, 3e2, 4e-2, 2e-2,1e-2,1e-4,1e-6,1e-3};
        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
        fSpectrum->SetParameters(SpectrumParameters);
                                    fSpectrum->SetParLimits(0,50, 1e8);////working
                                    fSpectrum->SetParLimits(1,0.03, 0.25);////working
                                    fSpectrum->SetParLimits(2,0.001, 0.1);////working
                                    fSpectrum->SetParLimits(3,50, 1e8);////working
                                    fSpectrum->SetParLimits(4,0.0014, 0.30);////working
                                    fSpectrum->SetParLimits(5,0.0015, 0.3);////working
                                   //fSpectrum->SetParLimits(6,0.0015, 0.40);////working
                                   // fSpectrum->SetParLimits(7,0.0025, 0.4);
                                   // fSpectrum->SetParLimits(8,0.0015, 0.3);
                                  //  fSpectrum->SetParLimits(9,0.0015, 0.3);
                      
                      
              */
                      
                      
                      
                      
                      
                   projY->Fit("fSpectrum","","",0.045,0.27);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.045,0.27);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.045,0.27);
                  
            fitvaluefile<<charry[chnumber]<<"\t"<<fSpectrum->GetParameter(1)/pi0mass<<"\t"<<fSpectrum->GetParError(1)/pi0mass<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()   <<endl;
                
                  }////// else
                  
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet4-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       


     }//////imgginnnerset 4 ///////////////////////////************  imgginnner 18 to 22  squares ******* SET - 4     ********/////////////// END



/////////////////************  imgginnner  19 to 23  squares ******* SET - 5    ********/////////////// START

void imgginnnerset5(int& numch, int *charry, ofstream& fitvaluefile, double& pi0mass)
     {/////imgginnnerset 4

                    TCanvas  *Canvas[38];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;   int canvaspad=1;
                    gStyle->SetLineWidth(1);
                    gStyle->SetOptTitle(kFALSE);
         for(int i=0;i<=4;i++)
         {//// canvas loop
             
            // if(i>0)continue;
             
             
                 Canvas[i]=new TCanvas(Form("Canvas%d",i));
                 Canvas[i]->Divide(16,10);
                 stringstream a;a<<i+1;
                 outname=a.str();
             for(int j=0;j<160;j++)
              {////// each pad
                 int chnumber=(160*i+j);
            //   if(charry[chnumber]<0) continue;///// if the #canvas x pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
             Canvas[i]->cd(j+1);
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  
               
                  if(projY->GetEntries()<=18000)
                  {
                  fitvaluefile<<charry[chnumber]<<"\t"<<-1<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<< 0  <<endl;
                  }
                  else
                  {

        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.045,0.27);
                  
        fSpectrum->SetLineWidth(1.0);
        binmax[chnumber] = projY->GetMaximumBin();
        xvaluebinmax[chnumber] = projY->GetXaxis()->GetBinCenter(binmax[chnumber]);///// X-value of the max
        hightbinmax[chnumber] = projY->GetMaximum();
        const Int_t nparSpectrum=10;
                  
                  
                      /*
                      
                  Double_t SpectrumParameters[nparSpectrum]={hightbinmax[chnumber],xvaluebinmax[chnumber],0.001, 9e2, 1, 1,1,1,1,1};
                            
                            
                  fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
                  fSpectrum->SetParameters(SpectrumParameters);
                            
                                                        fSpectrum->SetParLimits(0,0, 1e3);////working
                                                        fSpectrum->SetParLimits(1,0.003, 0.25);////working
                                                        fSpectrum->SetParLimits(2,0.001, 0.009);////working
                                                        fSpectrum->SetParLimits(3,0, 1e3);////working
                                                        fSpectrum->SetParLimits(4,0.0024, 0.250);////working
                                                        fSpectrum->SetParLimits(5,0.001, 0.6);////working
 

                      */
                      
                      
                                 
                      Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.001, 9e2, 1, 1,1,1,1,1};
                                
                                
                      fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
                      fSpectrum->SetParameters(SpectrumParameters);
                                
                                                            fSpectrum->SetParLimits(0,0, 1e8);////working
                                                            fSpectrum->SetParLimits(1,0.003, 0.25);////working
                                                            fSpectrum->SetParLimits(2,0.001, 0.009);////working
                                                            fSpectrum->SetParLimits(3,0, 1e8);////working
                                                            fSpectrum->SetParLimits(4,0.0024, 0.250);////working
                                                            fSpectrum->SetParLimits(5,0.001, 0.6);////working
                      
                      
                  
                   projY->Fit("fSpectrum","","",0.045,0.27);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.045,0.27);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.045,0.27);
                  
            fitvaluefile<<charry[chnumber]<<"\t"<<fSpectrum->GetParameter(1)/pi0mass<<"\t"<<fSpectrum->GetParError(1)/pi0mass<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()   <<endl;
                
               
                  }/////
                      
                  
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet5-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       


     }//////imgginnnerset 5 ///////////////////////////************  imgginnner 19 to 23  squares ******* SET - 5     ********/////////////// END





/////////////////************  imgginnner  24 to 28  squares ******* SET - 6    ********/////////////// START

void imgginnnerset6(int& numch, int *charry, ofstream& fitvaluefile, double& pi0mass)
     {/////imgginnnerset 4

       
         
      
         
                    TCanvas  *Canvas[38];///////725 channels are in 37 canvas x 20 per canvas
                    char nl[256];
                    char histoname[256];TH1F *h[numch+1];
                    int binmax[2801]; double xvaluebinmax[2801], hightbinmax[2801];
                    TFile *t=new TFile("BetargetAll.root");
                    TH2I *hPi0MassVsChNum = (TH2I *) t->Get("FCAL_Pi0TOF/Pi0MassVsChNum");
                    string outname;   int canvaspad=1;
                    gStyle->SetLineWidth(1);
                    gStyle->SetOptTitle(kFALSE);
         for(int i=0;i<=4;i++)
         {//// canvas loop
             
            // if(i>0)continue;
             
             
                 Canvas[i]=new TCanvas(Form("Canvas%d",i));
                 Canvas[i]->Divide(10,10);
                 stringstream a;a<<i+1;
                 outname=a.str();
             for(int j=0;j<100;j++)
              {////// each pad
                 int chnumber=(100*i+j);
                  
                   //   if(charry[chnumber]<0) continue;///// if the #canvas x pads per canvas not equal to channel array number this will take care of the extra non existing ones   IMP don't remove this , if you remove you will see some bad numbers in fitparametyer file
                  
    TH1D *projY=hPi0MassVsChNum->ProjectionY(Form("Channel%.4i",charry[chnumber]),charry[chnumber],charry[chnumber]);//create histogram for each inner channel
                  
                  
                   Canvas[i]->cd(j+1);
                  
               
                  if(projY->GetEntries()<=15000)

                  {
                     TF1 *fSpectrum1 = new TF1("fSpectrum1","gaus(0)",0.1,0.17);
                           fSpectrum1->SetLineWidth(1.0); fSpectrum1->SetLineColor(kGreen);
                           binmax[chnumber] = projY->GetMaximumBin();
                           xvaluebinmax[chnumber] = projY->GetXaxis()->GetBinCenter(binmax[chnumber]);///// X-value of the max
                           hightbinmax[chnumber] = projY->GetMaximum();
                           const Int_t nparSpectrum=10;
                        Double_t SpectrumParameters[nparSpectrum]={hightbinmax[chnumber],projY->GetMean(),projY->GetRMS(), 9e2, 1, 1,1,1,1,1};
                        fSpectrum1->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
                         fSpectrum1->SetParameters(SpectrumParameters);
                          fSpectrum1->SetParLimits(0,0, 1e8);////working
                          fSpectrum1->SetParLimits(1,0.003, 0.25);////working
                          fSpectrum1->SetParLimits(2,0.001, 0.009);////working
                          fSpectrum1->SetParLimits(3,0, 1e8);////working
                         fSpectrum1->SetParLimits(4,0.0024, 0.250);////working
                          fSpectrum1->SetParLimits(5,0.001, 0.6);////working
                                         
                   
                                      // projY->Fit("fSpectrum1","","",0.1,0.17);
                                   //   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.07,0.20);
                                     
                                   //   auto result = projY->Fit("fSpectrum","SQR");
                                    //  result->Print();
                      projY->Draw();
                                      projY->GetXaxis()->SetRangeUser(0.03,0.3);
                   // fitvaluefile<<charry[chnumber]<<"\t"<<-1.0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t\t\t"<<0<<endl;
                  }
                  else
                  {
                  
        TF1 *fSpectrum = new TF1("fSpectrum","gaus(0)+landau(3)+pol4(6)",0.045,0.27);
         
   
                  
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
                      
                      
                      /*
                      
                      
        Double_t SpectrumParameters[nparSpectrum]={hightbinmax[i],xvaluebinmax[i],0.001, 9e2, 1, 1,1,1,1,1};
                  
                  
        fSpectrum->SetParNames("Strength","Mean","Sigma","Constant","MPV","LandauSigma","coeff1","coeff2","coeff3");
        fSpectrum->SetParameters(SpectrumParameters);
                  
                                              fSpectrum->SetParLimits(0,0, 1e3);////working
                                              fSpectrum->SetParLimits(1,0.003, 0.25);////working
                                              fSpectrum->SetParLimits(2,0.001, 0.009);////working
                                              fSpectrum->SetParLimits(3,0, 1e3);////working
                                              fSpectrum->SetParLimits(4,0.0024, 0.250);////working
                                              fSpectrum->SetParLimits(5,0.001, 0.6);////working
                      
                      */
                      
                    projY->Fit("fSpectrum","","",0.045,0.27);
                   TFitResultPtr r = projY->Fit("fSpectrum","SQR","",0.045,0.27);
                  
                   auto result = projY->Fit("fSpectrum","SQR");
                   result->Print();
                   projY->GetXaxis()->SetRangeUser(0.045,0.27);
                  
            fitvaluefile<<charry[chnumber]<<"\t"<<fSpectrum->GetParameter(1)/pi0mass<<"\t"<<fSpectrum->GetParError(1)/pi0mass<<"\t"<<fSpectrum->GetParameter(2)<<"\t"<<fSpectrum->GetParError(2)<<"\t\t\t"<< r->Chi2()/r->Ndf()   <<endl;
                
                  
                  
                  
                  }////////
                  
                  
               
                  
              }///// each  pad
              
             Canvas[i]->Print(Form("Betgt2DfitinnersquareSet6-%s.pdf",outname.c_str()));
             Canvas[i]->Update();
             
             
         }////////canvas loop
       


         
     }//////imgginnnerset 6 ///////////////////////////************  imgginnner 24 to 28  squares ******* SET - 6     ********/////////////// END
/////////////////////////////////////////
void Betgt2Dfits()
{///////  fitting2D starting
    
    
    
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
       int insquare6ch[60],inr6ch;
       ifstream f7;f7.open("FCALinnersquare7.txt",ios::in);
       int insquare7ch[70],inr7ch;
       ifstream f8;f8.open("FCALinnersquare8.txt",ios::in);
       int insquare8ch[80],inr8ch;
       ifstream f9;f9.open("FCALinnersquare9.txt",ios::in);
       int insquare9ch[90],inr9ch;
       ifstream f10;f10.open("FCALinnersquare10.txt",ios::in);
       int insquare10ch[100],inr10ch;
       ifstream f11;f11.open("FCALinnersquare11.txt",ios::in);
       int insquare11ch[110],inr11ch;
       ifstream f12;f12.open("FCALinnersquare12.txt",ios::in);
       int insquare12ch[120],inr12ch;
       ifstream f13;f13.open("FCALinnersquare13.txt",ios::in);
       int insquare13ch[140],inr13ch;
       ifstream f14;f14.open("FCALinnersquare14.txt",ios::in);
       int insquare14ch[150],inr14ch;
       ifstream f15;f15.open("FCALinnersquare15.txt",ios::in);
       int insquare15ch[150],inr15ch;
       ifstream f16;f16.open("FCALinnersquare16.txt",ios::in);
       int insquare16ch[150],inr16ch;
       ifstream f17;f17.open("FCALinnersquare17.txt",ios::in);
       int insquare17ch[150],inr17ch;
       ifstream f18;f18.open("FCALinnersquare18.txt",ios::in);
       int insquare18ch[160],inr18ch;
       ifstream f19;f19.open("FCALinnersquare19.txt",ios::in);
       int insquare19ch[170],inr19ch;
       ifstream f20;f20.open("FCALinnersquare20.txt",ios::in);
       int insquare20ch[180],inr20ch;
       ifstream f21;f21.open("FCALinnersquare21.txt",ios::in);
       int insquare21ch[170],inr21ch;
       ifstream f22;f22.open("FCALinnersquare22.txt",ios::in);
       int insquare22ch[160],inr22ch;
       ifstream f23;f23.open("FCALinnersquare23.txt",ios::in);
       int insquare23ch[150],inr23ch;
       ifstream f24;f24.open("FCALinnersquare24.txt",ios::in);
       int insquare24ch[140],inr24ch;
       ifstream f25;f25.open("FCALinnersquare25.txt",ios::in);
       int insquare25ch[120],inr25ch;
       ifstream f26;f26.open("FCALinnersquare26.txt",ios::in);
         int insquare26ch[110],inr26ch;
       ifstream f27;f27.open("FCALinnersquare27.txt",ios::in);
       int insquare27ch[90],inr27ch;
       ifstream f28;f28.open("FCALinnersquare28.txt",ios::in);
       int insquare28ch[70],inr28ch;

       
       
       
    int innerset1squares[900],inrset1squaresch,inrset1squareschcnt=0;////////   set -I   inner 5 rings
    
    int innerset2squares[1200],inrset2squaresch,inrset2squareschcnt=0;////////   set -2   squares 6 to squares 11
           
    int innerset3squares[1200],inrset3squaresch,inrset3squareschcnt=0;////////   set -3   squares 12 to squares 17
    
    int innerset4squares[1200],inrset4squaresch,inrset4squareschcnt=0;////////   set -4   squares 18
    
    int innerset5squares[1200],inrset5squaresch,inrset5squareschcnt=0;////////   set -5   squares 19 to squares 23
        
    int innerset6squares[1200],inrset6squaresch,inrset6squareschcnt=0;////////   set -6   squares 24 to squares 28
    
              
               while(f1>>inr1ch)
               {////f1>>inr1ch
               for(int i=0;i<2800;i++)
                   {////////for(int i=0;i<2800;i++)
                     if (inr1ch==i)
                           {//////matching channels
                             innerset1squares[inrset1squareschcnt]=inr1ch;
                             inrset1squareschcnt++;
                            }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                }/////f1>>inr1ch
           
                while(f2>>inr2ch)
                {////f2>>inr2ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                      if (inr2ch==i)
                            {//////matching channels
                              innerset1squares[inrset1squareschcnt]=inr2ch;
                               inrset1squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f2>>inr2ch
    
while(f3>>inr3ch)
                  {////f3>>inr3ch
                  for(int i=0;i<2800;i++)
                      {////////for(int i=0;i<2800;i++)
                        if (inr3ch==i)
                              {//////matching channels
                               innerset1squares[inrset1squareschcnt]=inr3ch;
                               inrset1squareschcnt++;
                               }///// matching channels
                       }////////for(int i=0;i<2800;i++)
                   }/////f3>>inr3ch
    
 while(f4>>inr4ch)
                {////f4>>inr4ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                      if (inr4ch==i)
                            {//////matching channels
                              innerset1squares[inrset1squareschcnt]=inr4ch;
                           inrset1squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f4>>inr4ch
    
while(f5>>inr5ch)
                {////f5>>inr5ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                      if (inr5ch==i)
                            {//////matching channels
                           innerset1squares[inrset1squareschcnt]=inr5ch;
                           inrset1squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f5>>inr5ch

    
    

/////// SET -2     squares 6 to squares 11
    
while(f6>>inr6ch)
                 {////f6>>inr6ch
                 for(int i=0;i<2800;i++)
                     {////////for(int i=0;i<2800;i++)
                       if (inr6ch==i)
                             {//////matching channels
                                // cout<<inr6ch<<endl;
                             innerset2squares[inrset2squareschcnt]=inr6ch;
                             inrset2squareschcnt++;
                              }///// matching channels
                      }////////for(int i=0;i<2800;i++)
                  }/////f6>>inr6ch
   


 while(f7>>inr7ch)
                {////f7>>inr7ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                      if (inr7ch==i)
                            {//////matching channels
                               innerset2squares[inrset2squareschcnt]=inr7ch;
                                inrset2squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f7>>inr7ch

    
 while(f8>>inr8ch)
                 {////f8>>inr8ch
                 for(int i=0;i<2800;i++)
                     {////////for(int i=0;i<2800;i++)
                       if (inr8ch==i)
                             {//////matching channels
                             innerset2squares[inrset2squareschcnt]=inr8ch;
                           inrset2squareschcnt++;
                              }///// matching channels
                      }////////for(int i=0;i<2800;i++)
                  }/////f8>>inr8ch
 while(f9>>inr9ch)
                   {////f9>>inr9ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr9ch==i)
                               {//////matching channels
                                innerset2squares[inrset2squareschcnt]=inr9ch;
                              inrset2squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f9>>inr9ch

 while(f10>>inr10ch)
                {////f10>>inr10ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                      if (inr10ch==i)
                            {//////matching channels
                               innerset2squares[inrset2squareschcnt]=inr10ch;
                            inrset2squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f10>>inr10ch
    
    
    while(f11>>inr11ch)
                   {////f11>>inr11ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr11ch==i)
                               {//////matching channels
                                  innerset2squares[inrset2squareschcnt]=inr11ch;
                               inrset2squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f11>>inr11ch
    
    
 /////// SET -3     squares 12 to squares 17
    
    while(f12>>inr12ch)
                   {////f12>>inr12ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr12ch==i)
                               {//////matching channels
                                  innerset3squares[inrset3squareschcnt]=inr12ch;
                               inrset3squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f12>>inr12ch
    
    
    
    while(f13>>inr13ch)
                   {////f13>>inr13ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr13ch==i)
                               {//////matching channels
                                  innerset3squares[inrset3squareschcnt]=inr13ch;
                               inrset3squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f13>>inr13ch
    
    
    while(f14>>inr14ch)
                   {////f14>>inr14ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr14ch==i)
                               {//////matching channels
                                  innerset3squares[inrset3squareschcnt]=inr14ch;
                               inrset3squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f14>>inr14ch
    
    
    while(f15>>inr15ch)
                   {////f12>>inr12ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr15ch==i)
                               {//////matching channels
                                  innerset3squares[inrset3squareschcnt]=inr15ch;
                               inrset3squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f15>>inr15ch
    
    
    while(f16>>inr16ch)
                   {////f16>>inr16ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr16ch==i)
                               {//////matching channels
                                  innerset3squares[inrset3squareschcnt]=inr16ch;
                               inrset3squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f16>>inr16ch
    
    
    while(f17>>inr17ch)
                   {////f17>>inr17ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr17ch==i)
                               {//////matching channels
                                  innerset3squares[inrset3squareschcnt]=inr17ch;
                               inrset3squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f17>>inr17ch
  

  /////// SET -4     squares 18
    

    while(f18>>inr18ch)
                   {////f18>>inr18ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr18ch==i)
                               {//////matching channels
                                  innerset4squares[inrset4squareschcnt]=inr18ch;
                               inrset4squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f18>>inr18ch
  
   
    
    
     /////// SET -5    squares 19- 23
    
    
    while(f19>>inr19ch)
                   {////f19>>inr19ch
                   for(int i=0;i<2800;i++)
                       {////////for(int i=0;i<2800;i++)
                         if (inr19ch==i)
                               {//////matching channels
                                  innerset5squares[inrset5squareschcnt]=inr19ch;
                               inrset5squareschcnt++;
                                }///// matching channels
                        }////////for(int i=0;i<2800;i++)
                    }/////f19>>inr19ch
    
    
    
    
    while(f20>>inr20ch)
                   {////f20>>inr20ch
                    for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                     if (inr20ch==i)
                         {//////matching channels
                           innerset5squares[inrset5squareschcnt]=inr20ch;
                            inrset5squareschcnt++;
                           }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                    }/////f20>>inr20ch
    
    
    
    while(f21>>inr21ch)
                {////f21>>inr21ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                        if (inr21ch==i)
                            {//////matching channels
                            innerset5squares[inrset5squareschcnt]=inr21ch;
                            inrset5squareschcnt++;
                            }///// matching channels
                    }////////for(int i=0;i<2800;i++)
                }/////f21>>inr21ch
    
    
    while(f22>>inr22ch)
                {////f22>>inr22ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                        if (inr22ch==i)
                            {//////matching channels
                            innerset5squares[inrset5squareschcnt]=inr22ch;
                            inrset5squareschcnt++;
                            }///// matching channels
                    }////////for(int i=0;i<2800;i++)
                }/////f22>>inr22ch
    
    
   while(f23>>inr23ch)
               {////f23>>inr23ch
               for(int i=0;i<2800;i++)
                   {////////for(int i=0;i<2800;i++)
                       if (inr23ch==i)
                           {//////matching channels
                           innerset5squares[inrset5squareschcnt]=inr23ch;
                           inrset5squareschcnt++;
                           }///// matching channels
                   }////////for(int i=0;i<2800;i++)
               }/////f23>>inr23ch
    
    
    
   
    
     while(f24>>inr24ch)
                    {////f24>>inr24ch
                     for(int i=0;i<2800;i++)
                     {////////for(int i=0;i<2800;i++)
                      if (inr24ch==i)
                          {//////matching channels
                            innerset6squares[inrset6squareschcnt]=inr24ch;
                             inrset6squareschcnt++;
                            }///// matching channels
                      }////////for(int i=0;i<2800;i++)
                     }/////f24>>inr24ch
     
     
     
     while(f25>>inr25ch)
                 {////f21>>inr21ch
                 for(int i=0;i<2800;i++)
                     {////////for(int i=0;i<2800;i++)
                         if (inr25ch==i)
                             {//////matching channels
                             innerset6squares[inrset6squareschcnt]=inr25ch;
                             inrset6squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f21>>inr21ch
     
     
     while(f26>>inr26ch)
                 {////f22>>inr22ch
                 for(int i=0;i<2800;i++)
                     {////////for(int i=0;i<2800;i++)
                         if (inr26ch==i)
                             {//////matching channels
                             innerset6squares[inrset6squareschcnt]=inr26ch;
                             inrset6squareschcnt++;
                             }///// matching channels
                     }////////for(int i=0;i<2800;i++)
                 }/////f22>>inr22ch
  
     
    while(f27>>inr27ch)
                {////f23>>inr23ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                        if (inr27ch==i)
                            {//////matching channels
                            innerset6squares[inrset6squareschcnt]=inr27ch;
                            inrset6squareschcnt++;
                            }///// matching channels
                    }////////for(int i=0;i<2800;i++)
                }/////f23>>inr23ch
    
    
     
    while(f28>>inr28ch)
                {////f23>>inr23ch
                for(int i=0;i<2800;i++)
                    {////////for(int i=0;i<2800;i++)
                        if (inr28ch==i)
                            {//////matching channels
                            innerset6squares[inrset6squareschcnt]=inr28ch;
                            inrset6squareschcnt++;
                            }///// matching channels
                    }////////for(int i=0;i<2800;i++)
                }/////f23>>inr23ch
     
    
    
    
                  ofstream fitvaluefile,lowstatvaluefile,outeringvaluefile;
                  string fitfilename,lowstatfilename,outeringfilename;
                  fitfilename ="goodchannelsfitparameters.txt";
                  lowstatfilename ="lowstatchannelsfitparameters.txt";
                  outeringfilename ="outerchannelsfitparameters.txt";
    
    
    
    
                  fitvaluefile.open(fitfilename.c_str(),ios::out| ios::app) ;
                  lowstatvaluefile.open(lowstatfilename.c_str(),ios::out| ios::app) ;
                  outeringvaluefile.open(outeringfilename.c_str(),ios::out| ios::app) ;
    
    //cout<<inrset1squareschcnt<<endl;
     //  imgginnnerset1(inrset1squareschcnt , innerset1squares, fitvaluefile, PDGPi0Mass);
     
    
     //cout<<inrset2squareschcnt<<endl;
     //  imgginnnerset2(inrset2squareschcnt , innerset2squares, fitvaluefile, PDGPi0Mass);
    
    
     //   cout<<inrset3squareschcnt<<endl;
     //   imgginnnerset3(inrset3squareschcnt , innerset3squares, fitvaluefile, PDGPi0Mass);
    
    
    
      // cout<<inrset4squareschcnt<<endl;
     // imgginnnerset4(inrset4squareschcnt , innerset4squares, fitvaluefile, PDGPi0Mass);
    
    
   //  cout<<inrset5squareschcnt<<endl;
   //   imgginnnerset5(inrset5squareschcnt , innerset5squares, fitvaluefile, PDGPi0Mass);///////// need to work
    
    
     // cout<<inrset6squareschcnt<<endl;
      imgginnnerset6(inrset6squareschcnt , innerset6squares, fitvaluefile, PDGPi0Mass);///////// need to work
    

    
    
    fitvaluefile.close();
    
    
}///////  fitting2D ending






























