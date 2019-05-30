#!/bin/sh

# Simple script to run the regenerate_plots.csh script
# every 10 minutes.
# (Didn't want something as permanent as cronjob)

while ( true ); do

 ./regenerate_plots.csh -p -N -w recon_2018-01_ver02_batch05
 sleep 500 

done

