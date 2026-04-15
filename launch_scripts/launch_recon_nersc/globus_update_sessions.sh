#!/usr/bin/env bash

# updates Globus sessions for NERSC and JLab endpoints


echo "Updating Globus sessions for NERSC endpoint"
globus session update nersc.gov clients.auth.globus.org

echo "Updating Globus sessions for JLab endpoint"
globus session update jlab.org

echo "Done updating Globus sessions; run 'globus_check.sh' to verify that this was successful"
