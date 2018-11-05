import MySQLdb
import sys
import datetime
from optparse import OptionParser
import subprocess
from subprocess import call

dbhost = "hallddb.jlab.org"
dbuser = 'mcuser'
dbpass = ''
dbname = 'gluex_mc'

conn=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
curs=conn.cursor(MySQLdb.cursors.DictCursor)


#DELETE FROM Attempts WHERE Job_ID IN (SELECT ID FROM Jobs WHERE Project_ID=65);

def ListUnDispatched():
    query = "SELECT * FROM Project WHERE Is_Dispatched='NO'"
    curs.execute(query) 
    rows=curs.fetchall()
    print(rows)

def DispatchProject(ID,SYSTEM,PERCENT):
    query = "SELECT * FROM Project WHERE ID="+str(ID)
    curs.execute(query) 
    rows=curs.fetchall()

    if(len(rows) != 0):
        if SYSTEM == "INTERACTIVE":
            DispatchToInteractive(ID,rows[0],PERCENT)
        elif SYSTEM == "SWIF":
            DispatchToSWIF(ID,rows[0],PERCENT)
        elif SYSTEM == "OSG":
            DispatchToOSG(ID,rows[0],PERCENT)
    else:
        print("Error: Cannot find Project with ID="+ID)
    
    
def DispatchToInteractive(ID,order,PERCENT):
    MCconfig_file= open("MCDispatched.config","w+")
    MCconfig_file.write(order["Config_Stub"])
    MinE=str(order["GenMinE"])
    if len(MinE) > 5:
        cutnum=len(MinE)-5
        MinE = MinE[:-cutnum]
    MaxE=str(order["GenMaxE"])
    if len(MaxE) > 5:
        cutnum=len(MaxE)-5
        MaxE = MaxE[:-cutnum]

    MCconfig_file.write("GEN_MIN_ENERGY="+MinE+"\n")
    MCconfig_file.write("GEN_MAX_ENERGY="+MaxE+"\n")
    MCconfig_file.write("NOSECONDARIES="+str(abs(order["GeantSecondaries"]-1))+"\n")
    MCconfig_file.close()
    RunNumber=str(order["RunNumLow"])
    if order["RunNumLow"] != order["RunNumHigh"] :
        RunNumber = RunNumber + "-" + str(order["RunNumHigh"])


    cleangen=1
    if order["SaveGeneration"]==1:
        cleangen=0

    cleangeant=1
    if order["SaveGeant"]==1:
        cleangeant=0
    
    cleansmear=1
    if order["SaveSmear"]==1:
        cleansmear=0
    
    cleanrecon=1
    if order["SaveReconstruction"]==1:
        cleanrecon=0

    # CHECK THE OUTSTANDING JOBS VERSUS ORDER
    TotalOutstanding_Events_check = "SELECT SUM(NumEvts), MAX(FileNumber) FROM Jobs WHERE IsActive=1 && Project_ID="+str(ID)+" && ID IN (SELECT Job_ID FROM Attempts WHERE ExitCode=0);"
    curs.execute(TotalOutstanding_Events_check)
    TOTALOUTSTANDINGEVENTS = curs.fetchall()

    RequestedEvents_query = "SELECT NumEvents FROM Project WHERE ID="+str(ID)+";"
    curs.execute(RequestedEvents_query)
    TotalRequestedEventsret = curs.fetchall()
    TotalRequestedEvents= TotalRequestedEventsret[0]["NumEvents"]

    OutstandingEvents=0
    if(TOTALOUTSTANDINGEVENTS[0]["SUM(NumEvts)"]):
        OutstandingEvents=TOTALOUTSTANDINGEVENTS[0]["SUM(NumEvts)"]
    
    FileNumber_NewJob=int(-1)
    if(TOTALOUTSTANDINGEVENTS[0]["MAX(FileNumber)"]):
        FileNumber_NewJob=TOTALOUTSTANDINGEVENTS[0]["MAX(FileNumber)"]

    FileNumber_NewJob+=1

    NumEventsToProduce=min(int(float(TotalRequestedEvents)*float(PERCENT)),TotalRequestedEvents-OutstandingEvents)
    
    percentDisp=float(NumEventsToProduce+OutstandingEvents)/float(TotalRequestedEvents)

    if NumEventsToProduce > 0:
        updatequery="UPDATE Project SET Is_Dispatched='"+str(percentDisp) +"', Dispatched_Time="+"NOW() "+"WHERE ID="+str(ID)+";"
        #print updatequery
        curs.execute(updatequery)
        conn.commit()
        command="gluex_MC.py MCDispatched.config "+str(RunNumber)+" "+str(NumEventsToProduce)+" per_file=250000 base_file_number="+str(FileNumber_NewJob)+" generate="+str(order["RunGeneration"])+" cleangenerate="+str(cleangen)+" geant="+str(order["RunGeant"])+" cleangeant="+str(cleangeant)+" mcsmear="+str(order["RunSmear"])+" cleanmcsmear="+str(cleansmear)+" recon="+str(order["RunReconstruction"])+" cleanrecon="+str(cleanrecon)+" projid="+str(ID)+" batch=0"
        print(command)
        status = subprocess.call(command, shell=True)
    else:
        print "All jobs submitted for this order"

def DispatchToSWIF(ID,order,PERCENT):
    status = subprocess.call("cp $MCWRAPPER_CENTRAL/examples/SWIFShell.config ./MCDispatched.config", shell=True)
    MCconfig_file= open("MCDispatched.config","a")
    splitlist=order["OutputLocation"].split("/")
    MCconfig_file.write("WORKFLOW_NAME="+splitlist[len(splitlist)-2]+"\n")
    MCconfig_file.write(order["Config_Stub"])
    MinE=str(order["GenMinE"])
    if len(MinE) > 5:
        cutnum=len(MinE)-5
        MinE = MinE[:-cutnum]
    MaxE=str(order["GenMaxE"])
    if len(MaxE) > 5:
        cutnum=len(MaxE)-5
        MaxE = MaxE[:-cutnum]
    MCconfig_file.write("GEN_MIN_ENERGY="+MinE+"\n")
    MCconfig_file.write("GEN_MAX_ENERGY="+MaxE+"\n")
    MCconfig_file.write("NOSECONDARIES="+str(abs(order["GeantSecondaries"]-1))+"\n")
    MCconfig_file.close()
    RunNumber=str(order["RunNumLow"])
    if order["RunNumLow"] != order["RunNumHigh"] :
        RunNumber = RunNumber + "-" + str(order["RunNumHigh"])


    cleangen=1
    if order["SaveGeneration"]==1:
        cleangen=0

    cleangeant=1
    if order["SaveGeant"]==1:
        cleangeant=0
    
    cleansmear=1
    if order["SaveSmear"]==1:
        cleansmear=0
    
    cleanrecon=1
    if order["SaveReconstruction"]==1:
        cleanrecon=0

    # CHECK THE OUTSTANDING JOBS VERSUS ORDER
    TotalOutstanding_Events_check = "SELECT SUM(NumEvts), MAX(FileNumber) FROM Jobs WHERE IsActive=1 && Project_ID="+str(ID)+" && ID IN (SELECT Job_ID FROM Attempts WHERE ExitCode=0);"
    curs.execute(TotalOutstanding_Events_check)
    TOTALOUTSTANDINGEVENTS = curs.fetchall()

    RequestedEvents_query = "SELECT NumEvents FROM Project WHERE ID="+str(ID)+";"
    curs.execute(RequestedEvents_query)
    TotalRequestedEventsret = curs.fetchall()
    TotalRequestedEvents= TotalRequestedEventsret[0]["NumEvents"]

    OutstandingEvents=0
    if(TOTALOUTSTANDINGEVENTS[0]["SUM(NumEvts)"]):
        OutstandingEvents=TOTALOUTSTANDINGEVENTS[0]["SUM(NumEvts)"]
    
    FileNumber_NewJob=int(-1)
    if(TOTALOUTSTANDINGEVENTS[0]["MAX(FileNumber)"]):
        FileNumber_NewJob=TOTALOUTSTANDINGEVENTS[0]["MAX(FileNumber)"]

    FileNumber_NewJob+=1

    NumEventsToProduce=min(int(float(TotalRequestedEvents)*float(PERCENT)),TotalRequestedEvents-OutstandingEvents)
    
    percentDisp=float(NumEventsToProduce+OutstandingEvents)/float(TotalRequestedEvents)

    if NumEventsToProduce > 0:
        updatequery="UPDATE Project SET Is_Dispatched='"+str(percentDisp) +"', Dispatched_Time="+"NOW() "+"WHERE ID="+str(ID)+";"
        #print updatequery
        curs.execute(updatequery)
        conn.commit()
        command="$MCWRAPPER_CENTRAL/gluex_MC.py MCDispatched.config "+str(RunNumber)+" "+str(NumEventsToProduce)+" per_file=50000 base_file_number="+str(FileNumber_NewJob)+" generate="+str(order["RunGeneration"])+" cleangenerate="+str(cleangen)+" geant="+str(order["RunGeant"])+" cleangeant="+str(cleangeant)+" mcsmear="+str(order["RunSmear"])+" cleanmcsmear="+str(cleansmear)+" recon="+str(order["RunReconstruction"])+" cleanrecon="+str(cleanrecon)+" projid="+str(ID)+" batch=2"
        print(command)
        status = subprocess.call(command, shell=True)
    else:
        print "All jobs submitted for this order"



def DispatchToOSG(ID,order,PERCENT):
    status = subprocess.call("cp $MCWRAPPER_CENTRAL/examples/OSGShell.config ./MCDispatched.config", shell=True)
    MCconfig_file= open("MCDispatched.config","a")
    splitlist=order["OutputLocation"].split("/")
    MCconfig_file.write("WORKFLOW_NAME="+splitlist[len(splitlist)-2]+"\n")
    MCconfig_file.write(order["Config_Stub"]+"\n")
    MinE=str(order["GenMinE"])
    if len(MinE) > 5:
        cutnum=len(MinE)-5
        MinE = MinE[:-cutnum]
    MaxE=str(order["GenMaxE"])
    if len(MaxE) > 5:
        cutnum=len(MaxE)-5
        MaxE = MaxE[:-cutnum]
    MCconfig_file.write("GEN_MIN_ENERGY="+MinE+"\n")
    MCconfig_file.write("GEN_MAX_ENERGY="+MaxE+"\n")
    MCconfig_file.write("NOSECONDARIES="+str(abs(order["GeantSecondaries"]-1))+"\n")
    MCconfig_file.close()
    RunNumber=str(order["RunNumLow"])
    if order["RunNumLow"] != order["RunNumHigh"] :
        RunNumber = RunNumber + "-" + str(order["RunNumHigh"])


    cleangen=1
    if order["SaveGeneration"]==1:
        cleangen=0

    cleangeant=1
    if order["SaveGeant"]==1:
        cleangeant=0
    
    cleansmear=1
    if order["SaveSmear"]==1:
        cleansmear=0
    
    cleanrecon=1
    if order["SaveReconstruction"]==1:
        cleanrecon=0

    # CHECK THE OUTSTANDING JOBS VERSUS ORDER
    TotalOutstanding_Events_check = "SELECT SUM(NumEvts), MAX(FileNumber) FROM Jobs WHERE IsActive=1 && Project_ID="+str(ID)+" && ID IN (SELECT Job_ID FROM Attempts WHERE ExitCode=0);"
    curs.execute(TotalOutstanding_Events_check)
    TOTALOUTSTANDINGEVENTS = curs.fetchall()

    RequestedEvents_query = "SELECT NumEvents FROM Project WHERE ID="+str(ID)+";"
    curs.execute(RequestedEvents_query)
    TotalRequestedEventsret = curs.fetchall()
    TotalRequestedEvents= TotalRequestedEventsret[0]["NumEvents"]

    OutstandingEvents=0
    if(TOTALOUTSTANDINGEVENTS[0]["SUM(NumEvts)"]):
        OutstandingEvents=TOTALOUTSTANDINGEVENTS[0]["SUM(NumEvts)"]
    
    FileNumber_NewJob=int(-1)
    if(TOTALOUTSTANDINGEVENTS[0]["MAX(FileNumber)"]):
        FileNumber_NewJob=TOTALOUTSTANDINGEVENTS[0]["MAX(FileNumber)"]

    FileNumber_NewJob+=1

    NumEventsToProduce=min(int(float(TotalRequestedEvents)*float(PERCENT)),TotalRequestedEvents-OutstandingEvents)
    
    percentDisp=float(NumEventsToProduce+OutstandingEvents)/float(TotalRequestedEvents)

    if NumEventsToProduce > 0:
        updatequery="UPDATE Project SET Is_Dispatched='"+str(percentDisp) +"', Dispatched_Time="+"NOW() "+"WHERE ID="+str(ID)+";"
        #print updatequery
        curs.execute(updatequery)
        conn.commit()
        command="$MCWRAPPER_CENTRAL/gluex_MC.py MCDispatched.config "+str(RunNumber)+" "+str(NumEventsToProduce)+" per_file=100000 base_file_number="+str(FileNumber_NewJob)+" generate="+str(order["RunGeneration"])+" cleangenerate="+str(cleangen)+" geant="+str(order["RunGeant"])+" cleangeant="+str(cleangeant)+" mcsmear="+str(order["RunSmear"])+" cleanmcsmear="+str(cleansmear)+" recon="+str(order["RunReconstruction"])+" cleanrecon="+str(cleanrecon)+" projid="+str(ID)+" batch=1"
        print(command)
        status = subprocess.call(command, shell=True)
    else:
        print "All jobs submitted for this order"

def main(argv):
    #print(argv)


    #print(args)
    ID=-1
    MODE="VIEW"
    SYSTEM="NULL"
    PERCENT=1.0
    argindex=-1

    for argu in argv:
            argindex=argindex+1

            if argindex == 1:
                MODE=str(argv[0]).upper()
            
            if argindex == len(argv)-1:
                ID=argv[argindex]
            
            if argu[0] == "-":
                if argu == "-sys":
                    SYSTEM=str(argv[argindex+1]).upper()
                if argu == "-percent":
                    PERCENT=argv[argindex+1]


    #print MODE
    #print SYSTEM
    #print ID

    if MODE == "DISPATCH" and ID != "All":
        DispatchProject(ID,SYSTEM,PERCENT)
    elif MODE == "DISPATCH" and ID == "All":
        query = "SELECT ID FROM Project WHERE Is_Dispatched!='1.0'"
        curs.execute(query) 
        rows=curs.fetchall()
        for row in rows:
            print(row["ID"])
            DispatchProject(row["ID"],SYSTEM,PERCENT)
    elif MODE=="VIEW" and ID == "Und":
        ListUnDispatched()





    

if __name__ == "__main__":
   main(sys.argv[1:])
