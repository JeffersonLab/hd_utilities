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
import shutil
import time

from script_job import print_command_line_arguments
from utilities import (
  ensure_dict_value_exists,
  get_config_dict_from_env_file,
  get_job_size,
  read_run_numbers_from_file,
)


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


class FileTransferMapGenerator:
  """Class for generating a map of source file paths to destination file paths for transferring files from the SWIF output directory to the target directory."""

  def __init__(
    self,
    run_number:             int,
    job_dir:                str,
    raw_data_root:          str,
    nmb_processes_per_task: int,
    target_dir:             str,
  ) -> None:
    self.run_number             = run_number
    self.job_dir                = job_dir
    self.raw_data_root          = raw_data_root
    self.nmb_processes_per_task = nmb_processes_per_task
    self.target_dir             = target_dir
    self._file_transfer_map: list[tuple[str, str]] = []  # pairs of old and new file paths for moving   #TODO use a dictionary instead to avoid transformation in `transfer_files` function
    self._run_dir = f"{self.job_dir}/RUN{self.run_number:06d}"

  def process_run_dir(self) -> None:
    """Process the run directory defined by the arguments and append to map of original file paths to new file paths."""
    if not os.path.isdir(self._run_dir):
      print(f"WARNING: '{self._run_dir}' does not exist or is not a directory; ignoring run {self.run_number}.")  #TODO collect missing runs and report at the end of the script
      return
    print(f"Processing run directory '{self._run_dir}'")
    #TODO get list of EVIO files and check that output exists for each of them instead of relying on the number of files and the file numbering scheme
    nmb_files, nmb_tasks, _, _, = get_job_size(self.run_number, self.raw_data_root, self.nmb_processes_per_task)
    for task_index in range(nmb_tasks):  # loop over tasks
      self.process_task_dir(task_index, nmb_tasks, nmb_files)

  def process_task_dir(
    self,
    task_index: int,
    nmb_tasks:  int,
    nmb_files:  int,
  ) -> None:
    """Process the task directory defined by the task index and append to map of original file paths to new file paths."""
    task_dir = f"{self._run_dir}/TASK{task_index:03d}"
    if not os.path.isdir(task_dir):
      raise ValueError(f"ERROR: '{task_dir}' does not exist or is not a directory; aborting.") #TODO collect missing task directories and report at the end of the script instead of aborting
    print(f"  Processing task directory '{task_dir}'")
    file_number_start = task_index * self.nmb_processes_per_task
    file_number_end   = min(file_number_start + self.nmb_processes_per_task, nmb_files)
    for file_number in range(file_number_start, file_number_end):  # loop over EVIO file numbers
      self.process_file_dir(task_index, nmb_tasks, file_number)

  def process_file_dir(
    self,
    task_index:  int,
    nmb_tasks:   int,
    file_number: int,
  ) -> None:
    """Process the file directory defined by the arguments and append to map of original file paths to new file paths."""
    task_dir = f"{self._run_dir}/TASK{task_index:03d}"
    file_dir = f"{task_dir}/FILE{file_number:03d}"
    if not os.path.isdir(file_dir):
      raise ValueError(f"ERROR: '{file_dir}' does not exist or is not a directory; aborting.")  #TODO collect missing file directories and report at the end of the script instead of aborting
    print(f"    Processing file directory '{file_dir}'")
    #TODO divert log and core files for failed hd_root processes into separate directory tree; in case of failure just move all files in the FILE directory
    # always process log files
    job_info_dir = f"{self.target_dir}/job_info/{self.run_number:06d}/job_info_{self.run_number:06d}_{file_number:03d}"  # target directory for all log files
    self.process_hd_root_log_files(file_dir,  job_info_dir)
    self.process_task_log_files   (task_dir,  job_info_dir)
    self.process_job_log_files    (nmb_tasks, job_info_dir)
    # ensure that hd_root exit code is 0
    #TODO use get_hd_root_return_code function
    hd_root_rc_file_path = f"{file_dir}/hd_root.rc"
    hd_root_rc_file_content: str = open(hd_root_rc_file_path).read()
    match = re.search(r"exit code (\d+)$", hd_root_rc_file_content)
    hd_root_return_code = None
    if not match:
      print(f"WARNING: malformed hd_root return-code file '{hd_root_rc_file_path}': '{hd_root_rc_file_content}'; ignoring EVIO file")
      return
    hd_root_return_code = int(match.group(1))
    if hd_root_return_code != 0:
      print(f"WARNING: hd_root return code for run {self.run_number} and EVIO file number {file_number} is {hd_root_return_code}; ignoring EVIO file")
      return
    # process hd_root output files
    for subdir_name, (file_base_name, file_type) in RECON_SUBDIR_BASENAME_MAP.items():
      file_name = f"hd_rawdata_{self.run_number:06d}_{file_number:03d}.{file_base_name}.{file_type}" if file_type == "evio" else f"{file_base_name}.{file_type}"
      file_path = f"{file_dir}/{file_name}"
      if not os.path.isfile(file_path):
        print(f"WARNING: expected file '{file_path}' is missing; ignoring")  #TODO collect missing files and report at the end of the script
        continue
      new_file_name = f"{file_base_name}_{self.run_number:06d}_{file_number:03d}.{file_type}"  # fix file names of evio files and make file names of non-evio files unique
      new_file_path = f"{self.target_dir}/{subdir_name}/{self.run_number:06d}/{new_file_name}"
      self._file_transfer_map.append((file_path, f"{new_file_path}"))

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
        print(f"WARNING: cannot find file '{log_file_path}'; ignoring.")  #TODO collect missing files and report at the end of the script
        continue
      self._file_transfer_map.append((log_file_path, f"{dest_dir}/{log_file_name}"))

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


def transfer_files(
  file_transfer_map: list[tuple[str, str]],
  link_files:    bool = True,
  #TODO add dry-run option that only prints planned file operations without actually performing them
) -> None:
  """Move unique source files and copy duplicate source files before deleting the original."""
  print(f"Executing {len(file_transfer_map)} file operations:")
  # convert list into dictionary mapping source file paths to list of destination file paths
  destination_map: dict[str, list[str]] = {}
  for old_file_path, new_file_path in file_transfer_map:
    if old_file_path not in destination_map:
      destination_map[old_file_path] = []
    if new_file_path not in destination_map[old_file_path]:
      destination_map[old_file_path].append(new_file_path)
  # move files with unique destinations and copy files with multiple destinations before deleting original file
  # use links instead of copying/moving if `link_files` is True
  for old_file_path, new_file_paths in destination_map.items():
    for new_file_path in new_file_paths:
      #TODO verify that destination file does not already exist
      new_file_dir_name = os.path.dirname(new_file_path)
      if not os.path.isdir(new_file_dir_name):
        print(f"Creating directory '{new_file_dir_name}'")
        # os.makedirs(new_file_dir_name, exist_ok = True)
      if link_files:
        print(f"Linking '{old_file_path}' -> '{new_file_path}'")
        # os.link(old_file_path, new_file_path)
      elif len(new_file_paths) == 1:
        print(f"Moving '{old_file_path}' -> '{new_file_path}'")
        # shutil.move(old_file_path, new_file_path)
        continue
      else:
        print(f"Copying '{old_file_path}' -> '{new_file_path}'")
        # shutil.copy2(old_file_path, new_file_path)
    if not link_files and len(new_file_paths) > 1:
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
  target_dir = f"/lustre24/expphy/volatile/halld/offsite_prod/RunPeriod-2022-05/recon/ready_for_tape"  #TODO add command-line argument

  file_transfer_map: list[tuple[str, str]] = []  # pairs of old and new file paths for moving/copying
  for run_number in run_numbers:  # loop over runs
    file_transfer_map_gen = FileTransferMapGenerator(
      run_number             = run_number,
      job_dir                = swif_output_root,
      raw_data_root          = swif_raw_data_root,
      nmb_processes_per_task = nersc_nmb_processes_per_task,
      target_dir             = target_dir,
    )
    file_transfer_map_gen.process_run_dir()
    file_transfer_map += file_transfer_map_gen._file_transfer_map

  print("-------------------------------------------------------------------------------")
  transfer_files(file_transfer_map)
  #TODO tar log directories

  elapsed_time = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time // 60} min, {elapsed_time % 60} sec")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(
    description = "Generates list of runs to process for a given run period.",
  )
  parser.add_argument("--launch_env_file", default = "./launch.env", help = "Path to .env file defining the configuration variables of the reconstruction launch; default: '%(default)s'")
  parser.add_argument("--override_run_list", help = "Path to run-number list file to use instead of RCDB query")
  args = parser.parse_args()
  main(args)
