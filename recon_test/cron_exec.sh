#!/bin/bash

Setup_Environment()
{
	# USE THE ENVIRONMENT FOR THE LATEST NIGHTLY BUILD
	export BMS_OSNAME=Linux_RHEL7-x86_64-gcc4.9.2
	export DATE=`date +%Y-%m-%d`
	source /group/halld/Software/build_scripts/gluex_env_jlab.csh /scratch/gluex/nightly/${DATE}/${BMS_OSNAME}/version_${DATE}.xml -v
	export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF

	#if you change the below, you must also change them in the jobs_recon_test.config
	export MONITORING_HOME=/group/halld/Software/scripts/monitoring/
	export WORK_AREA=/u/scratch/gluex/recon_test/
}

Setup_Environment_File()
{
	export ENV_FILE_NAME=env_recon_test.sh
	echo 'source /group/halld/Software/build_scripts/gluex_env_jlab.csh /scratch/gluex/nightly/'${DATE}'/'${BMS_OSNAME}'/version_'${DATE}'.xml -v' >> $ENV_FILE_NAME
}

Setup_Config()
{
	cp $MONITORING_HOME/recon_test/jobs_recon_test.config .
	sed -i -e 's/YYYY-MM-DD/'$DATE'/g' jobs_recon_test.config #MUST BE FIRST!!
	sed -i -e 's/YYYY-MM/'$1'/g' jobs_recon_test.config
}

######################################################### EXECUTE SCRIPT ########################################################


# SETUP LOCAL WORK AREA
mkdir -p -m 755 ${WORK_AREA}
cd ${WORK_AREA}

# SUBMIT JOBS FOR EACH WORKFLOW
# launch.py args: job_config, min_run, max_run, -f, file_#

# Run Period 2016-02
Setup_Config 2016-02
python $MONITORING_HOME/launch/launch.py ${WORK_AREA}/jobs_recon_test.config 11529 11529 -f 000

# Run Period 2016-10
#Setup_Config 2016-10
#python $MONITORING_HOME/launch/launch.py ${WORK_AREA}/jobs_recon_test.config 20000 20000 -f 000

