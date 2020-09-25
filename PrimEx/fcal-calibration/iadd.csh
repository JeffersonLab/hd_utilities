#!/bin/tcsh -f
set wf=$1
set period=$2
set work_dir=/work/halld/home/gxproj2/calibration/fcal

set path_dir=$work_dir/$wf
set out_dir=$work_dir/$wf-added
set roodir=$out_dir
mkdir -p $roodir
set rundir=$out_dir/run
mkdir -p $rundir
set errdir=$out_dir/err
mkdir -p $errdir
set outdir=$out_dir/out
mkdir -p $outdir

echo 'create workflow' $wf
swif create $wf
foreach rnb ( `cat txt/$period.txt` )
    set ntuple=run_$rnb
    set runfile=$rundir/$ntuple.csh
    echo $runfile
    #echo "#!/bin/tcsh -f" > $runfile
    echo "source /work/halld/home/gxproj2/calibration/Env/custom_GlueX_dev.csh" > $runfile
    echo "hadd -f $roodir/run_$rnb.root $path_dir/*$rnb*.root" >> $runfile
    set exec = "source $runfile"
    echo $exec	
    #chmod +x $runfile
    set errfile=$errdir/$ntuple.err
    set outfile=$outdir/$ntuple.out
    #swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile $rundir/./$ntuple.csh
    #swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile 'csh -f $runfile'
    #swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile ./$runfile
    swif add-job -workflow $wf -project gluex -track analysis -stdout $outfile -stderr $errfile $exec
end
swif run $wf

