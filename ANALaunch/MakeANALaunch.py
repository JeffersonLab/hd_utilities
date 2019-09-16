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


translateDict =	{
  "1": "g",
  "2": "ep",
  "3": "em",
  "7": "pi0",
  "8": "pip",
  "9": "pim",
  "10": "kl",
  "11": "kp",
  "12": "km",
  "13": "n",
  "14": "p",
  "15": "antip",
  "16": "ks",
  "17": "eta",
  "18": "lamb",
  "20": "sig0",
  "22": "Xi0",
  "23": "Xim",
  "24": "omegam",
  "25": "antin",
  "26": "antilamb",
  "30": "antiXi0",
  "31": "antiXiPlus"
}

def TranslateID(id):
    #return id
    if id in translateDict.keys():
        return translateDict[id]
    else:
        return id

def TreeName(reaction):
    name=""
    particles=reaction.split("__")[1].split("_")
    for part in particles:
        trans=""
        if part[0]=="m":
            name=name+"miss"
            trans=TranslateID(part[+1:])
        else:
            trans=TranslateID(str(part))

        name=name+trans

    return name

def WriteLinesToFile(ReactionNum,jobj,configF):
    #print "Reaction"+str(ReactionNum)+" "+jobj["Reaction"]
    print "\n"
    #configF.write("\n\n")
    print "#"+jobj["Name"]
    print "#"+TreeName(jobj["Reaction"])
    #configF.write("#"+jobj["Name"]+"\n")
    print "Reaction"+str(ReactionNum)+" "+jobj["Reaction"]
    #configF.write("Reaction"+str(ReactionNum)+" "+jobj["Reaction"])
    decnum=0
    for dec in jobj["Decays"]:
        decnum=decnum+1
        #print "Reaction"+str(ReactionNum)+":Decay"+str(decnum)+" "+dec
        #configF.write("\n")
        print "Reaction"+str(ReactionNum)+":Decay"+str(decnum)+" "+dec
        #configF.write("Reaction"+str(ReactionNum)+":Decay"+str(decnum)+" "+dec)
    #configF.write("\n")
    flagstrpre="Reaction"+str(ReactionNum)+":Flags "

    flagstr=""
    if(jobj["B"] != "1"):
        flagstr+="B"+jobj["B"]+"_"
    if(jobj["F"] != "4"):
        flagstr+="F"+jobj["F"]+"_"
    if(jobj["T"] != "3"):
        flagstr+="T"+jobj["T"]+"_"
    if(jobj["U"] != "0"):
        flagstr+="U"+jobj["U"]+"_"
    
    flagstr=flagstr[:-1]
    
    for M in jobj["Marray"]:
        flagstr+="_M"+str(M)

    if flagstr != "":
        if flagstr[0]=="_":
        #    flagstr="TEST"
            flagstr=flagstr[1:]

    if len(flagstr) > 0:
        print flagstrpre+flagstr

    #configF.write(flagstr)
########################################################## MAIN ##########################################################

def FilterFiles(Files):
    toUse=[]
    for f in Files:
        if f not in toUse:
            toUse.append(f)

    return toUse

def main(argv):

    ananameSTR=""
    restnameSTR=""
    numThread="12"
    isMC=0
    dataset=""

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
        
        elif(splitArg[0]=="dataset"):
            dataset=splitArg[1]
        
        else:
            print "argument: "+splitArg[0]+" not found"
            #exit(1)


    dir="/u/group/halld/www/halldweb/data/webdata/analysis/newlines/"+dataset+"/"
    Files=os.listdir(dir)
    #print Files
    ReactionNum=0

    configF=1#open("ANALaunch.config","w+")
    print "PLUGINS monitoring_hists,ReactionFilter"
    #configF.write("PLUGINS monitoring_hists,ReactionFilter")
    
    #if(isMC != 0):
    #    configF.write(",mcthrown_tree")

    print "\nNTHREADS "+str(numThread)+"\n"
    #configF.write("\n\nNTHREADS "+str(numThread)+"\n\n")
    print "COMBO:MAX_NEUTRALS 15"
    #configF.write("COMBO:MAX_NEUTRALS 15\n\n")

   #print "REST:DATAVERSIONSTRING recon_"+restnameSTR
   #print "ANALYSIS:DATAVERSIONSTRING analysis_"+ananameSTR
    #configF.write("REST:DATAVERSIONSTRING recon_"+restnameSTR+"\n")
    #configF.write("ANALYSIS:DATAVERSIONSTRING analysis_"+ananameSTR)

    
    #Make an array of files to include in a seperate function.  Do below only for those files in that array.  Not directory
    FilesToUse=FilterFiles(Files)

    for f in FilesToUse:
        with open(dir+f) as tf:
            ReactionNum=ReactionNum+1
            data = json.load(tf)
            WriteLinesToFile(ReactionNum,data,configF)
            ##print data["Reaction"]
                
if __name__ == "__main__":
   main(sys.argv[1:])
