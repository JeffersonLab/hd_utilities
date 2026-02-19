#!/usr/bin/env bash

# Master script that submits reconstruction jobs to run at NERSC using
# swif2. One swif2/NERSC job is submitted for each run number in the
# given list file.

# The computation is subdivided into the following levels:
# 1) `hd_root` with 32 threads processing one evio file = 1 NERSC process
# 2) 256/32 = 8 NERSC processes running concurrently on a NERSC node = 1 NERSC task -> 1 task per node
# 3) 1 run number with N evio files, processed by ceil(N/8) NERSC tasks (=nodes) running concurrently = 1 NERSC job -> ceil(N/8) tasks (nodes) per job
# sbatch: submits one job per run number
# srun: starts one task per node
# task script: runs 8 `hd_root` processes in parallel, each processing one evio file

CONFIG_FILE="${1:-./do_my_launch.config.sh}"  # configuration file that defines all variables used in this script
echo "Reading configuration of reconstruction launch from '${CONFIG_FILE}'"
# shellcheck source=./do_my_launch.config.sh
source "${CONFIG_FILE}"

# copy scripts and config files to NERSC
echo "Copying launch scripts and config files from '../launch-${BATCH}' to '${NERSC_HOST}:${NERSC_PROJECT_DIR}'"
rsync --archive --ignore-times --delete --verbose "../launch-${BATCH}" "${NERSC_HOST}:${NERSC_PROJECT_DIR}"  # ensure pristine copy

# create and run swif2 workflow
if swif2 status test_swif_workflow2 &> /dev/null
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
  SWIF2_CMD=(
    swif2 add-job
    -workflow "${SWIF_WORKFLOW}"
    -name "GLUEX_recon_${RUN_NUMBER}"  # swif2 job name
  )
  # loop over all evio files of the run and subdivide file list into
  # chunks of size `${NERSC_NMB_PROCESSES_PER_TASK}` that will be
  # processed by individual NERSC tasks, defining the input and output
  # files for each task.
  EVIO_DIR="${SWIF_RAW_DATA_ROOT}/Run${RUN_NUMBER}"
  shopt -s failglob  # exit with error if no files match the pattern
  EVIO_FILE_PATHS=("${EVIO_DIR}"/*.evio)
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
    echo "mkdir -p ${WORK_DIR_TASK}"
    mkdir -p "${WORK_DIR_TASK}"  #TODO are also created by `script_nersc_multi_test.py`
    SWIF2_CMD+=(-output "match:${SUBDIR_TASK}/*" "${SWIF_OUTPUT_ROOT}")  # copy `${SUBDIR_TASK}/*` into `${SWIF_OUTPUT_ROOT}` after the job is done
  done
  # define NERSC job
  SWIF2_CMD+=(
    -sbatch
      # these options are passed to `sbatch` when swif2 submits job at NERSC
      --account="${NERSC_PROJECT}"
      --volume="'${NERSC_LAUNCH_DIR}:/launch-${BATCH}'"  # map `${NERSC_LAUNCH_DIR}` on host to `/launch-${BATCH}` in container
      --image="'${NERSC_CONTAINER_IMAGE}'"  #TODO verify that image exists in NERSC repository
      --module=cvmfs  # enable CVMFS in the container so it can access the `/group/halld` tree
      --time="${NERSC_MAX_WALL_TIME}"
      --nodes="${NERSC_NMB_TASKS}"  # 1 node per task
      --tasks-per-node=1
      --ntasks="${NERSC_NMB_TASKS}"
      --cpus-per-task="${NERSC_MAX_THREADS_PER_TASK}"
      #--exclusive  # allocated nodes cannot be shared with other jobs/users
      --qos="${NERSC_QOS}"
      --constraint="${NERSC_NODE_TYPE}"
      --output="job-%x-%j.out"  # write stdout and stderr of job to file named `job-<job name>-<job id>.out` in working directory of job
      ::
      # job script to run at NERSC
      #TODO could we directly submit the python script?
      "${NERSC_LAUNCH_DIR}/script_nersc_multi_test.sh"  # wrapper script for `script_nersc_multi_test.py`
      # arguments passed to script_nersc_multi_test.py
      "${NERSC_LAUNCH_DIR}"                    # launch_dir argument
      "/launch-${BATCH}/script_nersc_test.sh"  # script_file_task argument
      "/launch-${BATCH}/${JANA_CONFIG}"        # jana_config argument
      "'${JANA_CALIB_CONTEXT}'"                # jana_calib_context argument
      "'${JANA_GEOMETRY_URL}'"                 # jana_geometry_url argument
      "${HALLD_VERSION_SET_XML}"               # halld_version_set_xml argument
      "${NERSC_NMB_PROCESSES_PER_TASK}"        # nmb_processes_per_node argument
      "${NERSC_NMB_THREADS_PER_PROCESS}"       # nmb_threads_per_process argument
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
