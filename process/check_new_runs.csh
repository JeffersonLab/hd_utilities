#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.online

# Load standard environment for ROOT
#source /home/gluex/setup_jlab.csh
pushd .
cd
source /home/gluex/build.csh
popd

#set MONITORING_HOME=/home/gxproj1/halld/monitoring/process
set MONITORING_HOME=/home/gluex/halld/monitoring/process
set MONITORING_LIBDIR=$MONITORING_HOME/lib
set MONITORING_LOGDIR=$MONITORING_HOME/log
source $MONITORING_HOME/monitoring_env.csh

# run the script
cd $MONITORING_HOME

# manage log files
if( ! -d $MONITORING_LOGDIR ) then
    mkdir -p $MONITORING_LOGDIR/
endif

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    ./check_new_runs.py --logfile=$MONITORING_LOGDIR/check_new_runs.`date +%F_%T`.log
    #./check_new_runs.py --force
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif
