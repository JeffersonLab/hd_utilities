#!/usr/bin/env python3

import datetime
import os
import subprocess
import sys

import mysql.connector
import rcdb


d = datetime.datetime.today()
print(d.strftime("job start: %Y-%m-%d %H:%M:%S\n"))

argvs = sys.argv
argc = len(argvs)

if argc == 7:
  MINRUN = argvs[1]
  MAXRUN = argvs[2]
  BATCHNB = argvs[3]
  DATEDIR = argvs[4]
  TYPE = argvs[5]
  testing = argvs[6]
else:
  print("python batch_accounting.py [MINRUN] [MAXRUN] [BATCHNB] [DATEDIR] [TYPE:reprocessing/counting] [testing/production]")
  sys.exit()


RCDB_HOST = "hallddb.jlab.org"
RCDB_USER = "rcdb"
RCDB_QUERY = "@is_dirc_production and @status_approved"
db = rcdb.RCDBProvider(f"mysql://{RCDB_USER}@{RCDB_HOST}/rcdb")

if TYPE == "reprocessing":
  if testing == "production":
    cmd = (f"mkdir -p {DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB}/launch")
    os.system(cmd)
    cmd = f"cp *.* {DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB}/launch/"
    os.system(cmd)
    cmd = (f"scp -r {DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB} bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/")
    os.system(cmd)
    cmd = (f"scp jana_recon_nersc_{BATCHNB}.config bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/{DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB}/launch/")
    os.system(cmd)
  if testing == "testing":
    cmd = (f"mkdir -p {DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB}/launch")
    print(cmd)
    cmd = f"cp *.* {DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB}/launch/"
    print(cmd)
    cmd = (f"scp -r {DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB} bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/")
    print(cmd)


ctr = 0
rnb_ctr = 0
evio_files_ctr = 0
true_bit = 0
for r in db.select_runs(RCDB_QUERY, MINRUN, MAXRUN):
  pol = r.get_condition_value("polarization_direction")
  beam_current = r.get_condition_value("beam_current")
  evio_files_nb = r.get_condition_value("evio_files_count")
  rnb = r.number
  if beam_current > 0:
    rnb_ctr = 1 + rnb_ctr
    evio_files_ctr += evio_files_nb
    for evio_index in range(evio_files_nb):
      if evio_index < 10:
        cache_evio_file = f"/cache/halld/RunPeriod-2019-11/recon/ver01/REST/0{rnb}/dana_rest_0{rnb}_00{evio_index}.hddm"
        mss_evio_file = f"/mss/halld/RunPeriod-2019-11/recon/ver01/REST/0{rnb}/dana_rest_0{rnb}_00{evio_index}.hddm"
        evio_file = f"{rnb}_00{evio_index}"
      if 9 < evio_index and evio_index < 100:
        cache_evio_file = f"/cache/halld/RunPeriod-2019-11/recon/ver01/REST/0{rnb}/dana_rest_0{rnb}_0{evio_index}.hddm"
        mss_evio_file = f"/mss/halld/RunPeriod-2019-11/recon/ver01/REST/0{rnb}/dana_rest_0{rnb}_0{evio_index}.hddm"
        evio_file = f"{rnb}_0{evio_index}"
      if 99 < evio_index and evio_index < 999:
        cache_evio_file = f"/cache/halld/RunPeriod-2019-11/recon/ver01/REST/0{rnb}/dana_rest_0{rnb}_{evio_index}.hddm"
        mss_evio_file = f"/mss/halld/RunPeriod-2019-11/recon/ver01/REST/0{rnb}/dana_rest_0{rnb}_{evio_index}.hddm"
        evio_file = f"{rnb}_{evio_index}"

      py_name = f"launch_iu_{evio_file}.py"

      is_mssfile = os.path.isfile(mss_evio_file)
      is_cachefile = os.path.isfile(cache_evio_file)

      if os.path.isfile(mss_evio_file) or os.path.isfile(cache_evio_file):
        true_bit = 1
      else:
        if TYPE == "reprocessing":
          subprocess.call([f"sed 's,RUNNB,'{rnb}',g; s,FILENB,'{evio_index}',g; s,BATCHNB,'{BATCHNB}',g; s,DATEDIR,'{DATEDIR}',g' dummy.py > {py_name}"], shell = True)
          cmd = (f"chmod +x {py_name}")
          os.system(cmd)
          cmd = (f"scp {py_name} bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/{DATEDIR}.recon_2019-11_ver01_catchup_batch{BATCHNB}/launch/")
          if testing == "production":
            os.system(cmd)
          if testing == "testing":
            print(cmd)
          cmd = (f"./{py_name}")
          if testing == "production":
            os.system(cmd)
          if testing == "testing":
            print(cmd)
          print(evio_file)
          ctr = ctr + 1
        if TYPE == "counting":
          print(evio_file)
          ctr = ctr + 1

print(f"batch{BATCHNB} has {rnb_ctr} runs made of {evio_files_ctr} evio files")
print(f"Number of REST files on cache or mss {evio_files_ctr - ctr}")
print(f"Number of REST files missing {ctr}")
print(f"batch{BATCHNB} is {100 * (float(evio_files_ctr) - float(ctr)) / float(evio_files_ctr):.2f} percent done")

d = datetime.datetime.today()
print(d.strftime("job finish: %Y-%m-%d %H:%M:%S\n"))
