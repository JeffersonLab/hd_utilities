setenv BAR 32
setenv BAR_PAD `printf "%02d" ${BAR}`
#setenv CORR_FILE all_m2_allbars_sel.root.corr.root 
setenv CORR_FILE allbars_ver1.1.corr.root

foreach BIN ( `seq 2 7` )
  setenv BIN_PAD `printf "%02d" ${BIN}`
  ln -sf /sciclone/pscr/jrstevens01/TMPDIR/RunPeriod-2019-11/recon/ver01_pass03/merged/hd_root_sum_bar31_33.root hd_root_sum_bar${BAR_PAD}.root 
  ln -sf $CORR_FILE hd_root_sum_bar${BAR_PAD}.root.corr_${BAR_PAD}_${BIN_PAD}.root

  # no correction
  root -l -b -q loadlib.C reco_lut_02.C\(\"hd_root_sum_bar${BAR_PAD}.root\",\"lut_all_avr.root\",${BAR},${BIN},3.0,0,0,0,0\)

  # with correction
  root -l -b -q loadlib.C reco_lut_02.C\(\"hd_root_sum_bar${BAR_PAD}.root\",\"lut_all_avr.root\",${BAR},${BIN},3.0,0,0,0,2\)

  # no correction
  root -l -b -q loadlib.C reco_lut_02.C\(\"hd_root_sum_bar${BAR_PAD}.root\",\"lut_all_avr.root\",${BAR},${BIN},3.5,0,0,0,0\)
 
  # with correction
  root -l -b -q loadlib.C reco_lut_02.C\(\"hd_root_sum_bar${BAR_PAD}.root\",\"lut_all_avr.root\",${BAR},${BIN},3.5,0,0,0,2\)

end
