#!/bin/bash
#
# NOTE: all sbatch options are given via command line by
# swif2 when it submits the job. This is so they
# can be specified in the script run at JLab (launch_psc.py)
# without having to modify this file.
#
# This script will be run by the batch job on the PSC node.
# It will wake up in the working directory swif2 has created
# for the job. The input file will have a symbolic link in
# the current working directory.
#
# Arguments passed to this script:
#
# arg 1: singularity image (full path to image on bridges)
# arg 2: script (full path. usually to script_psc.sh)
# arg 3: jana config file
# arg 4: recon version (sub-directory of /group/halld/Software/builds/Linux_CentOS7-x86_64-gcc4.8.5-cntr)
# arg 5: RUN
# arg 6: FILE
#

export LAUNCHDIR=$1
export IMAGE=$2
export SCRIPTFILE=$3
export JANACONFIG=$4
export RECON_VER=$5
export RUN=$6
export FILE=$7

cvmfs_config probe
module load singularity
singularity run -B/cvmfs:/cvmfs -B${LAUNCHDIR}:/launch $IMAGE $SCRIPTFILE $JANACONFIG $RECON_VER $RUN $FILE



