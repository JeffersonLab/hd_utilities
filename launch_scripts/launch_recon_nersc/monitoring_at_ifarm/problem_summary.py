#!/usr/bin/env python3

"""
This is a convenience script for printing the count of each problem
type for all jobs in a given swif2 workflow.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import io
import subprocess

import dotenv


#TODO use function in launch_size.py
def ensure_value_exists(
  config: dict[str, str | None],
  key:    str,
) -> str:
  """Ensure that the given key exists in the config dictionary and has a non-None value; return the value."""
  value = config[key]
  if value is None:
    raise ValueError(f"Missing value for '{key}' key in .env file")
  return value


def main(args: argparse.Namespace) -> None:
  swif2_workflow_name = args.workflow

  # get job summary for the workflow
  try:
    result = subprocess.run(
      [
        "swif2",
        "status",
        "-workflow",
        swif2_workflow_name,
      ],
      capture_output = True,
      text = True,
      check = True,
    )
  except subprocess.CalledProcessError as e:
    print(f"'{e.cmd}' failed with error: '{e.stderr}'")
    return
  # parse `'key' = 'value'` pairs returned by `swif2 status` command into a dictionary
  swif2_info = dotenv.dotenv_values(stream = io.StringIO(result.stdout))

  # process command output to extract problem types and their counts
  nmb_jobs            = int(ensure_value_exists(swif2_info, "jobs"))
  nmb_jobs_dispatched = int(ensure_value_exists(swif2_info, "dispatched"))
  nmb_jobs_succeeded  = int(ensure_value_exists(swif2_info, "succeeded"))
  nmb_jobs_problems   = int(ensure_value_exists(swif2_info, "problems"))
  job_problems: list[str] = []
  if nmb_jobs_problems > 0:
    job_problems = ensure_value_exists(swif2_info, "problem_types").split(",")

  # get runs with problems
  try:
    result = subprocess.run(
      [
        "swif2",
        "status",
        "-workflow",
        swif2_workflow_name,
        "-problems",
      ],
      capture_output = True,
      text = True,
      check = True,
    )
  except subprocess.CalledProcessError as e:
    print(f"'{e.cmd}' failed with error: '{e.stderr}'")
    return

  # process command output to count each problem type
  #NOTE cannot easily apply dotenv here as keys are repeated for each job
  nmb_problems: defaultdict[str, int] = defaultdict(int)  # problem type -> count
  for line in result.stdout.splitlines():
    for problem in job_problems:
      if problem in line:
        nmb_problems[problem] += 1

  # print summary
  print("=======================================================")
  print(f"Summary of problems for swif2 workflow")
  print("-------------------------------------------------------")
  print(f" workflow name: {swif2_workflow_name}")
  print(f"number of jobs: {nmb_jobs}")
  print(f"    dispatched: {nmb_jobs_dispatched}")
  print(f"     succeeded: {nmb_jobs_succeeded}")
  print(f" with problems: {nmb_jobs_problems}")
  if nmb_jobs_problems > 0:
    print(f"  problem types: {', '.join(job_problems)}")
    print("-------------------------------------------------------")
    print("Count by problem type:")
    for problem, count in nmb_problems.items():
      print(f"    {count:4d} - {problem}")
    print("=======================================================\n")
    print("Retry command:\n")
    print(f"swif2 retry-jobs -workflow {swif2_workflow_name} -problems {' '.join(job_problems)}\n")
  else:
    print("=======================================================\n")
    print("No problems found. No retry needed.\n")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Print the count of each problem type for all jobs in a given swif2 workflow.")
  parser.add_argument("workflow", help="The swif2 workflow name. See a list by running 'swif2 list'")
  main(parser.parse_args())
