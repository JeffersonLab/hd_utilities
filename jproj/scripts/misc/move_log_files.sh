#!/bin/sh
job_id=$1
target_directory=$2
mkdir -p /u/scratch/$USER/move
# create command file
command_file=/u/scratch/$USER/move/move_${job_id}.sh
touch $command_file
echo \#\!/bin/sh >> $command_file
echo echo sleep 200 >> $command_file
echo sleep 200 >> $command_file
echo echo mkdir -p $target_directory >> $command_file
echo mkdir -p $target_directory >> $command_file
echo echo mv -v $HOME/.farm_out/\*$job_id\* $target_directory >> $command_file
echo mv -v $HOME/.farm_out/\*$job_id\* $target_directory >> $command_file
chmod a+x $command_file
# create jsub file
jsub_file=/u/scratch/$USER/move/move_${job_id}.jsub
touch $jsub_file
echo PROJECT: gluex >> $jsub_file
echo JOBNAME: move_$job_id >> $jsub_file
echo TRACK: debug >> $jsub_file
echo COMMAND: $command_file >> $jsub_file
echo MEMORY: 500 MB >> $jsub_file
jsub $jsub_file
echo command_file = $command_file, jsub_file = $jsub_file
