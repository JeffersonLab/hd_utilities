#!/usr/bin/env python3

"""Merges multiple run-list files into a single run-list file with unique run numbers"""

from __future__ import annotations

import argparse


def main(args: argparse.Namespace) -> None:
  run_numbers = set()
  print(f"Merging run numbers from {len(args.list_file_paths)} files into file '{args.merged_list_path}'")
  for file_path in args.list_file_paths:
    print(f"Reading run numbers from '{file_path}'")
    with open(file_path) as in_file:
      for line in in_file:
        run_numbers.add(int(line))
  print(f"Writing merged run numbers to '{args.merged_list_path}'")
  with open(args.merged_list_path, "w") as out_file:
    for run_number in sorted(run_numbers):
      out_file.write(f"{run_number}\n")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Merges multiple run-list files into a single run-list file with unique run numbers.",
  )
  parser.add_argument("merged_list_path", help = "Path of merged run-list file")
  parser.add_argument(
    "list_file_paths",
    nargs = "+",
    help = "Paths of run-list files to merge",
  )
  args = parser.parse_args()
  if len(args.list_file_paths) < 2:
    parser.error("provide at least two input run-list files after the path of the merged run-list file")
  main(args)
