#!/usr/bin/env python3

"""
Calculates the total size of the REST files in the given MSS directory.
"""

from __future__ import annotations

import argparse
import functools
import glob
import os
import time

from utilities import (
  print_command_line_arguments,
)
from utilities_dotenv import (
  get_file_size_from_mss_stub,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  # get list of REST file paths in the given MSS directory
  if not args.mss_directory.startswith("/mss/"):
    raise ValueError(f"MSS directory '{args.mss_directory}' does not start with '/mss/'")
  rest_file_paths: list[str] = sorted(glob.glob(f"{args.mss_directory}/*/dana_rest_??????_???.hddm"))
  if not rest_file_paths:
    raise ValueError(f"Pattern '{args.mss_directory}/*/dana_rest_??????_???.hddm' returned no files")
  print(f"Found {len(rest_file_paths)} REST files in MSS directory '{args.mss_directory}'")

  # load list of files to exclude from size calculation
  file_labels_to_exclude = set()
  if args.exclude_evio_file_list is not None:
    if not os.path.isfile(args.exclude_evio_file_list):
      raise ValueError(f"File '{args.exclude_evio_file_list}' does not exist")
    evio_file_paths_to_exclude: list[str] = []
    with open(args.exclude_evio_file_list, "r") as file:
      evio_file_paths_to_exclude = [line.strip() for line in file if line.strip()]
    file_labels_to_exclude = set(
      os.path.basename(evio_file_path).split(".")[0][-10:]  # get file label, e.g. "123456_123" from "hd_rawdata_123456_123.evio"
      for evio_file_path in evio_file_paths_to_exclude
    )

  total_rest_size_tb = 0.0
  count_rest_files_excluded = 0
  for rest_file_path in rest_file_paths:
    if os.path.basename(rest_file_path).split(".")[0][-10:] not in file_labels_to_exclude:
      total_rest_size_tb += get_file_size_from_mss_stub(rest_file_path) / 1024**4
    else:
      count_rest_files_excluded += 1
      print(f"    Excluding REST file '{rest_file_path}' from size calculation")
  print(f"Total size of REST files: {total_rest_size_tb:.2f} TB")
  print(f"Number of REST files excluded: {count_rest_files_excluded}")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Calculates the total size of the REST files in the given MSS directory.",
  )
  parser.add_argument("mss_directory",            help = "Path to the MSS directory containing the REST files")
  parser.add_argument("--exclude_evio_file_list", help = "Path to list with EVIO file paths used to determine the REST files to exclude from the size calculation")
  args = parser.parse_args()
  main(args)
