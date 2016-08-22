#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.recon.offline

set DATATYPE=recon
set VERSION=01
set RUNPERIOD=RunPeriod-2016-02
set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE
set INPUT_SMALLFILE_DIR=/work/halld2/recon/$RUNPERIOD
#set INPUTDIR=/cache/halld/$RUNPERIOD/$DATATYPE/$VERSION
set OUTPUTDIR=/work/halld2/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}
set ROOTOUTPUTDIR=/work/halld/data_monitoring/${RUNPERIOD}/${DATATYPE}_ver${VERSION}/rootfiles
set ARGS=" -b 11000 -e 11346 -v $RUNPERIOD,$VERSION --merge-trees=tree_bcal_hadronic_eff,tree_fcal_hadronic_eff,tree_sc_eff,tree_tof_eff,tree_trackeff,tree_TS_scaler --merge-skims=BCAL-LED,bigevents,FCAL-LED,sync --merged-root-output-dir=$ROOTOUTPUTDIR "
#set ARGS=" -b 11553 -v $RUNPERIOD,$VERSION --merge-trees=tree_bcal_hadronic_eff,tree_fcal_hadronic_eff,tree_sc_eff,tree_tof_eff,tree_trackeff,tree_TS_scaler --merge-skims=BCAL-LED,bigevents,FCAL-LED,sync "
#set ARGS=" -v $RUNPERIOD,$VERSION  -d -s -E"
#set ARGS=" -R 3185 -S -v $RUNPERIOD,$VERSION "
#set ARGS=" --force -d -s -S -v RunPeriod-2015-03,$VERSION "
#set ARGS=" -v RunPeriod-2015-03,$VERSION "
#set ARGS=" --force -S -v RunPeriod-2015-03,$VERSION "
#set ARGS=" -v RunPeriod-2015-03,10 "

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
if (-d $MONITORING_LOGDIR && $MONITORING_LOGDIR != "/log" ) then
    find $MONITORING_LOGDIR -mtime +30 -exec rm '{}' \;
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

