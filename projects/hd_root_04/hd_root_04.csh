#!/bin/csh
limit stacksize unlimited
set project=$1
set run=$2
set file=$3
echo processing run $run file $file
source /home/marki/gluex/setup.csh
echo ==environment==
printenv
hd_root -PPLUGINS=phys_tree,danarest bggen_hdgeant_smeared_${run}_${file}.hddm
echo ls -l
ls -l
echo copy
set rest_dir=/volatile/halld/home/marki/proj/$project/rest
mkdir -p $rest_dir
cp -v dana_rest.hddm $rest_dir/dana_rest_${run}_${file}.hddm
set hd_root_dir=/volatile/halld/home/marki/proj/$project/hd_root
mkdir -p $hd_root_dir
cp -v hd_root.root $hd_root_dir/hd_root_${run}_${file}.root
/home/marki/halld/jproj/scripts/move_log_files.sh $AUGER_ID /w/work/halld/home/marki/proj/${project}
exit
