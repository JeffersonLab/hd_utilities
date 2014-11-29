#!/bin/tcsh
# Set environmental variables for cron job
set JOBDATE=2014-11-28
set INPUTDIR=/volatile/halld/RunPeriod-2014-10/offline_monitoring
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-2014-10/ver04

echo ==loading environment==
# Load standard environment for ROOT
source /home/gluex/setup_jlab_commissioning.csh

echo ==loading monitoring env==
set MONITORING_HOME=/home/gluex/halld/monitoring/process
source $MONITORING_HOME/monitoring_env.csh

echo ==running script==
# run the script
cd $MONITORING_HOME
./process_new_offline_data.py $JOBDATE $INPUTDIR $OUTPUTDIR

