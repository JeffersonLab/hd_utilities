#!/bin/bash
while read line
do
    command=`echo $line | awk -F'/include/' '{print "full_path="$2}'`
    eval $command
    if [[ $full_path =~ / ]]
    then
	command=`echo $full_path | awk -F/ '{print "dir="$1}'`
	eval $command
	mkdir -p include/$dir
	git mv old_src/libraries/$full_path include/$full_path
    else
	mkdir -p include
	git mv old_src/libraries/include/$full_path include/$full_path
    fi
done
