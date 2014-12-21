#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set JOBDATE=2014-12-19
set INPUTDIR=/volatile/halld/RunPeriod-2014-10/offline_monitoring
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-2014-10/ver07
set ARGS=" "
#set ARGS="--force"
#set ARGS="--force -d"

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

