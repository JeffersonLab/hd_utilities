#!/bin/tcsh

# parse command line arguments
setenv RUNPERIOD $1
if( $RUNPERIOD == "" ) then
    echo Need to pass run period as first argument!
    exit 1
endif

setenv VERSION $2
if( $VERSION == "" ) then
    echo Need to pass data version as second argument!
    exit 1
endif

setenv RUN $3
if( $RUN == "" ) then
    echo Need to pass run number as third argument!
    exit 1
endif

# set up job
set INPUTDIR=/volatile/halld/offline_monitoring/$RUNPERIOD
set OUTPUTDIR=/work/halld/data_monitoring/$RUNPERIOD/ver$VERSION
set ARGS=" -R $RUN -S -v $RUNPERIOD,$VERSION "

# Load standard environment for ROOT
source /home/gxproj5/env_monitoring_launch

set MONITORING_HOME=/home/gxproj5/monitoring/process
source $MONITORING_HOME/monitoring_env.csh
set MONITORING_LIBDIR=$MONITORING_HOME/lib
set MONITORING_LOGDIR=$MONITORING_HOME/log

# run the script
cd $MONITORING_HOME

./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $OUTPUTDIR
