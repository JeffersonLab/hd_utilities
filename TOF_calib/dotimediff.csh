#!/bin/tcsh

set Run = $1

foreach x (`seq 1 1 44`)
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,0)"
end
foreach x (`seq 1 1 44`)
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,1)"
end
