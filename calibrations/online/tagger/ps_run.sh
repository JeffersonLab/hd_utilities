RunNo=$1
InputFile=$2
Variation=calib
cp -p ${InputFile} hd_root-ps.root
mkdir -p ps_offsets
ccdb dump /PHOTON_BEAM/pair_spectrometer/base_time_offset:${RunNo}:${Variation} > ps_offsets/base_time_offset_ccdb.txt
ccdb dump /PHOTON_BEAM/pair_spectrometer/coarse/tdc_timing_offsets:${RunNo}:${Variation} > ps_offsets/tdc_timing_offsets_psc_ccdb.txt
ccdb dump /PHOTON_BEAM/pair_spectrometer/coarse/adc_timing_offsets:${RunNo}:${Variation} > ps_offsets/adc_timing_offsets_psc_ccdb.txt
ccdb dump /PHOTON_BEAM/pair_spectrometer/fine/adc_timing_offsets:${RunNo}:${Variation} > ps_offsets/adc_timing_offsets_ps_ccdb.txt
root -b -q 'ps_fits.C("hd_root-ps.root",true)'
root -b -q 'ps_offsets.C("fits-csv")'
#rm -f hd_root.root
