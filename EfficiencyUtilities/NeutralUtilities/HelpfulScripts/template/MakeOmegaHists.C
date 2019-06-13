{
gROOT->ProcessLine(".L /home/fs7/jzarling/PhotonEffStudUtilities/AnalyzeOutput/MakeHists.cxx+");
gSystem->Load("/home/fs11/jzarling/ChargedAsymSyst/turn_key/src/MakeHists_cxx.so");

Redefine_FOUND_E_MIN_cut( E_MIN_TO_REPLACE );

MakeOmegaHists("FNAME_TO_REPLACE","omega_hists.root",false);

}