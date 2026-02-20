#!/usr/bin/env bash

# copies the launch directory to official location for production user account

CONFIG_FILE="${1:-./do_my_launch.config.sh}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
# shellcheck source=./do_my_launch.config.sh
source "${CONFIG_FILE}"

echo "Copying launch scripts and config files from '$(pwd)' to '${PRODUCTION_USER}@ifarm:~/${PRODUCTION_LAUNCH_DIR}'"
TMP=$(mktemp)
git ls-files -z >| "${TMP}"  # get NUL-delimited list of all files tracked by git (safe for spaces/newlines)
RSYNC_CMD=(rsync
  --verbose
  --archive
  --chown="${PRODUCTION_USER}:"
  --chmod=g+w
  --from0  # tell rsync to read NUL-delimited input
  --files-from="${TMP}"
  ./
  "${PRODUCTION_USER}@ifarm:/u/home/${PRODUCTION_USER}/${PRODUCTION_LAUNCH_DIR}/"  #NOTE there seems to be no sane way make rsync expand `~` or `${HOME}` correctly
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
ssh "${PRODUCTION_USER}@ifarm" "cat > ~/\"${PRODUCTION_LAUNCH_DIR}\"/DEPLOYED_HD_UTILITIES_GIT_HASH" <<< "${GIT_HASH}"
# alternative: printf '%s' "$VAR" | ssh user@remote-host "cat > /path/to/file"

exit 0
