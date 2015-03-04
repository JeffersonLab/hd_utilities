#!/bin/sh
echo == starting `date` ==
cd /home/gluex/halld/jproj/projects/dc_02
pwd
lockfile=dc_02_job_driver.lock
if [ -f $lockfile ]
  then
  echo lock file found, exiting
  exit 1
fi
date > $lockfile
export PATH=/home/gluex/halld/jproj/scripts:/site/bin:$PATH
nq=`jobstat -u gluex | grep 900 | grep _20 | grep ' A ' | wc -l`
echo number queued is $nq
if [ $nq -lt 50 ]
  then
    echo == submitting ==
    jproj.pl dc_02 submit 50
fi
echo == looking for disk output ==
jproj.pl dc_02 update_output /volatile/halld/home/gluex/proj/dc_02/rest
echo == looking for tape files ==
jproj.pl dc_02 update_silo /mss/halld/data_challenge/02/rest
rm -v $lockfile
echo == ending `date` ==
exit 0
