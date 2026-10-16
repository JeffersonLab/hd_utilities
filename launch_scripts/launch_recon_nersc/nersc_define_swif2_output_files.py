#!/usr/bin/env python3
#NOTE this script requires at least Python 3.9
# run `module load python/3.9` at NERSC to get Python 3.9

"""
Define the output files that should be transferred back to JLab for
the job corresponding to the given run number.

Use this to recover jobs that were killed due to Slurm timeout or
other failures that prevented the job script `script_job.py` from
finishing properly.

Example usage:
Run
  ./nersc_define_swif2_output_files.py 100570 \
  >> /pscratch/sd/j/jlab/swif/jobs/gxproj4/GlueX_recon_100570/70263844/job_100570_50937509.out
and then bless the job
  swif2 bless-jobs recon_2022-05_ver02-perl_NERSC-multi GlueX_recon_100570
"""

from __future__ import annotations

import argparse
import functools
import os
import pickle
import time

from FileTransferMapGenerator import define_swif2_output_files
from utilities import (
  get_nersc_swif_attempt_dir_path,
  print_command_line_arguments,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  work_dir_job_path = get_nersc_swif_attempt_dir_path(args.nersc_swif_jobs_root_dir_path, args.run_number, args.swif_attempt_id)
  assert work_dir_job_path is not None, f"Failed to find swif attempt directory for run number {args.run_number} in '{args.nersc_swif_jobs_root_dir_path}'"
  print(f"Processing swif attempt directory '{work_dir_job_path}'")

  # protect against accidentally appending files to existing `__swif_outfiles__` file
  if os.path.exists(f"./__swif_outfiles__"):
    raise RuntimeError(f"'__swif_outfiles__' already exists in '{work_dir_job_path}'; remove it before running this script")
  else:
    # load arguments of script_job.py
    script_job_args = None
    with open(f"job_{args.run_number:06d}_args.pkl", "rb") as args_file:
      script_job_args = pickle.load(args_file)
    assert script_job_args is not None, "Failed to load script job arguments from pickle file"
    assert args.run_number == script_job_args.run_number, "Mismatch between provided run number and run number used in job script"
    # define output files for swif2
    os.environ["SWIF_JOB_STAGE_DIR"] = work_dir_job_path  # needed by `./.swif/swif2` command
    print("-------------------------------------------------------------------------------")
    define_swif2_output_files(
      job_id                              = script_job_args.job_id,
      run_number                          = script_job_args.run_number,
      work_dir_job_path                   = work_dir_job_path,
      nmb_tasks                           = script_job_args.nmb_tasks,
      nmb_processes_per_task              = script_job_args.nmb_processes_per_task,
      hd_root_output_dest_dir_path        = script_job_args.swif_hd_root_output_dest_dir,
      log_files_dest_dir_path             = script_job_args.swif_log_files_output_dest_dir,
      failed_hd_root_output_dest_dir_path = script_job_args.swif_failed_hd_root_output_dest_dir,
    )

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed defining swif2 output files: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Define the output files that should be transferred back to JLab for the job corresponding to the given run number.",
  )
  parser.add_argument(metavar = "run-number",            dest = "run_number", type = int,                   help = "Run number of the job to process")
  parser.add_argument("--swif-attempt-id",                                    type = int,                   help = "SWIF attempt ID of the job to process; if not given, the highest attempt ID will be used")
  parser.add_argument("--nersc-swif-jobs-root-dir-path", default = "/pscratch/sd/j/jlab/swif/jobs/gxproj4", help = "Path of NERSC root directory for SWIF jobs; default: '%(default)s'")  #TODO pass from .env file
  main(parser.parse_args())
