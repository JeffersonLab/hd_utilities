
int AMP = 0;

void drawp(int PMT, int amp){

  AMP = amp;

  char fnam[128];
  sprintf(fnam,"ADC_positions/pmt_%d.dat",PMT);

  if (AMP){
    sprintf(fnam,"ADC_positions/pmt_amp_%d.dat",PMT);
  }

  ifstream INF(fnam);

  double XPOS[500], YPOS[500],dYPOS[500];
  int Cnt=0;
  while (!INF.eof()){
    INF>>XPOS[Cnt]>>YPOS[Cnt]>>dYPOS[Cnt];
    //cout<<XPOS[Cnt]<<"  "<<YPOS[Cnt]<<"   "<<dYPOS[Cnt]<<endl;
    Cnt++;
  }

  TGraphErrors *gr = new TGraphErrors(Cnt-1, XPOS, YPOS, NULL, dYPOS);
  char atit[128];
  sprintf(atit,"TOF ADC Integral MPV PMT %d",PMT);
  if (AMP){
    sprintf(atit,"TOF ADC Signal Amplitude MPV PMT %d",PMT);
  }
  gr->SetTitle(atit);
  gr->SetMarkerColor(4);
  gr->SetMarkerStyle(21);
  gr->Draw("AP");
  gr->GetXaxis()->SetTitle("Run Number");
  gr->GetYaxis()->SetTitle("ADC Integral");
  if (AMP) {
    gr->GetYaxis()->SetTitle("ADC Signal Amplitude");
  }
  double max = TMath::MaxElement(Cnt-1,gr->GetY());
  gr->GetYaxis()->SetRangeUser(0,max*1.1);
  gr->Draw("AP");
  sprintf(atit,"figures/adc_mpv_pmt%d.pdf",PMT);
  gPad->SetGrid();
  gPad->SaveAs(atit);

}
