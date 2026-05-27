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
import glob
import os
import sys
import time

from utilities import (
  define_swif2_output_files,
  print_command_line_arguments,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  # cd into run working directory
  run_working_dir = f"{args.nersc_swif_jobs_root_dir}/GlueX_recon_{args.run_number:06d}"
  os.chdir(run_working_dir)
  # find attempt subdirectory to process and cd into it
  swif_attempt_dirs = [entry for entry in sorted(glob.glob("*")) if os.path.isdir(entry)]
  if len(swif_attempt_dirs) == 0:
    print(f"Error: No directories with swif attempts found in '{run_working_dir}'")
    sys.exit(1)
  if args.swif_attempt_id is not None:
    # find matching swif attempt dir
    found_matching_swif_attempt_dir = False
    for swif_attempt_dir in swif_attempt_dirs:
      if os.path.basename(swif_attempt_dir) == str(args.swif_attempt_id):
        found_matching_swif_attempt_dir = True
        print(f"Found directory '{swif_attempt_dir}' for swif attempt ID {args.swif_attempt_id}")
        os.chdir(swif_attempt_dir)
        break
    if not found_matching_swif_attempt_dir:
      print(f"Error: No matching swif attempt directory for ID {args.swif_attempt_id} found in '{run_working_dir}'")
      sys.exit(1)
  else:
    if len(swif_attempt_dirs) > 1:
      print(f"Warning: Multiple directories with swif attempts found in '{run_working_dir}': {swif_attempt_dirs}; using the last one")
    os.chdir(swif_attempt_dirs[-1])
  print(f"Processing swif attempt directory '{os.getcwd()}'")

  # protect against accidentally appending files to existing `__swif_outfiles__` file
  if os.path.exists(f"./__swif_outfiles__"):
    raise RuntimeError(f"'__swif_outfiles__' already exists in '{run_working_dir}'; remove it before running this script")
  else:
    # define output files for swif2
    os.environ["SWIF_JOB_STAGE_DIR"] = os.path.abspath(os.getcwd())  # needed by `./.swif/swif2` command
    define_swif2_output_files(args.run_number, args.swif_output_root, args.transfer_all_files)

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed defining swif2 output files: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Define the output files that should be transferred back to JLab for the job corresponding to the given run number.",
  )
  parser.add_argument("run_number", type = int, help = "Run number of the job to process")
  parser.add_argument("--swif_attempt_id", type = int, help = "SWIF attempt ID of the job to process; if not given, the last attempt will be used")
  parser.add_argument("--nersc_swif_jobs_root_dir", default = "/pscratch/sd/j/jlab/swif/jobs/gxproj4", help = "NERSC root directory for SWIF jobs; default: '%(default)s'")
  parser.add_argument("--swif_output_root", default = "/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-2021-11/recon/ver05-perl", help = "Root of JLab directory tree, where output files will be copied to; default: '%(default)s'")  #TODO read this value from the jobs .env file
  parser.add_argument("--transfer_all_files", action = "store_true", help = "If set, do not filter output of failed hd_root processes; default: False")
  main(parser.parse_args())
