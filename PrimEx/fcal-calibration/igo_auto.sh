# !/bin/bash

iter=$1
period=$2
wfname=$3
wf=$wfname-$period-$iter

jan_cfg=jana_data_pi0.config
jan_dir=/work/halld/home/gxproj2/calibration/fcal
out_dir=$jan_dir
roodir=$out_dir/$wf
mkdir -p $roodir
rundir=$roodir/run
mkdir -p $rundir
errdir=$roodir/err
mkdir -p $errdir
outdir=$roodir/out
mkdir -p $outdir

path=/cache/halld/RunPeriod-2019-01/calib/ver18

echo 'create workflow' $wf
swif create $wf
txtfile=( $( cat txt/$period.txt ) )
for rnb in "${txtfile[@]}"; do
    echo $rnb
    for file in `ls $path/*$rnb*fcal*.evio`; do
	echo $file
	filename=`basename $file`
	rootname=${filename/evio/root}
	rootfile=$roodir/$rootname
	ntuple=${filename/.evio/}
	runfile=$rundir/$ntuple.sh
	errfile=$errdir/$ntuple.err
	outfile=$outdir/$ntuple.out
	#echo "#!bin/bash" > $runfile
	echo "bash" > $runfile
	echo "source /work/halld/home/gxproj2/calibration/Env/custom_GlueX_dev.sh" >> $runfile
	echo "cd ${out_dir}" >> $runfile
	echo "jcache get $file" >> $runfile
	echo "hd_root --config=${jan_dir}/$jan_cfg $file -o $rootfile" >> $runfile 
	chmod +x $runfile
	swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile $rundir/./$ntuple.sh
	h=$(($h+1))
   done
done
echo 'run workflow'
swif run $wf
