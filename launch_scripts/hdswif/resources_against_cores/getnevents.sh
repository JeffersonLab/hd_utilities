#!/bin/tcsh

# -------------------------
# 2015/10/31
# 
# Used to extract number of events in each file
# for study of resource usage against #cores.
# Workflow was 2015_03 ver96
#
# 2015/11/03
# Redo tests using only danarest plugin.
# Workflow was 2015_03 ver97
#
# -------------------------

set RUNPERIOD = ""
set VERSION   = ""
set WORKFLOW  = offline_monitoring_RunPeriod${RUNPERIOD}_ver${VERSION}_hd_rawdata

set RUNPERIOD = $1
set VERSION   = $2

if ( $RUNPERIOD == "" || VERSION == "" ) then
  echo "Usage:"
  echo "./run_resources_against_ncores.sh [RUNPERIOD] [VERSION]"
  exit
endif

set RUNPERIOD_HYPHEN = `echo $RUNPERIOD | sed 's/_/-/'`

foreach RUN ( "002931" "003079" "003180" "003185" )
  foreach FILE ( `seq 0 4` )
    foreach NCORES ( 1 2 3 4 5 6 7 8 9 12 15 18 20 24 32 42 )
      set LOGFILE = "/volatile/halld/offline_monitoring/RunPeriod-${RUNPERIOD_HYPHEN}/ver${VERSION}/${NCORES}/log/${RUN}/stdout_${RUN}_00${FILE}.out"

      set NEVENTS = ""
      set NEVENTS = `tail -n 60 $LOGFILE | grep -a -B 5 'Closed ROOT file' | grep -a 'events processed' | gawk '{print $3}'`
      if ( $NEVENTS == "" ) then
	  set NEVENTS = "-999"
      endif

      set PLUGINTIME = ""
      set PLUGINTIME = `tail -n 30 $LOGFILE | grep -a 'TIMEDIFF' | gawk '{print $2}'`
      if ( $PLUGINTIME == "" ) then
	set PLUGINTIME = "-999"
      endif

      echo "$RUN   00$FILE   $NCORES   $NEVENTS   $PLUGINTIME"

    end  # end of loop over cores
  end # end of loop over files
end # end of loop over runs
