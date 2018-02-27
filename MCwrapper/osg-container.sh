#!/bin/sh
#
# osg-container.sh - gluex job wrapper script for osg jobs
#
# This script contains GENERIC (non-job-specific) steps for getting
# set up inside the gluex container on an osg worker node and launching
# the job script. It should not normally be modified by the user.
#
# Usage: osg-container.sh <job script> [job script arguments]
#
# Author: Richard.T.Jones at uconn.edu
# Version: June 8, 2017

container="/cvmfs/singularity.opensciencegrid.org/markito3/gluex_docker_devel:latest"
#oasismount="/cvmfs/oasis.opensciencegrid.org"
oasismount="/cvmfs/oasis.opensciencegrid.org/gluex/group:/group"
dockerimage="docker://rjones30/gluex:latest"
userproxy=x509up_u$UID

# define the container context for running on osg workers

if [[ -f /environment ]]; then
    echo "Job running on" `hostname`
    [ -r .$userproxy ] && mv .$userproxy /tmp/$userproxy
    source /environment
    unset CCDB_CONNECTION
    unset RCDB_CONNECTION
    eval $*; retcode=$?
    echo "Job finished with exit code" $retcode
    rm -rf *.sqlite
    exit $retcode

elif [[ -f $container/environment ]]; then
    echo "Starting up container on" `hostname`
    [ -r /tmp/$userproxy ] && cp /tmp/$userproxy .$userproxy
    exec singularity exec --containall --bind ${oasismount} --home `pwd`:/srv --pwd /srv --scratch /tmp,/var/tmp ${container} \
    bash $0 $*

else
    echo "Job container not found on" `hostname`
fi
