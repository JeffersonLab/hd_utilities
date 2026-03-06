#!/usr/bin/env bash
# set -o nounset     # exit when trying to use an uninitialized variable; gxenv seems to require some variables to be unset, so we cannot use this option
set -o verbose       # print shell input lines as they are read, i.e. before any expansion
set -o xtrace        # print commands and their arguments as they are executed, i.e. after expansion and without I/O redirection
ulimit -c unlimited  # allow core dumps with no size limit
SECONDS=0  # shell variable that counts seconds since it was initialized; used to measure elapsed time of the script

# Slurm task script that runs an `hd_root` process for each EVIO file
# that it finds in its working directory.
#
# The task script runs inside a container launched by the job script
# `script_job.py`, which also prepares the task's working directory
# `RUNXXXXXX/TASKYYY`, where the YYY is the 3-digit slurm task ID.
# The task script wakes up in the job's working directory and goes
# into the task's working directory based on the value of
# `SLURM_PROCID`.  The output files produced by `hd_root` are written
# into subdirectories `FILEZZZ` of the task's working directory,
# defined by the 3-digit EVIO file number.  After all `hd_root`
# processes have completed, the script collects information about the
# task and the `hd_root` processes and prepares the output files for
# transfer back to JLab.  The script assumes that inside the container
# GlueX software is available in the usual `/group/halld/Software`
# location and sets up the environment using `gxenv`.  The output
# files are left in the working directory for swif2 to manage.

# Arguments:
#
# arg 1:  Run number for this task
# arg 2:  JANA config file
# arg 3:  JANA calibration context
# arg 4:  JANA geometry URL
# arg 5:  GlueX software version set XML file
# arg 6:  number of threads each `hd_root` process should use

set -o errexit  # exit when any command fails

echo "--- Get task script command-line arguments"
RUN_NUMBER="${1}"
JANA_CONFIG="${2}"
JANA_CALIB_CONTEXT="${3}"
JANA_GEOMETRY_URL_TASK="${4}"  #NOTE `${JANA_GEOMETRY_URL}` is reset by the `gxenv`call below
HALLD_VERSION_SET_XML="${5}"
NMB_THREADS_PER_PROCESS="${6}"

THIS_SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"  # get the directory of this script
echo "Using launch scripts from git commit hash: $(cat "${THIS_SCRIPT_DIR}/DEPLOYED_HD_UTILITIES_GIT_HASH" || true)"

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
echo "--- swif2 input directory: '${SWIF_INPUT_ROOT}':"
ls -l "${SWIF_INPUT_ROOT}"
WORK_DIR_JOB=$(pwd -P)
echo "--- Working directory of job: '${WORK_DIR_JOB}':"
ls -l "${WORK_DIR_JOB}"
WORK_DIR_RUN="${WORK_DIR_JOB}/$(printf "RUN%06d" "${RUN_NUMBER}")"
echo "--- Working directory of run: '${WORK_DIR_RUN}':"
ls -l "${WORK_DIR_RUN}"
echo "--- Construct the task's working directory and cd into it"
SUBDIR_TASK=$(printf "TASK%03d" "${SLURM_PROCID}")
WORK_DIR_TASK="${WORK_DIR_RUN}/${SUBDIR_TASK}"  # absolute path of working directory of container task, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/RUNXXXXXX/TASKYYY`, where `XXXXXX` is the run number and `YYY` is the 3-digit `${SLURM_PROCID}`
cd "${WORK_DIR_TASK}"
echo "--- Working directory of task: '$(pwd -P)':"
ls -lL .

echo "--- Make temporary local copy of CCDB and RCDB database files"
#NOTE this assumes that the task script has the whole node to itself
df /dev/shm
cp --verbose "/group/halld/www/halldweb/html/dist/ccdb.sqlite" /dev/shm
cp --verbose "/group/halld/www/halldweb/html/dist/rcdb.sqlite" /dev/shm
ls -l /dev/shm/{ccdb,rcdb}.sqlite
ls -l /dev/shm
# ensure files are removed when script exits or gets SIGINT or SIGTERM signal; also print wall time
trap 'rm --verbose --force /dev/shm/{ccdb,rcdb}.sqlite; echo "--- Wall time consumed by task script: $((SECONDS/60)) min, $((SECONDS%60)) sec"' EXIT INT TERM

echo "--- Set environment variables"
export JANA_CALIB_URL="sqlite:////dev/shm/ccdb.sqlite"
export CCDB_CONNECTION="${JANA_CALIB_URL}"
export RCDB_CONNECTION="sqlite:////dev/shm/rcdb.sqlite"
export JANA_GEOMETRY_URL="${JANA_GEOMETRY_URL_TASK}"
export JANA_RESOURCE_DIR="/group/halld/www/halldweb/html/resources"

echo "--- Log info about node and environment"
hostname >| ./node.hostname
declare -p | sed 's/^declare -[^ ]\+ //' >| ./node.env  # get alphabetically sorted list of environment variables without function definitions
findmnt --canonicalize --output=TARGET,FSTYPE,SIZE,USED,AVAIL,USE% >| ./node.mounts  # get list of mounted file systems with their sizes and usage
top -b -n 1 -c -w 512 >| ./node.top
cat /proc/cpuinfo >| ./node.cpuinfo

echo "--- Find EVIO files in current directory:"
shopt -s nullglob  # ensure that array is empty if no files match the pattern
evio_file_paths=("${WORK_DIR_TASK}"/hd_rawdata_??????_???.evio)
shopt -u nullglob
echo "Found ${#evio_file_paths[@]} EVIO files"

echo "--- Run hd_root process in parallel for each EVIO file in current directory"
#TODO evaluate using GNU parallel for this
# Do not exit immediately if `hd_root` fails to allow us to catch the
# exit code of each hd_root process and write it to a separate file.
# This is important since individual error codes are not captured by
# slurm.
set +o errexit
process_ids=()    # array to hold process IDs of background hd_root processes
rc_file_names=()  # array to hold file names where exit codes of hd_root processes are written
for evio_file_path in "${evio_file_paths[@]}"
do
  echo "--- Process EVIO file '${evio_file_path}':"
  ls -lL "${evio_file_path}"
  # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
  evio_file_name="$(basename "${evio_file_path}")"
  file_number="${evio_file_name:18:3}"  # extract 3-digit file number
  SUBDIR_PROCESS="FILE${file_number}"
  WORK_DIR_PROCESS="${WORK_DIR_TASK}/${SUBDIR_PROCESS}"
  mkdir --verbose --parents "${WORK_DIR_PROCESS}"
  cd "${WORK_DIR_PROCESS}"
  echo "--- Working directory of hd_root process: '$(pwd -P)'"
  # hd_root priorities for setting a parameter value is (lowest to highest):
  #   1) environment variable
  #   2) JANA config file
  #   3) command line argument
  HD_ROOT_CMD=(
    hd_root
    -PNTHREADS="${NMB_THREADS_PER_PROCESS}"  # override number of threads to use
    -Pjana:calib_context="${JANA_CALIB_CONTEXT}"  # override calibration context from local variable
    --loadconfigs "${JANA_CONFIG}"
    "${evio_file_path}"
  )
  echo "--- Run" "${HD_ROOT_CMD[@]}"
  # start hd_root process in background and redirect stdout and stderr to files
  "${HD_ROOT_CMD[@]}" 1> "hd_root.out" 2> "hd_root.err" &
  process_ids+=("${!}")  # capture the background process ID and store it in an array
  rc_file_names+=("${SUBDIR_PROCESS}/hd_root.rc")  # file path relative to `${WORK_DIR_TASK}` where exit code of this hd_root process will be written to
  echo "--- hd_root process with PID ${!} processes EVIO file '${evio_file_path}'"
done
cd "${WORK_DIR_TASK}"

echo "--- Wait for all background hd_root processes to complete and capture their exit codes"
max_exit_code=0  # variable to hold the maximum exit code among all hd_root processes; this will be the exit code of the task script, which is then forwarded to the job script
for process_index in "${!process_ids[@]}"
do
  pid="${process_ids[${process_index}]}"
  wait "${pid}"  # wait for the process to finish
  exit_code="${?}"  # capture the exit code of the process
  echo "hd_root process ${process_index} with PID ${pid} has exit code ${exit_code}" >| "${rc_file_names[${process_index}]}"
  if (( exit_code >= 128 ))
  then
    sig=$(( exit_code - 128 ))
    echo "hd_root process ${process_index} with PID ${pid} was terminated by signal ${sig}"
  fi
  if [[ "${exit_code}" -gt "${max_exit_code}" ]]  # determine the maximum exit code among all hd_root processes
  then
    max_exit_code="${exit_code}"
  fi
done
echo "--- Status of task's working directory '$(pwd -P)' after all hd_root processes have completed:"
ls -lR .

echo "--- Prepare output files for transfer back to JLab"
set -o errexit  # turn exit on error back on
shopt -s nullglob  # ensure that array is empty if no files match the pattern
evio_file_names=(hd_rawdata_??????_???.evio)
shopt -u nullglob
for evio_file_name in "${evio_file_names[@]}"
do
  #TODO avoid code duplication
  # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
  run_number="${evio_file_name:11:6}"  # extract 6-digit run number  #TODO check that it is identical to `${RUN_NUMBER}` passed as command line argument
  file_number="${evio_file_name:18:3}"  # extract 3-digit file number
  SUBDIR_PROCESS="FILE${file_number}"
  WORK_DIR_PROCESS="${WORK_DIR_TASK}/${SUBDIR_PROCESS}"
  cd "${WORK_DIR_PROCESS}"
  JOB_INFO_DIR="../job_info_${run_number}_${file_number}"
  echo "--- Move log files from process directory '$(pwd -P)' into subdirectory '${JOB_INFO_DIR}':"
  ls -lR .
  mkdir --verbose "${JOB_INFO_DIR}"
  mv --verbose hd_root.{out,err,rc} "${JOB_INFO_DIR}"  # move process log files
  cp --verbose ../node.{hostname,env,mounts,top,cpuinfo} "${JOB_INFO_DIR}"  # copy node log files; these files are identical for all processes of the task
  echo "--- Add '_${run_number}_${file_number}' suffix to all output file names if not already present"
  shopt -s nullglob  # ensure that array is empty if no files match the pattern
  files_names=(*.*)  # all files with an extension in the process working directory
  shopt -u nullglob
  for file_name in "${files_names[@]}"
  do
    if [[ ! "${file_name}" =~ .*_([0-9]{6})_([0-9]{3}).* ]]  # files that do not match the pattern `*_XXXXXX_YYY*` where X and Y are digits
    then
      new_file_name="${file_name/./_${run_number}_${file_number}.}"  # insert run and file number before the first '.' of the file extension
      mv --verbose "${file_name}" "${new_file_name}"
    else
      echo "File '${file_name}' already has run number and file number in its name; skipping renaming"
    fi
  done
done

cd "${WORK_DIR_TASK}"
echo "--- Status of task's working directory '$(pwd -P)' after rearranging output files:"
ls -lR .
echo "--- Move output files from all process directories to the task's working directory '$(pwd -P)'"
mv --verbose FILE???/*.* .

# swif2 will copy all files in ${WORK_DIR_TASK} back to JLab, so we
# have to clean up
echo "--- Clean up"
rm --verbose --force ./hd_rawdata_??????_???.evio  # links to input files
rm --verbose --force --recursive ./FILE???  # working directories of processes
rm --verbose --force ./node.{hostname,env,mounts,top,cpuinfo}  # node log files

echo "--- Status of task's working directory '$(pwd -P)' at end of task script:"
ls -lR .

# avoid returning exit codes reserved by shell
if (( max_exit_code >= 128 ))
then
  max_exit_code=$(( max_exit_code - 128 ))
fi
echo "--- Task script finished with maximum exit code ${max_exit_code} among all hd_root processes"
exit "${max_exit_code}"  # forward the maximum exit code among all `hd_root` processes to the job script  #TODO maybe it would be more useful to return number of failed processes?
