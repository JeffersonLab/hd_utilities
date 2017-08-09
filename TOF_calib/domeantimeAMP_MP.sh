#!/bin/sh

Run=$1
    
root -b -q "src/meantime1AMP.C++($Run,1,0)"

for x in `seq 2 1 10`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,0)" &
done
wait

for x in `seq 11 1 20`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,0)" &
done
wait

for x in `seq 21 1 30`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,0)" &
done
wait

for x in `seq 31 1 44`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,0)" &
done
wait

# now the other plane
for x in `seq 1 1 10`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,1)" &
done
wait

for x in `seq 11 1 20`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,1)" &
done
wait

for x in `seq 21 1 30`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,1)" &
done
wait

for x in `seq 31 1 44`; do
    echo $x
    root -b -q "src/meantime1AMP.C+($Run,$x,1)" &
done
wait

