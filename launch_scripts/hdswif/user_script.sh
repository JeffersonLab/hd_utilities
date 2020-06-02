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

echo "starting........ "
date

echo "ENVIRONMENT  = $ENVIRONMENT"
echo "INPUTFILE    = $INPUTFILE"
echo "RUN_NUMBER   = $RUN_NUMBER"
echo "FILE_NUMBER  = $FILE_NUMBER"
echo "OUTDIR       = $OUTDIR"
echo "NTHREADS     = $NTHREADS"

source $ENVIRONMENT

echo pwd = $PWD
echo "PATH = $PATH"

echo LD_LIBRARY_PATH = $LD_LIBRARY_PATH
echo PATH = $PATH

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

echo "starting plugins............"
date
set START = `date +%s`
hd_root $INPUTFILE -PPLUGINS=CDC_online -PNTHREADS=$NTHREADS -PTHREAD_TIMEOUT=300 -PPRINT_PLUGIN_PATHS=1 -PEVIO:ENABLE_DISENTANGLING=0
echo "ending plugins ............."
date
set END = `date +%s`
set TIMEDIFF = `expr $END - $START`
echo "TIMEDIFF: $TIMEDIFF"

# save ROOT output file
mkdir -p -m 775 ${OUTDIR}/ROOT/${RUN_NUMBER}/
cp -v hd_root.root ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root
chmod 775 ${OUTDIR}/ROOT/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root

# create directory for log files
mkdir -p -m 775 ${OUTDIR}/log/${RUN_NUMBER}/

echo "ending job ............."
date
