i=0
imax=14
while [ $i -lt $imax ]; do
    j=0
    jmax=6
    while [ $j -lt $jmax ]; do
	source ../Env/delete_swif.sh primex-fcal-gains-matching-period_$i-$j
	j=$(($j+1))
    done
    i=$(($i+1))
done
