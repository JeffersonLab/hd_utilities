#!/bin/tcsh -f
set wf=$1
echo wf $wf
set period=$2
echo period $period
set wfname=primex-fcal-gains-matching-$period
set gd=`echo $wf| sed "s/$wfname-//g" `
echo $gd
set gd=($gd:as/added//)
echo $gd
set ad=$period/g$gd
echo $ad
set br=\"
mkdir -p $ad
hadd -f $ad/runpi0.root $wf-added/*.root
###echo "root -q -b -l '/work/halld/home/gxproj2/calibration/fcal/FitPi0_FCAL.c+($gd,$br${ad}/$br)'" > run_$period-$gd.csh
###source run_$period-$gd.csh
