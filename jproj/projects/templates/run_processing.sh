#!/bin/tcsh

source ./monitoring_env.csh

# Generate new version number
./register_new_version.py add version_file.txt

# Create plots
./check_monitoring_data.csh
