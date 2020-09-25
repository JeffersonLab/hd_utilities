# !/bin/bash

wf=$1
period=$2
work_dir=/work/halld/home/gxproj2/calibration/fcal

path=$work_dir/$wf
out_dir=$work_dir/$wf-added
roodir=$out_dir
mkdir -p $roodir
rundir=$out_dir/run
mkdir -p $rundir
errdir=$out_dir/err
mkdir -p $errdir
outdir=$out_dir/out
mkdir -p $outdir

echo 'create workflow' $wf
swif create $wf
txtfile=( $( cat txt/$period.txt ) )
for rnb in "${txtfile[@]}"; do
    ntuple=run_$rnb
    runfile=$rundir/$ntuple.sh
    echo $runfile
    #echo "#!bin/bash" > $runfile
    echo "bash" > $runfile
    echo "source /work/halld/home/gxproj2/calibration/Env/custom_GlueX_dev.sh" >> $runfile
    echo "hadd -f $roodir/run_$rnb.root $path/*$rnb*.root" >> $runfile
    chmod +x $runfile
    errfile=$errdir/$ntuple.err
    outfile=$outdir/$ntuple.out
    swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile $rundir/./$ntuple.sh
done
swif run $wf

