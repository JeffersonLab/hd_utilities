#!/bin/bash

date
source ~/env_analysis_launch.sh
export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF
python ~/hd_utilities/launch_scripts/merge_trees/merge_trees.py ~/hd_utilities/launch_scripts/merge_trees/jobs_merge.config 0

