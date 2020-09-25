#!/bin/tcsh -f
set period=$1
set wfname=primex-fcal-gains-matching
set imax=7
set i=1
while ( $i < $imax )
    set wf=$wfname-$period-$i
    ./igo_auto.csh $i $period $wfname
    echo $wf $period $i
    python icontrol.py $wf $period $i
    @ i ++
    echo $i
end
