#!/usr/bin/env bash
# set -o nounset  # exit if trying to use an uninitialized variable
#TODO make debug output switchable with a command line argument
set -o verbose  # print shell input lines as they are read, i.e. before any expansion
set -o xtrace  # print commands and their arguments as they are executed, i.e. after expansion and without I/O redirection
ulimit -c unlimited  # allow core dumps with no size limit

# This should be placed in the "launch" directory in the NERSC
# project directory used for the job. e.g.
#
#     /global/project/projectdirs/m3120/launch/script_nersc.sh
#
# This script will wake up in the shifter container in the job
# directory created by swif2. GlueX software is available via CVMFS.
#
# 3 arguments are passed and the entire directory will be processed. Output
# files are left in the working directory for swif2 to manage.
#
# Arguments:
#
# arg 1:  JANA config file
# arg 2:  JANA calibration context
# arg 3:  JANA geometry URL
# arg 4:  Hall-D version set XML file
# arg 5:  Number of threads to use for this job

# mount /var/udiMount/  #TODO is this really needed?
# mount /var/udiMount/launch-BATCH  #TODO is this really needed?
docker inspect jeffersonlab/gluex_almalinux_9:latest  #TODO shouldn't the image be a parameter passed to the script?

# exit when any command fails
set -o errexit

# # keep track of last executed command
# trap 'last_command=${current_command}; current_command=${BASH_COMMAND}' DEBUG
# # echo error message before exiting if error occurs
# trap 'echo "\"${last_command}\" command failed with exit code ${?}."' EXIT  #TODO is this is executed also for non-error exit? If so, it should be modified to only print the message if the exit code is non-zero.

# get command line arguments
JANA_CONFIG="${1}"
JANA_CALIB_CONTEXT="${2}"
JANA_GEOMETRY_URL="${3}"
HALLD_VERSION_SET_XML="${4}"
NMB_TREADS_PER_PROCESS="${5}"

EXTRA_ARGS=""
work_dir_task="${PWD}"  # absolute path of working directory of container task, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/subjob????`, where `????` is the 4-digit `${SLURM_NODEID}`

# setup software environment according to Hall-D version set XML file
source "/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh"
gxenv "${HALLD_VERSIONS}/${HALLD_VERSION_SET_XML}"

# If the binaries in the group disk need to be replaced, then they can
# be placed in a directory on $SCRATCH and we'll use the HALLD_MY
# mechanism to supersede them. This was needed for offmon 2019-11 ver
# 13 when certain plugins had to be recompiled with a patch.
# export HALLD_MY="${CSCRATCH}/HALLD_MY/${HALLD_RECON_VERSION}"  #TODO this seems to be dysfunctional; ${CSCRATCH} does not exist anymore on NERSC
# export PATH="${HALLD_MY}/${BMS_OSNAME}/bin:${PATH}"

# Use CCDB and RCDB from CVMFS. Make a temporary local copy so that we
# don't interfere with other jobs locking the same file
cp --verbose "/group/halld/www/halldweb/html/dist/ccdb.sqlite" /dev/shm
cp --verbose "/group/halld/www/halldweb/html/dist/rcdb.sqlite" /dev/shm
ls -lrth /dev/shm/ccdb.sqlite
ls -lrth /dev/shm/rcdb.sqlite
export JANA_CALIB_URL="sqlite:////dev/shm/ccdb.sqlite"
export CCDB_CONNECTION="${JANA_CALIB_URL}"
export RCDB_CONNECTION="sqlite:////dev/shm/rcdb.sqlite"
export JANA_GEOMETRY_URL
export JANA_RESOURCE_DIR="/group/halld/www/halldweb/html/resources"

#TODO cleanup and improve log files and console output
# record some info about the node and environment
ls -lrth >| myverif.out
top -b -n 1 >| top.out
cat /proc/cpuinfo >| cpuinfo.out
env >| env.out
hostname >| hostname.out
ls -lrth >> myverif.out

# Do not exit immediately if hd_root fails. This allows us to
# catch the exit code and write it to a file. This is important
# for multi-file jobs since individual error codes are not captured
# by slurm.
set +o errexit

# run hd_root process for each evio file in the directory in parallel
ls -lrth "${JANA_CONFIG}" >> myverif.out
cat "${JANA_CONFIG}" >> myverif.out
echo "${PWD}" >> myverif.out
echo "${HALLD_RECON_HOME}" >> myverif.out
echo "I am here 0"
shopt -s nullglob  # ensure that array is empty if no files match the pattern
evio_files=(hd_rawdata_??????_???.evio)
shopt -u nullglob
process_ids=()  # array to hold process IDs of background hd_root processes
for evio_file in "${evio_files[@]}"
do
  # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
  run_number="${evio_file:11:6}"
  file_number="${evio_file:18:3}"
  echo "${run_number} ${file_number}"
  mkdir -p "${work_dir_task}/run-${run_number}-${file_number}"
  cd "${work_dir_task}/run-${run_number}-${file_number}"
  # hd_root priorities for setting a parameter value is (lowest to highest):
  #   1) environment variable
  #   2) JANA config file
  #   3) command line argument
  HD_ROOT_CMD=(
    hd_root
    -PNTHREADS="${NMB_TREADS_PER_PROCESS}"  # override number of threads to use
    -Pjana:calib_context="${JANA_CALIB_CONTEXT}"  # override calibration context from local variable
    --loadconfigs "${JANA_CONFIG}"
    "${EXTRA_ARGS}"
    "../${evio_file}"
  )
  echo "${HD_ROOT_CMD[@]}" >> ../myverif.out
  # start hd_root process in background and redirect stdout and stderr to files
  "${HD_ROOT_CMD[@]}" 2> "std_${run_number}_${file_number}.err" 1> "std_${run_number}_${file_number}.out" &
  process_ids+=("${!}")  # capture the background process ID and store it in an array
done
cd "${work_dir_task}"

# wait for all background jobs to complete and capture their exit codes
echo "I am here 1"
for process_index in "${!process_ids[@]}"
do
  wait "${process_ids[${process_index}]}"  # wait for the process to finish
  echo "Exit code for process ${process_index}: ${?}" > "exitcode_${process_index}.txt"  #TODO the text does not add anything useful? why not just write the exit code to the file?
done

set -o errexit # turn on exit on error back
echo "I am here 2"
ls -lrth >> myverif.out
ls -lrth ./*/* >> myverif.out

shopt -s nullglob  # ensure that array is empty if no files match the pattern
evio_files=(hd_rawdata_??????_???.evio)
shopt -u nullglob
for evio_file in "${evio_files[@]}"
do
  # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
  run_number="${evio_file:11:6}"
  file_number="${evio_file:18:3}"
  cd "${work_dir_task}/run-${run_number}-${file_number}"
  echo "${run_number} ${file_number}"
  echo "${PWD}"
  ls -lrth >> ../myverif.out
  # move small files into a directory and make a tarball
  JOB_INFO_DIR="job_info_${run_number}_${file_number}"
  echo "${JOB_INFO_DIR}"
  mkdir "${JOB_INFO_DIR}"
  cp ../top.out "${JOB_INFO_DIR}"
  cp ../cpuinfo.out "${JOB_INFO_DIR}"
  cp ../env.out "${JOB_INFO_DIR}"
  cp ../hostname.out "${JOB_INFO_DIR}"
  cp ../exitcode_*.txt  "${JOB_INFO_DIR}/"
  cp ../myverif.out "${JOB_INFO_DIR}"
  mv "std_${run_number}_${file_number}.err" "${JOB_INFO_DIR}"
  mv "std_${run_number}_${file_number}.out" "${JOB_INFO_DIR}"
  #TODO also fetch job and task log files
  tar czf "${JOB_INFO_DIR}.tgz" "${JOB_INFO_DIR}"
  shopt -s nullglob  # ensure that array is empty if no files match the pattern
  files_tab=(*.*)
  shopt -u nullglob
  for file in "${files_tab[@]}"
  do
    echo "${file}"
    if [[ ! "${file}" =~ .*_([0-9]{6})_([0-9]{3}).* ]]  # files that do not match the pattern `*_XXXXXX_YYY*` where X and Y are digits
    then
      basefile=$(basename "${file}")  # extract base file name
      extension="${basefile##*.}"  # extract the file extension
      new_name="${basefile%.*}_${run_number}_${file_number}.${extension}"  # new file name with run_number and file_number inserted before the extension
      mv "${file}" "${new_name}"  # rename the file
    fi
  done
done
cd "${work_dir_task}"

echo "I am here 3"
ls ./* >| my-second-verif.txt
ls ./*/* >> my-second-verif.txt
mv run-*/*.* .

# The swif2 job will copy all files in ${work_dir_task} back to JLab
# so we have to clean up
echo "I am here 4"
# remove ccdb.sqlite and rcdb.sqlite files
rm -f /dev/shm/ccdb.sqlite
rm -f /dev/shm/rcdb.sqlite
rm -f hd_rawdata_??????_???.evio  # remove link to input file.
#TODO this clean up is largely incomplete
#rm -rf run-*
