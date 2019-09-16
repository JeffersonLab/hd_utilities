double min(double a, double b)
{
   if(a<b) return a;
   else    return b;
}

void geteff(TH1F* h_raw, TH1F *h_aft, TH1F *h_eff)
{
  const int nbin = 28;
  double Nexp[nbin], Nobs[nbin], eff[nbin], sigma[nbin];
  for(int i=1;i<nbin+1;i++)
  {
      Nobs[i-1]=h_aft->GetBinContent(i);
      Nexp[i-1]=h_raw->GetBinContent(i);
      if(Nexp[i-1]==0){
          eff[i-1]=0;}
      else  {
          eff[i-1]=Nobs[i-1]/Nexp[i-1];}
      sigma[i-1]=sqrt(eff[i-1]*(1-eff[i-1])/Nexp[i-1]);
  }
     
  for(int i=0;i<nbin;i++)
  {
     h_eff->SetBinContent(i+1, eff[i]);
     h_eff->SetBinError(i+1, sigma[i]);
  }    
} 

// Get the photon dection efficiency distribution using "pi0 costheta" method
void CalEff()
{


   double Pgam1[4],Pgam2[4];
   double Egam1_B, Egam1_F, Egam2_B, Egam2_F;
   TLorentzVector gam1, gam2, pi0;

   //change this to the path and name of your own ROOT file   
   TChain *chain_da = new TChain("pi0pippim__B3_T4_Tree");//tree name
   chain_da->Add("entries_da.root");
   chain_da->SetBranchAddress("Pgamma1", &Pgam1);
   chain_da->SetBranchAddress("Pgamma2", &Pgam2);
   chain_da->SetBranchAddress("Egamma1_BCAL", &Egam1_B);
   chain_da->SetBranchAddress("Egamma1_FCAL", &Egam1_F);
   chain_da->SetBranchAddress("Egamma2_BCAL", &Egam2_B);
   chain_da->SetBranchAddress("Egamma2_FCAL", &Egam2_F);

   int nevent_da=(int)chain_da->GetEntries();
   int nbin=20;//number of bins for the cos\theta histogram
   double Ppi0,costhe,low=0,max=1,Mpi0=0.135;
   double wbin=(max-low)/nbin,var1,var2,count[20],count_max=0.0;
   double countB[20],countF[20],countBF[20],countB_max=0.0,countF_max=0.0,countBF_max=0.0;
   char histname[500], histnameB[500], histnameF[500],histnameBF[500];
   TH1F* h_Egam[20];
   TH1F* h_Egam_B[20];
   TH1F* h_Egam_F[20];
   TH1F* h_Egam_BF[20];

       TH1F *h_Egam_aft = new TH1F("h_Egam_mc","",28,0,1.4); 
       TH1F *h_Egam_raw = new TH1F("h_Eraw_mc","",28,0,1.4);
   TH1F *h_costhe_aft = new TH1F("h_costhe_mc","",20,0,1);

     TH1F *h_Egam_aft_B = new TH1F(  "h_Egam_mc_B","",28,0,1.4); 
     TH1F *h_Egam_raw_B = new TH1F(  "h_Eraw_mc_B","",28,0,1.4);
   TH1F *h_costhe_aft_B = new TH1F("h_costhe_mc_B","",20,0,1);

     TH1F *h_Egam_aft_F = new TH1F(  "h_Egam_mc_F","",28,0,1.4); 
     TH1F *h_Egam_raw_F = new TH1F(  "h_Eraw_mc_F","",28,0,1.4);
   TH1F *h_costhe_aft_F = new TH1F("h_costhe_mc_F","",20,0,1);

     TH1F *h_Egam_aft_BF = new TH1F(  "h_Egam_mc_BF","",28,0,1.4); 
     TH1F *h_Egam_raw_BF = new TH1F(  "h_Eraw_mc_BF","",28,0,1.4);
   TH1F *h_costhe_aft_BF = new TH1F("h_costhe_mc_BF","",20,0,1);
   for(int j=0;j<nbin;j++)
   { 
       sprintf(histname,"h_Egam_%d",j);   
       sprintf(histnameB,"h_Egam_B_%d",j);   
       sprintf(histnameF,"h_Egam_F_%d",j);   
       sprintf(histnameBF,"h_Egam_BF_%d",j);   
       h_Egam[j]=new TH1F(histname,"",28,0,1.4);
       h_Egam_B[j] =new TH1F(histnameB,"",28,0,1.4);
       h_Egam_F[j] =new TH1F(histnameF,"",28,0,1.4);
       h_Egam_BF[j]=new TH1F(histnameBF,"",28,0,1.4);
       count[j]=0.0;
       countB[j]=0.0;
       countF[j]=0.0;
       countBF[j]=0.0;
   }
   for(int i=0;i<nevent_da;i++)
   {
     chain_da->GetEntry(i);
     gam1.SetPxPyPzE(Pgam1[0],Pgam1[1],Pgam1[2],Pgam1[3]);
     gam2.SetPxPyPzE(Pgam2[0],Pgam2[1],Pgam2[2],Pgam2[3]);
     pi0=gam1+gam2;

     Ppi0=sqrt(pi0.E()*pi0.E()-Mpi0*Mpi0); 
     costhe=fabs(gam2.E()-gam1.E())/Ppi0;

       h_Egam_aft->Fill(min(gam1.E(), gam2.E()));
       h_costhe_aft->Fill(costhe);

     if(Egam1_B>0&&Egam2_B>0){// two photon both in BCAL
       h_Egam_aft_B->Fill(min(gam1.E(), gam2.E()));
       h_costhe_aft_B->Fill(costhe);
     }
 
     if(Egam1_F>0&&Egam2_F>0){// two photon both in FCAL
       h_Egam_aft_F->Fill(min(gam1.E(), gam2.E()));
       h_costhe_aft_F->Fill(costhe);
     }
      
     if((Egam1_B>0&&Egam2_F>0)||(Egam1_F>0&&Egam2_B>0)){// one photon in BCAL and another in FCAL
       h_Egam_aft_BF->Fill(min(gam1.E(), gam2.E()));
       h_costhe_aft_BF->Fill(costhe);
     }

    //to get the energy histogram from each bin of cos\theta
    for(int j=0;j<nbin;j++)
    {
       var1=low+wbin*j;
       var2=low+wbin*(j+1);
       if(costhe<var2&&costhe>var1){
           count[j]++;
           h_Egam[j]->Fill(min(gam1.E(), gam2.E()));        
           if(Egam1_B>0&&Egam2_B>0){ h_Egam_B[j]->Fill(min(gam1.E(), gam2.E())); countB[j]++;}
           if(Egam1_F>0&&Egam2_F>0){ h_Egam_F[j]->Fill(min(gam1.E(), gam2.E())); countF[j]++;}
           if((Egam1_B>0&&Egam2_F>0)||(Egam1_F>0&&Egam2_B>0)){
               h_Egam_BF[j]->Fill(min(gam1.E(), gam2.E())); 
               countBF[j]++;
           }
       }
   }
 }
        
double NN[28][20];  
for(int i=0;i<nbin;i++)
{
   if(count_max<count[i]) count_max=count[i];
   if(countB_max <countB[i])  countB_max =countB[i];
   if(countF_max <countF[i])  countF_max =countF[i];
   if(countBF_max<countBF[i]) countBF_max=countBF[i];
}
 // cout<<"countB_max = "<<countB_max<<endl;

   //to get the energy histogram if photon be detected with 100% efficiency
   for(int i=0;i<nbin;i++)
   {
     if(count[i]  >0.1) h_Egam[i]->Scale(count_max/count[i]);
     if(countB[i] >0.1) h_Egam_B[i]->Scale( countB_max/countB[i]);
     if(countF[i] >0.1) h_Egam_F[i]->Scale( countF_max/countF[i]);
     if(countBF[i]>0.1) h_Egam_BF[i]->Scale(countBF_max/countBF[i]);
      h_Egam_raw->Add(h_Egam[i],1);
      h_Egam_raw_B->Add( h_Egam_B[i],1);
      h_Egam_raw_F->Add( h_Egam_F[i],1);
      h_Egam_raw_BF->Add(h_Egam_BF[i],1);
   }

   
   TH1F *h_eff=new TH1F("h_eff_da",";E_{#gamma};eff",28,0,1.4); 
   TH1F *h_eff_B=new TH1F("h_eff_B_da",";E_{#gamma};eff",28,0,1.4); 
   TH1F *h_eff_F=new TH1F("h_eff_F_da",";E_{#gamma};eff",28,0,1.4); 
   TH1F *h_eff_BF=new TH1F("h_eff_BF_da",";E_{#gamma};eff",28,0,1.4); 

   //function to cal the efficiency and stat. error 
   geteff(h_Egam_raw, h_Egam_aft, h_eff);
   geteff(h_Egam_raw_B, h_Egam_aft_B, h_eff_B);
   geteff(h_Egam_raw_F, h_Egam_aft_F, h_eff_F);
   geteff(h_Egam_raw_BF, h_Egam_aft_BF, h_eff_BF);
    

   TCanvas *c2 = new TCanvas("c2","c2",1000,800);
   c2->Divide(2,2);
   c2->cd(1);
   h_eff->GetYaxis()->SetRangeUser(0.0,1.1);
   h_eff->Draw();
   c2->cd(2);
   h_eff_B->GetYaxis()->SetRangeUser(0.0,1.1);
   h_eff_B->Draw();
   c2->cd(3);
   h_eff_F->GetYaxis()->SetRangeUser(0.0,1.1);
   h_eff_F->Draw();
   c2->cd(4);
   h_eff_BF->GetYaxis()->SetRangeUser(0.0,1.1);
   h_eff_BF->Draw();
 
  //save the result
   TFile *ff=new TFile("output.root","recreate");
   h_eff->Write();
   h_eff_B->Write();
   h_eff_F->Write();
   h_eff_BF->Write();
   ff->Close();
}

