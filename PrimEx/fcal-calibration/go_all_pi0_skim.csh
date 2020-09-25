#!/bin/tcsh -f

set batch=$1
set wf=primex-daily-skim-batch$batch
swif create $wf

set jan_cfg=jana_data_pi0_skim.config

set jan_dir=/work/halld/home/gxproj2/calibration/fcal

set version=ver28b

set out_dir=/cache/halld/RunPeriod-2019-01/calib/$version-batch$batch

set roodir=$out_dir/root-files
mkdir -p $roodir

set rundir=$jan_dir/calskim-$version-batch$batch/run
mkdir -p $rundir

set errdir=$jan_dir/calskim-$version-batch$batch/err
mkdir -p $errdir

set outdir=$jan_dir/calskim-$version-batch$batch/out
mkdir -p $outdir

set path_dir=/cache/halld/RunPeriod-2019-01/rawdata/Run0
set i=48
set imax=49
while ( $i < $imax )
    foreach rnb ( `cat txt/period_$i.txt` )
	echo $rnb
	foreach file ( `ls $path_dir*/*$rnb*.evio` )
	    echo $file
	    set filename=`basename $file`
	    echo $filename
	    set rootname=($filename:as/evio/root/)
	    echo $rootname
	    set rootfile=$roodir/$rootname
	    set ntuple=($filename:as/.evio//)
	    set runfile=$rundir/$ntuple.csh
	    set errfile=$errdir/$ntuple.err
	    set outfile=$outdir/$ntuple.out
	    echo "source /work/halld/home/gxproj2/calibration/Env/custom_GlueX_dev.csh" > $runfile
	    echo "cd ${out_dir}" >> $runfile
	    echo "jcache get $file" >> $runfile
	    echo "hd_root --config=${jan_dir}/$jan_cfg $file -o $rootfile" >> $runfile 
	    set exec = "source $runfile"
	    echo $exec
	    swif add-job -workflow $wf -project gluex -track analysis -ram 5GB -time 8h -stdout $outfile -stderr $errfile $exec
	end
    end
    @ i++
end
echo 'run workflow'
swif run $wf
