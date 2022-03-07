#!/bin/tcsh

set Run = $1
set Bars = 44
if ($Run > 69999) set Bars = 46

foreach x (`seq 1 1 $Bars`)
    echo $x
    root -b -q "src/timedifference.C++($Run,$x,0)"
end
foreach x (`seq 1 1 $Bars`)
    echo $x
    root -b -q "src/timedifference.C++($Run,$x,1)"
end
