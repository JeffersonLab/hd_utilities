#!/usr/bin/python

# purpose: write accidental scaling factors to ccdb

import os, sys, getopt

Period = 0
OK = 0
opts, args = getopt.getopt(sys.argv[1:],"p:P:yY")
for opt,arg in opts:
    if opt in ("-p","-P"):
        Period = int(arg)
    if opt in ('-y','-Y'):
        OK = 1

if Period == 0:
    print('No run period given: use option -P or -p')
    sys.exit()


FirstR = 70000
LastR = 73266
inf = 'accidental_scaling_factors_TAGG_fall19.dat'

if Period == 6:
    inf = 'accidental_scaling_factors_TAGG_spring19.dat'
    FirstR = 60000
    LastR = 62020
if Period == 5:
    inf = 'accidental_scaling_factors_TAGG_fall18.dat'
    FirstR = 50000
    LastR = 52715
if Period == 4:
    inf = 'accidental_scaling_factors_TAGG_spring18.dat'
    FirstR = 40000
    LastR = 42577
if Period == 3:
    inf = 'accidental_scaling_factors_TAGG_spring17.dat'
    FirstR = 30000
    LastR = 31057
if Period == 2:
    inf = 'accidental_scaling_factors_TAGG_fall16.dat'
    FirstR = 20000
    LastR = 20000
if Period == 1:
    inf = 'accidental_scaling_factors_TAGG_spring16.dat'
    FirstR = 10000
    LastR = 11663

INF = open(inf, 'r')

DATA = {}
for line in INF:
    val = line.split()
    RunNumber = val[0]
    Micro = val[2]
    HodHigh = val[4]
    HodLow = '1.0'
    if val[5].find('nan')<0:
        HodLow = val[5]

    str1 = ' '+HodHigh+' 0.01  '+HodLow+'  0.01  '+Micro+'  0.01  8.90090200647 7.87789987845'  
    DATA[RunNumber] = str1

CCDBFile = 'ANALYSIS/accidental_scaling_factor'


for R in range(FirstR, LastR+1):
    Rnum = str(R)
    str1 = ' 1.00  0.01  1.00 0.01  1.00  0.01  8.90090200647 7.87789987845'
    if Rnum in DATA:
        str1 = DATA[Rnum]
        
    cmd1 = 'echo \"'+str1+'\" > file_to_import'
    cmd2 = 'ccdb add ' +CCDBFile+' -r '+Rnum+'-'+Rnum+' file_to_import'
    if (OK):
        os.system(cmd1)
        os.system(cmd2)
    else:
        print Rnum,cmd1

