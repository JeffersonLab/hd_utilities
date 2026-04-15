#!/usr/bin/env bash

# retrieves information from batch systems of JLab Farm and NERSC in JSON format

THIS_SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"  # get the directory of this script
CONFIG_FILE="${1:-${THIS_SCRIPT_DIR}/launch.env}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
# shellcheck source=./launch.env
source "${CONFIG_FILE}"

echo "Using launch scripts from git commit hash: $(cat "${THIS_SCRIPT_DIR}/DEPLOYED_HD_UTILITIES_GIT_HASH" || true)"

# get swif2 information
SWIF_OUT_FILE="./${SWIF_WORKFLOW}.swif.json"
echo "Getting data for swif2 workflow '${SWIF_WORKFLOW}' and saving to file '${SWIF_OUT_FILE}'"
swif2 status "${SWIF_WORKFLOW}" -jobs -display json | jq . -M > "${SWIF_OUT_FILE}"

# get NERSC information
NERSC_OUT_FILE="./${SWIF_WORKFLOW}.nersc.json"
NERSC_OUT_FILE_REMOTE="temp/$(basename "${NERSC_OUT_FILE}")"
echo "Getting SLURM data from NERSC batch system and saving to file '${NERSC_OUT_FILE}'"
# generate output file at NERSC
#NOTE SLURM history at NERSC that is accessible via `sacct` goes back only 30 days
REMOTE_CMD="sacct \
  --json \
  --allocations \
  --qos='regular_0,regular_1' \
  --starttime='${PRODUCTION_START_DATE}' \
  --format=JobID,JobName,Submit,Start,End,ExitCode,State,CPUTime,CPUTimeRaw,Elapsed,ElapsedRaw,Timelimit,TimelimitRaw,AllocNodes,AllocCPU \
  | jq . -M > \"\${HOME}/${NERSC_OUT_FILE_REMOTE}\""
echo "Running remote command at NERSC: ${REMOTE_CMD}"
ssh "${NERSC_HOST}" "${REMOTE_CMD}"
echo "Moving output file from NERSC to JLab: ${NERSC_HOST}:${NERSC_OUT_FILE_REMOTE} -> ${NERSC_OUT_FILE}"
rsync --verbose --archive --remove-source-files "${NERSC_HOST}:${NERSC_OUT_FILE_REMOTE}" "${NERSC_OUT_FILE}"

exit 0
