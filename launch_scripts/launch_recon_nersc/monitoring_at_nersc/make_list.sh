#!/usr/bin/env bash

# Makes lists of jobs in the queue at NERSC, both for all users and
# for user `jlab` specifically.


squeue >| list-queue.txt
squeue --user=jlab >| list-jlab-queue.txt
