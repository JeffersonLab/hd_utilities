#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.mc

set DATATYPE=mc
set VERSION=01
set RUNPERIOD=RunPeriod-2016-02
#set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE
set INPUTDIR=/cache/halld/gluex_simulations/sim1/
set INPUT_SMALLFILE_DIR=/work/halld2/mc/$RUNPERIOD  # ignore for now in MC
#set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE/$VERSION
set OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
set ROOTOUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}/rootfiles

# version for sim1.0
set ARGS=" -V 70  --force -E "


# Load standard environment for ROOT
source $HOME/env_monitoring_launch.csh

set MONITORING_HOME=$HOME/monitoring/process
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
    #./process_new_offline_data.py $ARGS $VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR --logfile=$MONITORING_LOGDIR/check_monitoring_data.`date +%F_%T`.log
    ./process_new_offline_data.py $ARGS $DATATYPE $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR 
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif

