"""
Collection of utility functions for the scripts in this directory.
"""

from __future__ import annotations

import glob
import os
import subprocess
from typing import TypeVar

import dotenv


KeyType   = TypeVar("KeyType",   bound = object)  # any not-None type
ValueType = TypeVar("ValueType", bound = object)  # any not-None type
def ensure_dict_value_exists(
  dictionary: dict[KeyType, ValueType | None],
  key:        KeyType,
) -> ValueType:
  """Ensure that value for the given key exists in the dictionary and has a non-None value; return the value."""
  value = dictionary[key]
  if value is None:
    raise ValueError(f"Missing value for key '{key}'")
  return value


def get_config_dict_from_env_file(env_file: str) -> dict[str, str | None]:
  """Load a .env file with the production parameters and return its contents as a dictionary."""
  print(f"Reading production parameters from .env file '{env_file}'")
  config: dict[str, str | None] = dotenv.dotenv_values(env_file)
  assert config, f"Failed to load .env file from '{env_file}'"
  if "NERSC_NMB_PROCESSES_PER_TASK" in config and config["NERSC_NMB_PROCESSES_PER_TASK"] is not None:
    # NERSC_NMB_PROCESSES_PER_TASK is actually an expression of the form `$(echo "256 / 32" | bc)` that needs to evaluated
    nersc_nmb_processes_per_task = config["NERSC_NMB_PROCESSES_PER_TASK"][2:-1]  # remove the leading `$(` and trailing `)` to get the inner expression
    nersc_nmb_processes_per_task = subprocess.check_output([nersc_nmb_processes_per_task], shell = True, text = True)
    config["NERSC_NMB_PROCESSES_PER_TASK"] = nersc_nmb_processes_per_task
  return config


def get_file_size_from_mss_stub(mss_file_path: str) -> int:
  """Extract the file size in bytes from the 'size' field in the MSS stub file."""
  assert mss_file_path.startswith("/mss"), f"File '{mss_file_path}' must be an `/mss` path"
  mss_stub = dotenv.dotenv_values(mss_file_path)
  assert mss_stub, f"Failed to load MSS stub file from '{mss_file_path}'"
  size_bytes = int(ensure_dict_value_exists(mss_stub, "size"))
  return size_bytes


def get_evio_file_paths_for_run(
  run_number:    int,
  raw_data_root: str,
) -> list[str]:
  """Get the list of EVIO file paths for the given run number and raw data root directory."""
  evio_run_dir = f"{raw_data_root}/Run{run_number:06d}"
  evio_file_paths: list[str] = sorted(glob.glob(f"{evio_run_dir}/hd_rawdata_{run_number:06d}_???.evio"))
  return evio_file_paths


def get_job_size(
  run_number:             int,
  raw_data_root:          str,
  nmb_processes_per_task: int,
) -> tuple[int, int, int, float] :
  """Calculate the number of files, number of tasks required, number of processes in last task, and total raw-data size in GB for the given run."""
  evio_file_paths = get_evio_file_paths_for_run(run_number, raw_data_root)
  nmb_files = len(evio_file_paths)
  nmb_tasks = (nmb_files + nmb_processes_per_task - 1) // nmb_processes_per_task
  nmb_remainder_processes = nmb_files % nmb_processes_per_task  # number of processes on last
  total_evio_size_gb = float(sum(get_file_size_from_mss_stub(file_path) for file_path in evio_file_paths)) / 1024**3
  return nmb_files, nmb_tasks, nmb_remainder_processes, total_evio_size_gb


def read_run_numbers_from_file(run_number_list_file: str) -> list[int]:
  """Reads a list of run numbers from the given file. The file is expected to contain one run number per line, and may contain blank lines which are ignored."""
  print(f"Reading list of run numbers from file '{run_number_list_file}'")
  run_numbers: list[int] = []
  with open(run_number_list_file) as file:
    run_numbers = [int(line.strip()) for line in file if line.strip()]  # skip blank lines
  return run_numbers


#TODO is this needed?
def get_file_number_from_evio_file_name(evio_file_path: str) -> int | None:
  """Extracts the 3-digit file number from the given EVIO file name,
  or returns `None` if it cannot be extracted."""
  evio_file_name = os.path.basename(evio_file_path)
  if len(evio_file_name) < 23:
    print(f"WARNING: EVIO file name '{evio_file_path}' is too short to contain a valid file number; expected at least 23 characters; ignoring")
    return None
  file_number_str = evio_file_name[20:23]  # extract the 3-digit file number from the file name pattern
  try:
    file_number = int(file_number_str)
    return file_number
  except ValueError:
    print(f"WARNING: could not extract file number from EVIO file name '{evio_file_path}'; expected characters 20-22 to be a 3-digit number; ignoring")
    return None
