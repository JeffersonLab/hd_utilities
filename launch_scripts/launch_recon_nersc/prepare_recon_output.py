#!/usr/bin/env python3

"""
Checks completeness of files in swif2 output directory and creates
directory structure in target directory that can be written to tape.
Files can either be moved or symlinked to the target directory.
"""

from __future__ import annotations

import argparse
import glob
import os
import re

import dotenv

from get_run_list import ensure_dict_value_exists


# subdirectory names and the file names they contain, i.e. subdir name -> (file base name, file type)
RECON_SUBDIR_INFO: dict[str, tuple[str, str]] = {
  # EVIO files
  "BCAL-LED" :               ("BCAL-LED",               "evio"),
  "cpp_2c_skim" :            ("cpp_2c",                 "evio"),  #TODO _skim name?
  "DIRC-LED" :               ("DIRC-LED",               "evio"),  #TODO mostly missing in ver01
  "ECAL-LED" :               ("ECAL-LED",               "evio"),  #TODO ECAL vs. FCAL in ver01?
  "epem_selection" :         ("epem_selection",         "evio"),  # new w.r.t ver01
  "npp_2g_skim" :            ("npp_2g",                 "evio"),  #TODO _skim name?
  "npp_2pi0_skim" :          ("npp_2pi0",               "evio"),  #TODO _skim name?
  "pippim_selection" :       ("pippim_selection",       "evio"),
  "random" :                 ("random",                 "evio"),
  "sync" :                   ("sync",                   "evio"),
  # HDDM files
  "converted_random" :       ("converted_random",       "hddm"),
  "REST" :                   ("dana_rest",              "hddm"),
  # ROOT files
  "hists" :                  ("hd_root",                "root"),
  "syncskim" :               ("syncskim",               "root"),  # new w.r.t ver01
  "tree_bcal_hadronic_eff" : ("tree_bcal_hadronic_eff", "root"),
  "tree_fcal_hadronic_eff" : ("tree_fcal_hadronic_eff", "root"),
  "tree_PSFlux" :            ("tree_PSFlux",            "root"),
  "tree_tof_eff" :           ("tree_tof_eff",           "root"),
  "tree_TPOL" :              ("tree_TPOL",              "root"),
  "tree_TS_scaler" :         ("tree_TS_scaler",         "root"),
}


def main(args: argparse.Namespace) -> None:
  print(f"Loading .env file from '{args.launch_env_file}'")
  launch_config: dict[str, str | None] = dotenv.dotenv_values(args.launch_env_file)
  assert launch_config, f"Failed to load .env file from '{args.launch_env_file}'"

  print(f"Reading configuration variables from file '{args.launch_env_file}'")
  swif_output_root = ensure_dict_value_exists(launch_config, "SWIF_OUTPUT_ROOT")

  target_dir = f"/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/test.prepare"  #TODO add command-line argument

  file_path_map: dict[str, str] = {}  # map from original file name to new file name

  run_dirs = sorted(glob.glob(f"{swif_output_root}/RUN??????"))  #TODO use regexp to ensure that ?????? are digits
  task_dir_name_pattern  = re.compile(r"^TASK\d{3}$")  # "TASK" + 3-digit run number
  file_dir_name_pattern  = re.compile(r"^FILE\d{3}$")  # "FILE" + 3-digit run number
  node_file_name_pattern = re.compile(r"^node\..*$")  # "node.*"
  for run_dir in run_dirs:
    print(f"Processing run directory '{run_dir}")
    run_number = int(os.path.basename(run_dir).removeprefix("RUN"))
    task_dirs = sorted(glob.glob(f"{run_dir}/*"))  # a run directory should only contain task directories
    for task_dir in task_dirs:
      # check that it is a task directory
      if not task_dir_name_pattern.match(os.path.basename(task_dir)):
        print(f"WARNING: unexpected file or directory '{task_dir}'; ignoring.")
      if not os.path.isdir(task_dir):
        raise ValueError(f"ERROR: '{task_dir}' is not a directory; aborting.")
      print(f"  Processing task directory '{task_dir}")
      all_entries_task_dir = sorted(glob.glob(f"{task_dir}/*"))
      file_dirs  = []  # directories with hd_root output for a single input file
      node_files = []  # files with node information
      for entry in all_entries_task_dir:
        entry_name = os.path.basename(entry)
        # filter out file directories
        if os.path.isdir(entry) and file_dir_name_pattern.match(entry_name):
          file_dirs.append(entry)
        # filter out node files
        if os.path.isfile(entry) and node_file_name_pattern.match(entry_name):
          node_files.append(entry)
      # ensure that there are no other files or directories in the task directory
      if len(all_entries_task_dir) > len(file_dirs) + len(node_files):
        unexpected_entries = set(all_entries_task_dir) - set(file_dirs) - set(node_files)
        print(f"WARNING: ignoring the following unexpected files or directories in '{task_dir}': {unexpected_entries}")
      for file_dir in file_dirs:
        print(f"    Processing file directory '{file_dir}")
        file_number = int(os.path.basename(file_dir).removeprefix("FILE"))
        for _, (file_base_name, file_type) in RECON_SUBDIR_INFO.items():
          file_name = f"hd_rawdata_{run_number:06d}_{file_number:03d}.{file_base_name}.{file_type}" if file_type == "evio" else f"{file_base_name}.{file_type}"
          file_path = f"{file_dir}/{file_name}"
          if not os.path.isfile(file_path):
            print(f"WARNING: expected file '{file_path}' is missing; ignoring")
            continue
          # fix file names of evio files and make file names of non-evio files unique
          new_file_name = f"{file_base_name}_{run_number:06d}_{file_number:03d}.{file_type}"
          file_path_map[file_path] = f"{target_dir}/{new_file_name}"
          print(f"      '{file_path}' -> '{target_dir}/{new_file_name}'")
          # os.symlink(file_path, f"{target_dir}/{new_file_name}")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Generates list of runs to process for a given run period.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  args = parser.parse_args()
  main(args)
