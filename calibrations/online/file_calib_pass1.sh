#!/bin/bash
# Do a second pass of calibrations on an EVIO file
# Mostly timing calibrations

RUN=$1
EVIOFILE=$2
FILE=001

if [ ! -z "$3" ]; then
    echo changing working directory to $3
    cd $3
fi


source setup_gluex.sh

echo ==START PASS1==
date

export CCDB_CONNECTION=mysql://ccdb_user@hallddb.jlab.org/ccdb    # save results in MySQL
export VARIATION=calib

export JANA_CALIB_CONTEXT="variation=$VARIATION" 
#export JANA_CALIB_CONTEXT="variation=default" 

RUNNUM=`echo ${RUN} | awk '{printf "%d\n",$0;}'`

echo running pass1 ... >> message.txt


# config
#CALIB_PLUGINS=HLDetectorTiming,BCAL_TDC_Timing,CDC_amp
# TODO: Add relevant parts from BCAL_TDC_Timing
CALIB_PLUGINS=HLDetectorTiming,CDC_amp,TOF_TDC_shift
#CALIB_PLUGINS=HLDetectorTiming,TOF_TDC_shift
#CALIB_PLUGINS=HLDetectorTiming
#CALIB_OPTIONS="  -Pjana:nevents=500000 -PTRKFIT:HYPOTHESES_POSITIVE=8 -PTRKFIT:HYPOTHESES_NEGATIVE=9 -PPID:OUT_OF_TIME_CUT=50 "
CALIB_OPTIONS="  -Pjana:nevents=500000 -PTRKFIT:HYPOTHESES_POSITIVE=8 -PTRKFIT:HYPOTHESES_NEGATIVE=9 "
PASS2_OUTPUT_FILENAME=hd_calib_pass1_Run${RUN}.root
# run
echo ==second pass==
echo Running these plugins: $CALIB_PLUGINS
timeout 7200 hd_root -Pnthreads=$NTHREADS  -PEVIO:RUN_NUMBER=${RUNNUM} -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS2_OUTPUT_FILENAME -PPLUGINS=$CALIB_PLUGINS $CALIB_OPTIONS $EVIOFILE
retval=$?

echo ==done==
date

exit $retval
