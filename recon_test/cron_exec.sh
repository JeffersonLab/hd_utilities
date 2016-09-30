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
	sed -i -e 's/YYYY-MM-DD/'$DATE'/g' jobs_recon_test.config #MUST BE FIRST!!
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

######################################################### EXECUTE SCRIPT ########################################################

# SETUP LOCAL WORK AREA
Setup_Environment
mkdir -p -m 755 ${WORK_AREA}
cd ${WORK_AREA}

# SUBMIT JOBS FOR EACH WORKFLOW (RUN# must be 6 digits, and FILE# must be 3 digits!!)
Submit_Job 2016-02 011529 000
#Submit_Job 2016-10 020000 000

