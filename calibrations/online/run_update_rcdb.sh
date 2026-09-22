#!/bin/bash

#export HOME=/home/sdobbs

source /gluex/software/gluex_RHEL95_top/gluex_env_boot.sh
gxenv

export  PYTHONPATH=/gapps/pyepics/pyepics-3.2.1/lib/python2.7/site-packages:$PYTHONPATH

cd /gluex/builds/devel/$BMS_OSNAME/bin
python3 update_rcdb.py $1-
