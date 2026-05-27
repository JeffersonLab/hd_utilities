"""Collection of utility functions for the scripts in this directory."""

from __future__ import annotations

import argparse
import glob
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
from typing import TypeVar
import zlib


def print_command_line_arguments(args: argparse.Namespace) -> None:
  """Prints all command-line arguments and their values and the git hash."""
  this_script_file_name = os.path.basename(sys.argv[0])  # get file name of script that was launched
  print("-------------------------------------------------------------------------------")
  print(f"Running script {this_script_file_name} with arguments:")
  max_arg_name_length = max(len(arg_name) for arg_name in vars(args).keys())
  for arg_name, arg_value in sorted(vars(args).items()):  # sort keys for stable, tidy output
    print(f"{arg_name:>{max_arg_name_length + 2}} : {arg_value}")
  this_script_dir = os.path.dirname(os.path.abspath(__file__))  #TODO why not use `sys.argv[0]` here?
  if os.path.isfile(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH"):
    deployed_git_hash = open(f"{this_script_dir}/DEPLOYED_HD_UTILITIES_GIT_HASH", "r", encoding = "utf-8").read().strip()
    print(f"Using launch scripts from git commit hash: {deployed_git_hash}")
  #TODO add case where the script is run from the git repo
  print("-------------------------------------------------------------------------------")


def write_env_to_file(output_file_name: str = "./env") -> None:
  """Writes environment variables in alphabetical order into given file."""
  with open(output_file_name, "w", encoding = "utf-8") as file:
    for env_var_name in sorted(os.environ.keys()):
      env_var_value = os.environ[env_var_name]
      file.write(f"{env_var_name}={shlex.quote(env_var_value)}\n")  # ensure bash-safe quoting of the value
  print(f"Wrote environment variables to '{output_file_name}'")


def print_python_env() -> None:
  """Prints information about the Python environment, e.g. Python version and PYTHONPATH."""
  print(f"Running Python version {sys.version} on platform {sys.platform}")
  print(f"Using PYTHONPATH={os.environ.get('PYTHONPATH')}")


KeyType   = TypeVar("KeyType",   bound = object)  # any not-None type
ValueType = TypeVar("ValueType", bound = object)  # any not-None type
def ensure_dict_value_exists(
  dictionary: dict[KeyType, ValueType | None],
  key:        KeyType,
) -> ValueType:
  """Returns the value for the given key in the dictionary, raising ValueError if the value is None."""
  value = dictionary[key]
  if value is None:
    raise ValueError(f"Missing value for key '{key}'")
  return value


def get_directory_size(path: str | Path) -> int:
  """Returns the total size in bytes of all files (including symlinks) in the directory tree rooted at the given path."""
  return sum(file_path.stat().st_size for file_path in Path(path).rglob("*") if file_path.is_file())


def get_file_crc32(
  file_path:       str,
  read_chunk_size: int = 2**23,  # 8 MiB
) -> str:
  """Calculates the CRC32 checksum of the given file."""
  crc = 0
  with open(file_path, "rb") as file:
    while chunk := file.read(read_chunk_size):
      crc = zlib.crc32(chunk, crc)
  return f"{crc & 0xffffffff:x}"  # convert to 32 bit unsigned integer and format as hexadecimal string, no leading zeros


def get_evio_file_paths_for_run(
  run_number:    int,
  raw_data_root: str,
) -> list[str]:
  """Gets the list of EVIO file paths for the given run number and raw data root directory."""
  evio_run_dir = f"{raw_data_root}/Run{run_number:06d}"
  evio_file_paths: list[str] = sorted(glob.glob(f"{evio_run_dir}/hd_rawdata_{run_number:06d}_???.evio"))
  return evio_file_paths


def read_run_numbers_from_file(run_number_list_file: str) -> list[int]:
  """Reads a list of run numbers from the given file. The file is expected to contain one run number per line, and may contain blank lines which are ignored."""
  print(f"Reading list of run numbers from file '{run_number_list_file}'")
  run_numbers: list[int] = []
  with open(run_number_list_file) as file:
    run_numbers = [int(line.strip()) for line in file if line.strip()]  # skip blank lines
  print(f"Read {len(run_numbers)} run number(s) from file '{run_number_list_file}'")
  return run_numbers


def get_hd_root_return_code(hd_root_rc_file_path: str) -> int | None:
  """Reads the hd_root return code from the given file and returns it as an int or `None` if unsuccessful."""
  try:
    with open(hd_root_rc_file_path, "r", encoding="utf-8") as file:
      hd_root_rc_file_content = file.read()
  except OSError as e:  # file not found, permission denied, etc.
    print(f"WARNING: unable to open hd_root return-code file '{hd_root_rc_file_path}': {e}")
    return None
  match = re.search(r"return code (\d+)$", hd_root_rc_file_content)
  if not match:
    print(f"WARNING: cannot get hd_root return code from malformed file '{hd_root_rc_file_path}': '{hd_root_rc_file_content}'")
    return None
  return int(match.group(1))


def get_file_transfer_paths(
  run_number:         int,
  swif_output_root:   str,
  transfer_all_files: bool = False,  # if True, do not filter out any files
) -> list[tuple[str, str]]:
  """Gets list of local relative paths w.r.t. current directory and absolute remote destination paths of all output files that should be transferred back to JLab."""
  # this function assumes that the current directory is the working directory of the job
  # first greedily collect all potential output items, then filter out directories and files that should not be transferred back to JLab
  relative_output_paths: list[str] = []
  relative_output_paths += sorted(glob.glob(f"*"))  # include all items in job dir
  relative_output_paths += sorted(glob.glob(f"RUN{run_number:06d}/*"))  # include all items in run dir
  # loop over task dirs
  task_dirs: list[str] = sorted(glob.glob(f"RUN{run_number:06d}/TASK???"))
  for task_dir in task_dirs:
    relative_output_paths += sorted(glob.glob(f"{task_dir}/*"))  # include all items in task dir
    # loop over file dirs
    file_dirs: list[str] = sorted(glob.glob(f"{task_dir}/FILE???"))
    for file_dir in file_dirs:
      if not transfer_all_files:
        hd_root_return_code = get_hd_root_return_code(f"{file_dir}/hd_root.rc")
        if hd_root_return_code is None or hd_root_return_code != 0:
          # do not copy hd_root output files for failed hd_root processes; but try to recover debug info
          print(f"WARNING: skipping hd_root output files in '{file_dir}' because hd_root return code is {hd_root_return_code} != 0")
          relative_output_paths += [f"{file_dir}/hd_root.err", f"{file_dir}/hd_root.out", f"{file_dir}/hd_root.rc"]  # transfer hd_root log files for debugging
          relative_output_paths += sorted(glob.glob(f"{file_dir}/core.hd_root*"))  # transfer core files for debugging
          continue
      relative_output_paths += sorted(glob.glob(f"{file_dir}/*"))  # include all items in file dir
  file_transfer_paths: list[tuple[str, str]] = []  # list of pairs of relative local file paths and absolute remote destination file paths
  for relative_output_path in relative_output_paths:
    # filter out all directories and nonexisting and unwanted and files
    if not os.path.isfile(relative_output_path) or os.path.islink(relative_output_path):  # keep only existing, non-symlinked files
      continue
    output_file_name = os.path.basename(relative_output_path)
    if output_file_name.startswith(".") or output_file_name.startswith("__"):  # skip hidden files and swif2 system files
      continue
    if re.fullmatch(r"hd_rawdata_\d{6}_\d{3}\.evio", output_file_name):  # skip raw-data files that match hd_rawdata_XXXXXX_YYY.evio, with 6-digit run number XXXXXX and 3-digit file number YYY
      continue
    file_transfer_paths.append((relative_output_path, f"{swif_output_root}/{relative_output_path}"))
  return file_transfer_paths


def define_swif2_output_files(
  run_number:         int,
  swif_output_root:   str,
  transfer_all_files: bool = False,  # if True, do not filter out any files
) -> None:
  """Registers all output files with swif2 for transfer back to JLab."""
  file_transfer_paths: list[tuple[str, str]] = get_file_transfer_paths(run_number, swif_output_root, transfer_all_files)
  print(f"Defining {len(file_transfer_paths)} output files for transfer back to JLab")
  for local_output_file_path, remote_output_file_path in file_transfer_paths:
    output_cmd = f"./.swif/swif2 output '{local_output_file_path}' '{remote_output_file_path}'"  #TODO for some reason, swif2 is not in path
    print(output_cmd)
    subprocess.run(output_cmd, shell = True, check = False)


#TODO is this needed?
def get_file_number_from_evio_file_name(evio_file_path: str) -> int | None:
  """Extracts the 3-digit file number from the given EVIO file name, or returns `None` if it cannot be extracted."""
  evio_file_name = os.path.basename(evio_file_path)
  if len(evio_file_name) < 21:
    print(f"WARNING: EVIO file name '{evio_file_path}' is too short to contain a valid file number; expected at least 21 characters; ignoring")
    return None
  file_number_str = evio_file_name[18:21]  # extract the 3-digit file number from the file name pattern
  try:
    file_number = int(file_number_str)
    return file_number
  except ValueError:
    print(f"WARNING: expected file name pattern 'hd_rawdata_XXXXXX_YYY*'; could not extract 3-digit file number YYY from EVIO file name '{evio_file_path}'; ignoring")
    return None
