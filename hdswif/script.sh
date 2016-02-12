#!/bin/csh -f

# --------------------------------------------------------
#
# Generic script to run hd_root on cluster.
# Modify hd_root command to have plugin(s) of choice.
#
# --------------------------------------------------------
setenv ENVIRONMENT $1
setenv INPUTFILE $2
setenv RUN_NUMBER $3
setenv FILE_NUMBER $4
setenv OUTDIR $5
setenv NTHREADS $6
setenv PLUGINS $7

echo "starting........ "
date

echo "ENVIRONMENT  = $ENVIRONMENT"
echo "INPUTFILE    = $INPUTFILE"
echo "RUN_NUMBER   = $RUN_NUMBER"
echo "FILE_NUMBER  = $FILE_NUMBER"
echo "OUTDIR       = $OUTDIR"
echo "NTHREADS     = $NTHREADS"
echo "PLUGINS      = $PLUGINS"

source $ENVIRONMENT

echo pwd = $PWD
printenv
echo "PERL INCLUDES: "
perl -e "print qq(@INC)"

ls -l

# This step is necessary since the cache files will be
# created as soft links in the current directory, and
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

set THREAD_TIMEOUT_VALUE = 300
set CALIB_CONTEXT_VALUE = ""

echo "starting plugins............"
date
set START = `date +%s`
#hd_root $INPUTFILE -PPLUGINS=CDC_online -PNTHREADS=$NTHREADS -PTHREAD_TIMEOUT=300 -PPRINT_PLUGIN_PATHS=1 -PEVIO:ENABLE_DISENTANGLING=0
hd_root $INPUTFILE -PPLUGINS=$PLUGINS -PNTHREADS=$NTHREADS -PTHREAD_TIMEOUT=$THREAD_TIMEOUT_VALUE
set RETURN_CODE = $?
echo Return Code = $RETURN_CODE
echo "ending plugins ............."
date
set END = `date +%s`
set TIMEDIFF = `expr $END - $START`
echo "TIMEDIFF: $TIMEDIFF"
if ($RETURN_CODE != 0) then
	exit $RETURN_CODE
endif

# save ROOT output file
echo "Saving ROOT file"
mkdir -p -m 775 ${OUTDIR}/ROOT/${RUN_NUMBER}/
if (-e hd_root.root) then
	cp -v hd_root.root ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root
	chmod 664 ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root
endif

# save REST output file
echo "Saving REST file"
mkdir -p -m 775 ${OUTDIR}/REST/${RUN_NUMBER}/
if (-e dana_rest.hddm) then
	cp -v dana_rest.hddm ${OUTDIR}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm
	chmod 664 ${OUTDIR}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm
endif

# create directory for log files
mkdir -p -m 775 ${OUTDIR}/log/${RUN_NUMBER}/

echo "ending job ............."
date
