void plot_pi0pippim (TString filename, TString tag="")
{
// File: Z2pi_trees.C
//

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
    
    TFile *f = new TFile(filename+".root","read");
    
    cout << "Opening root file: " << (filename+".root").Data() << endl;
    
    TH1F *NumEventsSurvivedAction = (TH1F*)f->Get("NumEventsSurvivedAction");
    TH2F *NumCombosSurvivedAction = (TH2F*)f->Get("NumCombosSurvivedAction");
    TH1F *BeamEnergy = (TH1F*)f->Get("BeamEnergy");
    // TH1F *pMomentumMeasured = (TH1F*)f->Get("pMomentumMeasured");
    TH1F *Chi2NDF = (TH1F*)f->Get("Chi2NDF");
    TH2F *Chi2_vs_Pi0Theta = (TH2F*)f->Get("Chi2_vs_Pi0Theta");
    
    TH1F *ProtonP = (TH1F*)f->Get("ProtonP");
    TH1F *PiPlusP = (TH1F*)f->Get("PiPlusP");
    TH1F *PiMinusP = (TH1F*)f->Get("PiMinusP");
    TH1F *Pi0P = (TH1F*)f->Get("Pi0P");
    TH1F *Photon1P = (TH1F*)f->Get("Photon1P");
    TH1F *CostheStarB_FCALBCAL = (TH1F*)f->Get("CostheStarB_FCALBCAL");
    TH2F *CosTheta_Psi = (TH2F*)f->Get("CosTheta_Psi");
    
    TH1F *Pi0MeasuredMass = (TH1F*)f->Get("Pi0MeasuredMass");
    TH2F *Photon1P_MeasVsKin = (TH2F*)f->Get("Photon1P_MeasVsKin");
    TH1F *CostheStarA_FCALBCAL = (TH1F*)f->Get("CostheStarA_FCALBCAL");
    TH1F *Mass3pi = (TH1F*)f->Get("Mass3pi");
    TH2F *Pi0P_MeasVsKin = (TH2F*)f->Get("Pi0P_MeasVsKin");
    // TH1F *pDeltap = (TH1F*)f->Get("pDeltap");
    TH2F *Photon2P_vs_Photon1P = (TH2F*)f->Get("Photon2P_vs_Photon1P");
    TH2F *ProtondEdX_vs_P = (TH2F*)f->Get("ProtondEdX_vs_P");

    TH1F *CostheStarB_BCAL = (TH1F*)f->Get("CostheStarB_BCAL");
    TH1F *CostheStarB_FCAL = (TH1F*)f->Get("CostheStarB_FCAL");
    TH1F *CL = (TH1F*)f->Get("CL");
    TH1F *CostheStarA_BCAL = (TH1F*)f->Get("CostheStarA_BCAL");
    TH1F *CostheStarA_FCAL = (TH1F*)f->Get("CostheStarA_FCAL");
    TH1F *Photon2P = (TH1F*)f->Get("Photon2P");

    TH1F *Pi0Theta_BCAL= (TH1F*)f->Get("Pi0Theta_BCAL");
    TH1F *Pi0Theta_FCAL= (TH1F*)f->Get("Pi0Theta_FCAL");
    TH1F *Pi0Theta_FCALBCAL= (TH1F*)f->Get("Pi0Theta_FCALBCAL");
    TH1F *PhotonTheta_BCAL= (TH1F*)f->Get("PhotonTheta_BCAL");
    TH1F *PhotonTheta_FCAL= (TH1F*)f->Get("PhotonTheta_FCAL");
    TH1F *PhotonTheta_FCALBCAL= (TH1F*)f->Get("PhotonTheta_FCALBCAL");
    
    TH1F *E2_w1_BCAL= (TH1F*)f->Get("E2_w1_BCAL");
    TH1F *E2_w1_FCAL= (TH1F*)f->Get("E2_w1_FCAL");
    TH1F *E2_w1_FCALBCAL= (TH1F*)f->Get("E2_w1_FCALBCAL");
    
    TH1F *E2_weff_BCAL= (TH1F*)f->Get("E2_weff_BCAL");
    TH1F *E2_weff_FCAL= (TH1F*)f->Get("E2_weff_FCAL");
    TH1F *E2_weff_FCALBCAL= (TH1F*)f->Get("E2_weff_FCALBCAL");
    
    TH1F *E2_eff_BCAL= (TH1F*)f->Get("E2_eff_BCAL");
    TH1F *E2_eff_FCAL= (TH1F*)f->Get("E2_eff_FCAL");
    TH1F *E2_eff_FCALBCAL= (TH1F*)f->Get("E2_eff_FCALBCAL");
    
    TH2F *dH2_E2_w1_BCAL= (TH2F*)f->Get("dH2_E2_w1_BCAL");
    TH2F *dH2_E2_w1_FCAL= (TH2F*)f->Get("dH2_E2_w1_FCAL");
    TH2F *dH2_E2_w1_FCALBCAL= (TH2F*)f->Get("dH2_E2_w1_FCALBCAL");
    
    TH2F *dH2_E2_weff_BCAL= (TH2F*)f->Get("dH2_E2_weff_BCAL");
    TH2F *dH2_E2_weff_FCAL= (TH2F*)f->Get("dH2_E2_weff_FCAL");
    TH2F *dH2_E2_weff_FCALBCAL= (TH2F*)f->Get("dH2_E2_weff_FCALBCAL");
    
    TH2F *dH2_E2_eff_BCAL= (TH2F*)f->Get("dH2_E2_eff_BCAL");
    TH2F *dH2_E2_eff_FCAL= (TH2F*)f->Get("dH2_E2_eff_FCAL");
    TH2F *dH2_E2_eff_FCALBCAL= (TH2F*)f->Get("dH2_E2_eff_FCALBCAL");
    
    E2_eff_BCAL->Reset();
    E2_eff_FCAL->Reset();
    E2_eff_FCALBCAL->Reset();
    
    dH2_E2_eff_BCAL->Reset();
    dH2_E2_eff_FCAL->Reset();
    dH2_E2_eff_FCALBCAL->Reset();
    
    // now divide histograms to produce an efficiency. Must do this if input histograms have been added together (i.e. efficiencies will not be correct).
    E2_eff_FCAL->Add(E2_weff_FCAL);
    E2_eff_FCAL->Divide(E2_w1_FCAL);
    E2_eff_BCAL->Add(E2_weff_BCAL);
    E2_eff_BCAL->Divide(E2_w1_BCAL);
    E2_eff_FCALBCAL->Add(E2_weff_FCALBCAL);
    E2_eff_FCALBCAL->Divide(E2_w1_FCALBCAL);
    
    dH2_E2_eff_FCAL->Add(dH2_E2_weff_FCAL);
    dH2_E2_eff_FCAL->Divide(dH2_E2_w1_FCAL);
    dH2_E2_eff_BCAL->Add(dH2_E2_weff_BCAL);
    dH2_E2_eff_BCAL->Divide(dH2_E2_w1_BCAL);
    dH2_E2_eff_FCALBCAL->Add(dH2_E2_weff_FCALBCAL);
    dH2_E2_eff_FCALBCAL->Divide(dH2_E2_w1_FCALBCAL);
    
    
    Double_t xmin = 0;
    Double_t xmax = 2;
    Double_t ymin = 100;
    Double_t ymax = 10000;
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1000,700);

   c0->Divide(3,2);
    c0->cd(1);
    // gPad->SetLogy();
        
    NumEventsSurvivedAction->SetTitle(filename);
    // NumEventsSurvivedAction->GetXaxis()->SetRangeUser(xmin,xmax);
    // NumEventsSurvivedAction->GetYaxis()->SetRangeUser(ymin,ymax);
    NumEventsSurvivedAction->GetXaxis()->SetTitleSize(0.05);
    NumEventsSurvivedAction->GetYaxis()->SetTitleSize(0.05);
    // NumEventsSurvivedAction->GetXaxis()->SetTitle("Events");
    NumEventsSurvivedAction->SetMarkerColor(4);
    NumEventsSurvivedAction->Draw();
    
    c0->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 5000;
    
    NumCombosSurvivedAction->SetTitle(filename);
    // NumCombosSurvivedAction->GetXaxis()->SetRangeUser(xmin,xmax);
    // NumCombosSurvivedAction->GetYaxis()->SetRangeUser(ymin,ymax);
    NumCombosSurvivedAction->GetXaxis()->SetTitleSize(0.05);
    NumCombosSurvivedAction->GetYaxis()->SetTitleSize(0.05);
    // NumCombosSurvivedAction->GetXaxis()->SetTitle("Events");
    // NumCombosSurvivedAction->SetMarkerColor(4);
    NumCombosSurvivedAction->Draw("colz");
    
    c0->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    BeamEnergy->SetTitle(filename);
    // BeamEnergy->GetXaxis()->SetRangeUser(xmin,xmax);
    // BeamEnergy->GetYaxis()->SetRangeUser(ymin,ymax);
    BeamEnergy->GetXaxis()->SetTitleSize(0.05);
    BeamEnergy->GetYaxis()->SetTitleSize(0.05);
    BeamEnergy->GetXaxis()->SetTitle("Beam Energy (GeV)");
    BeamEnergy->SetMarkerColor(4);
    BeamEnergy->Draw();
    
    c0->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CL->SetTitle(filename);
    gPad->SetLogy();
    // CL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CL->GetYaxis()->SetRangeUser(ymin,ymax);
    CL->GetXaxis()->SetTitleSize(0.05);
    CL->GetYaxis()->SetTitleSize(0.05);
    CL->GetXaxis()->SetTitle("Confidence Level");
    CL->SetMarkerColor(4);
    CL->Draw();
    
    c0->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Chi2NDF->SetTitle(filename);
    // Chi2NDF->GetXaxis()->SetRangeUser(xmin,xmax);
    // Chi2NDF->GetYaxis()->SetRangeUser(ymin,ymax);
    Chi2NDF->GetXaxis()->SetTitleSize(0.05);
    Chi2NDF->GetYaxis()->SetTitleSize(0.05);
    Chi2NDF->GetXaxis()->SetTitle("Chi2/NDF");
    Chi2NDF->SetMarkerColor(4);
    Chi2NDF->Draw();
    
    c0->cd(6);
    gPad->SetLogz();
    ymin = 0;
    ymax = 4000;
    
    Chi2_vs_Pi0Theta->SetTitle(filename);
    // Chi2_vs_Pi0Theta->GetXaxis()->SetRangeUser(xmin,xmax);
    // Chi2_vs_Pi0Theta->GetYaxis()->SetRangeUser(ymin,ymax);
    Chi2_vs_Pi0Theta->GetXaxis()->SetTitleSize(0.05);
    Chi2_vs_Pi0Theta->GetYaxis()->SetTitleSize(0.05);
    Chi2_vs_Pi0Theta->GetYaxis()->SetTitle("Chi2/NDF");
    Chi2_vs_Pi0Theta->GetXaxis()->SetTitle("Pi0 Theta");
    Chi2_vs_Pi0Theta->SetMarkerColor(4);
    Chi2_vs_Pi0Theta->Draw("colz");
    
    
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,1000,700);
    
    c1->Divide(3,2);
    
    
    c1->cd(1);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    ProtonP->SetTitle(filename);
    // ProtonP->GetXaxis()->SetRangeUser(xmin,xmax);
    // ProtonP->GetYaxis()->SetRangeUser(ymin,ymax);
    ProtonP->GetXaxis()->SetTitleSize(0.05);
    ProtonP->GetYaxis()->SetTitleSize(0.05);
    ProtonP->GetXaxis()->SetTitle("Proton P (GeV)");
    ProtonP->SetMarkerColor(4);
    ProtonP->Draw();
    
    c1->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    PiPlusP->SetTitle(filename);
    // PiPlusP->GetXaxis()->SetRangeUser(xmin,xmax);
    // PiPlusP->GetYaxis()->SetRangeUser(ymin,ymax);
    PiPlusP->GetXaxis()->SetTitleSize(0.05);
    PiPlusP->GetYaxis()->SetTitleSize(0.05);
    PiPlusP->GetXaxis()->SetTitle("#pi^{+} P (GeV)");
    PiPlusP->SetMarkerColor(4);
    PiPlusP->Draw();
    
    c1->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    PiMinusP->SetTitle(filename);
    // PiMinusP->GetXaxis()->SetRangeUser(xmin,xmax);
    // PiMinusP->GetYaxis()->SetRangeUser(ymin,ymax);
    PiMinusP->GetXaxis()->SetTitleSize(0.05);
    PiMinusP->GetYaxis()->SetTitleSize(0.05);
    PiMinusP->GetXaxis()->SetTitle("#pi^{-} P (GeV)");
    PiMinusP->SetMarkerColor(4);
    PiMinusP->Draw();
    
    c1->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Pi0P->SetTitle(filename);
    // Pi0P->GetXaxis()->SetRangeUser(xmin,xmax);
    // Pi0P->GetYaxis()->SetRangeUser(ymin,ymax);
    Pi0P->GetXaxis()->SetTitleSize(0.05);
    Pi0P->GetYaxis()->SetTitleSize(0.05);
    Pi0P->GetXaxis()->SetTitle("#pi^{0} P (GeV)");
    Pi0P->SetMarkerColor(4);
    Pi0P->Draw();
    
    c1->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Photon1P->SetTitle(filename);
    // Photon1P->GetXaxis()->SetRangeUser(xmin,xmax);
    // Photon1P->GetYaxis()->SetRangeUser(ymin,ymax);
    Photon1P->GetXaxis()->SetTitleSize(0.05);
    Photon1P->GetYaxis()->SetTitleSize(0.05);
    Photon1P->GetXaxis()->SetTitle("Photon 1 P (GeV)");
    Photon1P->GetXaxis()->SetNdivisions(505);
    Photon1P->SetMarkerColor(4);
    Photon1P->Draw();
    
    c1->cd(6);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Photon2P->SetTitle(filename);
    // Photon2P->GetXaxis()->SetRangeUser(xmin,xmax);
    // Photon2P->GetYaxis()->SetRangeUser(ymin,ymax);
    Photon2P->GetXaxis()->SetTitleSize(0.05);
    Photon2P->GetYaxis()->SetTitleSize(0.05);
    // Photon2P->GetYaxis()->SetTitle("CosTheta");
    Photon2P->GetXaxis()->SetTitle("Photon 2 P (GeV)");
    Photon2P->SetMarkerColor(4);
    Photon2P->Draw();
    
    
    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1000,700);
    
    c2->Divide(3,2);
    
    c2->cd(1);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Pi0MeasuredMass->SetTitle(filename);
    // Pi0MeasuredMass->GetXaxis()->SetRangeUser(xmin,xmax);
    // Pi0MeasuredMass->GetYaxis()->SetRangeUser(ymin,ymax);
    Pi0MeasuredMass->GetXaxis()->SetTitleSize(0.05);
    Pi0MeasuredMass->GetYaxis()->SetTitleSize(0.05);
    // Pi0MeasuredMass->GetXaxis()->SetTitle("");
    Pi0MeasuredMass->SetMarkerColor(4);
    Pi0MeasuredMass->GetXaxis()->SetNdivisions(505);
    Pi0MeasuredMass->Fit("gaus");
    Pi0MeasuredMass->Draw();
    
    c2->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Mass3pi->SetTitle(filename);
    // Mass3pi->GetXaxis()->SetRangeUser(xmin,xmax);
    // Mass3pi->GetYaxis()->SetRangeUser(ymin,ymax);
    Mass3pi->GetXaxis()->SetTitleSize(0.05);
    Mass3pi->GetYaxis()->SetTitleSize(0.05);
    // Mass3pi->GetXaxis()->SetTitle("#chi^{2}");
    Mass3pi->SetMarkerColor(4);
    Mass3pi->Draw();
    
    c2->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Pi0P_MeasVsKin->SetTitle(filename);
    // Pi0P_MeasVsKin->GetXaxis()->SetRangeUser(xmin,xmax);
    // Pi0P_MeasVsKin->GetYaxis()->SetRangeUser(ymin,ymax);
    Pi0P_MeasVsKin->GetXaxis()->SetTitleSize(0.05);
    Pi0P_MeasVsKin->GetYaxis()->SetTitleSize(0.05);
    Pi0P_MeasVsKin->GetYaxis()->SetTitle("Pi0 P MEAS");
    Pi0P_MeasVsKin->GetXaxis()->SetTitle("Pi0 P KIN");
    Pi0P_MeasVsKin->SetMarkerColor(4);
    Pi0P_MeasVsKin->Draw("colz");
    
    c2->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Photon1P_MeasVsKin->SetTitle(filename);
    // Photon1P_MeasVsKin->GetXaxis()->SetRangeUser(xmin,xmax);
    // Photon1P_MeasVsKin->GetYaxis()->SetRangeUser(ymin,ymax);
    Photon1P_MeasVsKin->GetXaxis()->SetTitleSize(0.05);
    Photon1P_MeasVsKin->GetYaxis()->SetTitleSize(0.05);
    Photon1P_MeasVsKin->GetYaxis()->SetTitle("Photon 1 P MEAS");
    Photon1P_MeasVsKin->GetXaxis()->SetTitle("Photon 1 P KIN");
    Photon1P_MeasVsKin->SetMarkerColor(4);
    Photon1P_MeasVsKin->GetXaxis()->SetNdivisions(505);
    Photon1P_MeasVsKin->Draw("colz");
    
    c2->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Photon2P_vs_Photon1P->SetTitle(filename);
    // Photon2P_vs_Photon1P->GetXaxis()->SetRangeUser(xmin,xmax);
    // Photon2P_vs_Photon1P->GetYaxis()->SetRangeUser(ymin,ymax);
    Photon2P_vs_Photon1P->GetXaxis()->SetTitleSize(0.05);
    Photon2P_vs_Photon1P->GetYaxis()->SetTitleSize(0.05);
    Photon2P_vs_Photon1P->GetYaxis()->SetTitle("Photon 2 P");
    Photon2P_vs_Photon1P->GetXaxis()->SetTitle("Photon 1 P");
    Photon2P_vs_Photon1P->SetMarkerColor(4);
    Photon2P_vs_Photon1P->Draw("colz");
    
    c2->cd(6);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    ProtondEdX_vs_P->SetTitle(filename);
    // ProtondEdX_vs_P->GetXaxis()->SetRangeUser(xmin,xmax);
    // ProtondEdX_vs_P->GetYaxis()->SetRangeUser(ymin,ymax);
    ProtondEdX_vs_P->GetXaxis()->SetTitleSize(0.05);
    ProtondEdX_vs_P->GetYaxis()->SetTitleSize(0.05);
    ProtondEdX_vs_P->GetYaxis()->SetTitle("dE/dx Proton");
    ProtondEdX_vs_P->GetXaxis()->SetTitle("Proton P");
    ProtondEdX_vs_P->SetMarkerColor(4);
    ProtondEdX_vs_P->Draw("colz");

    
    TCanvas *c3 = new TCanvas("c3", "c3",200,10,1000,700);
    
    c3->Divide(3,2);

    c3->cd(1);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Pi0Theta_BCAL->SetTitle(filename);
    // Pi0Theta_BCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // Pi0Theta_BCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    Pi0Theta_BCAL->GetXaxis()->SetTitleSize(0.05);
    Pi0Theta_BCAL->GetYaxis()->SetTitleSize(0.05);
    Pi0Theta_BCAL->GetXaxis()->SetTitle("Pi0 BCAL Theta (deg)");
    Pi0Theta_BCAL->SetMarkerColor(4);
    Pi0Theta_BCAL->Draw();

    c3->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Pi0Theta_FCAL->SetTitle(filename);
    // Pi0Theta_FCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // Pi0Theta_FCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    Pi0Theta_FCAL->GetXaxis()->SetTitleSize(0.05);
    Pi0Theta_FCAL->GetYaxis()->SetTitleSize(0.05);
    Pi0Theta_FCAL->GetXaxis()->SetTitle("Pi0 FCAL Theta (deg)");
    Pi0Theta_FCAL->SetMarkerColor(4);
    Pi0Theta_FCAL->Draw();

    c3->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    Pi0Theta_FCALBCAL->SetTitle(filename);
    // Pi0Theta_FCALBCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // Pi0Theta_FCALBCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    Pi0Theta_FCALBCAL->GetXaxis()->SetTitleSize(0.05);
    Pi0Theta_FCALBCAL->GetYaxis()->SetTitleSize(0.05);
    Pi0Theta_FCALBCAL->GetXaxis()->SetTitle("Pi0 FCAL/BCAL Theta (deg)");
    Pi0Theta_FCALBCAL->SetMarkerColor(4);
    Pi0Theta_FCALBCAL->Draw();

    c3->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    PhotonTheta_BCAL->SetTitle(filename);
    // PhotonTheta_BCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // PhotonTheta_BCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    PhotonTheta_BCAL->GetXaxis()->SetTitleSize(0.05);
    PhotonTheta_BCAL->GetYaxis()->SetTitleSize(0.05);
    PhotonTheta_BCAL->GetXaxis()->SetTitle("Photon BCAL Theta (deg)");
    PhotonTheta_BCAL->SetMarkerColor(4);
    PhotonTheta_BCAL->Draw();

    c3->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    PhotonTheta_FCAL->SetTitle(filename);
    // PhotonTheta_FCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // PhotonTheta_FCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    PhotonTheta_FCAL->GetXaxis()->SetTitleSize(0.05);
    PhotonTheta_FCAL->GetYaxis()->SetTitleSize(0.05);
    PhotonTheta_FCAL->GetXaxis()->SetTitle("Photon FCAL Theta (deg)");
    PhotonTheta_FCAL->SetMarkerColor(4);
    PhotonTheta_FCAL->Draw();

    c3->cd(6);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    PhotonTheta_FCALBCAL->SetTitle(filename);
    // PhotonTheta_FCALBCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // PhotonTheta_FCALBCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    PhotonTheta_FCALBCAL->GetXaxis()->SetTitleSize(0.05);
    PhotonTheta_FCALBCAL->GetYaxis()->SetTitleSize(0.05);
    PhotonTheta_FCALBCAL->GetXaxis()->SetTitle("Photon FCAL/BCAL Theta (deg)");
    PhotonTheta_FCALBCAL->SetMarkerColor(4);
    PhotonTheta_FCALBCAL->Draw();
    
    TCanvas *c4 = new TCanvas("c4", "c4",200,10,1000,700);
    
    c4->Divide(3,2);

    c4->cd(1);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CostheStarA_BCAL->SetTitle(filename);
    // CostheStarA_BCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CostheStarA_BCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    CostheStarA_BCAL->GetXaxis()->SetTitleSize(0.05);
    CostheStarA_BCAL->GetYaxis()->SetTitleSize(0.05);
    CostheStarA_BCAL->GetXaxis()->SetTitle("Costhe* A BCAL");
    CostheStarA_BCAL->SetMarkerColor(4);
    CostheStarA_BCAL->Draw();
    
    c4->cd(2);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CostheStarA_FCAL->SetTitle(filename);
    // CostheStarA_FCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CostheStarA_FCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    CostheStarA_FCAL->GetXaxis()->SetTitleSize(0.05);
    CostheStarA_FCAL->GetYaxis()->SetTitleSize(0.05);
    CostheStarA_FCAL->GetXaxis()->SetTitle("Costhe* A FCAL");
    CostheStarA_FCAL->SetMarkerColor(4);
    CostheStarA_FCAL->Draw();
    
    c4->cd(3);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CostheStarA_FCALBCAL->SetTitle(filename);
    // CostheStarA_FCALBCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CostheStarA_FCALBCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    CostheStarA_FCALBCAL->GetXaxis()->SetTitleSize(0.05);
    CostheStarA_FCALBCAL->GetYaxis()->SetTitleSize(0.05);
    CostheStarA_FCALBCAL->GetXaxis()->SetTitle("Costhe* A FCAL/BCAL");
    CostheStarA_FCALBCAL->SetMarkerColor(4);
    CostheStarA_FCALBCAL->Draw();
    
    c4->cd(4);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CostheStarB_BCAL->SetTitle(filename);
    // CostheStarB_BCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CostheStarB_BCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    CostheStarB_BCAL->GetXaxis()->SetTitleSize(0.05);
    CostheStarB_BCAL->GetYaxis()->SetTitleSize(0.05);
    CostheStarB_BCAL->GetXaxis()->SetTitle("Costhe* B BCAL");
    CostheStarB_BCAL->SetMarkerColor(4);
    CostheStarB_BCAL->Draw();
    
    c4->cd(5);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CostheStarB_FCAL->SetTitle(filename);
    // CostheStarB_FCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CostheStarB_FCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    CostheStarB_FCAL->GetXaxis()->SetTitleSize(0.05);
    CostheStarB_FCAL->GetYaxis()->SetTitleSize(0.05);
    CostheStarB_FCAL->GetXaxis()->SetTitle("Costhe* B FCAL");
    CostheStarB_FCAL->SetMarkerColor(4);
    CostheStarB_FCAL->Draw();
    
    c4->cd(6);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    CostheStarB_FCALBCAL->SetTitle(filename);
    // CostheStarB_FCALBCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    // CostheStarB_FCALBCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    CostheStarB_FCALBCAL->GetXaxis()->SetTitleSize(0.05);
    CostheStarB_FCALBCAL->GetYaxis()->SetTitleSize(0.05);
    CostheStarB_FCALBCAL->GetXaxis()->SetTitle("Costhe* B FCAl/BCAL");
    CostheStarB_FCALBCAL->SetMarkerColor(4);
    CostheStarB_FCALBCAL->Draw();
    
    
    TCanvas *c5 = new TCanvas("c5", "c5",200,10,1000,700);
    
    c5->Divide(3,2);

    c5->cd(1);
    // gPad->SetLogy();
    ymin = 0;
    ymax = 4000;
    
    //
    // unpack costhestar distribution and intepret it as an efficiency distribution as a function of E2meas
    
    const Int_t ndim = CostheStarB_FCAL->GetNbinsX();
    Double_t xlo = CostheStarB_FCAL->GetBinLowEdge(1);
    Double_t width = CostheStarB_FCAL->GetBinWidth(1);
    int max_bin;
    max_bin = CostheStarA_BCAL->GetMaximumBin();
    Double_t max_contentA_BCAL = CostheStarA_BCAL->GetBinContent(max_bin);     // No longer used: use as normalization for efficiency
    max_bin = CostheStarA_FCAL->GetMaximumBin();
    Double_t max_contentA_FCAL = CostheStarA_FCAL->GetBinContent(max_bin);     // No longer used:  use as normalization for efficiency
    max_bin = CostheStarA_FCALBCAL->GetMaximumBin();
    Double_t max_contentA_FCALBCAL = CostheStarA_FCALBCAL->GetBinContent(max_bin);     // No longer used:  used as normalization for efficiency
    
    Double_t cosmin = 0;                                  // Use interval [cosmin,cosmax] to set efficiency = 1
    Double_t cosmax = 0.25;
    Double_t eff0 = 1.0;
    Double_t ave_contentB_BCAL = 0;
    Int_t npts_contentB_BCAL = 0;
    for (Int_t j=0; j<ndim; j++) {
        Float_t cos = xlo + (j+0.5)*width;
        Float_t content = CostheStarB_BCAL->GetBinContent(j+1);
        if (cos > cosmin && cos < cosmax) {
            ave_contentB_BCAL += content;
            npts_contentB_BCAL++;
        }
    }
    ave_contentB_BCAL = npts_contentB_BCAL>0? ave_contentB_BCAL*eff0/npts_contentB_BCAL: 0;
    cout << " ave_contentB_BCAL=" << ave_contentB_BCAL << endl;
    
    Double_t ave_contentB_FCAL = 0;
    Int_t npts_contentB_FCAL = 0;
    for (Int_t j=0; j<ndim; j++) {
        Float_t cos = xlo + (j+0.5)*width;
        Float_t content = CostheStarB_FCAL->GetBinContent(j+1);
        if (cos > cosmin && cos < cosmax) {
            ave_contentB_FCAL += content;
            npts_contentB_FCAL++;
        }
    }
    ave_contentB_FCAL = npts_contentB_FCAL>0? ave_contentB_FCAL*eff0/npts_contentB_FCAL: 0;
    cout << " ave_contentB_FCAL=" << ave_contentB_FCAL << endl;
    
    Double_t ave_contentB_FCALBCAL = 0;
    Int_t npts_contentB_FCALBCAL = 0;
    for (Int_t j=0; j<ndim; j++) {
        Float_t cos = xlo + (j+0.5)*width;
        Float_t content = CostheStarB_FCALBCAL->GetBinContent(j+1);
        if (cos > cosmin && cos < cosmax) {
            ave_contentB_FCALBCAL += content;
            npts_contentB_FCALBCAL++;
        }
    }
    ave_contentB_FCALBCAL = npts_contentB_FCALBCAL>0? ave_contentB_FCALBCAL*eff0/npts_contentB_FCALBCAL: 0;
    cout << " ave_contentB_FCALBCAL=" << ave_contentB_FCALBCAL << endl;

    
    TH1F *CostheStarA_BCAL_eff =  (TH1F*)CostheStarB_FCAL->Clone("CostheStarA_BCAL_eff");
    TH1F *CostheStarA_FCAL_eff =  (TH1F*)CostheStarB_FCAL->Clone("CostheStarA_FCAL_eff");
    TH1F *CostheStarA_FCALBCAL_eff =  (TH1F*)CostheStarB_FCALBCAL->Clone("CostheStarA_FCALBCAL_eff");
    TH1F *CostheStarB_BCAL_eff =  (TH1F*)CostheStarB_FCAL->Clone("CostheStarB_BCAL_eff");
    TH1F *CostheStarB_FCAL_eff =  (TH1F*)CostheStarB_FCAL->Clone("CostheStarB_FCAL_eff");
    TH1F *CostheStarB_FCALBCAL_eff =  (TH1F*)CostheStarB_FCAL->Clone("CostheStarB_FCALBCAL_eff");
    
    Double_t E2calcA_BCAL[ndim];
    Double_t E2calcA_BCAL_err[ndim];
    Double_t E2calcA_BCAL_eff[ndim];
    Double_t E2calcA_BCAL_efferr[ndim];
    
    Double_t E2calcA_FCAL[ndim];
    Double_t E2calcA_FCAL_err[ndim];
    Double_t E2calcA_FCAL_eff[ndim];
    Double_t E2calcA_FCAL_efferr[ndim];
    
    Double_t E2calcA_FCALBCAL[ndim];
    Double_t E2calcA_FCALBCAL_err[ndim];
    Double_t E2calcA_FCALBCAL_eff[ndim];
    Double_t E2calcA_FCALBCAL_efferr[ndim];
    
    Double_t E2calcB_BCAL[ndim];
    Double_t E2calcB_BCAL_err[ndim];
    Double_t E2calcB_BCAL_eff[ndim];
    Double_t E2calcB_BCAL_efferr[ndim];
    
    Double_t E2calcB_FCAL[ndim];
    Double_t E2calcB_FCAL_err[ndim];
    Double_t E2calcB_FCAL_eff[ndim];
    Double_t E2calcB_FCAL_efferr[ndim];
    
    Double_t E2calcB_FCALBCAL[ndim];
    Double_t E2calcB_FCALBCAL_err[ndim];
    Double_t E2calcB_FCALBCAL_eff[ndim];
    Double_t E2calcB_FCALBCAL_efferr[ndim];
    
    eff0 = 0.96;
    Int_t npointsA_BCAL=0;
    Int_t npointsA_FCAL=0;
    Int_t npointsA_FCALBCAL=0;
    Int_t npointsB_BCAL=0;
    Int_t npointsB_FCAL=0;
    Int_t npointsB_FCALBCAL=0;
    
    Double_t MPI = 0.1349766;
    Double_t Epi0mean = Pi0P->GetMean();
    Double_t Ppi0mean = sqrt(Epi0mean*Epi0mean - MPI*MPI);
    
    
    for(Int_t j=0;j<ndim;j++) {
        Double_t costhestar_bin;
        Double_t content;
        Double_t efficiency;
        Double_t sigma;
        
        costhestar_bin = CostheStarA_BCAL->GetBinCenter(j+1);
        content = CostheStarA_BCAL->GetBinContent(j+1);
        efficiency = max_contentA_BCAL > 0? content/max_contentA_BCAL: 0;
        CostheStarA_BCAL_eff->SetBinContent(j+1,efficiency);
        sigma = sqrt (content)/max_contentA_BCAL;   //  just take uncertainty in point, ignore fluctuation in maximum
        CostheStarA_BCAL_eff->SetBinError(j+1,sigma);
        if (efficiency > 0) {
            E2calcA_BCAL[npointsA_BCAL] = 0.5*(Epi0mean - Ppi0mean*costhestar_bin);      // use nominal pi0 energy in "reconstruction"
            E2calcA_BCAL_err[npointsA_BCAL] = 0.005;
            E2calcA_BCAL_eff[npointsA_BCAL] = efficiency/eff0;     // normalize by externally determined efficiency
            E2calcA_BCAL_efferr[npointsA_BCAL] = sigma;
            npointsA_BCAL++;
        }
        costhestar_bin = CostheStarA_FCAL->GetBinCenter(j+1);
        content = CostheStarA_FCAL->GetBinContent(j+1);
        efficiency = max_contentA_FCAL > 0? content/max_contentA_FCAL: 0;
        CostheStarA_FCAL_eff->SetBinContent(j+1,efficiency);
        sigma = sqrt (content)/max_contentA_FCAL;   //  just take uncertainty in point, ignore fluctuation in maximum
        CostheStarA_FCAL_eff->SetBinError(j+1,sigma);
        if (efficiency > 0) {
            E2calcA_FCAL[npointsA_FCAL] = 0.5*(Epi0mean - Ppi0mean*costhestar_bin);      // use nominal pi0 energy in "reconstruction"
            E2calcA_FCAL_err[npointsA_FCAL] = 0.005;
            E2calcA_FCAL_eff[npointsA_FCAL] = efficiency/eff0;     // normalize by externally determined efficiency
            E2calcA_FCAL_efferr[npointsA_FCAL] = sigma;
            npointsA_FCAL++;
        }
        costhestar_bin = CostheStarA_FCALBCAL->GetBinCenter(j+1);
        content = CostheStarA_FCALBCAL->GetBinContent(j+1);
        efficiency = max_contentA_FCALBCAL > 0? content/max_contentA_FCALBCAL: 0;
        CostheStarA_FCALBCAL_eff->SetBinContent(j+1,efficiency);
        sigma = sqrt (content)/max_contentA_FCALBCAL;   //  just take uncertainty in point, ignore fluctuation in maximum
        CostheStarA_FCALBCAL_eff->SetBinError(j+1,sigma);
        if (efficiency > 0) {
            E2calcA_FCALBCAL[npointsA_FCALBCAL] = 0.5*(Epi0mean - Ppi0mean*costhestar_bin);      // use nominal pi0 energy in "reconstruction"
            E2calcA_FCALBCAL_err[npointsA_FCALBCAL] = 0.005;
            E2calcA_FCALBCAL_eff[npointsA_FCALBCAL] = efficiency/eff0;     // normalize by externally determined efficiency
            E2calcA_FCALBCAL_efferr[npointsA_FCALBCAL] = sigma;
            npointsA_FCALBCAL++;
        }
        
        
        costhestar_bin = CostheStarB_BCAL->GetBinCenter(j+1);
        content = CostheStarB_BCAL->GetBinContent(j+1);
        efficiency = ave_contentB_BCAL > 0? content/ave_contentB_BCAL: 0;
        CostheStarB_BCAL_eff->SetBinContent(j+1,efficiency);
        sigma = ave_contentB_BCAL > 0? sqrt (content)/ave_contentB_BCAL: 0;   //  just take uncertainty in point, ignore fluctuation in maximum
        CostheStarB_BCAL_eff->SetBinError(j+1,sigma);
        if (efficiency > 0) {
            E2calcB_BCAL[npointsB_BCAL] = 0.5*(Epi0mean - Ppi0mean*costhestar_bin);      // use nominal pi0 energy in "reconstruction"
            E2calcB_BCAL_err[npointsB_BCAL] = 0.005;
            E2calcB_BCAL_eff[npointsB_BCAL] = efficiency;
            E2calcB_BCAL_efferr[npointsB_BCAL] = sigma;
            npointsB_BCAL++;
        }
        costhestar_bin = CostheStarB_FCAL->GetBinCenter(j+1);
        content = CostheStarB_FCAL->GetBinContent(j+1);
        efficiency = ave_contentB_FCAL > 0? content/ave_contentB_FCAL: 0;
        CostheStarB_FCAL_eff->SetBinContent(j+1,efficiency);
        sigma = ave_contentB_FCAL > 0? sqrt (content)/ave_contentB_FCAL: 0;   //  just take uncertainty in point, ignore fluctuation in maximum
        CostheStarB_FCAL_eff->SetBinError(j+1,sigma);
        if (efficiency > 0) {
            E2calcB_FCAL[npointsB_FCAL] = 0.5*(Epi0mean - Ppi0mean*costhestar_bin);      // use nominal pi0 energy in "reconstruction"
            E2calcB_FCAL_err[npointsB_FCAL] = 0.005;
            E2calcB_FCAL_eff[npointsB_FCAL] = efficiency;
            E2calcB_FCAL_efferr[npointsB_FCAL] = sigma;
            npointsB_FCAL++;
        }
        costhestar_bin = CostheStarB_FCALBCAL->GetBinCenter(j+1);
        content = CostheStarB_FCALBCAL->GetBinContent(j+1);
        efficiency = ave_contentB_FCALBCAL > 0? content/ave_contentB_FCALBCAL: 0;
        CostheStarB_FCALBCAL_eff->SetBinContent(j+1,efficiency);
        sigma = ave_contentB_FCALBCAL > 0? sqrt (content)/ave_contentB_FCALBCAL: 0;   //  just take uncertainty in point, ignore fluctuation in maximum
        CostheStarB_FCALBCAL_eff->SetBinError(j+1,sigma);
        if (efficiency > 0) {
            E2calcB_FCALBCAL[npointsB_FCALBCAL] = 0.5*(Epi0mean - Ppi0mean*costhestar_bin);      // use nominal pi0 energy in "reconstruction"
            E2calcB_FCALBCAL_err[npointsB_FCALBCAL] = 0.005;
            E2calcB_FCALBCAL_eff[npointsB_FCALBCAL] = efficiency;
            E2calcB_FCALBCAL_efferr[npointsB_FCALBCAL] = sigma;
            npointsB_FCALBCAL++;
        }
        
        
    }
    
    xmin=0.;
    xmax=2.0;
    ymin=0;
    ymax=1.2;
    
    TGraphErrors *gr_E2calcA_BCAL = new TGraphErrors(npointsA_BCAL,E2calcA_BCAL,E2calcA_BCAL_eff,E2calcA_BCAL_err,E2calcA_BCAL_efferr);
    
    c5->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcA_BCAL->SetTitle("Method A BCAL");
    gr_E2calcA_BCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcA_BCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcA_BCAL->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcA_BCAL->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcA_BCAL->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcA_BCAL->GetYaxis()->SetTitle("Rel Efficiency");
    gr_E2calcA_BCAL->SetLineColor(2);
    gr_E2calcA_BCAL->SetMarkerColor(2);
    gr_E2calcA_BCAL->SetMarkerStyle(20);
    gr_E2calcA_BCAL->SetMarkerSize(0.3);
    gr_E2calcA_BCAL->Draw("Ap");
    
    TGraphErrors *gr_E2calcA_FCAL = new TGraphErrors(npointsA_FCAL,E2calcA_FCAL,E2calcA_FCAL_eff,E2calcA_FCAL_err,E2calcA_FCAL_efferr);
    
    c5->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcA_FCAL->SetTitle("Method A FCAL");
    gr_E2calcA_FCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcA_FCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcA_FCAL->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcA_FCAL->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcA_FCAL->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcA_FCAL->GetYaxis()->SetTitle("Rel Efficiency");
    gr_E2calcA_FCAL->SetLineColor(2);
    gr_E2calcA_FCAL->SetMarkerColor(2);
    gr_E2calcA_FCAL->SetMarkerStyle(20);
    gr_E2calcA_FCAL->SetMarkerSize(0.3);
    gr_E2calcA_FCAL->Draw("Ap");

    TGraphErrors *gr_E2calcA_FCALBCAL = new TGraphErrors(npointsA_FCALBCAL,E2calcA_FCALBCAL,E2calcA_FCALBCAL_eff,E2calcA_FCALBCAL_err,E2calcA_FCALBCAL_efferr);
    
    c5->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcA_FCALBCAL->SetTitle("Method A FCAL/BCAL");
    gr_E2calcA_FCALBCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcA_FCALBCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcA_FCALBCAL->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcA_FCALBCAL->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcA_FCALBCAL->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcA_FCALBCAL->GetYaxis()->SetTitle("Rel Efficiency");
    gr_E2calcA_FCALBCAL->SetLineColor(2);
    gr_E2calcA_FCALBCAL->SetMarkerColor(2);
    gr_E2calcA_FCALBCAL->SetMarkerStyle(20);
    gr_E2calcA_FCALBCAL->SetMarkerSize(0.3);
    gr_E2calcA_FCALBCAL->Draw("Ap");
    
    TGraphErrors *gr_E2calcB_BCAL = new TGraphErrors(npointsB_BCAL,E2calcB_BCAL,E2calcB_BCAL_eff,E2calcB_BCAL_err,E2calcB_BCAL_efferr);
    
    c5->cd(4);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_BCAL->SetTitle("Method B BCAL");
    gr_E2calcB_BCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcB_BCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcB_BCAL->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcB_BCAL->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcB_BCAL->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcB_BCAL->GetYaxis()->SetTitle("Rel Efficiency");
    gr_E2calcB_BCAL->SetLineColor(2);
    gr_E2calcB_BCAL->SetMarkerColor(2);
    gr_E2calcB_BCAL->SetMarkerStyle(20);
    gr_E2calcB_BCAL->SetMarkerSize(0.3);
    gr_E2calcB_BCAL->Draw("Ap");
    
    
    TGraphErrors *gr_E2calcB_FCAL = new TGraphErrors(npointsB_FCAL,E2calcB_FCAL,E2calcB_FCAL_eff,E2calcB_FCAL_err,E2calcB_FCAL_efferr);
    
    c5->cd(5);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_FCAL->SetTitle("Method B FCAL");
    gr_E2calcB_FCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcB_FCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcB_FCAL->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcB_FCAL->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcB_FCAL->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcB_FCAL->GetYaxis()->SetTitle("Rel Efficiency");
    gr_E2calcB_FCAL->SetLineColor(2);
    gr_E2calcB_FCAL->SetMarkerColor(2);
    gr_E2calcB_FCAL->SetMarkerStyle(20);
    gr_E2calcB_FCAL->SetMarkerSize(0.3);
    gr_E2calcB_FCAL->Draw("Ap");
    
    
    TGraphErrors *gr_E2calcB_FCALBCAL = new TGraphErrors(npointsB_FCALBCAL,E2calcB_FCALBCAL,E2calcB_FCALBCAL_eff,E2calcB_FCALBCAL_err,E2calcB_FCALBCAL_efferr);
    
    c5->cd(6);
    ymax=6;
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_FCALBCAL->SetTitle("Method B FCAL/BCAL");
    gr_E2calcB_FCALBCAL->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcB_FCALBCAL->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcB_FCALBCAL->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcB_FCALBCAL->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcB_FCALBCAL->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcB_FCALBCAL->GetYaxis()->SetTitle("Rel Efficiency");
    gr_E2calcB_FCALBCAL->SetLineColor(2);
    gr_E2calcB_FCALBCAL->SetMarkerColor(2);
    gr_E2calcB_FCALBCAL->SetMarkerStyle(20);
    gr_E2calcB_FCALBCAL->SetMarkerSize(0.3);
    gr_E2calcB_FCALBCAL->Draw("Ap");
    
    
    TCanvas *c6 = new TCanvas("c6", "c6",200,10,1000,500);
    
    c6->Divide(3,1);
    
    c6->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_BCAL->Draw("Ap");
    E2_eff_BCAL->Draw("same");
    
    c6->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_FCAL->Draw("Ap");
    E2_eff_FCAL->Draw("same");
    
    c6->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_FCALBCAL->Draw("Ap");
    E2_eff_FCALBCAL->Draw("same");
    
    
    c0->SaveAs(filename+".pdf(");
    c1->SaveAs(filename+".pdf");
    c2->SaveAs(filename+".pdf");
    c3->SaveAs(filename+".pdf");
    c4->SaveAs(filename+".pdf");
    c5->SaveAs(filename+".pdf");
    c6->SaveAs(filename+".pdf)");
    
    TString outfile = filename+"_effplots.root";
    TFile *out = new TFile(outfile,"recreate");
    
    gr_E2calcB_BCAL->Write("gr_E2calcB_BCAL");
    gr_E2calcB_FCAL->Write("gr_E2calcB_FCAL");
    E2_eff_BCAL->Write("h1_E2_eff_BCAL");
    E2_eff_FCAL->Write("h1_E2_eff_FCAL");
    dH2_E2_eff_BCAL->Write("dH2_E2_eff_BCAL");
    dH2_E2_eff_FCAL->Write("dH2_E2_eff_FCAL");
    
    out->Close();
    
}
