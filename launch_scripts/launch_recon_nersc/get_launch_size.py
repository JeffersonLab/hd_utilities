#!/usr/bin/env python3

"""
Estimates the size of the raw data for the reconstruction launch and
the number of NERSC nodes required to process them.
"""

from __future__ import annotations

import argparse
import functools
import math
import numpy as np
import time
from typing import Sequence

import ROOT

from script_job import print_command_line_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_evio_file_paths_for_run,
  get_file_size_from_mss_stub,
  get_job_size,
  read_run_numbers_from_file,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


def plot_evio_file_size(
  run_numbers:   Sequence[int],
  raw_data_root: str,
  swif_workflow: str,
) -> None:
  """Plots the distribution of EVIO file sizes for the given run numbers."""
  print(f"Plotting EVIO file size distribution for {len(run_numbers)} runs...")
  # get sizes of all EVIO files for the given runs
  file_sizes_GB = np.array(
    [
      float(get_file_size_from_mss_stub(evio_file_path)) / 1024**3
      for run_number in run_numbers
      for evio_file_path in get_evio_file_paths_for_run(run_number, raw_data_root)
    ],
    dtype = float,
  )
  # histogram file sizes
  canv = ROOT.TCanvas()
  hist = ROOT.TH1F(f"evio_file_size.{swif_workflow}", f"{swif_workflow};EVIO File Size (GB);Count", 200, 0, math.ceil(max(file_sizes_GB) * 1.05))
  hist.FillN(len(file_sizes_GB), file_sizes_GB, ROOT.nullptr)
  canv.SetLogy()
  hist.SetMinimum(0.1)  # improve log scale
  hist.SetFillColor(ROOT.kBlue - 10)
  hist.Draw()
  #TODO print # of files, total, average, and median file size on plot, file size standard deviation, and maybe also min and max file size
  nmb_files       = len(file_sizes_GB)
  min_size_GB     = min(file_sizes_GB)
  max_size_GB     = max(file_sizes_GB)
  mean_size_GB    = hist.GetMean()
  std_dev_size_GB = hist.GetRMS()
  probabilities   = np.array([0.5])  # array of cumulative probabilities where quantiles should be evaluated
  quantiles_GB    = np.array([0.])   # array to be filled with quantiles; must be same length as probabilities
  hist.GetQuantiles(1, quantiles_GB, probabilities)
  label = ROOT.TLatex()
  label.SetNDC()
  label.SetTextSize(0.04)
  label.SetTextAlign(ROOT.kHAlignLeft + ROOT.kVAlignBottom)
  label.DrawLatex(0.135, 0.86, f"{len(run_numbers)} runs with {nmb_files} files and total size of {sum(file_sizes_GB) / 1024:.1f} TB")
  label.DrawLatex(0.135, 0.80, f"Min size = {min_size_GB:.1f} GB, max size = {max_size_GB:.1f} GB")
  label.DrawLatex(0.135, 0.74, f"Mean size = {mean_size_GB:.1f} GB, median size = {quantiles_GB[0]:.1f} GB")
  label.DrawLatex(0.135, 0.68, f"Size standard deviation = {std_dev_size_GB:.1f} GB")
  canv.RedrawAxis()
  canv.SaveAs(f"./{hist.GetName()}.pdf")


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_period                   =     ensure_dict_value_exists(launch_config, "RUN_PERIOD")
  run_number_list_file         =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  swif_raw_data_root           =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  swif_workflow                =     ensure_dict_value_exists(launch_config, "SWIF_WORKFLOW")
  nmb_processes_per_nersc_node = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))
  print(f"Allocating {nmb_processes_per_nersc_node} hd_root processes per NERSC node")

  run_numbers: list[int] = read_run_numbers_from_file(run_number_list_file)
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
      _,
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

  print("-------------------------------------------------------------------------------")
  plot_evio_file_size(run_numbers, swif_raw_data_root, swif_workflow)

  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  ROOT.gROOT.SetBatch(True)
  ROOT.gStyle.SetOptStat(False)
  # ROOT.gStyle.SetOptStat(111111)
  parser = argparse.ArgumentParser(
    description = "Estimates the size of the raw data for the reconstruction launch and the number of NERSC nodes required to process them.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
