fill_in_job_details.pl offmon_rp2014m10_v10_type2
mysql -hhalldweb1 -ufarmer farming -e "select id,run,file,jobId,hostname,status,timeSubmitted,timeActive,walltime,cput,timeComplete,result,error from offmon_rp2014m10_v10_type2Job"
