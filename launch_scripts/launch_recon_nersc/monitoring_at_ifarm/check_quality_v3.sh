#!/usr/bin/env bash

# Checks if expected output files exist for each raw data file.
# Writes list of good and bad files into separate text files.

#TODO also check whether ROOT files are actually readable?


RUN_PERIOD="${1}"  # run period, e.g. "2017-01"
VER="${2}"  # reconstruction version, e.g. "ver01"
PROD_ENV="${3}"  # production environment, e.g. "ver01-perl"

VOLATILE_ROOT_DIR="/volatile/halld/offsite_prod/RunPeriod-${RUN_PERIOD}/recon/${PROD_ENV}/${VER}"
VOLATILE_REST_DIR="${VOLATILE_ROOT_DIR}/REST"
VOLATILE_HISTS_DIR="${VOLATILE_ROOT_DIR}/hists"
# mkdir -p "${VOLATILE_HISTS_DIR}"
VOLATILE_FLUX_DIR="${VOLATILE_ROOT_DIR}/tree_PSFlux"

ls -1 "${VOLATILE_REST_DIR}" > "rp-${RUN_PERIOD}-files-to-copy-to-cache.txt"

RAW_DATA_DIR="/mss/halld/RunPeriod-${RUN_PERIOD}/rawdata"
RECON_DIR_MSS="/mss/halld/RunPeriod-${RUN_PERIOD}/recon/${VER}"
RECON_DIR_CACHE="/cache/halld/RunPeriod-${RUN_PERIOD}/recon/${VER}"
WORK_DIR="/work/halld/home/gxproj4/offsite_prod/RunPeriod-${RUN_PERIOD}"
VOLATILE_DIR="/volatile/halld/recon/RunPeriod-${RUN_PERIOD}/${VER}"

RUN_NUMBERS=($(cat "rp-${RUN_PERIOD}-files-to-copy-to-cache.txt"))

rm "good-list-${RUN_PERIOD}.txt"
touch "good-list-${RUN_PERIOD}.txt"
rm "bad-list-${RUN_PERIOD}.txt"
touch "bad-list-${RUN_PERIOD}.txt"

i1=0
i2=0
i3=0
i4=0
i5=0
i6=0
i7=0
for RUN_NUMBER in "${RUN_NUMBERS[@]}"
do
  echo "run number: ${RUN_NUMBER}"
  EVIO_FILES=($(ls "${RAW_DATA_DIR}/Run${RUN_NUMBER}/hd_rawdata_??????_???.evio"))
  i7=0
  # mkdir -p "${VOLATILE_HISTS_DIR}/${RUN_NUMBER}""
  for EVIO_FILE in "${EVIO_FILES[@]}"
  do
    read RUN_NUMBER < <(echo "${EVIO_FILE}" | grep -o '[0-9]\+' | tail -n 2)
    #TODO RUN_NUMBER from file name should be the same as RUN_NUMBER from list; check this and print warning if not
    read FILE_NUMBER < <(echo "${EVIO_FILE}" | grep -o '[0-9]\+' | tail -n 1)
    #echo "run number: ${RUN_NUMBER} - file number: ${FILE_NUMBER}"
    #echo "file: ${EVIO_FILE}"
    #TODO improve logic and code
    file1a="${RECON_DIR_CACHE}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm"
    file2a="${RECON_DIR_CACHE}/tree_PSFlux/${RUN_NUMBER}/tree_PSFlux_${RUN_NUMBER}_${FILE_NUMBER}.root"
    file3a="${RECON_DIR_CACHE}/hists/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root"
    file1b="${RECON_DIR_MSS}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm"
    file2b="${RECON_DIR_MSS}/tree_PSFlux/${RUN_NUMBER}/tree_PSFlux_${RUN_NUMBER}_${FILE_NUMBER}.root"
    file3b="${RECON_DIR_MSS}/hists/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root"
    #filehe="${WORK_DIR}/job_info_${RUN_NUMBER}_${FILE_NUMBER}/helicity.log"
    filear="${VOLATILE_DIR}/REST/${RUN_NUMBER}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm"
    fileaf="${VOLATILE_DIR}/tree_PSFlux/${RUN_NUMBER}/tree_PSFlux_${RUN_NUMBER}_${FILE_NUMBER}.root"
    fileah="${VOLATILE_DIR}/hists/${RUN_NUMBER}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root"
    if { [[ -f "${file1a}" ]] || [[ -f "${file1b}" ]]; } && \
       { [[ -f "${file2a}" ]] || [[ -f "${file2b}" ]]; } && \
       { [[ -f "${file3a}" ]] || [[ -f "${file3b}" ]]; }
    then
      #echo "${RUN_NUMBER} ${FILE_NUMBER}" >> "good-list-${RUN_PERIOD}.txt"
      ((i1++))
    else
      echo "${RUN_NUMBER} ${FILE_NUMBER}" >> "bad-list-${RUN_PERIOD}.txt"
      #echo "Missing outputs ${RUN_NUMBER} ${FILE_NUMBER}"
      ((i2++))
      ((i7++))
    fi
    #TODO also use -f below?
    if [[ -e "${file1a}" || -e "${file1b}" ]]
    then
      # if [[ -f "${filear}" ]]
      # then
      #   cp "${filear}" "${VOLATILE_REST_DIR}/${RUN_NUMBER}"
      # fi
      ((i3++))
    fi
    if [[ -e "${file2a}" || -e "${file2b}" ]]
    then
      # if [[ -f "${fileaf}" ]]
      # then
      #   cp "${fileaf}" "${VOLATILE_FLUX_DIR}/${RUN_NUMBER}"
      # fi
      ((i4++))
    fi
    if [[ -e "${file3a}" || -e "${file3b}" ]]
    then
      # if [[ -f "${filehe}" ]]
      # then
      #   cp "${filehe}" "${VOLATILE_HISTS_DIR}/${RUN_NUMBER}/helicity_${RUN_NUMBER}_${FILE_NUMBER}.log"
      # fi
      # if [[ -f "${fileah}" ]]
      # then
      #   cp "${fileah}" "${VOLATILE_HISTS_DIR}/${RUN_NUMBER}"
      # fi
      ((i5++))
    fi
    ((i6++))
  done
  if (( i7 == 0 ))
  then
    echo "${RUN_NUMBER} ${FILE_NUMBER}" >> "good-list-${RUN_PERIOD}.txt"
  fi
done
echo "Total number of correct files with REST, PSFlux, and hists: ${i1}"
echo "Total number of incorrect files: ${i2}"
echo "Total number of correct files with REST: ${i3}"
echo "Total number of correct files with PSFlux: ${i4}"
echo "Total number of correct files with hists: ${i5}"
echo "Total number of files: ${i6}"
