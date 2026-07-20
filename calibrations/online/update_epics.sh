#!/bin/bash

export HOME=/home/sdobbs
source /home/sdobbs/work/calibration_train/online/setup_gluex.sh

python /group/halld/Users/sdobbs/epics/check_epics_runs.py -p GlueX_2come
