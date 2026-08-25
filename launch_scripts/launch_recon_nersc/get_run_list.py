#!/usr/bin/env python3

"""Generates list of runs and files to process for a given run period."""
# if GlueX environment is not set up, use `PYTHONPATH="/group/halld/Software/builds/Linux_Alma9-x86_64-gcc11.5.0/rcdb/rcdb_0.09.01/python:${PYTHONPATH}" ./get_run_list.py`

from __future__ import annotations

import argparse
from dataclasses import dataclass
from enum import Enum, auto
import functools
import glob
import os
import textwrap
import time

import rcdb

from utilities import (
  ensure_dict_value_exists,
  print_command_line_arguments,
  read_run_numbers_from_file,
)
from utilities_dotenv import get_config_dict_from_env_file


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


class EvioFileErrorType(Enum):
  """Enum for failure modes when getting list of EVIO files."""
  NO_RCDB_FILES_COUNT  = auto()
  NO_FILES_FOUND       = auto()
  MISSING_EVIO_FILE    = auto()
  UNEXPECTED_EVIO_FILE = auto()

@dataclass
class EvioFileError:
  """Class holding error information when getting list of EVIO files."""
  errorType: EvioFileErrorType
  detail:    str | None = None

def get_evio_file_paths_for_run(
  run_info:      rcdb.model.Run,  # RCDB run info object to read information from
  raw_data_root: str,             # root dir for EVIO files
) -> tuple[list[str], list[EvioFileError]]:
  """Gets list of EVIO files for the given RCDB run-info object."""
  run_number              = int(run_info.number)
  raw_data_dir_run        = f"{raw_data_root}/Run{run_number:06d}"
  evio_files_count        = run_info.get_condition_value("evio_files_count")  # get number of evio files for this run; file numbers are in range 0 to evio_files_count - 1
  evio_pattern            = f"{raw_data_dir_run}/hd_rawdata_{run_number:06d}_???.evio"
  evio_file_paths_pattern = sorted(glob.glob(evio_pattern))
  evio_file_paths: list[str]           = []
  rcdb_errors:     list[EvioFileError] = []
  if evio_files_count is None:
    print(f"WARNING: RCDB does not contain a value for `evio_files_count` for run {run_number}")
    rcdb_errors.append(EvioFileError(errorType = EvioFileErrorType.NO_RCDB_FILES_COUNT))
    print(f"Using files matching '{evio_pattern}' instead")
    evio_file_paths = evio_file_paths_pattern
  else:
    evio_last_files_excluded = run_info.get_condition_value("evio_last_files_excluded")  # get number of evio files to exclude from the end of the run
    evio_files_count -= evio_last_files_excluded or 0
    print(f"Ensuring all {evio_files_count:3d} EVIO files for run {run_number} exist"
          + (f" (excluding {evio_last_files_excluded} last files)" if evio_last_files_excluded is not None else ""))
    for evio_file_index in range(evio_files_count):
      evio_file_path = f"{raw_data_dir_run}/hd_rawdata_{run_number:06d}_{evio_file_index:03d}.evio"
      if not os.path.isfile(evio_file_path):
        print(f"WARNING: expected EVIO file '{evio_file_path}' does not exist")
        rcdb_errors.append(EvioFileError(errorType = EvioFileErrorType.MISSING_EVIO_FILE, detail = evio_file_path))
      else:
        evio_file_paths.append(evio_file_path)
        evio_file_paths_pattern.remove(evio_file_path)  # remove existing files from list found by pattern matching
    # check consistency with pattern matching
    if len(evio_file_paths_pattern) > 0:
      for evio_file_path in evio_file_paths_pattern:
        print(f"WARNING: unexpected EVIO file '{evio_file_path}'")
        rcdb_errors.append(EvioFileError(errorType = EvioFileErrorType.UNEXPECTED_EVIO_FILE, detail = evio_file_path))
  if len(evio_file_paths) == 0:
    print(f"WARNING: could not find any EVIO files for run {run_number}")
    rcdb_errors.append(EvioFileError(errorType = EvioFileErrorType.NO_FILES_FOUND))
    return [], rcdb_errors
  return evio_file_paths, rcdb_errors


def get_evio_file_paths(
  db:            rcdb.RCDBProvider,  # RCDB object to read run information from
  run_period:    str,                # e.g. '2022-05'
  run_list:      list[int],          # list of runs to process
  raw_data_root: str,                # root dir for EVIO files
) -> dict[int, list[str]]:
  """Gets the list of EVIO files for each run in the given list of runs, and also tallies any errors encountered in the process."""
  print(f"Searching for raw-data .evio files in '{raw_data_root}'")
  evio_file_paths_per_run: dict[int, list[str]] = {}
  error_counts: dict[EvioFileErrorType, int] = {errorType : 0 for errorType in EvioFileErrorType}
  for run_number in run_list:
    # get run information from RCDB
    run_info = db.get_run(run_number)
    # get list of evio files for this run, and any errors encountered in the process
    evio_file_paths: list[str]
    rcdb_errors:     list[EvioFileError]
    evio_file_paths, rcdb_errors = get_evio_file_paths_for_run(run_info, raw_data_root)
    evio_file_paths_per_run[run_number] = evio_file_paths
    # tally any errors returned for this run
    for err in rcdb_errors:
      error_counts[err.errorType] += 1
  total_nmb_evio_files = sum(len(file_paths) for file_paths in evio_file_paths_per_run.values())
  print(textwrap.dedent(f"""
    ============================================================================================
    Summary
    --------------------------------------------------------------------------------------------
                Run period: {run_period}
            Number of runs: {len(evio_file_paths_per_run)}
      Number of EVIO files: {total_nmb_evio_files}
    --------------------------------------------------------------------------------------------
    Error summary
    --------------------------------------------------------------------------------------------
      {error_counts[EvioFileErrorType.NO_RCDB_FILES_COUNT ]:5d} runs with no `evio_files_count` value in RCDB
              for these runs the number of EVIO files was estimated by counting files in `/mss`
      {error_counts[EvioFileErrorType.NO_FILES_FOUND      ]:5d} runs with no EVIO files found
              for these runs no jobs will be submitted
      {error_counts[EvioFileErrorType.MISSING_EVIO_FILE   ]:5d} EVIO files not found
              for these files no jobs will be submitted
      {error_counts[EvioFileErrorType.UNEXPECTED_EVIO_FILE]:5d} unexpected EVIO files found
              for these files jobs will be submitted
    ============================================================================================
    """))
  return evio_file_paths_per_run


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period           =     ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_min       = int(ensure_dict_value_exists(launch_config, "RUN_NUMBER_MIN"))
  run_number_max       = int(ensure_dict_value_exists(launch_config, "RUN_NUMBER_MAX"))
  rcdb_query           =     ensure_dict_value_exists(launch_config, "RCDB_QUERY")
  raw_data_root        =     ensure_dict_value_exists(launch_config, "RAW_DATA_ROOT")
  run_number_list_file =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE")
  evio_path_list_file  =     ensure_dict_value_exists(launch_config, "EVIO_PATH_LIST_FILE")

  print(f"Getting run list for run period {run_period}")
  rcdb_host = 'hallddb.jlab.org'
  rcdb_user = 'rcdb'
  db = rcdb.RCDBProvider(f'mysql://{rcdb_user}@{rcdb_host}/rcdb2')
  run_list: list[int] = []
  if args.override_run_list is None:
    print(f"Getting run list from RCDB using {run_number_min} <= run number <= {run_number_max} and query '{rcdb_query}'")
    run_list = sorted([int(run.number) for run in db.select_runs(rcdb_query, run_number_min, run_number_max)])
  else:
    run_list = read_run_numbers_from_file(args.override_run_list)
  print(f"Found {len(run_list)} runs")

  evio_file_paths_per_run: dict[int, list[str]] = get_evio_file_paths(db, run_period, run_list, raw_data_root)
  print(f"Writing list of run numbers to './{run_number_list_file}'")
  try:
    with open(f"./{run_number_list_file}", mode = "x") as file:
      for run_number in sorted(evio_file_paths_per_run.keys()):
        if len(evio_file_paths_per_run[run_number]) > 0:
          file.write(f"{run_number:06d}\n")
  except FileExistsError:
    print(f"WARNING: file './{run_number_list_file}' already exists; skipping write")

  print(f"Writing list of EVIO file paths to './{evio_path_list_file}'")
  try:
    with open(f"./{evio_path_list_file}", mode = "x") as file:
      for run_number in sorted(evio_file_paths_per_run.keys()):
        for evio_file in sorted(evio_file_paths_per_run[run_number]):
          file.write(f"{evio_file}\n")
  except FileExistsError:
    print(f"WARNING: file './{evio_path_list_file}' already exists; skipping write")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Generates list of runs and files to process for a given run period.",
  )
  parser.add_argument("launch_env_file",     help = "Path to .env file defining the configuration variables of the reconstruction launch")
  parser.add_argument("--override-run-list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
