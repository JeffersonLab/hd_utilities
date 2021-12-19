#!/usr/bin/python

# read from directory pedestals/ and create list to write to ccdb

import os,sys,getopt

OK=0

opts, args = getopt.getopt(sys.argv[1:],"yY")
for opt,arg in opts:
    if opt in ("-Y","-y"):
        OK = 1
 
loc = 'pedestals/'

R4List = []
R5List = []
R6List = []
R7List = []
for f in os.listdir(loc):
    if f.startswith('pedpos_'):
        r = int(f[10:15])
        if r<50000:
            R4List.append(f)
        elif r<60000:
            R5List.append(f)
        elif r<70000:
            R6List.append(f)
        elif r<80000:
            R7List.append(f)

R4List.sort()
R5List.sort()
R6List.sort()
R7List.sort()

Start = 70000
Stop = 79999
cnt = 0
oldf = ''
for f in R7List:
    if cnt<1:
        oldf = loc+f
        cnt += 1
        continue
    rend = int(f[10:15])
    cmd = 'ccdb add -r '+str(Start)+'-'+str(rend)+' TOF2/pedestals '+oldf
    print cmd
    if OK:
        os.system(cmd)
    Start = rend
    oldf = loc+f
    cnt += 1
    if cnt == len(R7List):
        cmd = 'ccdb add -r '+str(Start)+'-'+str(Stop)+' TOF2/pedestals '+oldf
        print cmd
        if OK:
            os.system(cmd)
        
