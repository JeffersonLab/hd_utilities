#!/usr/bin/env python3
#NOTE this script requires at least Python 3.9
# run `module load python/3.9` at NERSC to get Python 3.9

"""
Prints the last N lines of the stdout and stderr log files for hd_root processes with non-zero return code.
"""

from __future__ import annotations

import argparse
from collections import deque
import functools
import glob
import os
import signal
import time

from utilities import (
  get_file_number_from_evio_file_name,
  get_hd_root_return_code,
  get_nersc_swif_attempt_dir_path,
  print_command_line_arguments,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def tail_file(
  file_name: str,
  nmb_lines: int = 3
) -> list[str]:
  """Returns the last n lines of the given file."""
  try:
    with open(file_name, "r", encoding = "utf-8") as file:
      return list(deque(file, maxlen = nmb_lines))
  except FileNotFoundError:
    print(f"WARNING: cannot find '{file_name}'")
    return []
  except UnicodeDecodeError as error:
    print(f"WARNING: unable to decode '{file_name}' as utf-8: {error}")
    return []


def return_code_signal_name(return_code: int | None) -> str:
  """Returns "<return_code> (<signal_name>)" if signal is known else the return code."""
  if return_code is None:
    return "unknown"
  sig_names = {
    value : name
    for name, value in signal.__dict__.items()
    if name.startswith("SIG") and not name.startswith("SIG_")
  }
  signal_number = return_code - 128
  if signal_number in sig_names.keys():
    return f"{return_code} ({sig_names[signal_number]})"
  else:
    return str(return_code)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  work_dir_job_path = get_nersc_swif_attempt_dir_path(args.nersc_swif_jobs_root_dir_path, args.run_number, args.swif_attempt_id)
  assert work_dir_job_path is not None, f"Failed to find swif attempt directory for run number {args.run_number} in '{args.nersc_swif_jobs_root_dir_path}'"
  print(f"Processing swif attempt directory '{work_dir_job_path}'")

  #TODO perform additional sanity checks of job output
  # find `hd_root.rc` files with hd_root return codes
  hd_root_rc_files:        list[str] = []
  hd_root_failed_rc_files: list[str] = []
  for task_dir in sorted(glob.glob(f"{work_dir_job_path}/RUN??????/TASK???")):
    if not os.path.isdir(task_dir):
      print(f"WARNING: expected '{task_dir}' to be a directory; skipping it")
      continue
    # ensure a FILE??? subdirectory exists for each EVIO file in the task directory
    evio_file_names = sorted(glob.glob(f"{task_dir}/*.evio"))
    for evio_file_name in evio_file_names:
      file_dir = f"{task_dir}/FILE{get_file_number_from_evio_file_name(evio_file_name):03d}"
      hd_root_rc_file = f"{file_dir}/hd_root.rc"
      if os.path.isfile(hd_root_rc_file):
        hd_root_rc_files.append(hd_root_rc_file)
      else:
        print(f"WARNING: cannot find '{file_dir}'")
        hd_root_failed_rc_files.append(hd_root_rc_file)  # add non-existing hd_root.rc file

  # process existing hd_root.rc files and identify failed hd_root processes with non-zero return code
  if len(hd_root_rc_files) == 0:
    print(f"WARNING: No hd_root return code files found in '{work_dir_job_path}'")
  for hd_root_rc_file in hd_root_rc_files:
    hd_root_return_code = get_hd_root_return_code(hd_root_rc_file)
    print(f"Found hd_root return code {hd_root_return_code:3d} in '{hd_root_rc_file}'")
    if hd_root_return_code is None or hd_root_return_code != 0:
      hd_root_failed_rc_files.append(hd_root_rc_file)
  print(f"Found {len(hd_root_failed_rc_files)} hd_root processes with non-zero or unknown return code")

  # report failed hd_root processes and tail their log files for debugging
  for hd_root_rc_file in hd_root_failed_rc_files:
    # tail hd_root log files if return code is non-zero or could not be read
    hd_root_log_dir_name = os.path.dirname(hd_root_rc_file)
    print("-------------------------------------------------------------------------------")
    print(f"hd_root return code: {return_code_signal_name(get_hd_root_return_code(hd_root_rc_file))} from '{hd_root_rc_file}'")
    for hd_root_log_file_name in ("hd_root.out", "hd_root.err"):
      hd_root_log_file_path = f"{hd_root_log_dir_name}/{hd_root_log_file_name}"
      print(f"--- {hd_root_log_file_path}")
      for line in tail_file(hd_root_log_file_path, args.nmb_lines):
        print(line, end = "")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Print the last N lines of the stdout and stderr log files for hd_root processes with non-zero return code.",
  )
  parser.add_argument(metavar = "run-number",            dest = "run_number", type = int,                   help = "Run number of the job to process")
  parser.add_argument("--swif-attempt-id",                                    type = int,                   help = "SWIF attempt ID of the job to process; if not given, the highest attempt ID will be used")
  parser.add_argument("--nersc-swif-jobs-root-dir-path", default = "/pscratch/sd/j/jlab/swif/jobs/gxproj4", help = "Path of NERSC root directory for SWIF jobs; default: '%(default)s'")  #TODO pass from .env file
  parser.add_argument("--nmb-lines",                     type = int, default = 3,                           help = "Number of lines to print from the end of the log files; default: %(default)i")
  main(parser.parse_args())
