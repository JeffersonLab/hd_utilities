#!/usr/bin/env python3

"""
Master script that submits reconstruction jobs to run at NERSC using
swif2.  One swif2/NERSC job is submitted for each run number in the
given list file.

The computation is structured into the following levels (lowest to highest):
  1) 1 NERSC process: 1 `hd_root` process running 32 threads to process one EVIO file
  2) 1 NERSC task: 256/32 = 8 NERSC processes running concurrently on a NERSC node -> 1 task per node
  3) 1 NERSC job: 1 run number with N EVIO files, processed by ceil(N/8) NERSC tasks (=nodes) running concurrently -> ceil(N/8) tasks (nodes) per job
This is performed by the following scripts:
  * task script: runs 8 `hd_root` processes in parallel, each processing one EVIO file
  * job script: starts one NERSC task per node using `srun`
  * this script: submits one NERSC job per run number using `sbatch`
"""

from __future__ import annotations

import argparse
import functools
import os
import shlex
import stat
import subprocess
import sys
import time
from typing import Any

from script_job import print_command_line_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_evio_file_paths_for_run,
  read_run_numbers_from_file,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def run_shell_cmd(
  cmd:              str,
  echo_cmd:         bool = False,  # whether to print the command before executing it
  dry_run:          bool = False,  # if True, only print the command without executing it
  **kwargs_for_run: Any,           # additional keyword arguments forwarded to subprocess.run()
) -> bool:
  """Runs the given command using subprocess.run() and returns True if the command succeeds."""
  if echo_cmd or dry_run:
    print(f"+ {cmd}")
  if dry_run:
    return True
  return subprocess.run(cmd, shell = True, **kwargs_for_run).returncode == 0


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)

  # print git hash  #TODO move to separate utility function and use in all scripts
  this_script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))  # get the directory of this script
  if os.path.isfile(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH"):
    git_hash = open(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH", "r", encoding = "utf-8").read().strip()
    print(f"Using launch scripts in '{this_script_dir}' with git commit hash: {git_hash}")

  if args.dry_run:
    print("DRY RUN mode enabled: no external commands will be executed")

  # copy scripts and config files to NERSC
  run_shell_cmd(f"{this_script_dir}/deploy_launch_dir_to_nersc.sh {args.launch_env_file}", echo_cmd = True, dry_run = args.dry_run)

  # verify that container image exists in NERSC's `shifter` repository
  nersc_host            = ensure_dict_value_exists(launch_config, "NERSC_HOST")
  nersc_container_image = ensure_dict_value_exists(launch_config, "NERSC_CONTAINER_IMAGE")
  if run_shell_cmd(f'ssh {nersc_host} shifterimg lookup "{nersc_container_image}"', check = False, stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, dry_run = args.dry_run):
    print(f"Container image '{nersc_container_image}' exists in NERSC `shifter` repository")
  else:
    print(f"Container image '{nersc_container_image}' does not exist in NERSC `shifter` repository; aborting")
    sys.exit(1)

  swif_workflow            = ensure_dict_value_exists(launch_config, "SWIF_WORKFLOW")
  swif_site                = ensure_dict_value_exists(launch_config, "SWIF_SITE")
  swif_max_concurrent_jobs = ensure_dict_value_exists(launch_config, "SWIF_MAX_CONCURRENT_JOBS")
  if run_shell_cmd(f'swif2 status "{swif_workflow}"', check = False, stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, dry_run = args.dry_run):
    print(f"Workflow '{swif_workflow}' already exists; pausing workflow")
    run_shell_cmd(f'swif2 pause "{swif_workflow}"', dry_run = args.dry_run)  # pausing workflow to allow inspection of submitted jobs before resuming
  else:
    print(f"Creating swif2 workflow '{swif_workflow}' at site '{swif_site}' with {swif_max_concurrent_jobs} max concurrent jobs")
    run_shell_cmd(f"swif2 create {swif_workflow} -site {swif_site} -max-concurrent {swif_max_concurrent_jobs}", dry_run = args.dry_run)

  # read run numbers from file and submit one swif2 job each run
  print("-------------------------------------------------------------------------------")
  run_period                    =     ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_list_file          =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  raw_data_root                 =     ensure_dict_value_exists(launch_config, "RAW_DATA_ROOT")
  nersc_nmb_processes_per_task  = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))
  nersc_project                 =     ensure_dict_value_exists(launch_config, "NERSC_PROJECT")
  nersc_node_type               =     ensure_dict_value_exists(launch_config, "NERSC_NODE_TYPE")
  nersc_qos                     =     ensure_dict_value_exists(launch_config, "NERSC_QOS")
  nersc_max_wall_time           =     ensure_dict_value_exists(launch_config, "NERSC_MAX_WALL_TIME")
  nersc_max_threads_per_task    =     ensure_dict_value_exists(launch_config, "NERSC_MAX_THREADS_PER_TASK")
  nersc_launch_dir              =     ensure_dict_value_exists(launch_config, "NERSC_LAUNCH_DIR")
  nersc_launch_dir_container    =     ensure_dict_value_exists(launch_config, "NERSC_LAUNCH_DIR_CONTAINER")
  jana_config                   =     ensure_dict_value_exists(launch_config, "JANA_CONFIG")
  jana_calib_context            =     ensure_dict_value_exists(launch_config, "JANA_CALIB_CONTEXT")
  halld_version_set_xml         =     ensure_dict_value_exists(launch_config, "HALLD_VERSION_SET_XML")
  nersc_nmb_processes_per_task  = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))
  nersc_nmb_threads_per_process = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_THREADS_PER_PROCESS"))
  swif_output_root              =     ensure_dict_value_exists(launch_config, "SWIF_OUTPUT_ROOT")
  run_numbers: list[int] = read_run_numbers_from_file(run_number_list_file)
  print(f"Submitting a swif2 job for each of the {len(run_numbers)} run(s) of run period '{run_period}'")
  for run_counter, run_number in enumerate(run_numbers):
    #TODO limit run umbers to [RUN_NUMBER_MIN, RUN_NUMBER_MAX] from config file
    # construct command to submit a swif2 job for the given run number
    #NOTE swif2 jobs for remote sites do not produce stdout and stderr
    #  on JLab Farm and the `-stdout` and `-stderr` arguments would need
    #  to point to remote paths at NERSC.  Specifying the NERSC log
    #  files using the `-sbatch --output` argument is more flexible.
    swif_job_name = f"GlueX_recon_{run_number:06d}"
    swif2_cmd: list[str] = [
      "swif2",
      "add-job",
      "-workflow", swif_workflow,
      "-name",     swif_job_name,
    ]
    # loop over all EVIO files of the run and subdivide file list into
    # chunks of size `${NERSC_NMB_PROCESSES_PER_TASK}` that will be
    # processed by individual NERSC tasks, defining the input and output
    # files for each task.
    evio_file_paths = get_evio_file_paths_for_run(run_number, raw_data_root)
    nmb_evio_files = len(evio_file_paths)
    assert nmb_evio_files > 0, print(f"No EVIO files found for run {run_number} in '{raw_data_root}'; aborting")
    for evio_file_path in evio_file_paths:
      # construct the swif2 input line for the given file, e.g. `-input file1.evio mss:/mss/some_path/file1.evio -input file2.evio mss:/mss/some_path/file2.evio ...`
      # `mss:/` takes files directly from tape, bypassing JLab file systems; this is the most efficient way to transfer files to NERSC
      # `file:/` paths should be used for debugging only
      evio_file_abs_path = os.path.abspath(evio_file_path)
      evio_file_name = os.path.basename(evio_file_path)
      swif2_cmd += ["-input", evio_file_name, ("mss:" if evio_file_abs_path.startswith("/mss/") else "file:") + evio_file_abs_path]
    # all output files that swif2 should transfer back to JLab are defined inside the job script
    # calculate number of tasks from number of EVIO files and number of processes to run per task
    nersc_nmb_tasks = (nmb_evio_files + nersc_nmb_processes_per_task - 1) // nersc_nmb_processes_per_task
    swif2_cmd += [
      "-sbatch",
        # these options are passed to Slurm's `sbatch` when swif2 submits job at NERSC
        f"--account={nersc_project}",
        f"--constraint={nersc_node_type}",
        f"--qos={nersc_qos}",
        f"--time={nersc_max_wall_time}",
        f"--nodes={nersc_nmb_tasks}",  # 1 node per task
         "--ntasks-per-node=1",
        f"--ntasks={nersc_nmb_tasks}",
        f"--cpus-per-task={nersc_max_threads_per_task}",
        #  "--exclusive",  # allocated nodes cannot be shared with other jobs/users  #TODO clarify whether this is beneficial or not; swif2 also has `-exclusive` option that can be set when creating the workflow; is it redundant to set it in both places?
        f"--image={nersc_container_image}",
        f"--volume={nersc_launch_dir}:{nersc_launch_dir_container}",  # map `${NERSC_LAUNCH_DIR}` on host to `${NERSC_LAUNCH_DIR_CONTAINER}` in container
         "--module=cvmfs",  # enable CVMFS in the container so it can access the `/group/halld` tree
        f"--output=job_{run_number:06d}_%j.out",  # write stdout and stderr of job to file named `job_<run number>_<job id>.out` into working directory of job
        "::",
        # job script to run at NERSC
        f"{nersc_launch_dir}/script_job_wrapper.sh",  # sets up Python environment and runs the actual job script `script_job.py` with the given arguments
          nersc_launch_dir,  # NERSC directory that contains `script_job.py`
          # arguments passed to `script_job.py`
          f"--run_number={run_number}",
          f"--launch_dir={nersc_launch_dir_container}",  # path of launch directory inside container
          f"--jana_config={nersc_launch_dir_container}/{jana_config}",  # path of JANA config file inside container
          f"--jana_calib_context={jana_calib_context}",
          f"--halld_version_set_xml={halld_version_set_xml}",
          f"--nmb_processes_per_task={nersc_nmb_processes_per_task}",
          f"--nmb_threads_per_process={nersc_nmb_threads_per_process}",
          f"--swif_output_root={swif_output_root}",
    ]
    # handle optional arguments of the job script
    jana_geometry_url_override = launch_config.get("JANA_GEOMETRY_URL_OVERRIDE")
    if jana_geometry_url_override is not None:
      swif2_cmd += [f"--jana_geometry_url_override={jana_geometry_url_override}"]
    # apply shell-escaping to `swif2` command and write it to a file so it becomes a shell script that can be run directly
    submit_job_script_name = f"submit_job_for_RUN{run_number:06d}.sh"
    with open(submit_job_script_name, "w") as script_file:
      script_file.write(f"{shlex.join(swif2_cmd)}\n")
    current_permissions = os.stat(submit_job_script_name).st_mode
    os.chmod(submit_job_script_name, current_permissions | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)  # make file executable for user, group, and others
    # run the generated script to submit the job
    print(f"    [{run_counter + 1:4d}/{len(run_numbers):4d}] Submitting swif2 job '{swif_job_name}' for run {run_number} with {nmb_evio_files} EVIO file(s) using {nersc_nmb_tasks} NERSC tasks (nodes) with {nersc_nmb_processes_per_task} processes per task")
    run_shell_cmd(f'./"{submit_job_script_name}"', dry_run = args.dry_run)

  print("-------------------------------------------------------------------------------")
  print(f"Status of swif2 workflow '{swif_workflow}' after submitting all jobs; view jobs at https://scicomp.jlab.org/scicomp/swif/active")
  run_shell_cmd("swif2 list", dry_run = args.dry_run)
  run_shell_cmd(f"swif2 status {swif_workflow}", dry_run = args.dry_run)
  print(f"Run 'swif2 run {swif_workflow}' to (re)start the paused workflow")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Submits reconstruction jobs to run at NERSC using swif2.",
  )
  parser.add_argument("launch_env_file",                  help = "Path to .env file defining the configuration variables  of the reconstruction launch")  #TODO transfer this to other scripts
  parser.add_argument("--override_run_list",              help = "Path to run-number list file to use instead of RCDB query")
  parser.add_argument("--dry_run", action = "store_true", help = "Preview commands without performing them; default: false")
  args = parser.parse_args()
  main(args)
