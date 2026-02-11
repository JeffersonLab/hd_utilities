#!/bin/bash

# Submit jobs to run at NERSC using swif2
#
# See more detailed documentation here:
#  https://halldweb.jlab.org/wiki/index.php/HOWTO_Execute_a_Launch_using_NERSC
#
# Master script that submits a reconstruction job for each run number
# in the given list file.
#
# Here is how this is supposed to work:
#
# > do_my_launch.sh
#   |
#   |-> my_launch.sh
#       * generates script_nersc_test.sh and jana_recon_nersc.config from templates
#       * creates and runs swif2 workflow at ifarm
#       * copies job scripts to NERSC
#       * creates and executes exec_<run number>.sh scripts for each run number
#       |
#       |-> exec_<run number>.sh
#           |
#           |-> swif2 add-job (submits job at ifarm queue) + sbatch (submits job at NERSC queue)
#               |
#               |-> @NERSC: script_nersc_multi_test.sh
#                   |
#                   |-> @NERSC: script_nersc_multi_test.py
#                       |
#                       |-> @NERSC: srun run_shifter_multi.sh
#                           |
#                           |-> @NERSC: shifter script_nersc_test.sh (this is run from inside a shifter container)
#                               |
#                               |-> @NERSC: hd_root


rnb_tab=( $( cat list-2025-01-ver03-perl.txt ) )
for rnb in "${rnb_tab[@]}"; do
    echo "bash my_launch.sh 2025-01 $rnb 03-perl 32 ${rnb_tab[0]}"
    bash my_launch.sh 2025-01 $rnb 03-perl 32 ${rnb_tab[0]}  # run period, run number, batch, thread number, first run number
done
