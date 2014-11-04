#!/bin/tcsh

# 2014/10/31 Kei Moriya
# To run cron job, do
# crontab cron_plugins
# 
# To check if cron is running, use
# crontab -l
#
# To end cron job, use
# crontab -r

set DATE   = `date +"%m-%d-%Y-%T"`

set MINRUN = 0
set MAXRUN = 999

set MINRUN = $1
set MAXRUN = $2

# RUNDIR is the directory containing all of the list of files for different times
set RUNDIR = /home/gluex/halld/monitoring/cron/filelists
cd $RUNDIR

set THIS_OUTFILE = files${DATE}.txt

# Check all files within the run directory
cd /mss/halld/RunPeriod-2014-10/rawdata/
ls --color=never > ${RUNDIR}/${THIS_OUTFILE}
cd -

# If list of files is different from current, run plugin job on new files
if ( -e diff.txt ) then
  rm -f diff.txt
endif
diff files_current.txt ${THIS_OUTFILE} | grep '> ' > diff.txt

if ( `ls -l diff.txt | gawk '{ print $5}'` == 0 ) then
  # File was same as before, remove current output since
  # this is just the same as previous output.
  rm -f ${THIS_OUTFILE}
  echo "no new runs, exiting at $DATE" 
  exit
else
  # New entries are found, use regexpchanger.pl
  # to remove "> " and non-run output from diff.
  /home/gluex/halld/monitoringcron/regexpchanger.pl diff.txt

  # link new ls output as current
  rm -f files_current.txt
  ln -s ${THIS_OUTFILE} files_current.txt
endif

echo "contents of diff.txt:"
cat diff.txt

# Remove "Run" for each line
# Also need to remove leading '0's,
# this will cause generatejobs_plugins_rawdata.sh
# to interpret runs as octal.
sed 's/Run//' diff.txt | sed 's/> //' | sed 's/^0*//' > diff2.txt

echo "2nd time: contents of diff2.txt:"
cat diff2.txt

# Run plugin script on these new runs
foreach RUN ( `cat diff2.txt` )

  if ( $MINRUN <= $RUN && $RUN <= $MAXRUN) then

    # echo "checking run $RUN"
    set FORMATTED_RUN = `printf %06d $RUN`

    # Check that time stamp of file is more than 5 min old
    @ filetime = `stat -c %Y /mss/halld/RunPeriod-2014-10/rawdata/Run${FORMATTED_RUN}`
	@ currenttime = `date +%s`
	@ timediff = `expr $currenttime - $filetime`
    # echo "$filetime, $currenttime, $timediff"
	if ( $timediff > 300 ) then
	echo "processing run $RUN"
	# create job
	/home/gluex/halld/monitoring/batch/generatejobs_plugins_rawdata.sh $RUN
	# submit job
	    chmod u+x run_rawdata_${FORMATTED_RUN}.sh
	./run_rawdata_${FORMATTED_RUN}.sh
    endif

  endif
end

# Delete temporary file
rm -f diff.txt diff.txt.bak diff2.txt
