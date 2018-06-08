{

  TFile *F1 = new TFile("hd_root.root");
  TTree *rec = (TTree*)F1->Get("PHYSICS/recon");
  TTree *gen = (TTree*)F1->Get("PHYSICS/thrown");

  PHYSICS->cd();
  rec->Process("b1pi_rec.C");
  gen->Process("b1pi_gen.C");


  delete rec;
  delete gen;
  F1->Close();

  exit;
}
