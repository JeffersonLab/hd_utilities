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

#echo merging...
#rm -f hd_calib_pass1_Run${RUN}.root
#hadd -k hd_calib_pass1_Run${RUN}.root hd_calib_pass1_Run${RUN}_*.root

echo calculating pass1 constants ... >> message.txt

RUN_OUTPUT_FILENAME=hd_calib_pass1_Run${RUN}.root

echo ==second pass calibrations==
echo Running: HLDetectorTiming, AdjustTiming.C
#python run_single_root_command.py $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/FitScripts/AdjustTiming.C\(\"${RUN_OUTPUT_FILENAME}\",${RUNNUM},\"${VARIATION}\"\)
python3 run_single_root_command.py AdjustTiming.C\(\"${RUN_OUTPUT_FILENAME}\",${RUNNUM},\"${VARIATION}\"\)
#echo Running: CDC_amp, CDC_gains.C
#python run_single_root_command.py -F $RUN_OUTPUT_FILENAME $HALLD_RECON_HOME/src/plugins/Calibration/CDC_amp/CDC_gains.C\(1\)
#echo Running: BCAL_TDC_Timing, ExtractTimeWalk.C
#python run_single_root_command.py $HALLD_RECON_HOME/src/plugins/Calibration/BCAL_TDC_Timing/FitScripts/ExtractTimeWalk.C\(\"${RUN_OUTPUT_FILENAME}\"\)
#echo Running: PS_E_calib, PSEcorr.C
#python run_single_root_command.py $HALLD_RECON_HOME/src/plugins/Calibration/PS_E_calib/PSEcorr.C\(\"${RUN_OUTPUT_FILENAME}\"\)
#python run_calib_pass2.py $RUN_OUTPUT_FILENAME

echo ==make monitoring output==
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_CalorimeterTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_CalorimeterTiming.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_PIDSystemTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_PIDSystemTiming.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TrackMatchedTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TrackMatchedTiming.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerTiming.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TrackingTiming $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TrackingTiming.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerRFAlignment $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerRFAlignment.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerRFAlignment2 $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerRFAlignment2.C
python3 run_single_root_command.py -F $RUN_OUTPUT_FILENAME -O pass2_TaggerSCAlignment $HALLD_RECON_HOME/src/plugins/Calibration/HLDetectorTiming/HistMacro_TaggerSCAlignment.C

# update CCDB
echo ==update CCDB==
./add_consts-adjust.sh ${RUNNUM} 
#./add_consts-adjust-nofield.sh ${RUNNUM} 
retval=$?

echo ==DEBUG==
ls -lhR



###################################################
# now, move the constants to default if it looks like they need it
#if [ "$retval" -eq "0" ]; then
## finalize things and clean them up
echo ==push constants to production==
python3 push_tables_to_production.py  online_ccdb_tables_to_push -R $RUN -m $RUN --logentry=logbook.txt --mask_file=channel_masks >> message.txt

# add more info to the logbook entry - taken out of push_tables_to_production.py so that we can push the calibrations sooner
#echo "<h3>Other Tables Updated for Run-dependent Calibrations</h3>\n" >> logbook.txt
#cat updated_tables.txt >> logbook.txt

# make a logbook entry if the data is there
if [ -f "logbook.txt" ]; then
    echo ==submit logbook entry==
    /site/ace/certified/apps/bin/logentry --title "Run $RUN online calibrations" --html --body logbook.txt --entrymaker hdops --tag Autolog --logbook HDMONITOR --logbook HDRUN --noqueue
fi
   
# and send update email
echo ==send update email==
if [ -f "message.txt" ]; then
    echo "\nadditional updated tables" >> message.txt
    cat updated_tables.txt >> message.txt
    
    cp -v message.txt /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations/
    cd /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations
    /group/halld/Users/sdobbs/simple_email_list/scripts/simple_email_list.pl
fi

#fi

###################################################

exit 0
