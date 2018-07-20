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
#import MySQLdb.cursors
from os import environ
from optparse import OptionParser
import os.path
#import mysql.connector
import time
import os
import getpass
import sys
import re
import subprocess
from subprocess import call
import glob
import json
import time
from datetime import timedelta
from datetime import datetime

dbhost = "hallddb.jlab.org"
dbuser = 'mcuser'
dbpass = ''
dbname = 'gluex_mc'

dbcnx=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
dbcursor=dbcnx.cursor(MySQLdb.cursors.DictCursor)


def checkSWIF():
        print "CHECKING SWIF JOBS"
        #queryswifjobs="SELECT OutputLocation,ID,NumEvents,Completed_Time FROM Project WHERE ID IN (SELECT Project_ID FROM Jobs WHERE IsActive=1 && ID IN (SELECT Job_ID FROM Attempts WHERE BatchSystem= 'SWIF') )"
        queryswifjobs="SELECT * FROM Project WHERE ID IN (SELECT Project_ID FROM Jobs WHERE IsActive=1 && ID IN (SELECT DISTINCT Job_ID FROM Attempts WHERE BatchSystem= 'SWIF' && Completed_Time IS NULL) )"
        dbcursor.execute(queryswifjobs)
        AllWkFlows = dbcursor.fetchall()
       
        TOTCompletedEvtsquery="SELECT Project_ID,SUM(NumEvts) FROM Jobs WHERE ID IN (SELECT Job_ID FROM Attempts WHERE ExitCode=0) GROUP BY Project_ID"
        dbcursor.execute(TOTCompletedEvtsquery)
        TOTCompletedEvtret=dbcursor.fetchall()

        #LOOP OVER SWIF WORKFLOWS
        #print AllWkFlows
        for workflow in AllWkFlows:
            splitnames=workflow["OutputLocation"].split("/")
            wkflowname=splitnames[len(splitnames)-2]
            #print wkflowname
            ProjID=workflow["ID"]
            #statuscommand="swif status -workflow "+str("pim_g3_1_70_v2_20180718011203pm")+" -jobs -display json"
            statuscommand="swif status -workflow "+str(wkflowname)+" -jobs -display json"
            #print statuscommand
            jsonOutputstr=subprocess.check_output(statuscommand.split(" "))
            ReturnedJobs=json.loads(jsonOutputstr)
            #print "*******************"
            #print ReturnedJobs
            #print "======================"
            #LOOP OVER ALL JOBS IN WORKFLOW
            for job in ReturnedJobs["jobs"]:
                #NON RUNNING DISPATCHED JOBS ARE A SPECIAL CASE
                if int(job["num_attempts"]) == 0:
                    #print "truncated update of attempt pre dispatch"
                    updatejobstatus="UPDATE Attempts SET Status=\""+str(job["status"])+"\"" +" WHERE BatchJobID="+str(job["id"])
                    #print updatejobstatus
                    dbcursor.execute(updatejobstatus)
                    dbcnx.commit()
                else:
                    #print "Update all the attempts"
                    LoggedSWIFAttemps_query="SELECT ID from Attempts where BatchJobID="+str(job["id"])+" ORDER BY ID"
                    dbcursor.execute(LoggedSWIFAttemps_query)
                    LoggedSWIFAttemps=dbcursor.fetchall()
                    loggedindex=0
                    #LOOP OVER ALL ATTEMPTS OF A JOBS
                    for attempt in job["attempts"]:
                        #print "|||||||||||||||||||||"
    
                        WallTime=timedelta(seconds=0)
                        CpuTime=timedelta(seconds=0)
                        Start_Time=timedelta(seconds=0)
                        RAMUSED="0MB"
                        ExitCode=0
                        #print attempt
                        #print "||||||||||||||||||||"
                        #print attempt["exitcode"]
                        if attempt["exitcode"] or job["status"]=="succeeded":
                            ExitCode=attempt["exitcode"]
                        else:
                            ExitCode=-1
                        
                        Completed_Time='NULL'

                        if(job["status"]=="problem" or job["status"]=="succeeded"):
                            Completed_Time=attempt["auger_ts_complete"]

                        if(attempt["auger_wall_sec"]):
                            WallTime=timedelta(seconds=attempt["auger_wall_sec"])
                        if(attempt["auger_ts_active"]):
                            Start_Time=timedelta(seconds=attempt["auger_ts_active"])
                        if(attempt["auger_cpu_sec"]):
                            CpuTime=timedelta(seconds=attempt["auger_cpu_sec"])
                        if attempt["auger_vmem_kb"]:
                            RAMUsed=str(float(attempt["auger_vmem_kb"])/1000.)+"MB"

                        #print RAMUsed
                        #print "|||||||||||||||||||||"
                        #SOME VODOO IF RETRY JOBS HAPPENED OUTSIDE OF THE DB
                        if loggedindex == len(LoggedSWIFAttemps):
                            #print "FOUND AN ATTEMPT EXTERNALLY CREATED"
                            GetLinkToJob_query="SELECT Job_ID FROM Attempts WHERE BatchJobID="+str(job["id"])
                            #print GetLinkToJob_query
                            dbcursor.execute(GetLinkToJob_query)
                            LinkToJob=dbcursor.fetchall()

                            if(len(LinkToJob)==0):
                                continue

                            #print len(LoggedSWIFAttemps)
                            #print LinkToJob
                            #print datetime.fromtimestamp(float(attempt["auger_ts_submitted"])/float(1000))
                            
                            addFoundAttempt="INSERT INTO Attempts (Job_ID,Creation_Time,BatchSystem,BatchJobID, ThreadsRequested, RAMRequested,Start_Time) VALUES (%s,'%s','SWIF',%s,%s,%s,%s)" % (LinkToJob[0]["Job_ID"],datetime.fromtimestamp(float(attempt["auger_ts_submitted"])/float(1000)),attempt["job_id"],attempt["cpu_cores"], "'"+str(float(attempt["ram_bytes"])/float(1000000000))+"GB"+"'",datetime.fromtimestamp(float(attempt["auger_ts_active"])/float(1000)))
                            #print addFoundAttempt
                            dbcursor.execute(addFoundAttempt)
                            dbcnx.commit()

                            LoggedSWIFAttemps_query="SELECT ID from Attempts where BatchJobID="+str(job["id"])+" ORDER BY ID"
                            dbcursor.execute(LoggedSWIFAttemps_query)
                            LoggedSWIFAttemps=dbcursor.fetchall()
                            #print len(LoggedSWIFAttemps)

                        #print "UPDATING ATTEMPT"
                        #print str(ExitCode)
                        #UPDATE THE SATUS
                        updatejobstatus="UPDATE Attempts SET Status=\""+str(job["status"])+"\", ExitCode="+str(ExitCode)+", Completed_Time="+time.strftime("%H:%M:%S",time.gmtime(int(Completed_Time)))+", RunningLocation="+"'"+str(attempt["auger_node"])+"'"+", WallTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(WallTime.seconds))+"'"+", Start_Time="+"'"+time.strftime("%H:%M:%S",time.gmtime(Start_Time.seconds))+"'"+", CPUTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(CpuTime.seconds))+"'"+", RAMUsed="+"'"+RAMUsed+"'"+" WHERE BatchJobID="+str(job["id"])+" && ID="+str(LoggedSWIFAttemps[loggedindex]["ID"])
                        if Completed_Time== 'NULL':
                            updatejobstatus="UPDATE Attempts SET Status=\""+str(job["status"])+"\", ExitCode="+str(ExitCode)+", RunningLocation="+"'"+str(attempt["auger_node"])+"'"+", WallTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(WallTime.seconds))+"'"+", Start_Time="+"'"+time.strftime("%H:%M:%S",time.gmtime(Start_Time.seconds))+"'"+", CPUTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(CpuTime.seconds))+"'"+", RAMUsed="+"'"+RAMUsed+"'"+" WHERE BatchJobID="+str(job["id"])+" && ID="+str(LoggedSWIFAttemps[loggedindex]["ID"])

                        #print updatejobstatus
                        dbcursor.execute(updatejobstatus)
                        dbcnx.commit()
                        loggedindex+=1


        #CHECK IF ORDER IS DONE NOW
        TOTCompletedEvt=0
        if TOTCompletedEvtret:
            for proj in TOTCompletedEvtret:
                if str(proj["Project_ID"]) == str(ProjID):
                    TOTCompletedEvt=int(proj["SUM(NumEvts)"])
                    break
        

            #FIND THE RIGHT PROJECT AND IF APPLICABLE MARKE AS COMPLETED
            for workflow in AllWkFlows:
                #print str(ProjID) + " | "+workflow["ID"]
                if ProjID == workflow["ID"] and workflow["NumEvents"] == TOTCompletedEvt:
                    updateProjectstatus="UPDATE Project SET Completed_Time=NOW() WHERE ID="+str(ProjID)+"&& Completed_Time IS NULL;"
                    #print updatejobstatus
                    dbcursor.execute(updateProjectstatus)
                    dbcnx.commit()

       
        
def checkOSG():
        print "CHECKING OSG JOBS"
        queryswifjobs="SELECT * FROM Project WHERE ID IN (SELECT Project_ID FROM Jobs WHERE IsActive=1 && ID IN (SELECT DISTINCT Job_ID FROM Attempts WHERE BatchSystem= 'OSG' && Completed_Time IS NULL) )"
        dbcursor.execute(queryswifjobs)
        AllWkFlows = dbcursor.fetchall()


        queryosgjobs="SELECT * from Attempts WHERE BatchSystem='OSG' ;"#&& Completed_Time IS NULL;"
        #print queryosgjobs
        dbcursor.execute(queryosgjobs)
        Alljobs = dbcursor.fetchall()

        TOTCompletedEvtsquery="SELECT Project_ID,SUM(NumEvts) FROM Jobs WHERE ID IN (SELECT Job_ID FROM Attempts WHERE ExitCode=0) GROUP BY Project_ID"
        dbcursor.execute(TOTCompletedEvtsquery)
        TOTCompletedEvtret=dbcursor.fetchall()
        
        for job in Alljobs:
            #print job
            statuscommand="condor_q "+str(job["BatchJobID"])+" -json"
            #print statuscommand
            jsonOutputstr=subprocess.check_output(statuscommand.split(" "))
            #print "================"
            #print jsonOutputstr
            #print "================"
            if( jsonOutputstr != ""):
                JSON_jobar=json.loads(jsonOutputstr)
                #print JSON_jobar[0]
                JSON_job=JSON_jobar[0]
                ExitCode="NULL"
                if (JSON_job["JobStatus"]!=3 and JSON_job["JobStatus"]<=1):
                    ExitCode=str(JSON_job["ExitStatus"])


                Completed_Time='NULL'

                if(JSON_job["JobStatus"] >= 3):
                    Completed_Time=JSON_job["JobFinishedHookDone"]

                WallTime=timedelta(seconds=JSON_job["RemoteWallClockTime"])
                CpuTime=timedelta(seconds=JSON_job["RemoteUserCpu"])
                Start_Time=timedelta(seconds=JSON_job["JobStartDate"])
                #"MemoryUsage": "\/Expr(( ( ResidentSetSize + 1023 ) / 1024 ))\/"
                RAMUSED=str(float(int(JSON_job["ResidentSetSize"]) + 1023) / float(1024))+"MB"
                TransINSize=JSON_job["TransferInputSizeMB"]

                updatejobstatus="UPDATE Attempts SET Status=\""+str(JSON_job["JobStatus"])+"\", ExitCode="+ExitCode+", Completed_Time="+"'"+time.strftime("%H:%M:%S",time.gmtime(int(Completed_Time)))+"'"+", Start_Time="+"'"+time.strftime("%H:%M:%S",time.gmtime(Start_Time.seconds))+"'"+", RunningLocation="+"'"+str(JSON_job["RemoteHost"])+"'"+", WallTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(WallTime.seconds))+"'"+", CPUTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(CpuTime.seconds))+"'"+", RAMUsed="+"'"+RAMUSED+"'"+", Size_In="+str(TransINSize)+" WHERE BatchJobID="+str(job["BatchJobID"])+";"
                if Completed_Time == 'NULL':
                    updatejobstatus="UPDATE Attempts SET Status=\""+str(JSON_job["JobStatus"])+"\", ExitCode="+ExitCode+", Start_Time="+"'"+time.strftime("%H:%M:%S",time.gmtime(Start_Time.seconds))+"'"+", RunningLocation="+"'"+str(JSON_job["RemoteHost"])+"'"+", WallTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(WallTime.seconds))+"'"+", CPUTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(CpuTime.seconds))+"'"+", RAMUsed="+"'"+RAMUSED+"'"+", Size_In="+str(TransINSize)+" WHERE BatchJobID="+str(job["BatchJobID"])+";"

                #print updatejobstatus
                dbcursor.execute(updatejobstatus)
                dbcnx.commit()
            else:
                #print "looking up history"
                historystatuscommand="condor_history "+str(job["BatchJobID"])+" -json"
                #print historystatuscommand
                jsonOutputstr=subprocess.check_output(historystatuscommand.split(" "))
                #print "================"
                #print jsonOutputstr
                #print "================"
                if( jsonOutputstr != ""):
                    JSON_jobar=json.loads(jsonOutputstr)
                    #print JSON_jobar[0]
                    JSON_job=JSON_jobar[0]
                    ExitCode="NULL"

                    #print JSON_job["JobStatus"]
                    if (JSON_job["JobStatus"]!=3 and JSON_job["JobStatus"]<=1):
                        ExitCode=str(JSON_job["ExitStatus"])
                    
                    Completed_Time='NULL'
                    if(JSON_job["JobStatus"] >= 3):
                        Completed_Time=JSON_job["JobFinishedHookDone"]

                    WallTime=timedelta(seconds=JSON_job["RemoteWallClockTime"])
                    CpuTime=timedelta(seconds=JSON_job["RemoteUserCpu"])
                    #"MemoryUsage": "\/Expr(( ( ResidentSetSize + 1023 ) / 1024 ))\/"
                    RAMUSED=str(float(int(JSON_job["ResidentSetSize"]) + 1023) / float(1024))+"MB"
                    TransINSize=JSON_job["TransferInputSizeMB"]
                    updatejobstatus="UPDATE Attempts SET Status=\""+str(JSON_job["JobStatus"])+"\", ExitCode="+ExitCode+", Completed_Time="+time.strftime("%H:%M:%S",time.gmtime(int(Completed_Time)))+", RunningLocation="+"'"+str(JSON_job["LastRemoteHost"])+"'"+", WallTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(WallTime.seconds))+"'"+", CPUTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(CpuTime.seconds))+"'"+", RAMUsed="+"'"+RAMUSED+"'"+", Size_In="+str(TransINSize)+" WHERE BatchJobID="+str(job["BatchJobID"])+";"
                    
                    if Completed_Time == 'NULL':
                        updatejobstatus="UPDATE Attempts SET Status=\""+str(JSON_job["JobStatus"])+"\", ExitCode="+ExitCode+", RunningLocation="+"'"+str(JSON_job["LastRemoteHost"])+"'"+", WallTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(WallTime.seconds))+"'"+", CPUTime="+"'"+time.strftime("%H:%M:%S",time.gmtime(CpuTime.seconds))+"'"+", RAMUsed="+"'"+RAMUSED+"'"+", Size_In="+str(TransINSize)+" WHERE BatchJobID="+str(job["BatchJobID"])+";"

                    #print updatejobstatus
                    dbcursor.execute(updatejobstatus)
                    dbcnx.commit()

        TOTCompletedEvt=0
        if TOTCompletedEvtret:
            for proj in TOTCompletedEvtret:
                for order in AllWkFlows:
                    if str(proj["Project_ID"]) == str(order["ID"]):
                        TOTCompletedEvt=int(proj["SUM(NumEvts)"])
                        if( order["NumEvents"] == TOTCompletedEvt):
                            updateProjectstatus="UPDATE Project SET Completed_Time=NOW() WHERE ID="+str(ProjID)+"&& Completed_Time IS NULL;"
                            print updatejobstatus
                            dbcursor.execute(updateProjectstatus)
                            dbcnx.commit()

########################################################## MAIN ##########################################################
        
def main(argv):

        checkSWIF()
        checkOSG()
        
        dbcnx.close()
                
if __name__ == "__main__":
   main(sys.argv[1:])
