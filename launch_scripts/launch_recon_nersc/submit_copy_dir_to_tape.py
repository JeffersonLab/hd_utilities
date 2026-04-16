#!/usr/bin/env python3

"""Submits a swif2 job that copies the full content of a directory to tape."""

from __future__ import annotations

import argparse
import functools
import os
import subprocess
import time

from script_job import print_command_line_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period = ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  batch      = ensure_dict_value_exists(launch_config, "BATCH")

  dir_to_copy   = "/w/halld-scshelf2101/bgrube/halldRepos/hd_utilities/launch_scripts/launch_recon_nersc/test/test_work_dir_job"
  dest_dir      = "file:/volatile/halld/home/bgrube/test_copy_dir_to_tape"
  swif_workflow = f"copy_{run_period}_{batch}"

  print(f"Copying content of directory '{dir_to_copy}' to into destination directory '{dest_dir}' using swif2 workflow '{swif_workflow}'")

  # create swif2 workflow
  workflow_status = subprocess.run(["swif2", "status", swif_workflow], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL, check = False)
  if workflow_status.returncode == 0:
    print(f"Workflow '{swif_workflow}' already exists; skipping creation")
  else:
    print(f"Creating swif2 workflow '{swif_workflow}'")
    subprocess.run(["swif2", "create", swif_workflow], check = True)

  # submit swif2 job that copies the directory structure with all files to tape
  job_name = f"GlueX_copy_{run_period}_{batch}"
  swif2_cmd: list[str] = [
    "swif2",
    "add-job",
    "-workflow", swif_workflow,  # swif2 workflow name
    "-name", job_name,  # swif2 job name
    # "-account", "halld-pro",
    "-account", "halld",
    # "-partition", "production",
    "-partition", "priority",
    "-cores", "1",
    "-disk", "1GB",
    "-ram", "1GB",
    "-time", f"30min",
    # "-output", f"match:{recon_dir_path}/**", f"file:{tape_dir_path}",
    "-stdout", f"/volatile/halld/home/bgrube/{job_name}.out",
    "-stderr", f"/volatile/halld/home/bgrube/{job_name}.err",
    f'{os.path.abspath("./script_copy_dir_to_tape.py")} --src_dir_path="{recon_dir_path}" --dest_dir_path="{tape_dir_path}"',
  ]
  print(f"Submitting job: '{' '.join(swif2_cmd)}'")
  swif2_result = subprocess.run(swif2_cmd, check = False)
  print(f"`swif2` finished with return code {swif2_result.returncode:d}")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Submits a swif2 job that copies the full content of the given directory to tape.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  # parser.add_argument("--dir_to_copy", help = "Path to the directory, the content of which will be copied to tape")
  args = parser.parse_args()
  main(args)
