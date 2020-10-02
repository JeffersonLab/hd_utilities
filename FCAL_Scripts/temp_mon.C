#include <TString.h>
#include <sstream>
#include <string>
#include <iostream>
#include <cctype>
#include <cstdlib>


temp_mon(){
	gStyle->SetStatH(0.20);
	gStyle->SetStatW(0.20);
	gStyle->SetTitleSize(0.05,"xy");
	//gStyle->SetTitleSize(0.08,"h");
	gStyle->SetLabelSize(0.05,"xy");
	//gPad->SetLogy();
	

//TFile *file = TFile::Open("mon_nov20_10to3pm.root");
//TFile *file = TFile::Open("mon_imon_nov20.root");
//TFile *file = TFile::Open("mon_nov21_23.root");
//TFile *file = TFile::Open("mon_dec8_10.root");
TFile *file = TFile::Open("../MonitoringData/Bases_dec17_18.root");
   TIter   next(file->GetListOfKeys());
   TKey *key;
   TTree *T;
   int k = 0;
   
   TH1F* h1=0; 
   TH1F *h2 = new TH1F("h2","Temperature Mean;Mean Base Temperature [C]",100,10,-10);
   TH1F *h3 = new TH1F("h3","Temperature RMS;RMS Base Temperature [C]",100,10,-10);
   TH2F *h4 = new TH2F("h4", "Temperature Map; X; Y",61,-30,30,61,-30,30);
   while ((key=(TKey*)next())) {
      if (strcmp(key->GetClassName(),"TTree")) continue; //do not use keys that are not trees
      T = (TTree*)file->Get(key->GetName()); //fetch the Tree header in memory
      
      TString keyname=key->GetName();
      keyname.ReplaceAll("M","-");
      keyname.ReplaceAll("FCAL_hv_"," ");
      keyname.ReplaceAll("_"," ");
      
      stringstream extract;
      extract << keyname ;
      string word1, word2 = "";
     
     //while(!extract.fail())
     //{
        extract >> word1 >> word2;
       
     //  cout << "X: "<<  word1 << " Y: "<< word2 << endl;
       
       int x = std::atoi(word1.c_str());
       int y = std::atoi(word2.c_str());
       //cout << "X: "<< x << " Y: "<< y << endl; 
      /*
      string buf;
      stringstream ss(keyname);
      vector <string> tokens;
       while (ss >> buf) cout << buf<< endl;
      */
      //TString first, second;
      //keyname >> first >> second; 
      //cout <<keyname << endl;
      int totalEvents    = T->GetEntries();
       delete h1;
       TH1F *h1 = new TH1F("h1","my histogram",100,10,-10);
      // T->Draw("v0set-vmon>>h1","status == 1 && v0set > 0","goff");
        T->Draw("temp>>h1","","goff");
       double m = h1->GetMean();
       double r  = h1->GetRMS();
       h2->Fill(m);
       h3->Fill(r);
       h4->Fill(x,y,m);
       //if ( m > 50 || r > 20 ) {
       //k = k+1;
       //cout << "Channel: "<<  keyname << " Mean: " << m << " RMS: "<< r<<endl;
       //cout <<   keyname << " \t" << m << "\t "<< r<<endl;
       //}
     // printf("%50s\t%i\n",keyname,totalEvents);
      //delete h1;
      delete T;
   }
   h4->SetMinimum(20);
   h4->Draw("colz");
   TCanvas * c2 = new TCanvas("c2", "c", 600, 600);
   c2->Divide(1,2);
   c2->cd(1);
   gPad->SetLogy();
   h2->Draw();
   c2->cd(2);
   gPad->SetLogy();
   h3->Draw();
     //cout << "Number of channels > 5 mA: " << k << endl;
   //cout << "# of channels with Temperature diff. > 50 V: " << k << endl;
   }
