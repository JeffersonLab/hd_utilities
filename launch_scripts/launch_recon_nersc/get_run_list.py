#!/usr/bin/env python3

"""
Generates a list of runs and files to process for a given run period.
"""

from __future__ import annotations

import glob
import mysql.connector
import sys

import rcdb


RUNPERIOD = '2022-05'
MINFILENO = 0    # Min file number to process for each run (n.b. file numbers start at 0!)
MAXFILENO = 999  # Max file number to process for each run (n.b. file numbers start at 0!)

RCDB_HOST = 'hallddb.jlab.org'
RCDB_USER = 'rcdb'
RCDB      = None
BAD_RCDB_QUERY_RUNS: list[int] = []  # will be filled with runs that are missing evio_file_count field in RCDB query
BAD_FILE_COUNT_RUNS: list[int] = []  # will be filled with runs where number of evio files could not be obtained by any method
BAD_MSS_FILE_RUNS: dict[int, list[int]] = {}  # will be filled with runs/files where the stub file in /mss is missing


def GetRunInfo(
  run_period:       str,         # e.g. '2022-05'
  rcdb_query:       str | None,  # if None all runs in range run_number_min-run_number_max
  run_list:         list[int],   # list of runs to process; if empty, run_number_min-run_number_max are searched in RCDB
  run_number_min:   int,         # if run_list is empty, then RCDB is queried for this range
  run_number_max:   int,         # if run_list is empty, then RCDB is queried for this range
  run_list_exclude: list[int],   # runs that should be excluded from processing
) -> dict[int, int]:
  """
  Get the list of runs to process and the number of EVIO files for each.
  The list is returned in the form of a dictionary with the run numbers
  as keys and the maximum evio file number for that run as values.
  Which runs show up in the list depends on how the run_list and rcdb_query
  globals are set:

  run_list is not None: All runs in the list are included
  run_list is empty and rcdb_query is None: All runs in the range run_number_min-run_number_max inclusive are included
  run_list is empty and rcdb_query is not None: RCDB is queried for the list of runs.

  n.b. that for the first 2 options above, the GetNumEVIOFiles routine
  below is called which queries the RCDB via mysql directly so the RCDB
  python module does not actually need to be in PYTHONPATH. For the 3rd
  option, the RCDB python API is used so it is needed.
  """

  global RUN_LIST_SOURCE, BAD_RCDB_QUERY_RUNS, BAD_FILE_COUNT_RUNS
  good_runs: dict[int, int] = {}

  # Query through RCDB API
  if len(run_list) == 0 and rcdb_query is not None:
    RUN_LIST_SOURCE = f'RCDB {run_number_min}-{run_number_max} (query = "{rcdb_query}")'
    print ('Querying RCDB for run list ....')
    db = rcdb.RCDBProvider(f'mysql://{RCDB_USER}@{RCDB_HOST}/rcdb2')
    print (f'rcdb_query = {rcdb_query}')
    for run in db.select_runs(rcdb_query, run_number_min, run_number_max):
      run_number = int(run.number)
      evio_files_count = run.get_condition_value('evio_files_count')
      if evio_files_count is None:
        print(f'ERROR in RCDB: Run {run_number} has no value for evio_files_count!...')
        BAD_RCDB_QUERY_RUNS.append(run_number)
        print('Attempting to extract number of files by examining /mss ...')
        rawdatafiles = glob.glob(f'/mss/halld/RunPeriod-{run_period}/rawdata/Run{run_number:06d}/hd_rawdata_{run_number:06d}_*.evio')
        if len(rawdatafiles) > 0:
          evio_files_count = len(rawdatafiles)
      if evio_files_count is None:
        print(f'ERROR getting number of files for: Run {run_number}')
        BAD_FILE_COUNT_RUNS.append(run_number)
        continue
      good_runs[run_number] = int(evio_files_count)
  elif len(run_list) == 0:
    RUN_LIST_SOURCE = f'All runs in range {run_number_min}-{run_number_max}'
    print (f'Getting info for all runs in range {run_number_min}-{run_number_max} ....')
    for run_number in range(run_number_min, run_number_max + 1):
      good_runs[run_number] = GetNumEVIOFiles(run_period, run_number)
  else:
    RUN_LIST_SOURCE = 'Custom list: ' + ' '.join([str(run_number) for run_number in run_list])
    print ('Getting info for runs : ' + ' '.join([str(run_number) for run_number in run_list]))
    for run_number in run_list:
      good_runs[run_number] = GetNumEVIOFiles(run_period, run_number)

  # Filter out runs in the run_list_exclude list
  good_runs_filtered = {}
  for run_number in good_runs.keys():
    if run_number not in run_list_exclude:
      good_runs_filtered[run_number] = good_runs[run_number]

  return good_runs_filtered


def GetNumEVIOFiles(
  run_period: str,  # e.g. '2022-05'
  run_number: int,
) -> int:

  global BAD_RCDB_QUERY_RUNS, BAD_FILE_COUNT_RUNS

  # Access RCDB to get the number of EVIO files for this run.
  # n.b. the file numbers start from 0 so the last valid file
  # number will be one less than the value returned
  global RCDB, cnx, cur
  if not RCDB:
    try:
      RCDB = f'mysql://{RCDB_USER}@{RCDB_HOST}/rcdb'
      cnx = mysql.connector.connect(user=RCDB_USER, host=RCDB_HOST, database='rcdb')
      cur = cnx.cursor()  # using dictionary=True crashes when running on ifarm (??)
    except Exception as e:
      print (f'Error connecting to RCDB: {RCDB}')
      print (str(e))
      sys.exit(-1)

  Nfiles = 0
  sql  = 'SELECT int_value from conditions,condition_types WHERE condition_type_id=condition_types.id'
  sql += f' AND condition_types.name="evio_files_count" AND run_number={run_number}'
  cur.execute(sql)
  c_rows = cur.fetchall()
  if len(c_rows)>0:
    Nfiles = int(c_rows[0][0])
  else:
    BAD_RCDB_QUERY_RUNS.append(run_number)
    print('Attempting to extract number of files by examining /mss ...')
    rawdatafiles = glob.glob(f'/mss/halld/RunPeriod-{run_period}/rawdata/Run{run_number:06d}/hd_rawdata_{run_number:06d}_*.evio')
    if len(rawdatafiles) > 0:
      Nfiles = len(rawdatafiles)
    else:
      BAD_FILE_COUNT_RUNS.append(run_number)


  return Nfiles


def PrintConfigSummary():
  print ('=================================================')
  print ('Launch Summary')
  print ('-----------------------------------------------')
  print (f'             RunPeriod: {RUNPERIOD}')
  print (f'    Origin of run list: {RUN_LIST_SOURCE}')
  print (f'        Number of runs: {len(good_runs)}')
  print (f'       Number of files: {NUM["files_to_process"]} (maximum {MAXFILENO-MINFILENO+1} files/run)')
  print (f'         Min. file no.: {MINFILENO}')
  print (f'         Max. file no.: {MAXFILENO}')
  print ('=================================================')


if __name__ == "__main__":
  # Initialize some counters
  NUM = {}
  NUM['files_to_process'] = 0
  NUM['files_submitted']  = 0
  NUM['jobs_to_process']  = 0
  NUM['jobs_submitted']   = 0

  # Get list of runs with number of evio files for each.
  # (parameters for search set at top of file)
  good_runs = GetRunInfo(
    run_period       = RUNPERIOD,
    rcdb_query       = "@is_cpp_production and @status_approved",
    # rcdb_query       = "@is_cpp_production and @status_approved==0 and status>0",
    run_list         = [],
    run_number_min   = 100000,
    run_number_max   = 109999,
    run_list_exclude = [],
  )
  #good_runs = db.select_runs(rcdb_query, run_number_min, run_number_max)

  for n in [x for (_, x) in good_runs.items()]:
    NUM['files_to_process'] += n

  print('\n')
  print('NOTE: The values in BAD_RCDB_QUERY_RUNS is informative about what is missing from')
  print('      the RCDB. An attempt to recover the information from the /mss filesystem')
  print('      was also made. Values in BAD_FILE_COUNT_RUNS are ones for which that failed.')
  print('      Thus, only runs listed in BAD_FILE_COUNT_RUNS will not have any jobs submitted')
  print (f'BAD_RCDB_QUERY_RUNS={BAD_RCDB_QUERY_RUNS}')
  print (f'BAD_FILE_COUNT_RUNS={BAD_FILE_COUNT_RUNS}')
  print (f'BAD_MSS_FILE_RUNS={BAD_MSS_FILE_RUNS}')
  #TODO
  # # Verify stub file exists before submitting job
  # if not os.path.exists( MSSFILE ):
  #   if RUN not in BAD_MSS_FILE_RUNS.keys(): BAD_MSS_FILE_RUNS[RUN] = []
  #   BAD_MSS_FILE_RUNS[RUN].append(FILE)
  # else:
  #   mss_files[EVIOFILE] = MSSFILE
  #   outpath = 'RUN%06d/FILE%03d' % (RUN, FILE)
  #   outdirs[EVIOFILE] = outpath

  PrintConfigSummary()

  NUM['missing_mss_files'] = 0
  for run,files in BAD_MSS_FILE_RUNS.items():
    NUM['missing_mss_files'] += len(files)

  print('------------------------------------')
  print(f'Number of runs: {len(good_runs)}  (only good runs)')
  print(f'{NUM["files_submitted"]}/{NUM["files_to_process"]} total files submitted  ({NUM["missing_mss_files"]} files missing from mss)')
  print(f'{NUM["jobs_submitted"]}/{NUM["jobs_to_process"]} total jobs submitted')
  print('')
