{
  TString path="", tok="";
  TString sys = gSystem->Getenv("JANA_PLUGIN_PATH");
  Ssiz_t from = 0;
  while (sys.Tokenize(tok, from,":")) {
    if(tok.Contains("halld_recon")) path = tok;    
  }
  
  //gSystem->Load(path+"/pid_dirc.so");
  cout << path.Data() << endl;
  //gSystem->AddIncludePath(" -I/work/halld2/home/gxproj7/monitoring/builds/halld_recon/Linux_CentOS7.7-x86_64-gcc4.8.5/include/ ");
  path = "/work/halld2/home/gxproj7/monitoring/builds/halld_recon/src/.Linux_CentOS7.7-x86_64-gcc4.8.5/plugins/Analysis/";
  //gSystem->Load(path+"dirc_tree/dirc_tree.so");
  //gSystem->Load(path+"lut_dirc/lut_dirc.so");
  gSystem->Load("DrcLutNode_cc.so");
}
