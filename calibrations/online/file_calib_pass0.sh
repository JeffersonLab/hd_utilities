#!/bin/bash 
# Do RF calibrations which only need a small number of events

RUN=$1
EVIOFILE=$2

source setup_gluex.sh

echo ==START PASS0==
date


# initialize CCDB before running - should get this working at some point?
#cp ${BASEDIR}/ccdb_start.sqlite ccdb.sqlite
export CCDB_CONNECTION=mysql://ccdb_user@hallddb.jlab.org/ccdb    # save results in MySQL
export VARIATION=calib

# Start by running over the current "default" calibrations
#export JANA_CALIB_CONTEXT="variation=$VARIATION"
export JANA_CALIB_CONTEXT="variation=default"

export NTHREADS=30

RUNNUM=`echo ${RUN} | awk '{printf "%d\n",$0;}'`

echo ==start run ${RUNNUM}== >> message.txt
echo running pass0 ... >> message.txt

##########################################################################
## STEP 1: basic timing validation

# config
NEVENTS_ZEROTH_PASS=100000
ZEROTH_CALIB_PLUGINS=RF_online
PASS0_OUTPUT_FILENAME=hd_calib_pass0.1_Run${RUN}.root
# run
echo ==zeroth pass, first step==
echo Running these plugins: $ZEROTH_CALIB_PLUGINS
echo hd_root  -Pnthreads=$NTHREADS -PEVIO:RUN_NUMBER=${RUNNUM}  -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS0_OUTPUT_FILENAME -Pjana:nevents=$NEVENTS_ZEROTH_PASS -PPLUGINS=$ZEROTH_CALIB_PLUGINS  $EVIOFILE 
timeout 1200 hd_root  -Pnthreads=$NTHREADS -PEVIO:RUN_NUMBER=${RUNNUM}  -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS0_OUTPUT_FILENAME -Pjana:nevents=$NEVENTS_ZEROTH_PASS -PPLUGINS=$ZEROTH_CALIB_PLUGINS  $EVIOFILE 
retval=$?

if [ "$retval" -ne "0" ]; then
    exit $retval
fi

# process the results
echo ==run calibrations==

echo Running: RF_online, RFMacro_ROCTITimes.C
python3 run_single_root_command.py -F $PASS0_OUTPUT_FILENAME -O pass0_RF_ROCTITimes $HALLD_RECON_HOME/src/plugins/monitoring/RF_online/calib_scripts/RFMacro_ROCTITimes.C
echo Running: RF_online, RFMacro_TDCConversion.C
python3 run_single_root_command.py -F $PASS0_OUTPUT_FILENAME -O pass0_RF_TDCConversion $HALLD_RECON_HOME/src/plugins/monitoring/RF_online/calib_scripts/RFMacro_TDCConversion.C
echo Running: RF_online, RFMacro_SignalPeriod.C
python3 run_single_root_command.py -F $PASS0_OUTPUT_FILENAME -O pass0_RF_SignalPeriod $HALLD_RECON_HOME/src/plugins/monitoring/RF_online/calib_scripts/RFMacro_SignalPeriod.C
echo Running: RF_online, RFMacro_BeamBunchPeriod.C
python3 run_single_root_command.py -F $PASS0_OUTPUT_FILENAME -O pass0_RF_BeamBunchPeriod $HALLD_RECON_HOME/src/plugins/monitoring/RF_online/calib_scripts/RFMacro_BeamBunchPeriod.C


##########################################################################
## STEP 3: fine RF calibration, time_offset_var

export VARIATION=calib

# Now that we've started to recalibrate, run over the newly calibrated values
export JANA_CALIB_CONTEXT="variation=$VARIATION"

# config
NEVENTS_ZEROTH_PASS=100000
ZEROTH_CALIB_PLUGINS=RF_online
PASS0_OUTPUT_FILENAME=hd_calib_pass0.3_Run${RUN}.root
# run
echo ==zeroth pass, third step==
echo Running these plugins: $ZEROTH_CALIB_PLUGINS
echo hd_root -Pnthreads=$NTHREADS -PEVIO:RUN_NUMBER=${RUNNUM} -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS0_OUTPUT_FILENAME -Pjana:nevents=$NEVENTS_ZEROTH_PASS -PPLUGINS=$ZEROTH_CALIB_PLUGINS     $EVIOFILE
timeout 1200  hd_root -Pnthreads=$NTHREADS -PEVIO:RUN_NUMBER=${RUNNUM} -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$PASS0_OUTPUT_FILENAME -Pjana:nevents=$NEVENTS_ZEROTH_PASS -PPLUGINS=$ZEROTH_CALIB_PLUGINS     $EVIOFILE
retval=$?

if [ "$retval" -ne "0" ]; then
    exit $retval
fi

# process the results
echo ==run calibrations==
#python run_calib_pass0.3.py $PASS0_OUTPUT_FILENAME
# error check?

echo Running: RF_online, RFMacro_FineTimeOffsets.C
python3 run_single_root_command.py -F  $PASS0_OUTPUT_FILENAME -O pass0_RF_FineTimeOffsets $HALLD_RECON_HOME/src/plugins/monitoring/RF_online/calib_scripts/RFMacro_FineTimeOffsets.C\(${RUNNUM},\"${VARIATION}\"\) 

# update CCDB
ccdb add /PHOTON_BEAM/RF/time_offset -v $VARIATION -r ${RUN}-${RUN} rf_fine_time_offsets.txt #"fine time offsets"
ccdb add /PHOTON_BEAM/RF/time_offset_var -v $VARIATION -r ${RUN}-${RUN} rf_time_offset_vars.txt #"time offset variances"

echo ==done==
date

exit 0
