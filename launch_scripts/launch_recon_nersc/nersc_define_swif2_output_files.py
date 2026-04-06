#!/usr/bin/env python3
#NOTE this script needs to be compatible with Python 3.6

"""
  Defines the output files that should be transferred back to JLab
  using `swif2 output` commands for the given run number.

  Use this to recover jobs that were killed due to Slurm timeout or
  other failures that prevented the job script `script_job.py` from
  finishing properly.
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
  print_command_line_arguments(args)

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

  # define output files for swif2
  define_swif2_output_files(run_number, args.swif_output_root)

  print("-------------------------------------------------------------------------------")
  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Define the output files that should be transferred back to JLab using `swif2 output` commands for the given run number.",
  )
  parser.add_argument("--run_working_dir",  required = True, help = "Working directory of the run")
  parser.add_argument("--swif_output_root", required = True, help = "Root of JLab directory tree, where output files will be copied to")  #TODO take from .env file or from info job .out file
  main(parser.parse_args())
