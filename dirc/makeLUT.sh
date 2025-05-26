
# link macros
ln -s $HD_UTILITIES_HOME/dirc/loadlib.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_add.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_avr.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_convert.C

# get includes and dictionaries (new for ROOT6)
ln -s $HALLD_RECON_HOME/$BMS_OSNAME/include/lut_dirc/DrcLutNode.h
ln -s $HALLD_RECON_HOME/src/.${BMS_OSNAME}/plugins/Analysis/lut_dirc/DrcLutNode_Dict_rdict.pcm

#$HD_UTILITIES_HOME/dirc/loop_lut_dirc.py
#$HD_UTILITIES_HOME/dirc/loop_lut_avr.py

root -b -q loadlib.C glxlut_add.C+'("root/lut_avr_*.root")'
root -b -q loadlib.C glxlut_convert.C

#rm AutoDict_vector_TVector3*
#rm glxlut_avr_C*
rm glxlut_add_C*
