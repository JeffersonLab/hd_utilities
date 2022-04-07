#!/bin/bash
#
# This is the standard job script for running on the OSG, based on the 
# similar script for NERSC.  We've got CVFMS and containers, so it's
# not so different
#
# Arguments:
#
# arg 1:  JANA config file
# arg 2:  halld_recon version
# arg 3:  run period
# arg 4:  run period version
# arg 5:  run
# arg 6:  file

JANA_CONFIG=$1
HALLD_RECON_VERSION=$2
RUNPERIOD=$3
RUNPERIOD_VER=$4
RUN=$5
FILE=$6

# print some diagnostics
hostname
#ulimit -c unlimited
echo JANA_CONFIG=$1
echo HALLD_RECON_VERSION=$2
echo RUNPERIOD=$3
echo RUNPERIOD_VER=$4
echo RUN=$5
echo FILE=$6


# Setup environment for the specified software version
#source /cvmfs/oasis.opensciencegrid.org/gluex/group/halld/Software/build_scripts/gluex_env_jlab.csh /cvmfs/oasis.opensciencegrid.org/gluex/group/halld/www/halldweb/html/halld_versions/version_4.21.1.xml
source /group/halld/Software/build_scripts/gluex_env_jlab.sh $HALLD_RECON_VERSION

# Use CCDB and RCDB from CVMFS. Make a temporary local
# copy so that we don't interfere with other jobs locking
# the same file
cp -v /group/halld/www/halldweb/html/dist/ccdb.sqlite .
cp -v /group/halld/www/halldweb/html/dist/rcdb.sqlite .
export JANA_CALIB_URL=sqlite:///`pwd`/ccdb.sqlite
export CCDB_CONNECTION=$JANA_CALIB_URL
export RCDB_CONNECTION=sqlite:///`pwd`/rcdb.sqlite

# Use JANA_RESOURCE_DIR from CVMFS
#export JANA_RESOURCE_DIR=/cvmfs/oasis.opensciencegrid.org/gluex/group/halld/www/halldweb/html/resources
export JANA_RESOURCE_DIR=/group/halld/www/halldweb/html/resources

# Record some info about the node and environment
rm -f cpuinfo.out
cat /proc/cpuinfo > cpuinfo.out

rm -f env.out
env > env.out

rm -f hostname.out
hostname > hostname.out

echo ==files before==
ls -lh

# test
curl -O http://hadron.physics.fsu.edu/~sdobbs/time
chmod +x time

# Run hd_root
echo running over /cvmfs/gluex.osgstorage.org/gluex/uconn1/resilient/RunPeriod-${RUNPERIOD}/recon/ver${RUNPERIOD_VER}/REST/${RUN}/dana_rest_${RUN}_${FILE}.hddm
echo command: hd_root --config=${JANA_CONFIG} /cvmfs/gluex.osgstorage.org/gluex/uconn1/resilient/RunPeriod-${RUNPERIOD}/recon/ver${RUNPERIOD_VER}/REST/${RUN}/dana_rest_${RUN}_${FILE}.hddm
./time -v hd_root --config=${JANA_CONFIG} /cvmfs/gluex.osgstorage.org/gluex/uconn1/resilient/RunPeriod-${RUNPERIOD}/recon/ver${RUNPERIOD_VER}/REST/${RUN}/dana_rest_${RUN}_${FILE}.hddm

echo ==files after==
ls -lh


# CLEAN UP
# Move ROOT files into a directory to ship them back
OUTDIR=out_${RUN}_${FILE}
mkdir $OUTDIR
mv *.root $OUTDIR

# Move small files into a directory and make a tarball
#JOB_INFO=$(printf "job_info_%06d_%03d" $RUN $FILE)
JOB_INFO=job_info_${RUN}_${FILE}
mkdir $JOB_INFO
mv top.out cpuinfo.out env.out hostname.out std.err $JOB_INFO
tar czf ${JOB_INFO}.tgz $JOB_INFO
rm -rf $JOB_INFO

# Remove ccdb.sqlite and rcdb.sqlite files
rm -f /tmp/ccdb.sqlite /tmp/rcdb.sqlite
