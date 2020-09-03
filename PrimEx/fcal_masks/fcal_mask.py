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
        

    aaa  = TH1F("TAGH Tagged Flux","TAGH Tagged Flux",300,0.5,299.5)


    block_quality_assignment = ccdb_conn.get_assignment("/FCAL/block_quality",run_number, VARIATION)

    block_quality = block_quality_assignment.constant_set.data_table



    kBeamHoleSize = 3

    blockSize  = 4.0157
    
    radius = 120.471
    
    
    MidBlock = 29
    
    innerRadius = ( int(kBeamHoleSize) - 1 ) / 2. * float(blockSize) * math.sqrt(2.)

    innerRadius *= 1.01


    mod_x = []
    mod_y = []
    
    index = 0


    for row_tmp in range(59):
        for column_tmp in range(59):

            x_block =  (int(column_tmp) - MidBlock ) * float(blockSize)
            y_block =  (int(row_tmp) - MidBlock ) * float(blockSize)
            
            thisRadius = math.sqrt(x_block*x_block + y_block*y_block)
            
            if thisRadius < radius:
                if thisRadius > innerRadius:
                    
                    print(thisRadius)
                    
                    mod_x.append(row_tmp)
                    mod_y.append(column_tmp)
                    
                    index = index + 1
                                    


    print("Dead Channels \n");


    dead_all    =  0
    dead_inner  =  0

    susp_all    =  0 
    susp_inner  =  0


    for ii in range(2800):
                        
        quality = int(block_quality[ii][0])
                        
        if int(block_quality[ii][0]) == 1:
                            
                            
            row_cent     =  mod_x[ii] - 29
            column_cent  =  mod_y[ii] - 29

            module_y = float(row_cent*4.)
            module_x = float(column_cent*4.)


  
            print("Run %d   %d   %d  %d  %d \n" %(run_number,ii, int(block_quality[ii][0]),mod_x[ii],mod_y[ii]))

            print("Row = %d  Column = %d    X = %f    Y = %f \n"%(row_cent,column_cent, module_x, module_y))

            dead_all = dead_all + 1
                
            if abs(module_x) < 40 and abs(module_y) < 40:

                dead_inner = dead_inner + 1;


    print("\n");
    print("\n");
                

    print("Suspicious Channels \n");

    for ii in range(2800):
            
        quality = int(block_quality[ii][0])
            
        if int(block_quality[ii][0]) == 5:
                

            row_cent     =  mod_x[ii] - 29
            column_cent  =  mod_y[ii] - 29

            module_y = float(row_cent*4.)
            module_x = float(column_cent*4.)



            susp_all = susp_all + 1
                
            if abs(module_x) < 40 and abs(module_y) < 40:

                susp_inner = susp_inner + 1;


# Print all bad channels

            print("Run %d   %d   %d  %d  %d \n" %(run_number,ii, int(block_quality[ii][0]),
                                                  mod_x[ii],mod_y[ii]))

            print("Row = %d  Column = %d    X = %f    Y = %f \n"%(row_cent,column_cent, 
                                                                  module_x, module_y))

            
                    


    print("\n");
    print("\n");


    print("Dead Channels: ALL (INNER)   =  %d (%d)     Susp  Channels:  ALL(INNER)    %d (%d)  \n"%(dead_all,dead_inner,susp_all,susp_inner) );
        



#                print("Run %d   %d   %d  %d  %d\n" %(run_number,ii, int(block_quality[ii][0]),row,column))
#                print(block_quality[ii][0])

     

## main function  
if __name__ == "__main__":
    main()
