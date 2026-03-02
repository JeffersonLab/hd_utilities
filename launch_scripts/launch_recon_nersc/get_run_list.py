#!/usr/bin/env python3

"""
Generates a list of runs and files to process for a given run period.
"""

import glob
import math
import mysql.connector
import sys

import rcdb


TESTMODE = True  # True=only print commands, but don't actually submit jobs
VERBOSE  = 3     # 1 is default

RUNPERIOD  = '2022-05'
LAUNCHTYPE = 'recon'  # 'offmon' or 'recon'
VER        = '02-perl'
BATCH      = 'NERSC-multi'
WORKFLOW   = f'{LAUNCHTYPE}_{RUNPERIOD}_ver{VER}_{BATCH}'
NAME       = f'GLUEX_{LAUNCHTYPE}'

RCDB_QUERY    = "@is_cpp_production and @status_approved"  # Comment out for all runs in range MINRUN-MAXRUN
RUNS          = []      # List of runs to process. If empty, MINRUN-MAXRUN are searched in RCDB
EXCLUDE_RUNS  = []      # Runs that should be excluded from processing
MINRUN        = 100000  # If RUNS is empty, then RCDB queried for this range
MAXRUN        = 109999  # If RUNS is empty, then RCDB queried for this range
MINFILENO     = 0       # Min file number to process for each run (n.b. file numbers start at 0!)
MAXFILENO     = 999     # Max file number to process for each run (n.b. file numbers start at 0!)
FILE_FRACTION = 1.0     # Fraction of files to process for each run in specified range (see GetFileNumbersToProcess)

RCDB_HOST = 'hallddb.jlab.org'
RCDB_USER = 'rcdb'
RCDB      = None
BAD_RCDB_QUERY_RUNS = []  # will be filled with runs that are missing evio_file_count field in RCDB query
BAD_FILE_COUNT_RUNS = []  # will be filled with runs where number of evio files could not be obtained by any method
BAD_MSS_FILE_RUNS   = {}  # will be filled with runs/files where the stub file in /mss is missing


def GetRunInfo():

  # Get the list of runs to process and the number of EVIO files for each.
  # The list is returned in the form of a dictionary with the run numbers
  # as keys and the maximum evio file number for that run as values.
  # Which runs show up in the list depends on how the RUNS and RCDB_QUERY
  # globals are set:
  #
  # RUNS is not None: All runs in the list are included
  # RUNS is empty and RCDB_QUERY is None: All runs in the range MINRUN-MAXRUN inclusive are included
  # RUNS is empty and RCDB_QUERY is not None: RCDB is queried for the list of runs.
  #
  # n.b. that for the first 2 options above, the GetNumEVIOFiles routine
  # below is called which queries the RCDB via mysql directly so the RCDB
  # python module does not actually need to be in PYTHONPATH. For the 3rd
  # option, the RCDB python API is used so it is needed.

  global RUNS, MINRUN, MAXRUN, RCDB_QUERY, RUN_LIST_SOURCE, BAD_RCDB_QUERY_RUNS, BAD_FILE_COUNT_RUNS
  good_runs = {}

  # If RCDB_QUERY is not defined, define with value None
  try:
    RCDB_QUERY
  except:
    RCDB_QUERY = None

  # Query through RCDB API
  if len(RUNS) == 0 and RCDB_QUERY is not None:
    RUN_LIST_SOURCE = f'RCDB {MINRUN}-{MAXRUN} (query="{RCDB_QUERY}")'
    print ('Querying RCDB for run list ....')

    # Import RCDB python module. Add a path on the CUE just in case
    # PYTHONPATH is not already set
    sys.path.append('/group/halld/Software/builds/Linux_Alma9-x86_64-gcc11.5.0-cntr/rcdb/rcdb_0.09.01/python')
    db = rcdb.RCDBProvider(f'mysql://{RCDB_USER}@{RCDB_HOST}/rcdb2')
    print (f'RCDB_QUERY = {RCDB_QUERY}')
    for r in db.select_runs(RCDB_QUERY, MINRUN, MAXRUN):
      evio_files_count = r.get_condition_value('evio_files_count')
      if evio_files_count == None:
        print(f'ERROR in RCDB: Run {r.number} has no value for evio_files_count!...')
        BAD_RCDB_QUERY_RUNS.append( int(r.number) )
        print('Attempting to extract number of files by examining /mss ...')
        rawdatafiles = glob.glob(f'/mss/halld/RunPeriod-{RUNPERIOD}/rawdata/Run{r.number:06d}/hd_rawdata_{r.number:06d}_*.evio')
        if len(rawdatafiles) > 0: evio_files_count = len(rawdatafiles)
      if evio_files_count == None:
        print(f'ERROR getting number of files for: Run {r.number}')
        BAD_FILE_COUNT_RUNS.append( int(r.number) )
        continue
      good_runs[r.number] = int(evio_files_count)
  elif len(RUNS)==0:
    RUN_LIST_SOURCE = f'All runs in range {MINRUN}-{MAXRUN}'
    print (f'Getting info for all runs in range {MINRUN}-{MAXRUN} ....')
    for RUN in range(MINRUN, MAXRUN+1):
      good_runs[RUN] = GetNumEVIOFiles(RUN)
  else:
    RUN_LIST_SOURCE = 'Custom list: ' + ' '.join([str(x) for x in RUNS])
    print ('Getting info for runs : ' + ' '.join([str(x) for x in RUNS]))
    for RUN in RUNS: good_runs[RUN] = GetNumEVIOFiles(RUN)

  # Filter out runs in the EXCLUDE_RUNS list
  global EXCLUDE_RUNS
  good_runs_filtered = {}
  for run in good_runs.keys():
    if run not in EXCLUDE_RUNS:
      good_runs_filtered[run] = good_runs[run]

  return good_runs_filtered


def GetNumEVIOFiles(RUN):

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
  sql += f' AND condition_types.name="evio_files_count" AND run_number={RUN}'
  cur.execute(sql)
  c_rows = cur.fetchall()
  if len(c_rows)>0:
    Nfiles = int(c_rows[0][0])
  else:
    BAD_RCDB_QUERY_RUNS.append(RUN)
    print('Attempting to extract number of files by examining /mss ...')
    rawdatafiles = glob.glob(f'/mss/halld/RunPeriod-{RUNPERIOD}/rawdata/Run{RUN:06d}/hd_rawdata_{RUN:06d}_*.evio')
    if len(rawdatafiles) > 0:
      Nfiles = len(rawdatafiles)
    else:
      BAD_FILE_COUNT_RUNS.append(RUN)


  return Nfiles


def GetFileNumbersToProcess(Nfiles):

  # This will return a list of file numbers to process for a run
  # given the number of files given by Nfiles. The list is determined
  # by the values MINFILENO, MAXFILENO and FILE_FRACTION.
  # First, the actual range of file numbers for the run is determined
  # by MINFILENO-MAXFILENO, but clipped if necessary to Nfiles.
  # Next, a list of files in that range representing FILE_FRACTION
  # of the range is formed and returned.
  #
  # Example 1: Process first 10 files of each run:
  #             MINFILENO = 0
  #             MAXFILENO = 9
  #         FILE_FRACTION = 1.0
  #
  # Example 2: Process first 5% of files of each run of
  #            files distributed throughout run:
  #             MINFILENO = 0
  #             MAXFILENO = 1000    n.b. set this to something really big
  #         FILE_FRACTION = 0.05
  #

  global MINFILENO, MAXFILENO, FILE_FRACTION

  # Make sure MINFILENO is not greater than max file number for the run
  if MINFILENO >= Nfiles:
    return []

  # Limit max file number to how many there are for this run according to RCDB
  maxfile = MAXFILENO+1  # set to 1 past the actual last file number we want to process
  if Nfiles < maxfile:
    maxfile = Nfiles

  # If FILE_FRACTION is 1.0 then we want all files in the range.
  if FILE_FRACTION == 1.0:
    return range( MINFILENO, maxfile)

  # At this point, maxfile should be one greater than the last file
  # number we want to process. If the last file we want to process
  # is the last file in the run, then it could be a short file. Thus,
  # use the next to the last file in the run to determine the range.
  if Nfiles < MAXFILENO:
    maxfile -= 1

  # Number of files in run to process
  Nrange = float(maxfile-1) - float(MINFILENO)
  N = math.ceil(FILE_FRACTION * Nrange)
  if N<2:
    return [MINFILENO]
  nskip = Nrange/(N-1)
  filenos = []
  for i in range(0, int(N)):
    filenos.append(int(i*nskip))
  # print ('Nrange=%f N=%f nskip=%f ' % (Nrange, N, nskip))

  return filenos


def PrintConfigSummary():
  print ('=================================================')
  print (f'Launch Summary  {"**** TEST MODE ****" if TESTMODE else ""}')
  print ('-----------------------------------------------')
  print (f'             RunPeriod: {RUNPERIOD}')
  print (f'           Launch type: {LAUNCHTYPE}')
  print (f'               Version: {VER}')
  print (f'                 batch: {BATCH}')
  print (f'              WORKFLOW: {WORKFLOW}')
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
  NUM['files_submitted'] = 0
  NUM['jobs_to_process'] = 0
  NUM['jobs_submitted'] = 0

  # Get list of runs with number of evio files for each.
  # (parameters for search set at top of file)
  good_runs = GetRunInfo()
  #good_runs = db.select_runs(RCDB_QUERY, MINRUN, MAXRUN)

  # Print some info before doing anything
  for n in [x for (y,x) in good_runs.items()]:
    NUM['files_to_process'] += len(GetFileNumbersToProcess(n))

  print('\n')
  print('NOTE: The values in BAD_RCDB_QUERY_RUNS is informative about what is missing from')
  print('      the RCDB. An attempt to recover the information from the /mss filesystem')
  print('      was also made. Values in BAD_FILE_COUNT_RUNS are ones for which that failed.')
  print('      Thus, only runs listed in BAD_FILE_COUNT_RUNS will not have any jobs submitted')
  print (f'BAD_RCDB_QUERY_RUNS={BAD_RCDB_QUERY_RUNS}')
  print (f'BAD_FILE_COUNT_RUNS={BAD_FILE_COUNT_RUNS}')
  print (f'BAD_MSS_FILE_RUNS={BAD_MSS_FILE_RUNS}')

  PrintConfigSummary()

  NUM['missing_mss_files'] = 0
  for run,files in BAD_MSS_FILE_RUNS.items():
    NUM['missing_mss_files'] += len(files)

  print('')
  print(f'WORKFLOW: {WORKFLOW}')
  print('------------------------------------')
  print(f'Number of runs: {len(good_runs)}  (only good runs)')
  print(f'{NUM["files_submitted"]}/{NUM["files_to_process"]} total files submitted  ({NUM["missing_mss_files"]} files missing from mss)')
  print(f'{NUM["jobs_submitted"]}/{NUM["jobs_to_process"]} total jobs submitted')
  print('')
