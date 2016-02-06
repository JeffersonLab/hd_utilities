#!/bin/tcsh

rm -f ~/incoming_log.txt
source ~/env_monitoring_incoming > ~/incoming_log.txt
python ~/monitoring/incoming/process_incoming.py 2016-02 ~/monitoring/incoming/input.config 20 >> ~/incoming_log.txt

