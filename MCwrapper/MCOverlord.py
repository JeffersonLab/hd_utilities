#!/usr/bin/env python
##########################################################################################################################
#
# 2017/03 Thomas Britton
#
#   Options:
#      MC variation can be changed by supplying "variation=xxxxx" option otherwise default: mc
#      the number of events to be generated per file (except for any remainder) can be set by "per_file=xxxx" default: 1000
#
#      If the user does not want genr8, geant, smearing, reconstruction to be performed the sequence will be terminated at the first instance of genr8=0,geant=0,mcsmear=0,recon=0 default: all on
#      Similarly, if the user wishes to retain the files created by any step you can supply the cleangenr8=0, cleangeant=0, cleanmcsmear=0, or cleanrecon=0 options.  By default all but the reconstruction files #      are cleaned. 
#
#      The reconstruction step is multi-threaded, for this step, if enabled, the script will use 4 threads.  This threading can be changed with the "numthreads=xxx" option 
#
#      By default the job will run interactively in the local directory.  If the user wishes to submit the jobs to swif the option "swif=1" must be supplied.
#
# SWIF DOCUMENTATION:
# https://scicomp.jlab.org/docs/swif
# https://scicomp.jlab.org/docs/swif-cli
# https://scicomp.jlab.org/help/swif/add-job.txt #consider phase!
#
##########################################################################################################################
from os import environ
from optparse import OptionParser
import os.path
import mysql.connector
import time
import os
import getpass
import sys
import re
import subprocess
from subprocess import call
import glob
import json

dbcnx = mysql.connector.connect(user='mcuser', database='gluex_mc', host='hallddb.jlab.org')
dbcursor = dbcnx.cursor()


########################################################## MAIN ##########################################################
        
def main(argv):
        queryswifjobs="SELECT OutputLocation FROM Project WHERE Is_Dispatched='SWIF'"
        dbcursor.execute(queryswifjobs)
        AllWkFlows = dbcursor.fetchall()
        sum=0
        for workflow in AllWkFlows:
                splitnames=workflow[0].split("/")
                wkflowname=splitnames[len(splitnames)-2]
                statuscommand="swif status -workflow "+str(wkflowname)+" -jobs -display json"
                jsonOutputstr=subprocess.check_output(statuscommand.split(" "))
                RETURNEDOBJECT=json.loads(jsonOutputstr)
                #print json.dumps(RETURNEDOBJECT,indent=4,sort_keys=True)
                sum=0
                alldone=True
                ProjID=-1
                for job in RETURNEDOBJECT["jobs"]:
                        numevtquery="SELECT Project_ID, NumEvts FROM Jobs WHERE BatchJobID="+str(job["id"])+";"
                        
                        dbcursor.execute(numevtquery)
                        Thejob=dbcursor.fetchall()
                        if Thejob != []:
                                print Thejob
                                sum+= Thejob[0][1]
                                ProjID=Thejob[0][0]
                        

                        print str(job["id"]) + " | " + job["status"]
                        if(job["status"] != "succeeded"):
                                alldone=False

                        updatejobstatus="UPDATE Jobs SET Status=\""+str(job["status"])+"\" WHERE BatchJobID="+str(job["id"])
                        #print updatejobstatus
                        dbcursor.execute(updatejobstatus)
                        dbcnx.commit()
                        #print "---------------------------------"
                print "=================================="
                totevtquery="SELECT NumEvents,Completed_Time FROM Project WHERE ID="+str(ProjID)+";"
                dbcursor.execute(totevtquery)
                TOTALEVTS=dbcursor.fetchall()
                UNMON=False
                TOTCOMP=False
                if TOTALEVTS != [] :

                        if(alldone and sum == TOTALEVTS[0][0] and not TOTALEVTS[0][1]):
                                print "COMPLETED"
                                TOTCOMP=True
                        else:
                                print str(alldone)+" "+str(sum)+" "+str(TOTALEVTS[0][0])
                else:
                        print "UNMONITORED"
                        UNMON=True

                if (UNMON or TOTCOMP):
                        updateProjectstatus="UPDATE Project SET Completed_Time=NOW() WHERE ID="+str(ProjID)+"&& Completed_Time IS NULL;"
                        #print updatejobstatus
                        dbcursor.execute(updateProjectstatus)
                        dbcnx.commit() 
                print "=================================="
       #subprocess.check_output("swif status -workflow")

        
        dbcnx.close()
                
if __name__ == "__main__":
   main(sys.argv[1:])
