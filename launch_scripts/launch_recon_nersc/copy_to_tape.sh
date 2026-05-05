#!/usr/bin/env bash

# copies the full content of the given source directory to tape using `jmigrate`
#NOTE needs to be run periodically until all files have been copied to tape
#NOTE running multiple instances of this script in parallel is not supported
#NOTE the preferred way to copy data to tape is to submit a dedicated swif2 job using `submit_copy_dir_to_tape.py`


LAUNCH_ENV_FILE="${1}"  # path to .env file defining the configuration variables of the reconstruction launch (required argument)
source utilities.sh
source_config_file "${LAUNCH_ENV_FILE}"

RECON_SRC_DIR="ver02-perl.ready_for_tape.2/job_info"
RECON_SRC_PATH="/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/${RECON_SRC_DIR}"  # absolute path to the prepared reconstruction directory, the content of which will be copied to tape
SRC_PATH_REPLACE="/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/ver02-perl.ready_for_tape.2"  # part of path to replace by DEST_PATH_REPLACE
DEST_PATH_REPLACE="/mss/halld/RunPeriod-2022-05/recon/ver02"  # path that replaces SRC_PATH_REPLACE
export JMIRROR_LOG_DIR="$(realpath "$(eval echo "~${PRODUCTION_USER}")/${PRODUCTION_LAUNCH_DIR}")/jmigrate_logs/${RECON_SRC_DIR}"  # directory to store jmigrate logs
# debug
# RECON_SRC_PATH="/w/halld-scshelf2101/bgrube/halldRepos/hd_utilities/launch_scripts/launch_recon_nersc/test/test_work_dir_job"
# SRC_PATH_REPLACE="${RECON_SRC_PATH}"
# DEST_PATH_REPLACE="/mss/halld/home/bgrube/test/test_copy_dir_to_tape2"
# export JMIRROR_LOG_DIR="./jmigrate_logs"

# export JMIRROR_MIN_MODIFICATION_AGE_SECONDS=600  # default is 1800 seconds, 0 means copy all files regardless of modification time
echo "Copying files from '${RECON_SRC_PATH}' to tape, replacing '${SRC_PATH_REPLACE}' with '${DEST_PATH_REPLACE}'"
JMIGRATE_CMD=(jmigrate
  "${RECON_SRC_PATH}"  # needs to be first argument so that lock file in /tmp is named correctly
  "${SRC_PATH_REPLACE}"
  "${DEST_PATH_REPLACE}"
  # -verbose  # not very helpful
  -delete written
)
echo "Running ${JMIGRATE_CMD[*]}"
echo "See logs in '${JMIRROR_LOG_DIR}'"
"${JMIGRATE_CMD[@]}"
