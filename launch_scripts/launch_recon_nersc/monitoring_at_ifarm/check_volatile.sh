rp="${1}"
ve="${2}"
nb="${3}"
ba="${4}"

SWIF_WORKFLOW="recon_${rp}_${ve}_batchNERSC-multi"
SWIF_OUTPUT_ROOT="/volatile/halld/offsite_prod/RunPeriod-${rp}/recon/${ve}"

swif2 status "${SWIF_WORKFLOW}"

echo "Number of runs processed"
ls -lrth "${SWIF_OUTPUT_ROOT}/RUN*/FILE${nb}/RUN*/FILE${nb}/exitcode_0.txt" | wc -l

echo "Number of runs not moved yet"
ls -lrth "${SWIF_OUTPUT_ROOT}/RUN*/FILE${nb}/RUN*/FILE${nb}/dana*_000.hddm"

echo "Number of rawdata files coming back"
ls -lrth "${SWIF_OUTPUT_ROOT}/RUN*/FILE${nb}/RUN*/FILE${nb}/hd_rawdata_??????_???.evio"
rm "${SWIF_OUTPUT_ROOT}/RUN??????/FILE???/RUN??????/FILE???/hd_rawdata_??????_???.evio"

swif2 status "${SWIF_WORKFLOW}"

echo "Number of runs cooked:"
ls -lrth "${SWIF_OUTPUT_ROOT}/RUN*/FILE${nb}/RUN*/FILE${nb}/dana*_000.hddm" | wc -l

echo "Number of runs cooked with .tgz:"
ls -lrth "${SWIF_OUTPUT_ROOT}/RUN*/FILE${nb}/RUN*/FILE${nb}/*_000.tgz" | wc -l

echo "Number of files cooked with and without .tgz:"
ls -lrth "${SWIF_OUTPUT_ROOT}/RUN*/FILE*/RUN*/FILE*/dana*.hddm" | wc -l

echo "Number of runs cooked and cached:"
ls "/cache/halld/RunPeriod-${rp}/recon/${ba}/REST/*/*_000.hddm" | wc -l

echo "Number of runs cooked and on tape:"
ls "/mss/halld/RunPeriod-${rp}/recon/${ba}/REST/*/*_000.hddm" | wc -l

echo "Number of runs checked and ordered not moved yet:"
ls -lrth "${SWIF_OUTPUT_ROOT}/${ba}/REST/*/dana*_000.hddm"

echo "Number of runs checked and ordered"
ls "${SWIF_OUTPUT_ROOT}/${ba}/REST/" | wc -l

echo "Number of REST files checked and ordered:"
ls "${SWIF_OUTPUT_ROOT}/${ba}/REST/*/dana*_???.hddm" | wc -l
