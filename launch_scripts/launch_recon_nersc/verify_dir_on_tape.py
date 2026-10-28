#!/usr/bin/env python3

"""Verifies that content of a directory was successfully copied to tape."""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
from enum import Enum, auto
import functools
import os
import textwrap
import time

from script_job_copy_dir_to_tape import get_transfer_file_paths
from utilities import (
  get_file_crc32,
  print_command_line_arguments,
)
from utilities_dotenv import (
  get_file_crc32_from_mss_stub,
  get_file_size_from_mss_stub,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


class FileErrorType(Enum):
  """Enum for errors encountered when verifying files on tape."""
  MISSING     = auto()  # expected file is missing on tape
  WRONG_SIZE  = auto()  # file on tape has wrong size
  WRONG_CRC32 = auto()  # file on tape has wrong CRC32 checksum

@dataclass
class FileErrorInfo:
  """Class holding information about an error encountered when verifying files on tape."""
  error:     FileErrorType
  path:      str  # full destination path on tape of the file associated with the error


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)

  src_dir_path  = os.path.abspath(args.src_dir_path)
  dest_dir_path = args.dest_dir_path
  print(f"Building list of files in '{src_dir_path}' and corresponding tape files in '{dest_dir_path}' to verify")
  file_transfer_paths: list[tuple[str, str]] = get_transfer_file_paths(src_dir_path, dest_dir_path, do_overwrite = True)
  if len(file_transfer_paths) == 0:
    print(f"Found no files in '{src_dir_path}'")
    return
  print(f"Verifying that all {len(file_transfer_paths)} files in '{src_dir_path}' were successfully copied to '{dest_dir_path}'")
  nmb_file_transfers = len(file_transfer_paths)
  max_nmb_digits     = len(str(nmb_file_transfers))
  errors: list[FileErrorInfo] = []
  for file_index, (src_file_path, dest_file_path) in enumerate(file_transfer_paths):
    print(f"Checking file [{file_index + 1:{max_nmb_digits}d}/{nmb_file_transfers:{max_nmb_digits}d}]: '{src_file_path}' -> '{dest_file_path}'")
    # check if file exists at destination path
    if not os.path.isfile(dest_file_path):
      print(f"ERROR: file '{dest_file_path}' does not exist")
      errors.append(FileErrorInfo(error = FileErrorType.MISSING, path = dest_file_path))
      continue
    # check if file size at destination path matches the size of the source file
    src_file_size  = os.path.getsize(src_file_path)
    dest_file_size = (
      get_file_size_from_mss_stub(dest_file_path) if dest_file_path.startswith("/mss")
      else os.path.getsize(dest_file_path)
    )
    if src_file_size != dest_file_size:
      print(f"ERROR: file '{dest_file_path}' has size {dest_file_size} bytes, but expected {src_file_size} bytes based on source file '{src_file_path}'")
      errors.append(FileErrorInfo(error = FileErrorType.WRONG_SIZE, path = dest_file_path))
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
        errors.append(FileErrorInfo(error = FileErrorType.WRONG_CRC32, path = dest_file_path))
        continue
    # file exists and has correct size (and CRC32, if enabled); assume it was successfully copied to tape
    if args.delete_verified_files:
      print(f"Deleting verified file '{src_file_path}'")
      os.remove(src_file_path)

  # tally all errors
  error_counts = Counter((error.error for error in errors))
  print(textwrap.dedent(f"""
    ===============================================================================
    Summary
    -------------------------------------------------------------------------------
  """).strip())
  print(f"  {error_counts[FileErrorType.MISSING]:5d} missing file(s)")
  print(f"  {error_counts[FileErrorType.WRONG_SIZE]:5d} file(s) with wrong size")
  if args.verify_file_content:
    print(f"  {error_counts[FileErrorType.WRONG_CRC32]:5d} file(s) with wrong CRC32")
  print("===============================================================================")


  for error_type, label in (
    (FileErrorType.MISSING,     "files missing on tape:"),
    (FileErrorType.WRONG_SIZE,  "files with incorrect size on tape:"),
    (FileErrorType.WRONG_CRC32, "files with incorrect CRC32 checksum on tape:"),
  ):
    error_file_paths = [error.path for error in errors if error.error == error_type]
    if error_file_paths:
      print(f"{len(error_file_paths)} {label}:")
      for error_file_path in error_file_paths:
        print(f"  {error_file_path}")
      print("-------------------------------------------------------------------------------")

  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Verifies that content of a directory was successfully copied to tape.",
  )
  parser.add_argument("--src-dir-path",            required = True,          help = "Path to source directory")
  parser.add_argument("--dest-dir-path",           required = True,          help = "Path to destination directory, the content of which will be verified against the source directory")
  parser.add_argument("--verify-file-content",     action = "store_true",    help = "Verify the content of files after copying by comparing their CRC32 checksums (slow!); default: false")
  parser.add_argument("--delete-verified-files",   action = "store_true",    help = "Delete verified files from source path after verification; default: false")
  args = parser.parse_args()
  main(args)
