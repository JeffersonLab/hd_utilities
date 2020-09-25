# !/bin/bash

wf=$1
period=$2
wfname=primex-fcal-gains-matching-$period
gd=${wf/$wfname-/}
echo $gd
gd=${gd/added/}
echo $gd
ad=$period/g$gd
echo $ad
mkdir -p $ad
hadd -f $ad/runpi0.root $wf-added/*.root
echo "root -q -b -l '/work/halld/home/gxproj2/calibration/fcal/FitPi0_FCAL.c+($gd, \"${ad}/\")'" > run_$period.sh
chmod +x run_$period.sh
./run_$period.sh
