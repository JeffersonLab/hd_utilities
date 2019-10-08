#!/bin/sh
#
#$ -S /bin/sh
#$ -V
#$ -j y
#$ -o led.log
#$ -cwd
#$ -pe mpi 5

RUNDIR="data/"
CMD="./fcal_doubleratio"

PIDS=""

printf "Starting %s script with %s...\n" "$CMD" "$NSLOTS"

for f in $(find "${RUNDIR}" -name "data_*.root" -type f) ; do
  $CMD "$f" "$f" & PIDS="$PIDS $!"
done

for p in $(printf "%s\n" $PIDS) ; do
  wait "$p"
done

printf "%s script over" "$CMD"
