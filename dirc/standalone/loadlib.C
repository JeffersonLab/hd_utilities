{
  TString path="", tok="";
  TString sys = gSystem->Getenv("JANA_PLUGIN_PATH");
  Ssiz_t from = 0;
  while (sys.Tokenize(tok, from,":")) {
    if(tok.Contains("halld_recon")) path = tok;    
  }
  
  gSystem->Load("DrcHit_cc.so");
  gSystem->Load("DrcEvent_cc.so");
  gSystem->Load(path+"/lut_dirc.so");
}
