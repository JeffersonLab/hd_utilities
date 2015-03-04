#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set JOBDATE=ver12
set INPUTDIR=/volatile/halld/offline_monitoring/RunPeriod-2014-10
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-2014-10/ver12
set ARGS=" -v RunPeriod-2014-10,12 "
#set ARGS="--force -d -v RunPeriod-2014-10,12 "
#set ARGS="--force"
#set ARGS="--force -d"
#set ARGS="--force -d -b 2439"
#set ARGS=""

# Load standard environment for ROOT
source /home/gxproj1/setup_jlab.csh

set MONITORING_HOME=/home/gxproj1/halld/jproj/projects/offline_monitoring_RunPeriod2014_10_ver12_hd_rawdata
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

