void vtest(int R, int pad){

  char fn[128];
  sprintf(fn,"calibration%d/deltat_tpos_refpad%dplane0.dat",R,pad);

  ifstream INF(fn);
  int paddle[50];
  double t[50], dt[50];

  double v[50];
  double x[50];
  int count=0;
  for (int k=0; k<46; k++){ 

    INF>>paddle[k]>>t[k]>>dt[k];
    if (k>0){

      double tdiff = TMath::Abs(TMath::Abs(t[k])-TMath::Abs(t[k-1]));
      double dist = 6.;
      if ((k==17) || (k==29)){
	dist = (4.5+6.)/2.;
      } else if ((k==18)||(k==28)){
	dist = 4.5;
      } else if ((k==19) || (k==27)){
	dist = (4.5+3.)/2.;
      } else if ((k==20) || (k==26)){
	dist = 3.;
      } else if ((k>20) && (k<26)){
	continue;
      }

      cout<<"v = "<<dist/tdiff<<endl;
      v[count]=dist/tdiff;
      x[count++]=k;
    }

  }

  TGraphErrors *gr = new TGraphErrors(count, x, v, NULL, NULL);
  gr->SetMarkerStyle(21);
  gr->SetMarkerColor(2);
  gr->Draw("AP");

  
}
