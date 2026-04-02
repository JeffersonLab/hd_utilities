#!/usr/bin/env bash

# copies the launch directory to official location for production user account


CONFIG_FILE="${1:-launch.env}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
# shellcheck source=./launch.env
source "${CONFIG_FILE}"

REMOTE_LAUNCH_DIR="/home/${PRODUCTION_USER}/${PRODUCTION_LAUNCH_DIR}"
echo "Copying launch scripts and config files from '$(pwd)' to '${PRODUCTION_USER}@ifarm:${REMOTE_LAUNCH_DIR}'"
# ensure the full destination path exists before rsync starts copying files.
if ! ssh "${PRODUCTION_USER}@ifarm" "mkdir --verbose --parents \"${REMOTE_LAUNCH_DIR}\""
then
  echo "ERROR: Could not create remote directory '${REMOTE_LAUNCH_DIR}'"
  exit 1
fi
TMP=$(mktemp)  # temporary file to hold list of files to copy
git ls-files -z >| "${TMP}"  # get NUL-delimited list of all files tracked by git (safe for spaces/newlines)
find . -maxdepth 1 -name "${RUN_NUMBER_LIST_FILE}" -print0 >> "${TMP}"
find . -maxdepth 1 -name "${JANA_CONFIG}" -print0 >> "${TMP}"
find . -maxdepth 1 -name "${CONFIG_FILE}" -print0 >> "${TMP}"
RSYNC_CMD=(rsync
  --verbose
  --archive
  --chown="${PRODUCTION_USER}:"
  --chmod=g+w
  --from0  # tell rsync to read NUL-delimited input
  --files-from="${TMP}"
  ./
  "${PRODUCTION_USER}@ifarm:${REMOTE_LAUNCH_DIR}/"
)
"${RSYNC_CMD[@]}"
rm --force "${TMP}"

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
