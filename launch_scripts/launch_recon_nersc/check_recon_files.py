#!/usr/bin/env python3

"""
Checks completeness of files in directory, where final reconstruction output is stored.
"""

from __future__ import annotations

import argparse
import glob
import os
import re

from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_run_numbers_from_file,
)


# subdirectory names and the file names they contain, i.e. subdir name -> (file base name, file type)
RECON_SUBDIR_INFO: dict[str, tuple[str, str]] = {
  "BCAL-LED" :               ("BCAL-LED",               "evio"),
  # "CCAL-LED" :               ("CCAL-LED",               "evio"),  # most runs missing
  "converted_random" :       ("converted_random",       "hddm"),  # some runs missing?
  "cpp_2c_skim" :            ("cpp_2c",                 "evio"),
  # "DIRC-LED" :               ("DIRC-LED",               "evio"),  # most runs missing
  "FCAL-LED" :               ("FCAL-LED",               "evio"),
  "hists" :                  ("hd_root",                "root"),
  "job_info" :               ("job_info",               "tgz" ),
  "npp_2g_skim" :            ("npp_2g",                 "evio"),
  "npp_2pi0_skim" :          ("npp_2pi0",               "evio"),
  "random" :                 ("random",                 "evio"),
  "REST" :                   ("dana_rest",              "hddm"),
  "sync" :                   ("sync",                   "evio"),
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
  run_period           = ensure_dict_value_exists(launch_config, "RUN_PERIOD")  # e.g. 2022-05
  recon_version_label  = args.recon_version_label if args.recon_version_label else ensure_dict_value_exists(launch_config, "VER")  # e.g. ver02
  run_number_list_file = args.run_list if args.run_list else ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE")
  recon_data_root_dir  = f"/mss/halld/RunPeriod-{run_period}/recon/{recon_version_label}"
  raw_data_root_dir    = f"/mss/halld/RunPeriod-2022-05/rawdata"

  run_numbers_from_list: list[int] = get_run_numbers_from_file(run_number_list_file)

  run_dir_name_pattern = re.compile(r"^\d{6}$")  # 6-digit run number
  for recon_subdir, (recon_file_base_name, recon_file_type) in RECON_SUBDIR_INFO.items():
    # get run numbers in recon subdir
    run_numbers_from_subdirs: list[int] = []
    for item in sorted(glob.glob(f"{recon_data_root_dir}/{recon_subdir}/*")):  # loop over all items in the recon subdir
      item_name = os.path.basename(item)
      if os.path.isdir(item) and run_dir_name_pattern.match(item_name):  # filter for directories with 6-digit names
        run_numbers_from_subdirs.append(int(item_name))
    run_numbers_from_subdirs.sort()
    # check that the run numbers from the list file match the run numbers from the recon subdir
    if run_numbers_from_subdirs != run_numbers_from_list:
      print(f"ERROR: recon subdir '{recon_subdir}' contains run numbers, which do not match the run numbers from the list file")
      missing_in_recon_dir = sorted(set(run_numbers_from_list) - set(run_numbers_from_subdirs))
      if missing_in_recon_dir:
        print(f"    Missing run numbers in '{recon_subdir}': {missing_in_recon_dir}")
      missing_in_list_file = sorted(set(run_numbers_from_subdirs) - set(run_numbers_from_list))
      if missing_in_list_file:
        print(f"    Extra run numbers in '{recon_subdir}': {missing_in_list_file}")
    else:
      print(f"SUCCESS: recon subdir '{recon_subdir}' contains the expected run numbers from the list file")
    # check that run subdirs contain the expected files
    for run_number in run_numbers_from_subdirs:
      recon_data_run_dir = f"{recon_data_root_dir}/{recon_subdir}/{run_number:06d}"
      raw_data_run_dir = f"{raw_data_root_dir}/Run{run_number:06d}"
      evio_files: list[str] = sorted(glob.glob(f"{raw_data_run_dir}/hd_rawdata_{run_number:06d}_???.evio"))
      for evio_file in evio_files:
        file_labels = os.path.splitext(os.path.basename(evio_file))[0].split("_")[-2:]  # e.g. "hd_rawdata_123456_000.evio" -> [123456, 000]
        file_label = "_".join(file_labels)
        recon_file_name = f"{recon_file_base_name}_{file_label}.{recon_file_type}"
        expected_recon_file = f"{recon_data_run_dir}/{recon_file_name}"
        if not os.path.isfile(expected_recon_file):
          print(f"ERROR: expected recon file '{expected_recon_file}' does not exist")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Generates list of runs to process for a given run period.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--run_list", help = "Path to run-number list file to use instead of the one defined in .env file")
  parser.add_argument("--recon_version_label", help = "Reconstruction version label (e.g. `ver02`) to use instead of the one defined in .env file")
  args = parser.parse_args()
  main(args)
