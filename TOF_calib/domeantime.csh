#!/bin/tcsh
set Run = $1
foreach x (`seq 1 44`)
    echo $x
    root -b -q "src/meantime1.C+($Run,$x,0);"
end

foreach x (`seq 1 44`)
    echo $x
    root -b -q "src/meantime1.C+($Run,$x,1)"
end

root -b -q "src/meantime2.C+($Run)"


