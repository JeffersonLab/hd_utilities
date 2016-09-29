#!/bin/bash

source ~/env_monitoring_incoming.sh
python ~/monitoring/incoming/submit_jobs.py 2016-10 ~/monitoring/incoming/jobs_incoming.config 5

