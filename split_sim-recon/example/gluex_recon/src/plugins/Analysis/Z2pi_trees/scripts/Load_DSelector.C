{
	gSystem->AddIncludePath("-I${HALLD_HOME}/${BMS_OSNAME}/include/");
	gSystem->AddIncludePath("-I${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/include/");
	gSystem->Load("$(ROOT_ANALYSIS_HOME)/$(BMS_OSNAME)/lib/libDSelector.so");
}
