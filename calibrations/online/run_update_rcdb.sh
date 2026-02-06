#!/bin/bash

export HOME=/home/sdobbs
source /home/sdobbs/work/calibration_train/online/setup_gluex.sh

export  PYTHONPATH=/gapps/pyepics/pyepics-3.2.1/lib/python2.7/site-packages:$PYTHONPATH

cd /gluonwork1/Users/sdobbs/calibration_train/online
python3 update_rcdb.py 131593-
