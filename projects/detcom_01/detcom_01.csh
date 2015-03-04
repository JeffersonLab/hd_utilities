#!/bin/csh
limit stacksize unlimited
set project=$1
set run=$2
set file=$3
echo processing project $project run $run file $file
cp -pv /home/gluex/halld/detcom/01/conditions/* .
source setup_jlab.csh
#
# set number of events
#
set number_of_events = 25000000
#
# set flag based on run number
#
@ runno = `echo $run | awk '{print $1 + 0}'`
if ($runno > 9103) then
    set em = 1
else
    set em = 0
endif
#
echo ==start run==
date
echo ==environment==
printenv
if (! $em) then
    echo ==run bggen==
    cp -v run.ffr.template run.ffr
    gsr.pl '<random_number_seed>' $file run.ffr
    gsr.pl '<run_number>' $run run.ffr
    gsr.pl '<number_of_events>' $number_of_events run.ffr
    rm -f fort.15
    ln -s run.ffr fort.15
    bggen
    echo ==ls -l after bggen==
    ls -l
endif
echo ==run hdgeant==
set run4=`echo $run | perl -n -e 'printf "%4d", $_;'`
rm -f control.in
cp -v control.in_$run4 control.in
if ($em) gsr.pl '<number_of_events>' $number_of_events control.in
set command = hdgeant
echo command = $command
$command
echo ==ls -l after hdgeant==
ls -l
echo ==run mcsmear==
set command = "mcsmear -PJANA:BATCH_MODE=1 -PTHREAD_TIMEOUT=300 -PNTHREADS=1 hdgeant.hddm"
echo command = $command
$command
echo ls -l after mcsmear
ls -l
echo ==run hd_ana to make evio output==
set command = "hd_ana -PPLUGINS=rawevent -PJANA:BATCH_MODE=1 -PTHREAD_TIMEOUT=300 -PNTHREADS=1 hdgeant_smeared.hddm"
echo command = $command
$command
echo ==run hd_root==
# set the bfield map
if ( $run == 09101 ) then
    set bfield_option = -PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520
else if ( $run == 09102 ) then
    set bfield_option = -PBFIELD_TYPE=NoField
else if ( $run == 09104 ) then
    set bfield_option = -PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520
else if ( $run == 09105 ) then
    set bfield_option = -PBFIELD_TYPE=NoField
else
    echo illegal run number in detcom_01.csh, run = $run
    exit 1
endif
# set plugins list
if ($em) then
    set plugins_option = -PPLUGINS=monitoring_hists,CDC_online,FDC_online,ST_online,TOF_online,FCAL_online,BCAL_online
else
    set plugins_option = -PPLUGINS=monitoring_hists,danarest
endif
set command = "hd_root -PJANA:BATCH_MODE=1 -PTHREAD_TIMEOUT=300 -PNTHREADS=1 \
    $plugins_option $bfield_option \
    hdgeant_smeared.hddm" 
echo command = $command
$command
echo ==ls -l after hd_root==
ls -l
echo ==copy output files to disk==
#set hdgeant_dir=/volatile/halld/$project/hdgeant
#mkdir -p $hdgeant_dir
#cp -v hdgeant.hddm $hdgeant_dir/hdgeant_${run}_${file}.hddm
set smeared_dir=/volatile/halld/$project/smeared
mkdir -p $smeared_dir
cp -v hdgeant_smeared.hddm $smeared_dir/hdgeant_smeared_${run}_${file}.hddm
set evio_dir=/volatile/halld/$project/evio
mkdir -p $evio_dir
cp -v rawevent_0${run}.evio $evio_dir/rawevent_${run}_${file}.evio
if (! $em) then
    set rest_dir=/volatile/halld/$project/rest
    mkdir -p $rest_dir
    cp -v dana_rest.hddm $rest_dir/dana_rest_${run}_${file}.hddm
endif
set hd_root_dir=/volatile/halld/$project/hd_root
mkdir -p $hd_root_dir
cp -v hd_root.root $hd_root_dir/hd_root_${run}_${file}.root
echo ==end run==
date
echo ==exit==
exit
