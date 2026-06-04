#!/usr/bin/env python3

"""Submits a swif2 job that copies the full content of a directory with prepared recon output to tape."""

from __future__ import annotations

import argparse
import functools
import os
import subprocess
import sys
import time

from utilities import (
  ensure_dict_value_exists,
  print_command_line_arguments,
)
from utilities_dotenv import get_config_dict_from_env_file


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period       = ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  ver              = ensure_dict_value_exists(launch_config, "VER")
  ver_label        = ensure_dict_value_exists(launch_config, "VER_LABEL")
  reco_data_root   = ensure_dict_value_exists(launch_config, "RECO_DATA_ROOT")
  swif_output_root = ensure_dict_value_exists(launch_config, "SWIF_OUTPUT_ROOT")

  recon_src_path = os.path.abspath(args.override_recon_src_path or f"{os.path.dirname(swif_output_root)}/{ver_label}.ready_for_tape")
  tape_dest_path = args.override_tape_dest_path or f"mss:{reco_data_root}/{ver}"
  swif_workflow  = f"copy_{run_period}_{ver_label}_NERSC-multi"
  print(f"Submitting jobs that copy the content of directory '{recon_src_path}' into the destination directory '{tape_dest_path}' using swif2 workflow '{swif_workflow}'")
  # recon_dir_size_TB = get_directory_size(recon_src_path) / 1024**4  #TODO this takes a looong time
  # print(f"Data volume to copy: {recon_dir_size_TB:.3f} TB")

  # create swif2 workflow
  #TODO use run_shell_cmd from submit_launch.py
  workflow_status = subprocess.run(["swif2", "status", swif_workflow], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, check = False)
  if workflow_status.returncode == 0:
    print(f"Workflow '{swif_workflow}' already exists; pausing workflow")
    subprocess.run(["swif2", "pause", swif_workflow], check = True)  # pausing workflow to allow inspection of submitted jobs before resuming
  else:
    print(f"Creating swif2 workflow '{swif_workflow}'")
    subprocess.run(["swif2", "create", swif_workflow], check = True)

  # submit swif2 job that copies the directory structure with all files to tape
  this_script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
  recon_dir_name  = os.path.basename(recon_src_path)
  # submit a separate job for each subdirectory in the recon directory to limit number of files per job to less than about 10k  #TODO Chris is working on extending swif2 to handle larger numbers of files per job
  subdir_names = [item_name for item_name in os.listdir(recon_src_path) if os.path.isdir(f"{recon_src_path}/{item_name}")]
  for subdir_name in subdir_names:
    print (f"Submitting copy job for subdirectory [{subdir_names.index(subdir_name) + 1}/{len(subdir_names)}] '{subdir_name}'")
    job_name = f"GlueX_copy_{run_period}_{recon_dir_name}_{subdir_name}"
    swif2_cmd: list[str] = [
      "swif2",
      "add-job",
      "-workflow",  swif_workflow,  # swif2 workflow name
      "-name",      job_name,       # swif2 job name
      "-account",   "halld-pro",
      "-partition", "production",
      # "-account",   "halld",
      # "-partition", "priority",
      "-cores",     "1",
      "-disk",      "1GB",
      "-ram",       "1GB",
      "-time",      "12h", # wall time of job is defined by the time it takes to create the list of output files; the copying is done in the `reaping` stage, which does not have a time limit
      "-stdout",    f"{this_script_dir}/{job_name}.out",
      "-stderr",    f"{this_script_dir}/{job_name}.err",
      "-shell",     "/bin/bash",  # ensure that the job runs in bash
      f'PYTHONPATH="{this_script_dir}:${{PYTHONPATH}}" {this_script_dir}/script_copy_dir_to_tape.py --src_dir_path="{recon_src_path}/{subdir_name}" --dest_dir_path="{tape_dest_path}/{subdir_name}"',
    ]
    print(f"Submitting job: {' '.join(swif2_cmd)}")
    swif2_result = subprocess.run(swif2_cmd, check = False)
    print(f"`swif2` finished with return code {swif2_result.returncode:d}")

  print("-------------------------------------------------------------------------------")
  print(f"Status of swif2 workflow '{swif_workflow}' after submitting all jobs; view jobs at https://scicomp.jlab.org/scicomp/swif/active")
  subprocess.run(["swif2", "list"], check = True)
  subprocess.run(["swif2", "status", swif_workflow], check = True)
  print(f"Run 'swif2 run {swif_workflow}' to (re)start the paused workflow")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Submits a swif2 job that copies the full content of the given directory with prepared recon output to tape.",
  )
  parser.add_argument("launch_env_file",           help = "Path to .env file defining the configuration variables of the reconstruction launch")
  parser.add_argument("--override_recon_src_path", help = "Path of the directory, the content of which will be copied to tape destination directory; default = '{SWIF_OUTPUT_ROOT}/{VER_LABEL}.ready_for_tape'")
  parser.add_argument("--override_tape_dest_path", help = "Path to the tape destination directory, the reconstructed data will be copied to; default = 'mss:{RECO_DATA_ROOT}/{VER}'")
  args = parser.parse_args()
  main(args)
