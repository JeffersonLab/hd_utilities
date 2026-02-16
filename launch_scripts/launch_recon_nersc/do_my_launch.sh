#!/usr/bin/env bash

# Master script that submits a reconstruction jobs to run at NERSC
# using swif2. One job is submitted for each run number in the given
# list file.

RUN_PERIOD="2025-01"  # Run period to process.
VER="03"  # Version of this reconstruction launch.
HALLD_VERSION_SET_XML="version_7.4.0.xml"  # XML file that defines the Hall-D version set to be used.
JANA_CONFIG="jana_recon_nersc.config"  # JANA config file to use; must be located in ${NERSC_LAUNCH_DIR}
BATCH="${VER}-perl"  # Batch label used to hold details of the launch; is appended to workflow name and directory names; should make it clear where/how the campaign was being run.
#
SWIF_MAX_CONCURRENT_JOBS=100  # Maximum number of swif2 jobs that can be in-flight at once. This can be set only once when the workflow is created. If jobs are submitted piecemeal by running this script multiple times specifying different run lists, only the first invocation that creates the workflow will set this parameter.
SWIF_RAW_DATA_ROOT="/mss/halld/RunPeriod-${RUN_PERIOD}/rawdata"  # Root of JLab directory tree, where raw data files are located.
SWIF_OUTPUT_ROOT="/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-${RUN_PERIOD}/recon/ver${BATCH}"  # Root of JLab directory tree, where output files are copied to.
SWIF_WORKFLOW="recon_${RUN_PERIOD}_ver${BATCH}_batchNERSC-multi"  # only change this if default name is not appropriate  #TODO fix name?
SWIF_SITE="nersc/perlmutter"  # swif2 site to use
#
NERSC_PROJECT="m3120"  # NERSC project to charge to.
NERSC_PROJECT_DIR="/global/cfs/cdirs/${NERSC_PROJECT}"  # Project directory in the NERSC Common File System (CFS) where config files and scripts will be copied to and run from.
NERSC_LAUNCH_DIR="${NERSC_PROJECT_DIR}/launch-${BATCH}"  # NERSC directory that will get mapped to /launch_${BATCH} inside the job container. Contains scripts and JANA config file to run the job.
NERSC_QOS="regular"  # NERSC queue to use; usually `regular` or `debug`. See NERSC documentation for details on charging and restrictions.
NERSC_NODE_TYPE="cpu"  # Constraint to use for NERSC job; usually `cpu` or `gpu`.
NERSC_MAX_WALL_TIME="5:00:00"  # Maximum wall time for NERSC job.
NERSC_MAX_TREADS_PER_NODE=256  # Maximum number of threads on a NERSC Perlmutter CPU node.
NERSC_NMB_TREADS_PER_JOB=32  # Number of threads to use per job; must be <= ${NERSC_MAX_TREADS_PER_NODE}.
NERSC_NMB_JOBS_PER_NODE=$(echo "${NERSC_MAX_TREADS_PER_NODE} / ${NERSC_NMB_TREADS_PER_JOB}" | bc)  # Number of jobs to run concurrently on a single NERSC Perlmutter CPU node.  #TODO works only if division is exact; need to round up if not exact
NERSC_HOST="perlmutter-p1.nersc.gov"  # NERSC hostname to use for ssh.
NERSC_CONTAINER_IMAGE="docker:jeffersonlab/gluex_almalinux_9:latest"  # Shifter image that was converted from Docker image. Is not pulled in automatically and needs to exist in Shifter registry.

# load run list into array
#TODO improve code
RUN_NUMBERS=( $( cat list-2025-01-ver03-perl.txt ) )


# prepare scripts and config files to be copied to NERSC
sed 's,BATCH,'${BATCH}',g' script_nersc_test.temp > script_nersc_test.sh  # set the name of the mount point in the container  #TODO it would be better to pass this down as an argument
sed 's,THREADNB,'${NERSC_NMB_TREADS_PER_JOB}',g' "./${JANA_CONFIG/.config/.temp}" > "./${JANA_CONFIG}"  # set number of threads in JANA config file  #TODO it would be better to pass this down as an argument to `hd_root`
chmod +x script_nersc_test.sh
echo "in if"  #TODO fix
scp -r ../launch-${BATCH} ${NERSC_HOST}:${NERSC_PROJECT_DIR}

# create and run swif2 workflow
#TODO check if workflow already exists and if so skip creation and just add jobs to it
swif2 create "${SWIF_WORKFLOW}" -site "${SWIF_SITE}" -maxconcurrent ${SWIF_MAX_CONCURRENT_JOBS}
swif2 run "${SWIF_WORKFLOW}"

# loop over run numbers and submit one swif2 job for each
for RUN_NUMBER in "${RUN_NUMBERS[@]}"
do
  # construct command to submit a swif2 job for the given run number
  SWIF2_CMD=(
    swif2 add-job
    -workflow "${SWIF_WORKFLOW}"
    -name "GLUEX_recon_${RUN_NUMBER}"  # swif2 job name
  )
  # loop over all evio files of the run and subdivide file list into
  # chunks of size ${NERSC_NMB_JOBS_PER_NODE} that will be processed
  # by individual NERSC nodes, defining the input and output files for
  # each node.
  EVIO_DIR="${SWIF_RAW_DATA_ROOT}/Run${RUN_NUMBER}"
  EVIO_FILE_PATHS=("${EVIO_DIR}"/*.evio)  #TODO this is not empty if there are no evio files, but contains the pattern itself
  # calculate number of nodes to request based on number of evio files and number of jobs to run per node
  NMB_EVIO_FILES=${#EVIO_FILE_PATHS[@]}
  echo "Run period: ${RUN_PERIOD} - run number: ${RUN_NUMBER} - number of evio files: ${NMB_EVIO_FILES} - divided by: ${NERSC_NMB_JOBS_PER_NODE}"
  NERSC_NMB_NODES=$(echo "(${NMB_EVIO_FILES} + ${NERSC_NMB_JOBS_PER_NODE} - 1) / ${NERSC_NMB_JOBS_PER_NODE}" | bc)  #TODO check whether this is generally correct
  echo "Number of nodes asked: ${NERSC_NMB_NODES}"
  for (( NERSC_NODE_INDEX=0; NERSC_NODE_INDEX < NERSC_NMB_NODES; NERSC_NODE_INDEX++ ))
  do
    # construct input lines for the given node, e.g. `-input file1.evio mss:/mss/some_path/file1.evio -input file2.evio mss:/mss/some_path/file2.evio ...`
    EVIO_FILE_START_INDEX=$((NERSC_NODE_INDEX * NERSC_NMB_JOBS_PER_NODE))
    EVIO_FILE_END_INDEX=$((EVIO_FILE_START_INDEX + NERSC_NMB_JOBS_PER_NODE))
    for (( EVIO_FILE_INDEX=EVIO_FILE_START_INDEX; EVIO_FILE_INDEX < EVIO_FILE_END_INDEX && EVIO_FILE_INDEX < NMB_EVIO_FILES; EVIO_FILE_INDEX++ ))
    do
      EVIO_FILE_PATH="${EVIO_FILE_PATHS[${EVIO_FILE_INDEX}]}"
      EVIO_FILE_NAME="$(basename "${EVIO_FILE_PATH}")"
      SWIF2_CMD+=(-input "${EVIO_FILE_NAME}" "mss:${EVIO_FILE_PATH}")
    done
    # construct output line for the given node, e.g. `-output match:RUN132194/FILE024/* /lustre/expphy/volatile/halld/offsite_prod/RunPeriod-2025-01/recon/ver03/RUN132194/FILE024/`
    RUNDIR=$(printf "RUN%06d/FILE%03d" "${RUN_NUMBER}" "${NERSC_NODE_INDEX}")  #TODO `FILE` is a misnomer; it should be something like `NODE` or `CHUNK`
    SWIF_OUTPUT_DIR="${SWIF_OUTPUT_ROOT}/${RUNDIR}"
    echo "mkdir -p ${SWIF_OUTPUT_DIR}"
    # mkdir -p ${SWIF_OUTPUT_DIR}
    SWIF2_CMD+=(-output "match:${RUNDIR}/*" "${SWIF_OUTPUT_DIR}")
  done
  # define NERSC job
  SWIF2_CMD+=(
    -sbatch
      # these options are passed to `sbatch` when swif2 submits job at NERSC
      -A "${NERSC_PROJECT}"  #TODO use --account=
      --volume=\""${NERSC_LAUNCH_DIR}:/launch-${BATCH}"\"
      --image="${NERSC_CONTAINER_IMAGE}"
      --module=cvmfs
      --time="${NERSC_MAX_WALL_TIME}"
      -N ${NERSC_NMB_NODES}  #TODO use --nodes=
      --tasks-per-node=1
      --cpus-per-task=${NERSC_MAX_TREADS_PER_NODE}
      --qos="${NERSC_QOS}"
      -C "${NERSC_NODE_TYPE}"  #TODO use --constraint=
      ::
      # job script to run at NERSC
      "${NERSC_LAUNCH_DIR}/script_nersc_multi_test.sh"  # wrapper script for script_nersc_multi_test.py
      # arguments passed to script_nersc_multi_test.py
      "${NERSC_LAUNCH_DIR}"                    # LAUNCH_DIR argument
      "/launch-${BATCH}/script_nersc_test.sh"  # SCRIPT_FILE argument
      "/launch-${BATCH}/${JANA_CONFIG}"        # JANA_CONFIG argument
      "${HALLD_VERSION_SET_XML}"               # HALLD_VERSION_SET_XML argument
      "${NERSC_NMB_JOBS_PER_NODE}"             # SLURM_JOBS_PER_NODE argument
  )
  echo "${SWIF2_CMD[@]}" >| "./exec_${RUN_NUMBER}.sh"
  # # generate shell-escaped version of command array and write it to file so it becomes a script that can be run directly
  #TODO this would be the safer approach, but in `-output match:RUN132194/FILE024/*` this would escape the `*`; not sure if this would cause problems with swif2
  # {
  #   printf '%q ' "${CMD[@]}"
  #   printf '\n'
  # } >| "exec_${RUN_NUMBER}.sh"
  chmod +x "./exec_${RUN_NUMBER}.sh"
  "./exec_${RUN_NUMBER}.sh"
done
