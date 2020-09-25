#!/bin/tcsh -f
echo `date`

set period=$1
set wfname=$2

echo period is $period
echo wfname is $wfname

set wf=$wfname-$period-ped
set jan_cfg=jana_data_ped.config
set jan_dir=/work/halld/home/gxproj2/calibration/fcal
set out_dir=$jan_dir
set roodir=$out_dir/$wf
mkdir -p $roodir
set rundir=$roodir/run
mkdir -p $rundir
set errdir=$roodir/err
mkdir -p $errdir
set outdir=$roodir/out
mkdir -p $outdir

#set path_dir=/cache/halld/RunPeriod-2019-01/calib/ver18
#set path_dir=/cache/halld/RunPeriod-2019-01/calib/ver28
set path_dir=/cache/halld/RunPeriod-2019-01/calib/ver28b-batchbatch-v3
echo 'create workflow' $wf
swif create $wf
foreach rnb ( `cat txt/$period.txt` )
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
	#echo "#!/bin/tcsh -f" > $runfile
	#echo "" >> $runfile
	echo "source /work/halld/home/gxproj2/calibration/Env/custom_GlueX_dev.csh" > $runfile
	echo "cd ${out_dir}" >> $runfile
	echo "jcache get $file" >> $runfile
	echo "hd_root --config=${jan_dir}/$jan_cfg $file -o $rootfile" >> $runfile 
	#chmod +x $runfile
	set exec = "source $runfile"
	echo $exec
	swif add-job -workflow $wf -project gluex -track analysis -ram 1GB -time 1h -stdout $outfile -stderr $errfile $exec
   end
end
echo 'run workflow'
swif run $wf
