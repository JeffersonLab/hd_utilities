#!/usr/bin/python


import subprocess
import sys,getopt,os;
import math

#opts, args = getopt.getopt(sys.argv[1:],"0:1:R:yY")

OK = 1
loc = './'

files = []
dbs = []
files.append("tofpmt_tdc_offsets_all_FULL_run")
dbs.append("timing_offsets_NEWAMP")

files.append("tof_walk_parameters_run")
dbs.append("timewalk_parms_NEWAMP")

files.append("tofpaddles_propagation_speed_run")
dbs.append("propagation_speed_NEWAMP") 

DIRS = []
RUNS = []
Cnts = 0
for d in os.listdir(loc):
    if d.startswith('calibration3'):
        r = d[11:16]

        nfiles = 0
        for k in range(0,3):    
            fname = loc+d+"/"+files[k]+str(r)+".DB"
            if os.path.isfile(fname): 
                nfiles += 1
        if nfiles == 3:
            RUNS.append(int(r))
            DIRS.append(d)
            Cnts +=1
        else:
            print "Error: Not all DB files found for run ",r

DIRS.sort()
RUNS.sort()


for r in range(0,Cnts):
    RunNumber0 = str(RUNS[r])
    RunNumber1 = '39999'
    if r<Cnts-1:
        RunNumber1 = str(RUNS[r+1]-1)

    for k in range(0,3):    
        thedir = loc+DIRS[r]
        cmd = "ccdb add -r "+RunNumber0+"-"+RunNumber1+"   /TOF/"+dbs[k]+"   "+thedir+"/"+files[k]+RunNumber0+".DB"
        print cmd
        if (OK):
            os.system(cmd)



