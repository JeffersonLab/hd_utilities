#!/bin/csh -f
setenv ENVIRONMENT $1
setenv INPUTFILE $2
setenv PLUGINS $3
setenv BFIELD_OPTION $4
setenv NTHREADS $5
# setenv LOG $5

echo "starting........ "
date

source $ENVIRONMENT

echo $ENVIRONMENT
echo $INPUTFILE
echo $PLUGINS
echo $BFIELD_OPTION
echo $NTHREADS

echo pwd = $PWD

echo LD_LIBRARY_PATH = $LD_LIBRARY_PATH

echo PATH = $PATH

# echo PREFIX = $PREFIX
echo

echo "starting plugins............"
date
hd_root $INPUTFILE -PPLUGINS=$PLUGINS ${BFIELD_OPTION} -PNTHREADS=$NTHREADS -PTHREAD_TIMEOUT=300 # |& tee $LOG
echo "ending plugins ............."
date


# Set up python environment, then
# update monitoring information.
echo "updating database ............."
source /u/home/gluex/halld/monitoring/process/monitoring_env.sh
/u/home/gluex/halld/monitoring/process/process_run_conditions.py $RUN_NUMBER
echo "ending job ............."
