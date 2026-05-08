#!/usr/bin/env bash

# Moves output of failed EVIO files from reconstruction jobs that ran
# at the JLab Farm to a separate directory for later inspection.


JOB_OUTPUT_BASEDIR=/lustre24/expphy/volatile/halld/recon/RunPeriod-2022-05/ver02
DESTINATION_BASEDIR=/lustre24/expphy/volatile/halld/recon/RunPeriod-2022-05/ver02.failed_evio_files_by_hd_root_return_code

# loop over all `hd_root` output files
for HD_ROOT_OUT_FILE_PATH in "${JOB_OUTPUT_BASEDIR}"/log/*/stdout.*.out
do
  echo "Processing log file ${HD_ROOT_OUT_FILE_PATH}"
  HD_ROOT_RC=$(awk -F'= ' '/Return Code =/{sub(/^[[:space:]]+/, "", $2); print $2}' "${HD_ROOT_OUT_FILE_PATH}")
  if [ -n "$HD_ROOT_RC" ] && [ "$HD_ROOT_RC" -gt 0 ]
  then
    echo "'${HD_ROOT_OUT_FILE_PATH}' FAILED with return code '${HD_ROOT_RC}'"
    HD_ROOT_OUT_FILE_NAME=$(basename "${HD_ROOT_OUT_FILE_PATH}")
    # extract the <run number>_<evio file number> part from the output file name, which is of the form stdout.<6-digit run number>_<3-digit evio file number>.out
    EVIO_FILE_LABEL="${HD_ROOT_OUT_FILE_NAME#stdout.}"
    EVIO_FILE_LABEL="${EVIO_FILE_LABEL%.out}"
    DEST_PATH="${DESTINATION_BASEDIR}/${HD_ROOT_RC}/${EVIO_FILE_LABEL}"
    RUN_NUMBER="${EVIO_FILE_LABEL:0:6}"
    echo "Moving all output for EVIO file '${EVIO_FILE_LABEL}' to '${DEST_PATH}'"
    mkdir --verbose --parents "${DEST_PATH}"
    mv --verbose "${JOB_OUTPUT_BASEDIR}"/*/"${RUN_NUMBER}"/*"${EVIO_FILE_LABEL}"* "${DEST_PATH}"
  fi
done
