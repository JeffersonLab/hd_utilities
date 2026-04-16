#!/usr/bin/env python3

"""swif2 job script that copies the full content of a source directory to a destination directory."""

from __future__ import annotations

import argparse
import glob
import os
import shutil
import subprocess
import sys
import time

# from script_job import print_command_line_arguments


#TODO import from script_job
def print_command_line_arguments(args: argparse.Namespace) -> None:
  """Prints all command-line arguments and their values and the git hash."""
  this_script_file_name = os.path.basename(sys.argv[0])  # get file name of script that was launched
  print("-------------------------------------------------------------------------------")
  print(f"Running script {this_script_file_name} with arguments:")
  max_arg_name_length = max(len(arg_name) for arg_name in vars(args).keys())
  for arg_name, arg_value in sorted(vars(args).items()):  # sort keys for stable, tidy output
    print(f"{arg_name:>{max_arg_name_length + 2}} : {arg_value}")
  this_script_dir = os.path.dirname(os.path.abspath(__file__))  #TODO why not use `sys.argv[0]` here?
  if os.path.isfile(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH"):
    deployed_git_hash = open(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH", "r", encoding = "utf-8").read().strip()
    print(f"Using launch scripts from git commit hash: {deployed_git_hash}")
  #TODO add case where the script is run from the git repo
  print("-------------------------------------------------------------------------------")


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
  print(f"!!! {shutil.which('swif2')=}")
  for src_file_path, dest_file_path in file_transfer_paths:
    # output_cmd = f"./.swif/swif2 output '{src_file_path}' '{dest_file_path}'"  #TODO for some reason, swif2 is not in path
    output_cmd = f"swif2 output '{src_file_path}' '{dest_file_path}'"
    print(output_cmd)
    subprocess.run(output_cmd, shell = True, check = False)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  print(f"Job script is running in directory: '{os.getcwd()}'")
  subprocess.run("ls -la", shell = True, check = False)

  define_swif2_output_files(args.src_dir_path, args.dest_dir_path)

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by job script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "swif2 job script that copies the full content of the given source directory to the given destination directory.",
  )
  parser.add_argument("--src_dir_path",  help = "Path to the source directory, the content of which will be copied to the destination directory")
  parser.add_argument("--dest_dir_path", help = "Path to the destination directory (usually on tape)")
  main(parser.parse_args())
