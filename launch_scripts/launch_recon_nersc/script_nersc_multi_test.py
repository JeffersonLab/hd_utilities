#!/usr/bin/python3

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
LAUNCHDIR    = sys.argv[1]
SCRIPTFILE   = sys.argv[2]
CONFIG       = sys.argv[3]
RECONVERSION = sys.argv[4]
SLURM_JOBS_PER_NODE = float(sys.argv[5])

workdir = os.getcwd()

SLURM_JOB_NUM_NODES     = os.getenv('SLURM_JOB_NUM_NODES')
SLURM_JOB_CPUS_PER_NODE = os.getenv('SLURM_JOB_CPUS_PER_NODE')

# Get list of raw data files
eviofiles = sorted(glob.glob('hd_rawdata_*.evio'))

# Calculate the expected number of nodes
expected_nodes = math.ceil(float(len(eviofiles)) / SLURM_JOBS_PER_NODE) #Integer division

print(f"LAUNCHDIR: {LAUNCHDIR}")
print(f"SCRIPTFILE: {SCRIPTFILE}")
print(f"CONFIG: {CONFIG}")
print(f"RECONVERSION: {RECONVERSION}")
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
    RUNDIR = 'RUN%06d/FILE%03d' % (int(run), int(float(fil) / SLURM_JOBS_PER_NODE))
    os.makedirs( RUNDIR , exist_ok=True )
    
    # Make symlink pointing to subjobdir so the subjob
    # can cd into it via SLURM_NODEID
    subjobdir = 'subjob%04d' % int( float(fil) / SLURM_JOBS_PER_NODE)
    if not os.path.exists ( subjobdir ) :
        os.symlink(RUNDIR, subjobdir)
    
    # Make symlink in subjobdir to evio file
    os.symlink( '../../' + eviofile, RUNDIR + '/' + eviofile )
        
         
# run all jobs
CMD = ['srun', '-n', SLURM_JOB_NUM_NODES, LAUNCHDIR+'/run_shifter_multi.sh']
CMD += [workdir]          # arg 1:  top-level directory for job
CMD += [SCRIPTFILE]       # arg 2:  script to run inside shifter (all subsequent args are eventually passed to this script)
CMD += [CONFIG]           # arg 3:  JANA config file
CMD += [RECONVERSION]     # arg 4:  sim-recon version
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
                
