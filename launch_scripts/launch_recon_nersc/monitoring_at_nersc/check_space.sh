#!/usr/bin/env bash

# Checks the disk space usage and prints the quota for the user account.


echo "NERSC Scratch Storage Space used by swif2"
du -hs /pscratch/sd/j/jlab/swif
echo
echo "NERSC Community File System (CFS) Storage Space used"
du -hs /global/cfs/cdirs/m3120
echo
echo "Quota for user 'jlab'"
myquota
