{

  TProfile *p = (TProfile*)gDirectory->Get("/CDC_Efficiency/Online/Efficiency Vs N");
  //For hadd_only output use this instead:
  //TProfile *p = (TProfile*)gDirectory->Get("/Online/Efficiency Vs N");

  if (!p) cout << " Cannot find Efficiency Vs N\n";
  if (!p) return;
  
  FILE *f = fopen("eff.txt","w");
  if (!f) cout << "Could not open output file" << endl;
  if (f) for (int i=1; i<= p->GetNbinsX(); i++) fprintf(f,"%.2f\n",p->GetBinContent(i));
  if (f) fclose(f);

}
