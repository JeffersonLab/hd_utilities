# !/bin/bash

period=$1
wfname=primex-fcal-gains-matching
imax=3
i=1
while [ $i -lt $imax ]; do
    wf=$wfname-$period-$i
    ./igo_auto.sh $i $period $wfname
    python icontrol.py $wf $period $i
    i=$(($i+1))
done
