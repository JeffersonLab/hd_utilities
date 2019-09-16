void plot_summary2DEff (TString tag="Chi2leq5_Epi1FVTagMassD_may6")
{
// File: plot_summary2DEff.C
    // Modeled after plot_compareEff.C
    // Reads in Data and MC efficiencies for a series of selected runs and outputs ratio.
//

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
    
    Double_t xmin=0.;
    Double_t xmax=2.0;
    Double_t ymin=0;
    Double_t ymax=1.5;
    Double_t zmin=0;
    Double_t zmax=1.2;
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1500,1400);
    c0->Divide(3,2);
    
    const Int_t nruns=6;
    TString runno[nruns] = {"030346_","030351_","030824_","030823_","030345_","030830_"};
    TString drawopt[nruns] = {"","psame","psame","psame","psame","psame"};
    TString legopt[nruns] = {"R30346 0/90/PERP", "R30351 AMO", "R30824 0/90 PERP"," 30823 AMO","30345 0/90 PARA","R30830 0/90 PARA"};
    Int_t markerno[nruns] = {25,20,25,20,24,24};
    Int_t markercolor[nruns] = {2,2,4,4,2,4};
        
    
    TString inData_name[nruns];
    TString inMC_name[nruns];
    
    TProfile *dP1_E2_eff_BCAL_Data[nruns];
    TProfile *dP1_E2_eff_FCAL_Data[nruns];
    TProfile *dP1_E2_eff_FCALBCAL_Data[nruns];
    
    TProfile *dP1_E2_eff_BCAL_MC[nruns];
    TProfile *dP1_E2_eff_FCAL_MC[nruns];
    TProfile *dP1_E2_eff_FCALBCAL_MC[nruns];
    
    TH1D *hdata1[nruns];
    TH1D *hdata2[nruns];
    TH1D *hdata3[nruns];
    
    TFile *inData[nruns];
    TFile *inMC[nruns];
    TLegend *leg5 = new TLegend(0.4,0.2,0.8,0.5);
    
    for (Int_t jj=0; jj< nruns; jj++) {
    
    inData_name[jj] = "DSelector_pi0pippim__B4_"+runno[jj]+tag+".root";
    inData[jj] = new TFile(inData_name[jj].Data(),"read");
    cout << " inData file=" << inData_name[jj].Data() << endl;
    
    inMC_name[jj] ="DSelector_pi0pippim__B4_gen_amp_G4_"+runno[jj]+tag+".root";
    inMC[jj] = new TFile(inMC_name[jj].Data(),"read");
    cout << " inMC file=" << inMC_name[jj].Data() << endl;
    
    dP1_E2_eff_BCAL_Data[jj] = (TProfile*)inData[jj]->Get("dP1_E2_eff_BCAL");
    dP1_E2_eff_FCAL_Data[jj] = (TProfile*)inData[jj]->Get("dP1_E2_eff_FCAL");
    dP1_E2_eff_FCALBCAL_Data[jj] = (TProfile*)inData[jj]->Get("dP1_E2_eff_FCALBCAL");
    
    dP1_E2_eff_BCAL_MC[jj] = (TProfile*)inMC[jj]->Get("dP1_E2_eff_BCAL");
    dP1_E2_eff_FCAL_MC[jj] = (TProfile*)inMC[jj]->Get("dP1_E2_eff_FCAL");
    dP1_E2_eff_FCALBCAL_MC[jj] = (TProfile*)inMC[jj]->Get("dP1_E2_eff_FCALBCAL");
    
    c0->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_BCAL_Data[jj]->SetTitle("BCAL(0.7<M_{3#pi}<3.0)");
    dP1_E2_eff_BCAL_Data[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_BCAL_Data[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_BCAL_Data[jj]->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_BCAL_Data[jj]->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_BCAL_Data[jj]->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_BCAL_Data[jj]->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_BCAL_Data[jj]->SetLineColor(2);
    dP1_E2_eff_BCAL_Data[jj]->SetMarkerColor(2);
    dP1_E2_eff_BCAL_Data[jj]->SetMarkerStyle(20);
    dP1_E2_eff_BCAL_Data[jj]->SetMarkerSize(0.5);
    dP1_E2_eff_BCAL_Data[jj]->Draw(drawopt[jj]);
    dP1_E2_eff_BCAL_MC[jj]->SetLineColor(4);
    dP1_E2_eff_BCAL_MC[jj]->SetMarkerColor(4);
    dP1_E2_eff_BCAL_MC[jj]->SetMarkerStyle(20);
    dP1_E2_eff_BCAL_MC[jj]->SetMarkerSize(0.5);
    dP1_E2_eff_BCAL_MC[jj]->Draw("psame");
        
    TLegend *leg2 = new TLegend(0.6,0.25,0.85,0.4);
    leg2->AddEntry(dP1_E2_eff_BCAL_Data[jj],"Data","p");
    leg2->AddEntry(dP1_E2_eff_BCAL_MC[jj],"MC","p");
    leg2->Draw();
    
    TString text="";
    // text.Form("Normalized to E=%.1f-%.1f GeV",Emin,Emax);
    TLatex *t1 = new TLatex (0.3,0.10,tag);
    t1->Draw();
    
    c0->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_FCAL_Data[jj]->SetTitle("FCAL (0.7<M_{3#pi}<0.85)");
    dP1_E2_eff_FCAL_Data[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_FCAL_Data[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_FCAL_Data[jj]->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCAL_Data[jj]->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCAL_Data[jj]->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_FCAL_Data[jj]->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_FCAL_Data[jj]->SetLineColor(2);
    dP1_E2_eff_FCAL_Data[jj]->SetMarkerColor(2);
    dP1_E2_eff_FCAL_Data[jj]->SetMarkerStyle(20);
    dP1_E2_eff_FCAL_Data[jj]->SetMarkerSize(0.5);
    dP1_E2_eff_FCAL_Data[jj]->Draw(drawopt[jj]);
    dP1_E2_eff_FCAL_MC[jj]->SetLineColor(4);
    dP1_E2_eff_FCAL_MC[jj]->SetMarkerColor(4);
    dP1_E2_eff_FCAL_MC[jj]->SetMarkerStyle(20);
    dP1_E2_eff_FCAL_MC[jj]->SetMarkerSize(0.5);
    dP1_E2_eff_FCAL_MC[jj]->Draw("psame");
    
    // data from Jon's analysis: data in e-mail 4/18/2019
    
    /*Double_t gr_m2_effic_Ebins_fx1001[14] = {
        0.1666667,
        0.4999997,
        0.8333327,
        1.166666,
        1.499999,
        1.833332,
        2.166665,
        2.499998,
        2.833331,
        3.166664,
        3.499997,
        3.83333,
        4.166663,
        4.499996};
    Double_t gr_m2_effic_Ebins_fy1001[14] = {
        0.682611,
        0.8232778,
        0.9118523,
        0.9389731,
        0.9575463,
        0.9586632,
        0.9598861,
        0.9604394,
        0.9625255,
        0.9591614,
        0.9509498,
        0.9426783,
        0.9306862,
        0.9407014};
    Double_t gr_m2_effic_Ebins_fex1001[14] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0};
    Double_t gr_m2_effic_Ebins_fey1001[14] = {
        0.01599996,
        0.008564864,
        0.007855693,
        0.007624827,
        0.007620991,
        0.008062109,
        0.008601666,
        0.009114182,
        0.01030098,
        0.01181837,
        0.0133631,
        0.0163474,
        0.01990572,
        0.0241965};
    TGraphErrors *gr_Jon_FCAL = new TGraphErrors(14,gr_m2_effic_Ebins_fx1001,gr_m2_effic_Ebins_fy1001,gr_m2_effic_Ebins_fex1001,gr_m2_effic_Ebins_fey1001);
    gr_Jon_FCAL->SetLineColor(1);
    gr_Jon_FCAL->SetMarkerColor(1);
    gr_Jon_FCAL->SetMarkerStyle(21);
    gr_Jon_FCAL->SetMarkerSize(0.7);
    gr_Jon_FCAL->Draw("psame");*/
    
    TLegend *leg3 = new TLegend(0.6,0.25,0.85,0.4);
    leg3->AddEntry(dP1_E2_eff_FCAL_Data[jj],"Data","p");
    leg3->AddEntry(dP1_E2_eff_FCAL_MC[jj],"MC","p");
    // leg3->AddEntry(gr_Jon_FCAL,"IU Eff","p");
    leg3->Draw();
    t1->DrawLatex(0.3,0.10,tag);
    
    zmin=0;
    ymin=0;
    // ymax=2.5;
    ymax=dP1_E2_eff_FCALBCAL_MC[jj]->GetMaximum()*1.2;
    
    c0->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_FCALBCAL_Data[jj]->SetTitle("FCALBCAL(0.7<M_{3#pi}<3.0)");
    dP1_E2_eff_FCALBCAL_Data[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_FCALBCAL_Data[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_FCALBCAL_Data[jj]->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCALBCAL_Data[jj]->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCALBCAL_Data[jj]->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_FCALBCAL_Data[jj]->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_FCALBCAL_Data[jj]->SetLineColor(2);
    dP1_E2_eff_FCALBCAL_Data[jj]->SetMarkerColor(2);
    dP1_E2_eff_FCALBCAL_Data[jj]->SetMarkerStyle(20);
    dP1_E2_eff_FCALBCAL_Data[jj]->SetMarkerSize(0.5);
    dP1_E2_eff_FCALBCAL_Data[jj]->Draw(drawopt[jj]);
    dP1_E2_eff_FCALBCAL_MC[jj]->SetLineColor(4);
    dP1_E2_eff_FCALBCAL_MC[jj]->SetMarkerColor(4);
    dP1_E2_eff_FCALBCAL_MC[jj]->SetMarkerStyle(20);
    dP1_E2_eff_FCALBCAL_MC[jj]->SetMarkerSize(0.5);
    dP1_E2_eff_FCALBCAL_MC[jj]->Draw("psame");
    
    TLegend *leg4 = new TLegend(0.6,0.25,0.85,0.4);
    leg4->AddEntry(dP1_E2_eff_FCALBCAL_Data[jj],"Data","p");
    leg4->AddEntry(dP1_E2_eff_FCALBCAL_MC[jj],"MC","p");
    leg4->Draw();
    t1->DrawLatex(0.3,0.10,tag);
    
    
    ymin=0;
    ymax=1.5;
    
    c0->cd(4);
    gPad->SetGridx();
    gPad->SetGridy();
    hdata1[jj] = dP1_E2_eff_BCAL_Data[jj]->ProjectionX();
    TH1D *hmc = dP1_E2_eff_BCAL_MC[jj]->ProjectionX();
    hdata1[jj]->Divide(hmc);
    hdata1[jj]->SetTitle("Data/MC BCAL");
    hdata1[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
    hdata1[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
    hdata1[jj]->GetXaxis()->SetTitleSize(0.05);
    hdata1[jj]->GetYaxis()->SetTitleSize(0.05);
    hdata1[jj]->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    hdata1[jj]->GetYaxis()->SetTitle("Ratio Data/MC");
    hdata1[jj]->SetLineColor(markercolor[jj]);
    hdata1[jj]->SetMarkerColor(markercolor[jj]);
    hdata1[jj]->SetMarkerStyle(markerno[jj]);
    hdata1[jj]->SetMarkerSize(0.5);
    // if (markercolor[jj] == 2) hdata1[jj]->Draw(drawopt[jj]);
    hdata1[jj]->Draw(drawopt[jj]);
    if (jj == nruns-1) leg5->Draw();
        
    c0->cd(5);
    gPad->SetGridx();
    gPad->SetGridy();
    hdata2[jj] = dP1_E2_eff_FCAL_Data[jj]->ProjectionX();
    hmc = dP1_E2_eff_FCAL_MC[jj]->ProjectionX();
    hdata2[jj]->Divide(hmc);
    hdata2[jj]->SetTitle("Data/MC FCAL");
    hdata2[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
    hdata2[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
    hdata2[jj]->GetXaxis()->SetTitleSize(0.05);
    hdata2[jj]->GetYaxis()->SetTitleSize(0.05);
    hdata2[jj]->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    hdata2[jj]->GetYaxis()->SetTitle("Ratio Data/MC");
    hdata2[jj]->SetLineColor(markercolor[jj]);
    hdata2[jj]->SetMarkerColor(markercolor[jj]);
    hdata2[jj]->SetMarkerStyle(markerno[jj]);
    hdata2[jj]->SetMarkerSize(0.5);
    hdata2[jj]->Draw(drawopt[jj]);
        
    leg5->AddEntry(hdata2[jj],legopt[jj],"p");
    if (jj == nruns-1) leg5->Draw();
        
        
    c0->cd(6);
    gPad->SetGridx();
    gPad->SetGridy();
    hdata3[jj] = dP1_E2_eff_FCALBCAL_Data[jj]->ProjectionX();
    hmc = dP1_E2_eff_FCALBCAL_MC[jj]->ProjectionX();
    hdata3[jj]->Divide(hmc);
    ymax=hdata3[jj]->GetMaximum()*1.2;
    hdata3[jj]->SetTitle("Data/MC FCALBCAL");
    hdata3[jj]->GetXaxis()->SetRangeUser(xmin,xmax);
    hdata3[jj]->GetYaxis()->SetRangeUser(ymin,ymax);
    hdata3[jj]->GetXaxis()->SetTitleSize(0.05);
    hdata3[jj]->GetYaxis()->SetTitleSize(0.05);
    hdata3[jj]->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    hdata3[jj]->GetYaxis()->SetTitle("Ratio Data/MC");
    hdata3[jj]->SetLineColor(markercolor[jj]);
    hdata3[jj]->SetMarkerColor(markercolor[jj]);
    hdata3[jj]->SetMarkerStyle(markerno[jj]);
    hdata3[jj]->SetMarkerSize(0.5);
    hdata3[jj]->Draw(drawopt[jj]);
    if (jj == nruns-1) leg5->Draw();

    }
    
    c0->SaveAs("plot_summary2DEff_"+tag+".pdf");
    
}
