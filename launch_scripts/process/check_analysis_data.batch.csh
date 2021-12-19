#!/bin/tcsh

setenv PHADD_TMPDIR `pwd`

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

# configure environment
set DATATYPE=analysis
#set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE
set INPUTDIR=/work/halld/$DATATYPE/$RUNPERIOD/
set INPUT_SMALLFILE_DIR=/work/halld2/recon/$RUNPERIOD
#set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE/$VERSION
set OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
set ROOTOUTPUTDIR=/cache/halld/${RUNPERIOD}/${DATATYPE}/ver${VERSION}/hists/merged
set ARGS=" --force -B -R $RUN -E -v $RUNPERIOD,$VERSION --merged-root-output-dir=$ROOTOUTPUTDIR "


# Load standard environment for ROOT
source /home/gxproj1/env_monitoring_launch.csh

set MONITORING_HOME=/home/gxproj1/monitoring/process
source $MONITORING_HOME/monitoring_env.csh
set MONITORING_LIBDIR=$MONITORING_HOME/lib
set MONITORING_LOGDIR=$MONITORING_HOME/log
setenv BATCH_TMPDIR `pwd`

# run the script
cd $MONITORING_HOME

./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
