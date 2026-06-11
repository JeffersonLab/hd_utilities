#!/usr/bin/env bash

# Puts all pending jobs into hold state in an infinite loop to catch
# any new jobs submitted by swif2

INTERVAL="10m"  # interval to wait between putting pending jobs into hold state

while true
do
  date
  echo "Putting all pending jobs into hold state; sleeping for ${INTERVAL} before repeating"
  while read -r JOB_ID
  do
    echo "    Putting job ${JOB_ID} into hold state"
    scontrol hold "${JOB_ID}"
  done < <(squeue -t PENDING --me -h -o "%i")
  sleep "${INTERVAL}"
done
