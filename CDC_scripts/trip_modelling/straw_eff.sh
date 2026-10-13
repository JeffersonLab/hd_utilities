#!/bin/sh

if [ $# -ne 1 ] ; then
  echo "Usage: straw_eff.sh <straw_number>"
  exit
fi

straw=$1
max=0
filename=straw_eff_per_run/eff_$straw

if [ -f $filename ] ; then
    rm $filename
fi

for x in `seq 150097 150528` ; do 
  if [ -f eff_from_histo/eff_$x.txt ] ; then 
    y=`head -n $straw eff_from_histo/eff_$x.txt | tail -n 1` 
    echo $x $y >> $filename

    if (( $(echo "$y > $max" | bc -l) )); then
	max=$y
    fi
    
  fi
 
done

echo max efficiency $max

python ../scripts/rplot.py $straw

python ../scripts/round_efficiency.py $filename $max



