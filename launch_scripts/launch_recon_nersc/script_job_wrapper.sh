#!/usr/bin/env bash

# Wrapper script that sets up the Python environment for the actual job script `script_job.py`.
# First argument is the path to the launch directory at NERSC.
# All other arguments are passed on to `script_job.py`.

NERSC_LAUNCH_DIR="${1}"
echo "Setting up Python 3.9 environment and running job script at '${NERSC_LAUNCH_DIR}/script_job.py'"
module load python/3.9
PYTHONPATH="${NERSC_LAUNCH_DIR}:${PYTHONPATH}" "${NERSC_LAUNCH_DIR}"/script_job.py "${@:2}"
