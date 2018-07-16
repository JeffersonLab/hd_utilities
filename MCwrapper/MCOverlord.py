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

def checkSWIF():
        print "CHECKING SWIF JOBS"
        queryswifjobs="SELECT OutputLocation,ID,NumEvents,Completed_Time FROM Project WHERE Is_Dispatched='SWIF'"
        dbcursor.execute(queryswifjobs)
        AllWkFlows = dbcursor.fetchall()

        TOTCompletedEvtsquery="SELECT Project_ID,SUM(NumEvts) FROM Jobs WHERE Status='succeeded' GROUP BY Project_ID;"
        dbcursor.execute(TOTCompletedEvtsquery)
        TOTCompletedEvt=dbcursor.fetchall()
        index=-1
        for workflow in AllWkFlows:
                index+=1
                ProjID=workflow[1]
                splitnames=workflow[0].split("/")
                wkflowname=splitnames[len(splitnames)-2]
                statuscommand="swif status -workflow "+str(wkflowname)+" -jobs -display json"
                jsonOutputstr=subprocess.check_output(statuscommand.split(" "))
                RETURNEDOBJECT=json.loads(jsonOutputstr)
                
                for job in RETURNEDOBJECT["jobs"]:

                        print str(job["id"]) + " | " + job["status"]

                        updatejobstatus="UPDATE Jobs SET Status=\""+str(job["status"])+"\" WHERE BatchJobID="+str(job["id"])
                        #print updatejobstatus
                        dbcursor.execute(updatejobstatus)
                        dbcnx.commit()
                        #print "---------------------------------"
                #print "=================================="
               
                if(int(TOTCompletedEvt[index][1]) == workflow[2] and not workflow[3]):
                        print "COMPLETE"
                        updateProjectstatus="UPDATE Project SET Completed_Time=NOW() WHERE ID="+str(ProjID)+"&& Completed_Time IS NULL;"
                        #print updatejobstatus
                        dbcursor.execute(updateProjectstatus)
                        dbcnx.commit() 
        
def checkOSG():
        print "CHECKING OSG JOBS"
        queryosgjobs="SELECT ID,NumEvents,Completed_Time FROM Project WHERE Is_Dispatched='OSG'"
        dbcursor.execute(queryosgjobs)
        AllWkFlows = dbcursor.fetchall()

        for wkflow in AllWkFlows:
                getosgjobs="SELECT ID,BatchJobID,NumEvts, FROM Jobs WHERE Project_ID="+str(wkflow[0])
                dbcursor.execute(getosgjobs)
                Allwkflow_jobs = dbcursor.fetchall()
                for job in Allwkflow_jobs:
                        statuscommand=" condor_q "+str(job[1])+" -json"
                        jsonOutputstr=subprocess.check_output(statuscommand.split(" "))
                        RETURNEDOBJECT=json.loads(jsonOutputstr)
                        print job[1]+" | "+RETURNEDOBJECT["JobStatus"]

       #subprocess.check_output("swif status -workflow")
########################################################## MAIN ##########################################################
        
def main(argv):

        #checkSWIF()
        checkOSG()
        
        dbcnx.close()
                
if __name__ == "__main__":
   main(sys.argv[1:])
