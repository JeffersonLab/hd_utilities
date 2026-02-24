RUN_PERIOD="${1}"
BATCH="${2}"
TASK_INDEX="${3}"
VER="${4}"


SWIF_WORKFLOW="recon_${RUN_PERIOD}_ver${BATCH}_NERSC-multi"
SWIF_OUTPUT_ROOT="/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-${RUN_PERIOD}/recon/ver${BATCH}"
SUBDIR_TASK=$(printf "RUN??????/TASK%03d" "${TASK_INDEX}")

swif2 status "${SWIF_WORKFLOW}"

echo "Number of runs processed"
ls -1 "${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}/exitcode_0.txt" | wc -l

echo "Number of runs not moved yet"
ls -1 "${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}/dana*_000.hddm" | wc -l

echo "Number of rawdata files coming back"
ls -1 "${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}/hd_rawdata_??????_???.evio" | wc -l
rm "${SWIF_OUTPUT_ROOT}/RUN??????/TASK???/hd_rawdata_??????_???.evio"

swif2 status "${SWIF_WORKFLOW}"

echo "Number of runs cooked:"
ls -1 "${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}/dana*_000.hddm" | wc -l

echo "Number of runs cooked with .tgz:"
ls -1 "${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}/*_000.tgz" | wc -l

echo "Number of files cooked with and without .tgz:"
ls -1 "${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}/dana*.hddm" | wc -l

echo "Number of runs cooked and cached:"
ls -1 "/cache/halld/RunPeriod-${RUN_PERIOD}/recon/ver${VER}/REST/*/*_000.hddm" | wc -l

echo "Number of runs cooked and on tape:"
ls -1 "/mss/halld/RunPeriod-${RUN_PERIOD}/recon/ver${VER}/REST/*/*_000.hddm" | wc -l

echo "Number of runs checked and ordered not moved yet:"
ls -1 "${SWIF_OUTPUT_ROOT}/ver${VER}/REST/*/dana*_000.hddm" | wc -l

echo "Number of runs checked and ordered"
ls -1 "${SWIF_OUTPUT_ROOT}/ver${VER}/REST/" | wc -l

echo "Number of REST files checked and ordered:"
ls -1 "${SWIF_OUTPUT_ROOT}/ver${VER}/REST/*/dana*_???.hddm" | wc -l
