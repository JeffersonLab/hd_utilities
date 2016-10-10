#!/bin/bash

Setup_Environment()
{
	#if you change the below, you must also change them in the jobs_recon_test.config
	export WORK_AREA=/u/scratch/gluex/recon_test/

	# USE THE ENVIRONMENT FOR THE LATEST NIGHTLY BUILD
	source /group/halld/Software/scripts/monitoring/recon_test/env_recon_test.sh
	export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF
}

Setup_Config()
{
	RUN_PERIOD=$1
	cp $MONITORING_HOME/recon_test/jobs_recon_test.config .
	sed -i -e 's/YYYY-MM-DD/'$DATE'/g' jobs_recon_test.config       #MUST BE FIRST!!
	sed -i -e 's/YYYY-MM/'$RUN_PERIOD'/g' jobs_recon_test.config
}

Submit_Job()
{
	RUN_PERIOD=$1
	RUN_NUMBER=$2
	FILE_NUMBER=$3

	Setup_Config $RUN_PERIOD
	python $MONITORING_HOME/launch/launch.py ${WORK_AREA}/jobs_recon_test.config ${RUN_NUMBER} ${RUN_NUMBER} -f ${FILE_NUMBER}

	# pin the file to make sure it stays there for next time
	jcache pin /cache/halld/RunPeriod-${RUN_PERIOD}/rawdata/Run${RUN_NUMBER}/hd_rawdata_${RUN_NUMBER}_${FILE_NUMBER}.evio -D 14
}

######################################################### CHECK WORKFLOW ########################################################

Check_Workflow()
{
	local WORKFLOW_NAME=$1

	local STATUS_OUTPUT="$(swif status -workflow $WORKFLOW_NAME)"
	local RETURN_CODE=$?
	echo "SWIF Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
		return $RETURN_CODE
	fi

	local num_jobs_index=-1
	local num_succeeded_index=-1
	local word_count=0

	for word in $STATUS_OUTPUT; do
		((word_count++))
		if [ "$word" = "jobs" ]; then
			num_jobs_index=$(($word_count + 2))
		fi
		if [ "$word" = "succeeded" ]; then
			num_succeeded_index=$(($word_count + 2))
		fi
	done
	#echo $num_jobs_index $num_succeeded_index


	local num_jobs=`echo $STATUS_OUTPUT | cut -d " " -f $num_jobs_index`
	local num_succeeded=`echo $STATUS_OUTPUT | cut -d " " -f $num_succeeded_index`
	#echo $num_jobs $num_succeeded

	if [ "$num_jobs" = "$num_succeeded" ]; then
		return 0
	fi

	return 1
}

Check_Workflow_Loop()
{
	local WORKFLOW_NAME=$1
	sleep 3600  #don't check for at least 1 hour
	local sleep_length=900  #check every 15 minutes
	local max_trys=188 #give up after 48 hours
	for try_count in `seq 1 $max_trys`;
	do
		Check_Workflow $WORKFLOW_NAME
		local RETURN_CODE=$?
		echo "Sleep try_count: "$try_count", RETURN_CODE = "$RETURN_CODE
		if [ "$RETURN_CODE" -eq "0" ]; then
			return 0
		fi
		sleep $sleep_length  
	done

	return 1
}

########################################################### SEND EMAIL ##########################################################

Send_Email()
{
	cd /group/halld/Software/scripts/simple_email_list/lists/recon_test/
	echo https://halldweb.jlab.org/recon_test/$DATE/ >> message.txt
	/group/halld/Software/scripts/simple_email_list/scripts/simple_email_list.pl
}

######################################################### EXECUTE SCRIPT ########################################################

# SETUP LOCAL WORK AREA
Setup_Environment
mkdir -p -m 755 ${WORK_AREA}
cd ${WORK_AREA}

# SUBMIT JOBS FOR EACH WORKFLOW (RUN# must be 6 digits, and FILE# must be 3 digits!!)
Submit_Job 2016-02 011529 000
#Submit_Job 2016-10 020000 000

# CHECK FOR JOB SUCCESS. IF SUCCESS, SEND EMAIL
Check_Workflow_Loop recon_tests
RETURN_CODE=$?
echo "Job success return code = "$RETURN_CODE
if [ "$RETURN_CODE" -eq "0" ]; then
	Send_Email
fi

