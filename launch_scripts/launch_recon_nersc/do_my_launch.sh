#!/bin/bash

# Master script that submits a reconstruction jobs to run at NERSC
# using swif2. One job is submitted for each run number in the given
# list file.

rnb_tab=( $( cat list-2025-01-ver03-perl.txt ) )
for rnb in "${rnb_tab[@]}"; do
    echo "bash my_launch.sh 2025-01 $rnb 03-perl 32 ${rnb_tab[0]}"
    bash my_launch.sh 2025-01 $rnb 03-perl 32 ${rnb_tab[0]}  # run period, run number, batch, thread number, first run number
done
