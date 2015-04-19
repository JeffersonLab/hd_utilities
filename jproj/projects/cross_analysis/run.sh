#!/bin/tcsh

set NEWVER = 13

make

# add table columns for new version
time ./add_columns $NEWVER

# fill table entries for new version
time ./write_inserts_each_launch.sh

# vers 09 - 10
time ./spitout_timePlugin_mem_vmem.sh 2 9
time ./process_timePlugin_mem_vmem -N 2 -V 9

# vers 09 - 11
time ./spitout_timePlugin_mem_vmem.sh 3 9
time ./process_timePlugin_mem_vmem -N 3 -V 9

# vers 09 - 12
time ./spitout_timePlugin_mem_vmem.sh 4 9
time ./process_timePlugin_mem_vmem -N 4 -V 9

cp figures/*ver${NEWVER}* /group/halld/www/halldweb/html/data_monitoring/launch_analysis/
