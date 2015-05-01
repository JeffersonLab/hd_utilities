#!/bin/tcsh

source /home/gxproj1/halld/jproj/scripts/setup.csh

# Update DB for files 000 - 019 only
foreach FILE ( `seq 0 19` )
  jproj.pl PROJECT update $FILE
end

# To update for all files simply run
# jproj.pl PROJECT update
