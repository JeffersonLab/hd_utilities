#!/usr/bin/env python3

"""Verifies that content of a directory with prepared recon output was successfully copied to tape."""

from __future__ import annotations

import argparse
import functools
import os
import time

from script_copy_dir_to_tape import get_transfer_file_paths
from utilities import (
  ensure_dict_value_exists,
  get_file_crc32,
  print_command_line_arguments,
)
from utilities_dotenv import (
  get_config_dict_from_env_file,
  get_file_crc32_from_mss_stub,
  get_file_size_from_mss_stub,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  ver            = ensure_dict_value_exists(launch_config, "VER")
  reco_data_root = ensure_dict_value_exists(launch_config, "RECO_DATA_ROOT")

  recon_src_path    = os.path.abspath(args.recon_src_path)
  recon_subdir_name = os.path.basename(recon_src_path)
  # tape_dest_path    = "/volatile/halld/home/bgrube/test_copy_dir_to_tape"
  # tape_dest_path    = "/mss/halld/home/bgrube/test/test_copy_dir_to_tape"
  tape_dest_path    = f"{reco_data_root}/{ver}/{recon_subdir_name}"
  file_transfer_paths: list[tuple[str, str]] = get_transfer_file_paths(recon_src_path, tape_dest_path, do_overwrite = True)
  if len(file_transfer_paths) == 0:
    print(f"Found no files to transfer from '{recon_src_path}' to '{tape_dest_path}'")
    return
  print(f"Verifying that all {len(file_transfer_paths)} files in '{recon_src_path}' were successfully copied to '{tape_dest_path}'")
  files_missing:     list[str] = []
  files_wrong_size:  list[str] = []
  files_wrong_crc32: list[str] = []
  for file_index, (src_file_path, dest_file_path) in enumerate(file_transfer_paths):
    print(f"Checking file [{file_index + 1:6d}/{len(file_transfer_paths):6d}]: '{src_file_path}' -> '{dest_file_path}'")
    # check if file exists at destination path
    if not os.path.isfile(dest_file_path):
      print(f"ERROR: file '{dest_file_path}' does not exist")
      files_missing.append(dest_file_path)
      continue
    # check if file size at destination path matches the size of the source file
    src_file_size  = os.path.getsize(src_file_path)
    dest_file_size = (
        get_file_size_from_mss_stub(dest_file_path) if dest_file_path.startswith("/mss")
        else os.path.getsize(dest_file_path)
      )
    if src_file_size != dest_file_size:
      print(f"ERROR: file '{dest_file_path}' has size {dest_file_size} bytes, but expected {src_file_size} bytes based on source file '{src_file_path}'")
      files_wrong_size.append(dest_file_path)
      continue
    if args.verify_file_content:
      # check if file at destination path matches the CRC32 checksum of the source file
      src_file_crc32  = get_file_crc32(src_file_path)
      dest_file_crc32 = (
          get_file_crc32_from_mss_stub(dest_file_path) if dest_file_path.startswith("/mss")
          else get_file_crc32(dest_file_path)
        )
      if src_file_crc32 != dest_file_crc32:
        print(f"ERROR: file '{dest_file_path}' has CRC32 {dest_file_crc32}, but expected {src_file_crc32} based on source file '{src_file_path}'")
        files_wrong_crc32.append(dest_file_path)
        continue
    # file exists and has correct size (and CRC32, if enabled); assume it was successfully copied to tape
    if args.delete_verified_files:
      print(f"Deleting verified file '{src_file_path}'")
      os.remove(src_file_path)

  print("-------------------------------------------------------------------------------")
  if len(files_missing) == 0 and len(files_wrong_size) == 0 and len(files_wrong_crc32) == 0:
    print("All files were successfully copied to tape!")
  else:
    if len(files_missing) > 0:
      print(f"{len(files_missing)} files are missing on tape:")
      for missing_file in files_missing:
        print(f"  {missing_file}")
    if len(files_wrong_size) > 0:
      print(f"{len(files_wrong_size)} files have incorrect size on tape:")
      for wrong_size_file in files_wrong_size:
        print(f"  {wrong_size_file}")
    if len(files_wrong_crc32) > 0:
      print(f"{len(files_wrong_crc32)} files have incorrect CRC32 checksum on tape:")
      for wrong_crc32_file in files_wrong_crc32:
        print(f"  {wrong_crc32_file}")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Verifies that content of a directory with prepared recon output was successfully copied to tape.",
  )
  parser.add_argument("--launch_env_file",       default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--verify_file_content",   action = "store_true",    help = "Verify the content of files after copying by comparing their CRC32 checksums; default: false")
  parser.add_argument("--delete_verified_files", action = "store_true",    help = "Delete verified files from source path after verification; default: false")
  parser.add_argument("--recon_src_path",                                  help = "Path to first-level subdirectory in the prepared reconstruction directory, the content of which will be verified")
  args = parser.parse_args()
  main(args)
