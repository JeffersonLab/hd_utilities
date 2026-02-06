#!/bin/bash

export HOME=/home/sdobbs
source /home/sdobbs/work/calibration_train/online/setup_gluex.sh

cd /gluonwork1/Users/sdobbs/calibration_train/online
python3 update_skim_files.py 131593-
