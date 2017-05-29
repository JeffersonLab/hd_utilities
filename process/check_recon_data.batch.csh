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
set DATATYPE=recon
set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE
set INPUT_SMALLFILE_DIR=/work/halld2/recon/$RUNPERIOD
#set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE/$VERSION
set OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
set ROOTOUTPUTDIR=/work/halld/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}/rootfiles
set ARGS=" --force -B -R $RUN -E -v $RUNPERIOD,$VERSION --merge-trees=tree_bcal_hadronic_eff,tree_fcal_hadronic_eff,tree_sc_eff,tree_tof_eff,tree_trackeff,tree_TS_scaler --merge-skims=BCAL-LED,bigevents,FCAL-LED,sync --merged-root-output-dir=$ROOTOUTPUTDIR "


# Load standard environment for ROOT
source /home/gxproj5/env_monitoring_launch.csh

set MONITORING_HOME=/home/gxproj5/monitoring/process
source $MONITORING_HOME/monitoring_env.csh
set MONITORING_LIBDIR=$MONITORING_HOME/lib
set MONITORING_LOGDIR=$MONITORING_HOME/log
setenv BATCH_TMPDIR `pwd`

# run the script
cd $MONITORING_HOME

./process_new_offline_data.py $ARGS ver$VERSION $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
