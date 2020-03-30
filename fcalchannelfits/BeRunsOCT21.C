#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
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








///////////////////////




    void createnormfiles()
         {///////////createnormfiles


     ofstream largeoutfile;///// ADC fit output file for all channels in all regions
     string largeoutname;///// string for ADC fit output file for all channels in all region

    largeoutname  ="HeTgtCHRegi.txt";
    largeoutfile.open(largeoutname.c_str(),ios::out|ios::app) ;

     ofstream wrongchoutfile;/////suspecious channels in all regions
     wrongchoutfile.open("wrongchannels.txt",ios::out|ios::app);



     ofstream tempoutfile;///// ADC fit output file for all channels in all regions
     string tempoutname;///// string for ADC fit output file for all channels in all region



     ofstream NpeaksfileR1;/////NPeaks in  channels in  region1
     NpeaksfileR1.open("R1NpeaksVSchannels.txt",ios::out|ios::app);

     ofstream NpeaksfileR2;/////NPeaks in  channels in  region2
     NpeaksfileR2.open("R2NpeaksVSchannels.txt",ios::out|ios::app);

     ofstream NpeaksfileR3;/////NPeaks in  channels in  region3
     NpeaksfileR3.open("R3NpeaksVSchannels.txt",ios::out|ios::app);

     ofstream NpeaksfileR4;/////NPeaks in  channels in  region4
     NpeaksfileR4.open("R4NpeaksVSchannels.txt",ios::out|ios::app);

     ofstream NpeaksfileR5;/////NPeaks in  channels in  region5
     NpeaksfileR5.open("R5NpeaksVSchannels.txt",ios::out|ios::app);




                   char nl[1024]; char hname[1024];



             //  for( int ii=0;ii<19;ii++)/////Number of runs = 19 for Be runs
               for( int ii=0;ii<1;ii++)/////Number of runs = 19 for Be runs
                {///////number of runs loop

                     int  ROOTfilenum=RunNumber[ii];///// ROOT FILE NUMBER
                     sprintf(nl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/calib%d.root", ROOTfilenum);
                     t[ii]= new TFile(nl);

                     // for(int regi=1;regi<=5;regi++)
                      for(int regi=1;regi<=2;regi++)
                        {/////regi loop


                                TH1F *h[2800]; /////Number of histograms for each region for Be runs

                          //  for(int ch=0;ch<2800;ch++)
                             for(int ch=2046;ch<2049;ch++)
                		 {//////channel loop
          			stringstream a;    a<<ch;          ////// channel #number
           		        stringstream b;    b<<regi;       ////    region number
           		        stringstream d;    d<<ii;       ////    region number
            			tempoutname  ="R"+b.str();
            			tempoutname +="CH";
            			tempoutname += a.str();
           			tempoutname +=".txt";

                  		tempoutfile.open(tempoutname.c_str(),ios::out| ios::app) ;

				sprintf(hname,"fcalchannelRegion%dintegral_%d",regi,ch);
                      		h[ch] = (TH1F*)t[ii]->Get(hname);


         TSpectrum *s = new TSpectrum(10);///// 10 = here 5 peaks x 2 usually  # peaks x 2
         int nfound = s->Search(h[ch],6,"",0.10);

         int npeaks=s->GetNPeaks(); //// this section is to select the peak from pedestals by selecting the highest X position of the peak

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


              TF1* fit = new TF1("fit","gaus",desiredpeakcenter-(desiredpeakcenter/5.),desiredpeakcenter+(desiredpeakcenter/5.));

             TFitResultPtr fitr = h[ch]->Fit("fit","S","",desiredpeakcenter-(desiredpeakcenter/5.),desiredpeakcenter+(desiredpeakcenter/5.));


	           int status = int ( fitr );//// status =0 means accurate fit, status =4 abnormal fit

                 
                         if ((status==0) && (npeaks<=2) )
                             {////// fit accurate check

          largeoutfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<endl;
          tempoutfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<endl;

                              }////// fit accurate check

                           else
                              {////// else fit accurate
                                 
                              //   if( (h[ch]->GetEntries())>=1000.) continue;
      wrongchoutfile<<RunNumber[ii]<<"\t"<<regi<<"\t"<<ch<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t"<<npeaks<<"\t"<<h[ch]->GetEntries()<<endl;

                              }////// else fit accurate

               tempoutfile.close();

                	}//////channel loop


                      }/////regi loop


                }///////number of runs loop

             largeoutfile.close();
    	wrongchoutfile.close();


        }///////////createnormfiles





///////////////////////

     void SplittingNormRunintoRegions()

           {////////SplittingNormRunintoRegions

     ifstream tempinfile;///// ADC fit input file
     string tempinname;///// string for ADC fit input file

     ofstream tempoutfile;///// normalised ADC output file
     string tempoutname;///// string for normalised  ADC fit output file


     ofstream temp1outfile;///// last normalised ADC fit output file
     string temp1outname;///// string for  last normalised ADC fit output file

     ofstream outbandfile;///// outside band output file
     string outbandname;///// string for  outband output file

                      for(int regi=1;regi<=5;regi++)

                        {/////regi loop

           		        stringstream s1;    s1<<regi;       ////    region number
                      temp1outname  ="R"+s1.str();
                      temp1outname +="NORMALISED.txt";
                  		temp1outfile.open(temp1outname.c_str(),ios::out| ios::app) ;

                      outbandname ="R"+s1.str();
                      outbandname +="outsideband.txt";
                      outbandfile.open(outbandname.c_str(),ios::out| ios::app) ;
                            for(int ch=0;ch<2800;ch++)

                		 {//////channel loop


	                        float a,b,c,d,e,f,g;
                                float ratio,erratio,lastratio[1],lasterratio[1];
                                vector<float> temp1;
                                vector<float> temp2;


            			stringstream s2;    s2<<ch;          ////// channel #number

            			tempinname  ="R"+s1.str();
            			tempinname +="CH";
            			tempinname += s2.str();
           			  tempinname +=".txt";
                  tempinfile.open(tempinname.c_str(),ios::in) ;



            			tempoutname  ="R"+s1.str();
            			tempoutname +="CH";
            			tempoutname += s2.str();
            			tempoutname += "NORMALISED";
           			  tempoutname +=".txt";

                  tempoutfile.open(tempoutname.c_str(),ios::out| ios::app) ;

        	                   while(!tempinfile.eof())
          		              {/////while loop

                                        tempinfile>>a>>b>>c>>d>>e;

                                        temp1.push_back(d);//// run number      mean value
                                        temp2.push_back(e);///// run number     error in mean
                                        ratio = d/temp1[0];//// last run mean divide by  first run mean
                                 
                                        erratio = (1./temp1[0]) * sqrt( (e*e) + ( (ratio*ratio)*(temp2[0]*temp2[0]) ) );
                                        tempoutfile<<a<<"\t"<<b<<"\t"<<c<<"\t"<<d<<"\t"<<e<<"\t"<<ratio<<"\t"<<erratio<<endl;
                                        lastratio[0] = ratio;
                                        lasterratio[0] = erratio;


          		              }/////while loop

         ///// acceptable band is between  0.9 to 1.1 i.e; 1.0 +_ 10% (errorbar 10%)
                        if((ratio<=0.9) || (ratio>1.1))
                          {outbandfile<<a<<"\t"<<b<<"\t"<<c<<"\t"<<d<<"\t"<<e<<"\t"<<lastratio[0]<<"\t"<<lasterratio[0]<<endl;}
                          else {temp1outfile<<a<<"\t"<<b<<"\t"<<c<<"\t"<<d<<"\t"<<e<<"\t"<<lastratio[0]<<"\t"<<lasterratio[0]<<endl;}






					tempoutfile.close();
					tempinfile.close();
                		 }//////channel loop

            temp1outfile.close();
          outbandfile.close();
                        }/////regi loop





          }////////SplittingNormRunintoRegions

  //////////////////////////////////////////////
  void  CanvasNormRunVsChannel()

   		{//////////CreateNormRunVsChannel

         int num=0;
         char regnl[256];
    	   TGraphErrors *RegNumber[6];

         int LEDCOLORS[6];

         LEDCOLORS[1] = kMagenta; LEDCOLORS[2] = kBlue; LEDCOLORS[3] = kGreen+2;
         LEDCOLORS[4] = kViolet; LEDCOLORS[5] = kCyan;


         int colors[6];
         colors[1] = kViolet; colors[2] = kBlue; colors[3] = kGreen+2;
         colors[4] = kViolet+2; colors[5] = kBlue;

         double size[6];
         size[1] = 1.0; size[2] = 1.0; size[3] = 1.0;
         size[4] = 1.0; size[5] = 1.0;

         int style[6];
         style[1] = 20; style[2] = 21; style[3] = 22;
         style[4] = 23; style[5] = 25;

            TCanvas *canvas = new TCanvas("canvas"," He Normalised Run VS Channels at Diff Regions",1200,600);
            canvas->SetWindowSize(1000,600);
            canvas->SetFillStyle(0);
            canvas->SetFrameFillStyle(0);


         TMultiGraph *mg = new TMultiGraph();
           for(int regi=1;regi<=5;regi++)
                           {////regi loop

            sprintf(regnl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dNORMALISED.txt",regi);
            RegNumber[num] = new TGraphErrors(regnl,"%*lg %*lg %lg %*lg %*lg %lg %lg");
            RegNumber[num]->SetMarkerSize(size[regi]);
            RegNumber[num]->SetMarkerStyle(style[regi]);
            RegNumber[num]->SetMarkerColor(colors[regi]);
            RegNumber[num]->SetLineColor(colors[regi]);
            mg->Add(RegNumber[num]);
            num++;
                               }////regi loop

/////  histogram band

   TH1D *hband = new TH1D("hband","histogramband", 2800, 0, 2799);
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


/////////////individual region plot

    	     TCanvas *Regicanvas[6];///////  one canvas for one region (5regions +1)
    	     TGraphErrors *RegioNumber[6];    ////// # Regions
       	     char regionl[256];

                   for(int kk=1;kk<=5;kk++)
                      {//////region canvas
                       Regicanvas[kk] = new TCanvas(Form("Chcanvas%d",kk));

            gStyle->SetOptTitle(0);/// top header
            gStyle->SetOptStat(0);
            Regicanvas[kk]->SetFillStyle(0);
            Regicanvas[kk]->SetFrameFillStyle(0);

                      sprintf(regionl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dNORMALISED.txt",kk);

                      RegioNumber[kk] = new TGraphErrors(regionl,"%*lg %*lg %lg %*lg %*lg %lg %lg");
                      RegioNumber[kk]->SetMarkerSize(size[kk]);
                      RegioNumber[kk]->SetMarkerStyle(style[kk]);
                      RegioNumber[kk]->SetMarkerColor(colors[kk]);
                      RegioNumber[kk]->SetLineColor(colors[kk]);


                     RegioNumber[kk]->GetXaxis()->SetTickLength(0.06);
                     RegioNumber[kk]->GetXaxis()->SetNdivisions(505);
                     RegioNumber[kk]->GetYaxis()->SetNdivisions(505);
                     RegioNumber[kk]->GetYaxis()->SetRangeUser(0,2.);                     
        	     RegioNumber[kk]->GetXaxis()->SetTitle("Channel Numbers");
        	     RegioNumber[kk]->GetYaxis()->SetTitle("Normalised ADC integral   ( Run_{N}/Run_{1} )");
        	     RegioNumber[kk]->GetYaxis()->SetTitleColor(6);
		     RegioNumber[kk]->GetXaxis()->SetLabelFont(42);
		     RegioNumber[kk]->GetXaxis()->SetTitleSize(0.03);
		     RegioNumber[kk]->GetXaxis()->SetLabelSize(0.03);
		     RegioNumber[kk]->GetXaxis()->SetTitleFont(42);
		     RegioNumber[kk]->GetYaxis()->SetLabelFont(42);
		     RegioNumber[kk]->GetYaxis()->SetLabelSize(0.03);
		     RegioNumber[kk]->GetYaxis()->SetTitleSize(0.03);
		     RegioNumber[kk]->GetYaxis()->SetTitleFont(42);
		     RegioNumber[kk]->GetYaxis()->CenterTitle(true);
		     RegioNumber[kk]->GetXaxis()->CenterTitle(true);









                     RegioNumber[kk]->Draw("ap");

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



                       }//////region canvas


  TLegend* tleg6 = new TLegend(.12, .6, .2, .85,NULL,"brNDC");
  tleg6->AddEntry(RegNumber[0],"Violet 12V","p")->SetTextColor(kViolet);
  tleg6->AddEntry(RegNumber[1],"Blue   10V","p")->SetTextColor(kBlue);
  tleg6->AddEntry(RegNumber[2],"Green  29V","p")->SetTextColor(kGreen);
  tleg6->AddEntry(RegNumber[3],"Violet 22V","p")->SetTextColor(kViolet);
  tleg6->AddEntry(RegNumber[4],"Blue   15V ","p")->SetTextColor(kBlue);

   tleg6->SetFillColor(0);
   tleg6->SetTextFont(40);
   tleg6->SetTextSize(0.04);
   tleg6->SetLineWidth(0);
   tleg6->SetLineColor(1);
   tleg6->SetBorderSize(0);
   tleg6->Draw();








      }//////////CreateNormRunVsChannel

//////////////////////////////////////////////////////

   void PlottingPeaksVsChannelandRuns()
        {///// PlottingPeaksVsChannelandRuns



         int colors[6];
         colors[1] = kViolet; colors[2] = kBlue; colors[3] = kGreen+2;
         colors[4] = kViolet+2; colors[5] = kBlue;

         double size[6];
         size[1] = 1.0; size[2] = 1.0; size[3] = 1.0;
         size[4] = 1.0; size[5] = 1.0;

         int style[6];
         style[1] = 20; style[2] = 21; style[3] = 22;
         style[4] = 23; style[5] = 25;
         

    	     TGraphErrors *RegioNumber[6];    ////// # Regions
       	     char regionl[256];
             TMultiGraph *mg = new TMultiGraph();
                   for(int kk=1;kk<=5;kk++)
                      {//////region 
                      sprintf(regionl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dNpeaksVSchannels.txt",kk);
                      RegioNumber[kk] = new TGraphErrors(regionl,"%lg %*lg %lg %*lg");
                      RegioNumber[kk]->SetMarkerSize(size[kk]);
                      RegioNumber[kk]->SetMarkerStyle(style[kk]);
                      RegioNumber[kk]->SetMarkerColor(colors[kk]);
                      RegioNumber[kk]->SetLineColor(colors[kk]);  
                     // RegioNumber[kk]->SetTextColor(colors[kk]);
                      mg->Add(RegioNumber[kk]);
                       }//////region 



    	        TCanvas *AllRegicanvas=new TCanvas("AllRegicanvas"," #nPeaks in channel vs runs in all regions",1200,600);
                gStyle->SetOptTitle(0);/// top header
                gStyle->SetOptStat(0);
             
                AllRegicanvas->cd();
                mg->Draw("AP");
                mg->GetXaxis()->SetTickLength(0.06);
                mg->GetXaxis()->SetNdivisions(505);
                mg->GetYaxis()->SetNdivisions(507);
                mg->GetYaxis()->SetRangeUser(0,3500.);
        	mg->GetXaxis()->SetTitle("Run Numbers");
        	mg->GetYaxis()->SetTitle("Channel Numbers");
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
                RegioNumber[1]->SetTitle("Violet 12V");
                RegioNumber[2]->SetTitle("Blue   10V");
                RegioNumber[3]->SetTitle("Green  29V");
                RegioNumber[4]->SetTitle("Violet 22V");
                RegioNumber[5]->SetTitle("Blue   15V");



                AllRegicanvas->BuildLegend();

   
                AllRegicanvas->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/Channelshas3ormorepeaksVSBeRuns.pdf");


       }///// PlottingPeaksVsChannelandRuns

/////////////////////////////////////////////////

      void Plotting2DFcal()
           {//////Plotting2DFcal



/*

      ifstream infile; 
      infile.open("channelnumberandROWSCOLUMNS.txt",ios::in);

      ifstream infile0; 
      infile0.open("wrongchannels.txt",ios::in);

      ifstream infile1; 
      infile1.open("R1NpeaksVSchannels.txt",ios::in);
      ifstream infile2; 
      infile2.open("R2NpeaksVSchannels.txt",ios::in);
      ifstream infile3; 
      infile3.open("R3NpeaksVSchannels.txt",ios::in);
      ifstream infile4; 
      infile4.open("R4NpeaksVSchannels.txt",ios::in);
      ifstream infile5; 
      infile5.open("R5NpeaksVSchannels.txt",ios::in);




      ofstream outfile0; 
      outfile0.open("AllRegibadchannels.txt",ios::out);
      ofstream outfile1; 
      outfile1.open("R1badchannels.txt",ios::out);
      ofstream outfile2; 
      outfile2.open("R2badchannels.txt",ios::out);
      ofstream outfile3; 
      outfile3.open("R3badchannels.txt",ios::out);
      ofstream outfile4; 
      outfile4.open("R4badchannels.txt",ios::out);
      ofstream outfile5; 
      outfile5.open("R5badchannels.txt",ios::out);




       float a,b,c,d,e,f,g,h,i,j,k,l,m,n,p,q,r,s,t,u,v,w,x,y,z,d1,e1,f1,g1 ;



        	                   while(infile>>a>>b>>c)
          		              {/////while loop


        	                        while(infile0>>d1>>e1>>f1>>g1)
          		                   {/////while loop1
                                             if(a==f1){
						outfile0<<a<<"\t"<<b<<"\t"<<c<<"\t"<<f1<<endl;                     
                                                      }   
                                           }////// while loop1
            					infile0.clear();//now clear the second stream (B)
            					infile0.seekg(0, infile0.beg);

        	                        while(infile1>>d>>e>>f>>g)
          		                   {/////while loop1
                                             if(a==f){
						outfile1<<a<<"\t"<<b<<"\t"<<c<<"\t"<<f<<endl;                     
                                                      }   
                                           }////// while loop1
            					infile1.clear();//now clear the second stream (B)
            					infile1.seekg(0, infile1.beg);

        	                        while(infile2>>h>>i>>j>>k)
          		                   {/////while loop2
                                             if(a==j){
						outfile2<<a<<"\t"<<b<<"\t"<<c<<"\t"<<j<<endl;                     
                                                      }   
                                           }////// while loop2
            					infile2.clear();//now clear the second stream (B)
            					infile2.seekg(0, infile2.beg);
        	                        while(infile3>>l>>m>>n>>p)
          		                   {/////while loop3
                                             if(a==n){
						outfile3<<a<<"\t"<<b<<"\t"<<c<<"\t"<<n<<endl;                     
                                                      }   
                                           }////// while loop3
            					infile3.clear();//now clear the second stream (B)
            					infile3.seekg(0, infile3.beg);
        	                        while(infile4>>q>>r>>s>>t)
          		                   {/////while loop4
                                             if(a==s){
						outfile4<<a<<"\t"<<b<<"\t"<<c<<"\t"<<s<<endl;                     
                                                      }   
                                           }////// while loop4
            					infile4.clear();//now clear the second stream (B)
            					infile4.seekg(0, infile4.beg);

        	                        while(infile5>>u>>v>>w>>x)
          		                   {/////while loop5
                                             if(a==w){
						outfile5<<a<<"\t"<<b<<"\t"<<c<<"\t"<<w<<endl;                     
                                                      }   
                                           }////// while loop5
            					infile5.clear();//now clear the second stream (B)
            					infile5.seekg(0, infile5.beg);


                                     }////// while loop




outfile1.close();infile.close();
infile0.close();infile1.close();
infile2.close();infile3.close();
infile4.close();infile5.close();

*/
         int colors[6];
         colors[1] = kViolet; colors[2] = kBlue; colors[3] = kGreen+2;
         colors[4] = kViolet+2; colors[5] = kBlue;

         double size[6];
         size[1] = 1.8; size[2] = 1.8; size[3] = 1.8;
         size[4] = 1.8; size[5] = 1.8;

         int style[6];
         style[1] = 20; style[2] = 21; style[3] = 22;
         style[4] = 23; style[5] = 25;


    	     TGraphErrors *RegioNumber[6];    ////// # Regions
       	     char regionl[256];
             TMultiGraph *mg = new TMultiGraph();
                   for(int kk=1;kk<=5;kk++)
                      {//////region 
                      sprintf(regionl,"/home/user/Desktop/fcalbadchannelchandra/BeRuns/R%dbadchannels.txt",kk);
                      RegioNumber[kk] = new TGraphErrors(regionl,"%*lg%lg%lg%*lg");
                      RegioNumber[kk]->SetMarkerSize(size[kk]);
                 //     RegioNumber[kk]->SetMarkerStyle(style[kk]);
                  //    RegioNumber[kk]->SetMarkerColor(colors[kk]);



                      RegioNumber[kk]->SetMarkerStyle(29);                              
                      RegioNumber[kk]->SetMarkerColor(kRed+2);
                      RegioNumber[kk]->SetLineColor(colors[kk]);  
                   //   RegioNumber[kk]->SetTextColor(colors[kk]);
                      mg->Add(RegioNumber[kk]);

                       }//////region 
         		

                TGraph *h0= new TGraph("/home/user/Desktop/fcalbadchannelchandra/BeRuns/channelnumberandROWSCOLUMNS.txt","%*lg%lg%lg%*lg");
		        h0->SetMarkerColor(kBlack);
   			h0->SetMarkerStyle(25);
                        h0->SetMarkerSize(1.6);
                                          
                        mg->Add(h0);
                
                TGraph *h1= new TGraph("/home/user/Desktop/fcalbadchannelchandra/BeRuns/AllRegibadchannels.txt","%*lg%lg%lg%*lg");
		        h1->SetMarkerColor(kRed+4);
   			h1->SetMarkerStyle(29);
                        h1->SetMarkerSize(1.6);


   
                //        mg->Add(h1);




    	        TCanvas *FCAL2Dcanvas=new TCanvas("FCAL2Dcanvas"," position of channel with more peaks in all regions",600,600);
                gStyle->SetOptTitle(0);/// top header
                gStyle->SetOptStat(0);
             
                FCAL2Dcanvas->cd();
                mg->Draw("AP");


                mg->GetXaxis()->SetNdivisions(505);
                mg->GetYaxis()->SetNdivisions(505);
                mg->GetXaxis()->SetRangeUser(-2,70.);
                mg->GetXaxis()->SetTickLength(0.02);
                mg->GetYaxis()->SetRangeUser(-2,70.);
                mg->GetYaxis()->SetTickLength(0.01);
        	mg->GetXaxis()->SetTitle("Columns");
        	mg->GetXaxis()->SetTitleOffset(0.5);
        	mg->GetYaxis()->SetTitle("Rows");
        	mg->GetYaxis()->SetTitleOffset(0.5);
		mg->GetXaxis()->SetLabelFont(42);
		mg->GetXaxis()->SetTitleSize(0.04);
		mg->GetXaxis()->SetLabelSize(0.04);
		mg->GetXaxis()->SetTitleFont(42);
		mg->GetYaxis()->SetLabelFont(42);
		mg->GetYaxis()->SetLabelSize(0.04);
		mg->GetYaxis()->SetTitleSize(0.04);
		mg->GetYaxis()->SetTitleFont(42);
		mg->GetYaxis()->CenterTitle(true);
		mg->GetXaxis()->CenterTitle(true);

                h0->SetTitle("FCAL Channels");
             //   h1->SetTitle("All suspicious channels");
            //    RegioNumber[1]->SetTitle("Violet 12V");
             //   RegioNumber[2]->SetTitle("Blue   10V");
             //   RegioNumber[3]->SetTitle("Green  29V");
             //   RegioNumber[4]->SetTitle("Violet 22V");
               RegioNumber[5]->SetTitle("All suspicious channels");



                FCAL2Dcanvas->BuildLegend();

////// creating the rings


	TEllipse  *circle1=new TEllipse(29,29,8);


   	circle1->SetLineColor(kRed);
   	circle1->SetFillStyle(0);
   	circle1->SetLineWidth(3);

	TEllipse  *circle2=new TEllipse(29,29,17);
   	circle2->SetLineColor(kBlue);
   	circle2->SetFillStyle(0);
   	circle2->SetLineWidth(3);

	TEllipse  *circle3=new TEllipse(29,29,30);
   	circle3->SetLineColor(kGreen);
   	circle3->SetFillStyle(0);
   	circle3->SetLineWidth(3);


   	circle1->SetLineWidth(3);
   	circle1->Draw("same");

   	circle2->SetLineWidth(3);
   	circle2->Draw("same");

   	circle3->SetLineWidth(3);
   	circle3->Draw("same");




                FCAL2Dcanvas->Print("/home/user/Desktop/fcalbadchannelchandra/BeRuns/BetgtPositionofChannelhas3ormorepeaks.pdf");



         }//////Plotting2DFcal

/////////////////////////////////////////////////////////////////////////////////////////////////

void BeRunsOCT21()
{//////start



     // createnormfiles();


    ///    SplittingNormRunintoRegions();

     //    CanvasNormRunVsChannel();

    //      PlottingPeaksVsChannelandRuns();
       
          Plotting2DFcal();

}/////end
