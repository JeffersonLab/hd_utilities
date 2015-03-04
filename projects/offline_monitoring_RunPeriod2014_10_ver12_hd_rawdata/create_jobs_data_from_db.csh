#!/bin/tcsh

# Tables will have names
# 1. offmon_rp[DATAPERIOD]_v[VER]_type[TYPE]
# 2. offmon_rp[DATAPERIOD]_v[VER]_type[TYPE]Job
#
# examples:
# - [DATAPERIOD] : rp2014m10
# - [VER]    : 10
# - [TYPE]       : 1 or 2 (data file was hd_raw or hd_rawdata)

set TYPE       = "1"
set TABLEJOB = "offline_monitoring_RunPeriod2014-10_ver12_hd_rawdataJob"
set TABLEAUX = "offline_monitoring_RunPeriod2014-10_ver12_hd_rawdata_aux"

mysql -hhalldweb1 -ufarmer farming -s -r -e "select ${TABLEJOB}.id, run, file, ${TABLEJOB}.jobId, ${TABLEJOB}.timeChange, hostname, \
status, exitCode, result, timeSubmitted, timeDependency, timePending, timeStagingIn, timeActive, timeStagingOut, timeComplete, \
walltime, cput, mem, vmem, error, nevents, timeCopy, timePlugin from ${TABLEJOB}, ${TABLEAUX} \
where ${TABLEJOB}.jobId = ${TABLEAUX}.jobId" > jobs_data.txt
