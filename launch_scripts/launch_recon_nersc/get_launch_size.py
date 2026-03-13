#!/usr/bin/env python3

"""
Estimates the size of the raw data for the reconstruction launch and
the number of NERSC nodes required to process them.
"""

from __future__ import annotations

import argparse
import time

from script_job import print_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_job_size,
  get_run_numbers_from_file,
)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period                   =     ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_list_file         =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  swif_raw_data_root           =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  nmb_processes_per_nersc_node = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))
  print(f"Allocating {nmb_processes_per_nersc_node} hd_root processes per NERSC node")

  run_numbers: list[int] = get_run_numbers_from_file(run_number_list_file)
  print(f"Calculating resources for '{run_period}' raw data: {len(run_numbers)} run(s) listed in '{run_number_list_file}' and located in '{swif_raw_data_root}'")
  nmb_files:             dict[int, int  ] = {}  # number of files per run
  nmb_nodes:             dict[int, int  ] = {}  # number of NERSC nodes required per run
  fraction_nodes_unused: dict[int, float] = {}  # unused fraction of last NERSC node per run
  size_gb:               dict[int, float] = {}  # GB per run
  for run_number in run_numbers:
    (
      nmb_files[run_number],
      nmb_nodes[run_number],
      nmb_remainder_jobs,
      size_gb[run_number],
    ) = get_job_size(run_number, swif_raw_data_root, nmb_processes_per_nersc_node)
    fraction_nodes_unused[run_number] = 0.0 if nmb_remainder_jobs == 0 else 1.0 - nmb_remainder_jobs / float(nmb_processes_per_nersc_node)
    print(f"    Run {run_number:6d} = {size_gb[run_number]:6.0f} GB, {nmb_files[run_number]:3d} files, {nmb_nodes[run_number]:3d} nodes, {fraction_nodes_unused[run_number]:5.1%} of last node wasted")
  # compute totals
  total_nmb_files        = sum(nmb_files.values())
  total_nmb_nodes        = sum(nmb_nodes.values())
  total_nmb_nodes_unused = sum(fraction_nodes_unused.values())
  total_size_gb          = sum(size_gb.values())
  print("-------------------------------------------------------------------------------")
  if total_nmb_files == 0:
    print("ERROR: Did not find any EVIO files.")
  else:
    print(f"Total for {len(run_numbers)} runs:\n"
          f"    {total_size_gb:.0f} GB of raw data in {total_nmb_files} EVIO files\n"
          f"    processed by {total_nmb_nodes} NERSC nodes,\n"
          f"    out of which {total_nmb_nodes_unused:.1f} nodes are unused (= {total_nmb_nodes_unused / total_nmb_nodes:.1%} of total nodes)")

  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Estimates the size of the raw data for the reconstruction launch and the number of NERSC nodes required to process them.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
