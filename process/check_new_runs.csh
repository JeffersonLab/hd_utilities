#!/bin/tcsh
# Set environmental variables for cron job

# Load standard environment for ROOT
source /home/gluex/setup_jlab_commissioning.csh

set MONITORING_HOME=/home/gluex/halld/monitoring/process
source $MONITORING_HOME/monitoring_env.csh

# run the script
cd $MONITORING_HOME
./check_new_runs.py -p -d
