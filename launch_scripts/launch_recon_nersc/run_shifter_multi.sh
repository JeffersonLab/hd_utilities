#!/usr/bin/env bash
set -o nounset  # exit if trying to use an uninitialized variable
set -o xtrace  # turn on command tracing

# This script will be run on the individual nodes
# to process a single file for multi-file jobs.
# It wakes up in the top-level working directory
# for the job and cd's into the directory for the
# file it is supposed to process based on the value
# of SLURM_NODEID which should be a number between
# 0 and Ntasks-1.
#
# The first argument passed to this script is the
# top-level working directory for the job. This should
# be the same as the directory we wake up in. This
# is used to cd to the working directory for the
# task (=file). It is removed from the arguments list
# so all other arguments can be passed to the shifter
# command.
#

# The actual working directory for this job will be
# named something like RUNXXXXXX/NODEYYY. However, we
# do not know the run/file numbers at this point, only
# the SLURM_NODEID can be used to distinguish us from
# other tasks. The script_nersc_multi.py script though
# created a symbolic link named subjobZZZZ (where the
# ZZZZ is the SLURM_NODEID) that points to the directory
# we should use.
subjobdir=$(printf "subjob%04d" ${SLURM_NODEID})
cd "${1}/${subjobdir}"


# # The run/file numbers should be passed as the last 2 arguments
# # to the shifter command so the script it runs can use them
# # to name the files. Here we extract them from the evio file
# # name. This assumes there is a file with a name like:
# # hd_rawdata_XXXXXX_YYY.evio.
# RUNNO=999999  # fallback that can be used for debugging
# FILENO=999    # fallback that can be used for debugging
# for file in *.evio
# do
# 	# First 2 lines extract strings from filename. Second
# 	# two lines strip leading 0's so script_nersc.sh does
# 	# not interpret them as octal numbers.
# 	runstr="${file:11:6}"
# 	filestr="${file:18:3}"
# 	RUNNO=$((10#$runstr))
# 	FILENO=$((10#$filestr))
# done

# The following removes the first argument passed to
# this script which is the top-level working directory
# for the job.
shift

# Pass all remaining arguments to shifter, including run/file numbers
shifter "${@}"
