#!/bin/tcsh
# Export environmental variables for cron job
export LOCKFILE=lock.offline

export DATATYPE=mon
export VERSION=07
export RUNPERIOD=RunPeriod-2016-02
export INPUTDIR=/cache/halld/offline_monitoring/$RUNPERIOD
export INPUT_SMALLFILE_DIR=/cache/halld/offline_monitoring/$RUNPERIOD
#export OUTPUTDIR=/work/halld/data_monitoring/$RUNPERIOD/ver$VERSION
export OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
export ROOTOUTPUTDIR=/work/halld/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}/rootfiles
export ARGS=" -T $ROOTOUTPUTDIR -v $RUNPERIOD,$VERSION  "

# Load standard environment for ROOT
source /home/gxproj5/env_monitoring_launch.sh

export MONITORING_HOME=/home/gxproj5/monitoring/process
source $MONITORING_HOME/monitoring_env.sh
export MONITORING_LIBDIR=$MONITORING_HOME/lib
export MONITORING_LOGDIR=$MONITORING_HOME/log

# log file handling
export LOGNAME=check_monitoring_data.log.`date +%F_%T`

# make log directory if it doesn't exist
if (! -d $MONITORING_LOGDIR ) then
    mkdir -p $MONITORING_LOGDIR
endif

# delete logs that are older than 30 days
if (-d $MONITORING_LOGDIR && $MONITORING_LOGDIR != "/log" ) then
    find $MONITORING_LOGDIR/ -mtime +30 -exec rm '{}' \;
endif

# run the script
cd $MONITORING_HOME

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    ./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR --logfile=$MONITORING_LOGDIR/check_monitoring_data.`date +%F_%T`.log
    #./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif

