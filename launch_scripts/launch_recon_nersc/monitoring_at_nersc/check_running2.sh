#!/usr/bin/env bash

# Checks status of reconstruction jobs at NERSC


TASK_INDEX="${1}"
TASK_INDEX=$(printf "%03d" "${TASK_INDEX}")  # ensure TASK_INDEX is zero-padded to three digits, e.g. 7 -> 007

echo "$(date)"

# report job statistics
ALL_JOBS=$(squeue --noheader)  # get all jobs in the queue, excluding the header line
USER_JOBS=$(squeue --noheader --user=jlab)  # get jobs in the queue for user `jlab`, excluding the header line
echo "Number of jobs for user jlab in the queue: $(echo "${USER_JOBS}" | wc -l)"
echo "Number of jobs running for user jlab: $(echo "${USER_JOBS}" | grep "  R" | wc -l)"
echo "Number of job in the queue: $(echo "${ALL_JOBS}" | wc -l)"
echo "Number of job running: $(echo "${ALL_JOBS}" | grep "  R" | wc -l)"


NERSC_WORKDIR_ROOT="/pscratch/sd/j/jlab/swif/jobs/gxproj4"  # root of the directory tree where NERSC jobs have their working directories

# check dana*_000.hddm files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/dana*_000.hddm
NMB_DANA_1=$(ls -1 "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/dana*_000.hddm | wc -l)

# check std.out files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/std.out
NMB_STDOUT_1=$(ls -1 "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/std.out | wc -l)

# check core* files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/TASK???/core*
chmod og+rw "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/TASK???/core*

# check .tgz tarballs with log files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/TASK???/*.tgz

echo 'Multiple jobs per node'

# check dana* files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/*000/dana*
NMB_DANA_2=$(ls -1 "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/*000/dana* | wc -l)

# check std*_000.out files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/*/std*_000.out
NMB_STDOUT_2=$(ls -1 "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/"TASK${TASK_INDEX}"/*/std*_000.out | wc -l)

# check core* files
ls -lrth "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/TASK???/*/core*
chmod og+rw "${NERSC_WORKDIR_ROOT}"/*_recon_*/*/RUN??????/TASK???/*/core*

source check_space.sh

# report number of nodes
#TODO use USER_JOBS and ALL_JOBS instead of list files
source make_list.sh  # make job lists "./list-queue.txt" and "./list-jlab-queue.txt" for all users and for user `jlab`, respectively
LIST_FILE="./list-queue.txt"
echo "Nb of dana 1 : ${NMB_DANA_1}, nb of std.out 1: ${NMB_STDOUT_1}"
echo "Nb of dana 2 : ${NMB_DANA_2}, nb of std.out 2: ${NMB_STDOUT_2}"
SUM=$(awk '{sum += $(NF-1)} END {print sum}' "${LIST_FILE}")  # sum second-to-last field on each line of the list file and return the total at the end
echo "Number of nodes requested by all jobs: ${SUM}"
LIST_FILE="./list-jlab-queue.txt"
SUM=$(awk '{sum += $(NF-1)} END {print sum}' "${LIST_FILE}")
echo "Number of nodes requested by swif2: ${SUM}"

echo "$(date)"
