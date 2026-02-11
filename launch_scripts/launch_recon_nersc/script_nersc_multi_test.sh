#!/bin/bash
#
# This script is the entrypoint for a large multi-file
# job. SLURM will run it and SWIF2 will make sure it
# starts in a working directory that has links to all
# of the raw data files in it.
#
# This is really just a wrapper for the python script.
# It dumps the environment and hostname into the top
# level directory for the job and then defers everything
# else to python.

LAUNCHDIR=$1

printenv > env.out
hostname > hostname.out
#python3 ${LAUNCHDIR}/script_nersc_multi.py $@
python3 ${LAUNCHDIR}/script_nersc_multi_test.py $@
