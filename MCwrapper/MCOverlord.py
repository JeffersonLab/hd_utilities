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
import MySQLdb
import MySQLdb.cursors
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
dbcursor = dbcnx.cursor(MySQLdb.cursors.DictCursor)


def checkSWIF():
        print "CHECKING SWIF JOBS"
        queryswifjobs="SELECT OutputLocation,ID,NumEvents,Completed_Time FROM Project WHERE ID IN (SELECT Project_ID FROM Jobs WHERE Status_Bits=1 && ID IN (SELECT Job_ID FROM Attempts WHERE BatchSystem= 'SWIF') )"
        dbcursor.execute(queryswifjobs)
        AllWkFlows = dbcursor.fetchall()

        #TOTCompletedEvtsquery="SELECT Project_ID,SUM(NumEvts) FROM Jobs WHERE Status='succeeded' GROUP BY Project_ID;"
        TOTCompletedEvtsquery="SELECT Project_ID,SUM(NumEvts) FROM Jobs WHERE ID IN (SELECT Job_ID FROM Attempts WHERE Status='succeeded')"
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
                        
                        print str(job["id"]) + " | " + job["status"]+ " | "+str(job["attempts"][0]["exitcode"])
                        print job["attempts"][0]

                        updatejobstatus="UPDATE Attempts SET Status=\""+str(job["status"])+"\", ExitCode="+str(job["attempts"][0]["exitcode"]) +", RunningLocation="+"'"+str(job["attempts"][0]["auger_node"])+"'"+" WHERE BatchJobID="+str(job["id"])
                        print updatejobstatus
                        dbcursor.execute(updatejobstatus)
                        dbcnx.commit()
                        #print "---------------------------------"
                #print "=================================="
                #print workflow

                TotalCompletedNum=0
                for proj in TOTCompletedEvt:
                        if str(proj[0]) == str(ProjID):
                                TotalCompletedNum=int(proj[1])
                                break


                if(int(TotalCompletedNum) == workflow[2] and not workflow[3]):
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
                #print wkflow
                getosgjobs="SELECT ID,BatchJobID,NumEvts FROM Jobs WHERE Project_ID="+str(wkflow[0])+";"
                dbcursor.execute(getosgjobs)
                Allwkflow_jobs = dbcursor.fetchall()
                for job in Allwkflow_jobs:
                        #print job
                        statuscommand="condor_q "+str(job[1])+" -json"
                        #print statuscommand
                        jsonOutputstr=subprocess.check_output(statuscommand.split(" "))
                        #print jsonOutputstr
                        if(jsonOutputstr != ""):
                                RETURNEDOBJECT=json.loads(jsonOutputstr)
                                print job[1]+" | "+str(RETURNEDOBJECT[0]["JobStatus"])+" | "+str(RETURNEDOBJECT[0]["ExitCode"])
                                updatejobstatus="UPDATE Jobs SET Status=\""+str(RETURNEDOBJECT[0]["JobStatus"])+"."+str(RETURNEDOBJECT[0]["ExitCode"])+"\" WHERE BatchJobID="+str(job[1])
                                #print updatejobstatus
                                dbcursor.execute(updatejobstatus)
                                dbcnx.commit()

                        else:
                                historystatuscommand="condor_history "+str(job[1])+" -json"
                                #print historystatuscommand
                                Outputstr=subprocess.check_output(historystatuscommand.split(" "))
                                #print Outputstr
                                RETURNEDHISTOBJECT=json.loads(Outputstr)
                                print job[1]+" | "+str(RETURNEDHISTOBJECT[0]["JobStatus"])+" | "+str(RETURNEDHISTOBJECT[0]["ExitCode"])
                                #print RETURNEDHISTOBJECT[0]['JobStartDate']
                                updatejobstatus="UPDATE Jobs SET Status=\""+str(RETURNEDHISTOBJECT[0]["JobStatus"])+"."+str(RETURNEDHISTOBJECT[0]["ExitCode"])+"\" WHERE BatchJobID="+str(job[1])
                                #print updatejobstatus
                                dbcursor.execute(updatejobstatus)
                                dbcnx.commit()
                                
                                

       #subprocess.check_output("swif status -workflow")
########################################################## MAIN ##########################################################
        
def main(argv):

        checkSWIF()
        #checkOSG()
        
        dbcnx.close()
                
if __name__ == "__main__":
   main(sys.argv[1:])
