#!/bin/tcsh

set RUNPERIOD  = ""
set VERSION    = ""

set RUNPERIOD = $1
set VERSION   = $2
if ( $RUNPERIOD == "" || $VERSION == "" ) then
  echo "Usage:"
  echo "addjobs.sh [RUNPERIOD] [VERSION]"
  echo "example: addjobs.sh 2015_03 99"
  echo "RUNPERIOD and VERSION must be set"
  exit
endif

# convert underscore to hyphen
set RUNPERIOD = `echo $RUNPERIOD | sed 's:_:-:'`
set RUNPERIOD_HYPHEN = `echo $RUNPERIOD | sed 's:_:-:'`

foreach NCORES ( 1 2 3 5 5 6 7 8 9 12 15 18 20 24 32 42 )
  rm -f input.config
  cat input.config.template | sed -e "s/NNN/$NCORES/g" -e "s/VVV/$VERSION/g" -e "s/RRR/${RUNPERIOD_HYPHEN}/g" > input.config
  # cat input.config
  hdswif.py add offline_monitoring_RunPeriod${RUNPERIOD}_ver${VERSION}_hd_rawdata -c input.config -r 2931 -f '00[0-4]'
  hdswif.py add offline_monitoring_RunPeriod${RUNPERIOD}_ver${VERSION}_hd_rawdata -c input.config -r 3079 -f '00[0-4]'
  hdswif.py add offline_monitoring_RunPeriod${RUNPERIOD}_ver${VERSION}_hd_rawdata -c input.config -r 3180 -f '00[0-4]'
  hdswif.py add offline_monitoring_RunPeriod${RUNPERIOD}_ver${VERSION}_hd_rawdata -c input.config -r 3185 -f '00[0-4]'
end
