#!/usr/bin/python

# purpose: sum individual root files into one single root file
#          for all runs found in loc


import os,sys,getopt

opts, args = getopt.getopt(sys.argv[1:],"r:R:yY")

DirLoc = 'localdir/'
RunNumber = 0
OK = 0
for opt,arg in opts:
    if opt in ("-R","-r"):
        RunNumber = arg
    if opt in ("-y","-Y"):
        OK = 1

R1 = RunNumber[0:1]
str1 = 'Run'+R1

RunDirs = {}
for d in os.listdir(DirLoc):
    if d.startswith(str1):
        r = d[3:8]
        RunDirs[r] = int(r)

if len(RunDirs)<1:
    print 'no files found for run series ',RunNumber
    sys.exit()

for key in RunDirs:

    DIRS = []
    print("add up run ",key)
    str1 = str(RunDirs[key])

    for d in os.listdir(DirLoc):
        if d.startswith('Run'):
            if d.find(str1)>1:
                DIRS.append(d)

    DIRS.sort()

    of = DirLoc+'tofdata_run'+str1+'.root'
    of1 = DirLoc+'tofmon_run'+str1+'.root'

    cmd = 'hadd '+of
    cmd1 = 'hadd '+of1

    for k in range(0,len(DIRS)):

        f = DirLoc+DIRS[k]+'/hd_root_tofcalib.root'
        cmd += ' '+f

        f1 = DirLoc+DIRS[k]+'/tofmon.root'
        cmd1 += ' '+f1

    print cmd1
    print
    print cmd
    print
    if OK:
        os.system(cmd)
        os.system(cmd1)

