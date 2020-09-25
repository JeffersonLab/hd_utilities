#!/bin/tcsh -f

set path_dir=/work/halld/home/gxproj2/calibration/fcal
set i=$1
#set imax=$2
#while ( $i < $imax )
cp pedcal_period_$i.root pedestalsall.root
root -q -b -l loop.C
mv Pedestal_vs_Channel.pdf Pedestal_vs_Channel_$i.pdf
mv Pedestal.pdf Pedestal_$i.pdf
mv pedestals_pedestalsall.root.txt pedestals_$i.txt
#    @ i ++
#    echo $i
#end
