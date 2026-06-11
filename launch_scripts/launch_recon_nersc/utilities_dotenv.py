"""Collection of utility functions that depend on the `dotenv` package, which is not installed at NERSC."""

from __future__ import annotations

import functools
import subprocess

import dotenv

from utilities import (
  ensure_dict_value_exists,
  get_evio_file_paths_for_run,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def get_config_dict_from_env_file(env_file: str) -> dict[str, str | None]:
  """Loads a .env file with the production parameters and returns its contents as a dictionary."""
  print(f"Reading production parameters from .env file '{env_file}'")
  config: dict[str, str | None] = dotenv.dotenv_values(env_file)
  assert config, f"Failed to load .env file from '{env_file}'"
  if "NERSC_NMB_PROCESSES_PER_TASK" in config and config["NERSC_NMB_PROCESSES_PER_TASK"] is not None:
    # NERSC_NMB_PROCESSES_PER_TASK is actually an expression of the form `$(echo "256 / 32" | bc)` that needs to evaluated
    nersc_nmb_processes_per_task = config["NERSC_NMB_PROCESSES_PER_TASK"][2:-1]  # remove the leading `$(` and trailing `)` to get the inner expression
    nersc_nmb_processes_per_task = subprocess.check_output([nersc_nmb_processes_per_task], shell = True, text = True)
    config["NERSC_NMB_PROCESSES_PER_TASK"] = nersc_nmb_processes_per_task
  return config


def load_mss_stub_file(mss_file_path: str) -> dict[str, str | None]:
  """Loads the MSS stub file at the given path and returns its contents as a dictionary."""
  assert mss_file_path.startswith("/mss"), f"File '{mss_file_path}' must be an `/mss` path"
  mss_stub = dotenv.dotenv_values(mss_file_path)
  assert mss_stub, f"Failed to load MSS stub file from '{mss_file_path}'"
  return mss_stub


def get_file_size_from_mss_stub(mss_file_path: str) -> int:
  """Extracts the file size in bytes from the 'size' field in the MSS stub file."""
  size_bytes = int(ensure_dict_value_exists(load_mss_stub_file(mss_file_path), "size"))
  return size_bytes


def get_file_crc32_from_mss_stub(mss_file_path: str) -> str:
  """Reads the CRC32 checksum from the 'crc32' field in a MSS stub file."""
  crc32 = ensure_dict_value_exists(load_mss_stub_file(mss_file_path), "crc32")
  return crc32


def get_job_size(
  run_number:             int,
  raw_data_root:          str,
  nmb_processes_per_task: int,
) -> tuple[int, int, int, float, list[str]]:
  """Calculates the number of files, number of tasks required, number of processes in last task, total raw-data size in GB, and list of EVIO file paths for the given run."""
  evio_file_paths = get_evio_file_paths_for_run(run_number, raw_data_root)
  nmb_files = len(evio_file_paths)
  nmb_tasks = (nmb_files + nmb_processes_per_task - 1) // nmb_processes_per_task
  nmb_remainder_processes = nmb_files % nmb_processes_per_task  # number of processes on last
  total_evio_size_gb = float(sum(get_file_size_from_mss_stub(file_path) for file_path in evio_file_paths)) / 1024**3
  return nmb_files, nmb_tasks, nmb_remainder_processes, total_evio_size_gb, evio_file_paths
