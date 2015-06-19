#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set VERSION=ver17
set INPUTDIR=/volatile/halld/offline_monitoring/RunPeriod-2014-10
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-2014-10/ver17
#set ARGS=" -v RunPeriod-2014-10,17 -b 2438 -e 2440 --force "
#set ARGS=" -v RunPeriod-2014-10,17 -R 2438 --force"
set ARGS=" -v RunPeriod-2014-10,17 "
#set ARGS=""

# Load standard environment for ROOT
source /home/gluex/setup_jlab.csh

#set MONITORING_HOME=/home/gxproj1/halld/jproj/projects/offline_monitoring_RunPeriod2014_10_ver13_hd_rawdata
set MONITORING_HOME=/home/gluex/halld/monitoring/process
source $MONITORING_HOME/monitoring_env.csh
set MONITORING_LIBDIR=$MONITORING_HOME/lib
set MONITORING_LOGDIR=$MONITORING_HOME/log

# log file handling
set LOGNAME=check_monitoring_data.log.`date +%F_%T`

# make log directory if it doesn't exist
if (! -d $MONITORING_LOGDIR ) then
    mkdir -p $MONITORING_LOGDIR
endif

# delete logs that are older than 30 days
#if (-d $MONITORING_LOGDIR && $MONITORING_LOGDIR != "/log" ) then
#    find $MONITORING_LOGDIR -mtime +30 -exec rm '{}' \;
#endif

# run the script
cd $MONITORING_HOME

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    ./process_new_offline_data.py $ARGS $VERSION $INPUTDIR $OUTPUTDIR --logfile=$MONITORING_LOGDIR/check_monitoring_data.`date +%F_%T`.log
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif

