#!/usr/bin/env python3

"""
Estimates the size of the raw data for the reconstruction launch and
the number of NERSC nodes required to process them.
"""

from __future__ import annotations

import argparse
import glob
import subprocess
from typing import TypeVar

import dotenv


KeyType   = TypeVar("KeyType",   bound = object)  # any not-None type
ValueType = TypeVar("ValueType", bound = object)  # any not-None type
def ensure_dict_value_exists(
  config: dict[KeyType, ValueType | None],
  key:    KeyType,
) -> ValueType:
  """Ensure that value for the given key exists in the dictionary and has a non-None value; return the value."""
  value = config[key]
  if value is None:
    raise ValueError(f"Missing value for key '{key}'")
  return value


def get_file_size_from_mss_stub(mss_file_path: str) -> int:
  """Extract the file size in bytes from 'size' field in the MSS stub file."""
  mss_stub = dotenv.dotenv_values(mss_file_path)
  assert mss_stub, f"Failed to load MSS stub file from '{mss_file_path}'"
  size_bytes = int(ensure_dict_value_exists(mss_stub, "size"))
  return size_bytes


def main(args: argparse.Namespace) -> None:
  print(f"Loading .env file from '{args.launch_env_file}'")
  launch_config: dict[str, str | None] = dotenv.dotenv_values(args.launch_env_file)
  assert launch_config, f"Failed to load .env file from '{args.launch_env_file}'"

  print(f"Reading values from .env file '{args.launch_env_file}'")
  run_period                   = ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_list_file         = ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE")
  swif_raw_data_root           = ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  nersc_max_threads_per_task   = ensure_dict_value_exists(launch_config, "NERSC_MAX_THREADS_PER_TASK")
  nersc_nmb_processes_per_task = ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK")
  # postprocess the values as needed
  nersc_max_threads_per_task   = int(nersc_max_threads_per_task)
  # NERSC_NMB_PROCESSES_PER_TASK is actually an expression of the form `$(echo "256 / 32" | bc)` that needs to evaluated
  nersc_nmb_processes_per_task = nersc_nmb_processes_per_task[2:-1]  # remove the leading `$(` and trailing `)` to get the inner expression
  nersc_nmb_processes_per_task = int(subprocess.check_output([nersc_nmb_processes_per_task], shell = True, text = True))

  run_numbers: list[int] = []
  with open(run_number_list_file) as file:
    run_numbers = [int(line.strip()) for line in file if line.strip()]
  print(f"Calculating resources for '{run_period}' raw data: {len(run_numbers)} run(s) listed in '{run_number_list_file}' and located in '{swif_raw_data_root}'")
  total_size_gb:         dict[int, float] = {}  # GB per run
  nmb_files:             dict[int, int  ] = {}  # number of files per run
  nmb_nodes:             dict[int, int  ] = {}  # number of NERSC nodes required per run
  fraction_nodes_unused: dict[int, float] = {}  # unused fraction of last NERSC node per run
  for run_number in run_numbers:
    evio_run_dir = f"{swif_raw_data_root}/Run{run_number:06d}"
    evio_file_names = glob.glob(f"{evio_run_dir}/*.evio")
    total_size_gb        [run_number] = sum(get_file_size_from_mss_stub(file_name) for file_name in evio_file_names) / (1024**3)
    nmb_files            [run_number] = len(evio_file_names)
    nmb_nodes            [run_number] = (nmb_files[run_number] + nersc_nmb_processes_per_task - 1) // nersc_nmb_processes_per_task  #TODO Igal's formula; see `do_my_launch.sh`
    fraction_nodes_unused[run_number] = 1.0 - (nmb_files[run_number] % nersc_nmb_processes_per_task) / float(nersc_nmb_processes_per_task)
    print(f"    Run {run_number:6d} = {total_size_gb[run_number]:6.0f} GB, {nmb_files[run_number]:3d} files, {nmb_nodes[run_number]:3d} nodes, {fraction_nodes_unused[run_number]:3.1%} of last node wasted")
  total_nmb_nodes        = sum(nmb_nodes.values())
  total_nmb_nodes_unused = sum(fraction_nodes_unused.values())
  print(f"Total for all runs: {sum(total_size_gb.values()):.0f} GB of raw data "
        f"in {sum(nmb_files.values())} files "
        f"processed by {total_nmb_nodes} NERSC nodes, "
        f"out of which {total_nmb_nodes_unused:.1f} nodes are unused (= {total_nmb_nodes_unused / total_nmb_nodes:.1%} of total nodes)")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Prepare directory structure and use srun to start a reconstruction task on each node (positional args).",
  )
  parser.add_argument("launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  args = parser.parse_args()
  main(args)
