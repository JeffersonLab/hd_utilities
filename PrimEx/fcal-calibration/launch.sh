#!/bin/bash

i=0
imax=14
while [ $i -lt $imax ]; do
    ./ilaunch_v0.sh period_$i 2>&1 > outlog_$i.txt &
    i=$(($i+1))
done
