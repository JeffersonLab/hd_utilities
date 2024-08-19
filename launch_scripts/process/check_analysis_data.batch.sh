#!/bin/bash

export PHADD_TMPDIR=`pwd`

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
fi

export RUN=$3
if [ -z "$RUN" ]; then
    echo Need to pass run number as third argument!
    exit 1
fi

# configure environment
export DATATYPE=analysis
#export INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE
export INPUTDIR=/volatile/halld/$DATATYPE/$RUNPERIOD
export INPUT_SMALLFILE_DIR=/volatile/halld/$DATATYPE/$RUNPERIOD
#export INPUTDIR=/volatile/halld/home/jrpybus/trees/data/rho_SRC/batch02/
#export INPUT_SMALLFILE_DIR=/volatile/halld/home/jrpybus/trees/data/rho_SRC/batch02/
#export INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE/$VERSION
#export OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
export OUTPUTDIR=/cache/halld/${RUNPERIOD}/${DATATYPE}/ver${VERSION}/hists/merged
export ROOTOUTPUTDIR=/cache/halld/${RUNPERIOD}/${DATATYPE}/ver${VERSION}/hists/merged
export ARGUMENTS=" --force -B -R $RUN -E -p -d --merged-root-output-dir=$ROOTOUTPUTDIR "
# -v $RUNPERIOD,$VERSION

echo "Load standard environment"
source /home/gxproj6/env_analysis_launch.sh

export MONITORING_HOME=/home/gxproj6/hd_utilities/launch_scripts/process
source $MONITORING_HOME/monitoring_env.csh
export MONITORING_LIBDIR=$MONITORING_HOME/lib
export MONITORING_LOGDIR=$MONITORING_HOME/log
export BATCH_TMPDIR=`pwd`

printenv

# run the script
cd $MONITORING_HOME

#which python

echo ./process_new_offline_data.py $ARGUMENTS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
./process_new_offline_data.py $ARGUMENTS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
