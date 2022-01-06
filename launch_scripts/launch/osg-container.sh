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
#
# Simplified version for use with markito3/gluex_docker_devel container
# Author: Mark Ito
# Version: February 28, 2018

userproxy=x509up_u$UID

# define the container context for running on osg workers

echo "Job running on" `hostname`
[ -r .$userproxy ] && mv .$userproxy /tmp/$userproxy
eval $*; retcode=$?
echo "Job finished with exit code" $retcode
exit $retcode	