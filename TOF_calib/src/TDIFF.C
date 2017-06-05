void TDIFF(int Run, int pad, int plane){
  
  gROOT->ProcessLine(".L src/timedifference.C+g"); 
  timedifference(Run,pad,plane);

}
