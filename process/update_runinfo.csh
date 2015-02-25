#!/bin/tcsh

# Load standard environment for ROOT
source /home/gluex/setup_jlab_commissioning.csh

set MONITORING_HOME=/home/gluex/halld/monitoring/process
source $MONITORING_HOME/monitoring_env.csh

# run the script
cd $MONITORING_HOME

# overwrite files
unset noclobber 
./datamon_db_tool.py DumpTSV run_info | awk 'NR<2{print $0;next}{print $0| "sort -n -k2,2"}' > /group/halld/www/halldweb1/html/online/run_info/runinfo.tsv.txt
./datamon_db_tool.py DumpCSV run_info | awk 'NR<2{print $0;next}{print $0| "sort -n -t, -k2,2"}' > /group/halld/www/halldweb1/html/online/run_info/runinfo.csv.txt

