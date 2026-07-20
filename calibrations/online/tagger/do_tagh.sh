#!/bin/bash

RunNo=$1
OutputDir=TAGH_timewalk_output
InputFile=$2

echo ==do_tagh.sh==
echo RunNo=$RunNo
echo InputFile=$InputFile

root -b -q "gaussian_fits.C(\"$InputFile\",true)"
root -b -q 'timewalk_fits.C("gaussian-fits-csv")'
mkdir -p $OutputDir
#mv *.txt $OutputDir; mv fits_* $OutputDir; mv parms_timewalk $OutputDir
#mv fits_* $OutputDir; mv parms_timewalk $OutputDir
mv fits_timewalk parms_timewalk $OutputDir
mv gaussian-fits-csv $OutputDir; mv overall_gaussian_fit.gif $OutputDir
cp tdc_timewalk.txt $OutputDir
