#!/bin/bash

#rnb_tab=( $( cat list-good-2021-11-d.txt ) )
#rnb_tab=( $( cat rp-2025-01-period-38.txt ) )
#rnb_tab=( $( cat list-to-run-ver02-2025-01.txt ) )
#rnb_tab=( $( cat list-to-run-ver05-2017-01.txt ) )
rnb_tab=( $( cat list-2025-01-ver03-perl.txt ) )
for rnb in "${rnb_tab[@]}"; do
    echo "bash my_launch.sh 2025-01 $rnb 03-perl 32 ${rnb_tab[0]}"
    bash my_launch.sh 2025-01 $rnb 03-perl 32 ${rnb_tab[0]}
done
