#!/usr/bin/env python3
#NOTE this script needs to be compatible with Python 3.6

"""
Prints the last N lines of the stdout and stderr log files for hd_root processes with non-zero return code.
"""

import argparse
from collections import deque
import functools
import glob
import os
import signal
import sys
import time
from typing import List, Optional

from script_job import (
  get_hd_root_return_code,
  print_command_line_arguments,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def tail_file(
  file_name: str,
  nmb_lines: int = 3
) -> List[str]:
  """Returns the last n lines of the given file."""
  with open(file_name, "r", encoding = "utf-8") as file:
    try:
      return list(deque(file, maxlen = nmb_lines))
    except UnicodeDecodeError as error:
      print(f"WARNING: unable to decode '{file_name}' as utf-8: {error}")
      return []


def return_code_signal_name(return_code: Optional[int]) -> str:
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

  # loop over files with hd_root return codes
  hd_root_rc_files = glob.glob(f"{args.run_working_dir}/RUN??????/TASK???/FILE???/hd_root.rc")
  if len(hd_root_rc_files) == 0:
    print(f"Error: No hd_root return code files found in '{args.run_working_dir}'")
    sys.exit(1)
  hd_root_failed_rc_files: List[str] = []
  for hd_root_rc_file in hd_root_rc_files:
    hd_root_return_code = get_hd_root_return_code(hd_root_rc_file)
    print(f"Found hd_root return code {hd_root_return_code:3d} in '{hd_root_rc_file}'")
    if hd_root_return_code is None or hd_root_return_code != 0:
      hd_root_failed_rc_files.append(hd_root_rc_file)
  print(f"Found {len(hd_root_failed_rc_files)} out of {len(hd_root_rc_files)} hd_root processes with non-zero or unknown return code")

  for hd_root_rc_file in hd_root_failed_rc_files:
    # tail hd_root log files if return code is non-zero or could not be read
    hd_root_log_dir_name = os.path.dirname(hd_root_rc_file)
    print("-------------------------------------------------------------------------------")
    print(f"hd_root return code: {return_code_signal_name(get_hd_root_return_code(hd_root_rc_file))} (from '{hd_root_rc_file}')")
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
  parser.add_argument("--run_working_dir", required = True,         help = "Working directory of the run")
  parser.add_argument("--nmb_lines",       type = int, default = 3, help = "Number of lines to print from the end of the log files; default: %(default)i")
  main(parser.parse_args())
