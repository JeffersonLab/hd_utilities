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


    
def LoadCCDB():
    #sqlite_connect_str = "mysql://ccdb_user@gluondb1.jlab.org/ccdb"
    sqlite_connect_str = "mysql://ccdb_user@hallddb.jlab.org/ccdb"
    #sqlite_connect_str = "sqlite:////group/halld/www/halldweb/html/dist/ccdb.sqlite"
    provider = ccdb.AlchemyProvider()                        # this class has all CCDB manipulation functions
    provider.connect(sqlite_connect_str)                     # use usual connection string to connect to database
    provider.authentication.current_user_name = "hdsys"  # to have a name in logs

    return provider

def caget_archive(pv, rcdb_conn):

    # get the start time for the run
    rundata = rcdb_conn.get_run(run)    
    if rundata.start_time is None and rundata.end_time is None:
        return

    run_start_time = rundata.start_time
    run_end_time = rundata.end_time
    if run_start_time is None:
        run_start_time = run_end_time
        if run_start_time.minute-5 < 0.:
            run_start_time = datetime.datetime(run_start_time.year,run_start_time.month,run_start_time.day,run_start_time.hour-1,run_start_time.minute-5+60,run_start_time.second)
    if run_end_time is None or run_end_time == run_start_time:
        run_end_time = run_start_time
        if run_end_time.minute+15. > 60.:
            run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour+1,run_end_time.minute-60+15,run_end_time.second)
    if run_end_time == run_start_time:
        run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour,run_end_time.minute+1,run_end_time.second)

    begintime = datetime.datetime.strftime(run_start_time, '%Y-%m-%d %H:%M:%S')
    # if the run has a set end time, then use that, otherwise use the current time
    if run_end_time:
        endtime = datetime.datetime.strftime(run_end_time, '%Y-%m-%d %H:%M:%S')
    else:
        endtime = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')  # current date/time
        
    cmds = []
    cmds.append("myStats")
    cmds.append("-b")
    cmds.append(begintime)
    cmds.append("-e")
    cmds.append(endtime)
    cmds.append("-c")
    cmds.append("IBCAD00CRCUR6")
    cmds.append("-r")
    cmds.append("5:5000")        
    cmds.append("-l")
    cmds.append(pv)
    #print(cmds)
    # execute external command
    p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
    # iterate over output
    n = 0
    for line in p.stdout:
        n += 1
        if n == 1:     # skip header
            continue 
        tokens = line.strip().split()
        if len(tokens) < 3:
            continue
        key = tokens[0]
        value = tokens[2]    ## average value
        if key == pv.encode():
            if tokens[2] == b'N/A':
                return -1
            return tokens[2]  # HACK: return first data point we get

    return -1



def update_beam_currents(rcdb_conn, run):
    # get the start time for the run
    rundata = rcdb_conn.get_run(run)    
    if rundata.start_time is None and rundata.end_time is None:
        return

    run_start_time = rundata.start_time
    run_end_time = rundata.end_time
    if run_start_time is None:
        run_start_time = run_end_time
        if run_start_time.minute-5 < 0.:
            run_start_time = datetime.datetime(run_start_time.year,run_start_time.month,run_start_time.day,run_start_time.hour-1,run_start_time.minute-5+60,run_start_time.second)
    if run_end_time is None or run_end_time == run_start_time:
        run_end_time = run_start_time
        if run_end_time.minute+15. > 60.:
            run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour+1,run_end_time.minute-60+15,run_end_time.second)
    if run_end_time == run_start_time:
        run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour,run_end_time.minute+1,run_end_time.second)

    begintime = datetime.datetime.strftime(run_start_time, '%Y-%m-%d %H:%M:%S')
    # if the run has a set end time, then use that, otherwise use the current time
    if run_end_time:
        endtime = datetime.datetime.strftime(run_end_time, '%Y-%m-%d %H:%M:%S')
    else:
        endtime = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')  # current date/time

    # Beam current - uses the primary BCM, IBCAD00CRCUR6
    conditions = {}
    # We could also use the following: IPM5C11.VAL,IPM5C11A.VAL,IPM5C11B.VAL,IPM5C11C.VAL
    try: 
        # save integrated beam current over the whole run
        # use MYA archive commands to calculate average

        # build myStats command
        cmds = []
        cmds.append("myStats")
        cmds.append("-b")
        cmds.append(begintime)
        cmds.append("-e")
        cmds.append(endtime)
        cmds.append("-l")
        cmds.append("IBCAD00CRCUR6")
        # execute external command
        p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
        # iterate over output
        n = 0
        for line in p.stdout:
            n += 1
            if n == 1:     # skip header
                continue 
            tokens = line.strip().split()
            if len(tokens) < 3:
                continue
            key = tokens[0]
            value = tokens[2]    ## average value
            if key == b'N/A':
                conditions["beam_current"] = -1.
                break
            if key == b"IBCAD00CRCUR6":
                conditions["beam_current"] = float(value)
    except:
        conditions["beam_current"] = -1.

    # now only do this when the current is on
    try: 
        # save integrated beam current over the whole run
        # use MYA archive commands to calculate average
        # build myStats command
        cmds = []
        cmds.append("myStats")
        cmds.append("-b")
        cmds.append(begintime)
        cmds.append("-e")
        cmds.append(endtime)
        cmds.append("-c")
        cmds.append("IBCAD00CRCUR6")
        cmds.append("-r")
        cmds.append("5:5000")        
        cmds.append("-l")
        cmds.append("IBCAD00CRCUR6")
        # execute external command
        p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
        # iterate over output
        n = 0
        for line in p.stdout:
            n += 1
            if n == 1:     # skip header
                continue 
            tokens = line.strip().split()
            if len(tokens) < 3:
                continue
            key = tokens[0]
            value = tokens[2]    ## average value
            if key == b'N/A':
                conditions["beam_on_current"] = -1.
                break
            if key == b"IBCAD00CRCUR6":
                conditions["beam_on_current"] = float(value)
    except:
        conditions["beam_on_current"] = -1.
        
    print("new beam current = %6.3f   beam on current = %6.3f"%(conditions["beam_current"],conditions["beam_on_current"]))

    # Add all the values that we've determined to the RCDB
    rcdb_conn.add_conditions(run, conditions, replace=True)

def update_beam_energy(rcdb_conn, run):
    # get the start time for the run
    rundata = rcdb_conn.get_run(run)    
    if rundata.start_time is None and rundata.end_time is None:
        return

    run_start_time = rundata.start_time
    run_end_time = rundata.end_time
    if run_start_time is None:
        run_start_time = run_end_time
        if run_start_time.minute-5 < 0.:
            run_start_time = datetime.datetime(run_start_time.year,run_start_time.month,run_start_time.day,run_start_time.hour-1,run_start_time.minute-5+60,run_start_time.second)
    if run_end_time is None or run_end_time == run_start_time:
        run_end_time = run_start_time
        if run_end_time.minute+15. > 60.:
            run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour+1,run_end_time.minute-60+15,run_end_time.second)
    if run_end_time == run_start_time:
        run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour,run_end_time.minute+1,run_end_time.second)

    begintime = datetime.datetime.strftime(run_start_time, '%Y-%m-%d %H:%M:%S')
    # if the run has a set end time, then use that, otherwise use the current time
    if run_end_time:
        endtime = datetime.datetime.strftime(run_end_time, '%Y-%m-%d %H:%M:%S')
    else:
        endtime = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')  # current date/time

    # Beam current - uses the primary BCM, IBCAD00CRCUR6
    conditions = {}
    try: 
        # save integrated beam current over the whole run
        # use MYA archive commands to calculate average
        # build myStats command
        cmds = []
        cmds.append("myStats")
        cmds.append("-b")
        cmds.append(begintime)
        cmds.append("-e")
        cmds.append(endtime)
        cmds.append("-c")
        cmds.append("IBCAD00CRCUR6")
        cmds.append("-r")
        cmds.append("5:5000")        
        cmds.append("-l")
        cmds.append("HALLD:p")
        # execute external command
        p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
        # iterate over output
        n = 0
        for line in p.stdout:
            n += 1
            if n == 1:     # skip header
                continue 
            tokens = line.strip().split()
            if len(tokens) < 3:
                continue
            key = tokens[0]
            value = tokens[2]    ## average value
            if key == b'N/A':
                conditions["beam_energy"] = -1.
                break
            if key == b"HALLD:p":
                conditions["beam_energy"] = float(value)
    except:
        conditions["beam_energy"] = -1.

    print("new beam energy = %6.2f"%conditions["beam_energy"])
        
    # Add all the values that we've determined to the RCDB
    rcdb_conn.add_conditions(run, conditions, replace=True)

def update_old_cdc_pressure(rcdb_conn, run):
    # get the start time for the run
    rundata = rcdb_conn.get_run(run)    
    if rundata.start_time is None and rundata.end_time is None:
        return

    run_start_time = rundata.start_time
    run_end_time = rundata.end_time
    if run_start_time is None:
        run_start_time = run_end_time
        if run_start_time.minute-5 < 0.:
            run_start_time = datetime.datetime(run_start_time.year,run_start_time.month,run_start_time.day,run_start_time.hour-1,run_start_time.minute-5+60,run_start_time.second)
    if run_end_time is None or run_end_time == run_start_time:
        run_end_time = run_start_time
        if run_end_time.minute+15. > 60.:
            run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour+1,run_end_time.minute-60+15,run_end_time.second)
    if run_end_time == run_start_time:
        run_end_time = datetime.datetime(run_end_time.year,run_end_time.month,run_end_time.day,run_end_time.hour,run_end_time.minute+1,run_end_time.second)

    begintime = datetime.datetime.strftime(run_start_time, '%Y-%m-%d %H:%M:%S')
    # if the run has a set end time, then use that, otherwise use the current time
    if run_end_time:
        endtime = datetime.datetime.strftime(run_end_time, '%Y-%m-%d %H:%M:%S')
    else:
        endtime = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S')  # current date/time

    # Beam current - uses the primary BCM, IBCAD00CRCUR6
    conditions = {}
    try: 
        # save integrated beam current over the whole run
        # use MYA archive commands to calculate average
        # build myStats command
        cmds = []
        cmds.append("myStats")
        cmds.append("-b")
        cmds.append(begintime)
        cmds.append("-e")
        cmds.append(endtime)
        cmds.append("-c")
        cmds.append("IBCAD00CRCUR6")
        cmds.append("-r")
        cmds.append("5:5000")        
        cmds.append("-l")
        cmds.append("RESET:i:GasPanelBarPress1")
        # execute external command
        p = subprocess.Popen(cmds, stdout=subprocess.PIPE)
        # iterate over output
        n = 0
        for line in p.stdout:
            n += 1
            if n == 1:     # skip header
                continue 
            tokens = line.strip().split()
            if len(tokens) < 3:
                continue
            key = tokens[0]
            value = tokens[2]    ## average value
            if key == b'N/A':
                conditions["cdc_gas_pressure"] = -1.
                break
            if key == b"RESET:i:GasPanelBarPress1":
                conditions["cdc_gas_pressure"] = float(value)
    except:
        conditions["cdc_gas_pressure"] = -1.

    print("new CDC pressure = %6.2f"%conditions["cdc_gas_pressure"])
        
    # Add all the values that we've determined to the RCDB
    rcdb_conn.add_conditions(run, conditions, replace=True)

def update_cdc_parms(rcdb_conn, ccdb_conn, run):
    TOLERANCE = 1.e-5  # constant used for comparisons

    # get info from EPICS
    cdc_pressure = float(caget_archive("SOL:i:BarPress2", rcdb_conn))

    # check ranges to make sure things are not crazy
    if(cdc_pressure < 50. or cdc_pressure > 150.):
        print("bad CDC pressure = %6.3f"%cdc_pressure)
        return

    temp1 = float(caget_archive("GAS:i::CDC_Temps-CDC_D1_Temp", rcdb_conn))
    temp2 = float(caget_archive("GAS:i::CDC_Temps-CDC_D3_Temp", rcdb_conn))
    temp3 = float(caget_archive("GAS:i::CDC_Temps-CDC_D4_Temp", rcdb_conn))
    temp4 = float(caget_archive("GAS:i::CDC_Temps-CDC_D5_Temp", rcdb_conn))
    
    #calp = 1.064*cdc_pressure - 5.098    # convert Hall D gas panel reading into kPa following https://logbooks.jlab.org/entry/3810976
    t = 273.15 + 0.25*(temp1 + temp2 + temp3 + temp4);    # convert from C to K 
  
    d = cdc_pressure/t   # kPa/K
    #d = calp/t   # kPa/K
    print("new CDC P/T = %6.2f"%d)

    # save ratio in RCDB
    conditions = {}
    conditions["cdc_povert"] = d
    rcdb_conn.add_conditions(run, conditions, replace=True)
    
    
    a1 = 2.0157 + -2.9468 *d;
    b1 = -1.1960 + 3.1979 *d;
    c1 = 0.6225 + -1.8051 *d;
    a2 = -2.3818 + 6.8484 *d;
    b2 = 3.1014 + -10.5439 *d;
    c2 = -2.4503 + 7.9724 *d;
    
    cdc_ttod_table = []
    cdc_ttod_table.append(["%.6f"%a1,"%.6f"%a2,0,"%.6f"%b1,"%.6f"%b2,0,"%.6f"%c1,"%.6f"%c2,0,1.1,-0.08])
    cdc_ttod_table.append(["%.6f"%a1,"%.6f"%(-a2),0,"%.6f"%b1,"%.6f"%(-b2),0,"%.6f"%c1,"%.6f"%(-c2),0,1.1,-0.08])

    
    # update CCDB for this run
    ccdb_conn.create_assignment(
        data=cdc_ttod_table,
        path="/CDC/drift_parameters",
        variation_name="default",
        min_run=run,
        max_run=run,
        comment="Online update based on EPICS")


    
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
    RCDB_PRODUCTION_SEARCH = "event_count > 100000"
    #RCDB_PRODUCTION_SEARCH = "@is_dirc_production"
    #RCDB_PRODUCTION_SEARCH = "@is_dirc_production and event_count>100000000"
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
    RCDB_SEARCH_MIN  = 130405
    RCDB_SEARCH_MAX  = 140000
    FORCE = False


    parser = OptionParser(usage = "update_rcdb.py")
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

    for run in RCDB_RUNS:
        print("Processing Run %d"%run)
        if not run in RCDB_PRODUCTION_RUNS:
            print("Not a good run, skipping...")
            continue

        rundata = rcdb_conn.get_run(run)    
        if rundata.end_time is None:
            print("Run still in progress, skipping...")
            continue
        if rundata.start_time is None:
            continue
        begintime = datetime.datetime.strftime(rundata.start_time, '%Y-%m-%d %H:%M:%S')
        if rundata.end_time:
            endtime = datetime.datetime.strftime(rundata.end_time, '%Y-%m-%d %H:%M:%S')

        # make sure run is in calibration tracking DB
        query = "SELECT run FROM online_info WHERE run='%s'"%run 
        calibdb_cursor.execute(query)
        run_info = calibdb_cursor.fetchone()
        if run_info is None or run_info[0] is None:
            query = "INSERT INTO online_info (run,done,rcdb_update,launched_skim) VALUES (%s,FALSE,FALSE,FALSE)"%run
            calibdb_cursor.execute(query)
            calibdb_cnx.commit()

        # now try to update things
        query = "SELECT rcdb_update FROM online_info WHERE run='%s'"%run 
        calibdb_cursor.execute(query)
        run_info = calibdb_cursor.fetchone()
        if run_info is None or run_info[0] is None:
            print("Problem accessing DB (rcdb_update), skipping run...")
        else:
            if FORCE or run_info[0]==0:
                ccdb_conn = LoadCCDB()

                # update some RCDB settings
                update_beam_currents(rcdb_conn, run)
                update_beam_energy(rcdb_conn, run)
                update_old_cdc_pressure(rcdb_conn, run)
                update_cdc_parms(rcdb_conn, ccdb_conn, run)

                rcdb_conn.add_condition(run, "electron_polarization_status", -1, True)

                # update tracking DB settings
                query = "UPDATE online_info SET rcdb_update=TRUE WHERE run='%s'"%run
                print(query)
                calibdb_cursor.execute(query)
                calibdb_cnx.commit()
            else:
                print("Already updated RCDB")
