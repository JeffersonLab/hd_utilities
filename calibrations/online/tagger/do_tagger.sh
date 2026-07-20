#!/bin/bash

if [ -z "$1" ]; then
    echo no run specified, skipping tagger calibrations...
    exit 0
fi

run=$1

# actually update the master DB
export CCDB_CONNECTION=mysql://ccdb_user@hallddb.jlab.org/ccdb
export CCDB_USER=hdsys

# diagnostics
echo CCDB_CONNECTION=$CCDB_CONNECTION
echo JANA_CALIB_URL=$JANA_CALIB_URL

touch updated_tables.txt

# run data
echo ==tagger pass1: run $run==
#timeout 12000 hd_root  --nthreads=35 -PJANA:BATCH_MODE=1 -POUTPUT_FILENAME=hd_root_r${run}-tagger.root -PPLUGINS=TAGM_TW,PS_timing -PTAGMHit:DELTA_T_ADC_TDC_MAX=100 /gluonraid2/rawdata/volatile/RunPeriod-2021-08/rawdata/Run0${run}/hd_rawdata_0${run}_*.evio
timeout 12000 hd_root  --nthreads=35 -PJANA:BATCH_MODE=1 -POUTPUT_FILENAME=hd_root_r${run}-tagger.root -PPLUGINS=TAGM_TW,PS_timing,TAGH_timewalk -PTAGMHit:DELTA_T_ADC_TDC_MAX=200 /gluonraid2/rawdata/volatile/RunPeriod-2023-01/rawdata/Run${run}/hd_rawdata_${run}_000.evio
retval=$?

if [ "$retval" -eq "0" ]; then

    # TAGM calibrations
    python tagm_timing.py -b hd_root_r${run}-tagger.root ${run} rf default
    python tagm_tw.py -b hd_root_r${run}-tagger.root ${run}
    #root -l -b tw_tagm.C\(\"hd_root_r${run}-tagger.root\",${run}\)
    echo ==submit TAGM constants==
    ccdb add /PHOTON_BEAM/microscope/fadc_time_offsets -v default -r ${run}-${run} adc_offsets-${run}.txt    2>&1 >> updated_tables.txt
    ccdb add /PHOTON_BEAM/microscope/tdc_time_offsets -v default -r ${run}-${run} tdc_offsets-${run}.txt     2>&1 >> updated_tables.txt
    ccdb add /PHOTON_BEAM/microscope/tdc_timewalk_corrections -v default -r ${run}-${run} tw-corr-${run}.txt 2>&1 >> updated_tables.txt

    # TAGH calibrations
    ./do_tagh-offsets.sh  ${run} hd_root_r${run}-tagger.root
    ccdb add /PHOTON_BEAM/hodoscope/tdc_time_offsets -v default -r ${run}-${run} offsets/tdc_time_offsets.txt    2>&1 >> updated_tables.txt
    ccdb add /PHOTON_BEAM/hodoscope/fadc_time_offsets -v default -r ${run}-${run} offsets/fadc_time_offsets.txt  2>&1 >> updated_tables.txt
    mkdir TAGH_offsets_output
    mv offsets TAGH_offsets_output
    mv fits-csv TAGH_offsets_output

    # PS calibrations
    ./ps_run.sh ${run} hd_root_r${run}-tagger.root
    echo ==submit PS constants==
    ccdb add /PHOTON_BEAM/pair_spectrometer/base_time_offset -v default -r ${run}-${run} ps_offsets/base_time_offset.txt                 2>&1 >> updated_tables.txt
    ccdb add /PHOTON_BEAM/pair_spectrometer/coarse/tdc_timing_offsets -v default -r ${run}-${run} ps_offsets/tdc_timing_offsets_psc.txt  2>&1 >> updated_tables.txt
    ccdb add /PHOTON_BEAM/pair_spectrometer/coarse/adc_timing_offsets -v default -r ${run}-${run} ps_offsets/adc_timing_offsets_psc.txt  2>&1 >> updated_tables.txt
    ccdb add /PHOTON_BEAM/pair_spectrometer/fine/adc_timing_offsets -v default -r ${run}-${run} ps_offsets/adc_timing_offsets_ps.txt     2>&1 >> updated_tables.txt
    mkdir PS_calib_output
    mv fits-csv ps_offsets PS_calib_output


fi


echo ==tagger pass2: run $run==
#timeout 24000 hd_root  --nthreads=35 -PJANA:BATCH_MODE=1 -POUTPUT_FILENAME=hd_root_r${run}-tagger-p2.root -PPLUGINS=TAGH_timewalk /gluonraid2/rawdata/volatile/RunPeriod-2021-08/rawdata/Run0${run}/hd_rawdata_0${run}_00[01].evio
#timeout 24000 hd_root  --nthreads=35 -PJANA:BATCH_MODE=1 -POUTPUT_FILENAME=hd_root_r${run}-tagger-p2.root -PPLUGINS=TAGH_timewalk /gluonraid2/rawdata/volatile/RunPeriod-2021-08/rawdata/Run0${run}/hd_rawdata_0${run}_001.evio
timeout 30000 hd_root  --nthreads=35 -PJANA:BATCH_MODE=1 -POUTPUT_FILENAME=hd_root_r${run}-tagger-p2.root -PPLUGINS=TAGH_timewalk /gluonraid2/rawdata/volatile/RunPeriod-2023-01/rawdata/Run${run}/hd_rawdata_${run}_00[012].evio
retval=$?

if [ "$retval" -eq "0" ]; then
    ./do_tagh.sh ${run} hd_root_r${run}-tagger-p2.root

    echo ==submit TAGH constants==
    ccdb add PHOTON_BEAM/hodoscope/tdc_timewalk -v default -r ${run}-${run} tdc_timewalk.txt 2>&1 >> updated_tables.txt
fi

## finalize things and clean them up
#echo ==push constants to production==
#python push_tables_to_production.py  online_ccdb_tables_to_push -R $run -m $run --logentry=logbook.txt --mask_file=channel_masks >> message.txt

# add more info to the logbook entry - taken out of push_tables_to_production.py so that we can push the calibrations sooner
echo "<h3>Other Tables Updated for Run-dependent Calibrations</h3>\n" >> logbook.txt
cat updated_tables.txt >> logbook.txt


# make a logbook entry if the data is there
if [ -f "logbook.txt" ]; then
    echo ==submit logbook entry==
    /site/ace/certified/apps/bin/logentry --title "Run $run online calibrations" --html --body logbook.txt --entrymaker hdops --tag Autolog --logbook HDMONITOR --logbook HDRUN --noqueue
fi
#fi

# and send update email
echo ==send update email==
if [ -f "message.txt" ]; then
    echo "\nadditional updated tables" >> message.txt
    cat updated_tables.txt >> message.txt

    cp -v message.txt /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations/
    cd /group/halld/Users/sdobbs/simple_email_list/lists/online_calibrations
    /group/halld/Users/sdobbs/simple_email_list/scripts/simple_email_list.pl
fi


exit 0
