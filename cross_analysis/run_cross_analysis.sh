#!/bin/tcsh -e

################################################################
#
# 2015/11/27 Kei Moriya
#
# This directory contains scripts to run the cross-analysis
# of offline monitoring. The purpose is to correlate the
# results and resources for the same files across different
# launch versions.
#
# The results of the cross analysis are shown at
# https://halldweb.jlab.org/data_monitoring/launch_analysis/
# under links to each run period.
#
# Usage:
# run_cross_analysis [RUN PERIOD] [VERSION]
#
################################################################

# 0. Set current run period, version
set RUNPERIOD = ""
set VERSION   = ""
set RUNPERIOD = $1
set RUNPERIOD = `echo RUNPERIOD | sed 's:-:_:'`
set VERSION   = $2
if ( $RUNPERIOD == "" || $VERSION == "" ) then
  echo "run_cross_analysis.sh usage:"
  echo "run_cross_analysis.sh [RUN PERIOD] [VERSION]"
  echo "exiting..."
  exit
endif

# Set minimum to be ver15 (can go back as far as SWIF has been used)
set MINVERSION = 15

# For correlations, default is to go back 5 launches.
@ CMPMINVERSION = $VERSION - 5
if (  $CMPMINVERSION < 15 ) then
  set CMPMINVERSION = 15
endif
echo "MINVERSION = $MINVERSION"

# 1. Create table for this run period and version
echo "1. Create table for this run period and version"
time create_cross_analysis_table.sh $RUNPERIOD $VERSION

# 2. From SWIF XML output, add entries
#    - Each run/file should be a unique entry (no duplicates)
#    - Columns are 
#      + id, timeChange (automatic)
#      + run, file      (for each job from SWIF XML)
#      + auger_cpu_sec, auger_wall_sec, auger_mem_kb, auger_vmem_kb, finalState (from final attempt of job)
#      + problems (from all attempts of job)
#      + nevents, timeCopy, timePlugin (from stdout file for that job)
#
# RECREATE TABLE FROM 1. IF RERUNNING THIS STEP!!!
echo "2. From SWIF XML output, add entries"
time python fill_cross_analysis_entries.py $RUNPERIOD $VERSION

# To spit out table results do
# mysql -hhallddb -ufarmer farming -r -s -e " SELECT * FROM  cross_analysis_table_2015_03_ver20" > tmp.txt

# 3. Create row for table of stats for each launch, insert into table
echo "3. Create row for table of stats for each launch, insert into table"
time python create_stats_table_row.py $RUNPERIOD $VERSION

# 4. Create table for results of each file against version
# For this to work we need to have the DB in step 2. to have been created.
# Launches that do not have corresponding tables from step 2. will be skipped.
echo "4. Create table for results of each file against version"
time python create_stats_for_each_file.py $RUNPERIOD $MINVERSION $VERSION

# 5. Copy files to web-accessible location
echo "5. Copy files to web-accessible location"
cp -rv swif_rap_sheet_${RUNPERIOD}/*.html /group/halld/www/halldweb/html/data_monitoring/launch_analysis/${RUNPERIOD}/swif_rap_sheet_${RUNPERIOD}/

# 6. Create plots that cross-correlate resource usage
echo "6. Create plots that cross-correlate resource usage"
time python create_resource_correlation_plots.py $RUNPERIOD $CMPMINVERSION $VERSION
