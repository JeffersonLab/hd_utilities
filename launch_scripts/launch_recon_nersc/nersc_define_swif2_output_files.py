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

  define_swif2_output_files(args.run_number, args.swif_output_root)

  print("-------------------------------------------------------------------------------")
  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Define the output files that should be transferred back to JLab using `swif2 output` commands for the given run number.",
  )
  parser.add_argument("--run_number",       required = True, help = "Run number for this job", type = int)
  parser.add_argument("--swif_output_root", required = True, help = "Root of JLab directory tree, where output files will be copied to")
  main(parser.parse_args())
