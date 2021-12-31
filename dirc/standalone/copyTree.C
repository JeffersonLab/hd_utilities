void copyTree(TString treeName = "hd_root_72720_72729", TString inputDir = "/sciclone/pscr/jrstevens01/TMPDIR/RunPeriod-2019-11/recon/ver01_pass03/merged/") {
 gROOT->Reset();

 TString treeFileName = treeName + ".root";
 TFile f(Form("%s/%s", inputDir.Data(), treeFileName.Data()));
 TTree *T = (TTree*)f.Get("dirc");

 for(int bar = 8; bar < 48; bar++){
  cout<<treeName<<" bar="<<bar<<endl;
  TString newTreeFileName = treeName;
  newTreeFileName += Form("_bar%02d.root", bar);
  TFile f2(newTreeFileName,"recreate");
  TTree *T2 = T->CopyTree(Form("DrcEvent.fId == %d", bar));
  T2->Write();
  f2.Close();
  
  gSystem->Exec(Form("mv %s %s", newTreeFileName.Data(), (inputDir+newTreeFileName).Data()));
 }
}
