#!/usr/bin/env bash

# Master script that submits a reconstruction jobs to run at NERSC
# using swif2. One job is submitted for each run number in the given
# list file.

RUN_PERIOD="2025-01"
VER="03"  # Version of this reconstruction launch
RECON_VERSION="halld_recon/halld_recon-5.9.0"  # Version of reconstruction code to be used. The software environment is constructed from a CVMFS directory that mirrors the directory /group/halld/Software/builds/Linux_CentOS7-x86_64-gcc4.8.5-cntr. The given value should be a directory relative to that.  #TODO fix dir
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
NERSC_NMB_TREADS_PER_JOB=32  # Number of threads to use per job; must be <= ${NERSC_NMB_TREADS_PER_NODE}.
NERSC_NMB_TREADS_PER_NODE=256  # Maximum number of threads on a NERSC Perlmutter CPU node.
NERSC_NMB_JOBS_PER_NODE=$(echo "${NERSC_NMB_TREADS_PER_NODE} / ${NERSC_NMB_TREADS_PER_JOB}" | bc)  # Number of jobs to run concurrently on a single NERSC Perlmutter CPU node.  #TODO works only if division is exact; need to round up if not exact. Fix naming "jobs" is incorrect, since these are not swif2 jobs but rather processes in the same job
NERSC_HOST="perlmutter-p1.nersc.gov"  # NERSC hostname to use for ssh.
NERSC_IMAGE="docker:jeffersonlab/gluex_almalinux_9:latest"  # Shifter image that was converted from Docker image. Is not pulled in automatically and needs to exist in Shifter registry.

RUN_NUMBERS=( $( cat list-2025-01-ver03-perl.txt ) )


# TESTMODE
# VERBOSE
# LAUNCHTYPE
# NAME
# TIMELIMIT
# NODETYPE
# SCRIPTFILE


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
  EVIO_DIR="${SWIF_RAW_DATA_ROOT}/Run${RUN_NUMBER}"
  EVIO_FILE_PATHS=("${EVIO_DIR}"/*.evio)  #TODO this is not empty if there are no evio files, but contains the pattern itself
  EVIO_FILE_NAMES=($(basename --multiple "${EVIO_DIR}"/*.evio))
  # create a input lines for `swif2 add-job` command, e.g. `-input file1.evion mss:/mss/halld/RunPeriod-2025-01/rawdata/Run12345/file1.evion -input file2.evion mss:/mss/halld/RunPeriod-2025-01/rawdata/Run12345/file2.evion ...`
  files=()
  for i in "${!EVIO_FILE_PATHS[@]}"
  do
    files+=("-input ${EVIO_FILE_NAMES[i]} mss:${EVIO_FILE_PATHS[i]}")
  done
  NMB_EVIO_FILES=${#EVIO_FILE_PATHS[@]}
  echo "Run period: ${RUN_PERIOD} - run number: ${RUN_NUMBER} - number of evio files: ${NMB_EVIO_FILES} - divided by: ${NERSC_NMB_JOBS_PER_NODE}"
  #NERSC_NMB_NODES=$(echo "${NMB_EVIO_FILES} / ${NERSC_NMB_JOBS_PER_NODE}" | bc)
  NERSC_NMB_NODES=$(echo "(${NMB_EVIO_FILES} + ${NERSC_NMB_JOBS_PER_NODE} - 1) / ${NERSC_NMB_JOBS_PER_NODE}" | bc)
  echo "Number of nodes asked: ${NERSC_NMB_NODES}"
  SWIF_JOB_NAME="GLUEX_recon_${RUN_NUMBER}"
  command1="swif2 add-job -workflow ${SWIF_WORKFLOW} -name ${SWIF_JOB_NAME} "
  command2=""
  for (( NERSC_NODE_INDEX=0; NERSC_NODE_INDEX < NERSC_NMB_NODES; NERSC_NODE_INDEX++ ))
  do
    EVIO_FILE_START_INDEX=$((NERSC_NODE_INDEX * NERSC_NMB_JOBS_PER_NODE))
    EVIO_FILE_END_INDEX=$((EVIO_FILE_START_INDEX + NERSC_NMB_JOBS_PER_NODE))
    # Print 5 values from the array
    ##echo "NERSC_NODE_INDEX = ${NERSC_NODE_INDEX} -> Values: ${EVIO_FILE_PATHS[@]:EVIO_FILE_START_INDEX:5}"
    ##node_mss=${EVIO_FILE_PATHS[@]:EVIO_FILE_START_INDEX:5}
    node_mss=${files[@]:EVIO_FILE_START_INDEX:${NERSC_NMB_JOBS_PER_NODE}}
    #node_cache="${node_mss/'/mss'/'mss:/mss'}"
    node_cache="${node_mss}"
    RUNDIR=$(printf "RUN%06d/FILE%03d" "${RUN_NUMBER}" "${NERSC_NODE_INDEX}")
    node_volatile="${SWIF_OUTPUT_ROOT}/${RUNDIR}"
    echo "mkdir -p ${node_volatile}"
    mkdir -p ${node_volatile}
    #####command2+=" -name ${SWIF_JOB_NAME} -input ${node_cache} -output match:${RUNDIR}/* ${node_volatile}"
    command2+=" ${node_cache} -output match:${RUNDIR}/* ${node_volatile}"
    #echo "swif2 add-job -workflow ${SWIF_WORKFLOW} -name ${SWIF_JOB_NAME} -input ${node_cache} -output match:RUN${RUN_NUMBER}/FILE${i}/* ${node_volatile}"
  done
  command3=" -sbatch -A ${NERSC_PROJECT} --volume=\"${NERSC_LAUNCH_DIR}:/launch-${BATCH}\" --image=${NERSC_IMAGE} --module=cvmfs --time=5:00:00 -N ${NERSC_NMB_NODES} --tasks-per-node=1 --cpus-per-task=256 --qos=${NERSC_QOS} -C cpu :: ${NERSC_LAUNCH_DIR}/script_nersc_multi_test.sh ${NERSC_LAUNCH_DIR} /launch-${BATCH}/script_nersc_test.sh /launch-${BATCH}/${JANA_CONFIG} ${RECON_VERSION} ${NERSC_NMB_JOBS_PER_NODE}"
  command4=$command1$command2$command3
  echo ${command4} >| exec_${RUN_NUMBER}.sh
  #echo "${NERSC_LAUNCH_DIR}/script_nersc_multi.sh ${NERSC_LAUNCH_DIR} ${NERSC_LAUNCH_DIR}/script_nerscc.sh ${NERSC_LAUNCH_DIR}/${JANA_CONFIG} halld_recon/halld_recon-5.0.0" > exec.sh
  chmod +x exec_${RUN_NUMBER}.sh
  ./exec_${RUN_NUMBER}.sh
done

#srun -n $SLURM_JOB_NUM_NODES /path/to/launchdir/run_shifter_multi.sh test script.sh c.cfg halld0404
