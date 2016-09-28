#!/bin/bash

#rm -f ~/incoming_log.txt
source ~/env_monitoring_incoming.sh #> ~/incoming_log.txt
python ~/monitoring/incoming/submit_jobs.py 2016-10 ~/monitoring/incoming/input.config 5 #>> ~/incoming_log.txt

