#!/bin/tcsh

#-----------------------------------------
#
# Get information in stdout files from
# finished jobs.
#
#-----------------------------------------

set PARENT_DIR = "/volatile/halld/offline_monitoring/RunPeriod-RUNPERIOD/verVERSION/log"

set OUTFILE = PROJHOME/PROJECT/analysis/processed_job_info.txt

if( -e $OUTFILE) then
  rm -f $OUTFILE
endif
touch $OUTFILE

cd $PARENT_DIR

set NFILES = 0

foreach FILE ( `ls 00*/stdout*` )
  set NEVENTS = ""
  set NEVENTS = `tail -n 60 $FILE | grep -a -B 5 'Closed ROOT file' | grep -a 'events processed' | gawk '{print $3}'`
  if ( $NEVENTS == "" ) then
    set NEVENTS = "-999"
  endif

  set TIMECOPY = ""
  set TIMECOPY = `grep -a 'copy input file' $FILE | gawk '{print $6}'`
  if ( $TIMECOPY == "" ) then
    set TIMECOPY = "-999"
  endif

  set TIMEPLUGIN = ""
  set TIMEPLUGIN = `grep -a 'TIMEDIFF' $FILE | gawk '{print $2}'`
  if ( $TIMEPLUGIN == "" ) then
    set TIMEPLUGIN = "-999"
  endif

  set SEGFAULT = ""
  set SEGFAULT = `tail -n 60 $FILE | grep -a 'Segmentation fault' | gawk '{print $1}'`
  if ( $SEGFAULT == "" ) then
    set SEGFAULT = "0"
  else
    echo "SEGFAULT was $SEGFAULT for $FILE"
    set SEGFAULT = "1"
  endif

  # Get run and file so we can query the database for jobId
  set RUNNUM  = `basename $FILE | gawk '{print substr($0,8,6)}'`
  set FILENUM = `basename $FILE | gawk '{print substr($0,15,3)}'`

  # Get jobId from monitoring database
  set JOBID = `mysql -hhallddb -ufarmer farming -s -r -e "select jobId from PROJECTJob where run = $RUNNUM and file = $FILENUM" | tail -n 1`
  if ( $JOBID != "" ) then
    echo "$JOBID   $RUNNUM   $FILENUM   $NEVENTS   $TIMECOPY   $TIMEPLUGIN   $SEGFAULT" | gawk '{printf "%-8s   %06d   %03d   %8s   %6d   %6d   %2d\n", $1,$2,$3,$4,$5,$6,$7}' >> $OUTFILE
  endif

  @ NFILES += 1

  if ( $NFILES % 100 == 0) then
    echo "processed $NFILES files..."
  endif

end
