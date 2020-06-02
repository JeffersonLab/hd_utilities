#!/bin/tcsh

set RAWDATADIR=/cache/mss/halld/RunPeriod-2014-10/rawdata
set OUTPUTDIR=/work/halld/home/sdobbs/run_conditions
set JANA_ARGS="-PPLUGINS=DAQ,TTab,run_summary"

#source /home/gluex/setup_jlab_commissioning.csh
source /home/sdobbs/setup_jlab_commissioning.csh

foreach dir ( `ls $RAWDATADIR` )
    set run=`echo $dir | sed 's/Run//'`
    #echo "$run"
    set rootfile="hd_root_conditions_Run${run}.root"
    
    hd_root $JANA_ARGS -POUTPUT_FILENAME=$OUTPUTDIR/$rootfile $RAWDATADIR/Run$run/*.evio
end
