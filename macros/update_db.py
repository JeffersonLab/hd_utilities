#!/usr/bin/env python
import MySQLdb
import os
import datetime

# extract info from context
try:
    run = os.environ['RUN']
except:
    print "Could not find run number!  Exiting..."
    #os.exit(0)
    run = 30480

try:
    bms_osname = os.environ['BMS_OSNAME']
except:
    bms_osname = ""

try:
    #datestr = os.environ['B1Pi_TEST_DIR'].split('/')[-2]
    datestr = os.getwd().split('/')[-2]
except:
    now = datetime.datetime.now()
    datestr = now.strftime('%Y-%m-%d')

# read in values
vals = []
with open("data.txt") as f:
    for line in f:
        vals.append(line.strip())

# do database stuff
db_conn = MySQLdb.connect("hallddb.jlab.org", "datmon", "", "data_monitoring")
db = db_conn.cursor()

# put something into the display configuration for testing
config = '{ "recon_pi0": {"lowLimit": 990, "highLimit": 1020, "bounding": "typical_hi"} }'

db.execute("INSERT INTO b1pi_test_data (platform,date_generated,run,recon_pi0,recon_omega,recon_b1,recon_b1pi,gen_photons,gen_protons,gen_pip,gen_pim,recon_photons,recon_protons,recon_pip,recon_pim,config,comment)  VALUES ('%s','%s',%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,\"%s\",\"%s\")"%(bms_osname,datestr,run,vals[0],vals[1],vals[2],vals[3],vals[4],vals[5],vals[6],vals[7],vals[8],vals[9],vals[10],vals[11],config,""))

db_conn.commit()
db_conn.close()
