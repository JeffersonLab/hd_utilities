#!/bin/tcsh

if ( $1 == "" || $2 == "" ) then
  echo "write_inserts_each_launch.sh [run period] [ver #]"
  echo "example:   write_inserts_each_launch.sh 2015_03 03"
  exit
endif
set RUNPERIOD = $1
set VER       = $2

set OUTFILE = "write_inserts_${RUNPERIOD}_ver${VER}.pl"
if ( -e $OUTFILE) then
  rm -f $OUTFILE
endif

echo '#\!/usr/bin/env perl'                    > $OUTFILE
if($RUNPERIOD == "2010_10") then
  echo '$tablename = "cross_analysis_table";'   >> $OUTFILE
else
  echo '$tablename = "cross_analysis_table_'"${RUNPERIOD}"'";'   >> $OUTFILE
endif
echo 'while (<>) {'                           >> $OUTFILE
echo '    chomp;'                             >> $OUTFILE
echo '    @t0 = split;'                       >> $OUTFILE
echo '    $run = $t0[0];'                     >> $OUTFILE
echo '    $file = $t0[1];'                    >> $OUTFILE
echo '    $result = $t0[2];'                  >> $OUTFILE
echo '    $cput = $t0[3];'                    >> $OUTFILE
echo '    $walltime = $t0[4];'                >> $OUTFILE
echo '    $mem = $t0[5];'                     >> $OUTFILE
echo '    $vmem = $t0[6];'                    >> $OUTFILE
echo '    $nevents = $t0[7];'                 >> $OUTFILE
echo '    $timeCopy = $t0[8];'                >> $OUTFILE
echo '    $timePlugin = $t0[9];'              >> $OUTFILE
#echo '    $segfault = $t0[10];'               >> $OUTFILE
#echo '    $error = $t0[11];'                  >> $OUTFILE
echo ""                                       >> $OUTFILE
echo '    print "UPDATE $tablename SET'       >> $OUTFILE
echo "           result_ver${VER}     = \\'"'$'"result\\',"                >> $OUTFILE
echo "           cput_ver${VER}       = \\'"'$'"cput\\',"                  >> $OUTFILE
echo "           walltime_ver${VER}   = \\'"'$'"walltime\\',"              >> $OUTFILE
echo "           mem_ver${VER}        = \\'"'$'"mem\\',"                   >> $OUTFILE
echo "           vmem_ver${VER}       = \\'"'$'"vmem\\',"                  >> $OUTFILE
#echo "           error_ver${VER}      = \\'"'$'"error\\',"                 >> $OUTFILE
echo "           nevents_ver${VER}    = "'$'"nevents,"                     >> $OUTFILE
echo "           timeCopy_ver${VER}   = "'$'"timeCopy,"                    >> $OUTFILE
echo "           timePlugin_ver${VER} = "'$'"timePlugin"                   >> $OUTFILE
#echo "           segfault_ver${VER}   = "'$'"segfault"                     >> $OUTFILE
echo "           WHERE        run = "'$'"run AND file = "'$'"file;\\n"'";' >> $OUTFILE
echo             "}"                                                       >> $OUTFILE
echo "exit;" >> $OUTFILE

chmod u+x ./write_inserts_${RUNPERIOD}_ver${VER}.pl


if($RUNPERIOD == "2010_10") then
  time ./write_inserts_${RUNPERIOD}_ver${VER}.pl  < jobinfo_ver${VER}_modified.txt | mysql -hhallddb -ufarmer farming
else
  time ./write_inserts_${RUNPERIOD}_ver${VER}.pl  < jobinfo_${RUNPERIOD}_ver${VER}_modified.txt | mysql -hhallddb -ufarmer farming
endif
