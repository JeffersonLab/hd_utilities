{
	// TString filename = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_pipiP_match_off/tree_gamma_pipiP_MATCHOFF.root";
	TString filename = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_P/tree_gamma_P_MATCHON.root";
	// TString filename = "/home/fs7/jzarling/MCPhotonEffOut/Gun_plus_P_matchoff/tree_gamma_P_MATCHOFF.root";
	// TString treename = "gamma_pi+pi-p_nofit_Tree";
	TString treename = "gamma_p_nofit_Tree";
	TFile* f = TFile::Open( filename );
	TTree* tr = (TTree*)f->Get( treename );
	// tr->Process( "/home/fs7/jzarling/PhotonEffStudUtilities/DSelectors/DSelector_gammagun_pipiproton.C+" );
	tr->Process( "/home/fs7/jzarling/PhotonEffStudUtilities/DSelectors/DSelector_gammagun_proton.C+" );
}