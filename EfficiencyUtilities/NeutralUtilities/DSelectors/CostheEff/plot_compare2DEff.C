void plot_compare2DEff (TString tag="Chi2le25")
{
// File: plot_compare2DEff.C
    // Modeled after plot_compareEff.C
    // Reads in 2D histograms (numerator and denominator, computes the E2 efficiency in bins of angle.
//

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
    
    TString RunPeriod="2017";
    // TString RunPeriod="2018_Spring";
    //TString RunPeriod="2018_Fall";
    // TString RunPeriod="2018_LowEnergy";
    // TString RefLabel="2017";
    TString RefLabel="MC";
    TString inData_name = "../DSelector_pi0pippim__B4_030808-030900_"+tag+".root";   // 2017
    // TString inData_name = "DSelector_pi0pippim__B4_040856-042559_"+tag+".root";      // 2018 Spring
    // TString inData_name = "DSelector_pi0pippim__B4_050685-051768_"+tag+".root";      // 2018 Fall
    // TString inData_name = "DSelector_pi0pippim__B4_051384-051457_"+tag+".root";      // 2018 Low Energy
    TFile *inData = new TFile(inData_name.Data(),"read");
    cout << " inData file=" << inData_name.Data() << endl;
    
    TString inMC_name ="../DSelector_pi0pippim__B4_gen_amp_G4_030808-030900_"+tag+".root";
    // TString inMC_name ="DSelector_pi0pippim__B4_gen_amp_030808-030900_"+tag+".root";
    // Use Data file from 2017 as reference
    // TString inMC_name ="../DSelector_pi0pippim__B4_030808-030900_"+tag+".root";
    // TString inMC2_name ="../DSelector_pi0pippim__B4_030808-030900_"+tag+".root";
    
    TString inMC2_name ="DSelector_pi0pippim__B4_gen_amp_030274-031057_"+tag+".root";
    // TString inMC2_name ="DSelector_pi0pippim__B4_gen_amp_040856-042559_"+tag+".root";
    // TString inMC_name ="DSelector_pi0pippim__B4_gen_amp_G4_030800_"+tag+".root";
    // TString inMC2_name ="DSelector_pi0pippim__B4_gen_amp_G4_030800_"+tag+".root";
    TFile *inMC = new TFile(inMC_name.Data(),"read");
    cout << " inMC file=" << inMC_name.Data() << endl;
    TFile *inMC2 = new TFile(inMC2_name.Data(),"read");
    cout << " inMC2 file=" << inMC2_name.Data() << endl;
    
    TH1F *dH1_E2_weff_BCAL_Data= (TH1F*)inData->Get("E2_weff_BCAL");
    TH1F *dH1_E2_weff_FCAL_Data= (TH1F*)inData->Get("E2_weff_FCAL");
    TH1F *dH1_E2_weff_FCALBCAL_Data= (TH1F*)inData->Get("E2_weff_FCALBCAL");
    TH2F *dH2_E2_weff_BCAL_Data= (TH2F*)inData->Get("dH2_E2_weff_BCAL");
    TH2F *dH2_E2_weff_FCAL_Data= (TH2F*)inData->Get("dH2_E2_weff_FCAL");
    TH2F *dH2_E2_weff_FCALBCAL_Data= (TH2F*)inData->Get("dH2_E2_weff_FCALBCAL");
    
    TH1F *dH1_E2_w1_BCAL_Data= (TH1F*)inData->Get("E2_w1_BCAL");
    TH1F *dH1_E2_w1_FCAL_Data= (TH1F*)inData->Get("E2_w1_FCAL");
    TH1F *dH1_E2_w1_FCALBCAL_Data= (TH1F*)inData->Get("E2_w1_FCALBCAL");
    TH2F *dH2_E2_w1_BCAL_Data= (TH2F*)inData->Get("dH2_E2_w1_BCAL");
    TH2F *dH2_E2_w1_FCAL_Data= (TH2F*)inData->Get("dH2_E2_w1_FCAL");
    TH2F *dH2_E2_w1_FCALBCAL_Data= (TH2F*)inData->Get("dH2_E2_w1_FCALBCAL");
    
    TH1F *dH1_E2_eff_BCAL_Data= (TH1F*)inData->Get("E2_eff_BCAL");
    TH1F *dH1_E2_eff_FCAL_Data= (TH1F*)inData->Get("E2_eff_FCAL");
    TH1F *dH1_E2_eff_FCALBCAL_Data= (TH1F*)inData->Get("E2_eff_FCALBCAL");
    TH2F *dH2_E2_eff_BCAL_Data= (TH2F*)inData->Get("dH2_E2_eff_BCAL");
    TH2F *dH2_E2_eff_FCAL_Data= (TH2F*)inData->Get("dH2_E2_eff_FCAL");
    TH2F *dH2_E2_eff_FCALBCAL_Data= (TH2F*)inData->Get("dH2_E2_eff_FCALBCAL");
    
    TProfile *dP1_E2_eff_BCAL_Data= (TProfile*)inData->Get("dP1_E2_eff_BCAL");
    TProfile *dP1_E2_eff_FCAL_Data= (TProfile*)inData->Get("dP1_E2_eff_FCAL");
    TProfile *dP1_E2_eff_FCALBCAL_Data= (TProfile*)inData->Get("dP1_E2_eff_FCALBCAL");
    
    Float_t xlo=dH1_E2_eff_FCAL_Data->GetXaxis()->GetBinLowEdge(1);        // assume data and MC historgrams have the same binning.
    Float_t xwidth=dH1_E2_eff_FCAL_Data->GetXaxis()->GetBinWidth(1);
    Int_t xbins=dH1_E2_eff_FCAL_Data->GetNbinsX();
    Float_t xhi= xlo + xwidth*xbins;
    
    TH1D *dH1_E2_eff_BCAL_Ratio = new TH1D("dH1_E2_eff_BCAL_Ratio", "Ratio of Data/MC", xbins, xlo, xhi);
    dH1_E2_eff_BCAL_Ratio->Sumw2();
    
    TProfile2D *dP2_E2_eff_BCAL_Data= (TProfile2D*)inData->Get("dP2_E2_eff_BCAL");
    TProfile2D *dP2_E2_eff_FCAL_Data= (TProfile2D*)inData->Get("dP2_E2_eff_FCAL");
    TProfile2D *dP2_E2_eff_FCALBCAL_Data= (TProfile2D*)inData->Get("dP2_E2_eff_FCALBCAL");
    
    TH1F *dH1_E2_weff_BCAL_MC= (TH1F*)inMC->Get("E2_weff_BCAL");
    TH1F *dH1_E2_weff_FCAL_MC= (TH1F*)inMC2->Get("E2_weff_FCAL");
    TH1F *dH1_E2_weff_FCALBCAL_MC= (TH1F*)inMC->Get("E2_weff_FCALBCAL");
    TH2F *dH2_E2_weff_BCAL_MC= (TH2F*)inMC->Get("dH2_E2_weff_BCAL");
    TH2F *dH2_E2_weff_FCAL_MC= (TH2F*)inMC2->Get("dH2_E2_weff_FCAL");
    TH2F *dH2_E2_weff_FCALBCAL_MC= (TH2F*)inMC->Get("dH2_E2_weff_FCALBCAL");
    
    TH1F *dH1_E2_w1_BCAL_MC= (TH1F*)inMC->Get("E2_w1_BCAL");
    TH1F *dH1_E2_w1_FCAL_MC= (TH1F*)inMC2->Get("E2_w1_FCAL");
    TH1F *dH1_E2_w1_FCALBCAL_MC= (TH1F*)inMC->Get("E2_w1_FCALBCAL");
    TH2F *dH2_E2_w1_BCAL_MC= (TH2F*)inMC->Get("dH2_E2_w1_BCAL");
    TH2F *dH2_E2_w1_FCAL_MC= (TH2F*)inMC2->Get("dH2_E2_w1_FCAL");
    TH2F *dH2_E2_w1_FCALBCAL_MC= (TH2F*)inMC->Get("dH2_E2_w1_FCALBCAL");
    
    TH1F *dH1_E2_eff_BCAL_MC = (TH1F*)inMC->Get("E2_eff_BCAL");
    TH1F *dH1_E2_eff_FCAL_MC = (TH1F*)inMC2->Get("E2_eff_FCAL");
    TH1F *dH1_E2_eff_FCALBCAL_MC= (TH1F*)inMC->Get("E2_eff_FCALBCAL");
    TH2F *dH2_E2_eff_BCAL_MC = (TH2F*)inMC->Get("dH2_E2_eff_BCAL");
    TH2F *dH2_E2_eff_FCAL_MC = (TH2F*)inMC2->Get("dH2_E2_eff_FCAL");
    TH2F *dH2_E2_eff_FCALBCAL_MC= (TH2F*)inMC->Get("dH2_E2_eff_FCALBCAL");
    
    TProfile *dP1_E2_eff_BCAL_MC= (TProfile*)inMC->Get("dP1_E2_eff_BCAL");
    TProfile *dP1_E2_eff_FCAL_MC= (TProfile*)inMC2->Get("dP1_E2_eff_FCAL");
    TProfile *dP1_E2_eff_FCALBCAL_MC= (TProfile*)inMC->Get("dP1_E2_eff_FCALBCAL");
    TProfile2D *dP2_E2_eff_BCAL_MC= (TProfile2D*)inMC->Get("dP2_E2_eff_BCAL");
    TProfile2D *dP2_E2_eff_FCAL_MC= (TProfile2D*)inMC2->Get("dP2_E2_eff_FCAL");
    TProfile2D *dP2_E2_eff_FCALBCAL_MC= (TProfile2D*)inMC->Get("dP2_E2_eff_FCALBCAL");
    
    dH1_E2_eff_BCAL_Data->Reset();
    dH1_E2_eff_FCAL_Data->Reset();
    dH1_E2_eff_FCALBCAL_Data->Reset();
    
    dH2_E2_eff_BCAL_Data->Reset();
    dH2_E2_eff_FCAL_Data->Reset();
    dH2_E2_eff_FCALBCAL_Data->Reset();
    
    dH1_E2_eff_BCAL_MC->Reset();
    dH1_E2_eff_FCAL_MC->Reset();
    dH1_E2_eff_FCALBCAL_MC->Reset();
    
    dH2_E2_eff_BCAL_MC->Reset();
    dH2_E2_eff_FCAL_MC->Reset();
    dH2_E2_eff_FCALBCAL_MC->Reset();
    
    // now divide histograms to produce an efficiency. Must do this if input histograms have been added together (i.e. efficiencies will not be correct).
    dH1_E2_eff_FCAL_Data->Add(dH1_E2_weff_FCAL_Data);
    dH1_E2_eff_FCAL_Data->Divide(dH1_E2_w1_FCAL_Data);
    dH1_E2_eff_BCAL_Data->Add(dH1_E2_weff_BCAL_Data);
    dH1_E2_eff_BCAL_Data->Divide(dH1_E2_w1_BCAL_Data);
    dH1_E2_eff_FCALBCAL_Data->Add(dH1_E2_weff_FCALBCAL_Data);
    dH1_E2_eff_FCALBCAL_Data->Divide(dH1_E2_w1_FCALBCAL_Data);
    
    dH1_E2_eff_FCAL_MC->Add(dH1_E2_weff_FCAL_MC);
    dH1_E2_eff_FCAL_MC->Divide(dH1_E2_w1_FCAL_MC);
    dH1_E2_eff_BCAL_MC->Add(dH1_E2_weff_BCAL_MC);
    dH1_E2_eff_BCAL_MC->Divide(dH1_E2_w1_BCAL_MC);
    dH1_E2_eff_FCALBCAL_MC->Add(dH1_E2_weff_FCALBCAL_MC);
    dH1_E2_eff_FCALBCAL_MC->Divide(dH1_E2_w1_FCALBCAL_MC);
    
    
    xlo=dH1_E2_eff_FCAL_Data->GetXaxis()->GetBinLowEdge(1);        // assume data and MC historgrams have the same binning.
    xwidth=dH1_E2_eff_FCAL_Data->GetXaxis()->GetBinWidth(1);
    xbins=dH1_E2_eff_FCAL_Data->GetNbinsX();
    xhi= xlo + xwidth*xbins;
    
    for (Int_t j=0; j<xbins; j++) {
        Double_t Entries_Data=dH1_E2_w1_BCAL_Data->GetBinContent(j+1);
        Double_t Eff_Data=dH1_E2_eff_BCAL_Data->GetBinContent(j+1);
        Double_t Eff_Data_Err=dH1_E2_eff_BCAL_Data->GetBinError(j+1);
        Double_t error = Entries_Data > 0? error = Eff_Data/ sqrt(Entries_Data): 0;
        cout << " j=" << j << " Entries_Data=" << Entries_Data << " Eff_Data=" << Eff_Data << " Eff_Data_Err=" << Eff_Data_Err << " error=" << error << endl;
        dH1_E2_eff_BCAL_Data->SetBinError(j+1,error);
        
        Double_t Entries_MC=dH1_E2_w1_BCAL_MC->GetBinContent(j+1);
        Double_t Eff_MC=dH1_E2_eff_BCAL_MC->GetBinContent(j+1);
        Double_t Eff_MC_Err=dH1_E2_eff_BCAL_MC->GetBinError(j+1);
        error = Entries_MC > 0? Eff_MC/ sqrt(Entries_MC): 0;
        cout << " j=" << j << " Entries_MC=" << Entries_MC << " Eff_MC=" << Eff_MC << " Eff_MC_Err=" << Eff_MC_Err << " error=" << error << endl;
        dH1_E2_eff_BCAL_MC->SetBinError(j+1,error);
    }
    
    
    dH2_E2_eff_FCAL_Data->Add(dH2_E2_weff_FCAL_Data);
    dH2_E2_eff_FCAL_Data->Divide(dH2_E2_w1_FCAL_Data);
    dH2_E2_eff_BCAL_Data->Add(dH2_E2_weff_BCAL_Data);
    dH2_E2_eff_BCAL_Data->Divide(dH2_E2_w1_BCAL_Data);
    dH2_E2_eff_FCALBCAL_Data->Add(dH2_E2_weff_FCALBCAL_Data);
    dH2_E2_eff_FCALBCAL_Data->Divide(dH2_E2_w1_FCALBCAL_Data);
    
    dH2_E2_eff_FCAL_MC->Add(dH2_E2_weff_FCAL_MC);
    dH2_E2_eff_FCAL_MC->Divide(dH2_E2_w1_FCAL_MC);
    dH2_E2_eff_BCAL_MC->Add(dH2_E2_weff_BCAL_MC);
    dH2_E2_eff_BCAL_MC->Divide(dH2_E2_w1_BCAL_MC);
    dH2_E2_eff_FCALBCAL_MC->Add(dH2_E2_weff_FCALBCAL_MC);
    dH2_E2_eff_FCALBCAL_MC->Divide(dH2_E2_w1_FCALBCAL_MC);
    
    Double_t xmin=0.;
    Double_t xmax=2.0;
    // Double_t ymin=0.;
    // Double_t ymax=1.5;
    Double_t ymin=0.4;
    Double_t ymax=1.4;
    Double_t zmin=0;
    Double_t zmax=1.2;
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1500,1400);
    c0->Divide(3,2);
    
    c0->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_BCAL_Data->SetTitle("BCAL(0.7<M_{3#pi}<3.0)");
    dP1_E2_eff_BCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_BCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_BCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_BCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_BCAL_Data->SetLineColor(2);
    dP1_E2_eff_BCAL_Data->SetMarkerColor(2);
    dP1_E2_eff_BCAL_Data->SetMarkerStyle(20);
    dP1_E2_eff_BCAL_Data->SetMarkerSize(0.5);
    dP1_E2_eff_BCAL_Data->Draw();
    dP1_E2_eff_BCAL_MC->SetLineColor(4);
    dP1_E2_eff_BCAL_MC->SetMarkerColor(4);
    dP1_E2_eff_BCAL_MC->SetMarkerStyle(20);
    dP1_E2_eff_BCAL_MC->SetMarkerSize(0.5);
    dP1_E2_eff_BCAL_MC->Draw("psame");
    
    TLegend *leg2 = new TLegend(0.6,0.25,0.85,0.4);
    leg2->AddEntry(dP1_E2_eff_BCAL_Data,RunPeriod,"p");
    leg2->AddEntry(dP1_E2_eff_BCAL_MC,RefLabel,"p");
    leg2->Draw();
    
    TString text="";
    // text.Form("Normalized to E=%.1f-%.1f GeV",Emin,Emax);
    TLatex *t1 = new TLatex (0.3,0.10,tag);
    t1->Draw();
    
    c0->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_FCAL_Data->SetTitle("FCAL (0.7<M_{3#pi}<0.85)");
    dP1_E2_eff_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_FCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_FCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_FCAL_Data->SetLineColor(2);
    dP1_E2_eff_FCAL_Data->SetMarkerColor(2);
    dP1_E2_eff_FCAL_Data->SetMarkerStyle(20);
    dP1_E2_eff_FCAL_Data->SetMarkerSize(0.5);
    dP1_E2_eff_FCAL_Data->Draw();
    dP1_E2_eff_FCAL_MC->SetLineColor(4);
    dP1_E2_eff_FCAL_MC->SetMarkerColor(4);
    dP1_E2_eff_FCAL_MC->SetMarkerStyle(20);
    dP1_E2_eff_FCAL_MC->SetMarkerSize(0.5);
    dP1_E2_eff_FCAL_MC->Draw("psame");
    
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
    gr_Jon_FCAL->Draw("psame");
    
    TLegend *leg3 = new TLegend(0.6,0.25,0.85,0.4);
    leg3->AddEntry(dP1_E2_eff_FCAL_Data,RunPeriod,"p");
    leg3->AddEntry(dP1_E2_eff_FCAL_MC,RefLabel,"p");
    leg3->AddEntry(gr_Jon_FCAL,"IU Eff","p");
    leg3->Draw();
    t1->DrawLatex(0.3,0.10,tag);
    
    zmin=0;
    ymin=0;
    // ymax=2.5;
    ymax=dP1_E2_eff_FCALBCAL_MC->GetMaximum()*1.2;
    
    c0->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    dP1_E2_eff_FCALBCAL_Data->SetTitle("FCALBCAL(0.7<M_{3#pi}<3.0)");
    dP1_E2_eff_FCALBCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    dP1_E2_eff_FCALBCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    dP1_E2_eff_FCALBCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCALBCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dP1_E2_eff_FCALBCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dP1_E2_eff_FCALBCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    dP1_E2_eff_FCALBCAL_Data->SetLineColor(2);
    dP1_E2_eff_FCALBCAL_Data->SetMarkerColor(2);
    dP1_E2_eff_FCALBCAL_Data->SetMarkerStyle(20);
    dP1_E2_eff_FCALBCAL_Data->SetMarkerSize(0.5);
    dP1_E2_eff_FCALBCAL_Data->Draw();
    dP1_E2_eff_FCALBCAL_MC->SetLineColor(4);
    dP1_E2_eff_FCALBCAL_MC->SetMarkerColor(4);
    dP1_E2_eff_FCALBCAL_MC->SetMarkerStyle(20);
    dP1_E2_eff_FCALBCAL_MC->SetMarkerSize(0.5);
    dP1_E2_eff_FCALBCAL_MC->Draw("psame");
    
    TLegend *leg4 = new TLegend(0.6,0.25,0.85,0.4);
    leg4->AddEntry(dP1_E2_eff_FCALBCAL_Data,RunPeriod,"p");
    leg4->AddEntry(dP1_E2_eff_FCALBCAL_MC,RefLabel,"p");
    leg4->Draw();
    t1->DrawLatex(0.3,0.10,tag);
    
    
    ymin=0.4;
    ymax=1.4;
    
    // save histograms for NIM paper
    TFile *out = new TFile("CostheEff_"+tag+"_NIM.root","recreate");
    
    c0->cd(4);
    gPad->SetGridx();
    gPad->SetGridy();
    TH1D *hdata = dP1_E2_eff_BCAL_Data->ProjectionX();
    TH1D *hmc = dP1_E2_eff_BCAL_MC->ProjectionX();
    hdata->Divide(hmc);
    hdata->SetTitle(RunPeriod+"/"+RefLabel+" BCAL");
    hdata->GetXaxis()->SetRangeUser(xmin,xmax);
    hdata->GetYaxis()->SetRangeUser(ymin,ymax);
    hdata->GetXaxis()->SetTitleSize(0.05);
    hdata->GetYaxis()->SetTitleSize(0.05);
    hdata->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    hdata->GetYaxis()->SetTitle("Ratio "+RunPeriod+"/"+RefLabel);
    hdata->SetLineColor(2);
    hdata->SetMarkerColor(2);
    hdata->SetMarkerStyle(20);
    hdata->SetMarkerSize(0.5);
    hdata->Write();
    hdata->Draw();
    
    c0->cd(5);
    gPad->SetGridx();
    gPad->SetGridy();
    hdata = dP1_E2_eff_FCAL_Data->ProjectionX();
    hmc = dP1_E2_eff_FCAL_MC->ProjectionX();
    hdata->Divide(hmc);
    hdata->SetTitle(RunPeriod+"/"+RefLabel+" FCAL");
    hdata->GetXaxis()->SetRangeUser(xmin,xmax);
    hdata->GetYaxis()->SetRangeUser(ymin,ymax);
    hdata->GetXaxis()->SetTitleSize(0.05);
    hdata->GetYaxis()->SetTitleSize(0.05);
    hdata->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    hdata->GetYaxis()->SetTitle("Ratio "+RunPeriod+"/"+RefLabel);
    hdata->SetLineColor(2);
    hdata->SetMarkerColor(2);
    hdata->SetMarkerStyle(20);
    hdata->SetMarkerSize(0.5);
    hdata->Write();
    hdata->Draw();
    
    c0->cd(6);
    gPad->SetGridx();
    gPad->SetGridy();
    hdata = dP1_E2_eff_FCALBCAL_Data->ProjectionX();
    hmc = dP1_E2_eff_FCALBCAL_MC->ProjectionX();
    hdata->Divide(hmc);
    ymax=hdata->GetMaximum()*1.2;
    hdata->SetTitle(RunPeriod+"/"+RefLabel+" FCALBCAL");
    hdata->GetXaxis()->SetRangeUser(xmin,xmax);
    hdata->GetYaxis()->SetRangeUser(ymin,ymax);
    hdata->GetXaxis()->SetTitleSize(0.05);
    hdata->GetYaxis()->SetTitleSize(0.05);
    hdata->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    hdata->GetYaxis()->SetTitle("Ratio Data/Reference");
    hdata->SetLineColor(2);
    hdata->SetMarkerColor(2);
    hdata->SetMarkerStyle(20);
    hdata->SetMarkerSize(0.5);
    hdata->Write();
    hdata->Draw();
    
    out->Close();
    
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,800,800);
    c1->Divide(2,3);
    
    zmin=0;
    zmax=1.2;
    
    c1->cd(1);
    dH2_E2_eff_FCAL_Data->SetTitle(RunPeriod+": Rel Eff FCAL");
    dH2_E2_eff_FCAL_Data->GetZaxis()->SetRangeUser(zmin,zmax);
    dH2_E2_eff_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dH2_E2_eff_FCAL_Data->GetYaxis()->SetTitle("Angle Low (Deg)");
    dH2_E2_eff_FCAL_Data->Draw("colz");
    
    c1->cd(2);
    dH2_E2_eff_FCAL_MC->SetTitle(RefLabel+": Rel Eff FCAL");
    dH2_E2_eff_FCAL_MC->GetZaxis()->SetRangeUser(zmin,zmax);
    dH2_E2_eff_FCAL_MC->GetXaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCAL_MC->GetYaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCAL_MC->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dH2_E2_eff_FCAL_MC->GetYaxis()->SetTitle("Angle Low (Deg)");
    dH2_E2_eff_FCAL_MC->Draw("colz");
    
    c1->cd(3);
    dH2_E2_eff_BCAL_Data->SetTitle(RunPeriod+": Rel Eff BCAL");
    dH2_E2_eff_BCAL_Data->GetZaxis()->SetRangeUser(zmin,zmax);
    dH2_E2_eff_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dH2_E2_eff_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dH2_E2_eff_BCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dH2_E2_eff_BCAL_Data->GetYaxis()->SetTitle("Angle Low (Deg)");
    dH2_E2_eff_BCAL_Data->Draw("colz");
    
    c1->cd(4);
    dH2_E2_eff_BCAL_MC->SetTitle(RefLabel+": Rel Eff BCAL");
    dH2_E2_eff_BCAL_MC->GetZaxis()->SetRangeUser(zmin,zmax);
    dH2_E2_eff_BCAL_MC->GetXaxis()->SetTitleSize(0.05);
    dH2_E2_eff_BCAL_MC->GetYaxis()->SetTitleSize(0.05);
    dH2_E2_eff_BCAL_MC->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dH2_E2_eff_BCAL_MC->GetYaxis()->SetTitle("Angle Low (Deg)");
    dH2_E2_eff_BCAL_MC->Draw("colz");
    
    
    zmin=0;
    zmax=dH2_E2_eff_FCALBCAL_Data->GetMaximum()*1.2;

    c1->cd(5);
    dH2_E2_eff_FCALBCAL_Data->SetTitle(RunPeriod+": Rel Eff FCALBCAL");
    dH2_E2_eff_FCALBCAL_Data->GetZaxis()->SetRangeUser(zmin,zmax);
    dH2_E2_eff_FCALBCAL_Data->GetXaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCALBCAL_Data->GetYaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCALBCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dH2_E2_eff_FCALBCAL_Data->GetYaxis()->SetTitle("Angle Low (Deg)");
    dH2_E2_eff_FCALBCAL_Data->Draw("colz");
    
    c1->cd(6);
    dH2_E2_eff_FCALBCAL_MC->SetTitle(RefLabel+": Rel Eff FCALBCAL");
    dH2_E2_eff_FCALBCAL_MC->GetZaxis()->SetRangeUser(zmin,zmax);
    dH2_E2_eff_FCALBCAL_MC->GetXaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCALBCAL_MC->GetYaxis()->SetTitleSize(0.05);
    dH2_E2_eff_FCALBCAL_MC->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    dH2_E2_eff_FCALBCAL_MC->GetYaxis()->SetTitle("Angle Low (Deg)");
    dH2_E2_eff_FCALBCAL_MC->Draw("colz");
    
    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1000,1000);
    c2->Divide(4,5);
    
    xlo=dH2_E2_eff_FCAL_Data->GetXaxis()->GetBinLowEdge(1);
    xwidth=dH2_E2_eff_FCAL_Data->GetXaxis()->GetBinWidth(1);
    xbins=dH2_E2_eff_FCAL_Data->GetNbinsX();
    xhi= xlo + xwidth*xbins;
    
    Float_t ylo=dH2_E2_eff_FCAL_Data->GetYaxis()->GetBinLowEdge(1);
    Float_t ywidth=dH2_E2_eff_FCAL_Data->GetYaxis()->GetBinWidth(1);
    Int_t ybins=dH2_E2_eff_FCAL_Data->GetNbinsY();
    Int_t yhi= ylo + ywidth*ybins;
    
    const int nfcalprof=ybins;
    
    // Move away from Histograms to TProfile histograms
    /*TH1D *FCALDataprof[nfcalprof];
    
    // cout << " c2 nfcalprof=" << nfcalprof << endl;

    ymin=0.5;
    ymax=1.5;
    
    TLatex *t2 = new TLatex(0.2,0.8,"");
    t2->SetNDC();
    TString t2txt;
    
    for (Int_t j=0; j<nfcalprof; j++) {
        Double_t theta = ylo + (j+0.5)*ywidth;
        TString name = "FCALDataprof"+TString::Itoa(j,10);
        FCALDataprof[j] = new TH1D (name,name,xbins,xlo,xhi);
        FCALDataprof[j]->Sumw2();
        FCALDataprof[j] = dH2_E2_eff_FCAL_Data->ProjectionX(name,j+1,j+1);
        c2->cd(j+1);
        FCALDataprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
        FCALDataprof[j]->SetMarkerColor(2);
        FCALDataprof[j]->SetLineColor(2);
        FCALDataprof[j]->Draw();
        t2txt.Form("Theta=%.2f",theta);
        t2->DrawLatex(0.2,0.8,t2txt);
        t2->SetNDC();
    }*/
    
     TProfile *FCALDataprof[nfcalprof];
     
     // cout << " c2 nfcalprof=" << nfcalprof << endl;
     
     ymin=0.5;
     ymax=1.5;
     
     TLatex *t2 = new TLatex(0.2,0.8,"");
     t2->SetNDC();
     TString t2txt;
     
     for (Int_t j=0; j<nfcalprof; j++) {
     	Double_t theta = ylo + (j+0.5)*ywidth;
     	TString name = "FCALDataprof"+TString::Itoa(j,10);
     	FCALDataprof[j] = new TProfile (name,name,xbins,xlo,xhi);
     	FCALDataprof[j] = dP2_E2_eff_FCAL_Data->ProfileX(name,j+1,j+1);
     	c2->cd(j+1);
     	FCALDataprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
     	FCALDataprof[j]->SetMarkerColor(2);
     	FCALDataprof[j]->SetLineColor(2);
         FCALDataprof[j]->SetTitle("FCAL (0.7<M_{3#pi}<0.85 GeV)");
        FCALDataprof[j]->Draw();
        t2txt.Form("Theta=%.2f",theta);
     	t2->DrawLatex(0.2,0.8,t2txt);
     	t2->SetNDC();
     }
    
    
    // TCanvas *c3 = new TCanvas(" c3", " c3",200,10,1000,1000);
    // c3->Divide(4,5);
    
    TProfile *FCALMCprof[nfcalprof];
    
    for (Int_t j=0; j<nfcalprof; j++) {
        Double_t theta = ylo + (j+0.5)*ywidth;
        TString name = "FCALMCprof"+TString::Itoa(j,10);
        FCALMCprof[j] = new TProfile (name,name,xbins,xlo,xhi);
        FCALMCprof[j]->Sumw2();
        FCALMCprof[j] = dP2_E2_eff_FCAL_MC->ProfileX(name,j+1,j+1);
        // c3->cd(j+1);
        c2->cd(j+1);
        FCALMCprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
        FCALMCprof[j]->SetMarkerColor(4);
        FCALMCprof[j]->SetLineColor(4);
        FCALMCprof[j]->Draw("same");
        t2txt.Form("Theta=%.2f",theta);
        t2->DrawLatex(0.2,0.8,t2txt);
        t2->SetNDC();
    }
    
    TCanvas *c4 = new TCanvas("c4", "c4",200,10,1000,1000);
    c4->Divide(4,5);
    
    xlo=dH2_E2_eff_BCAL_Data->GetXaxis()->GetBinLowEdge(1);
    xwidth=dH2_E2_eff_BCAL_Data->GetXaxis()->GetBinWidth(1);
    xbins=dH2_E2_eff_BCAL_Data->GetNbinsX();
    xhi= xlo + xwidth*xbins;
    
    ylo=dH2_E2_eff_BCAL_Data->GetYaxis()->GetBinLowEdge(1);
    ywidth=dH2_E2_eff_BCAL_Data->GetYaxis()->GetBinWidth(1);
    ybins=dH2_E2_eff_BCAL_Data->GetNbinsY();
    yhi= ylo + ywidth*ybins;
    
    const int nBCALprof=ybins/2;                // only output the first half of the BCAL bins.
    TProfile *BCALDataprof[nBCALprof];
    
    // cout << " c2 nBCALprof=" << nBCALprof << endl;
    ymin=0.5;
    ymax=1.5;
    
    for (Int_t j=0; j<nBCALprof; j++) {
        Double_t theta = ylo + (j+0.5)*ywidth;
        TString name = "BCALDataprof"+TString::Itoa(j,10);
        BCALDataprof[j] = new TProfile (name,name,xbins,xlo,xhi);
        BCALDataprof[j]->Sumw2();
        BCALDataprof[j] = dP2_E2_eff_BCAL_Data->ProfileX(name,j+1,j+1);
        c4->cd(j+1);
        BCALDataprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
        BCALDataprof[j]->SetMarkerColor(2);
        BCALDataprof[j]->SetLineColor(2);
        BCALDataprof[j]->SetTitle("BCAL (0.7<M_{3#pi}<3.0 GeV)");
        BCALDataprof[j]->Draw();
        t2txt.Form("Theta=%.2f",theta);
        t2->DrawLatex(0.2,0.8,t2txt);
        t2->SetNDC();
    }
    
    // TCanvas *c5 = new TCanvas(" c5", " c5",200,10,1000,1000);
    // c5->Divide(4,5);
    
    TProfile *BCALMCprof[nBCALprof];
    
    for (Int_t j=0; j<nBCALprof; j++) {
        Double_t theta = ylo + (j+0.5)*ywidth;
        TString name = "BCALMCprof"+TString::Itoa(j,10);
        BCALMCprof[j] = new TProfile (name,name,xbins,xlo,xhi);
        BCALMCprof[j]->Sumw2();
        BCALMCprof[j] = dP2_E2_eff_BCAL_MC->ProfileX(name,j+1,j+1);
        // c5->cd(j+1);
        c4->cd(j+1);
        BCALMCprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
        BCALMCprof[j]->SetMarkerColor(4);
        BCALMCprof[j]->SetLineColor(4);
        BCALMCprof[j]->Draw("same");
        t2txt.Form("Theta=%.2f",theta);
        t2->DrawLatex(0.2,0.8,t2txt);
        t2->SetNDC();
    }
    
    
    TCanvas *c6 = new TCanvas("c6", "c6",200,10,1000,1000);
    c6->Divide(4,5);
    
    xlo=dH2_E2_eff_FCALBCAL_Data->GetXaxis()->GetBinLowEdge(1);
    xwidth=dH2_E2_eff_FCALBCAL_Data->GetXaxis()->GetBinWidth(1);
    xbins=dH2_E2_eff_FCALBCAL_Data->GetNbinsX();
    xhi= xlo + xwidth*xbins;
    
    ylo=dH2_E2_eff_FCALBCAL_Data->GetYaxis()->GetBinLowEdge(1);
    ywidth=dH2_E2_eff_FCALBCAL_Data->GetYaxis()->GetBinWidth(1);
    ybins=dH2_E2_eff_FCALBCAL_Data->GetNbinsY();
    yhi= ylo + ywidth*ybins;
    
    const int nFCALBCALprof=ybins;
    TProfile *FCALBCALDataprof[nFCALBCALprof];
    
    // cout << " c2 nFCALBCALprof=" << nFCALBCALprof << endl;

    ymin=0.5;
    ymax=3.0;
    
    for (Int_t j=0; j<nFCALBCALprof; j++) {
        Double_t theta = ylo + (j+0.5)*ywidth;
        TString name = "FCALBCALDataprof"+TString::Itoa(j,10);
        FCALBCALDataprof[j] = new TProfile (name,name,xbins,xlo,xhi);
        FCALBCALDataprof[j]->Sumw2();
        FCALBCALDataprof[j] = dP2_E2_eff_FCALBCAL_Data->ProfileX(name,j+1,j+1);
        c6->cd(j+1);
        FCALBCALDataprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
        FCALBCALDataprof[j]->SetMarkerColor(2);
        FCALBCALDataprof[j]->SetLineColor(2);
        FCALBCALDataprof[j]->SetTitle("FCAL/BCAL (0.7<M_{3#pi}<3.0 GeV)");
        FCALBCALDataprof[j]->Draw();
        t2txt.Form("Theta=%.2f",theta);
        t2->DrawLatex(0.2,0.8,t2txt);
        t2->SetNDC();
    }
    
    // TCanvas *c7 = new TCanvas(" c7", " c7",200,10,1000,1000);
    // c7->Divide(4,5);
    
    TProfile *FCALBCALMCprof[nFCALBCALprof];
    
    for (Int_t j=0; j<nFCALBCALprof; j++) {
        Double_t theta = ylo + (j+0.5)*ywidth;
        TString name = "FCALBCALMCprof"+TString::Itoa(j,10);
        FCALBCALMCprof[j] = new TProfile (name,name,xbins,xlo,xhi);
        FCALBCALMCprof[j]->Sumw2();
        FCALBCALMCprof[j] = dP2_E2_eff_FCALBCAL_MC->ProfileX(name,j+1,j+1);
        // c7->cd(j+1);
        c6->cd(j+1);
        FCALBCALMCprof[j]->GetYaxis()->SetRangeUser(ymin,ymax);
        FCALBCALMCprof[j]->SetMarkerColor(4);
        FCALBCALMCprof[j]->SetLineColor(4);
        FCALBCALMCprof[j]->Draw("same");
        t2txt.Form("Theta=%.2f",theta);
        t2->DrawLatex(0.2,0.8,t2txt);
        t2->SetNDC();
    }
    
    c0->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf(");
    c1->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf");
    c2->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf");
    // c3->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf");
    c4->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf");
    // c5->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf");
    c6->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf)");
    // c7->SaveAs("plot_compare2DEff_"+tag+"_"+RunPeriod+"_"+RefLabel+".pdf)");
    
}
