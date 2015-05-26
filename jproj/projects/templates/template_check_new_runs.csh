#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.online

# Load standard environment for ROOT
source PROJHOME/PROJECT/setup_jlab-RUNPERIOD.csh

set MONITORING_HOME=PROJHOME/PROJECT/processing
source $MONITORING_HOME/monitoring_env.csh

# run the script
cd $MONITORING_HOME

if ( ! -e $LOCKFILE ) then
    touch $LOCKFILE
    ./check_new_runs.py -c -p
    rm $LOCKFILE
else 
    echo "process is locked by another job, exiting..."
endif
