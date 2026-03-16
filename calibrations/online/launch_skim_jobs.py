#!/usr/bin/env python3
#
# Author: Sean Dobbs (sdobbs@jlab.org)
#

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
import tempfile



# current running conditions
#MAX_JOBS = 8
#NODES = ['gluon152', 'gluon153', 'gluon154', 'gluon155']
#NODES = [ 'gluon153', 'gluon154', 'gluon155']
#NODES = [ 'gluon151', 'gluon156' ]
#NODES = { 'gluon151':2, 'gluon156':2, 'gluon152':1, 'gluon153':1, 'gluon154':1, 'gluon155':1 }

# maximum processing
# MAX_JOBS = 20
# NODES = { 'gluon150':2, 'gluon151':2, 'gluon152':2, 'gluon153':2, 'gluon154':2, 'gluon155':2, 'gluon156':2, 'gluon157':2, 'gluon158':2, 'gluon159':2 }
MAX_JOBS = 18
NODES = { 'gluon150':2, 'gluon151':2, 'gluon152':2, 'gluon153':2, 'gluon154':2, 'gluon155':2, 'gluon156':2, 'gluon157':2, 'gluon158':2  }

DRY_RUN = False

# DB configuration parameters
# make them global so that 
CALIBDB_HOST     = 'hallddb'
CALIBDB_USER     = 'calibInformer'
CALIBDB_NAME     = 'calibInfo'

def ProcessSkims(run,fnum,fpath,host,calibdb_cursor):
    print("In ProcessSkims() ...")
    BASEDIR = "/gluonraid4/data1/online_skimming/run"
    #rundir = tempfile.TemporaryDirectory(dir=BASEDIR).name
    
    # connect to calibration tracking DB
    # note that each process needs its own connection!
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
        return 1
        
    #os.chdir(BASEDIR)
    #with tempfile.TemporaryDirectory() as rundir_obj:
    #with tempfile.TemporaryDirectory(dir=BASEDIR) as rundir:
    #with tempfile.mkdtemp(dir=BASEDIR) as rundir:

    rundir = tempfile.mkdtemp(dir=BASEDIR)
    print("tmpdir = ",rundir)
        
    #cmd = "cp /gluonwork1/Users/sdobbs/calibration_train/online/file_calib_skim.sh %s/"%rundir
    #print(cmd)
    #os.system(cmd)

    
    # Try flagging this early, so that it reduces the number of doubly processed files....
    query = "UPDATE skim_files SET done=1 WHERE run=%d AND file=%d"%(run,fnum)
    calibdb_cursor.execute(query)
    calibdb_cnx.commit()

    
    print("sshing to %s"%host)
    #cmd = "ssh %s 'cd %s; ./file_calib_skim.sh %d %d %s %s'"%(host,rundir,run,fnum,fpath,rundir)
    cmd = "ssh %s 'cd %s; /gluonwork1/Users/sdobbs/calibration_train/online/file_calib_skim.sh %d %d %s %s'>& /gluonwork1/Users/sdobbs/calibration_train/online/skim_logs/skim_log_%d_%d.%s"%(host,rundir,run,fnum,fpath,rundir,run,fnum,host)
    print(cmd)
    if DRY_RUN:
        print(cmd)
    else:
        retval = os.system(cmd)
        if(retval != 0):
            print("ERROR in run %d %s(%d): %d"%(run,cmd,fnum,retval))

        # update skim db
        query = "UPDATE skim_files SET done=1 WHERE run=%d AND file=%d"%(run,fnum)
        calibdb_cursor.execute(query)
        calibdb_cnx.commit()


    # cleanup
    cmd = "rm -r %s"%rundir
    os.system(cmd)
    
    calibdb_cursor.close()
    calibdb_cnx.close()

    return 0
    
def test_func(args=()):
    print("TEST")
    return 0

    
# this part gets executed when this file is run on the command line 
if __name__ == "__main__":

    # Parse command line arguments
    CALIBDB_HOST     = 'hallddb'
    CALIBDB_USER     = 'calibInformer'
    CALIBDB_NAME     = 'calibInfo'

    MIN_RUN = 131593


    # connect to calibration tracking DB
    #try:
    #    calibdb_cnx = mysql.connector.connect(user=CALIBDB_USER, 
    #                                          host=CALIBDB_HOST,
    #                                          database=CALIBDB_NAME)
    #    calibdb_cursor = calibdb_cnx.cursor()
    #except mysql.connector.Error as err:
    #    print(err)
    #    sys.exit(0)


    #compute_threads = []
    compute_thread_pool = multiprocessing.Pool(MAX_JOBS)


    # main loop
    while(1):
        # try reconnecting to the DB every iteration
        # connect to calibration tracking DB
        try:
            calibdb_cnx = mysql.connector.connect(user=CALIBDB_USER, 
                                                  host=CALIBDB_HOST,
                                                  database=CALIBDB_NAME)
            calibdb_cursor = calibdb_cnx.cursor()
        except mysql.connector.Error as err:
            print(err)
            sys.exit(0)

        
        # get the number of currently running jobs
        query = "SELECT COUNT(*) FROM running_jobs"
        calibdb_cursor.execute(query)
        num_jobs = calibdb_cursor.fetchone()[0]
        print("number of running jobs = ",num_jobs)

        if num_jobs<MAX_JOBS:
            query = "select distinct * from skim_files where done=0 and run>=%d limit %d"%(MIN_RUN,2*MAX_JOBS)
            calibdb_cursor.execute(query)
            skim_files_to_process = calibdb_cursor.fetchall()
            print(skim_files_to_process)
            
            query = "select run,file from running_jobs"
            calibdb_cursor.execute(query)
            files_being_processed = calibdb_cursor.fetchall()
            print(files_being_processed)
            
            for run,fnum,filepath,done in skim_files_to_process:
                #print(filepath)
                #print(run,fnum,filepath,done)
                #continue
                # sanity check: if the file does not exist anymore
                #  (e.g. it got flushed off of the volatile part of the gluonraidN)
                #  then just mark it as done
                if(not os.path.exists(os.path.realpath(filepath))):
                    print("skipping %s - not available anymore ..."%filepath)
                    query = "UPDATE skim_files SET done=1 WHERE run=%d AND file=%d"%(run,fnum)
                    calibdb_cursor.execute(query)
                    calibdb_cnx.commit()
                    continue
                # see if we are currently processsing this file or not
                if (run,fnum) in files_being_processed:
                    print("skipping run %d file %d - already being processed"%(run,fnum))
                    continue
                
                # figure where to run the next job
                next_node = None
                for node,max_node_jobs in NODES.items():
                    query = "SELECT COUNT(*) FROM running_jobs where host='%s'"%node
                    #print(query)
                    calibdb_cursor.execute(query)
                    num_result = calibdb_cursor.fetchone()
                    if num_result is None:
                        num_running_jobs = 0
                    else:
                        num_running_jobs = num_result[0]
                    if num_running_jobs<max_node_jobs:
                        next_node = node
                        break
                    
                if next_node == None:
                    break

                print("run skim job for run %d file %d on %s"%(run,fnum,next_node))
                query = 'INSERT INTO running_jobs (run,file,host) VALUES (%d,%d,"%s")'%(run,fnum,next_node)
                calibdb_cursor.execute(query)
                calibdb_cnx.commit()
                result = compute_thread_pool.apply_async(ProcessSkims, args=(run,fnum,filepath,next_node,calibdb_cursor))
                #print(result.get(timeout=1))
                #res = compute_thread_pool.apply_async(test_func, args=())
                
                # check to see if it's time to be done
                num_jobs += 1
                if num_jobs==MAX_JOBS:
                    break
                
        # see if we should stop earlier
        if os.path.exists("force_skim.stop"):
            print("stopping early...")
            break

        # sleep before next iteration
        print("sleeping for 10 minutes...")
        calibdb_cursor.close()
        calibdb_cnx.close()
        
        time.sleep(600)
        print("waking up, going for another round")

    

    # finish and clean up
    #calibdb_cursor.close()
    #calibdb_cnx.close()
    
    tagger_thread_pool.close()
    tagger_thread_pool.join()
