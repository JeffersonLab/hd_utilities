#!/bin/bash

RunNo=$1
OutputDir=TAGH_timewalk_output
InputFile=$2

echo ==do_tagh-offsets.sh==
echo RunNo=$RunNo
echo InputFile=$InputFile

# run calibration scripts
OutputDir=TAGH_offsets_output
#rm -rf $OutputDir
mkdir -p offsets
ccdb dump /PHOTON_BEAM/hodoscope/base_time_offset:${RunNo} > offsets/base_time_offset_ccdb.txt
ccdb dump /PHOTON_BEAM/hodoscope/tdc_time_offsets:${RunNo} > offsets/tdc_time_offsets_ccdb.txt
ccdb dump /PHOTON_BEAM/hodoscope/fadc_time_offsets:${RunNo} > offsets/fadc_time_offsets_ccdb.txt
root -b -q "tagh_fits.C(\"$InputFile\",true)"
root -b -q 'tagh_offsets.C("fits-csv")'
#ccdb add /PHOTON_BEAM/hodoscope/tdc_time_offsets -v default -r ${RunNo}-${RunNo} offsets/tdc_time_offsets.txt
#ccdb add /PHOTON_BEAM/hodoscope/fadc_time_offsets -v default -r ${RunNo}-${RunNo} offsets/fadc_time_offsets.txt
#mv offsets $OutputDir; mv fits-csv $OutputDir;  

