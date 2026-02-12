rnb_tab=( $( cat list_to_erase.txt ) )

idir=/pscratch/sd/j/jlab/swif/input/
for rnb in "${rnb_tab[@]}"; do
    echo "rnb: $rnb"
    rm $idir/$rnb
done
