#!/usr/bin/env python3

import datetime
import os
import subprocess
import sys


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

# mysql.connector not available via system and must come via PYTHONPATH
if not os.getenv("PYTHONPATH"):
  sys.path.append("/group/halld/Software/builds/Linux_CentOS7-x86_64-gcc4.8.5/ccdb/ccdb_1.06.06/python")
import mysql.connector
sys.path.append('/group/halld/Software/builds/Linux_CentOS7-x86_64-gcc4.8.5/rcdb/rcdb_0.06.00/python')
import rcdb

RCDB_HOST = "hallddb.jlab.org"
RCDB_USER = "rcdb"
RCDB_QUERY = "@is_dirc_production and @status_approved"
db = rcdb.RCDBProvider("mysql://" + RCDB_USER + "@" + RCDB_HOST + "/rcdb")

if TYPE == "reprocessing":
  if testing == "production":
    cmd = ("mkdir -p %s.recon_2019-11_ver01_catchup_batch%s/launch" % (DATEDIR, BATCHNB))
    os.system(cmd)
    cmd = "cp *.* %s.recon_2019-11_ver01_catchup_batch%s/launch/"
    os.system(cmd)
    cmd = ("scp -r %s.recon_2019-11_ver01_catchup_batch%s bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/" % (DATEDIR, BATCHNB))
    os.system(cmd)
    # cmd = ('scp jana_recon_nersc_%s.config bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/%s.recon_2019-11_ver01_catchup_batch%s/launch/'%(BATCHNB, DATEDIR, BATCHNB))
    # os.system(cmd)
  if testing == "testing":
    cmd = ("mkdir -p %s.recon_2019-11_ver01_catchup_batch%s/launch" % (DATEDIR, BATCHNB))
    print(cmd)
    cmd = "cp *.* %s.recon_2019-11_ver01_catchup_batch%s/launch/"
    print(cmd)
    cmd = ("scp -r %s.recon_2019-11_ver01_catchup_batch%s bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/" % (DATEDIR, BATCHNB))
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
        cache_evio_file = "/cache/halld/RunPeriod-2019-11/recon/ver01/REST/0" + str(rnb) + "/dana_rest_0" + str(rnb) + "_00" + str(evio_index) + ".hddm"
        mss_evio_file = "/mss/halld/RunPeriod-2019-11/recon/ver01/REST/0" + str(rnb) + "/dana_rest_0" + str(rnb) + "_00" + str(evio_index) + ".hddm"
        evio_file = str(rnb) + "_00" + str(evio_index)
      if 9 < evio_index and evio_index < 100:
        cache_evio_file = "/cache/halld/RunPeriod-2019-11/recon/ver01/REST/0" + str(rnb) + "/dana_rest_0" + str(rnb) + "_0" + str(evio_index) + ".hddm"
        mss_evio_file = "/mss/halld/RunPeriod-2019-11/recon/ver01/REST/0" + str(rnb) + "/dana_rest_0" + str(rnb) + "_0" + str(evio_index) + ".hddm"
        evio_file = str(rnb) + "_0" + str(evio_index)
      if 99 < evio_index and evio_index < 999:
        cache_evio_file = "/cache/halld/RunPeriod-2019-11/recon/ver01/REST/0" + str(rnb) + "/dana_rest_0" + str(rnb) + "_" + str(evio_index) + ".hddm"
        mss_evio_file = "/mss/halld/RunPeriod-2019-11/recon/ver01/REST/0" + str(rnb) + "/dana_rest_0" + str(rnb) + "_" + str(evio_index) + ".hddm"
        evio_file = str(rnb) + "_" + str(evio_index)

            py_name = "launch_iu_" + evio_file + ".py"

      is_mssfile = os.path.isfile(mss_evio_file)
      is_cachefile = os.path.isfile(cache_evio_file)

      if os.path.isfile(mss_evio_file) or os.path.isfile(cache_evio_file):
        true_bit = 1
      else:
        if TYPE == "reprocessing":
          subprocess.call(["sed 's,RUNNB,'%s',g; s,FILENB,'%s',g; s,BATCHNB,'%s',g; s,DATEDIR,'%s',g' dummy.py > %s" % (rnb, evio_index, BATCHNB, DATEDIR, py_name)], shell=True)
          cmd = ("chmod +x %s" % (py_name))
          os.system(cmd)
          cmd = ("scp %s bigred3.uits.iu.edu:/N/u/hpg/BigRed3/gluex/work/%s.recon_2019-11_ver01_catchup_batch%s/launch/" % (py_name, DATEDIR, BATCHNB))
          if testing == "production":
            os.system(cmd)
          if testing == "testing":
            print(cmd)
          cmd = ('./%s' % (py_name))
          if testing == "production":
            os.system(cmd)
          if testing == "testing":
            print(cmd)
          print("%s" % (evio_file))
          ctr = ctr + 1
        if TYPE == "counting":
          print("%s" % (evio_file))
          ctr = ctr + 1

print("batch%s has %s runs made of %s evio files" % (BATCHNB, rnb_ctr, evio_files_ctr))
print("Number of REST files on cache or mss %s" % (evio_files_ctr - ctr))
print("Number of REST files missing %s" % (ctr))
print("batch%s is %.2f percent done" % (BATCHNB, 100 * (float(evio_files_ctr) - float(ctr)) / float(evio_files_ctr)))

d = datetime.datetime.today()
print(d.strftime("job finish: %Y-%m-%d %H:%M:%S\n"))
