#!/bin/tcsh -f

set path_dir=/work/halld/home/gxproj2/calibration/fcal/
set i=$1
#set imax=$2

#while ( $i < $imax )
hadd -f $path_dir/pedcal_period_$i.root $path_dir/pedcal-period_$i-ped/*.root
#    @ i ++
#    echo $i
#end
