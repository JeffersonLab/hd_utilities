#!/usr/bin/env python3
#NOTE this script needs to be compatible with Python 3.6

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
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
from typing import List, Tuple, Optional


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)

#TODO find out whether this can be moved to the utilities module
def print_command_line_arguments(args: argparse.Namespace) -> None:
  """Print all command-line arguments and their values and the git hash."""
  this_script_file_name = os.path.basename(sys.argv[0])  # get file name of script that was launched
  print("-------------------------------------------------------------------------------")
  print(f"Running script {this_script_file_name} with arguments:")
  max_arg_name_length = max(len(arg_name) for arg_name in vars(args).keys())
  for arg_name, arg_value in sorted(vars(args).items()):  # sort keys for stable, tidy output
    print(f"{arg_name:>{max_arg_name_length + 2}} : {arg_value}")
  this_script_dir = os.path.dirname(os.path.abspath(__file__))
  if os.path.isfile(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH"):
    deployed_git_hash = open(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH", "r", encoding = "utf-8").read().strip()
    print(f"Using launch scripts from git commit hash: {deployed_git_hash}")
  #TODO add case where the script is run from the git repo
  print("-------------------------------------------------------------------------------")


def write_env_to_file(output_file_name: str = "./env") -> None:
  """Writes environment variables in alphabetical order into given file."""
  with open(output_file_name, "w", encoding = "utf-8") as file:
    for env_var_name in sorted(os.environ.keys()):
      env_var_value = os.environ[env_var_name]
      file.write(f"{env_var_name}={shlex.quote(env_var_value)}\n")  # ensure bash-safe quoting of the value
  print(f"Wrote environment variables to '{output_file_name}'")


def get_hd_root_return_code(hd_root_rc_file_path: str) -> Optional[int]:
  """Read the hd_root return code from the given file and return it as an int or `None` if unsucessful."""
  try:
    with open(hd_root_rc_file_path, "r", encoding="utf-8") as file:
      hd_root_rc_file_content = file.read()
  except OSError as e:  # file not found, permission denied, etc.
    print(f"WARNING: unable to open hd_root return-code file '{hd_root_rc_file_path}': {e}")
    return None
  match = re.search(r"exit code (\d+)$", hd_root_rc_file_content)
  if not match:
    print(f"WARNING: cannot get hd_root return code from malformed file '{hd_root_rc_file_path}': '{hd_root_rc_file_content}'")
    return None
  return int(match.group(1))


def get_output_file_paths(
  run_number:       int,
  swif_output_root: str,
) -> List[Tuple[str, str]]:
  """Get list with local relative paths w.r.t. current directory and absolute remote destination paths of all output files that should be transferred back to JLab"""
  # this function assumes that the current directory is the working directory of the job
  # first greedily collect all potential output items, then filter out directories and files that should not be transferred back to JLab
  relative_output_paths: List[str] = []
  relative_output_paths += sorted(glob.glob(f"*"))  # include all items in job dir
  relative_output_paths += sorted(glob.glob(f"RUN{run_number:06d}/*"))  # include all items in run dir
  # loop over task dirs
  task_dirs: List[str] = sorted(glob.glob(f"RUN{run_number:06d}/TASK???"))
  for task_dir in task_dirs:
    relative_output_paths += sorted(glob.glob(f"{task_dir}/*"))  # include all items in task dir
    # loop over file dirs
    file_dirs: List[str] = sorted(glob.glob(f"{task_dir}/FILE???"))
    for file_dir in file_dirs:
      hd_root_return_code = get_hd_root_return_code(f"{file_dir}/hd_root.rc")
      if hd_root_return_code is None or hd_root_return_code != 0:
        # do not copy hd_root output files for failed hd_root processes; but try to recover debug info
        print(f"WARNING: skipping hd_root output files in '{file_dir}' because hd_root return code is {hd_root_return_code} != 0")
        relative_output_paths += [f"{file_dir}/hd_root.err", f"{file_dir}/hd_root.out", f"{file_dir}/hd_root.rc"]  # transfer hd_root log files for debugging
        relative_output_paths += sorted(glob.glob(f"{file_dir}/core.hd_root*"))  # transfer core files for debugging
        continue
      relative_output_paths += sorted(glob.glob(f"{file_dir}/*"))  # include all items in file dir
  output_file_paths: List[Tuple[str, str]] = []  # list of pairs of relative local file paths and absolute remote destination file paths
  for relative_output_path in relative_output_paths:
    # filter out all directories and nonexisting and unwanted and files
    if not os.path.isfile(relative_output_path) or os.path.islink(relative_output_path):  # keep only existing, non-symlinked files
      continue
    output_file_name = os.path.basename(relative_output_path)
    if output_file_name.startswith(".") or output_file_name.startswith("__"):  # skip hidden files and swif2 system files
      continue
    if re.fullmatch(r"hd_rawdata_\d{6}_\d{3}\.evio", output_file_name):  # skip raw-data files that match hd_rawdata_XXXXXX_YYY.evio, with 6-digit run number XXXXXX and 3-digit file number YYY
      continue
    output_file_paths.append((relative_output_path, f"{swif_output_root}/{relative_output_path}"))
  return output_file_paths


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  # gather information about job environment and write it to files
  run_label = f"{args.run_number:06d}"
  write_env_to_file(f"job_{run_label}.env")
  for log_file_suffix, log_cmd in (
    ("hostname",  "hostnamectl"),
    ("diskquota", "myquota --verbose --full-path --limit --common --hpss"),  #TODO this does not seem to work NERSC nodes
    ("mounts",    "findmnt --canonicalize --output=TARGET,FSTYPE,SIZE,USED,AVAIL,USE%"),
  ):
    with open(f"job_{run_label}.{log_file_suffix}", "w", encoding = "utf-8") as log_file:
      subprocess.run(log_cmd, shell = True, check = False, stdout = log_file, stderr = subprocess.STDOUT)

  # get job working directory and list of input raw-data files
  work_dir_job = os.getcwd()  # working directory of job as created by swif2, i.e. `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}; (identical to `${SWIF_JOB_STAGE_DIR}` and `${SWIF_JOB_WORK_DIR}`)
  print(f"Job script is running in directory: '{work_dir_job}'")
  evio_file_names: List[str] = sorted(glob.glob("hd_rawdata_??????_???.evio"))  # list of raw-data file names in working directory of job
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
    print(f"Error: mismatch of number of tasks = {expected_nmb_tasks} needed for {len(evio_file_names)} EVIO files and number of allocated slurm tasks = {nmb_tasks}")
    sys.exit(101)

  # loop over tasks, create task directories, and assign args.nmb_processes_per_task EVIO files to each task by linking them into the task's directory
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
  # each task will run args.nmb_processes_per_task hd_root processes in parallel, each processing a single evio file using args.nmb_threads_per_process threads
  task_cmd: List[str] = [
    f"{args.launch_dir}/script_task.sh",  # task script to run inside a container on each NERSC node (all subsequent arguments are passed to this script)
    f"{args.run_number}",               # arg 1:  Run number for this task
    f"{args.jana_config}",              # arg 2:  JANA config file
    f"{args.jana_calib_context}",       # arg 3:  JANA calibration context
    f"{args.jana_geometry_url}",        # arg 4:  JANA geometry URL
    f"{args.halld_version_set_xml}",    # arg 5:  GlueX software version set XML file
    f"{args.nmb_threads_per_process}",  # arg 6:  number of threads each `hd_root` process should use
  ]
  srun_cmd: List[str] = [
    "srun",
    # f"--ntasks={nmb_tasks}",  # --ntasks is already specified in the `sbatch` command and srun will automatically use all allocated tasks
    "--kill-on-bad-exit=0",  # do not kill all tasks if one task fails; instead, let all tasks run to completion to capture their individual exit codes
    # "--slurmd-debug=verbose",  # increase verbosity of slurmd debug output; this will be written to the slurm log file of the job; only allowed for root
    "--verbose",  # this will print the exact command line that srun executes for each task
    f"--output=task_{run_label}_%t.out",  # write stdout and stderr of task to file named `task_<run number>_<task id>.out` into job's working directory
    "--",
    "/usr/bin/shifter",  #TODO use absolute path to work around spurious error `slurmstepd: error: execve(): shifter: No such file or directory` leading to `srun` return code 2
    "--",
    "bash",
    "-c",
    " ".join(task_cmd),  # command to run in container; needs to be passed as a single string to `bash -c`
  ]
  print(f"Submitting {nmb_tasks} tasks: '{' '.join(srun_cmd)}'")
  srun_result = subprocess.run(srun_cmd, check = False)
  with open(f"./srun_{run_label}.rc", "w", encoding = "utf-8") as log_file:
    log_file.write(f"{srun_result.returncode:d}")
  print(f"`srun` finished with return code {srun_result.returncode:d}")
  # `srun` will return i) a non-zero slurm exit code, if it cannot
  # start the tasks, or ii) the highest exit code of any failed tasks.
  # If a task is killed by signal, 128 + signal number is returned.

  print("-------------------------------------------------------------------------------")
  # define all output files that swif2 should transfer back to JLab
  output_file_paths: List[Tuple[str, str]] = get_output_file_paths(args.run_number, args.swif_output_root)
  print(f"Transferring {len(output_file_paths)} files back to JLab")
  for local_output_file_path, remote_output_file_path in output_file_paths:
    output_cmd = f"./.swif/swif2 output '{local_output_file_path}' '{remote_output_file_path}'"  #TODO   for some reason, swif2 is not in path
    print(f"Defining output file: '{output_cmd}'")
    subprocess.run(output_cmd, shell = True, check = False)

  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time // 60} min, {elapsed_time % 60} sec")
  sys.exit(srun_result.returncode)  # forward return code of srun to the caller of this script, i.e. swif2


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Prepare directory structure and use srun to start a reconstruction task on each node (positional args).",
  )
  parser.add_argument("--run_number",              required = True, help = "Run number for this job", type = int)
  parser.add_argument("--launch_dir",              required = True, help = "Path to launch directory containing scripts and config files inside container")
  parser.add_argument("--jana_config",             required = True, help = "JANA config file")
  parser.add_argument("--jana_calib_context",      required = True, help = "JANA calibration context")
  parser.add_argument("--jana_geometry_url",       required = True, help = "JANA geometry URL")
  parser.add_argument("--halld_version_set_xml",   required = True, help = "GlueX software version set XML file")
  parser.add_argument("--nmb_processes_per_task",  required = True, help = "Number of processes per task",            type = int)
  parser.add_argument("--nmb_threads_per_process", required = True, help = "Number of threads per `hd_root` process", type = int)
  parser.add_argument("--swif_output_root",        required = True, help = "Root of JLab directory tree, where output files will be copied to")
  main(parser.parse_args())
