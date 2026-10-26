#!/usr/bin/env python3

"""
Checks completeness of reconstruction output files written to JLab tape.
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
from enum import Enum, auto
import functools
import os
import textwrap
import time

from FileTransferMapGenerator import RECON_SUBDIR_BASENAME_MAP
from utilities import (
  ensure_dict_value_exists,
  get_file_number_from_evio_file_name,
  get_raw_data_evio_file_paths_for_run,
  print_command_line_arguments,
  read_run_numbers_from_file,
)
from utilities_dotenv import get_config_dict_from_env_file


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


class FileStatusType(Enum):
  """Enum for non-standard situations when getting EVIO file."""
  NO_EVIO_FILES = auto()
  FILE_MISSING  = auto()

@dataclass
class FileErrorInfo:
  """Class holding information about a missing output file."""
  status:         FileStatusType
  path:           str
  base_name:      str
  evio_file_path: str | None = None


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_number_list_file_path = ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  raw_data_root_dir_path    = ensure_dict_value_exists(launch_config, "RAW_DATA_ROOT")
  reco_data_dest_dir_path   = ensure_dict_value_exists(launch_config, "SWIF_HD_ROOT_OUTPUT_DEST_DIR")
  swif_workflow             = ensure_dict_value_exists(launch_config, "SWIF_WORKFLOW")

  run_numbers_from_list: list[int] = read_run_numbers_from_file(run_number_list_file_path)
  errors: list[FileErrorInfo] = []
  nmb_evio_files_found = 0
  for run_number in run_numbers_from_list:
    print()
    #TODO respect RCDB file count and exclusions
    evio_file_paths, evio_run_dir_path = get_raw_data_evio_file_paths_for_run(run_number, raw_data_root_dir_path)
    if not evio_file_paths:
      print(f"ERROR: no EVIO files found in '{evio_run_dir_path}'. Skipping.")
      errors.append(FileErrorInfo(status = FileStatusType.NO_EVIO_FILES, path = evio_run_dir_path, base_name = ""))
      continue
    print(f"Checking output files for {len(evio_file_paths)} EVIO files in '{evio_run_dir_path}'")
    nmb_evio_files_found += len(evio_file_paths)
    for evio_file_path in evio_file_paths:
      evio_file_number = get_file_number_from_evio_file_name(evio_file_path)
      for recon_subdir, (recon_file_base_name, recon_file_type) in RECON_SUBDIR_BASENAME_MAP.items():
        recon_file_path = f"{reco_data_dest_dir_path}/{recon_subdir}/{run_number:06d}/{recon_file_base_name}_{run_number:06d}_{evio_file_number:03d}.{recon_file_type}"
        if not os.path.isfile(recon_file_path):
          print(f"  ERROR: missing recon file: {recon_file_path}")
          errors.append(FileErrorInfo(status = FileStatusType.FILE_MISSING, path = recon_file_path, base_name = recon_file_base_name, evio_file_path = evio_file_path))
      #TODO check completeness of log files

  # tally all errors
  error_counts = Counter(((err.status, err.base_name) for err in errors))
  print(textwrap.dedent(f"""
    ============================================================================================
    Summary
    --------------------------------------------------------------------------------------------
  """).strip())
  print(f"  {error_counts[(FileStatusType.NO_EVIO_FILES, '')]:5d} run(s) with no EVIO files found")
  for recon_subdir, (recon_file_base_name, recon_file_type) in RECON_SUBDIR_BASENAME_MAP.items():
    print(f"  {error_counts[(FileStatusType.FILE_MISSING, recon_file_base_name)]:5d} missing '{recon_file_base_name}' output file(s)")
  print("============================================================================================")

  #TODO write out lists for all missing files by type; excluding failed EVIO files?
  # write list of EVIO files that need to be reproduced
  required_output_base_names = {"dana_rest", "hd_root"}  # if any of these output files is missing, the EVIO file is flagged as failed  #TODO test that all files are missing
  failed_evio_file_paths = set()
  for err in errors:
    if err.status == FileStatusType.FILE_MISSING and err.base_name in required_output_base_names:
      failed_evio_file_paths.add(err.evio_file_path)
  if len(failed_evio_file_paths) == 0:
    print(f"All {nmb_evio_files_found} found EVIO files were processed successfully")
  else:
    print(f"{len(failed_evio_file_paths)} out of {nmb_evio_files_found} EVIO file(s) failed processing; missing {required_output_base_names} files:")
    for failed_evio_file_path in sorted(failed_evio_file_paths):
      print(f"  {failed_evio_file_path}")
    #TODO: add command-line argument to override file path
    failed_evio_list_file_path = f"./{swif_workflow}.failed_evio_files.list"  #TODO improve naming
    print(f"Writing list of failed EVIO files to '{failed_evio_list_file_path}'")
    try:
      with open(failed_evio_list_file_path, mode = "x") as file:
        for failed_evio_file_path in sorted(failed_evio_file_paths):
          file.write(f"{failed_evio_file_path}\n")
    except FileExistsError:
      print(f"WARNING: file '{failed_evio_list_file_path}' already exists; skipping write")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Checks completeness of reconstruction output files written to JLab tape.",
  )
  parser.add_argument(metavar = "launch-env-file", dest = "launch_env_file", help = "Path to .env file defining the configuration variables of the reconstruction launch")
  parser.add_argument("--override-run-list",                                 help = "Path to run-number list file to use instead the one defined in .env file")
  args = parser.parse_args()
  main(args)
