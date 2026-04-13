#!/usr/bin/env python3

"""
Checks completeness of files in swif2 output directory and creates
directory structure in target directory that can be written to tape.
Files can either be moved or symlinked to the target directory.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import functools
import glob
import os
import shutil
import time

from script_job import (
  get_hd_root_return_code,
  print_command_line_arguments,
)
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_file_number_from_evio_file_name,
  get_job_size,
  read_run_numbers_from_file,
)


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


#TODO this should go in a period-dependent file
# map subdirectory names in the final directory layout to the files
# they contain, i.e. subdir name -> (file base name, file type)
#NOTE in most cases, subdir name == file base name
RECON_SUBDIR_BASENAME_MAP: dict[str, tuple[str, str]] = {
  # EVIO files
  "cpp_2c" :                 ("cpp_2c",                 "evio"),
  "epem_selection" :         ("epem_selection",         "evio"),  # new w.r.t ver01
  "npp_2g" :                 ("npp_2g",                 "evio"),
  "npp_2pi0" :               ("npp_2pi0",               "evio"),
  "pippim_selection" :       ("pippim_selection",       "evio"),
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
# construct reverse map: file base name -> (subdir name, file type)
RECON_BASENAME_SUBDIR_MAP: dict[str, tuple[str, str]] = {
  file_base_name : (subdir_name, file_type) for subdir_name, (file_base_name, file_type) in RECON_SUBDIR_BASENAME_MAP.items()
}


#TODO the transfer maps should be generated per original EVIO file and not per run
#TODO add whitelist of EVIO files not to check/process
class FileTransferMapGenerator:
  """Class for generating a map of source file paths to destination file paths for transferring files from the SWIF output directory to the target directory."""

  def __init__(
    self,
    run_number:             int,
    job_dir:                str,  # directory containing the SWIF output for the run; assuming structure: <job_dir>/RUN<run number>/TASK<task index>/FILE<file number>
    raw_data_root:          str,  # root directory containing the EVIO data files; assuming structure: <raw_data_root>/RUN<run number>/hd_rawdata_<run number>_<file number>.evio
    nmb_processes_per_task: int,  # number of processes per task used in the reconstruction launch
    target_dir:             str,  # root directory to which the output files of hd_root processes with return code 0 will be moved
    # failed_log_file_dir:    str,  # directory to which log files of hd_root processes with non-zero return code will be moved for further investigation
  ) -> None:
    self.run_number             = run_number
    self.job_dir                = job_dir
    # self.job_dir                = job_dir + "/../test"
    self.raw_data_root          = raw_data_root
    self.nmb_processes_per_task = nmb_processes_per_task
    self.target_dir             = target_dir
    self._run_dir               = f"{self.job_dir}/RUN{self.run_number:06d}"  # directory containing the SWIF output for the run
    self._evio_file_paths:   list[str]                  = []  # list of EVIO file paths for the run
    self._file_transfer_map: list[tuple[str, str]]      = []  # pairs of old and new file paths for moving   #TODO use a dictionary instead to avoid transformation in `transfer_files` function
    self._failed_evio_files: list[str]                  = []  # collect paths of EVIO files that are missing or for which hd_root failed
    self._missing_items:     defaultdict[str, set[str]] = defaultdict(set)  # collect missing items by item type for reporting at the end of the script

  def process_run_dir(self) -> None:
    """Process the run directory defined by the arguments and append to map of original file paths to new file paths."""
    nmb_evio_files, nmb_tasks, _, _, self._evio_file_paths = get_job_size(self.run_number, self.raw_data_root, self.nmb_processes_per_task)
    if not os.path.isdir(self._run_dir):
      print(f"WARNING: '{self._run_dir}' does not exist or is not a directory; ignoring run {self.run_number}")
      self._missing_items["run dir(s)"].add(self._run_dir)
      for evio_file_path in self._evio_file_paths:
        self._failed_evio_files.append(evio_file_path)
      return
    print(f"Processing run {self.run_number} with {nmb_tasks} tasks and {nmb_evio_files} files in directory '{self._run_dir}'")
    for task_index in range(nmb_tasks):  # loop over tasks
      self.process_task_dir(task_index, nmb_tasks)

  def process_task_dir(
    self,
    task_index: int,
    nmb_tasks:  int,
  ) -> None:
    """Process the task directory defined by the task index and append to map of original file paths to new file paths."""
    evio_file_start_index = task_index * self.nmb_processes_per_task
    evio_file_end_index   = min(evio_file_start_index + self.nmb_processes_per_task, len(self._evio_file_paths))
    task_dir = f"{self._run_dir}/TASK{task_index:03d}"
    if not os.path.isdir(task_dir):
      print(f"WARNING: '{task_dir}' does not exist or is not a directory; ignoring task {task_index}")
      self._missing_items["task dir(s)"].add(task_dir)
      for evio_file_path in self._evio_file_paths[evio_file_start_index:evio_file_end_index]:
        self._failed_evio_files.append(evio_file_path)
      return
    print(f"  Processing task {task_index} in directory '{task_dir}'")
    for evio_file_path in self._evio_file_paths[evio_file_start_index:evio_file_end_index]:  # loop over EVIO files
      self.process_file_dir(task_index, nmb_tasks, evio_file_path)

  def process_file_dir(
    self,
    task_index:     int,
    nmb_tasks:      int,
    evio_file_path: str,
  ) -> None:
    """Process the file directory defined by the arguments and append to map of original file paths to new file paths."""
    task_dir = f"{self._run_dir}/TASK{task_index:03d}"
    file_number = get_file_number_from_evio_file_name(evio_file_path)
    assert file_number is not None, f"Failed to extract file number from EVIO file name '{evio_file_path}'"
    file_dir = f"{task_dir}/FILE{file_number:03d}"
    if not os.path.isdir(file_dir):
      print(f"WARNING: '{file_dir}' does not exist or is not a directory; ignoring EVIO file")
      self._missing_items["file dir(s)"].add(file_dir)
      self._failed_evio_files.append(evio_file_path)
      return
    print(f"    Processing file {file_number} in directory '{file_dir}'")
    #TODO divert log and core files for failed hd_root processes into separate directory tree; in case of failure just move all files in the FILE directory
    # always process log files
    job_info_dir = f"{self.target_dir}/job_info/{self.run_number:06d}/job_info_{self.run_number:06d}_{file_number:03d}"  # target directory for all log files
    self.process_hd_root_log_files(file_dir,  job_info_dir)
    self.process_task_log_files   (task_dir,  job_info_dir)
    self.process_job_log_files    (nmb_tasks, job_info_dir)
    # ensure that hd_root return code is 0
    hd_root_rc_file_path = f"{file_dir}/hd_root.rc"
    hd_root_return_code = get_hd_root_return_code(hd_root_rc_file_path)
    if hd_root_return_code != 0:
      print(f"WARNING: hd_root return code for run {self.run_number} and EVIO file number {file_number} is {hd_root_return_code}; ignoring EVIO file")
      self._failed_evio_files.append(evio_file_path)
      return
    # process hd_root output files
    for subdir_name, (file_base_name, file_type) in RECON_SUBDIR_BASENAME_MAP.items():
      file_name = f"hd_rawdata_{self.run_number:06d}_{file_number:03d}.{file_base_name}.{file_type}" if file_type == "evio" else f"{file_base_name}.{file_type}"
      file_path = f"{file_dir}/{file_name}"
      if not os.path.isfile(file_path):
        print(f"WARNING: expected hd_root output file '{file_path}' is missing; ignoring this file")
        self._missing_items[f"{file_base_name} file(s)"].add(file_path)
        continue
      new_file_name = f"{file_base_name}_{self.run_number:06d}_{file_number:03d}.{file_type}"  # fix file names of evio files and make file names of non-evio files unique
      new_file_path = f"{self.target_dir}/{subdir_name}/{self.run_number:06d}/{new_file_name}"
      self._file_transfer_map.append((file_path, f"{new_file_path}"))

  def process_job_log_files(
    self,
    nmb_tasks:    int,
    job_info_dir: str,
  ) -> None:
    """Process log files in the given job directory and append to map of original file paths to new file paths."""
    log_file_names: list[str] = [
      # f"job_{run_number}.diskquota",
      f"job_{self.run_number}.env",
      f"job_{self.run_number}.hostname",
      f"job_{self.run_number}.mounts",
      f"srun_{self.run_number}.rc",
    ]
    # add main job log file that has name `job_<run number>_<job id>.out`; use glob since we don't know the job ID at this stage
    files: list[str] = sorted(glob.glob(f"{self.job_dir}/job_{self.run_number}_*.out"))
    assert len(files) == 1, f"File pattern '{self.job_dir}/job_{self.run_number}_*.out' did not return exactly one result: {files}"
    log_file_names.append(os.path.basename(files[0]))
    # add task log files
    for task_index in range(nmb_tasks):
      log_file_names.append(f"task_{self.run_number}_{task_index}.out")
    self.process_log_files(log_file_names, self.job_dir, job_info_dir)

  def process_task_log_files(
    self,
    task_dir:     str,
    job_info_dir: str,
  ) -> None:
    """Process node log files in the given task directory and append to map of original file paths to new file paths."""
    log_file_names: list[str] = [
      "node.cpuinfo",
      "node.env",
      "node.hostname",
      "node.mounts",
      "node.top",
    ]
    self.process_log_files(log_file_names, task_dir, job_info_dir)

  def process_hd_root_log_files(
    self,
    file_dir:     str,
    job_info_dir: str,
  ) -> None:
    """Process hd_root log files in the given file directory and append to map of original file paths to new file paths."""
    log_file_names: list[str] = [
      "hd_root.err",
      "hd_root.out",
      "hd_root.rc",
    ]
    self.process_log_files(log_file_names, file_dir, job_info_dir)

  def process_log_files(
    self,
    log_file_names: list[str],
    src_dir:        str,
    dest_dir:       str,
  ) -> None:
    """Process log files in the given log directory and append to map of original file paths to new file paths."""
    for log_file_name in log_file_names:
      log_file_path = f"{src_dir}/{log_file_name}"
      if not os.path.isfile(log_file_path):
        print(f"WARNING: expected log file '{log_file_path}' is missing; ignoring this file")
        self._missing_items["log file(s)"].add(log_file_path)
        continue
      self._file_transfer_map.append((log_file_path, f"{dest_dir}/{log_file_name}"))


def transfer_files(
  file_transfer_map: list[tuple[str, str]],
  symlink_files:     bool = False,
  dryrun:            bool = False,
) -> None:
  """Move unique source files and copy duplicate source files before deleting the original."""
  print(f"{'Executing' if not dryrun else 'Previewing'} {len(file_transfer_map)} file operations:")
  # convert list into dictionary mapping source file paths to list of destination file paths
  destination_map: dict[str, list[str]] = {}
  for old_file_path, new_file_path in file_transfer_map:
    if old_file_path not in destination_map:
      destination_map[old_file_path] = []
    if new_file_path not in destination_map[old_file_path]:
      destination_map[old_file_path].append(new_file_path)
  # move files with unique destinations and copy files with multiple destinations before deleting original file
  # use symbolic links instead of copying/moving if `symlink_files` is True
  for old_file_path, new_file_paths in destination_map.items():
    for new_file_path in new_file_paths:
      #TODO verify that destination file does not already exist
      new_file_dir_name = os.path.dirname(new_file_path)
      if not os.path.isdir(new_file_dir_name):
        if not dryrun:
          print(f"Creating directory '{new_file_dir_name}'")
          os.makedirs(new_file_dir_name, exist_ok = True)
      if symlink_files:
        if not dryrun:
          print(f"Linking '{old_file_path}' -> '{new_file_path}'")
          os.symlink(old_file_path, new_file_path)
      elif len(new_file_paths) > 1:
        if not dryrun:
          print(f"Copying '{old_file_path}' -> '{new_file_path}'")
          # shutil.copy2(old_file_path, new_file_path)
      else:  # len(new_file_paths) == 1
        if not dryrun:
          print(f"Moving '{old_file_path}' -> '{new_file_path}'")
          # shutil.move(old_file_path, new_file_path)
        continue
    if not symlink_files and len(new_file_paths) > 1:
      if not dryrun:
        print(f"Deleting '{old_file_path}'")
        # os.remove(old_file_path)


def main(args: argparse.Namespace) -> None:
  start_time = time.time()
  print_command_line_arguments(args)
  launch_config: dict[str, str | None] = get_config_dict_from_env_file(args.launch_env_file)
  run_number_list_file         =     ensure_dict_value_exists(launch_config, "RUN_NUMBER_LIST_FILE") if args.override_run_list is None else args.override_run_list
  swif_output_root             =     ensure_dict_value_exists(launch_config, "SWIF_OUTPUT_ROOT")
  swif_raw_data_root           =     ensure_dict_value_exists(launch_config, "SWIF_RAW_DATA_ROOT")
  nersc_nmb_processes_per_task = int(ensure_dict_value_exists(launch_config, "NERSC_NMB_PROCESSES_PER_TASK"))

  run_numbers: list[int] = read_run_numbers_from_file(run_number_list_file)
  # target_dir = f"/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/ready_for_tape"  #TODO add command-line argument
  target_dir = f"./ready_for_tape"

  total_nmb_evio_files = 0
  file_transfer_map:  list[tuple[str, str]]            = []  # pairs of old and new file paths for moving/copying
  failed_evio_files:  list[str]                        = []  # collect paths of EVIO files for which hd_root failed
  missing_items_runs: list[defaultdict[str, set[str]]] = []  # collect missing items for each run by item type for reporting at the end of the script
  for run_number in run_numbers:  # loop over runs
    print("...............................................................................")
    print(f"Verifying completeness of files for run {run_number} and preparing file transfer map")
    file_transfer_map_gen = FileTransferMapGenerator(
      run_number             = run_number,
      job_dir                = swif_output_root,
      raw_data_root          = swif_raw_data_root,
      nmb_processes_per_task = nersc_nmb_processes_per_task,
      target_dir             = target_dir,
    )
    file_transfer_map_gen.process_run_dir()
    total_nmb_evio_files += len(file_transfer_map_gen._evio_file_paths)
    file_transfer_map    += file_transfer_map_gen._file_transfer_map
    failed_evio_files    += file_transfer_map_gen._failed_evio_files
    missing_items_runs.append(file_transfer_map_gen._missing_items)

  # merge missing items into one dictionary mapping item type to set of missing items across all runs
  missing_items_merged: defaultdict[str, set[str]] = defaultdict(set)
  for missing_items_run in missing_items_runs:
    for item_type, missing_items in missing_items_run.items():
      missing_items_merged[item_type].update(missing_items)
  # print summary of missing items by item type
  if len(missing_items_merged) == 0:
    print("Found no missing items; all expected files are present")
  else:
    print("-------------------------------------------------------------------------------")
    print(f"Summary of missing items across {len(run_numbers)} run(s) with {total_nmb_evio_files} EVIO file(s):")
    for item_type, missing_items in missing_items_merged.items():
      #TODO also print fraction of missing items among all expected items of the given type
      print(f"{len(missing_items)} {item_type} missing:")
      for missing_item in sorted(missing_items):
        print(f"  {missing_item}")
  # print summary of failed EVIO files
  if len(failed_evio_files) == 0:
    print("Found no EVIO files, that are missing or for which hd_root has a non-zero return code")
  else:
    print("-------------------------------------------------------------------------------")
    print(f"{len(failed_evio_files)} out of {total_nmb_evio_files} EVIO file(s) {'are' if len(failed_evio_files) != 1 else 'is'} missing or have a non-zero hd_root return code:")
    #TODO calculate fraction of failed EVIO files among all processed EVIO files
    for failed_evio_file in sorted(failed_evio_files):
      print(f"  {failed_evio_file}")

  print("-------------------------------------------------------------------------------")
  if args.mode == "check":
    print(f"Check mode: found {len(file_transfer_map)} file operations")
  elif args.mode == "dryrun":
    transfer_files(file_transfer_map, dryrun = True)
  elif args.mode == "symlink":
    transfer_files(file_transfer_map, symlink_files = True)
  elif args.mode == "move":
    transfer_files(file_transfer_map)
  else:
    raise ValueError(f"Unknown mode '{args.mode}'")
  #TODO tar log directories

  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Checks completeness of files in swif2 output directory and creates directory structure in target directory that can be written to tape.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  parser.add_argument("--mode", choices = ["check", "dryrun", "symlink", "move"], default = "check", help = "Operation mode: 'check': verify completeness of files, 'dryrun': preview transfer commands, 'symlink': create symbolic links, or 'move' perform transfers; default: '%(default)s'")
  args = parser.parse_args()
  main(args)
