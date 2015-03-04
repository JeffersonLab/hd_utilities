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
cp run.ffr.template run.ffr
gsr.pl '<random_number_seed>' $file run.ffr
gsr.pl '<number_of_events>' 1000 run.ffr
gsr.pl '<run_number>' $run run.ffr
rm -f fort.15
ln -s run.ffr fort.15
bggen
echo ls -l after bggen
ls -l
set run4=`echo $run | perl -n -e 'printf "%4d", $_;'`
cp -v control.in_$run4 control.in
hdgeant
echo ls -l after hdgeant
ls -l
mcsmear -PJANA:BATCH_MODE=1 -PJANA:BATCH_MODE=1 hdgeant.hddm
echo ls -l after mcsmear
ls -l
echo copy smeared
mkdir -p /volatile/halld/home/gluex/proj/$project/smeared
cp -v hdgeant_smeared.hddm /volatile/halld/home/gluex/proj/$project/smeared/hdgeant_smeared_${run}_${file}.hddm
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
