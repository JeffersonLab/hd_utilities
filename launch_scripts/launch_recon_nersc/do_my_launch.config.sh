# Set all configuration variables needed to define a reconstruction
# launch in one place.  This file is sourced by the `do_my_launch.sh`
# script.

# shellcheck shell=bash
# shellcheck disable=SC2034  # variables are consumed by `do_my_launch.sh`

# set names and run list
readonly RUN_PERIOD="2022-05"  # Run period to process.
readonly VER="02"  # Version of this reconstruction launch.
readonly BATCH="${VER}-perl"  # Batch label used to hold details of the launch; is appended to workflow name and directory names; should make it clear where/how the campaign was being run.
readonly LAUNCH_DIR="launch.${RUN_PERIOD}_ver${BATCH}"  # subdirectory where production scripts are located
readonly RUN_NUMBER_LIST_FILE="list.${RUN_PERIOD}_ver${BATCH}.txt"  # text file with list of run numbers to process; one run number per line

# set production campaign parameters for JLab site
readonly PRODUCTION_USER="gxproj4"  # production user account name
readonly PRODUCTION_START_DATE="2026-02-19"  # start date of production campaign
readonly PRODUCTION_WORK_DIR="NERSC/${PRODUCTION_START_DATE}.recon.${RUN_PERIOD}_ver${BATCH}"  # subdirectory in home directory of `${PRODUCTION_USER}` where all production info is stored
readonly PRODUCTION_LAUNCH_DIR="${PRODUCTION_WORK_DIR}/${LAUNCH_DIR}"  # directory where production is launched from at JLab

# set reconstruction parameters
readonly HALLD_VERSION_SET_XML="version_7.4.0.xml"  # XML file that defines the Hall-D version set to be used.
readonly JANA_CONFIG="jana_recon_nersc.${RUN_PERIOD}_ver${BATCH}.config"  # JANA config file to use; must be located in ${NERSC_LAUNCH_DIR}.
readonly JANA_CALIB_CONTEXT="calibtime=2026-02-18-00-00-01"  # JANA calibration context; overrides any value set in the JANA config file or in the environment.
readonly JANA_GEOMETRY_URL="ccdb:///GEOMETRY/cpp_HDDS.xml"  # JANA geometry to be used in reconstruction; usually `ccdb:///GEOMETRY/main_HDDS.xml` but some data sets such as CPP require custom geometry.

# set swif2 job parameters
readonly SWIF_MAX_CONCURRENT_JOBS=100  # Maximum number of swif2 jobs that can be in-flight at once. This can be set only once when the workflow is created. If jobs are submitted piecemeal by running this script multiple times specifying different run lists, only the first invocation that creates the workflow will set this parameter.
readonly SWIF_RAW_DATA_ROOT="/mss/halld/RunPeriod-${RUN_PERIOD}/rawdata"  # Root of JLab directory tree, where raw data files are located. Must be an `/mss` path.
readonly SWIF_OUTPUT_ROOT="/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-${RUN_PERIOD}/recon/ver${BATCH}"  # Root of JLab directory tree, where output files are copied to.
readonly SWIF_WORKFLOW="recon_${RUN_PERIOD}_ver${BATCH}_NERSC-multi"  # Only change this if default name is not appropriate
readonly SWIF_SITE="nersc/perlmutter"  # swif2 site to submit jobs to

# set NERSC job parameters
readonly NERSC_PROJECT="m3120"  # NERSC project to charge to.
readonly NERSC_PROJECT_DIR="/global/cfs/cdirs/${NERSC_PROJECT}"  # Project directory in the NERSC Common File System (CFS) where config files and scripts will be copied to and run from.
readonly NERSC_LAUNCH_DIR="${NERSC_PROJECT_DIR}/${LAUNCH_DIR}"  # NERSC directory that will get mapped to /launch_${BATCH} inside the task container. Contains scripts and JANA config file to run hd_root.
readonly NERSC_QOS="regular"  # NERSC queue to use; usually `regular` or `debug`. See NERSC documentation for details on charging and restrictions.
readonly NERSC_NODE_TYPE="cpu"  # Constraint for NERSC jobs; usually `cpu` or `gpu`.
readonly NERSC_MAX_WALL_TIME="5:00:00"  # Maximum wall time for NERSC jobs.
readonly NERSC_MAX_THREADS_PER_TASK=256  # Maximum number of threads available on a NERSC Perlmutter CPU node.
readonly NERSC_NMB_THREADS_PER_PROCESS=32  # Number of threads that each `hd_root` process uses; must be <= ${NERSC_MAX_THREADS_PER_TASK}. Overrides any value set in the JANA config file.
readonly NERSC_NMB_PROCESSES_PER_TASK=$(echo "${NERSC_MAX_THREADS_PER_TASK} / ${NERSC_NMB_THREADS_PER_PROCESS}" | bc)  # Number of hd_root processes to run concurrently on a single NERSC Perlmutter CPU node.  #TODO works only if division is exact; need to round up if not exact
readonly NERSC_HOST="perlmutter-p1.nersc.gov"  # NERSC hostname to use for ssh.
readonly NERSC_CONTAINER_IMAGE="docker:jeffersonlab/gluex_almalinux_9:latest"  # Shifter image that was converted from Docker image. Is not pulled in automatically and needs to exist in Shifter registry.

# variables in the original script that are not used in the new version but might be needed in the future:
# TESTMODE
# VERBOSE
# LAUNCHTYPE
# NAME
# TIMELIMIT
# SCRIPTFILE
