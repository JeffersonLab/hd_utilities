

void viewwalk(int PMT){


  char datafile[128] = "walkparms.dat";

  ifstream INF(datafile);
  double DATAY[11][500];
  double DATAX[500];
  int cnt=0;
  while (!INF.eof()){
    INF>>DATAX[cnt];
    for (int k=0;k<10;k++){
      INF>>DATAY[k][cnt];
    }
    cnt++;
  }
  cnt--;

  TCanvas *c1 = new TCanvas("c1", "Walk Parameters",1000, 1000);


  c1->Divide(4,3);

  TGraphErrors *graphs[9];
  char gtit[128];
  for (int k=0;k<9;k++){
    c1->cd(k+1);
    graphs[k] = new TGraphErrors(cnt, DATAX, DATAY[k], NULL, NULL);
    graphs[k]->GetXaxis()->SetTitle("run number [#]");
    sprintf(gtit,"Walk Parameter %d of PMT %d",k, PMT);
    graphs[k]->SetTitle(gtit);
    graphs[k]->SetMarkerColor(4);
    graphs[k]->SetMarkerStyle(21); 
    graphs[k]->Draw("AP");
    gPad->Update();
  }

  sprintf(gtit,"figures/walkparameters_of_PMT_%d.pdf",PMT);
  c1->SaveAs(gtit);

}
