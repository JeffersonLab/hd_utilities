#!/usr/bin/env bash

# Checks the disk space usage and prints the quota for the `jlab` user account.


THIS_SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"  # get the directory of this script
CONFIG_FILE="${1:-${THIS_SCRIPT_DIR}/launch.env}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
source "${CONFIG_FILE}"

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
