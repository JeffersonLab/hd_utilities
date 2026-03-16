#!/bin/bash

#export HOME=/home/sdobbs
#source /home/sdobbs/work/calibration_train/online/setup_gluex.sh

source /gluex/software/gluex_RHEL95_top/gluex_env_boot.sh
gxenv

cd /gluex/builds/devel/$BMS_OSNAME/bin
# update run range for a new run period
python3 update_skim_files.py $1- 
