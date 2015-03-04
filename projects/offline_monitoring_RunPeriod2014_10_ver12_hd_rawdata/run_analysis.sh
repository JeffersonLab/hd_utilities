#!/bin/tcsh

# Create _aux table that contains information about launch from stdout files
mysql -hhallddb -ufarmer farming < ./create_offline_monAux.sql

# Fill in _aux table
write_inserts_aux.pl < processed_job_info.txt | mysql -hhallddb -ufarmer farming

# Create txt file containing merged info from tables [project]Job and [project]_aux
./create_jobs_data_from_db.csh

# Format the txt file
root -b -q format_jobs_data.C

# Run analysis to create plots
root -b -q analyze.C

# Publish on web
# NOT YET!!!!!

