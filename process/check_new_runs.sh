#!/bin/tcsh
# Set environmental variables for cron job

# Load standard environment for ROOT
source /u/home/gluex/halld/setup_jlab_commissioning.csh

# load ROOT python bindings
setenv PYTHONPATH $ROOTSYS/lib

# need python 2.7 for MySQL bindings
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

# run the script
cd /u/home/gluex/halld/monitoring/process
./check_new_runs.py