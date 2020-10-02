#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
 #include "TH1.h"
 #include "TF1.h"
 #include "TRandom.h"
 #include "TSpectrum.h"
 #include "TVirtualFitter.h"
#include "TLatex.h"
using namespace std;




  int RunNumber[]={61321,61322,61323,61325,61327,61329,61330,61331,61332,61333,
                   61334,61335,61336,61337,61340,61341,61342,61343,61344};

  int RN,RNcount;//////RNcount is #number of files in RunNumber array ex:5or200depends on #root files to analyse,
  int ROOTfilenum;///// ROOTnum is ROOT FILE NUMBER
  TFile *t[19];/////Number of Be root files 19 it has to be equal to the  RunNumber[] array {#}


  void fetchingtheRootfile()
        {/////fetchingtheRootfile loop
       char nl[256];
       ROOTfilenum=RunNumber[RNcount];///// ROOT FILE NUMBER
       sprintf(nl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib%d.root", ROOTfilenum);
       cout<< nl<<"\t"<<RNcount<<endl;
       t[RNcount]= new TFile(nl);
        }/////fetchingtheRootfile loop



  void fetchingtherunnumber()
      {RN=RunNumber[RNcount];}
/////////////////////////////////////////////////////

//////////////CREATE NORMALISED VALUES AND ERRORS/////////////////////////



   void Createnormfiles()

              {//////////normalised files start


     ofstream largeoutfile;///// ADC fit output file for all channels in all regions
     string largeoutname;///// string for ADC fit output file for all channels in all region
     largeoutname  ="BeTgtCHRegi.txt";
     largeoutfile.open(largeoutname.c_str(),ios::out) ;


     ofstream tempoutfile;///// ADC fit output file for all channels in all regions
     string tempoutname;///// string for ADC fit output file for all channels in all region


     ofstream wrongchoutfile;/////suspecious channels in all regions
     wrongchoutfile.open("wrongchannels.txt",ios::out);

     ofstream tempfile1;/////
     string tempname1;/////


     ofstream NpeaksfileR1;/////NPeaks in  channels in  region1
     NpeaksfileR1.open("R1NpeaksVSchannels.txt",ios::out);

     ofstream NpeaksfileR2;/////NPeaks in  channels in  region2
     NpeaksfileR2.open("R2NpeaksVSchannels.txt",ios::out);

     ofstream NpeaksfileR3;/////NPeaks in  channels in  region3
     NpeaksfileR3.open("R3NpeaksVSchannels.txt",ios::out);

     ofstream NpeaksfileR4;/////NPeaks in  channels in  region4
     NpeaksfileR4.open("R4NpeaksVSchannels.txt",ios::out);

     ofstream NpeaksfileR5;/////NPeaks in  channels in  region5
     NpeaksfileR5.open("R5NpeaksVSchannels.txt",ios::out);





     ofstream fitparametersfile;/////fitparameters for abnormal channels in all regions
     string fitparametersfilename;






                   char hname[1024];
                   char nl[1024];
                   float mean[2801][6][20],ermean[2801][6][20],Ratio[2801][6][20],erRatio[2801][6][20];

 // TFile *t61341R5CH2558=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61341.root");
 // TH1F *h61341R5CH2558=(TH1F*)t61341R5CH2558->Get("fcalchannelRegion5integral_2558");

  //TFile *t61332R5CH2639=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61332.root");
 // TH1F *h61332R5CH2639=(TH1F*)t61332R5CH2639->Get("fcalchannelRegion5integral_2639");

              for( int ii=0;ii<19;ii++)/////Number of runs = 19 for Be runs
             //  for( int ii=0;ii<1;ii++)/////Number of runs = 19 for Be runs
                {///////number of runs loop

                     int  ROOTfilenum=RunNumber[ii];///// ROOT FILE NUMBER
                     sprintf(nl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib%d.root", ROOTfilenum);
                     t[ii]= new TFile(nl);

                      for(int regi=1;regi<=5;regi++)
                    //  for(int regi=1;regi<=1;regi++)
                        {/////regi loop


                                TH1F *h[2800]; /////Number of histograms for each region for Be runs

           		        stringstream bb;    bb<<regi;       ////    region number

            			tempname1  ="BeTgtCHRegi"+bb.str();
            			tempname1 +="normalised";
           			tempname1 +=".txt";
                   		tempfile1.open(tempname1.c_str(),ios::out| ios::app ) ; /////   ios::app  is very important to append DONT FORGET TO ADD

int abnormalchcount=0;
                           for(int ch=0;ch<2800;ch++)
                        // for(int ch=2558;ch<2559;ch++)
                		 {//////channel loop
            			stringstream a;    a<<ch;          ////// channel #number
           		        stringstream b;    b<<regi;       ////    region number
           		        stringstream d;    d<<ii;       ////    region number
            			tempoutname  ="R"+b.str();
            			tempoutname +="CH";
            			tempoutname += a.str();
           			tempoutname +=".txt";
                  	tempoutfile.open(tempoutname.c_str(),ios::out| ios::app ) ; /////   ios::app  is very important to append DONT FORGET TO ADD



      fitparametersfilename = "R"+b.str();
      fitparametersfilename +="abnormalfitCH.txt";
      fitparametersfile.open(fitparametersfilename.c_str(),ios::out|ios::app);





			sprintf(hname,"fcalchannelRegion%dintegral_%d",regi,ch);
                      	h[ch] = (TH1F*)t[ii]->Get(hname);


	///if(h[ch]->GetMean() <500) continue;
	///if(h[ch]->GetEntries() <100)continue;
         TSpectrum *s = new TSpectrum(10);///// 10 = here 5 peaks x 2 usually  # peaks x 2
         int nfound = s->Search(h[ch],6,"",0.10);

         int npeaks=s->GetNPeaks(); //// this section is to select the peak from pedestals by selecting the highest X position of the peak

//cout<<nfound<<endl;


                 if(npeaks>2){

                       if(regi==1){
                  NpeaksfileR1<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<npeaks<<endl;
                                  }
                       if(regi==2){
                  NpeaksfileR2<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<npeaks<<endl;
                                  }

                       if(regi==3){
                  NpeaksfileR3<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<npeaks<<endl;
                                  }

                       if(regi==4){
                  NpeaksfileR4<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<npeaks<<endl;
                                  }

                       if(regi==5){
                  NpeaksfileR5<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<npeaks<<endl;
                                  }

                        }



         double desiredpeakcenter =0;
         double *Xpeakposition = s->GetPositionX();
            for(int i=0;i<npeaks;i++)
             {////// npeaks

           if (Xpeakposition[i]> desiredpeakcenter)
             desiredpeakcenter = Xpeakposition[i];
             }////// npeaks
            //	cout<<"npeaks="<<npeaks<<endl;
                                 double center=h[ch]->GetMean();
                      		double width=h[ch]->GetRMS();

// "S"  The result of the fit is returned in the TFitResultPtr





              TF1* fit = new TF1("fit", "gaus",  desiredpeakcenter-(desiredpeakcenter/5.),  desiredpeakcenter+(desiredpeakcenter/5.));

          //    h[ch]->Fit("fit","","", desiredpeakcenter-(desiredpeakcenter/5.),  desiredpeakcenter+(desiredpeakcenter/5.));

TFitResultPtr fitr = h[ch]->Fit("fit","S","", desiredpeakcenter-(desiredpeakcenter/5.),  desiredpeakcenter+(desiredpeakcenter/5.));


	int status = int ( fitr );//// status =0 means accurate fit, status =4 abnormal fit
			//cout<<"status="<<status<<endl;




         if ((status==0) && (npeaks<=2) )



         {////// legitimate number check  Fitting accuracy check

                                 //	cout<<"npeaks="<<npeaks<<endl;
                        	mean[ch][regi][ii]     =  fit->GetParameter(1);
                        	ermean[ch][regi][ii]   =  fit->GetParError(1);

            	//cout<<"mean[ch][regi][ii]="<<mean[ch][regi][ii]<<endl;
	//if(mean[ch][regi][ii] <400) continue;

            //	cout<<"mean[ch][regi][ii] AFTER 500 CUT="<<mean[ch][regi][ii]<<endl;

         if ((mean[ch][regi][0]<=500) )


          	 {
           mean[ch][regi][0]=  mean[ch][regi][ii];
           ermean[ch][regi][0]=  ermean[ch][regi][ii];

           	}

            	//cout<<"mean[ch][regi][0] AFTER 500 CUT="<<mean[ch][regi][0]<<endl;
//cout<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<<Ratio[ch][regi][ii]<<"\t"<<erRatio[ch][regi][ii]<<endl;

                        	Ratio[ch][regi][ii]    =  mean[ch][regi][ii] /mean[ch][regi][0];

            			erRatio[ch][regi][ii]  = (1./mean[ch][regi][0])*(sqrt ( (ermean[ch][regi][ii] *ermean[ch][regi][ii] ) +  (  ( Ratio[ch][regi][ii]* Ratio[ch][regi][ii])  *  (ermean[ch][regi][0]* ermean[ch][regi][0] ) )    ))  ;

 				largeoutfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<<Ratio[ch][regi][ii]<<"\t"<<erRatio[ch][regi][ii]<<endl;

 				tempoutfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<<Ratio[ch][regi][ii]<<"\t"<<erRatio[ch][regi][ii]<<endl;

				if(ii==18) {
tempfile1<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<<Ratio[ch][regi][ii]<<"\t"<<erRatio[ch][regi][ii]<<endl;
          				 }







                              }////// legitimate number check   Fitting accuracy check

                           else
                              {////// legitimate number check	  Fitting accuracy check

abnormalchcount++;



      wrongchoutfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<<Ratio[ch][regi][ii]<<"\t"<<erRatio[ch][regi][ii]<<endl;

		fitparametersfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t"<<fit->GetParError(1)<<"\t\t"<<Ratio[ch][regi][ii]<<"\t"<<erRatio[ch][regi][ii]<<"\t"<<npeaks<<"\t"<<abnormalchcount<<endl;


                              }////// legitimate number check
                     fitparametersfile.close();

               tempoutfile.close();
                	}//////channel loop
	//fitparametersfile.close();
		tempfile1.close();
                      }/////regi loop

                }///////number of runs loop

             largeoutfile.close();

    	wrongchoutfile.close();

	//fitparametersfile.close();

      }///////normalised files end


/////////////////////////////////////////////////////////////
////   THIS CreateNormRunVsChannel() IS TO DRAW A HISTOGRAM FOR THE LAST
////   NORMALISED RATIO VS CHGANNEL NUMBER FOR EACH REGION
////////////////////////////////////////


  void  CreateNormRunVsChannelBAND()

   		{//////////CreateNormRunVsChannelBAND

         int num=0;
         char regnl[256];
    	 TGraphErrors *RegNumber[6];
         int colors[6];
         colors[1] = kViolet; colors[2] = kBlue; colors[3] = kGreen+2; colors[4] = kViolet+2; colors[5] = kBlue;
         double size[6];
         size[1] = 1.0; size[2] = 1.0; size[3] = 1.0; size[4] = 1.0; size[5] = 1.0;
         int style[6];
         style[1] = 20; style[2] = 21; style[3] = 22; style[4] = 23; style[5] = 25;



            TCanvas *canvas = new TCanvas("canvas"," Be Normalised Run VS Channels at Diff Regions",1200,600);
            canvas->SetWindowSize(1000,600);
            canvas->SetFillStyle(0);
            canvas->SetFrameFillStyle(0);


         TMultiGraph *mg = new TMultiGraph();
           for(int regi=1;regi<=5;regi++)
                           {////regi loop





            sprintf(regnl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/BeTgtCHRegi%dnormalised.txt",regi);
            RegNumber[num] = new TGraphErrors(regnl,"%*lg %*lg %lg%*lg %*lg %lg%lg");
            RegNumber[num]->SetMarkerSize(size[regi]);
            RegNumber[num]->SetMarkerStyle(style[regi]);
            RegNumber[num]->SetMarkerColor(colors[regi]);
            RegNumber[num]->SetLineColor(colors[regi]);
            mg->Add(RegNumber[num]);
            num++;
                               }////regi loop


/////  histogram band

   TH1D *hband = new TH1D("hband","histogramband", 2801, 0, 2800);
    hband->GetYaxis()->SetRangeUser(0.95, 1.05);
           for(int ii=0;ii<2800;ii++)
             {/////  histgram band
             double xposi = ii;
             hband->SetBinContent(xposi,1.);
             hband->SetBinError(ii,0.1);
             }/////  histogram band

            		mg->Draw("AP");
             hband->SetFillColor(kYellow);
             hband->SetFillStyle(3001);/////https://root.cern/doc/master/classTAttFill.html
                     mg->GetXaxis()->SetTickLength(0.06);
                     mg->GetXaxis()->SetNdivisions(505);
                     mg->GetYaxis()->SetNdivisions(505);
        	mg->GetXaxis()->SetTitle("Channel Numbers");
        	mg->GetYaxis()->SetTitle("Normalised ADC integral ( Run_{N}/Run_{1} )");
		mg->GetXaxis()->SetLabelFont(42);
		mg->GetXaxis()->SetTitleSize(0.03);
		mg->GetXaxis()->SetLabelSize(0.03);
		mg->GetXaxis()->SetTitleFont(42);
		mg->GetYaxis()->SetLabelFont(42);
		mg->GetYaxis()->SetLabelSize(0.03);
		mg->GetYaxis()->SetTitleSize(0.03);
		mg->GetYaxis()->SetTitleFont(42);
		mg->GetYaxis()->CenterTitle(true);
		mg->GetXaxis()->CenterTitle(true);
                        hband->Draw("E3same");

  TLegend* tl6 = new TLegend(.12, .6, .2, .85,NULL,"brNDC");

  tl6->AddEntry(RegNumber[0],"Violet 12V","p")->SetTextColor(kViolet);
  tl6->AddEntry(RegNumber[1],"Blue   10V","p")->SetTextColor(kBlue);
  tl6->AddEntry(RegNumber[2],"Green  29V","p")->SetTextColor(kGreen);
  tl6->AddEntry(RegNumber[3],"Violet 22V","p")->SetTextColor(kViolet);
  tl6->AddEntry(RegNumber[4],"Blue   15V ","p")->SetTextColor(kBlue);

   tl6->SetFillColor(0);
   tl6->SetTextFont(40);
   tl6->SetTextSize(0.04);
   tl6->SetLineWidth(0);
   tl6->SetLineColor(1);
   tl6->SetBorderSize(0);
   tl6->Draw();

canvas->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/AllRegiNormVSchannels.pdf");

/////////////individual region1 plot


            TCanvas *canvasR1 = new TCanvas("canvasR1","Be Region1 Violet 12V",1200,600);
            canvasR1->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasR1->SetFillStyle(0);
            canvasR1->SetFrameFillStyle(0);
	    TGraphErrors *Reg1 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/BeTgtCHRegi1normalised.txt","%*lg %*lg %lg%*lg %*lg %lg%lg");
            Reg1->SetMarkerSize(1);
            Reg1->SetMarkerStyle(20);
            Reg1->SetMarkerColor(kViolet);
            Reg1->SetLineColor(kViolet);
                     Reg1->GetXaxis()->SetTickLength(0.06);
                     Reg1->GetXaxis()->SetNdivisions(505);
                     Reg1->GetYaxis()->SetNdivisions(505);
        	Reg1->GetXaxis()->SetTitle("Channel Numbers");
        	Reg1->GetYaxis()->SetTitle("Normalised ADC integral Violet 12V ( Run_{N}/Run_{1} )");
        	Reg1->GetYaxis()->SetTitleColor(6);
		Reg1->GetXaxis()->SetLabelFont(42);
		Reg1->GetXaxis()->SetTitleSize(0.03);
		Reg1->GetXaxis()->SetLabelSize(0.03);
		Reg1->GetXaxis()->SetTitleFont(42);
		Reg1->GetYaxis()->SetLabelFont(42);
		Reg1->GetYaxis()->SetLabelSize(0.03);
		Reg1->GetYaxis()->SetTitleSize(0.03);
		Reg1->GetYaxis()->SetTitleFont(42);
		Reg1->GetYaxis()->CenterTitle(true);
		Reg1->GetXaxis()->CenterTitle(true);


            Reg1->Draw("AP");
   	    TH1D *hbandR1 = new TH1D("hbandR1","histogramband", 2800, 0, 2800);
            hbandR1->GetYaxis()->SetRangeUser(0.95, 1.05);
           for(int ii=0;ii<2800;ii++)
             {/////  histgram band
             double xposi = ii;
             hbandR1->SetBinContent(xposi,1.);
             hbandR1->SetBinError(ii,0.1);
             }/////  histogram band

             hbandR1->SetFillColor(kYellow);
             hbandR1->SetFillStyle(3001);
	     hbandR1->Draw("E3same");


   	TPad *canvas_1 = new TPad("canvas_1", "newpad",0.242485,0.1401051,0.6242485,0.290718);
   	canvas_1->Draw();
   	canvas_1->cd();
 	TGraphErrors  *R1CH1 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R1CH1.txt","%lg %*lg %*lg %*lg %*lg %lg %lg");

                     R1CH1->SetMarkerStyle(kOpenCircle);
                     R1CH1->SetMarkerColor(kViolet);
                     R1CH1->SetLineColor(kViolet);
                     R1CH1->SetMarkerSize(1.);

                     R1CH1->GetYaxis()->SetRangeUser(0.30,1.1);
                     R1CH1->GetXaxis()->SetTickLength(0.06);
                     R1CH1->GetXaxis()->SetNdivisions(505);
                    R1CH1->GetYaxis()->SetNdivisions(505);
        	R1CH1->GetXaxis()->SetTitle("Run Numbers");
		R1CH1->GetXaxis()->SetLabelFont(42);
		R1CH1->GetXaxis()->SetLabelSize(0.12);
		R1CH1->GetXaxis()->SetTitleSize(0);
		R1CH1->GetXaxis()->SetTitleFont(42);
		R1CH1->GetYaxis()->SetLabelFont(42);
		R1CH1->GetYaxis()->SetLabelSize(0.12);
		R1CH1->GetYaxis()->SetTitleSize(0.12);
		R1CH1->GetYaxis()->SetTitleOffset(0.29);
		R1CH1->GetYaxis()->SetTitleFont(42);
		R1CH1->GetYaxis()->CenterTitle(true);
                     R1CH1->Draw("ap");

canvasR1->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R1NormVSchannels.pdf");

/////////////individual region2 plot



            TCanvas *canvasR2 = new TCanvas("canvasR2","Be Region2 Blue 10V",1200,600);
            canvasR2->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasR2->SetFillStyle(0);
            canvasR2->SetFrameFillStyle(0);
	    TGraphErrors *Reg2 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/BeTgtCHRegi2normalised.txt","%*lg %*lg %lg%*lg %*lg %lg%lg");
            Reg2->SetMarkerSize(1);
            Reg2->SetMarkerStyle(20);
            Reg2->SetMarkerColor(kBlue);
            Reg2->SetLineColor(kBlue);
                     Reg2->GetXaxis()->SetTickLength(0.06);
                     Reg2->GetXaxis()->SetNdivisions(505);
                     Reg2->GetYaxis()->SetNdivisions(505);
        	Reg2->GetXaxis()->SetTitle("Channel Numbers");
        	Reg2->GetYaxis()->SetTitle("Normalised ADC integral Blue 10V ( Run_{N}/Run_{1} )");
        	Reg2->GetYaxis()->SetTitleColor(4);
		Reg2->GetXaxis()->SetLabelFont(42);
		Reg2->GetXaxis()->SetTitleSize(0.03);
		Reg2->GetXaxis()->SetLabelSize(0.03);
		Reg2->GetXaxis()->SetTitleFont(42);
		Reg2->GetYaxis()->SetLabelFont(42);
		Reg2->GetYaxis()->SetLabelSize(0.03);
		Reg2->GetYaxis()->SetTitleSize(0.03);
		Reg2->GetYaxis()->SetTitleFont(42);
		Reg2->GetYaxis()->CenterTitle(true);
		Reg2->GetXaxis()->CenterTitle(true);


            Reg2->Draw("AP");
   	    TH1D *hbandR2 = new TH1D("hbandR2","histogramband", 2800, 0, 2800);
            hbandR2->GetYaxis()->SetRangeUser(0.95, 1.05);
           for(int ii=0;ii<2800;ii++)
             {/////  histgram band
             double xposi = ii;
             hbandR2->SetBinContent(xposi,1.);
             hbandR2->SetBinError(ii,0.1);
             }/////  histogram band

             hbandR2->SetFillColor(kYellow);
             hbandR2->SetFillStyle(3001);
	     hbandR2->Draw("E3same");

  	TPad *canvasR2_1 = new TPad("canvasR2_1", "newpad",0.4,0.25,0.65,0.45);
   	canvasR2_1->Draw();
   	canvasR2_1->cd();
 	TGraphErrors  *R2CH1025 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R2CH1025.txt","%lg %*lg %*lg %*lg %*lg %lg %lg");

                     R2CH1025->SetMarkerStyle(kOpenCircle);
                     R2CH1025->SetMarkerColor(kBlue);
                     R2CH1025->SetLineColor(kBlue);
                     R2CH1025->SetMarkerSize(1.);

                     R2CH1025->GetYaxis()->SetRangeUser(0.0,1.1);
                     R2CH1025->GetXaxis()->SetTickLength(0.06);
                     R2CH1025->GetXaxis()->SetNdivisions(505);
                    R2CH1025->GetYaxis()->SetNdivisions(505);
        	R2CH1025->GetXaxis()->SetTitle("Run Numbers");
		R2CH1025->GetXaxis()->SetLabelFont(42);
		R2CH1025->GetXaxis()->SetLabelSize(0.12);
		R2CH1025->GetXaxis()->SetTitleSize(0);
		R2CH1025->GetXaxis()->SetTitleFont(42);
		R2CH1025->GetYaxis()->SetLabelFont(42);
		R2CH1025->GetYaxis()->SetLabelSize(0.12);
		R2CH1025->GetYaxis()->SetTitleSize(0.12);
		R2CH1025->GetYaxis()->SetTitleOffset(0.29);
		R2CH1025->GetYaxis()->SetTitleFont(42);
		R2CH1025->GetYaxis()->CenterTitle(true);
                     R2CH1025->Draw("ap");


canvasR2->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R2NormVSchannels.pdf");

/////////////individual region3 plot



            TCanvas *canvasR3 = new TCanvas("canvasR3","Be Region3 Green 29V",1200,600);
            canvasR3->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasR3->SetFillStyle(0);
            canvasR3->SetFrameFillStyle(0);
	    TGraphErrors *Reg3 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/BeTgtCHRegi3normalised.txt","%*lg %*lg %lg%*lg %*lg %lg%lg");
            Reg3->SetMarkerSize(1);
            Reg3->SetMarkerStyle(20);
            Reg3->SetMarkerColor(kGreen);
            Reg3->SetLineColor(kGreen);

                     Reg3->GetXaxis()->SetTickLength(0.06);
                     Reg3->GetXaxis()->SetNdivisions(505);
                     Reg3->GetYaxis()->SetNdivisions(505);
        	Reg3->GetXaxis()->SetTitle("Channel Numbers");
        	Reg3->GetYaxis()->SetTitle("Normalised ADC integral Green 29V ( Run_{N}/Run_{1} )");
        	Reg3->GetYaxis()->SetTitleColor(3);
		Reg3->GetXaxis()->SetLabelFont(42);
		Reg3->GetXaxis()->SetTitleSize(0.03);
		Reg3->GetXaxis()->SetLabelSize(0.03);
		Reg3->GetXaxis()->SetTitleFont(42);
		Reg3->GetYaxis()->SetLabelFont(42);
		Reg3->GetYaxis()->SetLabelSize(0.03);
		Reg3->GetYaxis()->SetTitleSize(0.03);
		Reg3->GetYaxis()->SetTitleFont(42);
		Reg3->GetYaxis()->CenterTitle(true);
		Reg3->GetXaxis()->CenterTitle(true);

            Reg3->Draw("AP");
   	    TH1D *hbandR3 = new TH1D("hbandR3","histogramband", 2800, 0, 2800);
            hbandR3->GetYaxis()->SetRangeUser(0.95, 1.05);
           for(int ii=0;ii<2800;ii++)
             {/////  histgram band
             double xposi = ii;
             hbandR3->SetBinContent(xposi,1.);
             hbandR3->SetBinError(ii,0.1);
             }/////  histogram band

             hbandR3->SetFillColor(kYellow);
             hbandR3->SetFillStyle(3001);
	     hbandR3->Draw("E3same");

canvasR3->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R3NormVSchannels.pdf");
////////////individual region4 plot



            TCanvas *canvasR4 = new TCanvas("canvasR4","Be Region4 Violet 22V",1200,600);
            canvasR4->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasR4->SetFillStyle(0);
            canvasR4->SetFrameFillStyle(0);
	    TGraphErrors *Reg4 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/BeTgtCHRegi4normalised.txt","%*lg %*lg %lg%*lg %*lg %lg%lg");
            Reg4->SetMarkerSize(1);
            Reg4->SetMarkerStyle(20);
            Reg4->SetMarkerColor(kViolet);
            Reg4->SetLineColor(kViolet);

                     Reg4->GetXaxis()->SetTickLength(0.06);
                     Reg4->GetXaxis()->SetNdivisions(505);
                     Reg4->GetYaxis()->SetNdivisions(505);
        	Reg4->GetXaxis()->SetTitle("Channel Numbers");
        	Reg4->GetYaxis()->SetTitle("Normalised ADC integral Violet 22V ( Run_{N}/Run_{1} )");
        	Reg4->GetYaxis()->SetTitleColor(6);
		Reg4->GetXaxis()->SetLabelFont(42);
		Reg4->GetXaxis()->SetTitleSize(0.03);
		Reg4->GetXaxis()->SetLabelSize(0.03);
		Reg4->GetXaxis()->SetTitleFont(42);
		Reg4->GetYaxis()->SetLabelFont(42);
		Reg4->GetYaxis()->SetLabelSize(0.03);
		Reg4->GetYaxis()->SetTitleSize(0.03);
		Reg4->GetYaxis()->SetTitleFont(42);
		Reg4->GetYaxis()->CenterTitle(true);
		Reg4->GetXaxis()->CenterTitle(true);

            Reg4->Draw("AP");
   	    TH1D *hbandR4 = new TH1D("hbandR4","histogramband", 2800, 0, 2800);
            hbandR4->GetYaxis()->SetRangeUser(0.95, 1.05);
           for(int ii=0;ii<2800;ii++)
             {/////  histgram band
             double xposi = ii;
             hbandR4->SetBinContent(xposi,1.);
             hbandR4->SetBinError(ii,0.1);
             }/////  histogram band

             hbandR4->SetFillColor(kYellow);
             hbandR4->SetFillStyle(3001);
	     hbandR4->Draw("E3same");



canvasR4->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R4NormVSchannels.pdf");

////////////individual region5 plot



            TCanvas *canvasR5 = new TCanvas("canvasR5","Be Region5 Blue 15V",1200,600);
            canvasR5->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasR5->SetFillStyle(0);
            canvasR5->SetFrameFillStyle(0);
	    TGraphErrors *Reg5 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/BeTgtCHRegi5normalised.txt","%*lg %*lg %lg%*lg %*lg %lg%lg");
            Reg5->SetMarkerSize(1);
            Reg5->SetMarkerStyle(20);
            Reg5->SetMarkerColor(kBlue);
            Reg5->SetLineColor(kBlue);

                     Reg5->GetXaxis()->SetTickLength(0.06);
                     Reg5->GetXaxis()->SetNdivisions(505);
                     Reg5->GetYaxis()->SetNdivisions(505);
        	Reg5->GetXaxis()->SetTitle("Channel Numbers");
        	Reg5->GetYaxis()->SetTitle("Normalised ADC integral Blue 15V ( Run_{N}/Run_{1} )");
        	Reg5->GetYaxis()->SetTitleColor(4);
		Reg5->GetXaxis()->SetLabelFont(42);
		Reg5->GetXaxis()->SetTitleSize(0.03);
		Reg5->GetXaxis()->SetLabelSize(0.03);
		Reg5->GetXaxis()->SetTitleFont(42);
		Reg5->GetYaxis()->SetLabelFont(42);
		Reg5->GetYaxis()->SetLabelSize(0.03);
		Reg5->GetYaxis()->SetTitleSize(0.03);
		Reg5->GetYaxis()->SetTitleFont(42);
		Reg5->GetYaxis()->CenterTitle(true);
		Reg5->GetXaxis()->CenterTitle(true);


            Reg5->Draw("AP");
   	    TH1D *hbandR5 = new TH1D("hbandR5","histogramband", 2800, 0, 2800);
            hbandR4->GetYaxis()->SetRangeUser(0.95, 1.05);
           for(int ii=0;ii<2800;ii++)
             {/////  histgram band
             double xposi = ii;
             hbandR5->SetBinContent(xposi,1.);
             hbandR5->SetBinError(ii,0.1);
             }/////  histogram band

             hbandR5->SetFillColor(kYellow);
             hbandR5->SetFillStyle(3001);
	     hbandR5->Draw("E3same");



  TFile *t61341R5CH2558=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61341.root");
  TH1F *h61341R5CH2558=(TH1F*)t61341R5CH2558->Get("fcalchannelRegion5integral_2558");

  TH1F *h61341R5CH1545=(TH1F*)t61341R5CH2558->Get("fcalchannelRegion5integral_1545");

  TFile *t61332R5CH2639=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61332.root");
  TH1F *h61332R5CH2639=(TH1F*)t61332R5CH2639->Get("fcalchannelRegion5integral_2639");

TPad *canvasR5_1 = new TPad("canvasR5_1", "newpad",0.15,0.4,0.9,0.84);
   	canvasR5_1->Divide(2,1);
   	canvasR5_1->Draw();


   	canvasR5_1->cd(1);
h61341R5CH1545->GetXaxis()->SetRangeUser(8000,16000);
h61341R5CH1545->Draw();

   	canvasR5_1->cd(2);
//h61341R5CH2558->GetXaxis()->SetRangeUser(0,20000);
//h61341R5CH2558->Draw();

h61332R5CH2639->GetXaxis()->SetRangeUser(8000,13000);
h61332R5CH2639->Draw();


canvasR5->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R5NormVSchannels.pdf");

     		}//////////CreateNormRunVsChannelBAND

/////////////////////////////////////////////////////
   void PrintHistogram()
     {//////////PrintHistogram
  TFile *t61332R5CH2639=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61332.root");
  TH1F *h61332R5CH2639=(TH1F*)t61332R5CH2639->Get("fcalchannelRegion5integral_2639");


  TFile *t61344R5CH2639=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61344.root");
  TH1F *h61344R5CH2639=(TH1F*)t61344R5CH2639->Get("fcalchannelRegion5integral_2639");

  TFile *t61341R5CH2558=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61341.root");
  TH1F *h61341R5CH2558=(TH1F*)t61341R5CH2558->Get("fcalchannelRegion5integral_2558");

  TFile *t61321R4CH1089=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
  TH1F *h61321R4CH1089=(TH1F*)t61321R4CH1089->Get("fcalchannelRegion4integral_1089");

  TFile *t61321R4CH2067=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
  TH1F *h61321R4CH2067=(TH1F*)t61321R4CH2067->Get("fcalchannelRegion4integral_2067");


  TFile *t61321R4CH2531=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
  TH1F *h61321R4CH2531=(TH1F*)t61321R4CH2531->Get("fcalchannelRegion4integral_2531");

  TFile *t61323R4CH2067=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61323.root");
  TH1F *h61323R4CH2067=(TH1F*)t61323R4CH2067->Get("fcalchannelRegion4integral_2067");


  TFile *t61321R3CH571=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
  TH1F *h61321R3CH571=(TH1F*)t61321R3CH571->Get("fcalchannelRegion3integral_571");

  TFile *t61325R3CH2283=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61325.root");
  TH1F *h61325R3CH2283=(TH1F*)t61325R3CH2283->Get("fcalchannelRegion3integral_2283");

  TFile *t61344R2CH1025=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61344.root");
  TH1F *h61344R2CH1025=(TH1F*)t61344R2CH1025->Get("fcalchannelRegion2integral_1025");



  TFile *t61344R2CH908=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61344.root");
  TH1F *h61344R2CH908=(TH1F*)t61344R2CH908->Get("fcalchannelRegion2integral_908");


  TFile *t61344R1CH86=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61344.root");
  TH1F *h61344R1CH86=(TH1F*)t61344R1CH86->Get("fcalchannelRegion1integral_86");



  TFile *t61321R1CH838=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
  TH1F *h61321R1CH838=(TH1F*)t61321R1CH838->Get("fcalchannelRegion1integral_838");



            TCanvas *canvasR5 = new TCanvas("canvasR5","Normalised Run VS CHannels at Diff Regions",1200,600);
            canvasR5->SetWindowSize(1000,600);
            canvasR5->SetFillStyle(0);
            canvasR5->SetFrameFillStyle(0);
            canvasR5->Divide(2,2);

	canvasR5->cd(1);
   	h61332R5CH2639->Draw();

	canvasR5->cd(2);
   	h61344R5CH2639->Draw();

	canvasR5->cd(3);
   	h61341R5CH2558->Draw();


            TCanvas *canvasR4 = new TCanvas("canvasR4","Normalised Run VS CHannels at Diff Regions",1200,600);
            canvasR4->SetWindowSize(1000,600);
            canvasR4->SetFillStyle(0);
            canvasR4->SetFrameFillStyle(0);
            canvasR4->Divide(2,2);





	canvasR4->cd(1);
   	h61321R4CH1089->Draw();

	canvasR4->cd(2);
   	h61321R4CH2067->Draw();


	canvasR4->cd(3);
   	h61321R4CH2531->Draw();


	canvasR4->cd(4);
   	h61323R4CH2067->Draw();

            TCanvas *canvasR3 = new TCanvas("canvasR3","Normalised Run VS CHannels at Diff Regions",1200,600);
            canvasR3->SetWindowSize(1000,600);
            canvasR3->SetFillStyle(0);
            canvasR3->SetFrameFillStyle(0);
            canvasR3->Divide(2,2);

	canvasR3->cd(1);
        h61321R3CH571->Draw();

	canvasR3->cd(2);
        h61325R3CH2283->Draw();


            TCanvas *canvasR2 = new TCanvas("canvasR2","Normalised Run VS CHannels at Diff Regions",1200,600);
            canvasR2->SetWindowSize(1000,600);
            canvasR2->SetFillStyle(0);
            canvasR2->SetFrameFillStyle(0);
            canvasR2->Divide(2,2);


	canvasR2->cd(1);
        h61344R2CH1025->Draw();

	canvasR2->cd(2);
	h61344R2CH908->Draw();

            TCanvas *canvasR1 = new TCanvas("canvasR1","Normalised Run VS CHannels at Diff Regions",1200,600);
            canvasR1->SetWindowSize(1000,600);
            canvasR1->SetFillStyle(0);
            canvasR1->SetFrameFillStyle(0);
            canvasR1->Divide(2,2);


	canvasR1->cd(1);
        h61321R1CH838->Draw();

	canvasR1->cd(2);
        h61344R1CH86->Draw();

      }//////////PrintHistogram
/////////////////////////////////////////////////////


       void FindingPeaksInHistogram()

          { //////FindingPeaksInHistogram

         TFile *t61321 =new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
         TH1F *hTotalE = (TH1F*)t61321->Get("fcalchannelRegion1integral_838");
         TSpectrum *s = new TSpectrum(10);///// 10 = here 5 peaks x 2 usually  # peaks x 2
         int nfound = s->Search(hTotalE,6,"",0.10);
         int npeaks=s->GetNPeaks();
         double *Xpeakposition = s->GetPositionX();
         cout<<npeaks<<endl;
         double desiredpeakcenter;////// selecting the peak
            for(int i=0;i<npeaks;i++)
             {////// npeaks


            if(Xpeakposition[i]<=300) continue;

            desiredpeakcenter = Xpeakposition[i];

             }////// npeaks

          } //////FindingPeaksInHistogram

////////////////////////////////////////////////////
   void SelectRings()
        {/////////SelectRings

      ofstream FirstRingfile;
      string FirstRingname;
      FirstRingname ="FirstRingChannels.txt";
      FirstRingfile.open(FirstRingname.c_str(),ios::out) ;

      ofstream SecondRingfile;
      string SecondRingname;
      SecondRingname  ="SecondRingChannels.txt";
      SecondRingfile.open(SecondRingname.c_str(),ios::out) ;


      ofstream OuterRingfile;
      string OuterRingname;
      OuterRingname  ="OuterRingChannels.txt";
      OuterRingfile.open(OuterRingname.c_str(),ios::out) ;


///////// FIRST RING/////////////


          for(int i=0;i<2800;i++)
             {/////////(int i=0;i<2800;i++)

               if  ( (i>987 && i<995))    {FirstRingfile<<i<<endl;}
               if  ( (i>1044 && i<1056))  {FirstRingfile<<i<<endl;}
               if  ( (i>1102 && i<1116))  {FirstRingfile<<i<<endl;}
               if  ( (i>1160 && i<1176))  {FirstRingfile<<i<<endl;}
               if  ( (i>1219 && i<1235))  {FirstRingfile<<i<<endl;}
               if  ( (i>1278 && i<1294))  {FirstRingfile<<i<<endl;}
               if  ( (i>1336 && i<1351))  {FirstRingfile<<i<<endl;}
               if  ( (i>1392 && i<1407))  {FirstRingfile<<i<<endl;}
               if  ( (i>1448 && i<1463))  {FirstRingfile<<i<<endl;}
               if  ( (i>1505 && i<1521))  {FirstRingfile<<i<<endl;}
               if  ( (i>1564 && i<1580))  {FirstRingfile<<i<<endl;}
               if  ( (i>1623 && i<1638))  {FirstRingfile<<i<<endl;}
               if  ( (i>1683 && i<1679))  {FirstRingfile<<i<<endl;}
               if  ( (i>1743 && i<1755))  {FirstRingfile<<i<<endl;}
               if  ( (i>1804 && i<1812))  {FirstRingfile<<i<<endl;}

             }/////////(int i=0;i<2800;i++)
FirstRingfile.close();

///////// SECOND RING/////////////


          for(int i=0;i<2800;i++)
             {/////////(int i=0;i<2800;i++)

               if  ( (i>490 && i<502)) {SecondRingfile<<i<<endl;}
               if  ( (i>539 && i<555)) {SecondRingfile<<i<<endl;}
               if  ( (i>589 && i<609)) {SecondRingfile<<i<<endl;}
               if  ( (i>642 && i<664)) {SecondRingfile<<i<<endl;}
               if  ( (i>696 && i<720)) {SecondRingfile<<i<<endl;}
               if  ( (i>750 && i<776)) {SecondRingfile<<i<<endl;}
               if  ( (i>805 && i<833)) {SecondRingfile<<i<<endl;}
               if  ( (i>861 && i<891)) {SecondRingfile<<i<<endl;}
               if  ( (i>918 && i<948)) {SecondRingfile<<i<<endl;}
               if  ( (i>975 && i<988)) {SecondRingfile<<i<<endl;}
               if  ( (i>994 && i<1007))  {SecondRingfile<<i<<endl;}
               if  ( (i>1034 && i<1045)) {SecondRingfile<<i<<endl;}
               if  ( (i>1055 && i<1066)) {SecondRingfile<<i<<endl;}
               if  ( (i>1092 && i<1103)) {SecondRingfile<<i<<endl;}
               if  ( (i>1115 && i<1126)) {SecondRingfile<<i<<endl;}
               if  ( (i>1151 && i<1161)) {SecondRingfile<<i<<endl;}
               if  ( (i>1175 && i<1185)) {SecondRingfile<<i<<endl;}
               if  ( (i>1210 && i<1220)) {SecondRingfile<<i<<endl;}
               if  ( (i>1234 && i<1245)) {SecondRingfile<<i<<endl;}
               if  ( (i>1268 && i<1279)) {SecondRingfile<<i<<endl;}
               if  ( (i>1293 && i<1304)) {SecondRingfile<<i<<endl;}
               if  ( (i>1328 && i<1337)) {SecondRingfile<<i<<endl;}
               if  ( (i>1350 && i<1360)) {SecondRingfile<<i<<endl;}
               if  ( (i>1384 && i<1393)) {SecondRingfile<<i<<endl;}
               if  ( (i>1406 && i<1416)) {SecondRingfile<<i<<endl;}
               if  ( (i>1440 && i<1449)) {SecondRingfile<<i<<endl;}
               if  ( (i>1462 && i<1472)) {SecondRingfile<<i<<endl;}
               if  ( (i>1496 && i<1506)) {SecondRingfile<<i<<endl;}
               if  ( (i>1520 && i<1530)) {SecondRingfile<<i<<endl;}
               if  ( (i>1555 && i<1565)) {SecondRingfile<<i<<endl;}
               if  ( (i>1579 && i<1589)) {SecondRingfile<<i<<endl;}
               if  ( (i>1614 && i<1624)) {SecondRingfile<<i<<endl;}
               if  ( (i>1637 && i<1648)) {SecondRingfile<<i<<endl;}
               if  ( (i>1673 && i<1684)) {SecondRingfile<<i<<endl;}
               if  ( (i>1696 && i<1707)) {SecondRingfile<<i<<endl;}
               if  ( (i>1733 && i<1744)) {SecondRingfile<<i<<endl;}
               if  ( (i>1754 && i<1765)) {SecondRingfile<<i<<endl;}
               if  ( (i>1792 && i<1805)) {SecondRingfile<<i<<endl;}
               if  ( (i>1811 && i<1824)) {SecondRingfile<<i<<endl;}
               if  ( (i>1850 && i<1881)) {SecondRingfile<<i<<endl;}
               if  ( (i>1908 && i<1938)) {SecondRingfile<<i<<endl;}
               if  ( (i>1966 && i<1994)) {SecondRingfile<<i<<endl;}
               if  ( (i>2023 && i<2049)) {SecondRingfile<<i<<endl;}
               if  ( (i>2079 && i<2103)) {SecondRingfile<<i<<endl;}
               if  ( (i>2135 && i<2157)) {SecondRingfile<<i<<endl;}
               if  ( (i>2190 && i<2210)) {SecondRingfile<<i<<endl;}
               if  ( (i>2245 && i<2260)) {SecondRingfile<<i<<endl;}
               if  ( (i>2296 && i<2309)) {SecondRingfile<<i<<endl;}

              }/////////(int i=0;i<2800;i++)
          SecondRingfile.close();



///////// OUTER RING/////////////


          for(int i=0;i<2800;i++)
             {/////////(int i=0;i<2800;i++)


               if  ( (i>=0 && i<=490)) {OuterRingfile<<i<<endl;}
               if  ( (i>=502 && i<=539)) {OuterRingfile<<i<<endl;}
               if  ( (i>=555 && i<=589)) {OuterRingfile<<i<<endl;}
               if  ( (i>=609 && i<=642)) {OuterRingfile<<i<<endl;}
               if  ( (i>=664 && i<=696)) {OuterRingfile<<i<<endl;}
               if  ( (i>=720 && i<=750)) {OuterRingfile<<i<<endl;}
               if  ( (i>=776 && i<=805)) {OuterRingfile<<i<<endl;}
               if  ( (i>=833 && i<=861)) {OuterRingfile<<i<<endl;}
               if  ( (i>=891 && i<=918)) {OuterRingfile<<i<<endl;}
               if  ( (i>=948 && i<=975)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1007 && i<=1034)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1066 && i<=1092)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1126 && i<=1151)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1185 && i<=1210)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1245 && i<=1268)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1304 && i<=1328)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1360 && i<=1384)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1416 && i<=1440)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1472 && i<=1496)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1530 && i<=1555)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1589 && i<=1614)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1648 && i<=1673)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1707 && i<=1733)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1765 && i<=1792)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1824 && i<=1850)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1881 && i<=1908)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1938 && i<=1966)) {OuterRingfile<<i<<endl;}
               if  ( (i>=1994 && i<=2023)) {OuterRingfile<<i<<endl;}
               if  ( (i>=2049 && i<=2079)) {OuterRingfile<<i<<endl;}
               if  ( (i>=2103 && i<=2135)) {OuterRingfile<<i<<endl;}
               if  ( (i>=2157 && i<=2190)) {OuterRingfile<<i<<endl;}
               if  ( (i>=2210 && i<=2245)) {OuterRingfile<<i<<endl;}
               if  ( (i>=2260 && i<=2296)) {OuterRingfile<<i<<endl;}
               if  ( (i>=2309 && i<=2799)) {OuterRingfile<<i<<endl;}

             }/////////(int i=0;i<2800;i++)
          OuterRingfile.close();



         }/////////SelectRings

///////////////////////////////////////////////////

     void CreateCanvasRings()
      {////// CreateRings


        TFile *t =new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/hd_root.root");
	TH3D* h3= (TH3D*)t->Get("Channel3D");
	TProfile2D* p2z = h3->Project3DProfile("xy");

	//p2z->Print("all");

	TEllipse  *circle1=new TEllipse(29,29,8);


   	circle1->SetLineColor(kRed);
   	circle1->SetFillStyle(0);
   	circle1->SetLineWidth(3);

	TEllipse  *circle2=new TEllipse(29,29,17);
   	circle2->SetLineColor(kBlue);
   	circle2->SetFillStyle(0);
   	circle2->SetLineWidth(3);

	//TEllipse  *circle3=new TEllipse(29,29,30);
	TEllipse  *circle3=new TEllipse(29,29,27);

   	circle3->SetLineColor(kGreen);
   	circle3->SetFillStyle(0);
   	circle3->SetLineWidth(3);


  	TCanvas *c1 = new TCanvas("c1","histogram",500,400);
   	c1->cd(1);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
	p2z->GetXaxis()->SetTitle("Column");
	p2z->GetXaxis()->SetLabelFont(42);
	p2z->GetXaxis()->SetLabelSize(0.035);
	p2z->GetXaxis()->SetTitleSize(0.035);
	p2z->GetXaxis()->SetTitleFont(42);
	p2z->GetYaxis()->SetTitle("Row");
	p2z->GetYaxis()->SetLabelFont(42);
	p2z->GetYaxis()->SetLabelSize(0.035);
	p2z->GetYaxis()->SetTitleSize(0.035);
	p2z->GetYaxis()->SetTitleFont(42);
	p2z->GetXaxis()->CenterTitle(true);
	p2z->GetYaxis()->CenterTitle(true);


   	p2z->Draw();
  // p2z->Draw("TEXT");
   //	circle1->SetLineWidth(3);
   //	circle1->Draw();

   //	circle2->SetLineWidth(3);
   //	circle2->Draw("same");

   	circle3->SetLineWidth(3);
   //	circle3->Draw("same");
   	circle3->Draw("same");


      }////// CreateRings

/////////////////////////////////////////////////
    void CreatecanvasNormRunsFirstRing()


           {/////////createcanvasNormRunsFirstRing loop

    char chnl[256];

    TCanvas *Regicanvas[6][2800];///////  one canvas for one region (5regions +1)
    TGraphErrors *ChNumber[2800];    ////// # CHANNELS
    TPaveText *Chpavetext[2800];    ////// # pave texts for each CHANNEL

    int rngch,rngarray[2800],rngcount=0;

     ifstream firstringfile;
     firstringfile.open("/home/user/Desktop/fcalbadchannelchandra/BeRuns/FirstRingChannels.txt") ;


        while(firstringfile>>rngch)
          {/////while loop
          rngarray[rngcount]=rngch;
          rngcount++;
          }////while loop
        firstringfile.close();


          for (int regi=1;regi<=5;regi++)
             { ////// for regi loop

               int num=0;
                 for(int jj=1;jj<=9;jj++)
                    {////// jj loop //// its just an intermediate loop


                int cdnum=0;

///// make sure  kk loop max number is a multiple of two numbers m and n equal with Divide (mxn)

              Regicanvas[regi][jj] = new TCanvas(Form("Chcanvas%d%d",regi,jj));
              Regicanvas[regi][jj]->Divide(4,5);


                 for(int kk=1;kk<=20;kk++)
                    {////// kk loop

            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
                 cdnum++;

                     Regicanvas[regi][jj]->cd(cdnum);

                      sprintf(chnl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dCH%d.txt",regi,rngarray[num]);

cout<<chnl<<endl;
                     ChNumber[num] = new TGraphErrors(chnl,"%lg %*lg %*lg %*lg %*lg %lg %lg");
                     ChNumber[num]->SetMarkerStyle(kOpenCircle);
                     ChNumber[num]->SetMarkerColor(kBlue);
                     ChNumber[num]->SetLineColor(kBlue);
                     ChNumber[num]->SetMarkerSize(1.);
                     ChNumber[num]->GetXaxis()->SetLabelSize(0.1);
                     ChNumber[num]->GetYaxis()->SetLabelSize(0.1);
                     ChNumber[num]->GetYaxis()->SetRangeUser(0.94,1.08);
                     ChNumber[num]->GetXaxis()->SetTickLength(0.06);
                     ChNumber[num]->GetXaxis()->SetNdivisions(505);
                     ChNumber[num]->GetYaxis()->SetNdivisions(510);
                     ChNumber[num]->Draw("ap");

                   //  ChNumber[num]->Draw("PESame");


   TPaveText *pt = new TPaveText(61329.57,1.082,61338.52,1.089,"br");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetTextColor(kBlue);
   pt->SetTextFont(102);
   pt->SetTextSize(0.1);
   TText *pt_LaTex = pt->AddText(Form("RING1 R%d CH %d",regi,rngarray[num]));
   pt->Draw();

                 num++;
                                  }////// kk loop

                        Regicanvas[regi][jj]->Update();
                                  }////// jj loop

                         } ////// for regi loop



          }//////////createcanvasNormRunsFirstRing loop
/////////////////////////////////////////////////////////

     void  CreatecanvasNormRunsSecondRing()
          {///////////////CreatecanvasNormRunsSecondRing


    	char chnl[256];

    	TCanvas *Regicanvas[6][2800];///////  one canvas for one region (5regions +1)
    	TGraphErrors *ChNumber[2800];    ////// # CHANNELS
    	TPaveText *Chpavetext[2800];    ////// # pave texts for each CHANNEL

    	int rngch,rngarray[2800],rngcount=0;

     	ifstream secondringfile;
     	secondringfile.open("/home/user/Desktop/fcalbadchannelchandra/BeRuns/SecondRingChannels.txt") ;


        	while(secondringfile>>rngch)
          		{/////while loop
          		rngarray[rngcount]=rngch;
          		rngcount++;
          		}////while loop
        	secondringfile.close();


          for (int regi=1;regi<=5;regi++)
             { ////// for regi loop

               int num=0;
                 for(int jj=1;jj<=23;jj++)
                    {////// jj loop //// its just an intermediate loop


                int cdnum=0;

///// make sure  kk loop max number is a multiple of two numbers m and n equal with Divide (mxn)

              Regicanvas[regi][jj] = new TCanvas(Form("Chcanvas%d%d",regi,jj));
              Regicanvas[regi][jj]->Divide(5,6);


                 for(int kk=1;kk<=30;kk++)
                    {////// kk loop

            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
                 cdnum++;

                     Regicanvas[regi][jj]->cd(cdnum);

   //TGraphErrors  *erband = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/errorbanpurposeonly.txt","%lg%lg%lg");
     //   erband-> SetFillColor(kYellow);   erband->SetFillStyle(1001);
     //   erband->Draw("E3AL");

                      sprintf(chnl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dCH%d.txt",regi,rngarray[num]);

                     ChNumber[num] = new TGraphErrors(chnl,"%lg %*lg %*lg %*lg %*lg %lg %lg");

      //cout<<chnl<<"\t"<<rngarray[num]<<endl;
                     ChNumber[num]->SetMarkerStyle(kOpenCircle);
                     ChNumber[num]->SetMarkerColor(kBlue);
                     ChNumber[num]->SetLineColor(kBlue);
                     ChNumber[num]->SetMarkerSize(1.);
                     ChNumber[num]->GetXaxis()->SetLabelSize(0.1);
                     ChNumber[num]->GetYaxis()->SetLabelSize(0.1);
                     ChNumber[num]->GetYaxis()->SetRangeUser(0.94,1.08);
                     ChNumber[num]->GetXaxis()->SetTickLength(0.06);
                     ChNumber[num]->GetXaxis()->SetNdivisions(505);
                     ChNumber[num]->GetYaxis()->SetNdivisions(510);
                     ChNumber[num]->Draw("ap");

                   //  ChNumber[num]->Draw("PESame");


   TPaveText *pt = new TPaveText(61329.57,1.082,61338.52,1.089,"br");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetTextColor(kBlue);
   pt->SetTextFont(102);
   pt->SetTextSize(0.1);
   TText *pt_LaTex = pt->AddText(Form("RING2 R%d CH %d",regi,rngarray[num]));
   pt->Draw();

                 num++;
                                  }////// kk loop

                        Regicanvas[regi][jj]->Update();
                                  }////// jj loop

                         } ////// for regi loop






          }///////////////CreatecanvasNormRunsSecondRing
//////////////////////////////////////////////////////////////

     void CreatecanvasNormRunsOuterRing()

           {///////////////CreatecanvasNormRunsOuterRing

    	char chnl[256];

    	TCanvas *Regicanvas[6][2800];///////  one canvas for one region (5regions +1)
    	TGraphErrors *ChNumber[2800];    ////// # CHANNELS
    	TPaveText *Chpavetext[2800];    ////// # pave texts for each CHANNEL

    	int rngch,rngarray[2800],rngcount=0;

     	ifstream outringfile;
     	outringfile.open("/home/user/Desktop/fcalbadchannelchandra/BeRuns/OuterRingChannels.txt") ;


        	while(outringfile>>rngch)
          		{/////while loop
          		rngarray[rngcount]=rngch;
          		rngcount++;
          		}////while loop
        	outringfile.close();


          for (int regi=4;regi<=4;regi++)
             { ////// for regi loop

               int num=0;
                 for(int jj=1;jj<=71;jj++)
                    {////// jj loop //// its just an intermediate loop


                int cdnum=0;

///// make sure  kk loop max number is a multiple of two numbers m and n equal with Divide (mxn)

              Regicanvas[regi][jj] = new TCanvas(Form("Chcanvas%d%d",regi,jj));
              Regicanvas[regi][jj]->Divide(3,9);


                 for(int kk=1;kk<=27;kk++)
                    {////// kk loop

            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
                 cdnum++;

                     Regicanvas[regi][jj]->cd(cdnum);

   //TGraphErrors  *erband = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/errorbanpurposeonly.txt","%lg%lg%lg");
     //   erband-> SetFillColor(kYellow);   erband->SetFillStyle(1001);
     //   erband->Draw("E3AL");

                      sprintf(chnl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dCH%d.txt",regi,rngarray[num]);

                     ChNumber[num] = new TGraphErrors(chnl,"%lg %*lg %*lg %*lg %*lg %lg %lg");

     // cout<<chnl<<"\t"<<rngarray[num]<<endl;
                     ChNumber[num]->SetMarkerStyle(kOpenCircle);
                     ChNumber[num]->SetMarkerColor(kBlue);
                     ChNumber[num]->SetLineColor(kBlue);
                     ChNumber[num]->SetMarkerSize(1.);
                     ChNumber[num]->GetXaxis()->SetLabelSize(0.1);
                     ChNumber[num]->GetYaxis()->SetLabelSize(0.1);
                     ChNumber[num]->GetYaxis()->SetRangeUser(0.94,1.08);
                     ChNumber[num]->GetXaxis()->SetTickLength(0.06);
                     ChNumber[num]->GetXaxis()->SetNdivisions(505);
                     ChNumber[num]->GetYaxis()->SetNdivisions(510);
                     ChNumber[num]->Draw("ap");

                   //  ChNumber[num]->Draw("PESame");


   TPaveText *pt = new TPaveText(61329.57,1.082,61338.52,1.089,"br");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetTextColor(kBlue);
   pt->SetTextFont(102);
   pt->SetTextSize(0.1);
   TText *pt_LaTex = pt->AddText(Form("RING3 R%d CH %d",regi,rngarray[num]));
   pt->Draw();

                 num++;
                                  }////// kk loop

                        Regicanvas[regi][jj]->Update();
                                  }////// jj loop

                         } ////// for regi loop




          }///////////////CreatecanvasNormRunsOuterRing

////////////////////////////////////////////////////////


///////////////////////////


  void  CreatecanvasOneChOneRegFirstRing()
    {/////////CreatecanvasOneChOneRegFirstRing

       	char chnl[256];

    	TCanvas *Regicanvas[6];///////  one canvas for one region (5regions +1)
    	TGraphErrors *ChNumber[2800];    ////// # CHANNELS
    	TPaveText *Chpavetext[2800];    ////// # pave texts for each CHANNEL

    	int rngch,rngarray[2800],rngcount=0;




          for (int ii=1;ii<=1;ii++)
             { ////// for regi loop

               int num=1240;
                int cdnum=0;

              Regicanvas[ii] = new TCanvas(Form("Chcanvas%d",ii));
              Regicanvas[ii]->Divide(1,5);

                 for(int kk=1;kk<=5;kk++)
                    {////// kk loop

            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
                 cdnum++;

                     Regicanvas[ii]->cd(cdnum);

   //TGraphErrors  *erband = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/errorbanpurposeonly.txt","%lg%lg%lg");
     //   erband-> SetFillColor(kYellow);   erband->SetFillStyle(1001);
     //   erband->Draw("E3AL");

                      sprintf(chnl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dCH%d.txt",kk,num);

                     ChNumber[num] = new TGraphErrors(chnl,"%lg %*lg %*lg %*lg %*lg %lg %lg");

     // cout<<chnl<<"\t"<<rngarray[num]<<endl;
                     ChNumber[num]->SetMarkerStyle(kOpenCircle);
                     ChNumber[num]->SetMarkerColor(kBlue);
                     ChNumber[num]->SetLineColor(kBlue);
                     ChNumber[num]->SetMarkerSize(1.);
                     ChNumber[num]->GetXaxis()->SetLabelSize(0.2);
                     ChNumber[num]->GetYaxis()->SetLabelSize(0.2);
                     ChNumber[num]->GetYaxis()->SetRangeUser(0.90,1.10);
                     ChNumber[num]->GetXaxis()->SetTickLength(0.06);
                     ChNumber[num]->GetXaxis()->SetNdivisions(505);
                     ChNumber[num]->GetYaxis()->SetNdivisions(505);
        ChNumber[num]->GetXaxis()->SetTitle("Run Numbers");
	ChNumber[num]->GetXaxis()->SetLabelFont(42);
	ChNumber[num]->GetXaxis()->SetLabelSize(0.12);
	ChNumber[num]->GetXaxis()->SetTitleSize(0);
	ChNumber[num]->GetXaxis()->SetTitleFont(42);
	ChNumber[num]->GetYaxis()->SetTitle("Normalised value");
	ChNumber[num]->GetYaxis()->SetLabelFont(42);
	ChNumber[num]->GetYaxis()->SetLabelSize(0.12);
	ChNumber[num]->GetYaxis()->SetTitleSize(0.12);
	ChNumber[num]->GetYaxis()->SetTitleOffset(0.29);

	ChNumber[num]->GetYaxis()->SetTitleFont(42);
	//ChNumber[num]->GetXaxis()->CenterTitle(true);
	ChNumber[num]->GetYaxis()->CenterTitle(true);
                     ChNumber[num]->Draw("ap");

                   //  ChNumber[num]->Draw("PESame");


   TPaveText *pt = new TPaveText(61330,1.07,61340.08,1.094308,"br");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetTextColor(kBlue);
   pt->SetTextFont(102);
   pt->SetTextSize(0.1);
   TText *pt_LaTex = pt->AddText(Form("R%d CH %d",kk,1240));
   pt->Draw();

                 num++;


                                  }////// kk loop

                       Regicanvas[ii]->Update();
                         } ////// for regi loop


     }/////////CreatecanvasOneChOneRegFirstRing

///////////////////////////////////////////////////////////



   void RegionsLEDcolor()
      {/////RegionsLEDcolor


  TFile *t1=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/61321_000.root");
  TH1F *h1=(TH1F*)t1->Get("hTotalE");


  TFile *t2=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/61321_003.root");
  TH1F *h2=(TH1F*)t2->Get("hTotalE");

  TFile *t3=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/61321_006.root");
  TH1F *h3=(TH1F*)t3->Get("hTotalE");

  TFile *t4=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/61321_010.root");
  TH1F *h4=(TH1F*)t4->Get("hTotalE");

  TFile *t5=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/61321_013.root");
  TH1F *h5=(TH1F*)t5->Get("hTotalE");

  TFile *t6=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/61321_016.root");
  TH1F *h6=(TH1F*)t6->Get("hTotalE");

            TCanvas *canvasR5 = new TCanvas("canvasR5","LED Colors",1200,600);
            canvasR5->SetWindowSize(1000,600);
            canvasR5->SetFillStyle(0);
            canvasR5->SetFrameFillStyle(0);
            canvasR5->Divide(3,2);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
		canvasR5->cd(1);
		h1->GetXaxis()->SetRangeUser(0,10000);
		h1->SetLineWidth(2);
		h1->SetLineColor(kViolet);
        h1->GetXaxis()->SetTitle(" Total FCAL Hit Energy");
	h1->GetXaxis()->SetLabelFont(42);
	h1->GetXaxis()->SetLabelSize(0.05);
	h1->GetXaxis()->SetTitleSize(0.05);
	h1->GetXaxis()->SetTitleFont(42);
	h1->GetYaxis()->SetTitle("#Entries");
	h1->GetYaxis()->SetLabelFont(42);
	h1->GetYaxis()->SetLabelSize(0.05);
	h1->GetYaxis()->SetTitleSize(0.05);
	h1->GetYaxis()->SetTitleOffset(0.29);
	h1->GetXaxis()->SetNdivisions(505);
        h1->GetYaxis()->SetNdivisions(502);
	h1->GetXaxis()->CenterTitle(true);
	h1->GetYaxis()->CenterTitle(true);

   		h1->Draw();

   TPaveText *pt1 = new TPaveText(7000,130,8000,150,"br");
   TText *pt1_LaTex = pt1->AddText("Violet 12V");
   pt1->SetBorderSize(0);
   pt1->SetFillColor(0);
   pt1->SetTextColor(kViolet);
   pt1->SetTextFont(102);
   pt1->SetTextSize(0.075);
   pt1->Draw();



		canvasR5->cd(2);
		h2->GetXaxis()->SetRangeUser(0,10000);
		h2->SetLineWidth(2);
		h2->SetLineColor(kBlue);
        h2->GetXaxis()->SetTitle(" Total FCAL Hit Energy");
	h2->GetXaxis()->SetLabelFont(42);
	h2->GetXaxis()->SetLabelSize(0.05);
	h2->GetXaxis()->SetTitleSize(0.05);
	h2->GetXaxis()->SetTitleFont(42);
	h2->GetYaxis()->SetTitle("#Entries");
	h2->GetYaxis()->SetLabelFont(42);
	h2->GetYaxis()->SetLabelSize(0.05);
	h2->GetYaxis()->SetTitleSize(0.05);
	h2->GetYaxis()->SetTitleOffset(0.29);
	h2->GetXaxis()->SetNdivisions(505);
        h2->GetYaxis()->SetNdivisions(502);
	h2->GetXaxis()->CenterTitle(true);
	h2->GetYaxis()->CenterTitle(true);

   		h2->Draw();

   TPaveText *pt2 = new TPaveText(7000,100,8000,120,"br");
   TText *pt2_LaTex = pt2->AddText("Blue 10V");
   pt2->SetBorderSize(0);
   pt2->SetFillColor(0);
   pt2->SetTextColor(kBlue);
   pt2->SetTextFont(102);
   pt2->SetTextSize(0.075);
   pt2->Draw();



		canvasR5->cd(3);
		h3->GetXaxis()->SetRangeUser(0,10000);
		h3->SetLineWidth(2);
		h3->SetLineColor(kGreen+2);
        h3->GetXaxis()->SetTitle(" Total FCAL Hit Energy");
	h3->GetXaxis()->SetLabelFont(42);
	h3->GetXaxis()->SetLabelSize(0.05);
	h3->GetXaxis()->SetTitleSize(0.05);
	h3->GetXaxis()->SetTitleFont(42);
	h3->GetYaxis()->SetTitle("#Entries");
	h3->GetYaxis()->SetLabelFont(42);
	h3->GetYaxis()->SetLabelSize(0.05);
	h3->GetYaxis()->SetTitleSize(0.05);
	h3->GetYaxis()->SetTitleOffset(0.29);
	h3->GetXaxis()->SetNdivisions(505);
        h3->GetYaxis()->SetNdivisions(502);
	h3->GetXaxis()->CenterTitle(true);
	h3->GetYaxis()->CenterTitle(true);

   		h3->Draw();

   TPaveText *pt3 = new TPaveText(7000,100,8000,120,"br");
   TText *pt3_LaTex = pt3->AddText("Green 29V");
   pt3->SetBorderSize(0);
   pt3->SetFillColor(0);
   pt3->SetTextColor(kGreen);
   pt3->SetTextFont(102);
   pt3->SetTextSize(0.075);
   pt3->Draw();



		canvasR5->cd(4);
		h4->GetXaxis()->SetRangeUser(0,10000);
		h4->SetLineWidth(2);
		h4->SetLineColor(kViolet);
        h4->GetXaxis()->SetTitle(" Total FCAL Hit Energy");
	h4->GetXaxis()->SetLabelFont(42);
	h4->GetXaxis()->SetLabelSize(0.05);
	h4->GetXaxis()->SetTitleSize(0.05);
	h4->GetXaxis()->SetTitleFont(42);
	h4->GetYaxis()->SetTitle("#Entries");
	h4->GetYaxis()->SetLabelFont(42);
	h4->GetYaxis()->SetLabelSize(0.05);
	h4->GetYaxis()->SetTitleSize(0.05);
	h4->GetYaxis()->SetTitleOffset(0.29);
	h4->GetXaxis()->SetNdivisions(505);
        h4->GetYaxis()->SetNdivisions(501);
	h4->GetXaxis()->CenterTitle(true);
	h4->GetYaxis()->CenterTitle(true);
	h4->GetYaxis()->SetRangeUser(0,55);

   		h4->Draw();

   TPaveText *pt4 = new TPaveText(2000,40,3000,50,"br");
   TText *pt4_LaTex = pt4->AddText("Violet 22V");
   pt4->SetBorderSize(0);
   pt4->SetFillColor(0);
   pt4->SetTextColor(kViolet);
   pt4->SetTextFont(102);
   pt4->SetTextSize(0.075);
   pt4->Draw();



		canvasR5->cd(5);
		h5->GetXaxis()->SetRangeUser(0,10000);
		h5->SetLineWidth(2);
		h5->SetLineColor(kBlue);
        h5->GetXaxis()->SetTitle(" Total FCAL Hit Energy");
	h5->GetXaxis()->SetLabelFont(42);
	h5->GetXaxis()->SetLabelSize(0.05);
	h5->GetXaxis()->SetTitleSize(0.05);
	h5->GetXaxis()->SetTitleFont(42);
	h5->GetYaxis()->SetTitle("#Entries");
	h5->GetYaxis()->SetLabelFont(42);
	h5->GetYaxis()->SetLabelSize(0.05);
	h5->GetYaxis()->SetTitleSize(0.05);
	h5->GetYaxis()->SetTitleOffset(0.29);
	h5->GetXaxis()->SetNdivisions(505);
        h5->GetYaxis()->SetNdivisions(502);
	h5->GetXaxis()->CenterTitle(true);
	h5->GetYaxis()->CenterTitle(true);
	h5->GetYaxis()->SetRangeUser(0,55);

   		h5->Draw();

   TPaveText *pt5 = new TPaveText(2000,40,3000,50,"br");
   TText *pt5_LaTex = pt5->AddText("Blue 15V");
   pt5->SetBorderSize(0);
   pt5->SetFillColor(0);
   pt5->SetTextColor(kBlue);
   pt5->SetTextFont(102);
   pt5->SetTextSize(0.075);
   pt5->Draw();
		canvasR5->cd(6);
		h6->GetXaxis()->SetRangeUser(0,10000);
        h6->GetXaxis()->SetTitle(" Total FCAL Hit Energy");
	h6->GetXaxis()->SetLabelFont(42);
	h6->GetXaxis()->SetLabelSize(0.05);
	h6->GetXaxis()->SetTitleSize(0.05);
	h6->GetXaxis()->SetTitleFont(42);
	h6->GetYaxis()->SetTitle("#Entries");
	h6->GetYaxis()->SetLabelFont(42);
	h6->GetYaxis()->SetLabelSize(0.05);
	h6->GetYaxis()->SetTitleSize(0.05);
	h6->GetYaxis()->SetTitleOffset(0.29);
	h6->GetXaxis()->SetNdivisions(505);
        h6->GetYaxis()->SetNdivisions(502);
	h6->GetXaxis()->CenterTitle(true);
	h6->GetYaxis()->CenterTitle(true);

   		h6->Draw();

   TPaveText *pt6 = new TPaveText(7000,0.5,8000,1,"br");
   TText *pt6_LaTex = pt6->AddText("NO LED");
   pt6->SetBorderSize(0);
   pt6->SetFillColor(0);
   pt6->SetTextColor(kBlack);
   pt6->SetTextFont(102);
   pt6->SetTextSize(0.075);
   pt6->Draw();


		canvasR5->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/LEDcolor.pdf");
















      }/////RegionsLEDcolor
/////////////////////////////////////////////

    void FCALtotalEnergyLEDcolors()
         {///////FCALtotalEnergyLEDcolors

  TFile *t0=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61321.root");
  TH1F *h0=(TH1F*)t0->Get("hTotalE");
            TCanvas *canvas0 = new TCanvas("canvas0","LED Colors",1200,600);
            canvas0->SetWindowSize(1000,600);
            canvas0->SetFillStyle(0);
            canvas0->SetFrameFillStyle(0);
           // canvas0->Divide(3,2);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
		canvas0->cd(1);
        h0->GetXaxis()->SetTitle(" Sum of FCAL ADC integrals for all channels");
	h0->GetXaxis()->SetLabelFont(42);
	h0->GetXaxis()->SetLabelSize(0.04);
	h0->GetXaxis()->SetTitleSize(0.04);
	h0->GetXaxis()->SetTitleFont(42);
	h0->GetXaxis()->SetRangeUser(0,15000);
	h0->GetYaxis()->SetTitle("#Entries");
	h0->GetYaxis()->SetLabelFont(42);
	h0->GetYaxis()->SetLabelSize(0.04);
	h0->GetYaxis()->SetTitleSize(0.04);
	h0->GetYaxis()->SetTitleOffset(0.29);
	h0->GetXaxis()->SetNdivisions(505);
        h0->GetYaxis()->SetNdivisions(502);
	h0->GetXaxis()->CenterTitle(true);
	h0->GetYaxis()->CenterTitle(true);

  	h0->Draw();

   	TPaveText *pt1 = new TPaveText(3700,944,6000,1050,"br");
   	TText *pt1_LaTex = pt1->AddText("Violet 12V");
   	pt1->SetBorderSize(0);
   	pt1->SetFillColor(0);
   	pt1->SetTextColor(kViolet);
   	pt1->SetTextFont(102);
   	pt1->SetTextSize(0.05);

   	pt1->Draw();
   	TArrow *arrow = new TArrow(3800,981.1967,2835.671,893.2385,0.01,"|>");
   	arrow->SetFillColor(51);
   	arrow->SetLineColor(51);
   	arrow->SetLineWidth(2);
   	arrow->SetFillStyle(1001);
   	arrow->Draw();

      TPaveText *pt2 = new TPaveText(5000,772.8747,7000,853.8888,"br");

   	TText *pt2_LaTex = pt2->AddText("Blue 10V");
   	pt2->SetBorderSize(0);
   	pt2->SetFillColor(0);
   	pt2->SetTextColor(kBlue);
   	pt2->SetTextFont(102);
   	pt2->SetTextSize(0.05);

   	pt2->Draw();

         TPaveText *pt3 = new TPaveText(6000,603.9024,7500,677.9725,"br");


   	TText *pt3_LaTex = pt3->AddText("Green 29V");
   	pt3->SetBorderSize(0);
  	 pt3->SetFillColor(0);
   	pt3->SetTextColor(kGreen);
   	pt3->SetTextFont(102);
   	pt3->SetTextSize(0.05);

   	pt3->Draw();

        TPaveText * pt4 = new TPaveText(8000,208.0905,9700,263.6431,"br");

 	TText *pt4_LaTex = pt4->AddText("Violet 22V");
   	pt4->SetBorderSize(0);
   	pt4->SetFillColor(0);
   	pt4->SetTextColor(kViolet);
   	pt4->SetTextFont(102);
   	pt4->SetTextSize(0.05);

   	pt4->Draw();

        TPaveText *pt5 = new TPaveText(10000,92.35609,12000,150.2233,"br");

   	TText *pt5_LaTex = pt5->AddText("Blue 15V");
   	pt5->SetBorderSize(0);
   	pt5->SetFillColor(0);
   	pt5->SetTextColor(kBlue);
   	pt5->SetTextFont(102);
   	pt5->SetTextSize(0.05);
   	pt5->Draw();

   	arrow = new TArrow(4964.93,740.4691,4338.677,689.5459,0.01,"|>");
   	arrow->SetFillColor(4);
   	arrow->SetLineColor(4);
   	arrow->SetLineWidth(2);
   	arrow->SetFillStyle(1001);
   	arrow->Draw();
   	arrow = new TArrow(6117.234,580.7555,5140.28,541.4058,0.01,"|>");
  	 arrow->SetFillColor(3);
  	 arrow->SetLineColor(3);
   	arrow->SetLineWidth(2);
  	 arrow->SetFillStyle(1001);
   	arrow->Draw();
   	arrow = new TArrow(8021.042,175.6849,7469.94,124.7617,0.01,"|>");
   	arrow->SetFillColor(51);
   	arrow->SetLineColor(51);
   	arrow->SetLineWidth(2);
   	arrow->SetFillStyle(1001);
   	arrow->Draw();
   	arrow = new TArrow(10576.15,94.67078,9473.948,85.41202,0.01,"|>");
   	arrow->SetFillColor(4);
   	arrow->SetLineColor(4);
   	arrow->SetLineWidth(2);
   	arrow->SetFillStyle(1001);
   	arrow->Draw();

		canvas0->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/FCALtotalEnergyLEDcolors.pdf");


         }//////FCALtotalEnergyLEDcolors

////////////////////////////////////

    void NpeaksCHHistogram()
      {///////AbnormalfitCHHistogram





            TCanvas *canvasR0 = new TCanvas("canvasR0","Be #peaks in All LEDs",1200,600);
            canvasR0->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasR0->SetFillStyle(0);
            canvasR0->SetFrameFillStyle(0);

	    TGraphErrors *R0 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R1NpeaksVSchannels.txt","%*lg %*lg %lg%lg ");
            R0->SetMarkerSize(1);
            R0->SetMarkerStyle(20);
            R0->SetMarkerColor(kViolet);
            R0->SetLineColor(kViolet);
            R0->GetXaxis()->SetNdivisions(505);
            R0->GetXaxis()->SetTitle("Channel number");
            R0->GetYaxis()->SetTitle("# Peaks");
            R0->GetYaxis()->SetNdivisions(505);
            R0->GetYaxis()->SetRangeUser(2,7);
		R0->GetXaxis()->CenterTitle(true);
		R0->GetYaxis()->CenterTitle(true);
	    R0->Draw("AP");

            TGraphErrors *R1 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R2NpeaksVSchannels.txt","%*lg %*lg %lg%lg ");
            R1->SetMarkerSize(1);
            R1->SetMarkerStyle(20);
            R1->SetMarkerColor(kBlue);
            R1->SetLineColor(kBlue);
            R1->GetXaxis()->SetNdivisions(505);
            R1->GetYaxis()->SetNdivisions(505);
	    R1->Draw("Psame");

	    TGraphErrors *R2 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R3NpeaksVSchannels.txt","%*lg %*lg %lg%lg ");
            R2->SetMarkerSize(1);
            R2->SetMarkerStyle(20);
            R2->SetMarkerColor(kGreen);
            R2->SetLineColor(kGreen);
            R2->GetXaxis()->SetNdivisions(505);
            R2->GetYaxis()->SetNdivisions(505);
	    R2->Draw("Psame");

            TGraphErrors *R3 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R4NpeaksVSchannels.txt","%*lg %*lg %lg%lg ");
            R3->SetMarkerSize(1);
            R3->SetMarkerStyle(20);
            R3->SetMarkerColor(kViolet);
            R3->SetLineColor(kViolet);
            R3->GetXaxis()->SetNdivisions(505);
            R3->GetYaxis()->SetNdivisions(505);
	    R3->Draw("Psame");


	    TGraphErrors *R4 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R5NpeaksVSchannels.txt","%*lg %*lg %lg%lg ");
            R4->SetMarkerSize(1);
            R4->SetMarkerStyle(20);
            R4->SetMarkerColor(kCyan);
            R4->SetLineColor(kCyan);
            R4->GetXaxis()->SetNdivisions(505);
            R4->GetYaxis()->SetNdivisions(507);
            R4->GetYaxis()->SetRangeUser(1,6);
	    R4->Draw("Psame");









  TLegend* tl = new TLegend(.12, .6, .2, .85,NULL,"brNDC");

  tl->AddEntry(R0,"Violet 12V","p")->SetTextColor(kViolet);
  tl->AddEntry(R1,"Blue   10V","p")->SetTextColor(kBlue);
  tl->AddEntry(R2,"Green  29V","p")->SetTextColor(kGreen);
  tl->AddEntry(R3,"Violet 22V","p")->SetTextColor(kViolet);
  tl->AddEntry(R4,"Blue   15V (Cyan)","p")->SetTextColor(kBlue);

   tl->SetFillColor(0);
   tl->SetTextFont(40);
   tl->SetTextSize(0.04);
   tl->SetLineWidth(0);
   tl->SetLineColor(1);
   tl->SetBorderSize(0);
   tl->Draw();





  TFile *t61323R5CH1632=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61323.root");
  TH1F *h61323R5CH1632=(TH1F*)t61323R5CH1632->Get("fcalchannelRegion5integral_1632");


  TFile *t61322R4CH2639=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61322.root");
  TH1F *h61322R4CH2639=(TH1F*)t61322R4CH2639->Get("fcalchannelRegion4integral_2639");


  TFile *t61344R3CH2283=new TFile("/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib61344.root");
  TH1F *h61344R3CH2283=(TH1F*)t61344R3CH2283->Get("fcalchannelRegion3integral_2283");


	TPad *canvasR0_1 = new TPad("canvasR5_1", "newpad",0.45,0.45,0.8,0.85);
   	canvasR0_1->Divide(3,1);
   	canvasR0_1->Draw();

   	canvasR0_1->cd(1);
	h61323R5CH1632->GetXaxis()->SetRangeUser(0,10000);
	h61323R5CH1632->Draw();

   	canvasR0_1->cd(2);
	h61322R4CH2639->GetXaxis()->SetRangeUser(0000,30000);
	h61322R4CH2639->Draw();

   	canvasR0_1->cd(3);
	h61344R3CH2283->GetXaxis()->SetRangeUser(0,20000);
	h61344R3CH2283->Draw();

canvasR0->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/ChvsPeaks.pdf");

            TCanvas *canvasRR0 = new TCanvas("canvasRR0","Be #peaks in All RUNS",1200,600);
            canvasRR0->SetWindowSize(1000,600);
            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            canvasRR0->SetFillStyle(0);
            canvasRR0->SetFrameFillStyle(0);

	    TGraphErrors *RR0 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R1NpeaksVSchannels.txt","%lg %*lg %*lg%lg ");
            RR0->SetMarkerSize(1);
            RR0->SetMarkerStyle(20);
            RR0->SetMarkerColor(kViolet);
            RR0->SetLineColor(kViolet);
            RR0->GetXaxis()->SetNdivisions(505);
            RR0->GetXaxis()->SetTitle("Run number");
            RR0->GetYaxis()->SetTitle("# Peaks");
            RR0->GetYaxis()->SetNdivisions(505);
            RR0->GetYaxis()->SetRangeUser(2,7);
		RR0->GetXaxis()->CenterTitle(true);
		RR0->GetYaxis()->CenterTitle(true);
	    RR0->Draw("AP");

            TGraphErrors *RR1 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R2NpeaksVSchannels.txt","%lg %*lg %*lg%lg ");
            RR1->SetMarkerSize(1);
            RR1->SetMarkerStyle(20);
            RR1->SetMarkerColor(kBlue);
            RR1->SetLineColor(kBlue);
            RR1->GetXaxis()->SetNdivisions(505);
            RR1->GetYaxis()->SetNdivisions(505);
	    RR1->Draw("Psame");

	    TGraphErrors *RR2 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R3NpeaksVSchannels.txt","%lg %*lg %*lg%lg ");
            RR2->SetMarkerSize(1);
            RR2->SetMarkerStyle(20);
            RR2->SetMarkerColor(kGreen);
            RR2->SetLineColor(kGreen);
            RR2->GetXaxis()->SetNdivisions(505);
            RR2->GetYaxis()->SetNdivisions(505);
	    RR2->Draw("Psame");

            TGraphErrors *RR3 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R4NpeaksVSchannels.txt","%lg %*lg %*lg%lg ");
            RR3->SetMarkerSize(1);
            RR3->SetMarkerStyle(20);
            RR3->SetMarkerColor(kViolet);
            RR3->SetLineColor(kViolet);
            RR3->GetXaxis()->SetNdivisions(505);
            RR3->GetYaxis()->SetNdivisions(505);
	    RR3->Draw("Psame");


	    TGraphErrors *RR4 = new TGraphErrors("/home/user/Desktop/fcalbadchannelchandra/BeRuns/R5NpeaksVSchannels.txt","%lg %*lg %*lg%lg ");
            RR4->SetMarkerSize(1);
            RR4->SetMarkerStyle(20);
            RR4->SetMarkerColor(kCyan);
            RR4->SetLineColor(kCyan);
            RR4->GetXaxis()->SetNdivisions(505);
            RR4->GetYaxis()->SetNdivisions(507);
            RR4->GetYaxis()->SetRangeUser(1,6);
	    RR4->Draw("Psame");
canvasRR0->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/RunsvsPeaks.pdf");

      }///////AbnormalfitCHHistogram



/////////////////////////////////////////////////////////////////////////////////////////////////

void BeRunstestOCT10()
{//////start



     //   Createnormfiles();

    //   CreateNormRunVsChannelBAND();



    //  NpeaksCHHistogram();

     //  PrintHistogram();

     //  RegionsLEDcolor();

     //   FCALtotalEnergyLEDcolors();


    //   FindingPeaksInHistogram();
     //  SelectRings();
    //   CreateCanvasRings();
    //   CreatecanvasNormRunsFirstRing();
    //  CreatecanvasNormRunsSecondRing();
    //   CreatecanvasNormRunsOuterRing();
     //  CreatecanvasOneChOneRegFirstRing();



}/////end
