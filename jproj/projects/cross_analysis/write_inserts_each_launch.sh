#!/bin/tcsh

if ( $1 == "" ) then
  echo "write_inserts_each_launch.sh"
  echo "Need to specify version..."
  exit
endif
set VER = $1

set OUTFILE = "write_inserts_ver${VER}.pl"
if ( -e $OUTFILE) then
  rm -f $OUTFILE
endif

echo '#\!/usr/bin/env perl'                    > $OUTFILE
echo '$tablename = "cross_analysis_table";'   >> $OUTFILE
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
echo '    $error = $t0[10];'                  >> $OUTFILE
echo ""                                       >> $OUTFILE
echo '    print "UPDATE $tablename SET'       >> $OUTFILE
echo "           result_ver${VER}     = \\'"'$'"result\\',"                >> $OUTFILE
echo "           cput_ver${VER}       = \\'"'$'"cput\\',"                  >> $OUTFILE
echo "           walltime_ver${VER}   = \\'"'$'"walltime\\',"              >> $OUTFILE
echo "           mem_ver${VER}        = \\'"'$'"mem\\',"                   >> $OUTFILE
echo "           vmem_ver${VER}       = \\'"'$'"vmem\\',"                  >> $OUTFILE
echo "           error_ver${VER}      = \\'"'$'"error\\',"                 >> $OUTFILE
echo "           nevents_ver${VER}    = "'$'"nevents,"                     >> $OUTFILE
echo "           timeCopy_ver${VER}   = "'$'"timeCopy,"                    >> $OUTFILE
echo "           timePlugin_ver${VER} = "'$'"timePlugin"                   >> $OUTFILE
echo "           WHERE        run = "'$'"run AND file = "'$'"file;\\n"'";' >> $OUTFILE
echo             "}"                                                       >> $OUTFILE
echo "exit;" >> $OUTFILE

chmod u+x ./write_inserts_ver${VER}.pl

time ./write_inserts_ver${VER}.pl  < jobinfo_ver${VER}.txt | mysql -hhallddb -ufarmer farming
