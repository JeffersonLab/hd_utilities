#!/usr/bin/env python3

"""Merges two run lists into a single run list with unique run numbers"""

from __future__ import annotations

import argparse


def main(args: argparse.Namespace) -> None:
  run_numbers = set()
  for file_path in (args.list_file_1_path, args.list_file_2_path):
    print(f"Reading run numbers from '{file_path}'")
    with open(file_path) as in_file:
      for line in in_file:
        run_numbers.add(int(line))
  print(f"Writing merged run numbers to '{args.merged_list_path}'")
  with open(args.merged_list_path, "w") as out_file:
    for run_number in sorted(run_numbers):
      out_file.write(f"{run_number}\n")


#TODO expand to merging N files
if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Merge two run list files into a single run list file.",
  )
  parser.add_argument("list_file_1_path", metavar = "list-file-1", help = "Path of first run list to merge")
  parser.add_argument("list_file_2_path", metavar = "list-file-2", help = "Path of second run list to merge")
  parser.add_argument("merged_list_path", metavar = "merged-list", help = "Path of merged run-list file")
  main(parser.parse_args())
