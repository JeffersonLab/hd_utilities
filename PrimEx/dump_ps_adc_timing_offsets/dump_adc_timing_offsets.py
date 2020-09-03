#!/usr/bin/env python

# Plot masked channels in FCAL
# A.S 

import os,sys
import rcdb
from ROOT import TFile,TGraph,TH1F,TF1,gRandom, FILE
from optparse import OptionParser
from array import array
from datetime import datetime
import pprint
import math
import MySQLdb

import ccdb
from ccdb import Directory, TypeTable, Assignment, ConstantSet

def LoadCCDB():
    sqlite_connect_str = "mysql://ccdb_user@hallddb.jlab.org/ccdb"
#    sqlite_connect_str = "sqlite:////home/somov/ccdb.sqlite"
    provider = ccdb.AlchemyProvider()                           # this class has all CCDB manipulation functions
    provider.connect(sqlite_connect_str)                        # use usual connection string to connect to database
    provider.authentication.current_user_name = "somov"   # to have a name in logs

    return provider

def loadCCDBContextList(runPeriod, restVer):
    dbhost = "hallddb.jlab.org"
    dbuser = 'datmon'
    dbpass = ''
    dbname = 'data_monitoring'

    conn=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
    curs=conn.cursor()    

    cmd = "SELECT revision,ccdb_context FROM version_info WHERE run_period=%s AND data_type='recon' AND revision<=%s ORDER BY revision DESC"
    curs.execute(cmd, [runPeriod, restVer])
    rows=curs.fetchall()
    return rows


def main():
    
    VARIATION   =  "default"

    parser = OptionParser(usage = "plot_flux_ccdb.py --begin-run beginRun --end-run endRun")

    parser.add_option("-b","--begin-run", dest="begin_run",
                      help="Starting run for output")

    parser.add_option("-e","--end-run", dest="end_run",
                     help="Ending run for output")
    
    (options, args) = parser.parse_args(sys.argv)

    FIRST_RUN = int(options.begin_run)

    run_number = FIRST_RUN

    OUTPUT_FILE_TAGH = "%d_tagh_ps_acc_cor.txt" %(FIRST_RUN)

    print(FIRST_RUN)



    ccdb_conn = LoadCCDB()
        
    
    timing_offsets_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/fine/adc_timing_offsets",run_number, VARIATION)
    
    timing_offsets = timing_offsets_assignment.constant_set.data_table
    
    
    OUTPUT_FILE = "adc_time_offsets_%d.txt" %(FIRST_RUN)
    
    data_file = open(OUTPUT_FILE,"w+");
    
    for ii in range(145):
        
        data_file.write("%10.5f   %10.5f \n" %(float(timing_offsets[ii][0]),float(timing_offsets[ii][1])))
        
        
        

## main function  
if __name__ == "__main__":
    main()
