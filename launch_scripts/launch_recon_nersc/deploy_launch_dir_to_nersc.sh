#!/usr/bin/env bash

# copies the launch directory to official location at NERSC


THIS_SCRIPT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"  # get the directory of this script
LAUNCH_ENV_FILE="${1}"  # path to .env file defining the configuration variables of the reconstruction launch
if [ -z "${LAUNCH_ENV_FILE}" ]
then
  echo "You must provide the configuration .env file as the first argument."
  exit 1
fi
echo "Reading production parameters from .env file '${LAUNCH_ENV_FILE}'"
source "${LAUNCH_ENV_FILE}"

DEST="${NERSC_HOST}:${NERSC_LAUNCH_DIR}"
echo "Copying launch scripts and config files from '${THIS_SCRIPT_DIR}' to '${DEST}'"
# rsync cannot set permissions on the destination directory if it does not exist beforehand

if ssh "${NERSC_HOST}" "mkdir --verbose --parents '${NERSC_LAUNCH_DIR}' && chown --verbose :${NERSC_PROJECT} '${NERSC_LAUNCH_DIR}'"
then
  echo "Successfully created launch directory '${NERSC_LAUNCH_DIR}' at NERSC with group ownership '${NERSC_PROJECT}' and write permissions for the group"
else
  echo "ERROR: Could not create remote directory '${NERSC_LAUNCH_DIR}' at NERSC with group ownership '${NERSC_PROJECT}' and write permissions for the group; aborting"
  exit 1
fi
#TODO exclude __pycache__ directories; use `--exclude` option of rsync; better copy only the files needed at NERSC; e.g. use `--files-from` option of rsync with a generated list of files to copy
RSYNC_CMD=(rsync
  --verbose
  --delete  # ensure pristine copy
  --archive
  --ignore-times
  --chown=:"${NERSC_PROJECT}"  # ensure write permissions for project group
  --chmod="Dg+rwx,Fg+rw"  # ensure write permissions for project group for subdirectories and files
  "${THIS_SCRIPT_DIR}/"  # trailing slash is important: copy contents of `THIS_SCRIPT_DIR` into existing `DEST` directory
  "${DEST}"
)
if "${RSYNC_CMD[@]}"
then
  echo "Successfully copied launch scripts and config files to '${DEST}'"
else
  echo "ERROR: Could not copy launch scripts and config files to '${DEST}'"
  exit 1
fi
unset DEST

exit 0
