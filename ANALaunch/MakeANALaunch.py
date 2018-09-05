#!/usr/bin/env python
# 2018/09 Thomas Britton


#import MySQLdb.cursors
from os import environ
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

def WriteLinesToFile(ReactionNum,jobj,configF):
    #print "Reaction"+str(ReactionNum)+" "+jobj["Reaction"]
    configF.write("\n\n")
    configF.write("#"+jobj["Name"]+"\n")
    configF.write("Reaction"+str(ReactionNum)+" "+jobj["Reaction"])
    decnum=0
    for dec in jobj["Decays"]:
        decnum=decnum+1
        #print "Reaction"+str(ReactionNum)+":Decay"+str(decnum)+" "+dec
        configF.write("\n")
        configF.write("Reaction"+str(ReactionNum)+":Decay"+str(decnum)+" "+dec)
    
    configF.write("\n")
    flagstr="Reaction"+str(ReactionNum)+":Flags "

   
    if(jobj["B"] != "1"):
        flagstr+="B"+jobj["B"]+"_"
    if(jobj["F"] != "4"):
        flagstr+="F"+jobj["F"]+"_"
    if(jobj["T"] != "3"):
        flagstr+="T"+jobj["T"]+"_"
    
    flagstr=flagstr[:-1]

    for M in jobj["Marray"]:
        flagstr+="_M"+str(M)

    configF.write(flagstr)
########################################################## MAIN ##########################################################

def FilterFiles(Files):
    toUse=[]
    for f in Files:
        if not f in toUse:
            toUse.append(f)
    return toUse

def main(argv):

    ananameSTR=""
    restnameSTR=""
    numThread="12"
    isMC=0

    for argu in argv:
        splitArg=argu.split("=")
        if(splitArg[0]=="anatag"):
            ananameSTR=splitArg[1]
        
        elif(splitArg[0]=="resttag"):
            restnameSTR=splitArg[1]

        elif(splitArg[0]=="mc"):
            isMC=splitArg[1]

        elif(splitArg[0]=="nthread"):
            numThread=splitArg[1]
        
        else:
            print "argument: "+splitArg[0]+" not found"
            exit(1)


    dir="/u/group/halld/www/halldweb/data/webdata/analysis/newlines/"
    Files=os.listdir(dir)
    #print Files
    ReactionNum=0

    configF=open("ANALaunch.config","w+")
    configF.write("PLUGINS monitoring_hists,ReactionFilter")
    
    if(isMC != 0):
        configF.write(",mcthrown_tree")

    configF.write("\n\nNTHREADS "+str(numThread)+"\n\n")
    configF.write("COMBO:MAX_NEUTRALS 15\n\n")

    configF.write("REST:DATAVERSIONSTRING recon_"+restnameSTR+"\n")
    configF.write("ANALYSIS:DATAVERSIONSTRING analysis_"+ananameSTR)

    
    #Make an array of files to include in a seperate function.  Do below only for those files in that array.  Not directory
    FilesToUse=FilterFiles(Files)

    for f in FilesToUse:
        with open(dir+f) as tf:
            ReactionNum=ReactionNum+1
            data = json.load(tf)
            jobj=json.loads(data)
            WriteLinesToFile(ReactionNum,jobj,configF)
            ##print data["Reaction"]
                
if __name__ == "__main__":
   main(sys.argv[1:])
