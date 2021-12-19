// Find z vertex resolution from difference between thrown and recon MC tracks

#include <string>
#include "TFile.h"
#include "TTree.h"
#include "TString.h"

void z_res_mc(TString mcfile="tree_mc.root", TString treeName="kpkm__B4_Tree", const Float_t window1=50.3, const Float_t window2=79.8, const Float_t dz=1.0) {

    /*  
    mcfile: MC data tree file
    treeName: name of tree
    window1: z position of upstream target window (cm)
    window2: z position of downstream  target window (cm)
    dz: length of section of target used to calculate resolution at either end (cm)
   
    windows are at 50.3 and 79.8 cm in https://halldweb.jlab.org/DocDB/0049/004924/002/TaskForce_ProtonFiducialVertex.pdf
    */

    if (gSystem->AccessPathName(mcfile)) {
        printf("Cannot find file %s\n",mcfile.Data());
        exit(0);
    } 

    if (window2 <= window1) {
        printf("window2 should be larger than window1\n");
        exit(0);
    }

    if (dz > (window2 - window1)) {
        printf("dz should be less than or equal to the distance between the target windows.\n");
        exit(0);
    }


    TDirectory *home = gDirectory;
    TFile *rootfile = new TFile(mcfile,"READ");
    TTree *t = (TTree*)rootfile->Get(treeName);

    if (!t) {
      printf("Cannot find tree %s\n",treeName.Data());
      exit(0);
    } 

    if (!t->GetListOfBranches()->FindObject("Thrown__X4")) {
      printf("Cannot find branch Thrown__X4\n");
      exit(0);
    }

    if (!t->GetListOfBranches()->FindObject("X4_Production")) {
      printf("Cannot find branch X4_Production\n");
      exit(0);
    }

    TLorentzVector* x4recon=0;
    TLorentzVector* x4thrown=0;
    TClonesArray* tarray=0;

    t->SetBranchAddress("X4_Production",&x4recon);
    t->SetBranchAddress("Thrown__X4",&tarray);

    home->cd();

    TH1I* hzt = new TH1I("hzt","Thrown vertex z; thrown vertex z (cm)", 100,45,85);
    TH1I* hzr = new TH1I("hzr","Reconstructed vertex z; reconstructed vertex z (cm)", 100,45,85);

    TH2I* hzdiff = new TH2I("hzdiff","Reconstructed vertex z  - thrown vertex z; thrown z (cm); reconstructed z - thrown z (cm)", 400,45,85,100, -5,5);
 

    for (Long64_t i = 0; i<t->GetEntries(); i++) {
      //      if (i>1000) break; 

      t->GetEntry(i);

      x4thrown = (TLorentzVector*)tarray->At(0);   // one object per particle, same vertex for all 

      Double_t zthrown = x4thrown->Z();
      Double_t zrecon = x4recon->Z();

      hzt->Fill(zthrown);
      hzr->Fill(zrecon);

      hzdiff->Fill(zthrown,zrecon-zthrown);

      tarray->Clear();
    }

    rootfile->Close();    //    delete rootfile;
    

    gStyle->SetCanvasDefH(500);
    gStyle->SetCanvasDefW(800);

	 
    TCanvas *c1 = new TCanvas(); 
    c1->Divide(2,1);
    c1->cd(1);
    hzt->Draw();
    c1->cd(2);
    hzr->Draw();
    
    TCanvas *c2 = new TCanvas("c2","c2",700,500);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1);
    hzdiff->Draw("colz");
    TPaveText * pt1 = new TPaveText(0.75, 0.8, 0.9, 0.9 ,"NB NDC");
    pt1->SetFillStyle(0);
    pt1->SetBorderSize(0);
    pt1->AddText(Form("Mean y  %.4f",hzdiff->GetMean(2)));
    pt1->AddText(Form("RMS y %.4f",hzdiff->GetRMS(2)));
    pt1->Draw();

    // check for trend with z
    
    Int_t nchunk=5;
    Float_t chunk = (window2-window1)/nchunk;
    Float_t mid = 0.5*(window1+window2);
    const Bool_t Naomi_is_awesome=kTRUE;   
    Float_t z1, z2;
    TH1D* hp;
    TF1* g = new TF1("g","gaus",-5,5);
    g->SetNpx(200);

    for (int i=0; i<5; i++) {
      z1 = mid - (0.5*nchunk-i)*chunk;
      z2 = z1+chunk;
      hp = (TH1D*)hzdiff->ProjectionY("hp",hzdiff->GetXaxis()->FindBin(z1),hzdiff->GetXaxis()->FindBin(z2));
      hp->Fit(g,"0Q");
      printf("%.1f cm to %.1f cm: mean %.4f cm err %.4f cm sigma %.3f cm err %.3f cm\n",z1,z2,g->GetParameter(1),g->GetParError(1),g->GetParameter(2),g->GetParError(2));

    }



    // Find resolution for a small (1cm?) chunk at each end, use that to calc total range

    TCanvas *c3 = new TCanvas("c3");
    c3->Divide(2,1);

    Double_t zsigma[2];    
    z1 = window1;
    TPaveText *pt[2];

    for (int i=0; i<2; i++) {
      c3->cd(i+1);

      if (i==1) z1 = window2 - dz;
      z2 = z1+dz;

      hp = (TH1D*)hzdiff->ProjectionY("hp",hzdiff->GetXaxis()->FindBin(z1),hzdiff->GetXaxis()->FindBin(z2));
      hp->SetTitle(Form("Projection z %.1f cm to %.1f cm",z1,z2));
      hp->GetXaxis()->SetRangeUser(-3,3);
      hp->DrawCopy();
      hp->Fit(g,"0Q");
      g->DrawCopy("same");

      pt[i] = new TPaveText(0.55, 0.7, 0.9, 0.9 ,"NB NDC");
      pt[i]->SetFillStyle(0);
      pt[i]->SetBorderSize(0);
      pt[i]->AddText(Form("Mean %.4f #pm %.4f",g->GetParameter(1),g->GetParError(1)));
      pt[i]->AddText(Form("Std dev %.4f #pm %.4f",g->GetParameter(2),g->GetParError(2)));
      pt[i]->Draw();
      printf("\n%.1f cm to %.1f cm: mean %.4f cm err %.4f cm sigma %.3f cm err %.3f cm\n",z1,z2,g->GetParameter(1),g->GetParError(1),g->GetParameter(2),g->GetParError(2));
      zsigma[i] = g->GetParameter(2);
    }

    printf("\nExcluding 3sigma, target extends from %.2f cm to %.2f cm\n",window1+3.0*zsigma[0],window2-3.0*zsigma[1]);
        
}
