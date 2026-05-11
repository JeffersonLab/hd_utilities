#!/usr/bin/env bash

# Moves output of failed EVIO files from reconstruction jobs that ran
# at the JLab Farm to a separate directory for later inspection.


JOB_OUTPUT_BASEDIR=/lustre24/expphy/volatile/halld/recon/RunPeriod-2022-05/ver02

# loop over all `hd_root` log files and copy them to a separate directories
for HD_ROOT_LOG_FILE_PATH in "${JOB_OUTPUT_BASEDIR}"/log/*/stdout.*.out
do
  echo "Processing log file ${HD_ROOT_LOG_FILE_PATH}"
  HD_ROOT_LOG_FILE_NAME=$(basename "${HD_ROOT_LOG_FILE_PATH}")
  # extract the <run number>_<evio file number> part from the log file name, which is of the form stdout.<6-digit run number>_<3-digit evio file number>.out
  EVIO_FILE_LABEL="${HD_ROOT_LOG_FILE_NAME#stdout.}"
  EVIO_FILE_LABEL="${EVIO_FILE_LABEL%.out}"
  RUN_NUMBER="${EVIO_FILE_LABEL:0:6}"
  HD_ROOT_LOG_DIR_NAME=$(dirname "${HD_ROOT_LOG_FILE_PATH}")
  LOG_FILES=("${HD_ROOT_LOG_DIR_NAME}"/*"${EVIO_FILE_LABEL}"*)
  DEST_DIR="${JOB_OUTPUT_BASEDIR}/job_info/${RUN_NUMBER}/job_info_${EVIO_FILE_LABEL}"
  echo "Copying log files ${LOG_FILES[*]} to '${DEST_DIR}'"
  mkdir --verbose --parents "${DEST_DIR}"
  cp --verbose -- "${LOG_FILES[@]}" "${DEST_DIR}"
done

# create tarballs for all log-file directories created above
echo "-------------------------------------------------------------------------------"
for JOB_INFO_DIR_PATH in "${JOB_OUTPUT_BASEDIR}"/job_info/*/job_info_*
do
  echo "Creating tarball for '${JOB_INFO_DIR_PATH}'"
  tar --verbose --create --gzip --file="${JOB_INFO_DIR_PATH}.tgz" --directory="$(dirname "${JOB_INFO_DIR_PATH}")" "$(basename "${JOB_INFO_DIR_PATH}")"
done
