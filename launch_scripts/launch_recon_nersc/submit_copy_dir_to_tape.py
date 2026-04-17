#!/usr/bin/env python3

"""Submits a swif2 job that copies the full content of a directory with prepared recon output to tape."""

from __future__ import annotations

import argparse
import functools
import os
import subprocess
import sys
import time

from script_job import print_command_line_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  # get_directory_size,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period     = ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  ver            = ensure_dict_value_exists(launch_config, "VER")
  batch          = ensure_dict_value_exists(launch_config, "BATCH")
  reco_data_root = ensure_dict_value_exists(launch_config, "RECO_DATA_ROOT")

  recon_src_path = os.path.abspath(args.recon_src_path)
  # tape_dest_path = "/volatile/halld/home/bgrube/test_copy_dir_to_tape"
  # tape_dest_path = "mss:/mss/halld/home/bgrube/test/test_copy_dir_to_tape"
  tape_dest_path = f"mss:{reco_data_root}/{ver}"
  swif_workflow  = f"copy_${run_period}_${batch}_NERSC-multi"
  print(f"Copying content of directory '{recon_src_path}' into destination directory '{tape_dest_path}' using swif2 workflow '{swif_workflow}'")
  # recon_dir_size_TB = get_directory_size(recon_src_path) / 1024**4
  # print(f"Data volume to copy: {recon_dir_size_TB:.3f} TB")

  # create swif2 workflow
  workflow_status = subprocess.run(["swif2", "status", swif_workflow], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, check = False)
  if workflow_status.returncode == 0:
    print(f"Workflow '{swif_workflow}' already exists; skipping creation")
  else:
    print(f"Creating swif2 workflow '{swif_workflow}'")
    subprocess.run(["swif2", "create", swif_workflow], check = True)

  # submit swif2 job that copies the directory structure with all files to tape
  this_script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
  recon_dir_name  = os.path.basename(recon_src_path)
  job_name        = f"GlueX_copy_{run_period}_{recon_dir_name}"
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
    "-time",      "30min", # wall time of job is defined by the time it takes to create the list of output files; the copying is done in the `reaping` stage, which does not have a time limit
    "-stdout",    f"{this_script_dir}/{job_name}.out",
    "-stderr",    f"{this_script_dir}/{job_name}.err",
    "-shell",     "/bin/bash",  # ensure that the job runs in bash
    f'PYTHONPATH="{this_script_dir}:${{PYTHONPATH}}" {this_script_dir}/script_copy_dir_to_tape.py --src_dir_path="{recon_src_path}" --dest_dir_path="{tape_dest_path}"',
  ]
  print(f"Submitting job: {' '.join(swif2_cmd)}")
  swif2_result = subprocess.run(swif2_cmd, check = False)
  print(f"`swif2` finished with return code {swif2_result.returncode:d}")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Submits a swif2 job that copies the full content of the given directory with prepared recon output to tape.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--recon_src_path",  help = "Path to the prepared reconstruction directory, the content of which will be copied to tape")
  args = parser.parse_args()
  main(args)
