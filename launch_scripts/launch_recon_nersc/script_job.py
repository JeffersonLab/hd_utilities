#!/usr/bin/env python3

"""
Main job script that processes all EVIO files of the given run.

Swif2 ensures that the script starts in a working directory containing
symbolic links to all of the EVIO input files for the given run.  The
script then groups these files into sets of `nmb_processes_per_task`
files, with each group assigned to a single Slurm task.  Each Slurm
task occupies an entire NERSC CPU node.  For each of the Slurm tasks,
the script sets up a dedicated task working directory
`RUNXXXXXX/TASKYYY`, where XXXXXX is the 6-digit run number and YYY is
the 3-digit task number.  Within the task directory, the script
creates symbolic links to all `nmb_processes_per_task` EVIO input
files the task will process.  The script then invokes `srun` to launch
the task script `script_task.sh` across all nodes allocated by the
submit script.  Finally, the script defines all output files that
should be transferred back to JLab using `swif2 output` commands.
"""

from __future__ import annotations

import argparse
import functools
import glob
import os
import shlex
import subprocess
import sys
import time

from utilities import (
  define_swif2_output_files,
  print_command_line_arguments,
  print_python_env,
)


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
  start_time = time.time()
  print_python_env()
  print_command_line_arguments(args)

  # gather information about job environment and write it to files
  run_label = f"{args.run_number:06d}"
  write_env_to_file(f"job_{run_label}.env")
  for log_file_suffix, log_cmd in (
    ("hostname",  "hostnamectl"),
    # ("diskquota", "myquota --verbose --full-path --limit --common --hpss"),  # does not work on NERSC nodes: /usr/lpp/mmfs/bin/mmlsquota is missing
    ("mounts",    "findmnt --canonicalize --output=TARGET,FSTYPE,SIZE,USED,AVAIL,USE%"),
  ):
    with open(f"job_{run_label}.{log_file_suffix}", "w", encoding = "utf-8") as log_file:
      subprocess.run(log_cmd, shell = True, check = False, stdout = log_file, stderr = subprocess.STDOUT)

  # get job working directory and list of input raw-data files
  work_dir_job = os.getcwd()  # working directory of job as created by swif2, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}; (identical to `${SWIF_JOB_STAGE_DIR}` and `${SWIF_JOB_WORK_DIR}`)
  print(f"Job script is running in directory: '{work_dir_job}'")
  evio_file_names: list[str] = sorted(glob.glob("hd_rawdata_??????_???.evio"))  # list of raw-data file names in working directory of job
  #TODO filter bad files if list is available?
  print(f"Found {len(evio_file_names)} EVIO files that will be processed by this job:")
  for index, evio_file_name in enumerate(evio_file_names):
    print(f"  {index:4d}: '{evio_file_name}'")
  nmb_tasks = os.getenv("SLURM_NTASKS")  # number of tasks allocated for this job = number of nodes
  assert nmb_tasks is not None, "Error: environment variable 'SLURM_NTASKS' is required but not set"
  print(f"Number of tasks allocated for this job: {nmb_tasks}")

  # ensure that nmb_tasks is consistent with number of evio files and nmb_processes_per_task
  expected_nmb_tasks = (len(evio_file_names) + args.nmb_processes_per_task - 1) // args.nmb_processes_per_task
  if expected_nmb_tasks != int(nmb_tasks):
    print(f"Error: mismatch of number of tasks = {expected_nmb_tasks} needed for {len(evio_file_names)} EVIO files and number of allocated Slurm tasks = {nmb_tasks}")
    sys.exit(101)

  # loop over tasks, create task directories, and assign args.nmb_processes_per_task EVIO files to each task by linking them into the task's directory
  #TODO move chopping of EVIO file list into separate function
  for task_index in range(int(nmb_tasks)):
    work_dir_task = f"RUN{run_label}/TASK{task_index:03d}"
    print(f"Creating working directory for task {task_index}: '{work_dir_task}'")
    os.makedirs(work_dir_task, exist_ok = True)
    file_index_start = task_index * args.nmb_processes_per_task
    file_index_end   = min(file_index_start + args.nmb_processes_per_task, len(evio_file_names))
    for evio_file_name in evio_file_names[file_index_start:file_index_end]:
      print(f"Linking EVIO file '{evio_file_name}' into task directory '{work_dir_task}'")
      os.symlink(f"../../{evio_file_name}", f"{work_dir_task}/{evio_file_name}")

  print("-------------------------------------------------------------------------------")
  # each task will run args.nmb_processes_per_task hd_root processes in parallel, each processing a single EVIO file using args.nmb_threads_per_process threads
  task_cmd: list[str] = [
    f"{args.launch_dir}/script_task.sh",  # task script to run inside a container on each NERSC node (all subsequent arguments are passed to this script)
    # required arguments
    f"{args.run_number}",               # arg 1:  Run number for this task
    f"{args.jana_config}",              # arg 2:  JANA config file
    f"{args.jana_calib_context}",       # arg 3:  JANA calibration context
    f"{args.halld_version_set_xml}",    # arg 4:  GlueX software version set XML file
    f"{args.nmb_threads_per_process}",  # arg 5:  number of threads each `hd_root` process should use
  ]
  # arg 6: optional JANA geometry URL override
  if args.jana_geometry_url_override is not None:
    task_cmd.append(f"{args.jana_geometry_url_override}")

  srun_cmd: list[str] = [
    "srun",
    # f"--ntasks={nmb_tasks}",  # --ntasks is already specified in the `sbatch` command and srun will automatically use all allocated tasks
    "--kill-on-bad-exit=0",  # do not kill all tasks if one task fails; instead, let all tasks run to completion to capture their individual return codes
    # "--slurmd-debug=verbose",  # increase verbosity of slurmd debug output; this will be written to the Slurm log file of the job; only allowed for root
    "--verbose",  # this will print the exact command line that srun executes for each task
    f"--output=task_{run_label}_%t.out",  # write stdout and stderr of task to file named `task_<run number>_<task id>.out` into job's working directory
    "--",
    "/usr/bin/shifter",  #TODO use absolute path to work around spurious error `slurmstepd: error: execve(): shifter: No such file or directory` leading to `srun` return code 2
    "--",
    "bash",
    "-c",
    " ".join(task_cmd),  # command to run in container; needs to be passed as a single string to `bash -c`
  ]
  print(f"Submitting {nmb_tasks} tasks: {' '.join(srun_cmd)}")
  srun_result = subprocess.run(srun_cmd, check = False)
  with open(f"./srun_{run_label}.rc", "w", encoding = "utf-8") as log_file:
    log_file.write(f"{srun_result.returncode:d}")
  print(f"`srun` finished with return code {srun_result.returncode:d}")
  # `srun` will return i) a non-zero Slurm return code, if it cannot
  # start the tasks, or ii) the highest return code of any failed
  # tasks. If a task is killed by signal, 128 + signal number is
  # returned.

  print("-------------------------------------------------------------------------------")
  #TODO move part of post-processing of output files here
  #     leave only processing of the log files to the script run at JLab
  define_swif2_output_files(args.run_number, args.swif_output_root)

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")
  sys.exit(srun_result.returncode)  # forward return code of srun to the caller of this script, i.e. swif2


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Main job script that processes all EVIO files of the given run.",
  )
  parser.add_argument("--run_number",                 required = True,  help = "Run number for this job", type = int)
  parser.add_argument("--launch_dir",                 required = True,  help = "Path to launch directory containing scripts and config files inside container")
  parser.add_argument("--jana_config",                required = True,  help = "JANA config file")
  parser.add_argument("--jana_calib_context",         required = True,  help = "JANA calibration context")
  parser.add_argument("--jana_geometry_url_override", required = False, help = "Override JANA geometry URL; optional")
  parser.add_argument("--halld_version_set_xml",      required = True,  help = "GlueX software version set XML file")
  parser.add_argument("--nmb_processes_per_task",     required = True,  help = "Number of processes per task",            type = int)
  parser.add_argument("--nmb_threads_per_process",    required = True,  help = "Number of threads per `hd_root` process", type = int)
  parser.add_argument("--swif_output_root",           required = True,  help = "Root of JLab directory tree, where output files will be copied to")
  #TODO add --dry_run flag
  main(parser.parse_args())
