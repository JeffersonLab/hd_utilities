#!/bin/bash

source ~/env_monitoring_incoming.sh
export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF
python ~/hd_utilities/launch_scripts/incoming/submit_jobs.py 2025-01 ~/hd_utilities/launch_scripts/incoming/jobs_incoming.config 5

