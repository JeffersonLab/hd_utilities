#!/bin/tcsh -e

#-------------------------------------------
#
# 2015/12/09 Kei Moriya
#
# Example flow of how to run test of resource
# usage against cores/threads.
#
#-------------------------------------------

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

# 1. Create workflow
hdswif.py create $WORKFLOW
hdswif.py run $WORKFLOW

# 2. Add jobs jobs with different number of cores/threads
# The file input.config.template has templates that are
# replaced by addjobs.sh for NCORES, RUNPERIOD, VERSION
./addjobs.sh $RUNPERIOD $VERSION

# 3. When workflow is done, grab all output from stdout/stderr
time ./getnevents.sh $RUNPERIOD $VERSION > nevents_resources_${RUNPERIOD}_ver${VERSION}txt

# 3. Make plots of all resources
time python plot_resources_against_ncores.py $RUNPERIOD $VERSION 
