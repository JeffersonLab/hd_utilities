#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks-per-node 28
#SBATCH -p RM
#SBATCH -t 0:30:00

#
# Note: To test interactively with cvmfs you need to issue:
#
#  interact --egress
#

nthreads=$1

export WORKDIR=${HOME}/pylon5/work/2019.04.05.scale_test/nthreads_$nthreads
export IMAGE=${HOME}/pylon5/singularity/gluex_docker_devel.simg
export JANACONFIG=${WORKDIR}/jana.conf
export RECON_VER=halld_recon/halld_recon-4.1.1
export RUN=40856
export FILE=0

cd $WORKDIR
cvmfs_config probe
module load singularity
singularity run -B/cvmfs:/cvmfs $IMAGE ../script_psc.sh $JANACONFIG $RECON_VER $RUN $FILE



