void plot_compareEff (TString tag="")
{
// File: plot_compareEff.C
    // Read in TGraphErrors with efficiency evaluations from data and MC and display on the same plot.
//

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
    
    // TString tag="CL6e-7";
    // TString tag="CL01";
    
    TFile *inData = new TFile("DSelector_pi0pippim__B4_030804_"+tag+"_effplots.root","read");
    TFile *inMC = new TFile("DSelector_pi0pippim__B4_gen_amp_051440_"+tag+"_effplots.root","read");
    
    TGraphErrors *gr_E2calcB_BCAL_Data = (TGraphErrors*) inData->Get("gr_E2calcB_BCAL");
    TGraphErrors *gr_E2calcB_FCAL_Data = (TGraphErrors*) inData->Get("gr_E2calcB_FCAL");
    TH1F *h1_E2_eff_BCAL_Data= (TH1F*)inData->Get("h1_E2_eff_BCAL");
    TH1F *h1_E2_eff_FCAL_Data= (TH1F*)inData->Get("h1_E2_eff_FCAL");
    
    TGraphErrors *gr_E2calcB_BCAL_MC = (TGraphErrors*) inMC->Get("gr_E2calcB_BCAL");
    TGraphErrors *gr_E2calcB_FCAL_MC = (TGraphErrors*) inMC->Get("gr_E2calcB_FCAL");
    TH1F *h1_E2_eff_BCAL_MC = (TH1F*)inMC->Get("h1_E2_eff_BCAL");
    TH1F *h1_E2_eff_FCAL_MC = (TH1F*)inMC->Get("h1_E2_eff_FCAL");
    
    // gPad->SetLogy();
    
    //
    // Find integral between 1.2 and 1.6 GeV and normalize to that value.
    
    Double_t efficiency;
    Double_t sigmaX;
    Double_t sigmaY;
    Double_t E;
    Double_t Emin=1.2;
    Double_t Emax=1.6;
    Double_t average;
    Double_t naverage;
    Double_t eff0;
    
    average=0;
    naverage=0;
    Int_t npoints;
    npoints = gr_E2calcB_BCAL_Data->GetN();
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_BCAL_Data->GetPoint(j,E,efficiency);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << endl;
        if (E>Emin && E<Emax) {
            average += efficiency;
            naverage += 1;
        }
    }
    average /= naverage;
    cout << " average efficiency=" << average << endl << endl;
    eff0=1;
    
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_BCAL_Data->GetPoint(j,E,efficiency);
        sigmaY = gr_E2calcB_BCAL_Data->GetErrorY(j);
        sigmaX = gr_E2calcB_BCAL_Data->GetErrorX(j);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << " sigmaX=" << sigmaX << " sigmaY=" << sigmaY << endl;
        gr_E2calcB_BCAL_Data->SetPoint(j,E,efficiency*eff0/average);
        gr_E2calcB_BCAL_Data->SetPointError(j,sigmaX,sigmaY*eff0/average);
    }
    average=0;
    naverage=0;
    npoints = gr_E2calcB_FCAL_Data->GetN();
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_FCAL_Data->GetPoint(j,E,efficiency);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << endl;
        if (E>Emin && E<Emax) {
            average += efficiency;
            naverage += 1;
        }
    }
    average /= naverage;
    cout << " average efficiency=" << average << endl << endl;
    eff0=1;
    
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_FCAL_Data->GetPoint(j,E,efficiency);
        sigmaY = gr_E2calcB_FCAL_Data->GetErrorY(j);
        sigmaX = gr_E2calcB_FCAL_Data->GetErrorX(j);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << " sigmaX=" << sigmaX << " sigmaY=" << sigmaY << endl;
        gr_E2calcB_FCAL_Data->SetPoint(j,E,efficiency*eff0/average);
        gr_E2calcB_FCAL_Data->SetPointError(j,sigmaX,sigmaY*eff0/average);
    }
    
    
    
    average=0;
    naverage=0;
    npoints = gr_E2calcB_BCAL_MC->GetN();
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_BCAL_MC->GetPoint(j,E,efficiency);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << endl;
        if (E>Emin && E<Emax) {
            average += efficiency;
            naverage += 1;
        }
    }
    average /= naverage;
    cout << " average efficiency=" << average << endl << endl;
    eff0=1;
    
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_BCAL_MC->GetPoint(j,E,efficiency);
        sigmaY = gr_E2calcB_BCAL_MC->GetErrorY(j);
        sigmaX = gr_E2calcB_BCAL_MC->GetErrorX(j);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << " sigmaX=" << sigmaX << " sigmaY=" << sigmaY << endl;
        gr_E2calcB_BCAL_MC->SetPoint(j,E,efficiency*eff0/average);
        gr_E2calcB_BCAL_MC->SetPointError(j,sigmaX,sigmaY*eff0/average);
    }
    average=0;
    naverage=0;
    npoints = gr_E2calcB_FCAL_MC->GetN();
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_FCAL_MC->GetPoint(j,E,efficiency);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << endl;
        if (E>Emin && E<Emax) {
            average += efficiency;
            naverage += 1;
        }
    }
    average /= naverage;
    cout << " average efficiency=" << average << endl << endl;
    eff0=1;
    
    for(Int_t j=0;j<npoints;j++) {
        gr_E2calcB_FCAL_MC->GetPoint(j,E,efficiency);
        sigmaY = gr_E2calcB_FCAL_MC->GetErrorY(j);
        sigmaX = gr_E2calcB_FCAL_MC->GetErrorX(j);
        // cout << " npoints=" << npoints << " j=" << j << " E=" << E << " efficiency=" << efficiency << " sigmaX=" << sigmaX << " sigmaY=" << sigmaY << endl;
        gr_E2calcB_FCAL_MC->SetPoint(j,E,efficiency*eff0/average);
        gr_E2calcB_FCAL_MC->SetPointError(j,sigmaX,sigmaY*eff0/average);
    }
    
    Double_t xmin=0.;
    Double_t xmax=2.0;
    Double_t ymin=0;
    Double_t ymax=1.5;
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1000,700);
    c0->Divide(2,1);
    TString text;
    text.Form("Normalized to E=%.1f-%.1f GeV",Emin,Emax);
    TLatex *t1 = new TLatex(0.3,0.45,text);
    
    c0->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_BCAL_Data->SetTitle("Method B BCAL");
    gr_E2calcB_BCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcB_BCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcB_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcB_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcB_BCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcB_BCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    gr_E2calcB_BCAL_Data->SetLineColor(2);
    gr_E2calcB_BCAL_Data->SetMarkerColor(2);
    gr_E2calcB_BCAL_Data->SetMarkerStyle(20);
    gr_E2calcB_BCAL_Data->SetMarkerSize(0.5);
    gr_E2calcB_BCAL_Data->Draw("Ap");
    gr_E2calcB_BCAL_MC->SetLineColor(4);
    gr_E2calcB_BCAL_MC->SetMarkerColor(4);
    gr_E2calcB_BCAL_MC->SetMarkerStyle(20);
    gr_E2calcB_BCAL_MC->SetMarkerSize(0.5);
    gr_E2calcB_BCAL_MC->Draw("psame");
    
    TLegend *leg = new TLegend(0.6,0.25,0.85,0.4);
    leg->AddEntry(gr_E2calcB_BCAL_Data,"Data","p");
    leg->AddEntry(gr_E2calcB_BCAL_MC,"MC","p");
    leg->Draw();
    
    t1->SetNDC();
    t1->SetTextSize(0.04);
    t1->DrawLatex(0.3,0.40,tag);
    
    c0->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_E2calcB_FCAL_Data->SetTitle("Method B FCAL");
    gr_E2calcB_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_E2calcB_FCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_E2calcB_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    gr_E2calcB_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    gr_E2calcB_FCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    gr_E2calcB_FCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    gr_E2calcB_FCAL_Data->SetLineColor(2);
    gr_E2calcB_FCAL_Data->SetMarkerColor(2);
    gr_E2calcB_FCAL_Data->SetMarkerStyle(20);
    gr_E2calcB_FCAL_Data->SetMarkerSize(0.5);
    gr_E2calcB_FCAL_Data->Draw("Ap");
    gr_E2calcB_FCAL_MC->SetLineColor(4);
    gr_E2calcB_FCAL_MC->SetMarkerColor(4);
    gr_E2calcB_FCAL_MC->SetMarkerStyle(20);
    gr_E2calcB_FCAL_MC->SetMarkerSize(0.5);
    gr_E2calcB_FCAL_MC->Draw("psame");
    
    TLegend *leg1 = new TLegend(0.6,0.25,0.85,0.4);
    leg1->AddEntry(gr_E2calcB_FCAL_Data,"Data","p");
    leg1->AddEntry(gr_E2calcB_FCAL_MC,"MC","p");
    leg1->Draw();
    t1->DrawLatex(0.3,0.40,tag);
    
    TCanvas *c1 = new TCanvas(" c1", " c1",200,10,1000,700);
    
    c1->Divide(2,1);
    
    c1->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_E2_eff_BCAL_Data->SetTitle("Method Weights BCAL");
    h1_E2_eff_BCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    h1_E2_eff_BCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_E2_eff_BCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_E2_eff_BCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_E2_eff_BCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    h1_E2_eff_BCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    h1_E2_eff_BCAL_Data->SetLineColor(2);
    h1_E2_eff_BCAL_Data->SetMarkerColor(2);
    h1_E2_eff_BCAL_Data->SetMarkerStyle(20);
    h1_E2_eff_BCAL_Data->SetMarkerSize(0.5);
    h1_E2_eff_BCAL_Data->Draw();
    h1_E2_eff_BCAL_MC->SetLineColor(4);
    h1_E2_eff_BCAL_MC->SetMarkerColor(4);
    h1_E2_eff_BCAL_MC->SetMarkerStyle(20);
    h1_E2_eff_BCAL_MC->SetMarkerSize(0.5);
    h1_E2_eff_BCAL_MC->Draw("psame");
    
    TLegend *leg2 = new TLegend(0.6,0.25,0.85,0.4);
    leg2->AddEntry(h1_E2_eff_BCAL_Data,"Data","p");
    leg2->AddEntry(h1_E2_eff_BCAL_MC,"MC","p");
    leg2->Draw();
    
    text.Form("Normalized to E=%.1f-%.1f GeV",Emin,Emax);
    t1->Draw();
    t1->DrawLatex(0.3,0.40,tag);
    
    c1->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    h1_E2_eff_FCAL_Data->SetTitle("Method Weights FCAL");
    h1_E2_eff_FCAL_Data->GetXaxis()->SetRangeUser(xmin,xmax);
    h1_E2_eff_FCAL_Data->GetYaxis()->SetRangeUser(ymin,ymax);
    h1_E2_eff_FCAL_Data->GetXaxis()->SetTitleSize(0.05);
    h1_E2_eff_FCAL_Data->GetYaxis()->SetTitleSize(0.05);
    h1_E2_eff_FCAL_Data->GetXaxis()->SetTitle("Low E_{#gamma} (GeV)");
    h1_E2_eff_FCAL_Data->GetYaxis()->SetTitle("Relative Efficiency");
    h1_E2_eff_FCAL_Data->SetLineColor(2);
    h1_E2_eff_FCAL_Data->SetMarkerColor(2);
    h1_E2_eff_FCAL_Data->SetMarkerStyle(20);
    h1_E2_eff_FCAL_Data->SetMarkerSize(0.5);
    h1_E2_eff_FCAL_Data->Draw();
    h1_E2_eff_FCAL_MC->SetLineColor(4);
    h1_E2_eff_FCAL_MC->SetMarkerColor(4);
    h1_E2_eff_FCAL_MC->SetMarkerStyle(20);
    h1_E2_eff_FCAL_MC->SetMarkerSize(0.5);
    h1_E2_eff_FCAL_MC->Draw("psame");
    
    TLegend *leg3 = new TLegend(0.6,0.25,0.85,0.4);
    leg3->AddEntry(h1_E2_eff_FCAL_Data,"Data","p");
    leg3->AddEntry(h1_E2_eff_FCAL_MC,"MC","p");
    leg3->Draw();
    t1->Draw();
    t1->DrawLatex(0.3,0.40,tag);
    
    c0->SaveAs("plot_compareEff_"+tag+".pdf(");
    c1->SaveAs("plot_compareEff_"+tag+".pdf)");
    
}
