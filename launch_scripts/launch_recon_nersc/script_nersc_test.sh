#!/usr/bin/env bash
set -o nounset  # exit if trying to use an uninitialized variable
set -o xtrace  # turn on command tracing

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
# arg 2:  Hall-D version set XML file
# arg 3:  Number of threads to use for this job

# mount /var/udiMount/  #TODO is this really needed?
# mount /var/udiMount/launch-BATCH  #TODO is this really needed?
docker inspect jeffersonlab/gluex_almalinux_9:latest  #TODO shouldn't the image be a parameter passed to the script?

# exit when any command fails
set -o errexit

# keep track of last executed command
trap 'last_command=${current_command}; current_command=${BASH_COMMAND}' DEBUG
# echo error message before exiting if error occurs
trap 'echo "\"${last_command}\" command failed with exit code ${?}."' EXIT  #TODO is this is executed also for non-error exit? If so, it should be modified to only print the message if the exit code is non-zero.

#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

JANA_CONFIG="${1}"
HALLD_VERSION_SET_XML="${2}"
NMB_TREADS_PER_JOB="${3}"
EXTRA_ARGS=""

ulimit -c unlimited

# Setup software environment according to Hall-D version set XML file
source "/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh"
gxenv "${HALLD_VERSIONS}/${HALLD_VERSION_SET_XML}"

# If the binaries in the group disk need to be replaced, then
# they can be placed in a directory on $SCRATCH and we'll use
# the HALLD_MY mechanism to superceed them. This was needed
# for offmon 2019-11 ver 13 when certain plugins had to be
# recompiled with a patch.
# export HALLD_MY="${CSCRATCH}/HALLD_MY/${HALLD_RECON_VERSION}"  #TODO this seems to be dysfunctional; ${CSCRATCH} does not exist anymore on NERSC
# export PATH="${HALLD_MY}/${BMS_OSNAME}/bin:${PATH}"

workdir="${PWD}"
# Use CCDB and RCDB from CVMFS. Make a temporary local
# copy so that we don't interfere with other jobs locking
# the same file
cp "/group/halld/www/halldweb/html/dist/ccdb.sqlite" /dev/shm
#cp "/global/cscratch1/sd/jlab/work/2020.07.07.split_jobs/ccdb.sqlite" /dev/shm   # TEMPORARY <-- corrupt ccdb.sqlite file on 7/7/2020 forced this
#if [ -z ${HOME+x} ]; then export HOME="/global/homes/j/jlab" ; fi
#cp "${HOME}/ccdb.sqlite" /dev/shm   # TEMPORARY <-- corrupt ccdb.sqlite file on 7/18/2020 forced this
cp "/group/halld/www/halldweb/html/dist/rcdb.sqlite" /dev/shm
ls -lrth /dev/shm/ccdb.sqlite
ls -lrth /dev/shm/rcdb.sqlite
export JANA_CALIB_URL="sqlite:////dev/shm/ccdb.sqlite"
export CCDB_CONNECTION="${JANA_CALIB_URL}"
export RCDB_CONNECTION="sqlite:////dev/shm/rcdb.sqlite"
export JANA_GEOMETRY_URL="ccdb:///GEOMETRY/main_HDDS.xml"

# Use JANA_RESOURCE_DIR from CVMFS
export JANA_RESOURCE_DIR="/group/halld/www/halldweb/html/resources"

ls -lrth > myverif.out

# Record some info about the node and environment
rm -f top.out
top -b -n 1 > top.out

rm -f cpuinfo.out
cat /proc/cpuinfo > cpuinfo.out

rm -f env.out
env > env.out

rm -f hostname.out
hostname > hostname.out

ls -lrth >> myverif.out

# Run script in background that will send abort signal after some hours (hard coded in script)
#/launch/assassin.sh &> assassin.out  &

# Do not exit immediately if hd_root fails. This allows us to
# catch the exit code and write it to a file. This is important
# for multi-file jobs since individual error codes are not captured
# by slurm.
set +o errexit

## Copy the raw data to RAM disk
#file=$(ls hd_rawdata*.evio)
#mv $file tmp.evio
#cp tmp.evio $file
#rm tmp.evio

ls -lrth ${JANA_CONFIG} >> myverif.out
cat ${JANA_CONFIG} >> myverif.out
echo ${PWD} >> myverif.out
echo ${HALLD_RECON_HOME} >> myverif.out
# Run hd_root
echo "I am here 0"
rawdata_tab=(hd_rawdata_??????_???.evio)
i=0
for rawdata in "${rawdata_tab[@]}"
do
    run_number="${rawdata:11:6}"
    file_number="${rawdata:18:3}"
    echo "${run_number} ${file_number} ${i}"
    mkdir -p "${workdir}/run-${run_number}-${file_number}"
    cd "${workdir}/run-${run_number}-${file_number}"
    CMD="hd_root -PNTHREADS=${NMB_TREADS_PER_JOB} --loadconfigs ${JANA_CONFIG} ${EXTRA_ARGS} ../${rawdata}"  # -PNTHREADS=N overwrites any NTHREADS value set in the JANA config file
    echo "${CMD}" >> ../myverif.out
    ${CMD} 2> "std_${run_number}_${file_number}.err" 1> "std_${run_number}_${file_number}.out" &

    # Capture the background process ID (PID)
    pid=${!}

    # Store the PID for later use in capturing the exit code
    pids[${i}]=${pid}
    ((i++))
done
echo "I am here 1"
cd ${workdir}
# Wait for all background jobs to complete and capture their exit codes
for i in "${!pids[@]}"
do
    wait ${pids[${i}]}   # Wait for the process to finish
    exitcode=${?}
    # Turn back on exit on error

    echo "Exit code for job ${i}: ${exitcode}" > "exitcode_${i}.txt"
done
set -o errexit
echo "I am here 2"
ls -lrth >> myverif.out
ls -lrth */* >> myverif.out

# Remove link to input file.
# The swif2 job will copy all files in this directory back
# to JLab so we don't want the raw data file to be copied back.
######rm -f hd_rawdata_??????_???.evio
#if [[ ! "$file" =~ rawdata_([0-9]{6})_([0-9]{3})\.evio ]]; then
rawdata_tab=(hd_rawdata_??????_???.evio)
###j=0
for rawdata in "${rawdata_tab[@]}"
do
    run_number="${rawdata:11:6}"   # Extracts 6 digits starting from index 9
    file_number="${rawdata:18:3}" # Extracts 3 digits starting from index 16
    cd "${workdir}/run-${run_number}-${file_number}"
    echo "${run_number} ${file_number} ${j}"
    echo "${PWD}"
    ls -lrth >> ../myverif.out
    # Move small files into a directory and make a tarball
    JOB_INFO="job_info_${run_number}_${file_number}"
    echo "${JOB_INFO}"
    mkdir "${JOB_INFO}"
    cp ../top.out "${JOB_INFO}"
    cp ../cpuinfo.out "${JOB_INFO}"
    cp ../env.out "${JOB_INFO}"
    cp ../hostname.out "${JOB_INFO}"
    #    cp ../exitcode_$j.txt  "${JOB_INFO}/"
    cp ../exitcode_*.txt  "${JOB_INFO}/"
    cp ../myverif.out "${JOB_INFO}"
    #mv helicity.log "std_${run_number}_${file_number}.err" "std_${run_number}_${file_number}.out" "${JOB_INFO}"
    mv "std_${run_number}_${file_number}.err" "${JOB_INFO}"
    mv "std_${run_number}_${file_number}.out" "${JOB_INFO}"
    tar czf "${JOB_INFO}.tgz" "${JOB_INFO}"
    #rm -rf "${JOB_INFO}"
    #mv "${JOB_INFO}.tgz" ../
    files_tab=(*.*)
    for file in "${files_tab[@]}"
    do
        echo "${file}"
        if [[ ! "${file}" =~ *_([0-9]{6})_([0-9]{3}).* ]]
        then
            # Extract the base filename without extension
            basefile=$(basename "${file}")
            # Extract the file extension
            extension="${basefile##*.}"
            # Construct the new filename with run_number and file_number inserted before the extension
            new_name="${basefile%.*}_${run_number}_${file_number}.${extension}"
            # Rename the file
            mv "${file}" "${new_name}"
        fi
    done
###((j++))
done
echo "I am here 3"
cd "${workdir}"
ls * > my-second-verif.txt
ls */* >> my-second-verif.txt
mv run-*/*.* .
echo "I am here 4"
# Remove ccdb.sqlite and rcdb.sqlite files
rm -f /dev/shm/ccdb.sqlite
rm -f /dev/shm/rcdb.sqlite
#rm -rf run-*
rm -f hd_rawdata_??????_???.evio
