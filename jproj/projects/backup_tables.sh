#!/bin/tcsh

#--------------------------------------------------------------------------
#
# This script will backup the tables created for offline monitoring
# using the command mysqldump.
#
# Executing the output sql file will recreate the table so acts
# as a backup. Note that doing so drops the table if it exists,
# so caution is advised.
#
#--------------------------------------------------------------------------

# Process tables for run periods and versions
set RUNPERIOD = $1
set VER       = $2

if ( $RUNPERIOD != "2014_10" && $RUNPERIOD != "2015_03" ) then
  echo "backup_tables.sh:"
  echo "backup_tables.sh [run period] [ver]"
  echo "Run period must be '2014_10' or '2015_03'"
  exit
endif

if ( $RUNPERIOD == "2014_10" && $VER == "09" ) then
  mysqldump -hhallddb -ufarmer farming offline_mon     \
                                       offline_monJob  \
                                       offline_monAux \
    > /group/halld/data_monitoring/table_backup/offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}.sql

else if ( $RUNPERIOD == "2014_10" && $VER == "10" ) then
  mysqldump -hhallddb -ufarmer farming offmon_rp2014m10_v10_type1     \
                                       offmon_rp2014m10_v10_type1Job  \
                                       offmon_rp2014m10_v10_type1_aux \
    > /group/halld/data_monitoring/table_backup/offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}.sql

  mysqldump -hhallddb -ufarmer farming offmon_rp2014m10_v10_type2     \
                                       offmon_rp2014m10_v10_type2Job  \
    > /group/halld/data_monitoring/table_backup/offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}_type2.sql

else
  mysqldump -hhallddb -ufarmer farming offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}_hd_rawdata     \
                                       offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}_hd_rawdataJob  \
                                       offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}_hd_rawdata_aux \
    > /group/halld/data_monitoring/table_backup/offline_monitoring_RunPeriod${RUNPERIOD}_ver${VER}.sql
endif
