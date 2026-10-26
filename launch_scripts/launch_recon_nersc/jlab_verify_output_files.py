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
import glob
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


class FileErrorType(Enum):
  """Enum for errors encountered when verifying output files."""
  NO_EVIO_FILES      = auto()  # no EVIO files found for the run
  FAILED_EVIO_FILE   = auto()  # EVIO file failed processing
  RECON_FILE_MISSING = auto()  # expected recon file is missing
  LOG_FILE_MISSING   = auto()  # expected log file is missing

@dataclass
class FileErrorInfo:
  """Class holding information about an error encountered when verifying output files."""
  error:     FileErrorType
  path:      str         # full path of the file or directory associated with the error
  base_name: str | None  # base name of the output file associated with the error, if applicable


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  launch_name               = ensure_dict_value_exists(launch_config, "LAUNCH_NAME")
  run_number_list_file_path = ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  raw_data_root_dir_path    = ensure_dict_value_exists(launch_config, "RAW_DATA_ROOT")
  reco_data_dest_dir_path   = ensure_dict_value_exists(launch_config, "SWIF_HD_ROOT_OUTPUT_DEST_DIR")
  log_files_dest_dir_path   = ensure_dict_value_exists(launch_config, "SWIF_LOG_FILES_OUTPUT_DEST_DIR")

  run_numbers_from_list: list[int] = read_run_numbers_from_file(run_number_list_file_path)
  nmb_evio_files_found = 0
  errors: list[FileErrorInfo] = []
  for run_number in run_numbers_from_list:
    print()
    #TODO respect RCDB file count and exclusions
    evio_file_paths, evio_run_dir_path = get_raw_data_evio_file_paths_for_run(run_number, raw_data_root_dir_path)
    if not evio_file_paths:
      print(f"Run {run_number}: ERROR: no EVIO files found in '{evio_run_dir_path}'. Skipping.")
      errors.append(FileErrorInfo(error = FileErrorType.NO_EVIO_FILES, path = evio_run_dir_path, base_name = None))
      continue
    print(f"Run {run_number}: verifying output files for {len(evio_file_paths)} EVIO files in '{evio_run_dir_path}'")
    nmb_evio_files_found += len(evio_file_paths)
    for evio_file_path in evio_file_paths:
      evio_file_number = get_file_number_from_evio_file_name(evio_file_path)
      all_output_files_missing = True
      errors_for_evio_file: list[FileErrorInfo] = []
      # distinguish two error cases:
      # case 1: all output files are missing for the EVIO file because the corresponding `hd_root` process failed
      # case 2: only some output files are missing for the EVIO file
      for recon_subdir, (recon_file_base_name, recon_file_type) in RECON_SUBDIR_BASENAME_MAP.items():
        recon_file_path = f"{reco_data_dest_dir_path}/{recon_subdir}/{run_number:06d}/{recon_file_base_name}_{run_number:06d}_{evio_file_number:03d}.{recon_file_type}"
        if os.path.isfile(recon_file_path):
          all_output_files_missing = False
        else:
          errors_for_evio_file.append(FileErrorInfo(
            error     = FileErrorType.RECON_FILE_MISSING,
            path      = recon_file_path,
            base_name = recon_file_base_name,
          ))
      if all_output_files_missing:
        # case 1: only report failed EVIO file
        print(f"  File {evio_file_number:03d}: ERROR: EVIO file '{evio_file_path}' failed; all output files are missing")
        errors.append(FileErrorInfo(
          error     = FileErrorType.FAILED_EVIO_FILE,
          path      = evio_file_path,
          base_name = None,
        ))
      else:
        # case 2: report missing output files for the EVIO file
        for error_info in errors_for_evio_file:
          print(f"  File {evio_file_number:03d}: ERROR: missing recon file '{error_info.path}'")
        errors += errors_for_evio_file
        # check for missing log files corresponding to the EVIO file  #TODO this is slow; make it optional
        log_file_dir_path = f"{log_files_dest_dir_path}/job_info/{run_number:06d}/job_info_{run_number:06d}_{evio_file_number:03d}"
        expected_log_file_name_patterns = (
          "hd_root.err",
          "hd_root.out",
          "hd_root.rc",
          f"job_{run_number:06d}_*.out",
          f"job_{run_number:06d}.env",
          f"job_{run_number:06d}.hostname",
          f"job_{run_number:06d}.mounts",
          "node.cpuinfo",
          "node.env",
          "node.hostname",
          "node.mounts",
          "node.top",
          f"srun_{run_number:06d}.rc",
          f"task_{run_number:06d}_*.out",
        )
        # check whether at least 1 file matching each log-file pattern exists
        #NOTE this does not ensure that there is exactly
        #     1 `job_<run number>_<slurm ID>.out` file; also the
        #     number of `task_<run number>_<task ID>.out` files is not
        #     verified beyond ensuring that there is at least 1
        for log_file_name_pattern in expected_log_file_name_patterns:
          log_file_path_pattern = f"{log_file_dir_path}/{log_file_name_pattern}"
          matching_log_files = glob.glob(log_file_path_pattern)
          if not matching_log_files:
            print(f"  File {evio_file_number:03d}: ERROR: missing log file(s) matching pattern '{log_file_path_pattern}'")
            errors.append(FileErrorInfo(
              error     = FileErrorType.LOG_FILE_MISSING,
              path      = log_file_path_pattern,
              base_name = None,
            ))

  # tally all errors
  error_counts = Counter(((err.error, err.base_name) for err in errors))
  print(textwrap.dedent(f"""
    ============================================================================================
    Summary
    --------------------------------------------------------------------------------------------
  """).strip())
  print(f"  {error_counts[(FileErrorType.NO_EVIO_FILES,    None)]:5d} run(s) with no EVIO files found")
  print(f"  {error_counts[(FileErrorType.FAILED_EVIO_FILE, None)]:5d} EVIO file(s) failed and produced no output files")
  for recon_subdir, (recon_file_base_name, recon_file_type) in RECON_SUBDIR_BASENAME_MAP.items():
    print(f"  {error_counts[(FileErrorType.RECON_FILE_MISSING, recon_file_base_name)]:5d} missing '{recon_file_base_name}' output file(s)")
  print(f"  {error_counts[(FileErrorType.LOG_FILE_MISSING, None)]:5d} missing log file(s)")
  print("============================================================================================")

  failed_evio_file_paths = set(error.path for error in errors if error.error == FileErrorType.FAILED_EVIO_FILE)
  if len(failed_evio_file_paths) == 0:
    print(f"All {nmb_evio_files_found} found EVIO files were processed successfully")
  else:
    # write list of failed EVIO files that need to be reproduced
    print(f"{len(failed_evio_file_paths)} out of {nmb_evio_files_found} EVIO file(s) failed processing:")
    for failed_evio_file_path in sorted(failed_evio_file_paths):
      print(f"  {failed_evio_file_path}")
    #TODO add command-line argument to override file path
    failed_evio_list_file_path = f"./failed_evio_files.{launch_name}.list"
    print(f"\nWriting list of failed EVIO files to '{failed_evio_list_file_path}'")
    try:
      with open(failed_evio_list_file_path, mode = "x") as file:
        for failed_evio_file_path in sorted(failed_evio_file_paths):
          file.write(f"{failed_evio_file_path}\n")
    except FileExistsError:
      print(f"WARNING: file '{failed_evio_list_file_path}' already exists; skipping write")

  # write a list for each kind of missing recon file
  for recon_file_base_name, _ in RECON_SUBDIR_BASENAME_MAP.values():
    missing_recon_file_paths = set(
      error.path for error in errors
      if error.error == FileErrorType.RECON_FILE_MISSING and error.base_name == recon_file_base_name
    )
    if missing_recon_file_paths:
      missing_recon_file_list_file_path = f"./missing_recon_files.{launch_name}.{recon_file_base_name}.list"
      print(f"Writing {len(missing_recon_file_paths)} missing '{recon_file_base_name}' recon file(s) to '{missing_recon_file_list_file_path}':")
      try:
        with open(missing_recon_file_list_file_path, mode = "x") as file:
          for missing_recon_file_path in sorted(missing_recon_file_paths):
            file.write(f"{missing_recon_file_path}\n")
      except FileExistsError:
        print(f"WARNING: file '{missing_recon_file_list_file_path}' already exists; skipping write")

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
