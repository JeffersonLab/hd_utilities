#!/usr/bin/env bash

# Checks the disk space usage and prints the quota for the `jlab` user account.


LAUNCH_ENV_FILE="${1}"  # path to .env file defining the configuration variables of the reconstruction launch (required argument)
source utilities.sh
source_config_file "${LAUNCH_ENV_FILE}"

echo "NERSC Scratch Storage Space used by swif2"
echo "swif2 input files:"
du -hs "${NERSC_SWIF_INPUT_ROOT_DIR}"
echo "swif2 job directories:"
du -hs "${NERSC_SWIF_JOBS_ROOT_DIR}"
echo
echo "NERSC Community File System (CFS) Storage Space used"
du -hs "${NERSC_PROJECT_DIR}"
echo
echo "Quota for user 'jlab'"
myquota --full-path --limit --common --hpss --user jlab
