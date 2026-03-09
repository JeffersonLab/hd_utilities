#!/bin/bash

export LOCKFILE=lock.online
cd /home/sdobbs/work/calibration_train/online

export HOME=/home/sdobbs
source setup_gluex.sh

# for logbook entries
export JAVA_HOME=/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/


#env >& log/env.`date +%F_%T`.log

# run job
if [ ! -f $LOCKFILE ]; then
    touch $LOCKFILE
    #python run_prompt_calibrations.py  60550-60851   >& log/calib.`date +%F_%T`.log
    #python run_prompt_calibrations.py  70995-71124 |& tee log/calib.`date +%F_%T`.log
    #python run_prompt_calibrations.py --no-field 81262- |& tee log/calib.`date +%F_%T`.log
    python3 run_prompt_calibrations.py 131593-  |& tee log/calib.`date +%F_%T`.log
    #python run_prompt_calibrations.py  71500- 
    rm -f $LOCKFILE

    # send update email
    if [ -f "message.txt" ]; then
	cp -v message.txt /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations/
    	cd /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations
    	/group/halld/Users/sdobbs/simple_email_list/scripts/simple_email_list.pl
    fi

else
    echo "process is locked by another job, exiting..."
fi
