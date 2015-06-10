#!/bin/tcsh

# Tables will have names
# 1. offmon_rp[DATAPERIOD]_v[VER]_type[TYPE]
# 2. offmon_rp[DATAPERIOD]_v[VER]_type[TYPE]Job
#
# examples:
# - [DATAPERIOD] : rp2014m10
# - [VER]    : 10
# - [TYPE]       : 1 or 2 (data file was hd_raw or hd_rawdata)

set TABLEJOB = "PROJECTJob"
set TABLEAUX = "PROJECT_aux"

# Print out results to txt file.
# Format times so that they are a single string, 20YY-MM-DD-HH:MM:SS
# This helps when the time is "NULL".
# Next, make each entry a single line. This helps in reading the values in.
mysql -hhallddb -ufarmer farming -s -r -e "select ${TABLEJOB}.id, run, file, ${TABLEJOB}.jobId, REPLACE(${TABLEJOB}.timeChange, ' ', '_'), hostname, status, exitCode, result, REPLACE(timeSubmitted, ' ', '_'), REPLACE(timeDependency, ' ', '_'), REPLACE(timePending, ' ', '_'), REPLACE(timeStagingIn, ' ', '_'), REPLACE(timeActive, ' ', '_'), REPLACE(timeStagingOut, ' ', '_'), REPLACE(timeComplete, ' ', '_'), walltime, cput, mem, vmem, REPLACE(error, ' ', '_'), nevents, timeCopy, timePlugin from ${TABLEJOB}, ${TABLEAUX} where ${TABLEJOB}.jobId = ${TABLEAUX}.jobId" | sed -e 's/\s/\n/g' > new_jobs_data.txt
