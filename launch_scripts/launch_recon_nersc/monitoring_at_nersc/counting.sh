#!/usr/bin/env bash

# Counts the number of jobs and nodes in NERSC queue and appends these
# numbers to a text file with a timestamp.
#TODO what is this good for?


UNIX_TIMESTAMP=$(date +%s)
NMB_JOBS_ALL=$(squeue --noheader | wc -l)
NMB_JOBS_USER=$(squeue --noheader --user=jlab | wc -l)
source make_list.sh
LIST_FILE="list-queue.txt"
NMB_NODES_ALL=$(awk '{sum += $(NF-1)} END {print sum}' "${LIST_FILE}")
LIST_FILE="list-jlab-queue.txt"
NMB_NODES_USER=$(awk '{sum += $(NF-1)} END {print sum}' "${LIST_FILE}")
NMB_JOBS_RUNNING_ALL=$(squeue | grep "  R" | wc -l)
mkdir --parents txt
echo "${UNIX_TIMESTAMP} ${NMB_JOBS_ALL} ${NMB_JOBS_USER} ${NMB_NODES_ALL} ${NMB_NODES_USER}" >> txt/file.txt
echo "${UNIX_TIMESTAMP} ${NMB_JOBS_ALL} ${NMB_JOBS_USER} ${NMB_NODES_ALL} ${NMB_NODES_USER} ${NMB_JOBS_RUNNING_ALL}" >> txt/file_co.txt
