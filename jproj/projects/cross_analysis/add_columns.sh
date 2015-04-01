#!/bin/tcsh

if ( $1 == "" ) then
  echo "add_columns.sh"
  echo "Need to specify version..."
  exit
endif
set VER = $1

set OUTFILE = "add_columns.sql"

if ( -e $OUTFILE) then
  rm -f $OUTFILE
endif

echo "ALTER TABLE "'`cross_analysis_table`'" ADD COLUMN (" >  $OUTFILE
echo ' `result_ver'"${VER}"'`'"     VARCHAR(40),"            >> $OUTFILE
echo ' `cput_ver'"${VER}"'`'"       VARCHAR(40),"            >> $OUTFILE
echo ' `walltime_ver'"${VER}"'`'"   VARCHAR(40),"            >> $OUTFILE
echo ' `mem_ver'"${VER}"'`'"        VARCHAR(40),"            >> $OUTFILE
echo ' `vmem_ver'"${VER}"'`'"       VARCHAR(40),"            >> $OUTFILE
echo ' `error_ver'"${VER}"'`'"      VARCHAR(80),"            >> $OUTFILE
echo ' `nevents_ver'"${VER}"'`'"    INT(10)    ,"            >> $OUTFILE
echo ' `timeCopy_ver'"${VER}"'`'"   INT(10)    ,"            >> $OUTFILE
echo ' `timePlugin_ver'"${VER}"'`'" INT(10)"                 >> $OUTFILE
echo ")"                                                     >> $OUTFILE

time mysql -hhallddb -ufarmer farming < ./add_columns.sql
