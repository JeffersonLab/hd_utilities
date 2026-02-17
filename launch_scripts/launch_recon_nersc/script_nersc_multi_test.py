#!/usr/bin/env python3

"""
This job script is called from the script-multi.sh script which just
serves as a wrapper for this one. It wakes up in the top-level working
directory for the job, sets up the directories for each of the
slurm task, and then calls `srun` to launch the tasks on all nodes.
"""

import argparse
import glob
import math
import os
import subprocess
import sys


def main(args: argparse.Namespace) -> None:

  work_dir_job = os.getcwd()  # directory where the job scripts are held; for the actual jobs, it is the "launch" directory  #TODO is this really true? doesn't swif2 define the working directory for the job?

  SLURM_JOB_NUM_NODES = os.getenv("SLURM_JOB_NUM_NODES")  # number of nodes allocated for this job
  evio_file_names = sorted(glob.glob("hd_rawdata_*.evio"))  # list of raw data file names
  # calculate the expected number of nodes
  expected_nmb_nodes = math.ceil(float(len(evio_file_names)) / args.nmb_processes_per_node)  # integer division

  #TODO use a more elegant way to print the arguments
  print(f"LAUNCH_DIR: {args.launch_dir}")
  print(f"SCRIPT_FILE: {args.task_script_file}")
  print(f"JANA_CONFIG: {args.jana_config}")
  print(f"HALLD_VERSION_SET_XML: {args.halld_version_set_xml}")
  print(f"NMB_PROCESSES_PER_NODE: {args.nmb_processes_per_node}")
  print(f"NMB_THREADS_PER_PROCESS: {args.nmb_threads_per_process}")
  print(f"Nb of evio files: {len(evio_file_names)}")
  print(f"Nb of nodes: {expected_nmb_nodes}")
  print(f"Nb of nodes asked: {SLURM_JOB_NUM_NODES}")

  # verify that the expected number of nodes matches the SLURM_JOB_NUM_NODES
  if int(expected_nmb_nodes) != int(SLURM_JOB_NUM_NODES):
    print(f"MISMATCH IN NUMBER OF EVIO FILES PER NODE! #EVIO={len(evio_file_names)}  "
      f"Expected Nodes={expected_nmb_nodes}  SLURM_JOB_NUM_NODES={SLURM_JOB_NUM_NODES}")
    sys.exit(101)

  # Loop over raw data files
  for evio_file_name in evio_file_names:
    # get RUN/FILE numbers from file names
    run  = int(evio_file_name[11:17])
    file = int(evio_file_name[18:21])

    # make subjob directory
    #TODO why is this called for every file? we know how many files each job processes
    run_dir_name = f"RUN{int(run):06d}/FILE{int(float(file) / args.nmb_processes_per_node):03d}"
    os.makedirs(run_dir_name, exist_ok = True)

    # make symlink pointing to subjobdir so the subjob
    # can cd into it via SLURM_NODEID
    #TODO why is this extra step needed? why not use FILE%03d directly in the subjob script? The only difference is the number of digits used
    subjob_dir_name = f"subjob{int(float(file) / args.nmb_processes_per_node):04d}"
    if not os.path.exists(subjob_dir_name):
      os.symlink(run_dir_name, subjob_dir_name)

    # make symlink in subjob directory to evio file
    os.symlink(f"../../{evio_file_name}", f"{run_dir_name}/{evio_file_name}")


  # run one task per node
  # each task will run args.nmb_processes_per_node hd_root processes in parallel, each processing a single evio file)
  command = [
    "srun",
    f"--ntasks={SLURM_JOB_NUM_NODES}",  # 1 task per node
    # "--output=task-%x-%j-%t.out",  # write stdout and stderr of task to file named task-<job name>-<job id>-<task id>.out in working directory of task
    f"{args.launch_dir}/run_shifter_multi.sh",  # script to run as task
    work_dir_job,                  # arg 1:  top-level directory of job
    args.task_script_file,         # arg 2:  script to run inside shifter (all subsequent args are eventually passed to this script)
    args.jana_config,              # arg 3:  JANA config file
    args.halld_version_set_xml,    # arg 4:  Hall-D version set XML file
    args.nmb_threads_per_process,  # arg 5:  number of threads of `hd_root` process
  ]
  # n.b. run/file are derived from evio file names. (see run_shifter_multi.sh)
  print(f"Nb of nodes asked: {command}")
  print(" ".join(command))
  with subprocess.Popen(command, stdout = subprocess.PIPE, stderr = subprocess.PIPE) as srun:
    stdout, stderr = srun.communicate()
    #TODO is this meaningful? wouldn't it be better to capture the output of each task separately using `srun --output`? the exit code also does not seem to be used
    with open("./std.out", "wb") as f:
      f.write(stdout)
    with open("./std.err", "wb") as f:
      f.write(stderr)
    with open("./exitcode", "w") as f:
      f.write(f"{srun.returncode:d}")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Prepare directory structure and use srun to start a reconstruction task on each node (positional args).",
  )
  parser.add_argument("launch_dir",              help = "Path to launch directory containing scripts and config files")
  parser.add_argument("task_script_file",        help = "Script file to run as task on each node")
  parser.add_argument("jana_config",             help = "JANA config file")
  parser.add_argument("halld_version_set_xml",   help = "Hall-D version set XML file")
  parser.add_argument("nmb_processes_per_node",  help = "Number of processes per node",            type = int)
  parser.add_argument("nmb_threads_per_process", help = "Number of threads per `hd_root` process", type = int)
  args = parser.parse_args()
  main(args)

  #TODO use named arguments
  # parser.add_argument("--launch-dir",              dest = "launch_dir",              required = True, help = "Path to launch directory containing scripts and config files")
  # parser.add_argument("--task-script-file",        dest = "task_script_file",        required = True, help = "Script file to run as task on each node")
  # parser.add_argument("--jana-config",             dest = "jana_config",             required = True, help = "JANA config file")
  # parser.add_argument("--halld-version-set-xml",   dest = "halld_version_set_xml",   required = True, help = "Hall-D version set XML file")
  # parser.add_argument("--nmb-processes-per-node",  dest = "nmb_processes_per_node",  required = True, help = "Number of processes per node",            type = int)
  # parser.add_argument("--nmb-threads-per-process", dest = "nmb_threads_per_process", required = True, help = "Number of threads per `hd_root` process", type = int)
