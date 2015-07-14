#!/bin/tcsh

if ( $1 == "" || $2 == "" ) then
  echo "Usage:"
  echo "add_columns.sh  [run period] [ver #]"
  echo "example:   add_columns.sh 2015_03 03"
  exit
endif
set RUNPERIOD = $1
set VER       = $2

set OUTFILE = "add_columns.sql"

if ( -e $OUTFILE) then
  rm -f $OUTFILE
endif

if ( $RUNPERIOD == "2014_10") then
  echo "ALTER TABLE "'`cross_analysis_table`'" ADD COLUMN (" >  $OUTFILE
else
  echo "ALTER TABLE "'`cross_analysis_table_'"${RUNPERIOD}"'`'" ADD COLUMN (" >  $OUTFILE
endif
echo ' `result_ver'"${VER}"'`'"     VARCHAR(40),"            >> $OUTFILE
echo ' `cput_ver'"${VER}"'`'"       VARCHAR(40),"            >> $OUTFILE
echo ' `walltime_ver'"${VER}"'`'"   VARCHAR(40),"            >> $OUTFILE
echo ' `mem_ver'"${VER}"'`'"        VARCHAR(40),"            >> $OUTFILE
echo ' `vmem_ver'"${VER}"'`'"       VARCHAR(40),"            >> $OUTFILE
#echo ' `error_ver'"${VER}"'`'"      VARCHAR(80),"            >> $OUTFILE
echo ' `nevents_ver'"${VER}"'`'"    INT(10)    ,"            >> $OUTFILE
echo ' `timeCopy_ver'"${VER}"'`'"   INT(10)    ,"            >> $OUTFILE
echo ' `timePlugin_ver'"${VER}"'`'" INT(10)"                 >> $OUTFILE
# echo ' `segfault_ver'"${VER}"'`'"   INT(10)"                 >> $OUTFILE
echo ")"                                                     >> $OUTFILE

time mysql -hhallddb -ufarmer farming < ./add_columns.sql
