#!/usr/bin/env bash

# Checks status of reconstruction jobs at JLab Farm

RUN_PERIOD="${1}"  # e.g. `2025-01`
BATCH="${2}" # e.g. `ver03`
TASK_INDEX="${3}"
VER="${4}"

TASK_INDEX=$(printf "%03d" "${TASK_INDEX}")  # ensure TASK_INDEX is zero-padded to three digits, e.g. 7 -> 007

SWIF_WORKFLOW="recon_${RUN_PERIOD}_${BATCH}_NERSC-multi"
SWIF_OUTPUT_ROOT="/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-${RUN_PERIOD}/recon/${BATCH}"
SUBDIR_TASK_PATTERN="RUN??????/TASK${TASK_INDEX}"

swif2 status "${SWIF_WORKFLOW}"

echo "Number of runs processed"
ls -1 "${SWIF_OUTPUT_ROOT}"/${SUBDIR_TASK_PATTERN}/exitcode_0.txt | wc -l

echo "Number of runs not moved yet"
ls -1 "${SWIF_OUTPUT_ROOT}"/${SUBDIR_TASK_PATTERN}/dana*_000.hddm | wc -l

echo "Number of rawdata files coming back"
ls -1 "${SWIF_OUTPUT_ROOT}"/${SUBDIR_TASK_PATTERN}/hd_rawdata_??????_???.evio | wc -l
# rm "${SWIF_OUTPUT_ROOT}"${SUBDIR_TASK_PATTERN}/hd_rawdata_??????_???.evio

swif2 status "${SWIF_WORKFLOW}"

echo "Number of runs cooked:"
ls -1 "${SWIF_OUTPUT_ROOT}"/${SUBDIR_TASK_PATTERN}/dana*_000.hddm | wc -l

echo "Number of runs cooked with .tgz:"
ls -1 "${SWIF_OUTPUT_ROOT}"/${SUBDIR_TASK_PATTERN}/*_000.tgz | wc -l

echo "Number of files cooked with and without .tgz:"
ls -1 "${SWIF_OUTPUT_ROOT}"/${SUBDIR_TASK_PATTERN}/dana*.hddm | wc -l

echo "Number of runs cooked and cached:"
ls -1 "/cache/halld/RunPeriod-${RUN_PERIOD}/recon/ver${VER}/REST"/*/*_000.hddm | wc -l

echo "Number of runs cooked and on tape:"
ls -1 "/mss/halld/RunPeriod-${RUN_PERIOD}/recon/ver${VER}/REST"/*/*_000.hddm | wc -l

echo "Number of runs checked and ordered not moved yet:"
ls -1 "${SWIF_OUTPUT_ROOT}/ver${VER}/REST"/*/dana*_000.hddm | wc -l

echo "Number of runs checked and ordered"
ls -1 "${SWIF_OUTPUT_ROOT}/ver${VER}/REST" | wc -l

echo "Number of REST files checked and ordered:"
ls -1 "${SWIF_OUTPUT_ROOT}/ver${VER}/REST"/*/dana*_???.hddm | wc -l
