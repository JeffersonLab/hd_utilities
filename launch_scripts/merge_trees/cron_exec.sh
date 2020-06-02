#!/bin/bash

date
source ~/env_monitoring_launch.sh
export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF
python ~/monitoring/merge_trees/merge_trees.py ~/monitoring/merge_trees/jobs_merge.config 0

