#!/bin/csh -f
setenv ENVIRONMENT $1
setenv INPUTFILE $2
setenv PLUGINS $3
setenv BFIELD_OPTION $4
setenv ADDITIONAL_OPTION $5
setenv RUN_NUMBER $6
setenv FILE_NUMBER $7
setenv OUTDIR $8
# setenv LOG $5

echo "starting........ "
date

source $ENVIRONMENT

echo $ENVIRONMENT
echo $INPUTFILE
echo $PLUGINS
echo $BFIELD_OPTION
echo $ADDITIONAL_OPTION
echo $RUN_NUMBER
echo $FILE_NUMBER
echo $OUTDIR

echo pwd = $PWD

# echo LD_LIBRARY_PATH = $LD_LIBRARY_PATH
# echo PATH = $PATH

echo "starting plugins............"
date
hd_root $INPUTFILE -PPLUGINS=$PLUGINS ${BFIELD_OPTION} ${ADDITIONAL_OPTION} -PNTHREADS=1 -PTHREAD_TIMEOUT=300 -PPRINT_PLUGIN_PATHS=1 # -PEVENTS_TO_KEEP=100 # |& tee $LOG
echo "ending plugins ............."
date

# save ROOT histogram file
cp -v hd_root.root ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root

# save 2track skim file
set BASENAME = `basename $INPUTFILE .evio`
cp -v ${BASENAME}.2tracks.evio ${OUTDIR}/skims/${RUN_NUMBER}/${BASENAME}.2tracks.evio

# save TOF TDC shift file
cp -v TOF_TDC_shift_${RUN_NUMBER}.txt ${OUTDIR}/misc/${RUN_NUMBER}/TOF_TDC_shift_${RUN_NUMBER}_${FILE_NUMBER}.txt

# Set up python environment, then
# update monitoring information.
# echo "updating database ............."
# source /u/home/gxproj1/halld/monitoring/process/monitoring_env.csh
# /u/home/gxproj1/halld/monitoring/process/process_run_conditions.py $RUN_NUMBER
echo "ending job ............."
