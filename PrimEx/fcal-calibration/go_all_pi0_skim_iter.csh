#!/bin/tcsh -f

set iter=$1
set runs_to_check=$2

set wf=$runs_to_check-$iter
swif create $wf

set jan_cfg=jana_data_pi0.config
#jan_cfg=jana_data_pi0_encor.config

set jan_dir=/work/halld/home/gxproj2/calibration/fcal

set out_dir=$jan_dir/$wf

set roodir=$out_dir/root-files
mkdir -p $roodir

set rundir=$out_dir/runr
mkdir -p $rundir

set errdir=$out_dir/err
mkdir -p $errdir

set outdir=$out_dir/outr
mkdir -p $outdir

set path_dir=/cache/halld/RunPeriod-2019-01/calib/ver18
#set path_dir=/cache/halld/RunPeriod-2019-01/calib/ver28

foreach rnb ( `cat txt/$runs_to_check.txt` )
    echo $rnb
    foreach file ( `ls $path_dir/*$rnb*fcal*.evio` )
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
	swif add-job -workflow $wf -project gluex -track analysis -ram 5GB -time 2h -stdout $outfile -stderr $errfile $exec
   end
end
echo 'run workflow'
swif run $wf
