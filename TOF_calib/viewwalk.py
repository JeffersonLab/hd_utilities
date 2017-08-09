#!/usr/bin/python

# purpose: read walk fit parameters and view as function of run number

import os,sys,getopt,math


PMT = 1
RANGE=176
opts, args = getopt.getopt(sys.argv[1:],"p:P:hH")

for opt,arg in opts:
    if opt in ("-p","-P"):
        PMT = int(arg)
    if opt in ("-h","-H"):
        print "usage:  ./viewwalk.py -p 1,...,176   # PMT number"
        sys.exit()

loc = './'

DIRS = []

for d in os.listdir(loc):
    if d.startswith('calibration3'):
        DIRS.append(d)

DIRS.sort()

of = 'walkparms.dat'
outf = open(of,'w')

for d in DIRS:
    rnum = d[11:16]
    f = d+'/'+'tof_walk_parameters_run'+rnum+'.DB'

    if os.path.isfile(f):
        inf = open(f,'r')
        lin = rnum
        for k in range(0,RANGE):
            l = inf.readline()
            if (PMT-1) == k:
                lin += '  '+l
            
        outf.write(lin)

outf.close()
    

cmd = 'root -q \"viewwalk.C('+str(PMT)+')\"'

os.system(cmd)
