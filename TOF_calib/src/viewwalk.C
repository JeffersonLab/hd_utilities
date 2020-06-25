

void viewwalk(int R){


  char datafile[128];
  sprintf(datafile, "calibration%d/walk_results_run%d.root", R, R);

  TFile *RF = new TFile(datafile, "READ");

  char hnam[128];
  char fnam1[128];
  char fnam2[128];
  for (int k=0; k<184; k++) {
    sprintf(hnam, "Twalk%d",k);
    sprintf(fnam1, "fit1hist%d",k);
    sprintf(fnam2, "fit2hist%d",k);
    TH1F *h = (TH1F*)RF->Get(hnam);
    TF1 *f1 = (TF1*)RF->Get(fnam1);
    TF1 *f2 = (TF1*)RF->Get(fnam2);
    if ((f2 == NULL) || (f1 == NULL)){
      cout<<"Error no fit function found for PMT: "<<k<<endl;
      continue;
    }
    h->Draw("colz");
    f1->SetLineColor(7);
    f2->SetLineColor(2);
    f1->Draw("same");
    f2->Draw("same");
    gPad->SetGrid();
    gPad->Update();
    //getchar();

  }

  RF->Close();
}
