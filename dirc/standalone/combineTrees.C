void combineTrees(TString inputDir = "/sciclone/pscr/jrstevens01/TMPDIR/RunPeriod-2019-11/recon/ver01_pass03/merged/") {
 gROOT->Reset();

 for(int bar = 24; bar < 48; bar++){
  cout<<"combining bar="<<bar<<endl;
  gSystem->Exec(Form("hadd %s/hd_root_sum_bar%02d.root %s/*_bar%02d.root", inputDir.Data(), bar, inputDir.Data(), bar));
 }
}
