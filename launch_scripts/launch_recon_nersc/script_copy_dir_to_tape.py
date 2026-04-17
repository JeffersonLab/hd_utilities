#!/usr/bin/env python3

"""swif2 job script that copies the full content of a source directory to a destination directory using the reaping stage."""

from __future__ import annotations

import argparse
import glob
import os
import subprocess
import time

from script_job import print_command_line_arguments  # needs PYTHONPATH to include the directory where `script_job.py` is located


def get_transfer_file_paths(
  src_dir_path:  str,  # path to the source directory, the content of which will be copied to the destination directory
  dest_dir_path: str,  # path to the destination directory
) -> list[tuple[str, str]]:  # list of pairs of absolute local file paths and absolute destination file paths
  """Builds list of all files with local absolute paths and absolute destination paths."""
  # go into the directory to copy and get list of all files with relative paths in that directory
  os.chdir(src_dir_path)
  relative_file_paths: list[str] = [path for path in glob.glob("**/*", recursive = True) if os.path.isfile(path)]
  # build list of pairs of absolute local file paths and absolute destination file paths
  file_transfer_paths: list[tuple[str, str]] = []
  for relative_file_path in relative_file_paths:
    dest_file_path = f"{dest_dir_path}/{relative_file_path}"
    if not os.path.isfile(dest_file_path):  #TODO check that sizes match?
      file_transfer_paths.append((os.path.abspath(relative_file_path), dest_file_path))
    else:
      print(f"File '{dest_file_path}' already exists; skipping transfer")
  return file_transfer_paths


def define_swif2_output_files(
  src_dir_path:  str,  # path to the source directory, the content of which will be copied to the destination directory
  dest_dir_path: str,  # path to the destination directory
) -> None:
  """Registers files with swif2 for transfer back to tape."""
  file_transfer_paths: list[tuple[str, str]] = get_transfer_file_paths(src_dir_path, dest_dir_path)
  print(f"Transferring {len(file_transfer_paths)} files from '{src_dir_path}' to '{dest_dir_path}'")
  for src_file_path, dest_file_path in file_transfer_paths:
    output_cmd = f"swif2 output '{src_file_path}' '{dest_file_path}'"
    print(output_cmd)
    subprocess.run(output_cmd, shell = True, check = False)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  define_swif2_output_files(args.src_dir_path, args.dest_dir_path)

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "swif2 job script that copies the full content of the given source directory to the given destination directory using the reaping stage.",
  )
  parser.add_argument("--src_dir_path",  help = "Path to the source directory, the content of which will be copied to the destination directory")
  parser.add_argument("--dest_dir_path", help = "Path to the destination directory (usually on tape)")
  main(parser.parse_args())
