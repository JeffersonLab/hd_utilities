#!/bin/bash
# check to see if there was an overall 2ns shift

RUN=$1
FILE=$2

if [ ! -z "$3" ]; then
    echo changing working directory to $3
    cd $3
fi


source setup_gluex.sh

echo ==START PASS1==
date

export JANA_CALIB_URL=mysql://ccdb_user@hallddb.jlab.org/ccdb
export CCDB_CONNECTION=mysql://ccdb_user@hallddb.jlab.org/ccdb    # save results in MySQL
export VARIATION=calib

export JANA_CALIB_CONTEXT="variation=$VARIATION" 
#export JANA_CALIB_CONTEXT="variation=default" 

RUNNUM=`echo ${RUN} | awk '{printf "%d\n",$0;}'`

echo ==running 2ns check==
echo running 2ns check ... >> message.txt

# copy input file to local disk - SWIF only sets up a symbolic link to it
#mv data.evio data_link.evio
#cp -v data_link.evio data.evio

# config
CALIB_PLUGINS=HLDetectorTiming
CALIB_OPTIONS="  -PEVENTS_TO_KEEP=200000 -PHLDETECTORTIMING:NO_TRACKS=1 "
PASS2_OUTPUT_FILENAME=hd_calib_2ns_check_Run${RUN}_${FILE}.root
# run
echo Running these plugins: $CALIB_PLUGINS
timeout 900 hd_root --nthreads=$NTHREADS  -PEVIO:RUN_NUMBER=${RUNNUM} -PJANA:BATCH_MODE=1 -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS2_OUTPUT_FILENAME -PPLUGINS=$CALIB_PLUGINS $CALIB_OPTIONS ./data/hd_rawdata_${RUN}_${FILE}.evio
retval=$?

# now check for correction
python fix_all_base_offset-primex.py ${RUN} ${PASS2_OUTPUT_FILENAME}  >> message.txt
python fix_all_base_offset-primex.py -V calib ${RUN} ${PASS2_OUTPUT_FILENAME}  >> message.txt


echo ==done==
date

exit $retval
