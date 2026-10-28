#!/usr/bin/env bash

#TODO move this to a Python script
# Moves output of failed EVIO files from reconstruction jobs that ran
# at the JLab Farm to a separate directory for later inspection.


JOB_OUTPUT_BASEDIR="/volatile/halld/recon/RunPeriod-2026-03/ver01"  #TODO make this a command-line argument
DESTINATION_BASEDIR="${JOB_OUTPUT_BASEDIR}.failed_evio_files_by_return_code"

# loop over all `hd_root` log files
for HD_ROOT_LOG_FILE_PATH in "${JOB_OUTPUT_BASEDIR}"/log/*/stdout.*.out
do
  echo "Processing log file ${HD_ROOT_LOG_FILE_PATH}"
  HD_ROOT_RC=$(awk -F'= ' '/Return Code =/{sub(/^[[:space:]]+/, "", $2); print $2}' "${HD_ROOT_LOG_FILE_PATH}")
  if [[ -n "${HD_ROOT_RC}" ]] && [[ "${HD_ROOT_RC}" -eq 0 ]]
  then
    echo "'${HD_ROOT_LOG_FILE_PATH}' SUCCEEDED with return code '${HD_ROOT_RC}'"
  else
    if [[ -z "${HD_ROOT_RC}" ]]
    then
      HD_ROOT_RC="missing"
    fi
    echo "'${HD_ROOT_LOG_FILE_PATH}' FAILED with return code '${HD_ROOT_RC}'"
    HD_ROOT_LOG_FILE_NAME=$(basename "${HD_ROOT_LOG_FILE_PATH}")
    # extract the <run number>_<evio file number> part from the log file name, which is of the form stdout.<6-digit run number>_<3-digit evio file number>.out
    EVIO_FILE_LABEL="${HD_ROOT_LOG_FILE_NAME#stdout.}"
    EVIO_FILE_LABEL="${EVIO_FILE_LABEL%.out}"
    DEST_PATH="${DESTINATION_BASEDIR}/${HD_ROOT_RC}/${EVIO_FILE_LABEL}"
    RUN_NUMBER="${EVIO_FILE_LABEL:0:6}"
    echo "Moving all output for EVIO file '${EVIO_FILE_LABEL}' to '${DEST_PATH}'"
    mkdir --verbose --parents "${DEST_PATH}"
    mv --verbose "${JOB_OUTPUT_BASEDIR}"/*/"${RUN_NUMBER}"/*"${EVIO_FILE_LABEL}"* "${DEST_PATH}"
  fi
done

echo "Deleting empty directories in '${JOB_OUTPUT_BASEDIR}/log'"
find "${JOB_OUTPUT_BASEDIR}/log" -type d -empty -delete
