#!/bin/bash

source ~/env_monitoring_incoming.sh
export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF
python ~/monitoring/incoming/submit_jobs.py 2017-01 ~/monitoring/incoming/jobs_incoming.config 5

