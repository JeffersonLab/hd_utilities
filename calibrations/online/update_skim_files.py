#!/usr/bin/env python3
#
# Author: Sean Dobbs (sdobbs@jlab.org)
#

#MariaDB [calibInfo]> describe skim_files;
#+----------+---------------+------+-----+---------+-------+
#| Field    | Type          | Null | Key | Default | Extra |
#+----------+---------------+------+-----+---------+-------+
#| run      | int(11)       | YES  |     | NULL    |       |
#| file     | int(11)       | YES  |     | NULL    |       |
#| filepath | varchar(1024) | YES  |     | NULL    |       |
#| done     | int(11)       | YES  |     | NULL    |       |
#+----------+---------------+------+-----+---------+-------+


import mysql.connector
from mysql.connector import errorcode
import sqlite3 as lite
import sys
import os
import re
import datetime
import time
import subprocess
import multiprocessing
import glob
from optparse import OptionParser
import rcdb
import ccdb
from epics import caget,caput
import glob

    

def GetHOSSFilepath(cursor, run, filenum):
    query = "select host,destination from HOSS_transfers where run=%d and file=%d and destination like '%%rawdata/volatile%%'"%(run,filenum)
    print(query)
    #"SELECT run FROM online_info WHERE run='%s'"%run 
    cursor.execute(query)
    file_info = cursor.fetchone()
    print(file_info)
    #if file_info is None or file_info[0] is None:
    #    query = "select host,destination from HOSS_transfers where run=%d and file=%d and destination like '%%rawdata/RunPeriod-2025-01/volatile%%'"%(run,filenum)
    #    cursor.execute(query)
    #    file_info = cursor.fetchone()
    #    print(file_info)
    if file_info is None or file_info[0] is None:
        return None
    return '/'+file_info[0]+file_info[1] 


def GetHOSSFilepathRun(cursor, run):
    query = "select file,host,destination from HOSS_transfers where run=%d and destination like '%%rawdata/volatile%%'"%(run)
    print(query)
    cursor.execute(query)

    data = [] 
    
    #print("FOR RUN ",run)
    for fnum,host,destination in cursor:
        #

        #print(fnum,host,destination)
        data.append([fnum,'/'+host+destination])

    #if file_info is None or file_info[0] is None:
    #    return None
    #return '/'+file_info[0]+file_info[1] 

    return data


def GetDiskFilepathRun(run):
    datapath = "/gluex/data/rawdata/all/Run%d"%run
    data = []

    for fpath in glob.glob(datapath+'/*.evio'):
        fname = fpath.split('/')[-1]
        tokens = fname.split('_')
        run = tokens[-2]
        fnum = tokens[-1][:3]
        #print(run,fnum)
        if(int(fnum)<20):
            if(os.path.exists(os.path.realpath(fpath))):
                data.append([fnum,os.path.realpath(fpath)])
    
    #print(data)
    return data

    
    

def GetRunSkimFiles(cursor, run):
    query = "select file,filepath from skim_files where run=%d"%run
    cursor.execute(query)

    data = []

    for fnum,filepath in cursor:
        data.append([fnum,filepath])

    return data
    
def GetRunSkimFileNums(cursor, run):
    query = "select file from skim_files where run=%d"%run
    cursor.execute(query)

    data = []

    for fnum in cursor:
        data.append(fnum[0])

    return data
    


    
# this part gets executed when this file is run on the command line 
if __name__ == "__main__":

    # Parse command line arguments
    CALIBDB_HOST     = 'hallddb'
    CALIBDB_USER     = 'calibInformer'
    CALIBDB_NAME     = 'calibInfo'
    RCDB_FILENAME    = ''
    #RCDB_HOST        = 'hallddb'
    RCDB_HOST        = 'gluondb1'
    RCDB_USER        = 'rcdb'
    RCDB_PASS        = 'GlueX_2come'
    RCDB_DB          = 'rcdb2'
    HOSS_HOST        = 'gluondb1'
    HOSS_USER        = 'hoss'
    HOSS_DB          = 'HOSS'
    #RCDB_PASS        = ''
    RUNS             = ''
    RUN_PERIOD       = 'RunPeriod-2025-01'
    #RCDB_PRODUCTION_SEARCH = "@is_dirc_production"
    RCDB_PRODUCTION_SEARCH = "@is_dirc_production and event_count>100000000"
    #RCDB_PRODUCTION_SEARCH = "@is_primex_production"
    #RCDB_PRODUCTION_SEARCH = "@is_primex_production and event_count>10000000"
    #RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_DIRC' and event_count > 5000000"
    #RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_DIRC' and event_count > 5000000 and solenoid_current > 100 and collimator_diameter != 'Blocking'"
    #RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_DIRC' and beam_current > 10. and event_count > 5000000 and solenoid_current > 100 and collimator_diameter != 'Blocking'"
    #RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_CPP' and beam_current > 10. and event_count > 5000000 and solenoid_current > 100 and collimator_diameter != 'Blocking'"
    #RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_SRC' and beam_current > 10. and event_count > 5000000 and solenoid_current > 100 and collimator_diameter != 'Blocking'"
    #RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_DIRC_TRD' and beam_current > 10. and event_count > 5000000 and solenoid_current > 100 and collimator_diameter != 'Blocking'"
    #3RCDB_PRODUCTION_SEARCH = "daq_run=='PHYSICS_DIRC_TRD' and event_count > 5000000"
    #RCDB_SEARCH_MIN  = 40000
    #RCDB_SEARCH_MIN  = 41857
    RCDB_SEARCH_MIN  = 131590
    RCDB_SEARCH_MAX  = 140000
    FORCE = False


    parser = OptionParser(usage = "update_skim_files.py ")
    parser.add_option("-f","--force", dest="force", action="store_true",
                      help="Process all runs")
    (options, args) = parser.parse_args(sys.argv)
    if options.force:
        FORCE = True

    if(len(args) > 1):
        RUNS=args[1]

    # Connect to RCDB
    try:
        if RCDB_FILENAME=='':
            # MySQL RCDB server
            RCDB = 'mysql://' + RCDB_USER + ':' + RCDB_PASS + '@' + RCDB_HOST + '/' + RCDB_DB
            cnx = mysql.connector.connect(user=RCDB_USER, password=RCDB_PASS, host=RCDB_HOST, database=RCDB_DB)
            #cnx = mysql.connector.connect(user=RCDB_USER, host=RCDB_HOST, database='rcdb')
            cur = cnx.cursor()  
            #cur = cnx.cursor(dictionary=True)  # make dictionary style cursor
        else:
            # SQLite RCDB file
            RCDB = RCDB_FILENAME
            con = lite.connect(RCDB_FILENAME)
            con.row_factory = lite.Row  # make next cursor dictionary style
            cur = con.cursor()
    except Exception as e:
        print('Error connectiong to RCDB: ' + RCDB)
        print(str(e))
        sys.exit(-1)

    # Get run range to process
    if '-' in RUNS:
        pos = RUNS.find('-')
        if len(RUNS[0:pos]) > 0:
            RUN_MIN = int(RUNS[0:pos])
        else:
            RUN_MIN = RCDB_SEARCH_MIN
        if len(RUNS[pos+1:]) > 0:
            RUN_MAX = int(RUNS[pos+1:])
        else:
            RUN_MAX = RCDB_SEARCH_MAX
    elif RUNS!='':
        RUN_MIN = RUNS
        RUN_MAX = RUNS
    else:
        # No run range given. Find it for last day
        sql = 'SELECT min(number) AS RUN_MIN,max(number) AS RUN_MAX FROM runs WHERE '
        if RCDB.startswith('mysql') :
            sql += 'UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(finished) <24*3600'
        else:
            sql += 'strftime("%s","now")-strftime("%s",finished) <24*3600'
        cur.execute(sql)
        c_rows = cur.fetchall()
        if len(c_rows)==0 :
            print('No runs specified and unable to find any in DB for last 24 hours')
            sys.exit(0)
        RUN_MIN = c_rows[0][0]
        RUN_MAX = c_rows[0][1]
        print('No run range specified. Processing runs completed in last 24 hours: ' + str(RUN_MIN) + '-' + str(RUN_MAX))

    print("%d %d"%(RUN_MIN,RUN_MAX))

    # select list of runs from RCDB
    RCDB_RUNS = []
    sql = 'SELECT number FROM runs WHERE number >= %d AND number <= %d'%(RUN_MIN,RUN_MAX)
    cur.execute(sql)
    c_rows = cur.fetchall()
    if len(c_rows)==0 :
        print('No runs found in the specified run range')
        sys.exit(0)

    RCDB_RUNS = [ c_rows[row][0] for row in range(len(c_rows)) ]

    # pull list of production runs
    rcdb_conn = rcdb.RCDBProvider(RCDB)
    runs = rcdb_conn.select_runs(RCDB_PRODUCTION_SEARCH, RCDB_SEARCH_MIN, RCDB_SEARCH_MAX)
    RCDB_PRODUCTION_RUNS = [ run.number for run in runs ]

    # connect to calibration tracking DB
    try:
        calibdb_cnx = mysql.connector.connect(user=CALIBDB_USER, 
                                              host=CALIBDB_HOST,
                                              database=CALIBDB_NAME)
        calibdb_cursor = calibdb_cnx.cursor()
    except mysql.connector.Error as err:
        #if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
        #    print("Something is wrong with your user name or password")
        #elif err.errno == errorcode.ER_BAD_DB_ERROR:
        #    print("Database does not exist")
        #else:
        print(err)
        sys.exit(0)

    # connect to HOSS DB
    try:
        hoss_cnx = mysql.connector.connect(user=HOSS_USER, 
                                              host=HOSS_HOST,
                                              database=HOSS_DB)
        hoss_cursor = hoss_cnx.cursor()
    except mysql.connector.Error as err:
        print(err)
        sys.exit(0)


        
    for run in RCDB_RUNS:
        print("Processing Run %d"%run)
        if not run in RCDB_PRODUCTION_RUNS:
            print("Not a production run, skipping...")
            continue

        # make sure run is in calibration tracking DB
        query = "SELECT run FROM online_info WHERE run='%s'"%run 
        calibdb_cursor.execute(query)
        run_info = calibdb_cursor.fetchone()
        if run_info is None or run_info[0] is None:
            query = "INSERT INTO online_info (run,done,rcdb_update,launched_skim) VALUES (%s,FALSE,FALSE,FALSE)"%run
            calibdb_cursor.execute(query)
            calibdb_cnx.commit()

        # now try to update things
        query = "SELECT launched_skim FROM online_info WHERE run='%s'"%run 
        calibdb_cursor.execute(query)
        run_info = calibdb_cursor.fetchone()
        if run_info is None or run_info[0] is None:
            print("Problem accessing DB (launched_skim), skipping run...")
        else:
            if FORCE or run_info[0]==0:
                #hoss_files = GetHOSSFilepathRun(hoss_cursor, run)
                hoss_files = GetDiskFilepathRun(run)
                #print(hoss_files)
                if len(hoss_files)==0:
                    continue
                
                skim_file_nums = GetRunSkimFileNums(calibdb_cursor, run)
                #print(skim_file_nums)
                #continue
                
                new_data = []
                for fnum,fpath in hoss_files:
                    # only process the first 20 files, for now
                    #if(fnum>20):
                    #
                    #continue
                    
                    if fnum not in skim_file_nums:
                        new_data.append( (run,fnum,fpath,0) )

                if len(new_data) > 0:
                    query = "insert into skim_files (run,file,filepath,done) values (%s, %s, %s, %s)"
                    #print(query)
                    #print(new_data)
                    calibdb_cursor.executemany(query, new_data)
                    calibdb_cnx.commit() 
                
                
                    # update tracking DB settings
                    query = "UPDATE online_info SET launched_skim=TRUE WHERE run='%s'"%run
                    print(query)
                    calibdb_cursor.execute(query)
                    calibdb_cnx.commit()
            else:
                print("Already updated RCDB")
