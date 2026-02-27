#!/usr/bin/env bash
# set -o nounset  # exit when trying to use an uninitialized variable; gxenv seems to require some variables to be unset, so we cannot use this option
#TODO make debug output switchable with a command line argument
set -o verbose  # print shell input lines as they are read, i.e. before any expansion
set -o xtrace  # print commands and their arguments as they are executed, i.e. after expansion and without I/O redirection
ulimit -c unlimited  # allow core dumps with no size limit

# Slurm task script that runs an `hd_root` process for each EVIO file
# that it finds in its working directory.
#
# The task script runs inside a container launched by the job script
# `script_job.py`, which also prepares the task's working directory
# `RUNXXXXXX/TASKYYY`, where the YYY is the 3-digit slurm task ID.
# The task script wakes up in the job's working directory and goes
# into the task's working directory based on the value of
# `SLURM_PROCID`.  The `hd-root` output files are written into
# subdirectories of the task's working directory, defined by run
# number and EVIO file index.  After all `hd_root` processes have
# completed, the script collects information about the task and the
# `hd_root` processes and writes it to files that are then tarred up for
# transfer back to JLab.  The script assumes that inside the container
# GlueX software is available in the usual `/group/halld/Software`
# location and sets up the environment using `gxenv`.  The output
# files are left in the working directory for swif2 to manage.

# Arguments:
#
# arg 1:  path of working directory RUNXXXXXX for run number XXXXXX
# arg 2:  JANA config file
# arg 3:  JANA calibration context
# arg 4:  JANA geometry URL
# arg 5:  GlueX software version set XML file
# arg 6:  number of threads each `hd_root` process should use

set -o errexit  # exit when any command fails

echo "--- Get task script command-line arguments"
WORK_DIR_RUN="${1}"  #TODO maybe it would be better to pass the run number?
JANA_CONFIG="${2}"
JANA_CALIB_CONTEXT="${3}"
JANA_GEOMETRY_URL="${4}"
HALLD_VERSION_SET_XML="${5}"
NMB_THREADS_PER_PROCESS="${6}"

echo "--- Setup GlueX software environment according to version set defined by XML file '${HALLD_VERSION_SET_XML}'"
source "/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh"
gxenv "${HALLD_VERSIONS}/${HALLD_VERSION_SET_XML}"
echo "--- Use halld_recon at '${HALLD_RECON_HOME}'"

# If the binaries in the group disk need to be replaced, then they can
# be placed in a directory on $SCRATCH and we'll use the HALLD_MY
# mechanism to supersede them. This was needed for offmon 2019-11 ver
# 13 when certain plugins had to be recompiled with a patch.
# export HALLD_MY="${CSCRATCH}/HALLD_MY/${HALLD_RECON_VERSION}"  #TODO this seems to be dysfunctional; ${CSCRATCH} does not exist anymore on NERSC
# export PATH="${HALLD_MY}/${BMS_OSNAME}/bin:${PATH}"

SWIF_INPUT_ROOT="/pscratch/sd/j/jlab/swif/input"
echo "--- swif2 input directory at '${SWIF_INPUT_ROOT}':"
# ls -lh "${SWIF_INPUT_ROOT}"
WORK_DIR_JOB=$(pwd -P)
echo "--- Working directory of job at '${WORK_DIR_JOB}':"
ls -lh "${WORK_DIR_JOB}"
echo "--- Working directory of run at '${WORK_DIR_RUN}':"
ls -lh "${WORK_DIR_RUN}"
echo "--- Construct the task's working directory and cd into it"
SUBDIR_TASK=$(printf "TASK%03d" "${SLURM_PROCID}")
WORK_DIR_TASK="${WORK_DIR_RUN}/${SUBDIR_TASK}"  # absolute path of working directory of container task, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/RUNXXXXXX/TASKYYY`, where `XXXXXX` is the run number and `YYY` is the 3-digit `${SLURM_PROCID}`
cd "${WORK_DIR_TASK}"
echo "--- Working directory of task at '$(pwd -P)':"
ls -lLh .

echo "--- Make temporary local copy of CCDB and RCDB database files"
#NOTE this assumes that the task script has the whole node to itself
cp --verbose "/group/halld/www/halldweb/html/dist/ccdb.sqlite" /dev/shm
cp --verbose "/group/halld/www/halldweb/html/dist/rcdb.sqlite" /dev/shm
ls -lh /dev/shm/{ccdb,rcdb}.sqlite
ls -lh /dev/shm
trap 'rm --verbose --force /dev/shm/{ccdb,rcdb}.sqlite' EXIT INT TERM  # ensure files are removed when script exits or gets SIGINT or SIGTERM signal

echo "--- Set environment variables"
export JANA_CALIB_URL="sqlite:////dev/shm/ccdb.sqlite"
export CCDB_CONNECTION="${JANA_CALIB_URL}"
export RCDB_CONNECTION="sqlite:////dev/shm/rcdb.sqlite"
export JANA_GEOMETRY_URL  # from command line argument
export JANA_RESOURCE_DIR="/group/halld/www/halldweb/html/resources"

echo "--- Log info about node and environment"
hostname >| ./hostname.log
declare -p | sed 's/^declare -[^ ]\+ //' >| ./env.log  # get alphabetically sorted list of environment variables without function definitions
top -b -n 1 -c -w 512 >| ./top.log
cat /proc/cpuinfo >| ./cpuinfo.log

echo "--- Find EVIO files in current directory:"
shopt -s nullglob  # ensure that array is empty if no files match the pattern
evio_files=(hd_rawdata_??????_???.evio)  #TODO use absolute paths
shopt -u nullglob
process_ids=()  # array to hold process IDs of background hd_root processes
for evio_file in "${evio_files[@]}"
do
  # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
  run_number="${evio_file:11:6}"
  file_number="${evio_file:18:3}"
  echo "${run_number} ${file_number}"
  mkdir --parents "${WORK_DIR_TASK}/run-${run_number}-${file_number}"
  cd "${WORK_DIR_TASK}/run-${run_number}-${file_number}"
  # hd_root priorities for setting a parameter value is (lowest to highest):
  #   1) environment variable
  #   2) JANA config file
  #   3) command line argument
  echo "${PWD}" >> ../myverif.out
  ls -lLh "../${evio_file}" >> ../myverif.out
  HD_ROOT_CMD=(
    hd_root
    -PNTHREADS="${NMB_THREADS_PER_PROCESS}"  # override number of threads to use
    -Pjana:calib_context="${JANA_CALIB_CONTEXT}"  # override calibration context from local variable
    --loadconfigs "${JANA_CONFIG}"
    "../${evio_file}"
  )
  echo "${HD_ROOT_CMD[@]}" >> ../myverif.out
  # start hd_root process in background and redirect stdout and stderr to files
  "${HD_ROOT_CMD[@]}" 2> "hd_root_${run_number}_${file_number}.err" 1> "hd_root_${run_number}_${file_number}.out" &
  process_ids+=("${!}")  # capture the background process ID and store it in an array
done
cd "${WORK_DIR_TASK}"

# wait for all background jobs to complete and capture their exit codes
#TODO evaluate using GNU parallel for this
echo "I am here 1"
for process_index in "${!process_ids[@]}"
do
  wait "${process_ids[${process_index}]}"  # wait for the process to finish
  echo "Exit code for process ${process_index}: ${?}" > "exitcode_${process_index}.txt"  #TODO the text does not add anything useful? why not just write the exit code to the file? Improve file name
  #TODO the exit code should be written ino the `run-${run_number}-${file_number}` directory
done

set -o errexit  # turn exit on error back on
echo "I am here 2"
ls -lh >> myverif.out  #TODO is time ordering useful here?
ls -lh ./*/* >> myverif.out

shopt -s nullglob  # ensure that array is empty if no files match the pattern
evio_files=(hd_rawdata_??????_???.evio)
shopt -u nullglob
for evio_file in "${evio_files[@]}"
do
  # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
  run_number="${evio_file:11:6}"
  file_number="${evio_file:18:3}"
  cd "${WORK_DIR_TASK}/run-${run_number}-${file_number}"
  echo "${run_number} ${file_number}"
  echo "${PWD}"
  ls -lh >> ../myverif.out
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
  mv "hd_root_${run_number}_${file_number}.err" "${JOB_INFO_DIR}"
  mv "hd_root_${run_number}_${file_number}.out" "${JOB_INFO_DIR}"
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
cd "${WORK_DIR_TASK}"

echo "I am here 3"
ls ./* >| my-second-verif.txt
ls ./*/* >> my-second-verif.txt
mv run-*/*.* .

# The swif2 job will copy all files in ${WORK_DIR_TASK} back to JLab
# so we have to clean up
echo "I am here 4"
# remove ccdb.sqlite and rcdb.sqlite files
rm -f hd_rawdata_??????_???.evio  # remove link to input file.
#rm -rf run-*
