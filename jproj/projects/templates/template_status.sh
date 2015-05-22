#!/bin/tcsh
# get current directory name
set PROJECT = `basename $PWD`

source PROJHOME/../scripts/setup.csh
fill_in_job_details.pl ${PROJECT}
mysql -hhallddb -ufarmer farming -e "select id,run,file,jobId,hostname,status,timeSubmitted,timeActive,walltime,cput,timeComplete,result,error from ${PROJECT}Job"
