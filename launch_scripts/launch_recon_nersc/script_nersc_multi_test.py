#!/usr/bin/env python3

# This script is called from the script-multi.sh script
# which just serves as a wrapper for this one. It wakes
# up in the top-level working directory for the job.
# It sets up the directories for each of the sub-jobs
# and then calls srun to launch those jobs on all of
# the remote nodes.

import os
import sys
import glob
import subprocess
import math

# This is the directory where the job scripts will be held.
# For the actual jobs, it will be the "launch" directory
# which has been checked out from the halld subversion
# repository. Here, we just point to the testing directory.
LAUNCH_DIR             = sys.argv[1]
SCRIPT_FILE            = sys.argv[2]
JANA_CONFIG            = sys.argv[3]
HALLD_VERSION_SET_XML  = sys.argv[4]
NMB_PROCESSES_PER_NODE = float(sys.argv[5])
NMB_TREADS_PER_PROCESS = float(sys.argv[6])

workdir = os.getcwd()

SLURM_JOB_NUM_NODES     = os.getenv('SLURM_JOB_NUM_NODES')  # number of nodes allocated for this job
SLURM_JOB_CPUS_PER_NODE = os.getenv('SLURM_JOB_CPUS_PER_NODE')  # number of CPUs available to the job on the allocated nodes

# Get list of raw data files
eviofiles = sorted(glob.glob('hd_rawdata_*.evio'))

# Calculate the expected number of nodes
expected_nodes = math.ceil(float(len(eviofiles)) / NMB_PROCESSES_PER_NODE) #Integer division

print(f"LAUNCH_DIR: {LAUNCH_DIR}")
print(f"SCRIPT_FILE: {SCRIPT_FILE}")
print(f"JANA_CONFIG: {JANA_CONFIG}")
print(f"HALLD_VERSION_SET_XML: {HALLD_VERSION_SET_XML}")
print(f"Nb of evio files: {len(eviofiles)}")
print(f"Nb of nodes: {expected_nodes}")
print(f"Nb of nodes asked: {SLURM_JOB_NUM_NODES}")

# Verify that the expected number of nodes matches the SLURM_JOB_NUM_NODES
if int(expected_nodes) != int(SLURM_JOB_NUM_NODES):
    print(f'MISMATCH IN NUMBER OF EVIO FILES PER NODE! #EVIO={len(eviofiles)}  '
          f'Expected Nodes={expected_nodes}  SLURM_JOB_NUM_NODES={SLURM_JOB_NUM_NODES}')
    sys.exit(101)

    # Verify that we have exactly one node per raw data file
    #if len(expected_nodes) != int(SLURM_JOB_NUM_NODES):
    #   print('MISMATCH IN NUMBER OF EVIO FILES AND SLURM NODES! #EVIO=%d  SLURM_JOB_NUM_NODES=%d' % (len(eviofiles), int(SLURM_JOB_NUM_NODES)))
    #   sys.exit(101)

# Loop over raw data files
for i,eviofile in enumerate(eviofiles):
    # Get RUN/FILE numbers from file names
    run = int(eviofile[11:17])
    fil = int(eviofile[18:21])

    # Make subjob directory
    #TODO why is this called for every file? we know how many files each job processes
    RUNDIR = f"RUN{int(run):06d}/FILE{int(float(fil) / NMB_PROCESSES_PER_NODE):03d}"
    os.makedirs( RUNDIR , exist_ok=True )

    # Make symlink pointing to subjobdir so the subjob
    # can cd into it via SLURM_NODEID
    #TODO why is this extra step needed? why not use FILE%03d directly in the subjob script? The only difference is the number of digits used
    subjobdir = f"subjob{int(float(fil) / NMB_PROCESSES_PER_NODE):04d}"
    if not os.path.exists ( subjobdir ) :
        os.symlink(RUNDIR, subjobdir)

    # Make symlink in subjobdir to evio file
    os.symlink( '../../' + eviofile, RUNDIR + '/' + eviofile )


# run all jobs
CMD = ['srun', '-n', SLURM_JOB_NUM_NODES, LAUNCH_DIR+'/run_shifter_multi.sh']
CMD += [workdir]                 # arg 1:  top-level directory for job
CMD += [SCRIPT_FILE]             # arg 2:  script to run inside shifter (all subsequent args are eventually passed to this script)
CMD += [JANA_CONFIG]             # arg 3:  JANA config file
CMD += [HALLD_VERSION_SET_XML]   # arg 4:  Hall-D version set XML file
CMD += [NMB_TREADS_PER_PROCESS]  # arg 5:  Number of threads of `hd_root` process
#         n.b. run/file are derived from evio file names. (see run_shifter_multi.sh)
print(f"Nb of nodes asked: {CMD}")
print(' '.join(CMD))
with subprocess.Popen(CMD, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as proc:
    outs,errs = proc.communicate()
    with open('std.out', 'wb') as f:
        f.write(outs)
        f.close()
    with open('std.err', 'wb') as f:
        f.write(errs)
        f.close()
    with open('exitcode', 'w') as f:
        f.write('%d' % proc.returncode)
        f.close()
