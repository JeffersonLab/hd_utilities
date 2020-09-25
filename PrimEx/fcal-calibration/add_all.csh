#!/bin/tcsh -f

set iter=$1
set runs_to_check=$2
set wf=$runs_to_check-$iter-added

swif create $wf

set in_dir=$runs_to_check-$iter/root-files

set work_dir=/work/halld/home/gxproj2/calibration/fcal

set out_dir=$work_dir/$wf

set roodir=$out_dir
mkdir -p $roodir

set rundir=$roodir/run
mkdir -p $rundir

set errdir=$roodir/err
mkdir -p $errdir

set outdir=$roodir/out
mkdir -p $outdir

foreach rnb ( `cat txt/$runs_to_check.txt` )
    echo $rnb
    set ntuple=run_$rnb
    set runfile=$rundir/$ntuple.csh
    set errfile=$errdir/$ntuple.err
    set outfile=$outdir/$ntuple.out
    echo "source /work/halld/home/gxproj2/calibration/Env/custom_GlueX_dev.csh" > $runfile
    echo "cd $work_dir" >> $runfile
    echo "hadd -f $roodir/$ntuple.root $in_dir/*$rnb*.root" >> $runfile
    set exec = "source $runfile"
    echo $exec
    swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile $exec
end
swif run $wf
