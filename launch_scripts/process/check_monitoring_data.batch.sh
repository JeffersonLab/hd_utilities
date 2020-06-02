#!/bin/bash

# parse command line arguments
export RUNPERIOD=$1
if [ -z "$RUNPERIOD" ]; then
    echo Need to pass run period as first argument!
    exit 1
fi

export VERSION=$2
if [ -z "$VERSION" ]; then
    echo Need to pass data version as second argument!
    exit 1
endif

export RUN=$3
if [ -z "$RUN" ]; then
    echo Need to pass run number as third argument!
    exit 1
endif

# configure environment
export DATATYPE=mon  # monitoring data
export INPUTDIR=/cache/halld/offline_monitoring/$RUNPERIOD
export INPUT_SMALLFILE_DIR=/cache/halld/offline_monitoring/$RUNPERIOD
export OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
export ROOTOUTPUTDIR=/work/halld/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}/rootfiles
export ARGS=" -R $RUN -E -T $ROOTOUTPUTDIR -v $RUNPERIOD,$VERSION  "

# Load standard environment for ROOT
source /home/gxproj5/env_monitoring_launch.sh

export MONITORING_HOME=/home/gxproj5/monitoring/process
source $MONITORING_HOME/monitoring_env.sh
export MONITORING_LIBDIR=$MONITORING_HOME/lib
export MONITORING_LOGDIR=$MONITORING_HOME/log

# run the script
cd $MONITORING_HOME

./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
