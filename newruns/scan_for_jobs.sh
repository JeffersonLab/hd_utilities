#!/bin/tcsh

################################################################################
#
# 2014/12/09 Kei Moriya
#
# Script that will scan for new files appearing on /mss.
# This is the script that will be run by the cron job.
#
# Each time this script is run, all evio files in /mss will
# be recorded as a textfile in the directory filelists,
# with the date and time recorded in the file name.
# The most current one will be linked as
# filelists/files_current.txt
# and each time the script runs a comparison to this list
# is made.
#
# If there are files on /mss that were not on files_current.txt
# a job is created and launched for that file.
#
# Example usage:
# scan_for_jobs.sh 2014 12 19 0 99
# will create jobs based on the date of 2014/12/19
# for file numbers in the range 000 - 099.
#
# To reset the filelists/files_current.txt file, do
# echo "" > filelists/files_current.txt
#
# Then make sure that MINRUN and MAXRUN are set
# correctly to restrict the range of runs submitted.
#
################################################################################

# This is date when plugins were run on previous Friday
# These are just passed to the script
# generatejobs_plugins_rawdata.sh
# and used for bookkeeping of the date that libraries were built.
set YEAR  = $1
set MONTH = $2
set DAY   = $3

# This is current date
set DATE   = `date +"%Y-%m-%d-%T"`

set MINRUN = 2439
set MAXRUN = 9999

set MINFILE =   0
set MAXFILE = 999

set INPUT4 = $4
set INPUT5 = $5

if ( $INPUT4 != "" ) then
  set MINFILE = $INPUT4
endif

if ( $INPUT5 != "" ) then
  set MAXFILE = $INPUT5
endif

if ($1 == "" || $2 == "" || $3 == "" ) then
  echo "scan_for_jobs.sh"
  echo "Usage:"
  echo "scan_for_jobs.sh [year] [month] [day] (minfile) (maxfile)"
  exit
endif

# RUNSDIR is the directory containing all of the list of files for different times
set RUNSDIR = /home/gxproj1/halld/monitoring/newruns/filelists
mkdir -p /home/gxproj1/halld/monitoring/newruns/filelists

set FILELIST = ${RUNSDIR}/files${DATE}.txt

# Check all files within the run directory
cd /mss/halld/RunPeriod-2014-10/rawdata/
ls --color=never Run[0-9][0-9][0-9][0-9][0-9][0-9]/hd_raw*.evio > ${FILELIST}
cd -

# echo "contents of FILELIST:"
# cat $FILELIST

# If list of files is different from current, run plugin job on new files
if ( -e newfiles.txt ) then
  rm -f newfiles.txt
endif

diff ${RUNSDIR}/files_current.txt ${FILELIST} | grep '> ' > newfiles.txt

# echo "contents of newfiles.txt:"
# cat newfiles.txt

if ( `ls -l newfiles.txt | gawk '{ print $5}'` == 0 ) then
  # File was same as before, remove current output since
  # this is just the same as previous output.
  rm -f ${FILELIST}
  echo "no new runs, exiting at $DATE" 
else
  # New entries are found, use regexpchanger.pl
  # to remove "> " and non-run output from diff.
  /home/gxproj1/halld/monitoring/newruns/regexpchanger.pl newfiles.txt

  # link new ls output as current
  echo "removing ${RUNSDIR}/files_current.txt"
  rm -f ${RUNSDIR}/files_current.txt
  echo "linking ${FILELIST} to be ${RUNSDIR}/files_current.txt"
  ln -s ${FILELIST} ${RUNSDIR}/files_current.txt
endif

# echo "contents of newfiles.txt:"
# cat newfiles.txt

# Remove "Run" for each line
# Also need to remove leading '0's, since
# this will cause generatejobs_plugins_rawdata.sh
# to interpret runs as octal.
sed 's/Run//' newfiles.txt | sed 's/> //' | sed 's/^0*//' > newrunnums.txt

# echo "2nd time: contents of newrunnums.txt:"
# cat newrunnums.txt

# Run plugin script on these new runs
foreach FILE ( `cat newrunnums.txt` )

  # FILE will be of form RRRR/hd_rawdata_RRRRRR_FFF.evio
  set RUN = `dirname $FILE | sed 's:/::' `
  set FILENUM = `echo $FILE | sed 's/^.*_//' | sed 's/\.evio//' | sed 's/^0*//'`
  # We remove all leading 0's, so if file was 000, we are left with
  # empty string
  if ( $FILENUM == "" ) then
    set FILENUM = 0
  endif

  echo "RUN = $RUN FILENUM = $FILENUM"

  # If there is a Run000000 directory, skip
  if ( $RUN == "" ) then
    echo "skipping $RUN $FILENUM"
    continue
  endif

  if ( $MINRUN <= $RUN && $RUN <= $MAXRUN && $MINFILE <= $FILENUM && $FILENUM <= $MAXFILE ) then

    set FORMATTED_RUN = `printf %06d $RUN`

    # --- Don't need time stamp info, this part is obsolete ---
    # Check that time stamp of file is more than 5 min old
    # @ filetime = `stat -c %Y /mss/halld/RunPeriod-2014-10/rawdata/Run${FORMATTED_RUN}`
    # @ currenttime = `date +%s`
    # @ timediff = `expr $currenttime - $filetime`
    # echo "$filetime, $currenttime, $timediff"
    # if ( $timediff > 300 ) then
    #   echo "processing run $RUN"
    # endif

    # create job
    /bin/tcsh /home/gxproj1/halld/monitoring/newruns/generatejobs_plugins_rawdata.sh $YEAR $MONTH $DAY $RUN $RUN $FILENUM $FILENUM
    # submit job
    chmod u+x /home/gxproj1/halld/monitoring/newruns/runscripts/run_rawdata_${FORMATTED_RUN}.sh
    # /home/gxproj1/halld/monitoring/newruns/runscripts/run_rawdata_${FORMATTED_RUN}.sh
  endif
end

# Delete temporary file
rm -f newfiles.txt newfiles.txt.bak newrunnums.txt
