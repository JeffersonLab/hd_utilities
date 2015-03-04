fill_in_job_details.pl test2
mysql -hhalldweb1 -ufarmer farming -e "select id,run,file,jobId,hostname,status,timeSubmitted,timeActive,walltime,cput,timeComplete,result,error from test2Job"
