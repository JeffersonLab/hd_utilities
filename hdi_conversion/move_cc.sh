#!/bin/bash
while read line
do
    command=`echo $line | awk -F'/src/' '{print "full_path="$2}'`
    eval $command
    command=`echo $full_path | awk -F/ '{print "dir="$1}'`
    eval $command
    mkdir -p src/$dir
    git mv old_src/libraries/$full_path src/$full_path
done
