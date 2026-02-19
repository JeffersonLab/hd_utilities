#!/usr/bin/env bash
# set -o nounset  # exit if trying to use an uninitialized variable
set -o verbose  # print shell input lines as they are read, i.e. before any expansion
set -o xtrace  # print commands and their arguments as they are executed, i.e. after expansion and without I/O redirection

# This script is the entrypoint for a large multi-file
# job. SLURM will run it and SWIF2 will make sure it
# starts in a working directory that has links to all
# of the raw data files in it.
#
# This is really just a wrapper for the python script.
# It dumps the environment and hostname into the top
# level directory for the job and then defers everything
# else to python.


LAUNCH_DIR="${1}"

printenv > job.env.out
hostname > job.hostname.out
python3 "${LAUNCH_DIR}/script_nersc_multi_test.py" "${@}"
