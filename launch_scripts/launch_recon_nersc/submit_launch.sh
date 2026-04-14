#!/usr/bin/env bash
set -o nounset   # exit when trying to use an uninitialized variable
set -o errexit   # exit when any command fails
set -o pipefail  # exit if any command in a pipeline fails
set -o verbose   # print shell input lines as they are read, i.e. before any expansion
set -o xtrace    # print commands and their arguments as they are executed, i.e. after expansion and without I/O redirection

# Master script that submits reconstruction jobs to run at NERSC using
# swif2.  One swif2/NERSC job is submitted for each run number in the
# given list file.

# The computation is subdivided into the following levels (lowest to highest):
# 1) `hd_root` with 32 threads processing one evio file = 1 NERSC process
# 2) 256/32 = 8 NERSC processes running concurrently on a NERSC node = 1 NERSC task -> 1 task per node
# 3) 1 run number with N evio files, processed by ceil(N/8) NERSC tasks (=nodes) running concurrently = 1 NERSC job -> ceil(N/8) tasks (nodes) per job
# task script: runs 8 `hd_root` processes in parallel, each processing one evio file
# srun: starts one task per node
# sbatch: submits one job per run number

THIS_SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"  # get the directory of this script
CONFIG_FILE="${1:-${THIS_SCRIPT_DIR}/launch.env}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
# shellcheck source=./launch.env
source "${CONFIG_FILE}"

echo "Using launch scripts from git commit hash: $(cat "${THIS_SCRIPT_DIR}/DEPLOYED_HD_UTILITIES_GIT_HASH" || true)"

# copy scripts and config files to NERSC
#TODO better separate JLab and NERSC parts of code and put everything needed at NERSC into separate directory
DEST="${NERSC_HOST}:${NERSC_LAUNCH_DIR}"
echo "Copying launch scripts and config files from '${THIS_SCRIPT_DIR}' to '${DEST}'"
ssh "${NERSC_HOST}" "mkdir --verbose --parents '${NERSC_LAUNCH_DIR}' && chown --verbose :${NERSC_PROJECT} '${NERSC_LAUNCH_DIR}'"  # rsync cannot set permissions on the destination directory if it does not exist beforehand
if [ "${?}" -ne 0 ]
then
  echo "Failed to create launch directory '${NERSC_LAUNCH_DIR}' at NERSC with group ownership '${NERSC_PROJECT}' and write permissions for the group; aborting"
  exit 1
else
  echo "Successfully created launch directory '${NERSC_LAUNCH_DIR}' at NERSC with group ownership '${NERSC_PROJECT}' and write permissions for the group"
fi
#TODO exclude __pycache__ directories; use `--exclude` option of rsync; better copy only the files needed at NERSC; e.g. use `--files-from` option of rsync with a generated list of files to copy; maybe create an extra deploy script for NERSC
RSYNC_CMD=(rsync
  --verbose
  --delete  # ensure pristine copy
  --archive
  --ignore-times
  --chown=:"${NERSC_PROJECT}"  # ensure write permissions for project group
  --chmod="Dg+rwx,Fg+rw"  # ensure write permissions for project group for subdirectories and files
  "${THIS_SCRIPT_DIR}/"  # trailing slash is important: copy contents of `THIS_SCRIPT_DIR` into existing `DEST` directory
  "${DEST}"
)
"${RSYNC_CMD[@]}"
unset DEST

# verify that container image exists in NERSC repository
if ssh "${NERSC_HOST}" "shifterimg lookup \"${NERSC_CONTAINER_IMAGE#docker:}\""
then
  echo "Container image '${NERSC_CONTAINER_IMAGE}' exists in NERSC repository"
else
  echo "Container image '${NERSC_CONTAINER_IMAGE}' does not exist in NERSC repository; aborting"
  exit 1
fi

# create and run swif2 workflow
if swif2 status "${SWIF_WORKFLOW}" &> /dev/null
then
  echo "Workflow '${SWIF_WORKFLOW}' already exists; skipping creation"
else
  echo "Creating swif2 workflow '${SWIF_WORKFLOW}' at site '${SWIF_SITE}' with max concurrent jobs ${SWIF_MAX_CONCURRENT_JOBS}"
  swif2 create "${SWIF_WORKFLOW}" -site "${SWIF_SITE}" -max-concurrent "${SWIF_MAX_CONCURRENT_JOBS}"
fi

# loop over run numbers and submit one swif2 job each
readarray -t RUN_NUMBERS < "${RUN_NUMBER_LIST_FILE}"  # read lines into array without trailing newlines
for RUN_NUMBER in "${RUN_NUMBERS[@]}"
do
  #TODO limit run umbers to [RUN_NUMBER_MIN, RUN_NUMBER_MAX] from config file
  # construct command to submit a swif2 job for the given run number
  #NOTE swif2 jobs for remote sites do not produce stdout and stderr
  #  on JLab Farm and the `-stdout` and `-stderr` arguments would need
  #  to point to remote paths at NERSC.  Specifying the NERSC log
  #  files using the `-sbatch --output` argument is more flexible.
  SWIF2_CMD=(
    swif2 add-job
    -workflow "${SWIF_WORKFLOW}"
    -name "GlueX_recon_${RUN_NUMBER}"  # swif2 job name
  )
  # loop over all evio files of the run and subdivide file list into
  # chunks of size `${NERSC_NMB_PROCESSES_PER_TASK}` that will be
  # processed by individual NERSC tasks, defining the input and output
  # files for each task.
  EVIO_RUN_DIR="${SWIF_RAW_DATA_ROOT}/Run${RUN_NUMBER}"
  shopt -s failglob  # exit with error if no files match the pattern
  EVIO_FILE_PATHS=("${EVIO_RUN_DIR}"/*.evio)
  shopt -u failglob
  # calculate number of tasks to request based on number of evio files and number of processes to run per task
  NMB_EVIO_FILES=${#EVIO_FILE_PATHS[@]}
  echo "Run period: ${RUN_PERIOD} - run number: ${RUN_NUMBER} - number of evio files: ${NMB_EVIO_FILES} - divided by: ${NERSC_NMB_PROCESSES_PER_TASK}"
  NERSC_NMB_TASKS=$(echo "(${NMB_EVIO_FILES} + ${NERSC_NMB_PROCESSES_PER_TASK} - 1) / ${NERSC_NMB_PROCESSES_PER_TASK}" | bc)
  echo "Number of tasks asked: ${NERSC_NMB_TASKS}"
  for (( TASK_INDEX=0; TASK_INDEX < NERSC_NMB_TASKS; TASK_INDEX++ ))
  do
    # construct the input lines for the given task, e.g. `-input file1.evio mss:/mss/some_path/file1.evio -input file2.evio mss:/mss/some_path/file2.evio ...`
    EVIO_FILE_START_INDEX=$((TASK_INDEX * NERSC_NMB_PROCESSES_PER_TASK))
    EVIO_FILE_END_INDEX=$((EVIO_FILE_START_INDEX + NERSC_NMB_PROCESSES_PER_TASK))
    for (( EVIO_FILE_INDEX=EVIO_FILE_START_INDEX; EVIO_FILE_INDEX < EVIO_FILE_END_INDEX && EVIO_FILE_INDEX < NMB_EVIO_FILES; EVIO_FILE_INDEX++ ))
    do
      EVIO_FILE_PATH="${EVIO_FILE_PATHS[${EVIO_FILE_INDEX}]}"
      EVIO_FILE_NAME="$(basename "${EVIO_FILE_PATH}")"
      SWIF2_CMD+=(-input "${EVIO_FILE_NAME}" "mss:${EVIO_FILE_PATH}")  # `mss:/` takes files directly from tape, bypassing JLab file systems; this is the most efficient way to transfer files to NERSC; `file:/` paths should be used for debugging only  #TODO switch automatically to `file:/` paths if files are located in cache
      # SWIF2_CMD+=(-input "${EVIO_FILE_NAME}" "file:${EVIO_FILE_PATH}")
    done
  done
  # the output files that swif2 should transfer back to JLab are defined inside the job script
  SWIF2_CMD+=(
    -sbatch
      # these options are passed to `sbatch` when swif2 submits job at NERSC
      --account="${NERSC_PROJECT}"
      --constraint="${NERSC_NODE_TYPE}"
      --qos="${NERSC_QOS}"
      --time="${NERSC_MAX_WALL_TIME}"
      --nodes="${NERSC_NMB_TASKS}"  # 1 node per task
      --ntasks-per-node=1
      --ntasks="${NERSC_NMB_TASKS}"
      --cpus-per-task="${NERSC_MAX_THREADS_PER_TASK}"
      #--exclusive  # allocated nodes cannot be shared with other jobs/users  #TODO clarify whether this is beneficial or not; swif2 also has `-exclusive` option that can be set when creating the workflow; is it redundant to set it in both places?
      --image="${NERSC_CONTAINER_IMAGE}"
      --volume="${NERSC_LAUNCH_DIR}:${NERSC_LAUNCH_DIR_CONTAINER}"  # map `${NERSC_LAUNCH_DIR}` on host to `${NERSC_LAUNCH_DIR_CONTAINER}` in container
      --module=cvmfs  # enable CVMFS in the container so it can access the `/group/halld` tree
      --output="job_${RUN_NUMBER}_%j.out"  # write stdout and stderr of job to file named `job_<run number>_<job id>.out` into working directory of job
      ::
      # job script to run at NERSC
      "${NERSC_LAUNCH_DIR}/script_job.py"
        --run_number="${RUN_NUMBER}"
        --launch_dir="${NERSC_LAUNCH_DIR_CONTAINER}"  # path of launch directory inside container
        --jana_config="${NERSC_LAUNCH_DIR_CONTAINER}/${JANA_CONFIG}"
        --jana_calib_context="${JANA_CALIB_CONTEXT}"
        --jana_geometry_url="${JANA_GEOMETRY_URL}"
        --halld_version_set_xml="${HALLD_VERSION_SET_XML}"
        --nmb_processes_per_task="${NERSC_NMB_PROCESSES_PER_TASK}"
        --nmb_threads_per_process="${NERSC_NMB_THREADS_PER_PROCESS}"
        --swif_output_root="${SWIF_OUTPUT_ROOT}"
  )
  #TODO check that SWIF2_CMD does not exceed size limits for command line arguments
  SUBMIT_JOB_SCRIPT="submit_job_for_RUN${RUN_NUMBER}.sh"
  # generate shell-escaped version of command array and write it to file so it becomes a script that can be run directly
  {
    printf '%q ' "${SWIF2_CMD[@]}"
    printf '\n'
  } >| "${SUBMIT_JOB_SCRIPT}"
  chmod +x "${SUBMIT_JOB_SCRIPT}"
  "./${SUBMIT_JOB_SCRIPT}"  # run the generated script to submit the job
done

# print status of workflow after submitting all jobs
# view jobs at https://scicomp.jlab.org/scicomp/swif/active
swif2 list
swif2 status "${SWIF_WORKFLOW}"
# swif2 status "${SWIF_WORKFLOW}" -jobs
