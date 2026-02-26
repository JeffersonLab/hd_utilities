#!/usr/bin/env bash
set -o nounset  # exit when trying to use an uninitialized variable
set -o errexit  # exit when any command fails
set -o pipefail  # exit if any command in a pipeline fails
#TODO make debug output switchable
set -o verbose  # print shell input lines as they are read, i.e. before any expansion
set -o xtrace  # print commands and their arguments as they are executed, i.e. after expansion and without I/O redirection

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

CONFIG_FILE="${1:-./do_my_launch.env}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
# shellcheck source=./do_my_launch.env
source "${CONFIG_FILE}"

# copy scripts and config files to NERSC
SRC="/home/${PRODUCTION_USER}/${PRODUCTION_LAUNCH_DIR}"
DEST="${NERSC_HOST}:${NERSC_LAUNCH_DIR}"
echo "Copying launch scripts and config files from '${SRC}' to '${DEST}'"
ssh "${NERSC_HOST}" "mkdir --parents '${NERSC_LAUNCH_DIR}' && chown :${NERSC_PROJECT} '${NERSC_LAUNCH_DIR}'"  # rsync cannot set permissions on the destination directory if it does not exist beforehand
RSYNC_CMD=(rsync
  --verbose
  --delete  # ensure pristine copy
  --archive
  --ignore-times
  --chown=:"${NERSC_PROJECT}"  # ensure write permissions for project group
  --chmod="Dg+rwx,Fg+rw"  # ensure write permissions for project group for subdirectories and files
  "${SRC}/"  # trailing slash is important: copy contents of `SRC` into existing `DEST` directory
  "${DEST}"
)
"${RSYNC_CMD[@]}"
unset SRC DEST

# create and run swif2 workflow
if swif2 status "${SWIF_WORKFLOW}" &> /dev/null
then
  echo "Workflow '${SWIF_WORKFLOW}' already exists; skipping creation"
else
  echo "Creating swif2 workflow '${SWIF_WORKFLOW}' at site '${SWIF_SITE}' with max concurrent jobs ${SWIF_MAX_CONCURRENT_JOBS}"
  swif2 create "${SWIF_WORKFLOW}" -site "${SWIF_SITE}" -maxconcurrent "${SWIF_MAX_CONCURRENT_JOBS}"
fi
swif2 run "${SWIF_WORKFLOW}"  #TODO is it really a good idea to run the workflow immediately?

# loop over run numbers and submit one swif2 job each
readarray -t RUN_NUMBERS < "${RUN_NUMBER_LIST_FILE}"  # read lines into array without trailing newlines
for RUN_NUMBER in "${RUN_NUMBERS[@]}"
do
  # construct command to submit a swif2 job for the given run number
  #NOTE swif2 jobs for remote sites do not produce stdout and stderr
  #  on JLab Farm and the `-stdout` and `-stderr` arguments would need
  #  to point to remote paths at NERSC.  Specifying the NERSC log
  #  files using the `-sbatch --output` argument is more flexible.
  SWIF2_CMD=(
    swif2 add-job
    -workflow "${SWIF_WORKFLOW}"
    -name "GLUEX_recon_${RUN_NUMBER}"  # swif2 job name
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
  NERSC_NMB_TASKS=$(echo "(${NMB_EVIO_FILES} + ${NERSC_NMB_PROCESSES_PER_TASK} - 1) / ${NERSC_NMB_PROCESSES_PER_TASK}" | bc)  #TODO check whether this is generally correct
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
      SWIF2_CMD+=(-input "${EVIO_FILE_NAME}" "mss:${EVIO_FILE_PATH}")
    done
    # construct the output line for the given task, e.g. `-output match:RUN132194/TASK024/* /lustre/expphy/volatile/halld/offsite_prod/RunPeriod-2025-01/recon/ver03/RUN132194/TASK024/`
    SUBDIR_TASK=$(printf "RUN%06d/TASK%03d" "${RUN_NUMBER}" "${TASK_INDEX}")  # subdirectory for NERSC task given by `${TASK_INDEX}`
    SWIF_OUTPUT_DIR_TASK="${SWIF_OUTPUT_ROOT}/${SUBDIR_TASK}"  # output directory for NERSC tasks  #TODO is it really required to create them on the JLab file system or would swif2 take care of this?
    echo "mkdir --parents ${SWIF_OUTPUT_DIR_TASK}"
    mkdir --parents "${SWIF_OUTPUT_DIR_TASK}"  #TODO are also created by `script_job.py`
    # SWIF2_CMD+=(-output "match:${SUBDIR_TASK}/*" "${SWIF_OUTPUT_ROOT}")  # copy `${SUBDIR_TASK}/*` into `${SWIF_OUTPUT_ROOT}` after the job is done
    SWIF2_CMD+=(-output "match:*" "${SWIF_OUTPUT_ROOT}")  # copy everything in swif job attempt directory to `${SWIF_OUTPUT_ROOT}` after the job is done
  done
  # define NERSC job
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
      --image="'${NERSC_CONTAINER_IMAGE}'"  #TODO verify that container image exists in NERSC repository
      --volume="'${NERSC_LAUNCH_DIR}:/${LAUNCH_DIR}'"  # map `${NERSC_LAUNCH_DIR}` on host to `/${LAUNCH_DIR}` in container
      --module=cvmfs  # enable CVMFS in the container so it can access the `/group/halld` tree
      --output="job.RUN${RUN_NUMBER}.%j.out"  # write stdout and stderr of job to file named `job.RUN<run number>.<job id>.out`, which will be copied by slurm into `${SLURM_SUBMIT_DIR}`  #TODO this is `/global/u1/j/jlab`; better location?
      ::
      # job script to run at NERSC
      "${NERSC_LAUNCH_DIR}/script_job.py"
        --run-number="${RUN_NUMBER}"
        --launch-dir="${LAUNCH_DIR}"
        --jana-config="/${LAUNCH_DIR}/${JANA_CONFIG}"
        --jana-calib-context="'${JANA_CALIB_CONTEXT}'"
        --jana-geometry-url="'${JANA_GEOMETRY_URL}'"
        --halld-version-set-xml="${HALLD_VERSION_SET_XML}"
        --nmb-processes-per-task="${NERSC_NMB_PROCESSES_PER_TASK}"
        --nmb-threads-per-process="${NERSC_NMB_THREADS_PER_PROCESS}"
  )
  echo "${SWIF2_CMD[@]}" >| "./exec_${RUN_NUMBER}.sh"
  # # generate shell-escaped version of command array and write it to file so it becomes a script that can be run directly
  #TODO this would be the safer approach, but in `-output match:RUN132194/TASK024/*` this would escape the `*`; not sure if this would cause problems with swif2
  # {
  #   printf '%q ' "${CMD[@]}"
  #   printf '\n'
  # } >| "exec_${RUN_NUMBER}.sh"
  chmod +x "./exec_${RUN_NUMBER}.sh"
  "./exec_${RUN_NUMBER}.sh"
done

# print status of workflow after submitting all jobs
# view jobs at https://scicomp.jlab.org/scicomp/swif/active
swif2 list
swif2 status "${SWIF_WORKFLOW}"
swif2 status "${SWIF_WORKFLOW}" -jobs
