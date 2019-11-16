void compare_snap_voltages() {
  //
  // Compare two voltage settings for two snap files and plot differences.
  //

  gROOT->Reset();
gStyle->SetPalette(1,0);
// gStyle->SetOptStat(kFALSE);;
gStyle->SetOptStat(111111);
gStyle->SetOptFit(kFALSE);
// gStyle->SetOptFit(kTRUE);
// gStyle->SetOptFit(1111);
gStyle->SetPadRightMargin(0.15);
gStyle->SetPadLeftMargin(0.15);
gStyle->SetPadBottomMargin(0.15);
gStyle->SetFillColor(0);
//

   char string[256];
   char filename[80];
   char filename1[80];
   char filename2[80];
   // sprintf (filename1,"/gluex/data/burt/voltages/BCAL_BIAS/BCAL_BIAS_10_Mar_2015_12deg_14over.snap");;
   sprintf (filename1,"/gluonfs1/gluex/data/burt/voltages/FCAL_HV/ProductionHVFall2017.snap");
   sprintf (filename2,"/gluonfs1/gluex/data/burt/voltages/FCAL_HV/ProductionHVSpring2017.snap");
   // sprintf (filename2,"/gluex/data/burt/voltages/BCAL_BIAS/BCAL_BIAS_22_Jan_2016_7deg_14over.snap");
   // sprintf (filename2,"/gluex/data/burt/voltages/BCAL_BIAS/BCAL_BIAS_5_Mar_2015_18deg_14over.snap");
   
   Int_t i,j,jj;
   
   #define npts 2800;

  Float_t vset1[npts];
  Float_t vset2[npts];
  Float_t Dmax=0.01;    // Goal is to have all voltages set within 10 mV of optimal

  char Csnap[22];
  Int_t map[48]={10,11,12,13,14,15,16,17,18,19,1,
                 20,21,22,23,24,25,26,27,28,29,2,
                 30,31,32,33,34,35,36,37,38,39,3,
                 40,41,42,43,44,45,46,47,48,4,5,6,7,8,9};

  // input data from snap file into array

  ifstream in1,in2;
  in1.open(filename1);
  in2.open(filename2);
  printf ("\nFile 1 =%s\n",filename1);
  printf ("File 2 =%s\n\n",filename2);


  j=0;
  while (j<34) { // ignore header
    in1 >> string;
    // printf("H %s\n",string);
    in2 >> string;
    // printf("H %s\n",string);
    j++;
  }

  j=0;
  while (j<npts/8) { // one iteration of this loop per set of points Downstream

    jj=0;
    while (jj<4) { // inner index
      Int_t ndx=8*(map[j]-1)+jj+4;
      in1 >> Csnap >> i >> vset1[ndx];
      // printf("vset1: %d %s %d %.3f \n",ndx,Csnap,i,vset1[ndx]);
      in2 >> Csnap >> i >> vset2[ndx];
      // printf("vset2: %d %s %d %.3f \n",ndx,Csnap,i,vset2[ndx]);
      jj++;
    }
    j++;
  }

  j=0;
  while (j<npts/8) { // one iteration of this loop per set of points Upstream

    jj=0;
    while (jj<4) { // inner index
      Int_t ndx=8*(map[j]-1)+jj;
      in1 >> Csnap >> i >> vset1[ndx];
      // printf("vset1: %d %s %d %.3f \n",ndx,Csnap,i,vset1[ndx]);
      in2 >> Csnap >> i >> vset2[ndx];
      // printf("vset2: %d %s %d %.3f \n",ndx,Csnap,i,vset2[ndx]);
      jj++;
    }
    j++;
  }

  in1.close();
  in2.close();


   Double_t xmin=0;
   Double_t xmax=0.5;
   Double_t ymin=0;
   Double_t ymax=100;
   Int_t nbins=1000;

   TH1F *Hvdiff = new TH1F("Hvdiff","vset2 - vset1 (V)",nbins,-0.05,0.05);
   TH2F *Hvset2_vs_vset1 = new TH2F("Hvset2_vs_vset1","vset2 vs vset1",nbins,74,76,nbins,74,76);
   TH2F *Hvdiff_vs_ndx =  new TH2F("Hvdiff_vs_ndx","vdiff vs ndx",385,0,385,nbins,-0.05,0.05);

  // Fill histograms


  for (j=0;j<npts;j++) {
    Double_t vdiff = vset2[j] - vset1[j];
    Hvdiff->Fill(vdiff);
    Hvset2_vs_vset1->Fill(vset1[j],vset2[j]);
    Hvdiff_vs_ndx->Fill(j,vdiff);

    // printout outliers
    Int_t module = j/8 + 1;
    Int_t channel = j - 8*(module-1);
    sprintf (string,"upstream  ");
    if (channel > 3) sprintf (string,"downstream") ;

    if (fabs(vdiff) > Dmax) printf ("j=%d module=%d channel=%d side=%s vset1=%.3f vset2=%.3f vdiff=%.3f\n",j,module,channel,string,vset1[j],vset2[j],vdiff);
  }

   c1 = new TCanvas("c1","c1 Vop",200,10,700,500);
   c1->SetBorderMode(0);
   c1->SetFillColor(0);
   c1->Divide(2,2);

   c1->cd(1);
   c1_1->SetBorderMode(0);
   c1_1->SetFillColor(0);
   
   Hvdiff->SetTitle("");
   // Hvdiff->GetXaxis()->SetRangeUser(xmin,xmax);
   // Hvdiff->GetYaxis()->SetRangeUser(ymin,ymax);
   Hvdiff->GetXaxis()->SetLabelSize(0.05);
   Hvdiff->GetXaxis()->SetTitleSize(0.07);
   Hvdiff->GetYaxis()->SetLabelSize(0.05);
   Hvdiff->GetYaxis()->SetTitleSize(0.05);
   Hvdiff->GetYaxis()->SetTitleOffset(1.5);
   Hvdiff->GetXaxis()->SetTitle("vset2 - vset1 (V)");
   Hvdiff->GetXaxis()->SetNdivisions(505);
   Hvdiff->SetLineColor(4);
   Hvdiff->Draw();

   c1->cd(2);
   c1_2->SetBorderMode(0);
   c1_2->SetFillColor(0);
   
   Hvset2_vs_vset1->SetTitle("");
   // Hvset2_vs_vset1->GetXaxis()->SetRangeUser(xmin,xmax);
   // Hvset2_vs_vset1->GetYaxis()->SetRangeUser(ymin,ymax);
   Hvset2_vs_vset1->GetXaxis()->SetLabelSize(0.05);
   Hvset2_vs_vset1->GetXaxis()->SetTitleSize(0.07);
   Hvset2_vs_vset1->GetYaxis()->SetLabelSize(0.05);
   Hvset2_vs_vset1->GetYaxis()->SetTitleSize(0.05);
   Hvset2_vs_vset1->GetYaxis()->SetTitleOffset(1.5);
   Hvset2_vs_vset1->GetXaxis()->SetTitle("vset2 vs vset1 (V)");
   Hvset2_vs_vset1->GetXaxis()->SetNdivisions(505);
   Hvset2_vs_vset1->SetLineColor(4);
   Hvset2_vs_vset1->Draw("box");

   c1->cd(3);
   c1_3->SetBorderMode(0);
   c1_3->SetFillColor(0);
   
   Hvdiff_vs_ndx->SetTitle("");
   // Hvdiff_vs_ndx->GetXaxis()->SetRangeUser(xmin,xmax);
   // Hvdiff_vs_ndx->GetYaxis()->SetRangeUser(ymin,ymax);
   Hvdiff_vs_ndx->GetXaxis()->SetLabelSize(0.05);
   Hvdiff_vs_ndx->GetXaxis()->SetTitleSize(0.07);
   Hvdiff_vs_ndx->GetYaxis()->SetLabelSize(0.05);
   Hvdiff_vs_ndx->GetYaxis()->SetTitleSize(0.05);
   Hvdiff_vs_ndx->GetYaxis()->SetTitleOffset(1.5);
   Hvdiff_vs_ndx->GetXaxis()->SetTitle("vdiff(V) vs ndx");
   Hvdiff_vs_ndx->GetXaxis()->SetNdivisions(505);
   Hvdiff_vs_ndx->SetLineColor(4);
   Hvdiff_vs_ndx->Draw("box");

   c1->cd(4);
   c1_4->SetBorderMode(0);
   c1_4->SetFillColor(0);

   sprintf (string,"%s\n",filename1);
   printf("string=%s",string);
   t1 = new TLatex(0,0.8,string);
   t1->SetNDC();
   t1->SetTextSize(0.04);
   t1->Draw();
   sprintf (string,"%s\n",filename2);
   printf("string=%s",string);
   t1 = new TLatex(0,0.7,string);
   t1->SetNDC();
   t1->SetTextSize(0.04);
   t1->Draw();
 
   sprintf(filename,"/gluex/Subsystems/BCAL/plots/bias_compare/compare_snap_voltages.pdf");
   c1->SaveAs(filename);
  
}
