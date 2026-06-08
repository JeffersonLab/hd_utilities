#!/usr/bin/env bash

# Releases all pending jobs from hold state

echo "Releasing all pending jobs from hold state"
while read -r JOB_ID
do
  scontrol release "${JOB_ID}"
done < <(squeue --states="PENDING" --me --noheader --format="%i")
