#!/usr/bin/env bash

# Creates tarballs for all log-file directories of the reconstruction jobs in SWIF_LOG_FILES_OUTPUT_DEST_DIR

shopt -s nullglob  # make non-matching globs expand to an empty list instead of themselves


LAUNCH_ENV_FILE="${1}"  # path to .env file defining the configuration variables of the reconstruction launch (required argument)
source utilities.sh || { echo "ERROR: Failed to source utilities.sh"; exit 1; }
source_config_file "${LAUNCH_ENV_FILE}" || { echo "ERROR: Failed to source config file"; exit 1; }

# create tarballs for all log-file directories created above
LOG_FILE_DIR_PATH="${SWIF_LOG_FILES_OUTPUT_DEST_DIR}/job_info"
echo "Creating tarballs for all log-file directories in '${LOG_FILE_DIR_PATH}'"
for JOB_INFO_DIR_PATH in "${LOG_FILE_DIR_PATH}"/??????/job_info_??????_???
do
  echo "..............................................................................."
  echo "Processing '${JOB_INFO_DIR_PATH}'"
  if tar --verbose --create --gzip --file="${JOB_INFO_DIR_PATH}.tgz" --directory="$(dirname "${JOB_INFO_DIR_PATH}")" "$(basename "${JOB_INFO_DIR_PATH}")"
  then
    JOB_INFO_DIR_BACKUP_PATH="${JOB_INFO_DIR_PATH/"${SWIF_LOG_FILES_OUTPUT_DEST_DIR}"/"${SWIF_LOG_FILES_OUTPUT_DEST_DIR}.tarred"}"
    echo "tar was successful; moving '${JOB_INFO_DIR_PATH}' to backup location '${JOB_INFO_DIR_BACKUP_PATH}'"
    mkdir --parents --verbose "$(dirname "${JOB_INFO_DIR_BACKUP_PATH}")"
    mv --verbose "${JOB_INFO_DIR_PATH}" "${JOB_INFO_DIR_BACKUP_PATH}"
  else
    echo "ERROR: tar failed for '${JOB_INFO_DIR_PATH}'"
  fi
done
