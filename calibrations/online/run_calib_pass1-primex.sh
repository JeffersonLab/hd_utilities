#!/bin/bash
# Do a first pass of calibrations for a given run

RUN=$1

if [ ! -z "$2" ]; then
    echo changing working directory to $2
    cd $2
fi

source setup_gluex.sh

# python2.7 needed for CCDB command line tool - this is the version needed for the CentOS7 nodes
#export PATH=/apps/python/2.7.12/bin:$PATH
#export LD_LIBRARY_PATH=/apps/python/2.7.12/lib:$LD_LIBRARY_PATH

export CCDB_CONNECTION=mysql://ccdb_user@hallddb.jlab.org/ccdb    # save results in MySQL
export VARIATION=calib
export JANA_CALIB_CONTEXT="variation=$VARIATION" 
#export JANA_CALIB_CONTEXT="variation=default" 

###################################################

# process the results
RUNNUM=`echo ${RUN} | awk '{printf "%d\n",$0;}'`

echo merging...
rm -f hd_calib_pass1_Run${RUN}.root
hadd -k hd_calib_pass1_Run${RUN}.root hd_calib_pass1_Run${RUN}_*.root

echo calculating pass1 constants ... >> message.txt

RUN_OUTPUT_FILENAME=hd_calib_pass1_Run${RUN}.root

echo ==second pass calibrations==
echo Running: HLDetectorTiming, AdjustTiming_NoField.C
#python run_single_root_command.py $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/FitScripts/AdjustTiming.C\(\"${RUN_OUTPUT_FILENAME}\",${RUNNUM},\"${VARIATION}\"\)
python run_single_root_command.py AdjustTiming_NoField.C\(\"${RUN_OUTPUT_FILENAME}\",${RUNNUM},\"${VARIATION}\"\)
#echo Running: CDC_amp, CDC_gains.C
#python run_single_root_command.py -F $RUN_OUTPUT_FILENAME $HALLD_RECON_HOME/src/plugins/Calibration/CDC_amp/CDC_gains.C\(1\)
#echo Running: BCAL_TDC_Timing, ExtractTimeWalk.C
#python run_single_root_command.py $HALLD_RECON_HOME/src/plugins/Calibration/BCAL_TDC_Timing/FitScripts/ExtractTimeWalk.C\(\"${RUN_OUTPUT_FILENAME}\"\)
#echo Running: PS_E_calib, PSEcorr.C
#python run_single_root_command.py $HALLD_RECON_HOME/src/plugins/Calibration/PS_E_calib/PSEcorr.C\(\"${RUN_OUTPUT_FILENAME}\"\)
#python run_calib_pass2.py $RUN_OUTPUT_FILENAME

echo ==make monitoring output==
python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_CalorimeterTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_CalorimeterTiming.C
python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_PIDSystemTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_PIDSystemTiming.C
#python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TrackMatchedTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TrackMatchedTiming.C
python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerTiming.C
#python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TrackingTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TrackingTiming.C
python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerRFAlignment $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerRFAlignment.C
python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerRFAlignment2 $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerRFAlignment2.C
python run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerSCAlignment $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerSCAlignment.C

# update CCDB
echo ==update CCDB==
#ccdb add /CDC/digi_scales -v $VARIATION -r ${RUNNUM}-${RUNNUM} cdc_new_ascale.txt 
./add_consts-adjust-nofield.sh ${RUNNUM} 
#./add_consts-adjust.sh ${RUNNUM} 
#./add_consts-adjust.sh ${RUNNUM} ${RUNNUM}
retval=$?

#./add_consts-adjust.sh ${RUNNUM} ${RUNNUM}
#    ccdb add /BCAL/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} bcal_base_time.txt
#    ccdb add /CDC/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} cdc_base_time.txt
#    ccdb add /FCAL/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} fcal_base_time.txt
    #ccdb add /FDC/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} fdc_base_time.txt
#    ccdb add /START_COUNTER/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} sc_base_time.txt
#    ccdb add /PHOTON_BEAM/hodoscope/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} tagh_base_time.txt
#    ccdb add /PHOTON_BEAM/microscope/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} tagm_base_time.txt
#    ccdb add /TOF/base_time_offset -v $VARIATION -r ${RUNNUM}-${RUNNUM} tof_base_time.txt
    #ccdb add /BCAL/ADC_timing_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} bcal_adc_timing_offsets.txt
    #ccdb add /BCAL/TDC_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} bcal_tdc_timing_offsets.txt
    #ccdb add /FCAL/timing_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} fcal_adc_timing_offsets.txt
#    ccdb add /START_COUNTER/adc_timing_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} sc_adc_timing_offsets.txt
#    ccdb add /START_COUNTER/tdc_timing_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} sc_tdc_timing_offsets.txt
#    ccdb add /PHOTON_BEAM/microscope/fadc_time_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} tagm_adc_timing_offsets.txt
#    ccdb add /PHOTON_BEAM/microscope/tdc_time_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} tagm_tdc_timing_offsets.txt
#    ccdb add /PHOTON_BEAM/hodoscope/fadc_time_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} tagh_adc_timing_offsets.txt
#    ccdb add /PHOTON_BEAM/hodoscope/tdc_time_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} tagh_tdc_timing_offsets.txt
    #ccdb add /TOF/adc_timing_offsets -v $VARIATION -r ${RUNNUM}-${RUNNUM} tof_adc_timing_offsets.txt
    #ccdb add /BCAL/timewalk_tdc -v $VARIATION -r ${RUNNUM}-${RUNNUM} TimewalkBCAL.txt
    #ccdb add /PHOTON_BEAM/pair_spectrometer/fine/energy_corrections -v $VARIATION -r ${RUNNUM}-${RUNNUM} Eparms-TAGM.out


echo ==DEBUG==
ls -lhR

###################################################
# now, move the constants to default if it looks like they need it
if [ "$retval" -eq "0" ]; then
    python push_tables_to_production.py  online_ccdb_tables_to_push.primex -R $RUNNUM -m $RUNNUM --logentry=logbook.txt --mask_file=channel_masks >> message.txt
    ####python push_tables_to_production.py  online_ccdb_tables_to_push.tagm -R $RUNNUM -m $RUNNUM --logentry=logbook.txt --mask_file=channel_masks >> message.txt
fi

# make a logbook entry if the data is there
#if [ -f "logbook.txt" ]; then
#    echo ==submit logbook entry==
#    /site/ace/certified/apps/bin/logentry --title "Run $RUNNUM online calibrations" --html --body logbook.txt --entrymaker hdops --tag Autolog --logbook HDMONITOR --logbook HDRUN --noqueue
#    #/site/ace/certified/apps/bin/logentry --title "Run $RUNNUM online calibrations" --html --body logbook.txt --entrymaker hdops --tag Autolog --logbook HDMONITOR --logbook HDRUN --noqueue --cert /g#luonwork1/Users/sdobbs/calibration_train/online/elogcert
#    #/site/ace/certified/apps/bin/logentry --title "Run $RUNNUM online calibrations" --html --body logbook.txt --entrymaker hdops --tag Autolog --logbook TLOG
#fi
#fi

# and send update email
#if [ -f "message.txt" ]; then
#    cp -v message.txt /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations/
#    cd /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations
#    /group/halld/Users/sdobbs/simple_email_list/scripts/simple_email_list.pl
#fi

###################################################

exit 0
