#!/bin/tcsh
# Set environmental variables for cron job

# Load standard environment for ROOT
source /u/home/gluex/setup_jlab_commissioning.csh

set MONITORING_HOME /u/home/gluex/halld/monitoring/process
source $MONITORING_HOME/monitoring_env.sh

# run the script
cd $MONITORING_HOME
./check_new_runs.py
