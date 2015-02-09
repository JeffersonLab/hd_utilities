#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set JOBDATE=ver10
set INPUTDIR=/volatile/halld/offline_monitoring/RunPeriod-2014-10
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-2014-10/ver10
set ARGS=" -v RunPeriod-2014-10,10 "
#set ARGS="--force"
#set ARGS="--force -d"
#set ARGS="--force -d -b 2439"

# Load standard environment for ROOT
source /home/gluex/setup_jlab_commissioning.csh

set MONITORING_HOME=/home/gluex/halld/monitoring/process
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

