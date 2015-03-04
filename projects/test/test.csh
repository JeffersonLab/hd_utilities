#!/bin/csh
limit stacksize unlimited
set project=$1
set run=$2
set file=$3
echo processing project $project run $run file $file
date > out.dat
set out_dir=/volatile/halld/home/marki/proj/$project/out
mkdir -p $out_dir
cp -v out_dir $out_dir/out_${run}_${file}.dat
/home/gluex/halld/jproj/scripts/move_log_files.sh $AUGER_ID /w/work/halld/home/marki/proj/${project}
exit
