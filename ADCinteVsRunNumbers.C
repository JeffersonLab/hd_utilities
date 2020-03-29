#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
using namespace std;

int RunNumber[]={61321,61322,61323,61325};


//int RunNumber[]={61321,61322,61323,61325,61327,61329,61330,61331,61332,61333,
   //              61334,61335,61336,61337,61340,61341,61342,61343,61344,61345,
    //             61346,61347,61348,61349,61350,61352,61353,61357,61358,61359,61360};



int RN,RNcount;//////RNcount is #number of files in RunNumber array ex:5or200depends on #root files to analyse,
int ROOTfilenum;///// ROOTnum is ROOT FILE NUMBER
TFile *t[4];/////Number of root files has to be equal to the  RunNumber[] array {#}


void fetchingtheRootfile()
        {
       char nl[256]; 
       ROOTfilenum=RunNumber[RNcount];///// ROOT FILE NUMBER
       sprintf(nl,"/home/user/Desktop/fcalbadchannelchandra/calib%d.root", ROOTfilenum);
       cout<< nl<<"\t"<<RNcount<<endl;
       t[RNcount]= new TFile(nl);
        }



void fetchingtherunnumber()
      {RN=RunNumber[RNcount];}

//////////////////////creating normalised files
/////////

/////particular channel and particular region for all runs

//////////

void createnormfiles()
{//////////normalised files start



ofstream outfiles;///// ADC output file for all channels in all regions
string outname;///// string for ADC output file for all channels in all regions


TFile *t[4];/////Number of root files has to be equal to the  RunNumber[] array {#}


  //  for(int ch=0;ch<2800;ch++)

    for(int ch=0;ch<5;ch++)
      {//////channel loop
        for(int regi=1;regi<=5;regi++)
            {/////regi loop
                   stringstream a;    a<<ch;       ////channel number
                   stringstream b;    b<<regi;       ////region number
                   outname  ="CH"+a.str();
                   outname +="Regi"+b.str();
                   outname +=".txt";
                   outfiles.open(outname.c_str(),ios::out) ; 
               for( int ii=0;ii<4;ii++)
                {///////number of fruns loop
                    char hname[256];char nl[256];
                    TH1F *h[4]; int count=0; 
                    float channel[2800],region[2800]; 
                    float filenum[2800],mean[2800],ermean[2800],Ratio[2800],erRatio[2800];
      // cout<<"channel="<<ch<<"\t"<<"region="<<regi<<"\t"<<RunNumber[ii]<<endl;

                     int  ROOTfilenum=RunNumber[ii];///// ROOT FILE NUMBER
                     sprintf(nl,"/home/user/Desktop/fcalbadchannelchandra/calib%d.root", ROOTfilenum);
       //cout<< nl<<endl;
                      t[ii]= new TFile(nl);
                      sprintf(hname,"fcalchannelRegion%dintegral_%d",regi,ch);
                      h[ii] = (TH1F*)t[ii]->Get(hname);
       //  cout<<hname<<endl;
                      double center=h[ii]->GetMean();
                      double width=h[ii]->GetRMS();
                      TF1* fit = new TF1("fit", "gaus",  center-width, center+width);
                      h[ii]->Fit("fit","","",500,40000 );

                     if (fit->GetParameter(1) <=0.0 ) continue;

                if(ii==0){mean[0]=fit->GetParameter(1);ermean[0]=fit->GetParError(1);}


                     mean[ii]    =  fit->GetParameter(1);
                     ermean[ii]  =  fit->GetParError(1);
                     Ratio[ii]   =  mean[ii]/mean[0];

cout<<Ratio[ii]<< "\t"<<Ratio[ii]*Ratio[ii]<<"\t"<< ermean[0]<<"\t"<<ermean[0]*ermean[0]<<"\t"<<ermean[ii]<<"\t"<<ermean[ii]*ermean[ii]<<"\t"<<(sqrt ( (ermean[ii] *ermean[ii] ) +  (  ( Ratio[ii]* Ratio[ii])  *  (ermean[0]* ermean[0] ) )    ))<<endl;


                     erRatio[ii] = (1./mean[0])*(sqrt ( (ermean[ii] *ermean[ii] ) +  (  ( Ratio[ii]* Ratio[ii])  *  (ermean[0]* ermean[0] ) )    ))  ;

               outfiles<<RunNumber[ii]<<"\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<<  Ratio[ii]<<"\t"<<erRatio[ii]<<endl;


//cout<<mean[0]<<"\t\t"<<fit->GetParameter(1)<<"\t\t"<<fit->GetParError(1)<<"\t\t"<< (fit->GetParameter(1) )/mean[0]<<"\t\t"<<Ratio[ii]<<"\t\t"<<Ratio[ii]*Ratio[ii]<<"\t"<< ermean[0]<<"\t"<<ermean[0]*ermean[0]<<"\t"<<ermean[ii]<<"\t\t"<<ermean[ii]*ermean[ii]<<"\t\t"<<(1/mean[0])*(sqrt ( (ermean[ii] *ermean[ii] ) +  (  ( Ratio[ii]* Ratio[ii])  *  (ermean[0]* ermean[0] ) )    ))<<endl;




                       count++;
                 
                }///////number of runs loop

outfiles.close();


            }/////regi loop



       }//////channel loop




}///////normalised files end





////////////////   creating canvas for the ADC integrals VS run numbers for each channel

//////////  Channel integral canvas for each channel each region for all runs
void createfitcanvas()
{///////// createfitcanvas  start

TCanvas *Chcanvas[10];

TGraphErrors *RegiNumber[6];    //////change the array size depending on number of regions +1  
int num = 0;
char chnl[256];

       // for(int ch=0;ch<2800;ch++)
      // {//////// channelloop for all 2800 channels

         for(int ch=0;ch<1;ch++)
             {//////// channel loop
              Chcanvas[ch] = new TCanvas(Form("Chcanvas%d",ch));
          //  Chcanvas[ch]->Divide(3,2);
              Chcanvas[ch]->Divide(1,5);
gStyle->SetOptStat(0);////stats numbers
gStyle->SetOptTitle(0);/// top header
                for(int regi=1;regi<=5;regi++)  ////// regions 1 to 5 
                  {///// region loop
                     num++;
                    Chcanvas[ch]->cd(regi);
//cout<<ch<<"\t"<<regi<<endl;
                    sprintf(chnl,"/home/user/Desktop/fcalbadchannelchandra/fcalADCintegral/CH%dRegi%d.txt",ch,regi);
                    RegiNumber[regi] = new TGraphErrors(chnl,"%lg %*lg %*lg %lg %lg ");
//cout<<chnl<<endl;
                     RegiNumber[regi]->SetMarkerStyle(20);
                     RegiNumber[regi]->SetMarkerSize(1.);
                     RegiNumber[regi]->GetXaxis()->SetLabelSize(0.12);
                     RegiNumber[regi]->GetYaxis()->SetLabelSize(0.12);
                     RegiNumber[regi]->GetYaxis()->SetRangeUser(0.95,1.05);
                     RegiNumber[regi]->GetYaxis()->SetNdivisions(507);
                     RegiNumber[regi]->Draw("ap");
                  }///// region loop
         Chcanvas[ch]->Update();

              }///// channel loop

}///////// createfitcanvas  end

















////////////////////////////////main function starts from now
void ADCinteVsRunNumbers()
{//////start



createnormfiles();

createfitcanvas();//////// calling the createcanvas function





}/////end





































