#!/usr/bin/env python3

"""
Estimates the size of the raw data for the reconstruction launch and
the number of NERSC nodes required to process them.
"""

from __future__ import annotations

import argparse

from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_job_size,
  get_run_numbers_from_file,
)


def main(args: argparse.Namespace) -> None:
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)

  print(f"Reading configuration variables from file '{args.launch_env_file}'")
  run_period                   =     ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_list_file         =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  swif_raw_data_root           =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  nersc_nmb_processes_per_task = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))

  run_numbers: list[int] = get_run_numbers_from_file(run_number_list_file)
  print(f"Calculating resources for '{run_period}' raw data: {len(run_numbers)} run(s) listed in '{run_number_list_file}' and located in '{swif_raw_data_root}'")
  total_size_gb:         dict[int, float] = {}  # GB per run
  nmb_files:             dict[int, int  ] = {}  # number of files per run
  nmb_nodes:             dict[int, int  ] = {}  # number of NERSC nodes required per run
  fraction_nodes_unused: dict[int, float] = {}  # unused fraction of last NERSC node per run
  for run_number in run_numbers:
    (
      nmb_files[run_number],
      nmb_nodes[run_number],
      nmb_remainder_jobs,
      total_size_gb[run_number],
    ) = get_job_size(run_number, swif_raw_data_root, nersc_nmb_processes_per_task)
    fraction_nodes_unused[run_number] = 0.0 if nmb_remainder_jobs == 0 else 1.0 - nmb_remainder_jobs / float(nersc_nmb_processes_per_task)
    print(f"    Run {run_number:6d} = {total_size_gb[run_number]:6.0f} GB, {nmb_files[run_number]:3d} files, {nmb_nodes[run_number]:3d} nodes, {fraction_nodes_unused[run_number]:3.1%} of last node wasted")
  total_nmb_nodes        = sum(nmb_nodes.values())
  total_nmb_nodes_unused = sum(fraction_nodes_unused.values())
  print(f"Total for {len(run_numbers)} runs: {sum(total_size_gb.values()):.0f} GB of raw data "
        f"in {sum(nmb_files.values())} files "
        f"processed by {total_nmb_nodes} NERSC nodes, "
        f"out of which {total_nmb_nodes_unused:.1f} nodes are unused (= {total_nmb_nodes_unused / total_nmb_nodes:.1%} of total nodes)")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Prepare directory structure and use srun to start a reconstruction task on each node (positional args).",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
