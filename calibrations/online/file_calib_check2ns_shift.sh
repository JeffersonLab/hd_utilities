#!/bin/bash
# check to see if there was an overall 2ns shift

RUN=$1
EVIOFILE=$2

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

echo ==running 2ns check==
echo running 2ns check ... >> message.txt



# config
CALIB_PLUGINS=HLDetectorTiming
CALIB_OPTIONS="  -Pjana:nevents=100000 -PTRKFIT:HYPOTHESES_POSITIVE=8 -PTRKFIT:HYPOTHESES_NEGATIVE=9 " 
PASS2_OUTPUT_FILENAME=hd_calib_2ns_check_Run${RUN}.root
# run
echo Running these plugins: $CALIB_PLUGINS
timeout 900 hd_root -Pnthreads=$NTHREADS  -PEVIO:RUN_NUMBER=${RUNNUM} -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS2_OUTPUT_FILENAME -PPLUGINS=$CALIB_PLUGINS $CALIB_OPTIONS $EVIOFILE
retval=$?

# now check for correction
python3 fix_all_base_offset.py ${RUN} ${PASS2_OUTPUT_FILENAME}  >> message.txt
python3 fix_all_base_offset.py -V calib ${RUN} ${PASS2_OUTPUT_FILENAME}  >> message.txt


echo ==done==
date

exit $retval
