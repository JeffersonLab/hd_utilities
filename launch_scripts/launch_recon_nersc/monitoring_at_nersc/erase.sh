#!/usr/bin/env bash

# Erases input files from swif2 directory at NERSC.


RUN_NUMBERS=($(cat list_to_erase.txt))

NERSC_INPUTDIR_ROOT="/pscratch/sd/j/jlab/swif/input"  # root of the directory tree where NERSC jobs have their working directories
for RUN_NUMBER in "${RUN_NUMBERS[@]}"
do
  echo "run number: ${RUN_NUMBER}"
  rm --verbose "${NERSC_INPUTDIR_ROOT}/${RUN_NUMBER}"  #TODO this does not work because swif2 stores files with catalog ID as name
done
