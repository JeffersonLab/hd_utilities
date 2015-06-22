#!/bin/tcsh

set RUNPERIOD = $1
set VER_INIT  = $2
set VER_LAST  = $3
set NVERS = `expr ${VER_LAST} - ${VER_INIT} + 1`

if ( $RUNPERIOD == "" || $NVERS == "" || ${VER_INIT} == "" ) then
  echo "-----------------------------------------------------------------------------------------------"
  echo "Usage:"
  echo "spitout_results.sh [run period] [ver # of first launch] [ver # of last launch] "
  echo "-----------------------------------------------------------------------------------------------"
  echo ""
  echo "This script will create a txt file results_[run period].txt"
  echo "that contains the results of all jobs for past launches"
  echo "Example usage: "
  echo "spitout_results.sh 2015_03 2 8"

  exit
endif

if ( $RUNPERIOD != "2014_10" && $RUNPERIOD != "2015_03" ) then
  echo "RUNPERIOD must be either 2014_10 or 2015_03"
  exit
endif

set OUTFILE = "results_${RUNPERIOD}.txt"
set ALL_VERS_STRING = ""

foreach VER ( `seq ${VER_INIT} ${VER_LAST}` )
  set FORMATTED_VER = `printf %02d $VER`

  # SKIPPED LAUNCHES
  if ( $RUNPERIOD == "2014_10" && ${FORMATTED_VER} == 14) then
    echo "skipping VER${VER}"
    continue
  endif

  if ( $RUNPERIOD == "2015_03" && ${FORMATTED_VER} == 1) then
    echo "skipping VER${VER}"
    continue
  endif

  if ( ${VER} == ${VER_INIT} ) then
    set ALL_VERS_STRING = "result_ver${FORMATTED_VER}"
  else
    set ALL_VERS_STRING = "${ALL_VERS_STRING}, result_ver${FORMATTED_VER}"
  endif

end

if ( $RUNPERIOD == "2014_10" ) then
  time mysql -hhallddb -ufarmer farming -s -r -e "select run, file, ${ALL_VERS_STRING} FROM cross_analysis_table_${RUNPERIOD}" > $OUTFILE
else if ( $RUNPERIOD == "2015_03" ) then
  time mysql -hhallddb -ufarmer farming -s -r -e "select run, file, ${ALL_VERS_STRING} FROM cross_analysis_table_${RUNPERIOD}" > $OUTFILE
endif
