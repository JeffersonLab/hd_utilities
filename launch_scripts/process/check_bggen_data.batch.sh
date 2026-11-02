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
fi

export RUN=$3
if [ -z "$RUN" ]; then
    echo Need to pass run number as third argument!
    exit 1
fi

# configure environment
export DATATYPE=mc
export INPUTDIR=/cache/cache/halld/gluex_simulations/REQUESTED_MC/2019-11_bggen2026_batch01_4906/root/
export INPUT_SMALLFILE_DIR=/cache/cache/halld/gluex_simulations/REQUESTED_MC/2019-11_bggen2026_batch01_4906/root/
export OUTPUTDIR=/work/halld2/home/aaustreg/Analysis/sw/my_halld_el9/hd_utilities/launch_scripts/process/output_again
export ROOTOUTPUTDIR=/work/halld2/home/aaustreg/Analysis/sw/my_halld_el9/hd_utilities/launch_scripts/process/rootfiles_again
export ARGS=" --force -B -R $RUN -E -d -T $ROOTOUTPUTDIR"
# tree_scaler_primex,

# Load standard environment for ROOT
source /group/halld/Software/build_scripts/gluex_env_jlab.sh /group/halld/www/halldweb/html/halld_versions/version.xml

export MONITORING_HOME=/work/halld2/home/aaustreg/Analysis/sw/my_halld_el9/hd_utilities/launch_scripts/process/
source $MONITORING_HOME/monitoring_env.sh
export MONITORING_LIBDIR=$MONITORING_HOME/lib
export MONITORING_LOGDIR=$MONITORING_HOME/log
export BATCH_TMPDIR=`pwd`

# run the script
cd $MONITORING_HOME

./process_new_offline_data.py $ARGS $DATATYPE $INPUTDIR $INPUT_SMALLFILE_DIR $OUTPUTDIR
