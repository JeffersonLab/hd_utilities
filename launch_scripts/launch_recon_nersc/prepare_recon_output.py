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


def process_job_log_files(
  run_number:   int,
  nmb_tasks:    int,
  job_dir:      str,
  job_info_dir: str,
) -> list[tuple[str, str]]:
  """Process log files in the given job directory and return a map from original file paths to new file paths."""
  log_file_names: list[str] = [
    f"job_{run_number}.diskquota",
    f"job_{run_number}.env",
    f"job_{run_number}.hostname",
    f"job_{run_number}.mounts",
    f"srun_{run_number}.rc",
  ]
  # add main job log file that has name `job_<run number>_<job id>.out`; use glob since we don't know the job ID at this stage
  files: list[str] = sorted(glob.glob(f"{job_dir}/job_{run_number}_*.out"))
  assert len(files) == 1, f"File pattern '{job_dir}/job_{run_number}_*.out' did not return exactly one result: {files}"
  log_file_names.append(os.path.basename(files[0]))
  # add task log files
  for task_index in range(nmb_tasks):
    log_file_names.append(f"task_{run_number}_{task_index}.out")
  file_move_paths: list[tuple[str, str]] = []  # pairs of old and new file paths for moving
  for log_file_name in log_file_names:
    log_file_path = f"{job_dir}/{log_file_name}"
    if not os.path.isfile(log_file_path):
      print(f"WARNING: cannot find file '{log_file_path}'; ignoring.")
      continue
    file_move_paths.append((log_file_path, f"{job_info_dir}/{log_file_name}"))
  return file_move_paths


def process_task_log_files(
  task_dir:     str,
  job_info_dir: str,
) -> list[tuple[str, str]]:
  """Process node log files in the given task directory and return a map from original file paths to new file paths."""
  log_file_names: list[str] = [
    "node.cpuinfo",
    "node.env",
    "node.hostname",
    "node.mounts",
    "node.top",
  ]
  file_move_paths: list[tuple[str, str]] = []  # pairs of old and new file paths for moving
  for log_file_name in log_file_names:
    log_file_path = f"{task_dir}/{log_file_name}"
    if not os.path.isfile(log_file_path):
      print(f"WARNING: cannot find file '{log_file_path}'; ignoring.")
      continue
    file_move_paths.append((log_file_path, f"{job_info_dir}/{log_file_name}"))
  return file_move_paths


def process_hd_root_log_files(
  file_dir:     str,
  job_info_dir: str,
) -> list[tuple[str, str]]:
  """Process hd_root log files in the given file directory and return a map from original file paths to new file paths."""
  log_file_names: list[str] = [
    "hd_root.err",
    "hd_root.out",
    "hd_root.rc",
  ]
  file_move_paths: list[tuple[str, str]] = []  # pairs of old and new file paths for moving
  for log_file_name in log_file_names:
    log_file_path = f"{file_dir}/{log_file_name}"
    if not os.path.isfile(log_file_path):
      print(f"WARNING: cannot find file '{log_file_path}'; ignoring.")
      continue
    file_move_paths.append((log_file_path, f"{job_info_dir}/{log_file_name}"))
  return file_move_paths


def process_file_dir(
  job_dir:     str,
  run_number:  int,
  task_index:  int,
  nmb_tasks:   int,
  file_number: int,
  target_dir:  str,
) -> list[tuple[str, str]]:
  """Process the file directory defined by the arguments and return a map from original file paths to new file paths."""
  file_dir = f"{job_dir}/RUN{run_number:06d}/TASK{task_index:03d}/FILE{file_number:03d}"
  if not os.path.isdir(file_dir):
    raise ValueError(f"ERROR: '{file_dir}' does not exist or is not a directory; aborting.")
  print(f"    Processing file directory '{file_dir}")
  # always process log files
  job_info_dir = f"{target_dir}/job_info_{run_number:06d}_{file_number:03d}"  # target directory for all log files
  file_move_paths: list[tuple[str, str]] = []
  file_move_paths += process_hd_root_log_files(file_dir, job_info_dir)  # pairs of old and new file paths for moving
  file_move_paths += process_task_log_files   (f"{job_dir}/RUN{run_number:06d}/TASK{task_index:03d}", job_info_dir)
  file_move_paths += process_job_log_files    (run_number, nmb_tasks, job_dir, job_info_dir)
  # ensure that hd_root exit code is 0
  hd_root_rc_file_path = f"{file_dir}/hd_root.rc"
  hd_root_rc_file_content: str = open(hd_root_rc_file_path).read()
  match = re.search(r"exit code (\d+)$", hd_root_rc_file_content)
  hd_root_return_code = None
  if not match:
    print(f"WARNING: malformed hd_root return-code file '{hd_root_rc_file_path}': '{hd_root_rc_file_content}'; ignoring EVIO file")
    return file_move_paths
  hd_root_return_code = int(match.group(1))
  if hd_root_return_code != 0:
    print(f"WARNING: hd_root return code for run {run_number} and EVIO file number {file_number} is {hd_root_return_code}; ignoring EVIO file")
    return file_move_paths
  # process hd_root output files
  for _, (file_base_name, file_type) in RECON_SUBDIR_INFO.items():
    file_name = f"hd_rawdata_{run_number:06d}_{file_number:03d}.{file_base_name}.{file_type}" if file_type == "evio" else f"{file_base_name}.{file_type}"
    file_path = f"{file_dir}/{file_name}"
    if not os.path.isfile(file_path):
      print(f"WARNING: expected file '{file_path}' is missing; ignoring")
      continue
    # fix file names of evio files and make file names of non-evio files unique
    new_file_name = f"{file_base_name}_{run_number:06d}_{file_number:03d}.{file_type}"
    file_move_paths.append((file_path, f"{target_dir}/{new_file_name}"))
  return file_move_paths


def process_task_dir(
  job_dir:                str,
  run_number:             int,
  task_index:             int,
  nmb_tasks:              int,
  nmb_files:              int,
  nmb_processes_per_task: int,
  target_dir:             str,
) -> list[tuple[str, str]]:
  """Process the task directory defined by the arguments and return a map from original file paths to new file paths."""
  task_dir = f"{job_dir}/RUN{run_number:06d}/TASK{task_index:03d}"
  if not os.path.isdir(task_dir):
    raise ValueError(f"ERROR: '{task_dir}' does not exist or is not a directory; aborting.")
  print(f"  Processing task directory '{task_dir}")
  file_number_start = task_index * nmb_processes_per_task
  file_number_end   = min(file_number_start + nmb_processes_per_task, nmb_files)
  file_move_paths: list[tuple[str, str]] = []  # pairs of old and new file paths for moving
  for file_number in range(file_number_start, file_number_end):  # loop over EVIO file numbers
    file_move_paths += process_file_dir(job_dir, run_number, task_index, nmb_tasks, file_number, target_dir)
  return file_move_paths


def process_run_dir(
  run_number:             int,
  job_dir:                str,
  raw_data_root:          str,
  nmb_processes_per_task: int,
  target_dir:             str,
) -> list[tuple[str, str]]:
  """Process the run directory defined by the arguments and return a map from original file paths to new file paths."""
  run_dir = f"{job_dir}/RUN{run_number:06d}"
  if not os.path.isdir(run_dir):
    print(f"WARNING: '{run_dir}' does not exist or is not a directory; ignoring.")
    return []
  print(f"Processing run directory '{run_dir}")
  nmb_files, nmb_tasks, _, _, = get_job_size(run_number, raw_data_root, nmb_processes_per_task)
  file_move_paths: list[tuple[str, str]] = []  # pairs of old and new file paths for moving
  for task_index in range(nmb_tasks):  # loop over tasks
    file_move_paths += process_task_dir(job_dir, run_number, task_index, nmb_tasks, nmb_files, nmb_processes_per_task, target_dir)
  return file_move_paths


def main(args: argparse.Namespace) -> None:
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)

  print(f"Reading configuration variables from file '{args.launch_env_file}'")
  run_number_list_file         =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  swif_output_root             =     ensure_dict_value_exists(launch_config, "SWIF_OUTPUT_ROOT")
  swif_raw_data_root           =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  nersc_nmb_processes_per_task = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))

  run_numbers: list[int] = get_run_numbers_from_file(run_number_list_file)
  target_dir = f"/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/test.prepare"  #TODO add command-line argument

  file_move_paths: list[tuple[str, str]] = []  # pairs of old and new file paths for moving
  for run_number in run_numbers:  # loop over runs
    file_move_paths += process_run_dir(run_number, swif_output_root, swif_raw_data_root, nersc_nmb_processes_per_task, target_dir)
  for old_file_path, new_file_path in file_move_paths:
    print(f"'{old_file_path}' -> '{new_file_path}'")
    # os.symlink(file_path, f"{target_dir}/{new_file_name}")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Generates list of runs to process for a given run period.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
