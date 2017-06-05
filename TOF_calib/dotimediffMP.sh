#!/bin/sh

Run=$1

root -b -q "src/timedifference.C++($Run,1,0)" 

for x in  `seq 2 1 10`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,0)" &
done
# Wait for all parallel jobs to finish
wait 

for x in  `seq 11 1 20`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,0)" &
done
# Wait for all parallel jobs to finish
wait

for x in  `seq 21 1 30`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,0)" &
done
# Wait for all parallel jobs to finish
wait

for x in  `seq 31 1 40`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,0)" &
done
# Wait for all parallel jobs to finish
wait

for x in  `seq 41 1 44`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,0)" &
done
# Wait for all parallel jobs to finish
wait

# now second plane

for x in  `seq 1 1 10`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,1)" &
done
# Wait for all parallel jobs to finish
wait



for x in  `seq 11 1 20`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,1)" &
done
# Wait for all parallel jobs to finish
wait

for x in  `seq 21 1 30`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,1)" &
done
# Wait for all parallel jobs to finish
wait

for x in  `seq 31 1 40`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,1)" &
done
# Wait for all parallel jobs to finish
wait

for x in  `seq 41 1 44`; do
    echo $x
    root -b -q "src/timedifference.C+($Run,$x,1)" &
done
# Wait for all parallel jobs to finish
wait



