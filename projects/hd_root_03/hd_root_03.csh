#!/bin/csh
limit stacksize unlimited
set run=$1
set file=$2
echo processing run $run file $file
source /home/gluex/halld/build_scripts/gluex_env_jlab.csh
hd_root --nthreads=Ncores -PPLUGINS=phys_tree,danarest bggen_hdgeant_smeared_${1}_${2}.hddm
echo ls -l
ls -l
echo copy
mkdir -p /volatile/halld/home/marki/proj/hd_root_03
cp -v dana_rest.hddm /volatile/halld/home/marki/proj/hd_root_03/dana_rest_${run}_${file}.hddm
echo ==environment==
printenv
exit
