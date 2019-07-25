//to cal the difference between MC and data that was be 
void geteff() 
{

      TFile *d =new TFile("output.root","update"); 
      TH1F *d1=(TH1F*)d->Get("h_eff_mc");
      TH1F *d2=(TH1F*)d->Get("h_eff_da");

      const int nbin = 28;
      double eff_da[nbin], eff_mc[nbin], sig_da[nbin], sig_mc[nbin];
      double eff_dm[nbin], sig_dm[nbin];
      for(int i=0;i<nbin;i++)
      {
         eff_da[i]=d2->GetBinContent(i+1);
         sig_da[i]=d2->GetBinError(i+1);
         eff_mc[i]=d1->GetBinContent(i+1);
         sig_mc[i]=d1->GetBinError(i+1);
      } 

      TH1F *h_syserr=new TH1F("h_syserr",";E_{#gamma};eff_{da}/eff_{MC}",nbin,0,1.4);
      
      for(int i=0;i<nbin;i++)
      {
        if(eff_mc[i]>0) eff_dm[i]=eff_da[i]/eff_mc[i];
        else eff_dm[i]=999.0;
        //cal the stat. error 
        sig_dm[i]=(eff_dm[i])*sqrt((sig_da[i]/eff_da[i])*(sig_da[i]/eff_da[i])+(sig_mc[i]/eff_mc[i])*(sig_mc[i]/eff_mc[i])); 

        h_syserr->SetBinContent(i+1,eff_dm[i]);
        h_syserr->SetBinError(i+1,sig_dm[i]);
      }
      h_syserr->GetYaxis()->SetRangeUser(0.5,1.7);

      h_syserr->Write();


      //draw the histogram      
      TArrow *xy1= new TArrow(0.0,1.0,1.4,1.0,0);
      xy1->SetLineColor(2);
      xy1->SetLineWidth(3);      

      TLegend *leg1=new TLegend(0.5,0.5,0.7,0.7);
      leg1->AddEntry(d1,"MC","p");
      leg1->AddEntry(d2,"data","p");
      leg1->SetFillStyle(0);
      leg1->SetBorderSize(0);

      TCanvas *c1=new TCanvas("c1","c1",800,500);
      c1->Divide(2);
      c1->cd(1);
      d1->SetMarkerColor(2);
      d2->SetMarkerColor(1);
      d1->SetLineColor(2);
      d2->SetLineColor(1);
      d1->Draw("E");
      d2->Draw("sameE");
      leg1->Draw();


      c1->cd(2);
      h_syserr->Draw("E");
      xy1->Draw();
}
