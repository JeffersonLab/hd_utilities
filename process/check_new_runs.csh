#!/bin/tcsh
# Set environmental variables for cron job
set LOCKFILE=lock.online

# Load standard environment for ROOT
source /home/gxproj1/setup_jlab.csh

set MONITORING_HOME=/home/gxproj1/halld/monitoring/process
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
