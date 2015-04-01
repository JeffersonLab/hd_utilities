#!/bin/tcsh

if ( $1 == "" ) then
  echo "create_txt_file_from_dbs.sh"
  echo "Need to specify version..."
  exit
endif
set VER = $1

# Tables will have names
# 1. offmon_rp[DATAPERIOD]_v[VER]_type[TYPE]
# 2. offmon_rp[DATAPERIOD]_v[VER]_type[TYPE]Job

# standard names
set TABLEJOB = "offline_monitoring_RunPeriod2014_10_ver${VER}_hd_rawdataJob"
set TABLEAUX = "offline_monitoring_RunPeriod2014_10_ver${VER}_hd_rawdata_aux"
set OUTFILE  = "jobinfo_ver${VER}.txt"

if ( $VER == "10" ) then
  # for ver10
  set TABLEJOB = "offmon_rp2014m10_v10_type1Job"
else if ( $VER == "09" ) then
  # for ver09
  set TABLEJOB = "offline_monJob"
endif

echo $TABLEJOB
echo $TABLEAUX
echo $OUTFILE


mysql -hhallddb -ufarmer farming -s -r -e "select run, file, result, cput, walltime, mem, vmem, nevents, timeCopy, timePlugin, error \
from ${TABLEJOB}, ${TABLEAUX} where ${TABLEJOB}.jobId = ${TABLEAUX}.jobId" > tmp.txt

cat tmp.txt | sed 's:Job exceeded resource limit.:resource_limit:' \
            | sed 's:Job timed out.:timeout:' \
            | sed 's:Job failed with unknown reason.:unknown_fail:' \
            | sed 's:fail to get input file:input_fail:' \
            > $OUTFILE
rm -f tmp.txt
