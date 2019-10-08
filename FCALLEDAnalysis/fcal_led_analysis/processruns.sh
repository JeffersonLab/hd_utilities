#!/bin/sh
#
#$ -S /bin/sh
#$ -V
#$ -j y
#$ -o led.log
#$ -cwd
#$ -pe mpi 200

RUNDIR1="/fs12/mashephe/fcal_led_2019-01"
RUNDIR2="/fs12/mashephe/fcal_led_2018-08"
RUNDIR="/fs12/mashephe/fcal_led_2019/"
CMD="./fcal_processrun"

PIDS=""

printf "Starting %s script with %s...\n" "$CMD" "$NSLOTS"

for f in $(find "${RUNDIR1}" -name "led_Run*.root" -type f) ; do
  $CMD "$f" & PIDS="$PIDS $!"
done

for p in $(printf "%s\n" $PIDS) ; do
  wait "$p"
done

printf "Starting fall data....\n"

PIDS=""

for f in $(find "${RUNDIR2}" -name "led_Run*.root" -type f) ; do
  $CMD "$f" & PIDS="$PIDS $!"
done

for p in $(printf "%s\n" $PIDS) ; do
  wait "$p"
done

find data/ -type f -name "data_*.lock" -delete

printf "%s script over" "$CMD"
