#!/bin/sh
echo == starting `date` ==
cd /home/gluex/halld/jproj/projects/dc_02
pwd
lockfile=dc_02_tape_driver.lock
if [ -f $lockfile ]
  then
  echo lock file found, exiting
  exit 1
fi
date > $lockfile
export PATH=/home/gluex/halld/jproj/scripts:/site/bin:$PATH
echo == doing jput ==
jproj.pl dc_02 jput /volatile/halld/home/gluex/proj/dc_02/rest /mss/halld/data_challenge/02/rest 0 700
rm -v $lockfile
echo == ending `date` ==
exit 0
