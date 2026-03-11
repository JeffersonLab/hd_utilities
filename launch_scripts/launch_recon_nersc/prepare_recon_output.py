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

from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_job_size,
  get_run_numbers_from_file,
)


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
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)

  print(f"Reading configuration variables from file '{args.launch_env_file}'")
  run_number_list_file         =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  swif_output_root             =     ensure_dict_value_exists(launch_config, "SWIF_OUTPUT_ROOT")
  swif_raw_data_root           =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  nersc_nmb_processes_per_task = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))

  run_numbers: list[int] = get_run_numbers_from_file(run_number_list_file)
  target_dir = f"/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/test.prepare"  #TODO add command-line argument

  file_path_map: dict[str, str] = {}  # map from original file name to new file name
  for run_number in run_numbers:  # loop over runs
    job_log_files: list[str] = sorted(glob.glob(f"{swif_output_root}/job_{run_number:06d}*"))  #TODO move into separate function and check that all files are there
    run_dir = f"{swif_output_root}/RUN{run_number:06d}"
    if not os.path.isdir(run_dir):
      print(f"WARNING: '{run_dir}' does not exist or is not a directory; ignoring.")
      continue
    print(f"Processing run directory '{run_dir}")
    nmb_files, nmb_tasks, _, _, = get_job_size(run_number, swif_raw_data_root, nersc_nmb_processes_per_task)
    for task_index in range(nmb_tasks):  # loop over tasks
      task_dir = f"{run_dir}/TASK{task_index:03d}"
      if not os.path.isdir(task_dir):
        raise ValueError(f"ERROR: '{task_dir}' does not exist or is not a directory; aborting.")
      print(f"  Processing task directory '{task_dir}")
      node_log_files: list[str] = sorted(glob.glob(f"{task_dir}/node.*"))  #TODO move into separate function and check that all files are there
      file_number_start = task_index * nersc_nmb_processes_per_task
      file_number_end   = min(file_number_start + nersc_nmb_processes_per_task, nmb_files)
      for file_number in range(file_number_start, file_number_end):  # loop over EVIO file numbers
        file_dir = f"{task_dir}/FILE{file_number:03d}"
        if not os.path.isdir(file_dir):
          raise ValueError(f"ERROR: '{file_dir}' does not exist or is not a directory; aborting.")
        print(f"    Processing file directory '{file_dir}")
        #TODO also check hd_root_log_files
        for _, (file_base_name, file_type) in RECON_SUBDIR_INFO.items():  # loop over hd_root output files
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
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
