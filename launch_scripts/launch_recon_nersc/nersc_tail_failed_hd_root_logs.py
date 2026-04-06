#!/usr/bin/env python3
#NOTE this script needs to be compatible with Python 3.6

"""
Prints the last N lines of the stdout and stderr log files for hd_root processes with non-zero exit code.
"""

import argparse
from collections import deque
import functools
import glob
import os
import time
from typing import List

from script_job import (
  get_hd_root_return_code,
  print_command_line_arguments,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def tail(
  file_name: str,
  nmb_lines: int = 3
) -> List[str]:
  """Returns the last n lines of the given file."""
  with open(file_name, "r", encoding = "utf-8") as file:
    return list(deque(file, maxlen = nmb_lines))


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  # loop over files with hd_root exit codes
  hd_root_rc_files = glob.glob(f"{args.run_working_dir}/RUN??????/TASK????/FILE???/hd_root.rc")
  for hd_root_rc_file in hd_root_rc_files:
    hd_root_return_code = get_hd_root_return_code(hd_root_rc_file)
    if hd_root_return_code is None or hd_root_return_code != 0:
      # tail hd_root log files if return code is non-zero or could not be read
      hd_root_log_dir_name = os.path.dirname(hd_root_rc_file)
      print("-------------------------------------------------------------------------------")
      print(f"hd_root return code: {hd_root_return_code} (from '{hd_root_rc_file}')")
      for log_file in ("hd_root.out", "hd_root.err"):
        log_file_path = f"{hd_root_log_dir_name}/{log_file}"
        print(f"hd_root log file: '{log_file_path}'")
        for line in tail(log_file_path, args.nmb_lines):
          print(line, end = "")

  print("-------------------------------------------------------------------------------")
  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Print the last N lines of the stdout and stderr log files for hd_root processes with non-zero exit code.",
  )
  parser.add_argument("--run_working_dir", required = True,         help = "Working directory of the run")
  parser.add_argument("--nmb_lines",       type = int, default = 3, help = "Number of lines to print from the end of the log files; default: %(default)i")
  main(parser.parse_args())
