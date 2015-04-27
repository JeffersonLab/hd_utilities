#!/bin/tcsh
set NFILES  = $1
source /home/gxproj1/halld/jproj/scripts/setup.csh

# Update job managment database table with new files in the given file range
cd /home/gxproj1/halld/jproj/projects/PROJECT
set FILE = 0;
while ($FILE < $NFILES)
    echo "FILE is $FILE"
    jproj.pl PROJECT update $FILE
    @ FILE++
end

# submit the jobs for the new files
jproj.pl PROJECT submit
