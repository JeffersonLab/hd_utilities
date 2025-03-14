
ln -s $HD_UTILITIES_HOME/dirc/loadlib.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_add.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_avr.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_convert.C

#$HD_UTILITIES_HOME/dirc/loop_lut_dirc.py
#$HD_UTILITIES_HOME/dirc/loop_lut_avr.py

root -b -q loadlib.C glxlut_add.C+'("root/lut_avr_*.root")'
root -b -q loadlib.C glxlut_convert.C

#rm AutoDict_vector_TVector3*
#rm glxlut_avr_C*
rm glxlut_add_C*
