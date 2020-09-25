#!/bin/tcsh -f
set m=48
#set n=21
set imax=49
set wfname=pedcal
while ( $m < $imax )
    set wf=$wfname-period_$m-ped
    source igo_ped.csh period_$m $wfname
    python icontrol_ped.py $wf   
#    source iped_add.csh $m $n
#    source iloop.csh $m $n
    source iped_add.csh $m
    source iloop.csh $m
#    echo $m $n $wf
#    @ n ++
    @ m ++
end
