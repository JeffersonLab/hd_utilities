#!/usr/bin/env python3

"""
This is the main job script that processes all files of a given run.

Swif2 will make sure that the script starts in a working directory
that has links to all of the raw-data files of the run in it.  The
script divides the files into groups of `nmb_processes_per_task`
files, where each group is processed by a single slurm task.  Each
slurm task occupies a complete NERSC CPU node.  The script sets up the
directories for each of the slurm task.  Each directory holds links to
the input files and will receive the output files.  The script then
calls `srun` to launch the tasks on all nodes that were allocated by
the submit script.
"""

import argparse
import glob
import math
import os
import shlex
import socket
import subprocess
import sys


def write_env_to_file(output_file_name: str = "./env") -> None:
  """Writes environment variables to given file in alphabetical order."""
  with open(output_file_name, "w", encoding = "utf-8") as file:
    for env_var_name in sorted(os.environ.keys()):
      env_var_value = os.environ[env_var_name]
      file.write(f"{env_var_name}={shlex.quote(env_var_value)}\n")  # ensure bash-safe quoting of the value
  print(f"Wrote environment variables to '{output_file_name}'")


def main(args: argparse.Namespace) -> None:
  print("Running job script with arguments:")
  for arg_name, arg_value in sorted(vars(args).items()):  # sort keys for stable, tidy output
    print(f"{arg_name:>25} : {arg_value}")

  # get main job information
  write_env_to_file("job.env.out")
  host_name = socket.gethostname()  # name of compute node where this script is running
  with open("job.hostname.out", "w", encoding = "utf-8") as file:  #TODO is this really needed? the hostname is also available in the environment variable `SLURMD_NODENAME`
    file.write(host_name)
  work_dir_job = os.getcwd()  # working directory of job as created by swif2, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}; (identical to `${SWIF_JOB_STAGE_DIR}` and `${SWIF_JOB_WORK_DIR}`)
  print(f"Job script is running in directory: '{work_dir_job}'")
  evio_file_names = sorted(glob.glob("hd_rawdata_??????_???.evio"))  # list of raw data file names in working directory of job
  print(f"Found {len(evio_file_names)} EVIO files that will be processed by this job:")
  for index, evio_file_name in enumerate(evio_file_names):
    print(f"  {index:4d}: '{evio_file_name}'")
  nmb_tasks = os.getenv("SLURM_NTASKS")  # number of tasks allocated for this job = number of nodes  #TODO check for None
  print(f"Number of tasks allocated for this job: {nmb_tasks}")

  # ensure that nmb_tasks is consistent with number of evio files and nmb_processes_per_task
  expected_nmb_tasks = math.ceil(float(len(evio_file_names)) / args.nmb_processes_per_task)  # integer division  #TODO simplify
  if int(expected_nmb_tasks) != int(nmb_tasks):
    print(f"Error: mismatch of number of tasks = {expected_nmb_tasks} needed for {len(evio_file_names)} EVIO files and number of allocated slurm tasks = {nmb_tasks}")
    sys.exit(101)

  # loop over raw data files and create task directories with links to input files
  for evio_file_name in evio_file_names:
    # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
    run_number = int(evio_file_name[11:17])
    file_index = int(evio_file_name[18:21])
    node_index = int(float(file_index) / args.nmb_processes_per_task)
    # make work directory for task
    #TODO why is this called for every file? we know how many files each job processes
    work_dir_task = f"RUN{int(run_number):06d}/TASK{node_index:03d}"
    os.makedirs(work_dir_task, exist_ok = True)  #TODO are also created by `do_my_launch.sh`
    # create symlink to evio file in task directory
    os.symlink(f"../../{evio_file_name}", f"{work_dir_task}/{evio_file_name}")

  # run tasks in parallel
  # each task will run args.nmb_processes_per_task hd_root processes in parallel, each processing a single evio file)
  srun_cmd = [
    "srun",
    # f"--ntasks={nmb_tasks}",  # --ntasks is already specified in the `sbatch` command and srun will automatically use all allocated tasks
    # "--output=task-%x-%j-%t.out",  # write stdout and stderr of task to file named `task-<job name>-<job id>-<task id>.out` in working directory of task  #TODO use this instead of capturing the `srun` output below
    #TODO call shifter directly in srun command to avoid extra layer of indirection
    f"{args.launch_dir}/run_shifter_multi.sh",  # script to run as task
    work_dir_job,                       # arg 1:  top-level working directory of job
    args.script_file_task,              # arg 2:  script to run inside shifter (all subsequent args are eventually passed to this script)
    args.jana_config,                   # arg 3:  JANA config file
    args.jana_calib_context,            # arg 4:  JANA calibration context
    args.jana_geometry_url,             # arg 5:  JANA geometry URL
    args.halld_version_set_xml,         # arg 6:  Hall-D version set XML file
    str(args.nmb_threads_per_process),  # arg 7:  number of threads of `hd_root` process
  ]
  print(f"Executing command: '{' '.join(srun_cmd)}'")
  with subprocess.Popen(srun_cmd, stdout = subprocess.PIPE, stderr = subprocess.PIPE) as srun:
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
  parser.add_argument("--launch-dir",              dest = "launch_dir",              required = True, help = "Path to launch directory containing scripts and config files")
  parser.add_argument("--script-file-task",        dest = "script_file_task",        required = True, help = "Script file to run as task on each node")
  parser.add_argument("--jana-config",             dest = "jana_config",             required = True, help = "JANA config file")
  parser.add_argument("--jana-calib-context",      dest = "jana_calib_context",      required = True, help = "JANA calibration context")
  parser.add_argument("--jana-geometry-url",       dest = "jana_geometry_url",       required = True, help = "JANA geometry URL")
  parser.add_argument("--halld-version-set-xml",   dest = "halld_version_set_xml",   required = True, help = "Hall-D version set XML file")
  parser.add_argument("--nmb-processes-per-task",  dest = "nmb_processes_per_task",  required = True, help = "Number of processes per task",            type = int)
  parser.add_argument("--nmb-threads-per-process", dest = "nmb_threads_per_process", required = True, help = "Number of threads per `hd_root` process", type = int)
  main(parser.parse_args())
