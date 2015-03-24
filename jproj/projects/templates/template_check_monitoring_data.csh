#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set JOBDATE=verVERSION
set INPUTDIR=/volatile/halld/offline_monitoring/RunPeriod-RUNPERIOD
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-RUNPERIOD/verVERSION
set ARGS=" -v RunPeriod-RUNPERIOD,VERSION "
#set ARGS="--force -d -v RunPeriod-RUNPERIOD,VERSION "
#set ARGS="--force"
#set ARGS="--force -d"
#set ARGS="--force -d -b 2439"
#set ARGS=""

# Load standard environment for ROOT
source /home/gxproj1/setup_jlab.csh

set MONITORING_HOME=/home/gxproj1/halld/jproj/projects/PROJECT/processing
source $MONITORING_HOME/monitoring_env.csh

# run the script
cd $MONITORING_HOME

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    ./process_new_offline_data.py $ARGS $JOBDATE $INPUTDIR $OUTPUTDIR
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif

