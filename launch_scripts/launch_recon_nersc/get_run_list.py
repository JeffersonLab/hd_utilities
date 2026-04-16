#!/usr/bin/env python3

"""Generates list of runs and files to process for a given run period."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from enum import Enum, auto
import glob
import os
import time

import rcdb

from script_job import print_command_line_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  read_run_numbers_from_file,
)

class EvioFilesErrorKind(Enum):
  """Enum for failure modes when getting list of EVIO files."""
  NO_RCDB_FILES_COUNT = auto()
  NO_FILES_FOUND      = auto()
  MISSING_EVIO_FILE   = auto()

@dataclass
class EvioFilesError:
  """Class holding error information when getting list of EVIO files."""
  kind:   EvioFilesErrorKind
  detail: str | None = None

def get_evio_files_for_run(
  run_info:           rcdb.model.Run,  # RCDB run info object to read information from
  swif_raw_data_root: str,             # root dir for EVIO files
) -> tuple[list[str], list[EvioFilesError]]:
  """Gets list of EVIO files for the given RCDB run-info object."""
  run_number = int(run_info.number)
  raw_data_dir_run = f"{swif_raw_data_root}/Run{run_number:06d}"
  evio_files_count = run_info.get_condition_value("evio_files_count")  # get number of evio files for this run; file numbers are in range 0 to evio_files_count - 1
  rcdb_errors: list[EvioFilesError] = []
  if evio_files_count is None:
    print(f"WARNING: RCDB does not contain a value for `evio_files_count` for run {run_number}")
    rcdb_errors.append(EvioFilesError(kind = EvioFilesErrorKind.NO_RCDB_FILES_COUNT))
    evio_pattern = f"{raw_data_dir_run}/hd_rawdata_{run_number:06d}_???.evio"
    print(f"Counting files matching '{evio_pattern}' instead")
    evio_files_count = len(glob.glob(evio_pattern))
  if evio_files_count is None or evio_files_count == 0:
    print(f"WARNING: could not find any EVIO files for run {run_number}")
    rcdb_errors.append(EvioFilesError(kind = EvioFilesErrorKind.NO_FILES_FOUND))
    return [], rcdb_errors
  print(f"Ensuring all {evio_files_count:3d} EVIO files for run {run_number} exist")
  evio_files: list[str] = []
  for evio_file_index in range(evio_files_count):
    evio_file_path = f"{raw_data_dir_run}/hd_rawdata_{run_number:06d}_{evio_file_index:03d}.evio"
    if not os.path.isfile(evio_file_path):
      print(f"WARNING: expected EVIO file '{evio_file_path}' does not exist")
      rcdb_errors.append(EvioFilesError(kind = EvioFilesErrorKind.MISSING_EVIO_FILE, detail = evio_file_path))
    else:
      evio_files.append(evio_file_path)
  return evio_files, rcdb_errors


def get_evio_files(
  db:                 rcdb.RCDBProvider,  # RCDB object to read run information from
  run_period:         str,                # e.g. '2022-05'
  run_list:           list[int],          # list of runs to process
  swif_raw_data_root: str,                # root dir for EVIO files
) -> dict[int, list[str]]:
  """Gets the list of EVIO files for each run in the given list of runs, and also tallies any errors encountered in the process."""
  print(f"Searching for raw-data .evio files in '{swif_raw_data_root}'")
  evio_files_per_run: dict[int, list[str]] = {}
  error_counts: dict[EvioFilesErrorKind, int] = {kind : 0 for kind in EvioFilesErrorKind}
  for run_number in run_list:
    # get run information from RCDB
    run_info = db.get_run(run_number)
    # get list of evio files for this run, and any errors encountered in the process
    evio_files:  list[str]
    rcdb_errors: list[EvioFilesError]
    evio_files, rcdb_errors = get_evio_files_for_run(run_info, swif_raw_data_root)
    evio_files_per_run[run_number] = evio_files
    # tally any errors returned for this run
    for err in rcdb_errors:
      error_counts[err.kind] += 1
  total_nmb_evio_files = sum(len(files) for files in evio_files_per_run.values())
  print("============================================================================================")
  print("Summary")
  print("--------------------------------------------------------------------------------------------")
  print(f"            Run period: {run_period}")
  print(f"        Number of runs: {len(evio_files_per_run)}")
  print(f"  Number of EVIO files: {total_nmb_evio_files}")
  print("--------------------------------------------------------------------------------------------")
  print("Error summary")
  print("--------------------------------------------------------------------------------------------")
  print(f"  {error_counts[EvioFilesErrorKind.NO_RCDB_FILES_COUNT]:5d} runs with no `evio_files_count value` in RCDB")
  print(f"          for these runs the number of EVIO files was estimated by counting files in `/mss`")
  print(f"  {error_counts[EvioFilesErrorKind.NO_FILES_FOUND     ]:5d} runs with no EVIO files found")
  print(f"          for these runs no jobs will be submitted")
  print(f"  {error_counts[EvioFilesErrorKind.MISSING_EVIO_FILE  ]:5d} EVIO files not found")
  print(f"          for these files no jobs will be submitted")
  print("============================================================================================")
  return evio_files_per_run


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period           =     ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_min       = int(ensure_dict_value_exists(launch_config, "RUN_NUMBER_MIN"))
  run_number_max       = int(ensure_dict_value_exists(launch_config, "RUN_NUMBER_MAX"))
  rcdb_query           =     ensure_dict_value_exists(launch_config, "RCDB_QUERY")
  swif_raw_data_root   =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  run_number_list_file =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE")

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

  evio_files_per_run: dict[int, list[str]] = get_evio_files(db, run_period, run_list, swif_raw_data_root)
  print(f"Writing list of run numbers to './{run_number_list_file}'")
  try:
    with open(f"./{run_number_list_file}", mode = "x") as file:
      for run_number in sorted(evio_files_per_run.keys()):
        if len(evio_files_per_run[run_number]) > 0:
          file.write(f"{run_number}\n")
  except FileExistsError:
    print(f"WARNING: file './{run_number_list_file}' already exists; skipping write")

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Generates list of runs and files to process for a given run period.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
