void plot_compare_fcal (TString tag="jun19")
{
// File: plot_compare_fcal.C
    // Modeled after plot_compareEff.C
    // Reads in Data and MC efficiencies for fcal (using Costhe method) and also files from efficiencies computed by Jon using omega events.
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
        
    
    TString inData_name;
    TString inMC_name;

    TProfile *dP1_E2_eff_FCAL_Data;
    TProfile *dP1_E2_eff_FCAL_MC;
    
    TH1D *hdata1;
    
    TFile *inData;
    TFile *inMC;
    TLegend *leg5 = new TLegend(0.4,0.2,0.8,0.5);
    
    inData_name = "DSelector_pi0pippim__B4_030796-030900_"+tag+".root";
    inData = new TFile(inData_name.Data(),"read");
    cout << " inData file=" << inData_name.Data() << endl;
    
    inMC_name ="DSelector_pi0pippim__B4_gen_amp_G4_030796-030900_"+tag+".root";
    inMC = new TFile(inMC_name.Data(),"read");
    cout << " inMC file=" << inMC_name.Data() << endl;
    
    dP1_E2_eff_FCAL_Data = (TProfile*)inData->Get("dP1_E2_eff_FCAL");
    
    dP1_E2_eff_FCAL_MC = (TProfile*)inMC->Get("dP1_E2_eff_FCAL");

    TLatex *t1 = new TLatex (0.3,0.10,tag);
    // t1->Draw();
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1000,700);
    // c0->Divide(3,2);
    
    // c0->cd(1);
    Double_t fcal_scale = 0.9;                 // scale down to match high energy end of fcal data
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_FCAL_Data->SetTitle("FCAL (0.7<M_{3#pi}<0.85)");
    dP1_E2_eff_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_FCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_FCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_FCAL_Data->SetLineColor(4);
    dP1_E2_eff_FCAL_Data->SetMarkerColor(4);
    dP1_E2_eff_FCAL_Data->SetMarkerStyle(20);
    dP1_E2_eff_FCAL_Data->SetMarkerSize(0.7);
    dP1_E2_eff_FCAL_Data->Scale(fcal_scale);
    dP1_E2_eff_FCAL_Data->Draw();
    dP1_E2_eff_FCAL_MC->SetLineColor(4);
    dP1_E2_eff_FCAL_MC->SetMarkerColor(4);
    dP1_E2_eff_FCAL_MC->SetMarkerStyle(24);
    dP1_E2_eff_FCAL_MC->SetMarkerSize(0.7);
    dP1_E2_eff_FCAL_MC->Scale(fcal_scale);
    dP1_E2_eff_FCAL_MC->Draw("psame");
    
    // data from Jon's analysis: files in e-mail 6/21/19
    
    TFile *in2017Data = new TFile("zarling/for_elton/2017_effic.root","read");
    TGraphErrors *gr_m1_effic_Ebins = (TGraphErrors*)in2017Data->Get("gr_m1_effic_Ebins");
    gr_m1_effic_Ebins->SetLineColor(2);
    gr_m1_effic_Ebins->SetMarkerColor(2);
    gr_m1_effic_Ebins->SetMarkerStyle(24);
    gr_m1_effic_Ebins->SetMarkerSize(0.7);
    gr_m1_effic_Ebins->Draw("samep");
    
    TFile *inOmegaWeightGun = new TFile("zarling/for_elton/omegaweighted_gun_G4_effic.root","read");
    TGraphErrors *gr_gauscore_effic = (TGraphErrors*)inOmegaWeightGun->Get("gr_gauscore_effic");
    gr_gauscore_effic->SetLineColor(2);
    gr_gauscore_effic->SetMarkerColor(2);
    gr_gauscore_effic->SetMarkerStyle(20);
    gr_gauscore_effic->SetMarkerSize(0.7);
    gr_gauscore_effic->Draw("samep");
    
    TFile *inG4omegaMC = new TFile("zarling/for_elton/geant4_omegaMC_effic.root","read");
    TGraphErrors *grMC_m1_effic_Ebins = (TGraphErrors*)inG4omegaMC->Get("gr_m1_effic_Ebins");
    grMC_m1_effic_Ebins->SetLineColor(2);
    grMC_m1_effic_Ebins->SetMarkerColor(2);
    grMC_m1_effic_Ebins->SetMarkerStyle(21);
    grMC_m1_effic_Ebins->SetMarkerSize(0.7);
    grMC_m1_effic_Ebins->Draw("samep");
    
    TFile *inG4omegaMCtruth = new TFile("zarling/for_elton/geant4_omegaMC_effic_TruthCuts.root","read");
    TGraphErrors *grMC_m1_effic_Ebins_truth = (TGraphErrors*)inG4omegaMCtruth->Get("gr_m1_effic_Ebins");
    grMC_m1_effic_Ebins_truth->SetLineColor(2);
    grMC_m1_effic_Ebins_truth->SetMarkerColor(2);
    grMC_m1_effic_Ebins_truth->SetMarkerStyle(25);
    grMC_m1_effic_Ebins_truth->SetMarkerSize(0.7);
    grMC_m1_effic_Ebins_truth->Draw("samep");
    
    // data from Jon's analysis: data in e-mail 4/18/2019
    
    Double_t gr_m2_effic_Ebins_fx1001[14] = {
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
    // gr_Jon_FCAL->Draw("psame");
    
    TLegend *leg3 = new TLegend(0.5,0.15,0.85,0.4);
    TString text="";
    text.Form("Data Costhe Method (scale=%.2f)",fcal_scale);
    leg3->AddEntry(dP1_E2_eff_FCAL_Data,text.Data(),"p");
    text.Form("MC Costhe Method (scale=%.2f)",fcal_scale);
    leg3->AddEntry(dP1_E2_eff_FCAL_MC,text.Data(),"p");
    // leg3->AddEntry(gr_Jon_FCAL,"IU Eff","p");
    leg3->AddEntry(gr_m1_effic_Ebins,"IU Data 2017 #omega Tag-and-Probe","p");
    leg3->AddEntry(grMC_m1_effic_Ebins,"IU MC G4 #omega ","p");
    leg3->AddEntry(grMC_m1_effic_Ebins_truth,"IU MC G4 #omega (Truth)","p");
    leg3->AddEntry(gr_gauscore_effic,"IU MC G4 Gun #omega weight","p");
    leg3->Draw();
    t1->DrawLatex(0.3,0.10,tag);

    
        
    /*c0->cd(2);
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
    if (jj == nruns-1) leg5->Draw();*/
    
    c0->SaveAs("plot_compare_fcal_"+tag+".pdf");
    
}
