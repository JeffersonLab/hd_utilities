#!/bin/csh
limit stacksize unlimited
set run=$1
set file=$2
echo processing run $run file $file
source /home/gluex/halld/build_scripts/gluex_env_jlab.csh
eval `addpath.pl $HALLD_MY/bin/$BMS_OSNAME`
cp -v /home/marki/halld/bggen/run/* .
cp run.ffr.template run.ffr
gsr.pl '<random_number_seed>' $AUGER_ID run.ffr
rm -f fort.15
ln -s run.ffr fort.15
bggen
echo ls -l
ls -l
hdgeant
echo ls -l
ls -l
mcsmear hdgeant.hddm
echo ls -l
ls -l
echo copy
cp -v hdgeant_smeared.hddm /volatile/halld/home/marki/proj/bggen/bggen_hdgeant_smeared_${run}_${file}.hddm
echo ==environment==
printenv
exit
