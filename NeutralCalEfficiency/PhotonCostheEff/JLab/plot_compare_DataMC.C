void plot_compare_DataMC (TString run="",TString tag="")
{
// File: plot_compare_DataMC.C
    // Read in Data and MC distributions from DSelector_pi0pippim__B4 and plot for comparison
//

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
    
    // TString tag="CL6e-7";
    // TString tag="CL01";
    
    TString inDataName = "DSelector_pi0pippim__B4_"+run+"_"+tag+".root";
    // TString inDataName = "DSelector_pi0pippim__B4_030800_"+tag+".root";
    // TString inDataName = "DSelector_pi0pippim__B4_030274-030403_"+tag+".root";
    // TString inDataName = "DSelector_pi0pippim__B4_030796-030895_"+tag+".root";
    cout << "inDataName=" << inDataName << endl;
    
    // TString inMCName = "DSelector_pi0pippim__B4_gen_amp_G4_030800_"+tag+".root";
    TString inMCName = "DSelector_pi0pippim__B4_gen_amp_G4_"+run+"_"+tag+".root";
    cout << "inMCName=" << inMCName << endl;
    
    TString inMC2Name ="DSelector_pi0pippim__B4_gen_amp_G4_"+run+"_"+tag+".root";
    // TString inMC2Name ="DSelector_pi0pippim__B4_gen_amp_G3_030274_"+tag+".root";
    cout << "inMC2Name=" << inMC2Name << endl;
    
    
    TFile *inData = new TFile(inDataName,"read");
    TFile *inMC = new TFile(inMCName,"read");
    TFile *inMC2 = new TFile(inMC2Name,"read");
    
    TH1D *h1_Mass3pi_Data = (TH1D*)inData->Get("Mass3pi");
    TH1D *h1_Mass3pi_MC = (TH1D*)inMC->Get("Mass3pi");
    TH1D *h1_Mass3pi_MC2 = (TH1D*)inMC2->Get("Mass3pi");
    
    TH1D *h1_MassPiPlusP_Data = (TH1D*)inData->Get("MassPiPlusP");
    TH1D *h1_MassPiPlusP_MC = (TH1D*)inMC->Get("MassPiPlusP");
    TH1D *h1_MassPiPlusP_MC2 = (TH1D*)inMC2->Get("MassPiPlusP");
    
    TH1D *h1_MassPiMinusP_Data = (TH1D*)inData->Get("MassPiMinusP");
    TH1D *h1_MassPiMinusP_MC = (TH1D*)inMC->Get("MassPiMinusP");
    TH1D *h1_MassPiMinusP_MC2 = (TH1D*)inMC2->Get("MassPiMinusP");
    
    TH1D *h1_BeamEnergy_Data = (TH1D*)inData->Get("BeamEnergy");
    TH1D *h1_BeamEnergy_MC = (TH1D*)inMC->Get("BeamEnergy");
    TH1D *h1_BeamEnergy_MC2 = (TH1D*)inMC2->Get("BeamEnergy");
    
    TH1D *h1_MissingMassSquared_Data = (TH1D*)inData->Get("MissingMassSquared");
    TH1D *h1_MissingMassSquared_MC = (TH1D*)inMC->Get("MissingMassSquared");
    TH1D *h1_MissingMassSquared_MC2 = (TH1D*)inMC2->Get("MissingMassSquared");
    
    TH1D *h1_Chi2NDF_Data = (TH1D*)inData->Get("Chi2NDF");
    TH1D *h1_Chi2NDF_MC = (TH1D*)inMC->Get("Chi2NDF");
    TH1D *h1_Chi2NDF_MC2 = (TH1D*)inMC2->Get("Chi2NDF");
    
    TH1D *h1_t_BCAL_Data = (TH1D*)inData->Get("t_BCAL");
    TH1D *h1_t_BCAL_MC = (TH1D*)inMC->Get("t_BCAL");
    TH1D *h1_t_BCAL_MC2 = (TH1D*)inMC2->Get("t_BCAL");
    
    TH1D *h1_t_FCAL_Data = (TH1D*)inData->Get("t_FCAL");
    TH1D *h1_t_FCAL_MC = (TH1D*)inMC->Get("t_FCAL");
    TH1D *h1_t_FCAL_MC2 = (TH1D*)inMC2->Get("t_FCAL");
    
    TH1D *h1_t_FCALBCAL_Data = (TH1D*)inData->Get("t_FCALBCAL");
    TH1D *h1_t_FCALBCAL_MC = (TH1D*)inMC->Get("t_FCALBCAL");
    TH1D *h1_t_FCALBCAL_MC2 = (TH1D*)inMC2->Get("t_FCALBCAL");
    
    TH1D *h1_ProtonP_Data = (TH1D*)inData->Get("ProtonP");
    TH1D *h1_ProtonP_MC = (TH1D*)inMC->Get("ProtonP");
    TH1D *h1_ProtonP_MC2 = (TH1D*)inMC2->Get("ProtonP");
    
    TH1D *h1_PiPlusP_Data = (TH1D*)inData->Get("PiPlusP");
    TH1D *h1_PiPlusP_MC = (TH1D*)inMC->Get("PiPlusP");
    TH1D *h1_PiPlusP_MC2 = (TH1D*)inMC2->Get("PiPlusP");
    
    TH1D *h1_Photon1P_Data = (TH1D*)inData->Get("Photon1P");
    TH1D *h1_Photon1P_MC = (TH1D*)inMC->Get("Photon1P");
    TH1D *h1_Photon1P_MC2 = (TH1D*)inMC2->Get("Photon1P");
    
    TH1D *h1_Pi0MeasuredMass_Data = (TH1D*)inData->Get("Pi0MeasuredMass");
    TH1D *h1_Pi0MeasuredMass_MC = (TH1D*)inMC->Get("Pi0MeasuredMass");
    TH1D *h1_Pi0MeasuredMass_MC2 = (TH1D*)inMC2->Get("Pi0MeasuredMass");
    
    TH1D *h1_Photon2P_vs_Photon1P_Data = (TH1D*)inData->Get("Photon2P_vs_Photon1P");
    TH1D *h1_Photon2P_vs_Photon1P_MC = (TH1D*)inMC->Get("Photon2P_vs_Photon1P");
    TH1D *h1_Photon2P_vs_Photon1P_MC2 = (TH1D*)inMC2->Get("Photon2P_vs_Photon1P");
    
    TH1D *h1_ProtondEdX_vs_P_Data = (TH1D*)inData->Get("ProtondEdX_vs_P");
    TH1D *h1_ProtondEdX_vs_P_MC = (TH1D*)inMC->Get("ProtondEdX_vs_P");
    TH1D *h1_ProtondEdX_vs_P_MC2 = (TH1D*)inMC2->Get("ProtondEdX_vs_P");
    
    TH1D *h1_PhotonTheta_BCAL_Data = (TH1D*)inData->Get("PhotonTheta_BCAL");
    TH1D *h1_PhotonTheta_BCAL_MC = (TH1D*)inMC->Get("PhotonTheta_BCAL");
    TH1D *h1_PhotonTheta_BCAL_MC2 = (TH1D*)inMC2->Get("PhotonTheta_BCAL");
    
    TH1D *h1_PhotonTheta_FCAL_Data = (TH1D*)inData->Get("PhotonTheta_FCAL");
    TH1D *h1_PhotonTheta_FCAL_MC = (TH1D*)inMC->Get("PhotonTheta_FCAL");
    TH1D *h1_PhotonTheta_FCAL_MC2 = (TH1D*)inMC2->Get("PhotonTheta_FCAL");
    
    TH1D *h1_PhotonTheta_FCALBCAL_Data = (TH1D*)inData->Get("PhotonTheta_FCALBCAL");
    TH1D *h1_PhotonTheta_FCALBCAL_MC = (TH1D*)inMC->Get("PhotonTheta_FCALBCAL");
    TH1D *h1_PhotonTheta_FCALBCAL_MC2 = (TH1D*)inMC2->Get("PhotonTheta_FCALBCAL");
    
    TH1D *h1_Pi0Theta_FCALBCAL_Data = (TH1D*)inData->Get("Pi0Theta_FCALBCAL");
    TH1D *h1_Pi0Theta_FCALBCAL_MC = (TH1D*)inMC->Get("Pi0Theta_FCALBCAL");
    TH1D *h1_Pi0Theta_FCALBCAL_MC2 = (TH1D*)inMC2->Get("Pi0Theta_FCALBCAL");
    
    TH1D *h1_PiPlusTheta_vs_P_Data = (TH1D*)inData->Get("PiPlusTheta_vs_P");
    TH1D *h1_PiPlusTheta_vs_P_MC = (TH1D*)inMC->Get("PiPlusTheta_vs_P");
    TH1D *h1_PiPlusTheta_vs_P_MC2 = (TH1D*)inMC2->Get("PiPlusTheta_vs_P");
    
    TH1D *h1_Photon1Theta_vs_E_BCAL_Data = (TH1D*)inData->Get("Photon1Theta_vs_E_BCAL");
    TH1D *h1_Photon1Theta_vs_E_BCAL_MC = (TH1D*)inMC->Get("Photon1Theta_vs_E_BCAL");
    TH1D *h1_Photon1Theta_vs_E_BCAL_MC2 = (TH1D*)inMC2->Get("Photon1Theta_vs_E_BCAL");
    
    TH1D *h1_Photon1Theta_vs_E_FCAL_Data = (TH1D*)inData->Get("Photon1Theta_vs_E_FCAL");
    TH1D *h1_Photon1Theta_vs_E_FCAL_MC = (TH1D*)inMC->Get("Photon1Theta_vs_E_FCAL");
    TH1D *h1_Photon1Theta_vs_E_FCAL_MC2 = (TH1D*)inMC2->Get("Photon1Theta_vs_E_FCAL");
    
    TH1D *h1_Photon1Theta_vs_E_FCALBCAL_Data = (TH1D*)inData->Get("Photon1Theta_vs_E_FCALBCAL");
    TH1D *h1_Photon1Theta_vs_E_FCALBCAL_MC = (TH1D*)inMC->Get("Photon1Theta_vs_E_FCALBCAL");
    TH1D *h1_Photon1Theta_vs_E_FCALBCAL_MC2 = (TH1D*)inMC2->Get("Photon1Theta_vs_E_FCALBCAL");
    
    // gPad->SetLogy();
    
    Double_t xmin=0.;
    Double_t xmax=2.0;
    Double_t ymin=0;
    Double_t ymax=1.5;
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1000,700);
    c0->Divide(2,2);
    
    TString text;
    text.Form("Normalized to E=%.1f-%.1f GeV",xmin,xmax);
    TLatex *t1 = new TLatex(0.3,0.45,text);
    
    c0->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_Mass3pi_Data->SetTitle("");
    // h1_Mass3pi_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Mass3pi_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Mass3pi_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Mass3pi_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Mass3pi_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_Mass3pi_Data->GetXaxis()->SetTitle("M_{3#pi} (GeV)");
    h1_Mass3pi_Data->SetLineColor(2);
    h1_Mass3pi_Data->SetMarkerColor(2);
    h1_Mass3pi_Data->SetMarkerStyle(20);
    h1_Mass3pi_Data->SetMarkerSize(0.5);
    h1_Mass3pi_Data->Fit("gaus","","",0.9,2.5);
    TAxis *xaxis = h1_Mass3pi_Data->GetXaxis();
    Int_t binx1 = xaxis->FindBin(0.9);          // normalize to coherent peak
    Int_t binx2 = xaxis->FindBin(1.5);
    h1_Mass3pi_Data->Draw("");
    
    h1_Mass3pi_MC->SetLineColor(4);
    h1_Mass3pi_MC->SetMarkerColor(4);
    h1_Mass3pi_MC->SetMarkerStyle(20);
    h1_Mass3pi_MC->SetMarkerSize(0.5);
    Double_t sData = h1_Mass3pi_Data->Integral(binx1,binx2-1);    // ignore overflows
    Double_t sMC = h1_Mass3pi_MC->Integral(binx1,binx2-1);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_Mass3pi_MC->Scale(sData/sMC);
    h1_Mass3pi_MC->Draw("psame");
    
    h1_Mass3pi_MC2->SetLineColor(1);
    h1_Mass3pi_MC2->SetMarkerColor(1);
    h1_Mass3pi_MC2->SetMarkerStyle(20);
    h1_Mass3pi_MC2->SetMarkerSize(0.5);
    Double_t sMC2 = h1_Mass3pi_MC->Integral(binx1,binx2-1);
    h1_Mass3pi_MC2->Scale(sData/sMC);
    h1_Mass3pi_MC2->Draw("psame");
    
    
    TLegend *leg = new TLegend(0.55,0.68,0.65,0.83);
    leg->AddEntry(h1_Mass3pi_Data,"Data","p");
    leg->AddEntry(h1_Mass3pi_MC,"MC","p");
    leg->AddEntry(h1_Mass3pi_MC2,"MC2","p");
    leg->Draw();
    
    
    c0->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_BeamEnergy_Data->SetTitle("");
    // h1_BeamEnergy_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_BeamEnergy_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_BeamEnergy_Data->GetXaxis()->SetTitleSize(0.05);
    h1_BeamEnergy_Data->GetYaxis()->SetTitleSize(0.05);
    h1_BeamEnergy_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_BeamEnergy_Data->GetXaxis()->SetTitle("Beam Energy (GeV)");
    h1_BeamEnergy_Data->SetLineColor(2);
    h1_BeamEnergy_Data->SetMarkerColor(2);
    h1_BeamEnergy_Data->SetMarkerStyle(20);
    h1_BeamEnergy_Data->SetMarkerSize(0.5);
    xaxis = h1_BeamEnergy_Data->GetXaxis();
    binx1 = xaxis->FindBin(7.5);          // normalize to coherent peak
    binx2 = xaxis->FindBin(8.5);
    h1_BeamEnergy_Data->Draw("");
    
    h1_BeamEnergy_MC->SetLineColor(4);
    h1_BeamEnergy_MC->SetMarkerColor(4);
    h1_BeamEnergy_MC->SetMarkerStyle(20);
    h1_BeamEnergy_MC->SetMarkerSize(0.5);
    sData = h1_BeamEnergy_Data->Integral(binx1,binx2);
    sMC = h1_BeamEnergy_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_BeamEnergy_MC->Scale(sData/sMC);
    h1_BeamEnergy_MC->Draw("psame");
    
    h1_BeamEnergy_MC2->SetLineColor(1);
    h1_BeamEnergy_MC2->SetMarkerColor(1);
    h1_BeamEnergy_MC2->SetMarkerStyle(20);
    h1_BeamEnergy_MC2->SetMarkerSize(0.5);
    sMC2 = h1_BeamEnergy_MC2->Integral(binx1,binx2);
    h1_BeamEnergy_MC2->Scale(sData/sMC2);
    h1_BeamEnergy_MC2->Draw("psame");
    
    leg->Draw();
    
    c0->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_MissingMassSquared_MC->SetTitle("");
    // h1_MissingMassSquared_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_MissingMassSquared_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_MissingMassSquared_MC->GetXaxis()->SetTitleSize(0.05);
    h1_MissingMassSquared_MC->GetYaxis()->SetTitleSize(0.05);
    h1_MissingMassSquared_MC->GetYaxis()->SetTitle("Scaled Counts");
    h1_MissingMassSquared_MC->GetXaxis()->SetTitle("Missing Mass Squared (GeV^{2})");
    h1_MissingMassSquared_MC->SetLineColor(2);
    h1_MissingMassSquared_MC->SetMarkerColor(2);
    h1_MissingMassSquared_MC->SetMarkerStyle(20);
    h1_MissingMassSquared_MC->SetMarkerSize(0.5);
    h1_MissingMassSquared_MC->Draw("");
    
    h1_MissingMassSquared_Data->SetLineColor(4);
    h1_MissingMassSquared_Data->SetMarkerColor(4);
    h1_MissingMassSquared_Data->SetMarkerStyle(20);
    h1_MissingMassSquared_Data->SetMarkerSize(0.5);
    h1_MissingMassSquared_Data->Scale(h1_MissingMassSquared_MC->Integral()/h1_MissingMassSquared_Data->Integral());
    h1_MissingMassSquared_Data->Draw("psame");
    
    h1_MissingMassSquared_MC2->SetLineColor(1);
    h1_MissingMassSquared_MC2->SetMarkerColor(1);
    h1_MissingMassSquared_MC2->SetMarkerStyle(20);
    h1_MissingMassSquared_MC2->SetMarkerSize(0.5);
    h1_MissingMassSquared_MC2->Scale(h1_MissingMassSquared_Data->Integral()/h1_MissingMassSquared_MC2->Integral());
    h1_MissingMassSquared_MC2->Draw("psame");
    
    leg->Draw();
    
    c0->cd(4);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_Chi2NDF_MC->SetTitle("");
    // h1_Chi2NDF_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Chi2NDF_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Chi2NDF_MC->GetXaxis()->SetTitleSize(0.05);
    h1_Chi2NDF_MC->GetYaxis()->SetTitleSize(0.05);
    h1_Chi2NDF_MC->GetYaxis()->SetTitle("Scaled Counts");
    h1_Chi2NDF_MC->GetXaxis()->SetTitle("#chi^{2}/NDF");
    h1_Chi2NDF_MC->SetLineColor(4);
    h1_Chi2NDF_MC->SetMarkerColor(4);
    h1_Chi2NDF_MC->SetMarkerStyle(20);
    h1_Chi2NDF_MC->SetMarkerSize(0.5);
    h1_Chi2NDF_MC->Draw("");
    
    h1_Chi2NDF_Data->SetLineColor(2);
    h1_Chi2NDF_Data->SetMarkerColor(2);
    h1_Chi2NDF_Data->SetMarkerStyle(20);
    h1_Chi2NDF_Data->SetMarkerSize(0.5);
    h1_Chi2NDF_Data->Scale(h1_Chi2NDF_MC->Integral()/h1_Chi2NDF_Data->Integral());
    h1_Chi2NDF_Data->Draw("psame");
    
    h1_Chi2NDF_MC2->SetLineColor(1);
    h1_Chi2NDF_MC2->SetMarkerColor(1);
    h1_Chi2NDF_MC2->SetMarkerStyle(20);
    h1_Chi2NDF_MC2->SetMarkerSize(0.5);
    h1_Chi2NDF_MC2->Scale(h1_Chi2NDF_Data->Integral()/h1_Chi2NDF_MC2->Integral());
    h1_Chi2NDF_MC2->Draw("psame");
    
    leg->Draw();
    
    
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,1000,700);
    c1->Divide(2,2);
    
    c1->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogy();
    h1_t_BCAL_Data->SetTitle("");
    // h1_t_BCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_t_BCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_t_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_t_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_t_BCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_t_BCAL_Data->GetXaxis()->SetTitle("BCAL -t (GeV)");
    h1_t_BCAL_Data->SetLineColor(2);
    h1_t_BCAL_Data->SetMarkerColor(2);
    h1_t_BCAL_Data->SetMarkerStyle(20);
    h1_t_BCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_t_BCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(0.2);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(1.0);
    Double_t fitmin=0.2;
    Double_t fitmax=1.5;
    h1_t_BCAL_Data->Fit("expo","","",fitmin,fitmax);
    h1_t_BCAL_Data->Draw("");
    
    h1_t_BCAL_MC->SetLineColor(4);
    h1_t_BCAL_MC->SetMarkerColor(4);
    h1_t_BCAL_MC->SetMarkerStyle(20);
    h1_t_BCAL_MC->SetMarkerSize(0.5);
    sData = h1_t_BCAL_Data->Integral(binx1,binx2);
    sMC = h1_t_BCAL_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_t_BCAL_MC->Scale(sData/sMC);
    // h1_t_BCAL_MC->Fit("expo","","",fitmin,fitmax);
    h1_t_BCAL_MC->Draw("psame");
    
    leg->Draw();
    
    c1->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogy();
    h1_t_FCAL_Data->SetTitle("");
    // h1_t_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_t_FCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_t_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_t_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_t_FCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_t_FCAL_Data->GetXaxis()->SetTitle("FCAL -t (GeV)");
    h1_t_FCAL_Data->SetLineColor(2);
    h1_t_FCAL_Data->SetMarkerColor(2);
    h1_t_FCAL_Data->SetMarkerStyle(20);
    h1_t_FCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_t_FCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(0.2);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(1.0);
    h1_t_FCAL_Data->Fit("expo","","",fitmin,fitmax);
    h1_t_FCAL_Data->Draw("");
    
    /*h1_t_FCAL_MC->SetLineColor(4);
    h1_t_FCAL_MC->SetMarkerColor(4);
    h1_t_FCAL_MC->SetMarkerStyle(20);
    h1_t_FCAL_MC->SetMarkerSize(0.5);
    h1_t_FCAL_MC->Scale(h1_t_FCAL_Data->Integral()/h1_t_FCAL_MC->Integral());
    h1_t_FCAL_MC->Draw("psame");*/
    
    h1_t_FCAL_MC2->SetLineColor(1);
    h1_t_FCAL_MC2->SetMarkerColor(1);
    h1_t_FCAL_MC2->SetMarkerStyle(20);
    h1_t_FCAL_MC2->SetMarkerSize(0.5);
    sData = h1_t_FCAL_Data->Integral(binx1,binx2);
    sMC2 = h1_t_FCAL_MC2->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_t_FCAL_MC2->Scale(sData/sMC2);
    // h1_t_FCAL_MC2->Fit("expo","","",fitmin,fitmax);
    h1_t_FCAL_MC2->Draw("psame");
    
    leg->Draw();
    
    c1->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogy();
    h1_t_FCALBCAL_Data->SetTitle("");
    // h1_t_FCALBCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_t_FCALBCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_t_FCALBCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_t_FCALBCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_t_FCALBCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_t_FCALBCAL_Data->GetXaxis()->SetTitle("FCALBCAL -t (GeV)");
    h1_t_FCALBCAL_Data->SetLineColor(2);
    h1_t_FCALBCAL_Data->SetMarkerColor(2);
    h1_t_FCALBCAL_Data->SetMarkerStyle(20);
    h1_t_FCALBCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_t_FCALBCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(0.2);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(1.0);
    h1_t_FCALBCAL_Data->Fit("expo","","",fitmin,fitmax);
    h1_t_FCALBCAL_Data->Draw("");
    
    h1_t_FCALBCAL_MC->SetLineColor(4);
    h1_t_FCALBCAL_MC->SetMarkerColor(4);
    h1_t_FCALBCAL_MC->SetMarkerStyle(20);
    h1_t_FCALBCAL_MC->SetMarkerSize(0.5);
    sData = h1_t_FCALBCAL_Data->Integral(binx1,binx2);
    sMC = h1_t_FCALBCAL_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_t_FCALBCAL_MC->Scale(sData/sMC);
    // h1_t_FCALBCAL_MC->Fit("expo","","",fitmin,fitmax);
    h1_t_FCALBCAL_MC->Draw("psame");
    
    /*h1_t_FCALBCAL_MC2->SetLineColor(1);
     h1_t_FCALBCAL_MC2->SetMarkerColor(1);
     h1_t_FCALBCAL_MC2->SetMarkerStyle(20);
     h1_t_FCALBCAL_MC2->SetMarkerSize(0.5);
     h1_t_FCALBCAL_MC2->Scale(h1_t_FCALBCAL_Data->Integral()/h1_t_FCALBCAL_MC2->Integral());
     h1_t_FCALBCAL_MC2->Draw("psame");*/
    
    leg->Draw();
    
    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1000,700);
    c2->Divide(3,2);
    
    c2->cd(1);
    h1_ProtonP_Data->SetTitle("");
    // h1_ProtonP_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_ProtonP_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_ProtonP_Data->GetXaxis()->SetTitleSize(0.05);
    h1_ProtonP_Data->GetYaxis()->SetTitleSize(0.05);
    h1_ProtonP_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_ProtonP_Data->GetXaxis()->SetTitle("Proton P (GeV)");
    h1_ProtonP_Data->SetLineColor(2);
    h1_ProtonP_Data->SetMarkerColor(2);
    h1_ProtonP_Data->SetMarkerStyle(20);
    h1_ProtonP_Data->SetMarkerSize(0.5);
    xaxis = h1_ProtonP_Data->GetXaxis();
    binx1 = xaxis->FindBin(0.5);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(1.0);
    h1_ProtonP_Data->Draw("");
    
    h1_ProtonP_MC->SetLineColor(4);
    h1_ProtonP_MC->SetMarkerColor(4);
    h1_ProtonP_MC->SetMarkerStyle(20);
    h1_ProtonP_MC->SetMarkerSize(0.5);
    sData = h1_ProtonP_Data->Integral(binx1,binx2);
    sMC = h1_ProtonP_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_ProtonP_MC->Scale(sData/sMC);
    h1_ProtonP_MC->Draw("psame");
 
    leg->Draw();
    
    c2->cd(2);
    h1_PiPlusP_Data->SetTitle("");
    // h1_PiPlusP_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_PiPlusP_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_PiPlusP_Data->GetXaxis()->SetTitleSize(0.05);
    h1_PiPlusP_Data->GetYaxis()->SetTitleSize(0.05);
    h1_PiPlusP_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_PiPlusP_Data->GetXaxis()->SetTitle("#pi^{+} P (GeV)");
    h1_PiPlusP_Data->SetLineColor(2);
    h1_PiPlusP_Data->SetMarkerColor(2);
    h1_PiPlusP_Data->SetMarkerStyle(20);
    h1_PiPlusP_Data->SetMarkerSize(0.5);
    xaxis = h1_PiPlusP_Data->GetXaxis();
    binx1 = xaxis->FindBin(1.0);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(2.0);
    h1_PiPlusP_Data->Draw("");
    
    h1_PiPlusP_MC->SetLineColor(4);
    h1_PiPlusP_MC->SetMarkerColor(4);
    h1_PiPlusP_MC->SetMarkerStyle(20);
    h1_PiPlusP_MC->SetMarkerSize(0.5);
    sData = h1_PiPlusP_Data->Integral(binx1,binx2);
    sMC = h1_PiPlusP_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_PiPlusP_MC->Scale(sData/sMC);
    h1_PiPlusP_MC->Draw("psame");
    
    leg->Draw();
    
    c2->cd(3);
    h1_Photon1P_Data->SetTitle("");
    // h1_Photon1P_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1P_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1P_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1P_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_Photon1P_Data->GetXaxis()->SetTitle("Photon 1 P (GeV)");
    h1_Photon1P_Data->SetLineColor(2);
    h1_Photon1P_Data->SetMarkerColor(2);
    h1_Photon1P_Data->SetMarkerStyle(20);
    h1_Photon1P_Data->SetMarkerSize(0.5);
    xaxis = h1_Photon1P_Data->GetXaxis();
    binx1 = xaxis->FindBin(1.0);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(2.0);
    h1_Photon1P_Data->Draw("");
    
    h1_Photon1P_MC->SetLineColor(4);
    h1_Photon1P_MC->SetMarkerColor(4);
    h1_Photon1P_MC->SetMarkerStyle(20);
    h1_Photon1P_MC->SetMarkerSize(0.5);
    sData = h1_Photon1P_Data->Integral(binx1,binx2);
    sMC = h1_Photon1P_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_Photon1P_MC->Scale(sData/sMC);
    h1_Photon1P_MC->Draw("psame");
    
    leg->Draw();
    
    c2->cd(4);
    h1_Pi0MeasuredMass_Data->SetTitle("");
    // h1_Pi0MeasuredMass_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Pi0MeasuredMass_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Pi0MeasuredMass_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Pi0MeasuredMass_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Pi0MeasuredMass_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_Pi0MeasuredMass_Data->GetXaxis()->SetTitle("Pi0 Measured Mass (GeV)");
    h1_Pi0MeasuredMass_Data->SetLineColor(2);
    h1_Pi0MeasuredMass_Data->SetMarkerColor(2);
    h1_Pi0MeasuredMass_Data->SetMarkerStyle(20);
    h1_Pi0MeasuredMass_Data->SetMarkerSize(0.5);
    xaxis = h1_Pi0MeasuredMass_Data->GetXaxis();
    binx1 = xaxis->FindBin(0.05);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(0.2);
    h1_Pi0MeasuredMass_Data->Draw("");
    
    h1_Pi0MeasuredMass_MC->SetLineColor(4);
    h1_Pi0MeasuredMass_MC->SetMarkerColor(4);
    h1_Pi0MeasuredMass_MC->SetMarkerStyle(20);
    h1_Pi0MeasuredMass_MC->SetMarkerSize(0.5);
    sData = h1_Pi0MeasuredMass_Data->Integral(binx1,binx2);
    sMC = h1_Pi0MeasuredMass_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_Pi0MeasuredMass_MC->Scale(0.8*sData/sMC);
    h1_Pi0MeasuredMass_MC->Draw("psame");
    
    leg->Draw();
    
    c2->cd(5);
    h1_Photon2P_vs_Photon1P_Data->SetTitle(run+" DATA");
    // h1_Photon2P_vs_Photon1P_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon2P_vs_Photon1P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon2P_vs_Photon1P_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Photon2P_vs_Photon1P_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Photon2P_vs_Photon1P_Data->GetYaxis()->SetTitle("Photon 1 E (GeV)");
    h1_Photon2P_vs_Photon1P_Data->GetXaxis()->SetTitle("Photon 2 E (GeV)");
    h1_Photon2P_vs_Photon1P_Data->Draw("colz");
    
    c2->cd(6);
    h1_Photon2P_vs_Photon1P_MC->SetTitle(run+" MC");
    // h1_Photon2P_vs_Photon1P_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon2P_vs_Photon1P_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon2P_vs_Photon1P_MC->GetXaxis()->SetTitleSize(0.05);
    h1_Photon2P_vs_Photon1P_MC->GetYaxis()->SetTitleSize(0.05);
    h1_Photon2P_vs_Photon1P_MC->GetYaxis()->SetTitle("Photon 1 E (GeV)");
    h1_Photon2P_vs_Photon1P_MC->GetXaxis()->SetTitle("Photon 2 E (GeV)");
    h1_Photon2P_vs_Photon1P_MC->Draw("colz");
    
    TCanvas *c3 = new TCanvas("c3", "c3",200,10,1000,700);
    c3->Divide(3,2);
    
    c3->cd(1);
    h1_ProtondEdX_vs_P_Data->SetTitle(run+" DATA");
    // h1_ProtondEdX_vs_P_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_ProtondEdX_vs_P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_ProtondEdX_vs_P_Data->GetXaxis()->SetTitleSize(0.05);
    h1_ProtondEdX_vs_P_Data->GetYaxis()->SetTitleSize(0.05);
    h1_ProtondEdX_vs_P_Data->GetYaxis()->SetTitle("Proton dEdX");
    h1_ProtondEdX_vs_P_Data->GetXaxis()->SetTitle("P (GeV)");
    h1_ProtondEdX_vs_P_Data->Draw("colz");
    
    c3->cd(2);
    h1_ProtondEdX_vs_P_MC->SetTitle(run+" MC");
    // h1_ProtondEdX_vs_P_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_ProtondEdX_vs_P_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_ProtondEdX_vs_P_MC->GetXaxis()->SetTitleSize(0.05);
    h1_ProtondEdX_vs_P_MC->GetYaxis()->SetTitleSize(0.05);
    h1_ProtondEdX_vs_P_MC->GetYaxis()->SetTitle("Proton dEdX");
    h1_ProtondEdX_vs_P_MC->GetXaxis()->SetTitle("P (GeV)");
    h1_ProtondEdX_vs_P_MC->Draw("colz");
    
    c3->cd(3);
    h1_PhotonTheta_BCAL_Data->SetTitle("");
    // h1_PhotonTheta_BCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_PhotonTheta_BCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_PhotonTheta_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_PhotonTheta_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_PhotonTheta_BCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_PhotonTheta_BCAL_Data->GetXaxis()->SetTitle("Photon Theta BCAL (deg)");
    h1_PhotonTheta_BCAL_Data->SetLineColor(2);
    h1_PhotonTheta_BCAL_Data->SetMarkerColor(2);
    h1_PhotonTheta_BCAL_Data->SetMarkerStyle(20);
    h1_PhotonTheta_BCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_PhotonTheta_BCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(13);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(17);
    h1_PhotonTheta_BCAL_Data->Draw("");
    
    h1_PhotonTheta_BCAL_MC->SetLineColor(4);
    h1_PhotonTheta_BCAL_MC->SetMarkerColor(4);
    h1_PhotonTheta_BCAL_MC->SetMarkerStyle(20);
    h1_PhotonTheta_BCAL_MC->SetMarkerSize(0.5);
    sData = h1_PhotonTheta_BCAL_Data->Integral(binx1,binx2);
    sMC = h1_PhotonTheta_BCAL_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_PhotonTheta_BCAL_MC->Scale(sData/sMC);
    h1_PhotonTheta_BCAL_MC->Draw("psame");
    
    leg->Draw();
    
    c3->cd(4);
    h1_PhotonTheta_FCAL_Data->SetTitle("");
    // h1_PhotonTheta_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_PhotonTheta_FCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_PhotonTheta_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_PhotonTheta_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_PhotonTheta_FCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_PhotonTheta_FCAL_Data->GetXaxis()->SetTitle("Photon Theta FCAL (deg)");
    h1_PhotonTheta_FCAL_Data->SetLineColor(2);
    h1_PhotonTheta_FCAL_Data->SetMarkerColor(2);
    h1_PhotonTheta_FCAL_Data->SetMarkerStyle(20);
    h1_PhotonTheta_FCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_PhotonTheta_FCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(2);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(8);
    h1_PhotonTheta_FCAL_Data->Draw("");
    
    h1_PhotonTheta_FCAL_MC->SetLineColor(4);
    h1_PhotonTheta_FCAL_MC->SetMarkerColor(4);
    h1_PhotonTheta_FCAL_MC->SetMarkerStyle(20);
    h1_PhotonTheta_FCAL_MC->SetMarkerSize(0.5);
    sData = h1_PhotonTheta_FCAL_Data->Integral(binx1,binx2);
    sMC = h1_PhotonTheta_FCAL_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_PhotonTheta_FCAL_MC->Scale(sData/sMC);
    h1_PhotonTheta_FCAL_MC->Draw("psame");
    
    leg->Draw();
    c3->cd(5);
    h1_PhotonTheta_FCALBCAL_Data->SetTitle("");
    // h1_PhotonTheta_FCALBCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_PhotonTheta_FCALBCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_PhotonTheta_FCALBCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_PhotonTheta_FCALBCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_PhotonTheta_FCALBCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_PhotonTheta_FCALBCAL_Data->GetXaxis()->SetTitle("Photon Theta FCALBCAL (deg)");
    h1_PhotonTheta_FCALBCAL_Data->SetLineColor(2);
    h1_PhotonTheta_FCALBCAL_Data->SetMarkerColor(2);
    h1_PhotonTheta_FCALBCAL_Data->SetMarkerStyle(20);
    h1_PhotonTheta_FCALBCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_PhotonTheta_FCALBCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(2);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(8);
    h1_PhotonTheta_FCALBCAL_Data->Draw("");
    
    h1_PhotonTheta_FCALBCAL_MC->SetLineColor(4);
    h1_PhotonTheta_FCALBCAL_MC->SetMarkerColor(4);
    h1_PhotonTheta_FCALBCAL_MC->SetMarkerStyle(20);
    h1_PhotonTheta_FCALBCAL_MC->SetMarkerSize(0.5);
    sData = h1_PhotonTheta_FCALBCAL_Data->Integral(binx1,binx2);
    sMC = h1_PhotonTheta_FCALBCAL_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_PhotonTheta_FCALBCAL_MC->Scale(sData/sMC);
    h1_PhotonTheta_FCALBCAL_MC->Draw("psame");
    
    leg->Draw();
    
    c3->cd(6);
    h1_Pi0Theta_FCALBCAL_Data->SetTitle("");
    // h1_Pi0Theta_FCALBCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Pi0Theta_FCALBCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Pi0Theta_FCALBCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Pi0Theta_FCALBCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Pi0Theta_FCALBCAL_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_Pi0Theta_FCALBCAL_Data->GetXaxis()->SetTitle("Pi0 Theta FCALBCAL (deg)");
    h1_Pi0Theta_FCALBCAL_Data->SetLineColor(2);
    h1_Pi0Theta_FCALBCAL_Data->SetMarkerColor(2);
    h1_Pi0Theta_FCALBCAL_Data->SetMarkerStyle(20);
    h1_Pi0Theta_FCALBCAL_Data->SetMarkerSize(0.5);
    xaxis = h1_Pi0Theta_FCALBCAL_Data->GetXaxis();
    binx1 = xaxis->FindBin(5);          // normalize to 0.2 to 1.0
    binx2 = xaxis->FindBin(15);
    h1_Pi0Theta_FCALBCAL_Data->Draw("");
    
    h1_Pi0Theta_FCALBCAL_MC->SetLineColor(4);
    h1_Pi0Theta_FCALBCAL_MC->SetMarkerColor(4);
    h1_Pi0Theta_FCALBCAL_MC->SetMarkerStyle(20);
    h1_Pi0Theta_FCALBCAL_MC->SetMarkerSize(0.5);
    sData = h1_Pi0Theta_FCALBCAL_Data->Integral(binx1,binx2);
    sMC = h1_Pi0Theta_FCALBCAL_MC->Integral(binx1,binx2);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_Pi0Theta_FCALBCAL_MC->Scale(sData/sMC);
    h1_Pi0Theta_FCALBCAL_MC->Draw("psame");
    
    leg->Draw();
    
    
    TCanvas *c4 = new TCanvas("c4", "c4",200,10,1000,700);
    c4->Divide(3,2);
    
    c4->cd(1);
    h1_PiPlusTheta_vs_P_Data->SetTitle(run+" DATA");
    // h1_PiPlusTheta_vs_P_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_PiPlusTheta_vs_P_vs_P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_PiPlusTheta_vs_P_Data->GetXaxis()->SetTitleSize(0.05);
    h1_PiPlusTheta_vs_P_Data->GetYaxis()->SetTitleSize(0.05);
    h1_PiPlusTheta_vs_P_Data->GetYaxis()->SetTitle("Theta (deg)");
    h1_PiPlusTheta_vs_P_Data->GetXaxis()->SetTitle("PiPlus P (GeV)");
    h1_PiPlusTheta_vs_P_Data->Draw("colz");
    
    c4->cd(4);
    h1_PiPlusTheta_vs_P_MC->SetTitle(run+" MC");
    // h1_PiPlusTheta_vs_P_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_PiPlusTheta_vs_P_vs_P_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_PiPlusTheta_vs_P_MC->GetXaxis()->SetTitleSize(0.05);
    h1_PiPlusTheta_vs_P_MC->GetYaxis()->SetTitleSize(0.05);
    h1_PiPlusTheta_vs_P_MC->GetYaxis()->SetTitle("Theta (deg)");
    h1_PiPlusTheta_vs_P_MC->GetXaxis()->SetTitle("PiPlus P (GeV)");
    h1_PiPlusTheta_vs_P_MC->Draw("colz");
    
    c4->cd(2);
    h1_Photon1Theta_vs_E_BCAL_Data->SetTitle(run+" DATA");
    // h1_Photon1Theta_vs_E_BCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1Theta_vs_E_BCAL_vs_P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1Theta_vs_E_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_BCAL_Data->GetYaxis()->SetTitle("Theta (deg)");
    h1_Photon1Theta_vs_E_BCAL_Data->GetXaxis()->SetTitle("Photon1 E BCAL (GeV)");
    h1_Photon1Theta_vs_E_BCAL_Data->Draw("colz");
    
    c4->cd(5);
    h1_Photon1Theta_vs_E_BCAL_MC->SetTitle(run+" MC");
    // h1_Photon1Theta_vs_E_BCAL_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1Theta_vs_E_BCAL_vs_P_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1Theta_vs_E_BCAL_MC->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_BCAL_MC->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_BCAL_MC->GetYaxis()->SetTitle("Theta (deg)");
    h1_Photon1Theta_vs_E_BCAL_MC->GetXaxis()->SetTitle("Photon1 E BCAL (GeV)");
    h1_Photon1Theta_vs_E_BCAL_MC->Draw("colz");
    
    c4->cd(3);
    h1_Photon1Theta_vs_E_FCAL_Data->SetTitle(run+" DATA");
    // h1_Photon1Theta_vs_E_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1Theta_vs_E_FCAL_vs_P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1Theta_vs_E_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCAL_Data->GetYaxis()->SetTitle("Theta (deg)");
    h1_Photon1Theta_vs_E_FCAL_Data->GetXaxis()->SetTitle("Photon1 E FCAL (GeV)");
    h1_Photon1Theta_vs_E_FCAL_Data->Draw("colz");
    
    c4->cd(6);
    h1_Photon1Theta_vs_E_FCAL_MC->SetTitle(run+" MC");
    // h1_Photon1Theta_vs_E_FCAL_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1Theta_vs_E_FCAL_vs_P_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1Theta_vs_E_FCAL_MC->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCAL_MC->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCAL_MC->GetYaxis()->SetTitle("Theta (deg)");
    h1_Photon1Theta_vs_E_FCAL_MC->GetXaxis()->SetTitle("Photon1 E FCAL (GeV)");
    h1_Photon1Theta_vs_E_FCAL_MC->Draw("colz");
    
    TCanvas *c5 = new TCanvas("c5", "c5",200,10,1000,700);
    c5->Divide(3,2);
    
    c5->cd(1);
    h1_Photon1Theta_vs_E_FCALBCAL_Data->SetTitle(run+" DATA");
    // h1_Photon1Theta_vs_E_FCALBCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1Theta_vs_E_FCALBCAL_vs_P_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1Theta_vs_E_FCALBCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCALBCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCALBCAL_Data->GetYaxis()->SetTitle("Theta (deg)");
    h1_Photon1Theta_vs_E_FCALBCAL_Data->GetXaxis()->SetTitle("Photon1 E FCALBCAL (GeV)");
    h1_Photon1Theta_vs_E_FCALBCAL_Data->Draw("colz");
    
    c5->cd(4);
    h1_Photon1Theta_vs_E_FCALBCAL_MC->SetTitle(run+" MC");
    // h1_Photon1Theta_vs_E_FCALBCAL_MC->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_Photon1Theta_vs_E_FCALBCAL_vs_P_MC->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_Photon1Theta_vs_E_FCALBCAL_MC->GetXaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCALBCAL_MC->GetYaxis()->SetTitleSize(0.05);
    h1_Photon1Theta_vs_E_FCALBCAL_MC->GetYaxis()->SetTitle("Theta (deg)");
    h1_Photon1Theta_vs_E_FCALBCAL_MC->GetXaxis()->SetTitle("Photon1 E FCALBCAL (GeV)");
    h1_Photon1Theta_vs_E_FCALBCAL_MC->Draw("colz");
    
    c5->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_MassPiPlusP_Data->SetTitle("");
    // h1_MassPiPlusP_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_MassPiPlusP_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_MassPiPlusP_Data->GetXaxis()->SetTitleSize(0.05);
    h1_MassPiPlusP_Data->GetYaxis()->SetTitleSize(0.05);
    h1_MassPiPlusP_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_MassPiPlusP_Data->GetXaxis()->SetTitle("M(#pi^{+} p) (GeV)");
    h1_MassPiPlusP_Data->SetLineColor(2);
    h1_MassPiPlusP_Data->SetMarkerColor(2);
    h1_MassPiPlusP_Data->SetMarkerStyle(20);
    h1_MassPiPlusP_Data->SetMarkerSize(0.5);
    // h1_MassPiPlusP_Data->Fit("gaus","","",0.9,1.5);
    xaxis = h1_MassPiPlusP_Data->GetXaxis();
    binx1 = xaxis->FindBin(1.5);          // normalize to coherent peak
    binx2 = xaxis->FindBin(2.5);
    h1_MassPiPlusP_Data->Draw("");
    
    h1_MassPiPlusP_MC->SetLineColor(4);
    h1_MassPiPlusP_MC->SetMarkerColor(4);
    h1_MassPiPlusP_MC->SetMarkerStyle(20);
    h1_MassPiPlusP_MC->SetMarkerSize(0.5);
    sData = h1_MassPiPlusP_Data->Integral(binx1,binx2-1);    // ignore overflows
    sMC = h1_MassPiPlusP_MC->Integral(binx1,binx2-1);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_MassPiPlusP_MC->Scale(sData/sMC);
    h1_MassPiPlusP_MC->Draw("psame");
    
    h1_MassPiPlusP_MC2->SetLineColor(1);
    h1_MassPiPlusP_MC2->SetMarkerColor(1);
    h1_MassPiPlusP_MC2->SetMarkerStyle(20);
    h1_MassPiPlusP_MC2->SetMarkerSize(0.5);
    sMC2 = h1_MassPiPlusP_MC->Integral(binx1,binx2-1);
    h1_MassPiPlusP_MC2->Scale(sData/sMC);
    h1_MassPiPlusP_MC2->Draw("psame");
    
    c5->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_MassPiMinusP_Data->SetTitle("");
    // h1_MassPiMinusP_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    // h1_MassPiMinusP_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_MassPiMinusP_Data->GetXaxis()->SetTitleSize(0.05);
    h1_MassPiMinusP_Data->GetYaxis()->SetTitleSize(0.05);
    h1_MassPiMinusP_Data->GetYaxis()->SetTitle("Scaled Counts");
    h1_MassPiMinusP_Data->GetXaxis()->SetTitle("M(#pi^{-} p) (GeV)");
    h1_MassPiMinusP_Data->SetLineColor(2);
    h1_MassPiMinusP_Data->SetMarkerColor(2);
    h1_MassPiMinusP_Data->SetMarkerStyle(20);
    h1_MassPiMinusP_Data->SetMarkerSize(0.5);
    // h1_MassPiMinusP_Data->Fit("gaus","","",0.9,1.5);
    xaxis = h1_MassPiMinusP_Data->GetXaxis();
    binx1 = xaxis->FindBin(1.5);          // normalize to coherent peak
    binx2 = xaxis->FindBin(2.5);
    h1_MassPiMinusP_Data->Draw("");
    
    h1_MassPiMinusP_MC->SetLineColor(4);
    h1_MassPiMinusP_MC->SetMarkerColor(4);
    h1_MassPiMinusP_MC->SetMarkerStyle(20);
    h1_MassPiMinusP_MC->SetMarkerSize(0.5);
    sData = h1_MassPiMinusP_Data->Integral(binx1,binx2-1);    // ignore overflows
    sMC = h1_MassPiMinusP_MC->Integral(binx1,binx2-1);
    cout << " binx1=" << binx1 << " binx2=" << binx2 << " intData=" << sData << " intMC=" << sMC << endl;
    h1_MassPiMinusP_MC->Scale(sData/sMC);
    h1_MassPiMinusP_MC->Draw("psame");
    
    h1_MassPiMinusP_MC2->SetLineColor(1);
    h1_MassPiMinusP_MC2->SetMarkerColor(1);
    h1_MassPiMinusP_MC2->SetMarkerStyle(20);
    h1_MassPiMinusP_MC2->SetMarkerSize(0.5);
    sMC2 = h1_MassPiMinusP_MC->Integral(binx1,binx2-1);
    h1_MassPiMinusP_MC2->Scale(sData/sMC);
    h1_MassPiMinusP_MC2->Draw("psame");
    
    
    c0->SaveAs("plot_compare_DataMC_"+run+"_"+tag+".pdf(");
    c1->SaveAs("plot_compare_DataMC_"+run+"_"+tag+".pdf");
    c2->SaveAs("plot_compare_DataMC_"+run+"_"+tag+".pdf");
    c3->SaveAs("plot_compare_DataMC_"+run+"_"+tag+".pdf");
    c4->SaveAs("plot_compare_DataMC_"+run+"_"+tag+".pdf");
    c5->SaveAs("plot_compare_DataMC_"+run+"_"+tag+".pdf)");
    
}
