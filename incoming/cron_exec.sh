#!/bin/bash

source ~/env_monitoring_incoming.sh
export PATH=/site/bin:${PATH}
python ~/monitoring/incoming/submit_jobs.py 2016-10 ~/monitoring/incoming/jobs_incoming.config 5

