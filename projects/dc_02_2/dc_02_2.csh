#!/bin/csh
limit stacksize unlimited
set project=$1
set run=$2
set file=$3
echo processing project $project run $run file $file
cp -v /home/gluex/halld/data_challenge/02/conditions/* .
source setup_jlab.csh
echo ==environment==
printenv
echo ==copy in input==
cp -v /volatile/halld/home/gluex/proj/dc_02_1/smeared/hdgeant_smeared_${run}_${file}.hddm hdgeant_smeared.hddm
hd_root -PPLUGINS=monitoring_hists,danarest -PJANA:BATCH_MODE=1 -PHDDM:USE_COMPRESSION=0 hdgeant_smeared.hddm
echo ls -l after hd_root
ls -l
echo copy rest and hd_root
set rest_dir=/volatile/halld/home/gluex/proj/$project/rest
mkdir -p $rest_dir
cp -v dana_rest.hddm $rest_dir/dana_rest_${run}_${file}.hddm
set hd_root_dir=/volatile/halld/home/gluex/proj/$project/hd_root
mkdir -p $hd_root_dir
cp -v hd_root.root $hd_root_dir/hd_root_${run}_${file}.root
/home/gluex/halld/jproj_dc_01/scripts/move_log_files.sh $AUGER_ID /w/work/halld/data_challenge/${project}/logs
exit
