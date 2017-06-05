#!/bin/tcsh

set Run = $1
  
root -b -q "src/timedifferenceAMP.C++($Run,1,0)"

foreach x (`seq 2 1 10`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,0)" &
end
wait
foreach x (`seq 11 1 20`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,0)" &
end
wait
foreach x (`seq 21 1 30`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,0)" &
end
wait
foreach x (`seq 31 1 44`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,0)" &
end
wait


foreach x (`seq 1 1 10`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,1)" &
end
wait

foreach x (`seq 11 1 20`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,1)" &
end
wait

foreach x (`seq 21 1 30`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,1)" &
end
wait

foreach x (`seq 31 1 44`)
    echo $x
    root -b -q "src/timedifferenceAMP.C+($Run,$x,1)" &
end
wait
