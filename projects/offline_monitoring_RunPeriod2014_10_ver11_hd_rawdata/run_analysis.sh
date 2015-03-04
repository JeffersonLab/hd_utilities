#!/bin/tcsh

# If we need to reset the table, use this
mysql -hhallddb -ufarmer farming -e "drop table offline_monitoring_RunPeriod2014_10_ver11_hd_rawdata_aux"
# Create _aux table that contains information about launch from stdout files
time mysql -hhallddb -ufarmer farming < ./create_offline_monAux.sql

# Fill in _aux table
# If running this part again, delete processed_job_info.txt
time ./get_processed_job_info_from_stdout.sh

time write_inserts_aux.pl < processed_job_info.txt | mysql -hhallddb -ufarmer farming

# Create txt file containing merged info from tables [project]Job and [project]_aux
time ./create_jobs_data_from_db.csh

# Format the txt file
time root -b -q format_jobs_data.C

# Run analysis to create plots
time root -b -q analyze.C

# Publish on web
# NOT YET!!!!!

