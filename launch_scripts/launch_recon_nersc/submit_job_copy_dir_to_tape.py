#!/usr/bin/env python3

"""Submits a swif2 job that copies the full content of a directory to tape."""

from __future__ import annotations

import argparse
import functools
import os
import time

from FileTransferMapGenerator import RECON_SUBDIR_BASENAME_MAP
from utilities import (
  ensure_dict_value_exists,
  ensure_swif_workflow_exists,
  print_command_line_arguments,
  run_shell_cmd,
)
from utilities_dotenv import get_config_dict_from_env_file


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  this_script_dir = print_command_line_arguments(args)
  if args.dry_run:
    print("DRY RUN mode enabled: no external commands will be executed")
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period              = ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  ver                     = ensure_dict_value_exists(launch_config, "VER")
  ver_label               = ensure_dict_value_exists(launch_config, "VER_LABEL")
  production_user         = ensure_dict_value_exists(launch_config, "PRODUCTION_USER")
  log_files_dir_path      = ensure_dict_value_exists(launch_config, "SWIF_LOG_FILES_OUTPUT_DEST_DIR")
  reco_data_root_dir_path = ensure_dict_value_exists(launch_config, "RECO_DATA_ROOT")

  src_dir_path = os.path.abspath(args.override_src_dir_path or log_files_dir_path)
  tape_dest_path = args.override_tape_dest_path or f"mss:{reco_data_root_dir_path}/{ver}"
  swif_workflow  = f"copy_{run_period}_{ver_label}_NERSC-multi"
  print(f"Submitting jobs that copy the content of directory '{src_dir_path}' into the destination directory '{tape_dest_path}' using swif2 workflow '{swif_workflow}'")

  # create swif2 workflow if it does not exist yet
  ensure_swif_workflow_exists(swif_workflow, dry_run = args.dry_run)

  # submit swif2 job that copies the directory structure with all files to tape
  recon_dir_name  = os.path.basename(src_dir_path)
  swif_job_name = f"GlueX_copy_{run_period}.{recon_dir_name}"
  swif2_cmd: list[str] = [
    "swif2",
    "add-job",
    "-workflow",  swif_workflow,
    "-name",      swif_job_name,
    "-account",   "halld-pro",
    "-partition", "production",
    # "-account",   "halld",
    # "-partition", "priority",
    "-cores",     "1",
    "-disk",      "1GB",
    "-ram",       "1GB",
    "-time",      "2h",  # wall time of job is defined by the time it takes to create the list of output files; the copying is done in the `reaping` stage, which does not have a time limit
    "-stdout",    f'"/farm_out/{production_user}/{swif_workflow}/{swif_job_name}.out"',
    "-stderr",    f'"/farm_out/{production_user}/{swif_workflow}/{swif_job_name}.err"',
    "-shell",     "/bin/bash",  # ensure that the job runs in bash
    f'PYTHONPATH="{this_script_dir}:${{PYTHONPATH}}" "{this_script_dir}"/script_job_copy_dir_to_tape.py --src-dir-path="{src_dir_path}" --dest-dir-path="{tape_dest_path}"',
  ]
  print(f"Submitting swif2 copy job '{swif_job_name}' for directory '{src_dir_path}'")
  run_shell_cmd(' '.join(swif2_cmd), dry_run = args.dry_run)  #TODO if swif2_cmd is passed as array, swif2 exits with return code 13 and `ERROR You didn't specify any command.`

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
    description = "Submits a swif2 job that copies the full content of the given directory to tape.",
  )
  parser.add_argument(metavar = "launch-env-file", dest = "launch_env_file", help = "Path to .env file defining the configuration variables of the reconstruction launch")
  parser.add_argument("--override-src-dir-path",                             help = "Path of the directory, the content of which will be copied to the tape destination directory; default = '{SWIF_LOG_FILES_OUTPUT_DEST_DIR}'")
  parser.add_argument("--override-tape-dest-path",                           help = "Path to the tape destination directory, the reconstructed data will be copied to; default = 'mss:{RECO_DATA_ROOT}/{VER}'")
  parser.add_argument("--dry-run",                 action = "store_true",    help = "Preview job submission without performing it; default: false")
  args = parser.parse_args()
  main(args)
