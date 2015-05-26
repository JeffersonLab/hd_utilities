#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.offline

set JOBDATE=verVERSION
set INPUTDIR=/volatile/halld/offline_monitoring/RunPeriod-RUNPERIOD
set OUTPUTDIR=/w/halld-scifs1a/data_monitoring/RunPeriod-RUNPERIOD/verVERSION
set ARGS=" -v RunPeriod-RUNPERIOD,VERSION "
#set ARGS=" --force -p  -v RunPeriod-RUNPERIOD,VERSION -b 2438 "
#set ARGS="--force -d -v RunPeriod-RUNPERIOD,VERSION "
#set ARGS="--force"
#set ARGS="--force -d"
#set ARGS="--force -d -b 2439"
#set ARGS=""

# Load standard environment for ROOT
source PROJHOME/PROJECT/setup_jlab-RUNPERIOD.csh

#set MONITORING_HOME=/home/gxproj1/halld/jproj/projects/offline_monitoring_RunPeriod2014_10_verVERSION_hd_rawdata
set MONITORING_HOME=PROJHOME/PROJECT/processing
source $MONITORING_HOME/monitoring_env.csh
set MONITORING_LIBDIR=$MONITORING_HOME/lib
mkdir -p $MONITORING_LIBDIR
set MONITORING_LOGDIR=$MONITORING_HOME/log

# log file handling
set LOGNAME=check_monitoring_data.log.`date +%F_%T`

# make directory if it doesn't exist
if (! -d $MONITORING_LOGDIR ) then
    mkdir -p $MONITORING_LOGDIR
endif

# delete logs that are older than 30 days
if (-d $MONITORING_LOGDIR && $MONITORING_LOGDIR != "/log" ) then
    find $MONITORING_LOGDIR -mtime +30 -exec rm '{}' \;
endif



# run the script
cd $MONITORING_HOME

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    ./process_new_offline_data.py $ARGS $JOBDATE $INPUTDIR $OUTPUTDIR |& tee $MONITORING_LOGDIR/$LOGNAME
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif

