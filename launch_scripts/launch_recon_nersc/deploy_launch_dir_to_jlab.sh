#!/usr/bin/env bash

# copies the launch directory to official location for production user account


THIS_SCRIPT_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"  # get the directory of this script
LAUNCH_ENV_FILE="${1}"  # path to .env file defining the configuration variables of the reconstruction launch
if [ -z "${LAUNCH_ENV_FILE}" ]
then
  echo "You must provide the configuration .env file as the first argument."
  exit 1
fi
echo "Reading production parameters from .env file '${LAUNCH_ENV_FILE}'"
source "${LAUNCH_ENV_FILE}"

REMOTE_LAUNCH_DIR="/home/${PRODUCTION_USER}/${PRODUCTION_LAUNCH_DIR}"
DEST="${PRODUCTION_USER}@ifarm:${REMOTE_LAUNCH_DIR}"
echo "Copying launch scripts and config files from '${THIS_SCRIPT_DIR}' to '${DEST}'"
# ensure the full destination path exists before rsync starts copying files.
if ! ssh "${PRODUCTION_USER}@ifarm" "mkdir --verbose --parents \"${REMOTE_LAUNCH_DIR}\""
then
  echo "ERROR: Could not create remote directory '${REMOTE_LAUNCH_DIR}'"
  exit 1
fi
TMP=$(mktemp)  # temporary file to hold list of files to copy
git ls-files -z >| "${TMP}"  # get NUL-delimited list of all files tracked by git (safe for spaces/newlines)
#TODO print warning message if any of the following files are not found
find . -maxdepth 1 -name "${RUN_NUMBER_LIST_FILE}" -print0 >> "${TMP}"
find . -maxdepth 1 -name "${JANA_CONFIG}" -print0 >> "${TMP}"
find . -maxdepth 1 -name "${LAUNCH_ENV_FILE}" -print0 >> "${TMP}"
RSYNC_CMD=(rsync
  --verbose
  --archive
  --chown="${PRODUCTION_USER}:"
  --chmod=g+w
  --from0  # tell rsync to read NUL-delimited input
  --files-from="${TMP}"
  "${THIS_SCRIPT_DIR}/"  # trailing slash is important: copy contents of `THIS_SCRIPT_DIR` into existing `DEST` directory
  "${DEST}"
)
"${RSYNC_CMD[@]}"
if "${RSYNC_CMD[@]}"
then
  echo "Successfully copied launch scripts and config files to '${DEST}'"
else
  echo "ERROR: Could not copy launch scripts and config files to '${DEST}'"
  exit 1
fi
rm --force "${TMP}"
unset TMP

# write current git hash (short) into deployed tree
GIT_HASH=$(git rev-parse --short HEAD 2> /dev/null)
if [[ -z "${GIT_HASH}" ]]
then
  echo "Could not determine git hash; set it to 'unknown'"
  GIT_HASH="unknown"
fi
ssh "${PRODUCTION_USER}@ifarm" "cat >! ~/\"${PRODUCTION_LAUNCH_DIR}\"/DEPLOYED_HD_UTILITIES_GIT_HASH" <<< "${GIT_HASH}"
# alternative: printf '%s' "$VAR" | ssh user@remote-host "cat > /path/to/file"

exit 0
