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
echo ==run bggen==
cp -v run.ffr.template run.ffr
gsr.pl '<random_number_seed>' $file run.ffr
gsr.pl '<run_number>' $run run.ffr
if ( $run == 09001 ) then
    gsr.pl '<number_of_events>' 30000 run.ffr
else if ( $run == 09002 ) then
    gsr.pl '<number_of_events>' 9000 run.ffr
else if ( $run == 09003 ) then
    gsr.pl '<number_of_events>' 120000 run.ffr
endif
rm -f fort.15
ln -s run.ffr fort.15
bggen
echo ==ls -l after bggen==
ls -l
echo ==run hdgeant==
set run4=`echo $run | perl -n -e 'printf "%4d", $_;'`
ln -s control.in_$run4 control.in
hdgeant
echo ==ls -l after hdgeant==
ls -l
echo ==run mcsmear==
mcsmear -PJANA:BATCH_MODE=1 -PTHREAD_TIMEOUT_FIRST_EVENT=300 \
    -PTHREAD_TIMEOUT=300 -PNTHREADS=1 hdgeant.hddm
echo ls -l after mcsmear
ls -l
echo ==remove hdgeant.hddm==
rm -v hdgeant.hddm
#echo ==copy smeared==
#mkdir -p /volatile/halld/home/gluex/proj/$project/smeared
#cp -v hdgeant_smeared.hddm /volatile/halld/home/gluex/proj/$project/smeared/hdgeant_smeared_${run}_${file}.hddm
echo ==run hd_root==
hd_root -PJANA:BATCH_MODE=1 -PTHREAD_TIMEOUT_FIRST_EVENT=300 \
    -PTHREAD_TIMEOUT=300 -PNTHREADS=1 -PPLUGINS=monitoring_hists,danarest \
    -PHDDM:USE_COMPRESSION=1 hdgeant_smeared.hddm
echo ==ls -l after hd_root==
ls -l
echo ==copy rest and hd_root==
set rest_dir=/volatile/halld/home/gluex/proj/$project/rest
mkdir -p $rest_dir
cp -v dana_rest.hddm $rest_dir/dana_rest_${run}_${file}.hddm
set hd_root_dir=/volatile/halld/home/gluex/proj/$project/hd_root
mkdir -p $hd_root_dir
cp -v hd_root.root $hd_root_dir/hd_root_${run}_${file}.root
#echo ==submit move job==
#/home/gluex/halld/jproj/scripts/move_log_files.sh $AUGER_ID /w/work/halld/data_challenge/${project}/logs
echo ==exit==
exit
