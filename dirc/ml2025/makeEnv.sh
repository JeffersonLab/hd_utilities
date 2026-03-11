setenv JANA_PLUGIN_PATH $HALLD_RECON_HOME/$BMS_OSNAME/plugins/

# get includes and dictionaries (new for ROOT6)
ln -sf $HALLD_RECON_HOME/$BMS_OSNAME/include/dirc_tree/DrcHit.h
ln -sf $HALLD_RECON_HOME/$BMS_OSNAME/include/dirc_tree/DrcEvent.h
ln -sf $HALLD_RECON_HOME/src/plugins/Analysis/dirc_tree/DrcHit.cc
ln -sf $HALLD_RECON_HOME/src/plugins/Analysis/dirc_tree/DrcEvent.cc
ln -sf $HALLD_RECON_HOME/$BMS_OSNAME/include/lut_dirc/DrcLutNode.h

# compile local libraries for running reconstruction
root -l -b -q $HD_UTILITIES_HOME/dirc/ml2025/compileLib.C
