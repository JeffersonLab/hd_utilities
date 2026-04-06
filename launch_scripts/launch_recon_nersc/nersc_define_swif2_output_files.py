#!/usr/bin/env python3
#NOTE this script needs to be compatible with Python 3.6

"""
  Defines the output files that should be transferred back to JLab
  using `swif2 output` commands for the given run number.

  Use this to recover jobs that were killed due to Slurm timeout or
  other failures that prevented the job script `script_job.py` from
  finishing properly.

  Example usage:
    nersc_define_swif2_output_files.py \
      --run_working_dir /pscratch/sd/j/jlab/swif/jobs/gxproj4/GlueX_recon_100553/70263838 \
      --swif_output_root /lustre/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/ver02-perl \
    >> /pscratch/sd/j/jlab/swif/jobs/gxproj4/GlueX_recon_100553/70263838/job_100553_50929462.out
"""

import argparse
import functools
import glob
import os
import sys
import time

from script_job import (
  define_swif2_output_files,
  print_command_line_arguments,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()

  # go into run working directory
  try:
    os.chdir(args.run_working_dir)
  except Exception as e:
    print(f"Error: Failed to change into run working directory '{args.run_working_dir}': {e}")
    sys.exit(1)
  # find RUNXXXXXX subdirectory and extract run number from it
  run_dir = sorted(glob.glob(f"RUN??????"))
  if len(run_dir) == 0:
    print(f"Error: No run directory found in '{args.run_working_dir}'")
    sys.exit(1)
  elif len(run_dir) > 1:
    print(f"Error: Multiple run directories found in '{args.run_working_dir}': {run_dir}")
    sys.exit(1)
  elif not run_dir[0][3:].isdigit():
    print(f"Error: Invalid run directory name '{run_dir[0]}' in '{args.run_working_dir}'")
    sys.exit(1)
  run_number = int(run_dir[0][3:])

  # protect against accidentally appending files to existing `__swif_outfiles__` file
  if os.path.exists(f"./__swif_outfiles__"):
    raise RuntimeError(f"'__swif_outfiles__' already exists in '{args.run_working_dir}'; remove it before running this script")
  else:
    print_command_line_arguments(args)
    # define output files for swif2
    os.environ["SWIF_JOB_STAGE_DIR"] = os.path.abspath(os.getcwd())  # needed by `./.swif/swif2` command
    define_swif2_output_files(run_number, args.swif_output_root)

  # print("-------------------------------------------------------------------------------")  #TODO
  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Define the output files that should be transferred back to JLab using `swif2 output` commands for the given run number.",
  )
  parser.add_argument("--run_working_dir",  required = True, help = "Working directory of the run")
  parser.add_argument("--swif_output_root", required = True, help = "Root of JLab directory tree, where output files will be copied to")  #TODO take from .env file or from info job .out file
  main(parser.parse_args())
