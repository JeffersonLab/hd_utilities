#!/usr/bin/env python3

"""
Main job script that processes all files of a given run.

Swif2 will make sure that the script starts in a working directory
that has links to all of the raw-data files of the run in it.  The
script divides the files into groups of `nmb_processes_per_task`
files, where each group is processed by a single slurm task.  Each
slurm task occupies a complete NERSC CPU node.  The script sets up the
directories for each of the slurm task.  Each directory holds links to
the input files and will receive the output files.  The script then
calls `srun` to launch the task script `script_task.sh` on all nodes
that were allocated by the submit script.
"""

import argparse
import functools
import glob
import math
import os
import shlex
import shutil
import subprocess
import sys


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def write_env_to_file(output_file_name: str = "./env") -> None:
  """Writes environment variables in alphabetical order into given file."""
  with open(output_file_name, "w", encoding = "utf-8") as file:
    for env_var_name in sorted(os.environ.keys()):
      env_var_value = os.environ[env_var_name]
      file.write(f"{env_var_name}={shlex.quote(env_var_value)}\n")  # ensure bash-safe quoting of the value
  print(f"Wrote environment variables to '{output_file_name}'")


def main(args: argparse.Namespace) -> None:
  print("Running job script with arguments:")
  max_arg_name_length = max(len(arg_name) for arg_name in vars(args).keys())
  for arg_name, arg_value in sorted(vars(args).items()):  # sort keys for stable, tidy output
    print(f"{arg_name:>{max_arg_name_length + 2}} : {arg_value}")

  # gather information about job environment and write it to files
  run_label = f"{args.run_number:06d}"
  write_env_to_file(f"job_{run_label}.env")
  with open(f"job_{run_label}.hostname", "w", encoding = "utf-8") as file:
    subprocess.run(["hostnamectl"], stdout = file, stderr = subprocess.STDOUT)

  # get job working directory and list of input files
  work_dir_job = os.getcwd()  # working directory of job as created by swif2, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}; (identical to `${SWIF_JOB_STAGE_DIR}` and `${SWIF_JOB_WORK_DIR}`)
  print(f"Job script is running in directory: '{work_dir_job}'")
  evio_file_names = sorted(glob.glob("hd_rawdata_??????_???.evio"))  # list of raw data file names in working directory of job
  print(f"Found {len(evio_file_names)} EVIO files that will be processed by this job:")
  for index, evio_file_name in enumerate(evio_file_names):
    print(f"  {index:4d}: '{evio_file_name}'")
  nmb_tasks = os.getenv("SLURM_NTASKS")  # number of tasks allocated for this job = number of nodes
  assert nmb_tasks is not None, "Error: environment variable 'SLURM_NTASKS' is required but not set"
  print(f"Number of tasks allocated for this job: {nmb_tasks}")

  # ensure that nmb_tasks is consistent with number of evio files and nmb_processes_per_task
  expected_nmb_tasks = math.ceil(float(len(evio_file_names)) / args.nmb_processes_per_task)  # integer division  #TODO simplify
  if int(expected_nmb_tasks) != int(nmb_tasks):
    print(f"Error: mismatch of number of tasks = {expected_nmb_tasks} needed for {len(evio_file_names)} EVIO files and number of allocated slurm tasks = {nmb_tasks}")
    sys.exit(101)

  # loop over raw data files and create task directories with links to input files
  for evio_file_name in evio_file_names:
    # get run and file numbers from EVIO file names; assumes file names are of the form `hd_rawdata_XXXXXX_YYY.evio`
    # run_number = int(evio_file_name[11:17])
    file_index = int(evio_file_name[18:21])
    node_index = int(float(file_index) / args.nmb_processes_per_task)
    # make work directory for task
    #TODO why is this called for every file? we know how many files each job processes
    work_dir_task = f"RUN{run_label}/TASK{node_index:03d}"
    os.makedirs(work_dir_task, exist_ok = True)
    # create symlink to evio file in task directory
    os.symlink(f"../../{evio_file_name}", f"{work_dir_task}/{evio_file_name}")

  # debug error `slurmstepd: error: execve(): shifter: No such file or directory` leading to `srun` return code 2
  for debug_cmd in (
    "which shifter",
    "ls -l /usr/bin/shifter",
    "shifter",
    "bash -c shifter",
  ):
    print(f"Running debug command: '{debug_cmd}'")
    subprocess.run(debug_cmd, shell = True, check = False)
  print(f"{shutil.which('shifter')=}")
  # run tasks in parallel
  # each task will run args.nmb_processes_per_task hd_root processes in parallel, each processing a single evio file
  task_cmd = [
    f"/{args.launch_dir}/script_task.sh",  # task script to run inside a container on each NERSC node (all subsequent arguments are passed to this script)
    f"{args.run_number}",               # arg 1:  Run number for this task
    f"{args.jana_config}",              # arg 2:  JANA config file
    f"{args.jana_calib_context}",       # arg 3:  JANA calibration context
    f"{args.jana_geometry_url}",        # arg 4:  JANA geometry URL
    f"{args.halld_version_set_xml}",    # arg 5:  GlueX software version set XML file
    f"{args.nmb_threads_per_process}",  # arg 6:  number of threads each `hd_root` process should use
  ]
  srun_cmd = [
    "srun",
    # f"--ntasks={nmb_tasks}",  # --ntasks is already specified in the `sbatch` command and srun will automatically use all allocated tasks
    "--kill-on-bad-exit=0",  # do not kill all tasks if one task fails; instead, let all tasks run to completion to capture their individual exit codes
    # "--slurmd-debug=verbose",  # increase verbosity of slurmd debug output; this will be written to the slurm log file of the job; only allowed for root
    "--verbose",  # this will print the exact command line that srun executes for each task
    f"--output=task_{run_label}_%t.out",  # write stdout and stderr of task to file named `task_<run number>_<task id>.out` into job's working directory
    "--",
    "/usr/bin/shifter",
    "--",
    "bash",
    "-c",
    " ".join(task_cmd),  # command to run in container; needs to be passed as a single string to `bash -c`
  ]
  print(f"Submitting tasks: '{' '.join(srun_cmd)}'")
  srun_result = subprocess.run(srun_cmd, check = False)
  with open(f"./srun_{run_label}.rc", "w", encoding = "utf-8") as file:
    file.write(f"{srun_result.returncode:d}")
  print(f"srun finished with return code {srun_result.returncode:d}")
  sys.exit(srun_result.returncode)  # forward return code of srun to the caller of this script, i.e. swif2
  # `srun` will return i) a non-zero slurm exit code, if it cannot
  # start the tasks, or ii) the highest exit code of any failed tasks.
  # If a task is killed by signal, 128 + signal number is returned.


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Prepare directory structure and use srun to start a reconstruction task on each node (positional args).",
  )
  parser.add_argument("--run-number",              dest = "run_number",              required = True, help = "Run number for this job", type = int)
  parser.add_argument("--launch-dir",              dest = "launch_dir",              required = True, help = "Path to launch directory containing scripts and config files inside container")
  parser.add_argument("--jana-config",             dest = "jana_config",             required = True, help = "JANA config file")
  parser.add_argument("--jana-calib-context",      dest = "jana_calib_context",      required = True, help = "JANA calibration context")
  parser.add_argument("--jana-geometry-url",       dest = "jana_geometry_url",       required = True, help = "JANA geometry URL")
  parser.add_argument("--halld-version-set-xml",   dest = "halld_version_set_xml",   required = True, help = "GlueX software version set XML file")
  parser.add_argument("--nmb-processes-per-task",  dest = "nmb_processes_per_task",  required = True, help = "Number of processes per task",            type = int)
  parser.add_argument("--nmb-threads-per-process", dest = "nmb_threads_per_process", required = True, help = "Number of threads per `hd_root` process", type = int)
  main(parser.parse_args())
