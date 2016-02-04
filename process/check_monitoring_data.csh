#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set VERSION=25
set RUNPERIOD=RunPeriod-2015-03
set INPUTDIR=/volatile/halld/offline_monitoring/$RUNPERIOD
set OUTPUTDIR=/work/halld/data_monitoring/$RUNPERIOD/ver$VERSION
set ARGS=" -S -v $RUNPERIOD,$VERSION "
#set ARGS=" -R 3185 -S -v $RUNPERIOD,$VERSION "
#set ARGS=" --force -d -s -S -v RunPeriod-2015-03,$VERSION "
#set ARGS=" -v RunPeriod-2015-03,$VERSION "
#set ARGS=" --force -S -v RunPeriod-2015-03,$VERSION "
#set ARGS=" -v RunPeriod-2015-03,10 "

# Load standard environment for ROOT
#source /home/gxproj5/halld/hdswif/setup_jlab-2015-03.csh
source /home/gxproj5/env_monitoring_launch

set MONITORING_HOME=/home/gxproj5/monitoring/process
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
    ./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $OUTPUTDIR --logfile=$MONITORING_LOGDIR/check_monitoring_data.`date +%F_%T`.log
    #./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $OUTPUTDIR
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif

