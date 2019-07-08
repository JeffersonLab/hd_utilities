{
	
gROOT->ProcessLine(".L /home/fs7/jzarling/PhotonEffStudUtilities/AnalyzeOutput/FitHistsMakeEffic.cxx+");
gSystem->Load("/home/fs11/jzarling/ChargedAsymSyst/turn_key/src/FitHistsMakeEffic_cxx.so");
	
	
FitOmegaHists("omega_hists.root","omega_effic.root");	
	
}