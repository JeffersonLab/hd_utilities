#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.online

#set MONITORING_HOME=/home/gxproj1/halld/monitoring/process
set MONITORING_HOME=/home/gluex/halld/monitoring/process
set MONITORING_LIBDIR=$MONITORING_HOME/lib
set MONITORING_LOGDIR=$MONITORING_HOME/log

# Load standard environment for ROOT
#source /home/gluex/setup_jlab.csh
cd $MONITORING_HOME
source setup_jlab.csh
source $MONITORING_HOME/monitoring_env.csh

# run the script
#cd $MONITORING_HOME

# manage log files
if( ! -d $MONITORING_LOGDIR ) then
    mkdir -p $MONITORING_LOGDIR/
endif

echo ==which perl==
which perl
echo ==environment==
env

# clean out old logfiles
find log/ -mtime +90 -exec rm -f '{}' \;

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    #./check_new_runs.py 
    ./check_new_runs.py --logfile=$MONITORING_LOGDIR/check_new_runs.`date +%F_%T`.log
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif
