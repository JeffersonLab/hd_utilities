#!/bin/csh -f
setenv ENVIRONMENT $1
setenv INPUTFILE $2
setenv PLUGINS $3
setenv RUN_NUMBER $4
setenv FILE_NUMBER $5
setenv OUTDIR $6
setenv NTHREADS $7
setenv ADDITIONAL_OPTION $8
# setenv LOG $5

echo "starting........ "
date

source $ENVIRONMENT

echo $ENVIRONMENT
echo $INPUTFILE
echo $PLUGINS
echo $ADDITIONAL_OPTION
echo $RUN_NUMBER
echo $FILE_NUMBER
echo $OUTDIR

echo pwd = $PWD
echo "PATH = $PATH"

# echo LD_LIBRARY_PATH = $LD_LIBRARY_PATH
# echo PATH = $PATH

ls -l

# This step is necessary since the cache files will be
# created as links in the current directory, and
# we want to avoid large I/O processes.
# We first copy the input file to the current directory,
# then remove the link.
echo "copying input file............"
set START = `date +%s`
cp $INPUTFILE ./tmp.evio
set END = `date +%s`
set TIMEDIFF = `expr $END - $START`
echo "time to copy input file: $TIMEDIFF"
echo "finished copying input file..."
rm -f $INPUTFILE
mv tmp.evio $INPUTFILE

ls -l

echo "starting plugins............"
date
set START = `date +%s`
hd_root $INPUTFILE -PPLUGINS=$PLUGINS ${ADDITIONAL_OPTION} -PNTHREADS=$NTHREADS -PTHREAD_TIMEOUT=300 -PPRINT_PLUGIN_PATHS=1  # -PEVENTS_TO_KEEP=100
echo "ending plugins ............."
date
set END = `date +%s`
set TIMEDIFF = `expr $END - $START`
echo "TIMEDIFF: $TIMEDIFF"

# save ROOT histogram file
mkdir -p -m 775 ${OUTDIR}/ROOT/${RUN_NUMBER}/
cp -v hd_root.root ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root
chmod 775 ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root

# save 2track skim file
set BASENAME = `basename $INPUTFILE .evio`
mkdir -p -m 775 ${OUTDIR}/skims/${RUN_NUMBER}/
# cp -v ${BASENAME}.2tracks.evio ${OUTDIR}/skims/${RUN_NUMBER}/${BASENAME}.2tracks.evio
# chmod 775 ${BASENAME}.2tracks.evio ${OUTDIR}/skims/${RUN_NUMBER}/${BASENAME}.2tracks.evio

# save TOF TDC shift file
mkdir -p -m 775 ${OUTDIR}/misc/${RUN_NUMBER}/
# cp -v TOF_TDC_shift_${RUN_NUMBER}.txt ${OUTDIR}/misc/${RUN_NUMBER}/TOF_TDC_shift_${RUN_NUMBER}_${FILE_NUMBER}.txt

# save REST file
mkdir -p -m 775 ${OUTDIR}/REST/${RUN_NUMBER}/
cp -v dana_rest.hddm ${OUTDIR}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm
chmod 775 ${OUTDIR}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm

# save the EventStore idxa files
# (create directories in order so that permissions are set for each)
mkdir -p -m 775 ${OUTDIR}/idxa/
mkdir -p -m 775 ${OUTDIR}/idxa/${RUN_NUMBER}
mkdir -p -m 775 ${OUTDIR}/idxa/${RUN_NUMBER}/${FILE_NUMBER}/
cp -v *.idxa ${OUTDIR}/idxa/${RUN_NUMBER}/${FILE_NUMBER}/
chmod 775 ${OUTDIR}/idxa/${RUN_NUMBER}/${FILE_NUMBER}/*.idxa

# create txt file containing all skims and output idxa files
set IDXAOUT = ${OUTDIR}/idxa/${RUN_NUMBER}/${FILE_NUMBER}/idxafiles.txt
foreach FILE ( `ls *.idxa` )
  set SKIM = `echo $FILE | sed 's/eventlist_//' | sed 's/.idxa//'`
  echo "${SKIM}::${OUTDIR}/idxa/${RUN_NUMBER}/${FILE_NUMBER}/${FILE}" >> ${IDXAOUT}
end

# create directory for log files
mkdir -p -m 775 ${OUTDIR}/log/${RUN_NUMBER}/

# Set up python environment, then
# update monitoring information.
# echo "updating database ............."
# source /u/home/gxproj1/halld/monitoring/process/monitoring_env.csh
# /u/home/gxproj1/halld/monitoring/process/process_run_conditions.py $RUN_NUMBER
echo "ending job ............."
