#!/usr/bin/env python3

"""
Checks completeness of files in job working directory and creates
transfer map with correct directory structure for transfer to JLab.
"""

from __future__ import annotations

from collections import defaultdict
import functools
import glob
import os
import subprocess
import time

from utilities import get_hd_root_return_code


# always flush print() to reduce garbling of log files due to buffering
print = functools.partial(print, flush = True)


#TODO this should go in a period-dependent file
# map subdirectory names in the final directory layout to the files
# they contain, i.e. subdir name -> (file base name, file type)
#NOTE in most cases, subdir name == file base name
# # 2022-05_ver02
# RECON_SUBDIR_BASENAME_MAP: dict[str, tuple[str, str]] = {
#   # EVIO files
#   "cpp_2c" :                 ("cpp_2c",                 "evio"),
#   "epem_selection" :         ("epem_selection",         "evio"),  # new w.r.t ver01
#   "npp_2g" :                 ("npp_2g",                 "evio"),
#   "npp_2pi0" :               ("npp_2pi0",               "evio"),
#   "pippim_selection" :       ("pippim_selection",       "evio"),
#   "pippim_selection" :       ("pippim_selection",       "evio"),
#   # HDDM files
#   "converted_random" :       ("converted_random",       "hddm"),
#   "REST" :                   ("dana_rest",              "hddm"),
#   # ROOT files
#   "hists" :                  ("hd_root",                "root"),
#   "syncskim" :               ("syncskim",               "root"),  # new w.r.t ver01
#   "tree_bcal_hadronic_eff" : ("tree_bcal_hadronic_eff", "root"),
#   "tree_fcal_hadronic_eff" : ("tree_fcal_hadronic_eff", "root"),
#   "tree_PSFlux" :            ("tree_PSFlux",            "root"),
#   "tree_tof_eff" :           ("tree_tof_eff",           "root"),
#   "tree_TPOL" :              ("tree_TPOL",              "root"),
#   "tree_TS_scaler" :         ("tree_TS_scaler",         "root"),
# }
# 2021-11_ver05
RECON_SUBDIR_BASENAME_MAP: dict[str, tuple[str, str]] = {
  # EVIO files
  "ps" :               ("ps",               "evio"),
  # HDDM files
  "converted_random" : ("converted_random", "hddm"),
  "REST" :             ("dana_rest",        "hddm"),
  # ROOT files
  "hists" :            ("hd_root",          "root"),
  "tree_PSFlux" :      ("tree_PSFlux",      "root"),
  "tree_TPOL" :        ("tree_TPOL",        "root"),
  "tree_TS_scaler" :   ("tree_TS_scaler",   "root"),
}


#TODO add whitelist of EVIO files not to check/process
class FileTransferMapGenerator:
  """Class for generating a map of source file paths to destination file paths for transferring files from the job working directory to the destination directories at JLAB."""

  def __init__(
    self,
    job_id:                       int,  # Slurm ID of the job
    run_number:                   int,  # run number of the job
    work_dir_job_path:            str,  # path to working directory of job; assuming directory structure: <work_dir_job_path>/RUN<run number>/TASK<task index>/FILE<file number>
    nmb_tasks:                    int,  # number of tasks in the job
    nmb_processes_per_task:       int,  # number of processes per task used in the reconstruction launch
    hd_root_output_dest_dir_path: str,  # path of directory, to which the output files of hd_root processes with return code 0 will be copied
    log_files_dest_dir_path:      str,  # path of directory, to which the log files of hd_root processes with return code 0 will be copied
    failed_hd_root_dest_dir_path: str,  # path of directory, to which any log and output files of hd_root processes with non-zero return code will be copied for further investigation
  ) -> None:
    self.job_id                       = job_id
    self.run_number                   = run_number
    self.work_dir_job_path            = work_dir_job_path
    self.nmb_tasks                    = nmb_tasks
    self.nmb_processes_per_task       = nmb_processes_per_task
    self.hd_root_output_dest_dir_path = hd_root_output_dest_dir_path
    self.log_files_dest_dir_path      = log_files_dest_dir_path
    self.failed_hd_root_dest_dir_path = failed_hd_root_dest_dir_path
    self._run_dir_name                = f"RUN{self.run_number:06d}"  # directory containing the SWIF output for the run
    self._evio_file_names:      list[str]                  = []  # EVIO file names processed by the job
    self._failed_evio_files:    list[str]                  = []  # paths of EVIO files that are missing or for which hd_root failed
    self._missing_items:        defaultdict[str, set[str]] = defaultdict(set)  # missing items by item type for reporting
    self._file_transfer_map:    defaultdict[str, set[str]] = defaultdict(set)  # mapping of relative local file paths to absolute destination file paths; log files may have multiple destination paths

  @property
  def file_transfer_map(self) -> dict[str, set[str]]:
    """Returns mapping of relative local file paths to absolute destination file paths; log files may have multiple destination paths"""
    return dict(self._file_transfer_map)

  class InWorkDirJob:
    """Context manager for changing the current directory to the job working directory."""
    def __init__(
      self,
      gen: FileTransferMapGenerator,
    ) -> None:
      self._gen = gen

    def __enter__(self) -> "FileTransferMapGenerator.InWorkDirJob":
      self._old_path = os.getcwd()
      # ensure job working directory exists
      try:
        os.chdir(self._gen.work_dir_job_path)
      except Exception as e:
        print(f"WARNING: cannot change to job working directory '{self._gen.work_dir_job_path}': {e}")
        self._gen._missing_items["job working dir"].add(self._gen.work_dir_job_path)
        raise
      return self

    def __exit__(self, _exc_type, _exc_value, _traceback) -> None:
      os.chdir(self._old_path)

  def process_work_dir(self) -> None:
    """Processes the working directory of the job and generates file transfer map."""
    with FileTransferMapGenerator.InWorkDirJob(self):
      print(f"Processing run {self.run_number} in job working directory '{os.getcwd()}'")
      # get EVIO files
      self._evio_file_names = sorted(glob.glob(f"hd_rawdata_{self.run_number:06d}_???.evio"))  # list of raw-data file names in working directory of job
      nmb_evio_files = len(self._evio_file_names)
      if nmb_evio_files == 0:
        print(f"WARNING: No EVIO files found in job working directory '{self.work_dir_job_path}'")
        self._missing_items["EVIO file(s)"].add(f"Run {self.run_number}")
        return
      print(f"Found {nmb_evio_files} EVIO files that were processed by this job")
      # ensure run directory exists
      if not os.path.isdir(self._run_dir_name):
        print(f"WARNING: '{self._run_dir_name}' does not exist or is not a directory; tagging {nmb_evio_files} EVIO files as failed")
        self._missing_items["run dir"].add(self._run_dir_name)
        for evio_file_name in self._evio_file_names:
          self._failed_evio_files.append(evio_file_name)
        return
      # loop over task directories
      max_nmb_tasks_digits = len(str(self.nmb_tasks))
      for task_index in range(self.nmb_tasks):
        evio_file_start_index = task_index * self.nmb_processes_per_task
        evio_file_end_index   = min(evio_file_start_index + self.nmb_processes_per_task, len(self._evio_file_names))
        if evio_file_end_index < evio_file_start_index:
          print(f"WARNING: Missing EVIO files for task {task_index}")
          self._missing_items["EVIO file(s)"].add(f"Task {task_index}")
          continue
        task_dir_path = f"{self._run_dir_name}/TASK{task_index:03d}"
        # ensure task directory exists
        if not os.path.isdir(task_dir_path):
          print(f"WARNING: '{task_dir_path}' does not exist or is not a directory; tagging {evio_file_end_index - evio_file_start_index} EVIO files as failed")
          self._missing_items["task dir(s)"].add(task_dir_path)
          for evio_file_index in range(evio_file_start_index,evio_file_end_index):
            self._failed_evio_files.append(f"hd_rawdata_{self.run_number:06d}_{evio_file_index:03d}.evio")
          return
        print(f"  Processing task [{task_index + 1:{max_nmb_tasks_digits}d}/{self.nmb_tasks:{max_nmb_tasks_digits}d}] in directory '{task_dir_path}'")
        # loop over file directories of task
        for evio_file_index in range(evio_file_start_index,evio_file_end_index):
          self._process_file_dir(task_index, evio_file_index)

  def _process_file_dir(
    self,
    task_index:      int,  # index of the task
    evio_file_index: int,  # index of the EVIO file
  ) -> None:
    """Processes the file directory defined by the arguments and appends to the file transfer map."""
    task_dir_path = f"{self._run_dir_name}/TASK{task_index:03d}"
    file_dir_path = f"{task_dir_path}/FILE{evio_file_index:03d}"
    evio_file_name = f"hd_rawdata_{self.run_number:06d}_{evio_file_index:03d}.evio"
    if not os.path.isdir(file_dir_path):
      print(f"WARNING: '{file_dir_path}' does not exist or is not a directory; tagging EVIO file as failed")
      self._missing_items["file dir(s)"].add(file_dir_path)
      self._failed_evio_files.append(evio_file_name)
      return
    print(f"    Processing file {evio_file_index} in directory '{file_dir_path}'")
    # determine whether hd_root processes succeeded
    # if not, copy log and output files into separate directory for further investigation
    hd_root_rc_file_path = f"{file_dir_path}/hd_root.rc"
    hd_root_return_code = get_hd_root_return_code(hd_root_rc_file_path)
    if hd_root_return_code is None or hd_root_return_code != 0:
      failed_file_dest_dir_path = f"{self.failed_hd_root_dest_dir_path}/{hd_root_return_code}/{self.run_number:06d}_{evio_file_index:03d}"  #TODO shouldn't this contain a job-unique identifier to prevent overwriting?
      if hd_root_return_code is None:
        print(f"WARNING: could not read hd_root return-code file at {hd_root_rc_file_path}", end = "")
        self._missing_items["log file(s)"].add(hd_root_rc_file_path)
      else:
        print(f"WARNING: hd_root return code for run {self.run_number} and EVIO file number {evio_file_index} is {hd_root_return_code}", end = "")
      print(f"; tagging EVIO file as failed and moving output files to '{failed_file_dest_dir_path}'")
      self._failed_evio_files.append(evio_file_name)
      # copy job and task log files
      self._process_job_log_files (               failed_file_dest_dir_path)
      self._process_task_log_files(task_dir_path, failed_file_dest_dir_path)
      # copy all files in file dir
      self._process_hd_root_output_files(file_dir_path, failed_file_dest_dir_path, evio_file_index, copy_all_files = True)
      return
    # process output and log files of successful hd_root processes
    job_info_dest_dir_path = f"{self.log_files_dest_dir_path}/job_info/{self.run_number:06d}/job_info_{self.run_number:06d}_{evio_file_index:03d}"  # target directory for all log files
    # copy job and task log files
    self._process_job_log_files    (               job_info_dest_dir_path)
    self._process_task_log_files   (task_dir_path, job_info_dest_dir_path)
    # copy hd_root log and output files
    self._process_hd_root_log_files(file_dir_path, job_info_dest_dir_path)
    self._process_hd_root_output_files(file_dir_path, self.hd_root_output_dest_dir_path, evio_file_index, copy_all_files = False)

  def _process_job_log_files(
    self,
    dest_dir_path: str,
  ) -> None:
    """Processes log files in the job directory and appends to the file transfer map."""
    log_file_names: list[str] = [
      # f"job_{self.run_number:06d}.diskquota",
      f"job_{self.run_number:06d}.env",
      f"job_{self.run_number:06d}.hostname",
      f"job_{self.run_number:06d}.mounts",
      f"job_{self.run_number:06d}_{self.job_id}.out",
      f"srun_{self.run_number:06d}.rc",
    ]
    # add task log files
    for task_index in range(self.nmb_tasks):
      log_file_names.append(f"task_{self.run_number:06d}_{task_index}.out")
    self._process_log_files(log_file_names, src_dir_path = ".", dest_dir_path = dest_dir_path)

  def _process_task_log_files(
    self,
    task_dir_path: str,
    dest_dir_path: str,
  ) -> None:
    """Processes node log files in the given task directory and appends to the file transfer map."""
    log_file_names: list[str] = [
      "node.cpuinfo",
      "node.env",
      "node.hostname",
      "node.mounts",
      "node.top",
    ]
    self._process_log_files(log_file_names, src_dir_path = task_dir_path, dest_dir_path = dest_dir_path)

  def _process_hd_root_log_files(
    self,
    file_dir_path: str,
    dest_dir_path: str,
  ) -> None:
    """Processes hd_root log files in the given file directory and appends to the file transfer map."""
    log_file_names: list[str] = [
      "hd_root.err",
      "hd_root.out",
      "hd_root.rc",
    ]
    self._process_log_files(log_file_names, src_dir_path = file_dir_path, dest_dir_path = dest_dir_path)

  def _process_log_files(
    self,
    log_file_names: list[str],
    src_dir_path:   str,
    dest_dir_path:  str,
  ) -> None:
    """Processes log files in the given log directory and appends to the file transfer map."""
    for log_file_name in log_file_names:
      log_file_path = f"{src_dir_path}/{log_file_name}"
      if not os.path.isfile(log_file_path):
        print(f"WARNING: expected log file '{log_file_path}' is missing; ignoring this file")
        self._missing_items["log file(s)"].add(log_file_path)
        continue
      self._file_transfer_map[log_file_path].add(f"{dest_dir_path}/{log_file_name}")

  def _process_hd_root_output_files(
    self,
    file_dir_path:   str,
    dest_dir_path:   str,
    evio_file_index: int,
    copy_all_files:  bool,
  ) -> None:
    """Processes hd_root output files in the given file directory and appends to the file transfer map."""
    if copy_all_files:
      # copy all files in file directory
      for local_file_path in sorted(glob.glob(f"{file_dir_path}/*")):
        dest_file_path = f"{dest_dir_path}/{os.path.basename(local_file_path)}"
        self._file_transfer_map[local_file_path].add(dest_file_path)
    else:
      # copy only expected files
      for dest_subdir_name, (file_base_name, file_type) in RECON_SUBDIR_BASENAME_MAP.items():
        file_name = f"hd_rawdata_{self.run_number:06d}_{evio_file_index:03d}.{file_base_name}.{file_type}" if file_type == "evio" else f"{file_base_name}.{file_type}"
        local_file_path = f"{file_dir_path}/{file_name}"
        if not os.path.isfile(local_file_path):
          print(f"WARNING: expected hd_root output file '{local_file_path}' is missing; ignoring this file")
          self._missing_items[f"{file_base_name} file(s)"].add(local_file_path)
          continue
        dest_file_name = f"{file_base_name}_{self.run_number:06d}_{evio_file_index:03d}.{file_type}"  # fix names of evio files and make file names of non-evio files unique
        dest_file_path = f"{dest_dir_path}/{dest_subdir_name}/{self.run_number:06d}/{dest_file_name}"
        self._file_transfer_map[local_file_path].add(f"{dest_file_path}")

  def print_missing_items_summary(self) -> None:
    """Prints a summary of missing items by item type."""
    if len(self._missing_items) == 0:
      print("Found no missing items; all expected files are present")
    else:
      print(f"Summary of missing items for run {self.run_number}:")
      for item_type, missing_items in sorted(self._missing_items.items()):
        print(f"{len(missing_items)} {item_type} missing:")
        for missing_item in sorted(missing_items):
          print(f"  {missing_item}")

  def print_summary_failed_evio_files(self) -> None:
    """Prints a summary of failed EVIO files."""
    nmb_failed_evio_files = len(self._failed_evio_files)
    if nmb_failed_evio_files == 0:
      print("Found no EVIO files, that are missing or for which hd_root has a non-zero return code")
    else:
      print(f"{nmb_failed_evio_files} out of {len(self._evio_file_names)} EVIO file(s) {'are' if nmb_failed_evio_files != 1 else 'is'} missing or have a non-zero hd_root return code:")
    for failed_evio_file in sorted(self._failed_evio_files):
      print(f"  {failed_evio_file}")


def define_swif2_output_files(
  job_id:                       int,  # Slurm ID of the job
  run_number:                   int,  # run number of the job
  work_dir_job_path:            str,  # path to working directory of job; assuming directory structure: <work_dir_job_path>/RUN<run number>/TASK<task index>/FILE<file number>
  nmb_tasks:                    int,  # number of tasks in the job
  nmb_processes_per_task:       int,  # number of processes per task used in the reconstruction launch
  hd_root_output_dest_dir_path: str,  # path of directory, to which the output files of hd_root processes with return code 0 will be copied
  log_files_dest_dir_path:      str,  # path of directory, to which the log files of hd_root processes with return code 0 will be copied
  failed_hd_root_dest_dir_path: str,  # path of directory, to which any log and output files of hd_root processes with non-zero return code will be copied for further investigation
) -> None:
  """Registers all output files with swif2 for transfer back to JLab."""
  file_transfer_map_gen = FileTransferMapGenerator(
    job_id                       = job_id,
    run_number                   = run_number,
    work_dir_job_path            = work_dir_job_path,
    nmb_tasks                    = nmb_tasks,
    nmb_processes_per_task       = nmb_processes_per_task,
    hd_root_output_dest_dir_path = hd_root_output_dest_dir_path,
    log_files_dest_dir_path      = log_files_dest_dir_path,
    failed_hd_root_dest_dir_path = failed_hd_root_dest_dir_path,
  )
  file_transfer_map_gen.process_work_dir()
  print("-------------------------------------------------------------------------------")
  file_transfer_map_gen.print_missing_items_summary()
  print("-------------------------------------------------------------------------------")
  file_transfer_map_gen.print_summary_failed_evio_files()
  print("-------------------------------------------------------------------------------")
  file_transfer_map: dict[str, set[str]] = file_transfer_map_gen.file_transfer_map
  nmb_transfers = sum(len(dest_file_paths) for dest_file_paths in file_transfer_map.values())
  print(f"Defining {nmb_transfers} transfers to JLab")
  for local_file_path, dest_file_paths in sorted(file_transfer_map.items()):
    for dest_file_path in sorted(dest_file_paths):
      cmd = f"./.swif/swif2 output '{local_file_path}' '{dest_file_path}'"  #TODO for some reason, swif2 is not in path
      print(cmd)
      subprocess.run(cmd, shell = True, check = False)


def test() -> None:
  start_time = time.time()
  global RECON_SUBDIR_BASENAME_MAP
  RECON_SUBDIR_BASENAME_MAP = {
    # ROOT files
    "hists" :                  ("hd_root",                "root"),
    "tree_bcal_hadronic_eff" : ("tree_bcal_hadronic_eff", "root"),
    "tree_fcal_hadronic_eff" : ("tree_fcal_hadronic_eff", "root"),
    "tree_PSFlux" :            ("tree_PSFlux",            "root"),
    "tree_sc_eff" :            ("tree_sc_eff",            "root"),
    "tree_tof_eff" :           ("tree_tof_eff",           "root"),
    "tree_TPOL" :              ("tree_TPOL",              "root"),
    "tree_TS_scaler" :         ("tree_TS_scaler",         "root"),
  }
  define_swif2_output_files(
    job_id                       = 53624465,
    run_number                   = 101156,
    work_dir_job_path            = "./test/test_work_dir_job2",
    nmb_tasks                    = 3,
    nmb_processes_per_task       = 8,
    hd_root_output_dest_dir_path = "./test/test_work_dir_job2_dest/hd_root_output",
    log_files_dest_dir_path      = "./test/test_work_dir_job2_dest/log_files",
    failed_hd_root_dest_dir_path = "./test/test_work_dir_job2_dest/failed_hd_root",
  )
  print("-------------------------------------------------------------------------------")
  elapsed_time_sec = int(time.time() - start_time)
  print(f"Wall time consumed by script: {elapsed_time_sec // 60} min, {elapsed_time_sec % 60} sec")


if __name__ == "__main__":
  test()
