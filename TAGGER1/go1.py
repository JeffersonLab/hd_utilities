#!/usr/bin/python

import os, sys, getopt

OK = 0
Period = 0
USEMSS = 0
opts,args = getopt.getopt(sys.argv[1:],"P:p:yYmM")
for opt,arg in opts:
    if opt in ("-Y","-y"):
        OK = 1
    if opt in ("-P","-p"):
        Period = int(arg)
    if opt in ("m","M"):
        USEMSS = 1

if Period<1:
    print "User option -P X to indicate which run period you want"
    sys.exit()


#loc = '/mss/halld/RunPeriod-2018-08/rawdata/'
#loc = '/mss/halld/RunPeriod-2019-01/rawdata/'
locmss = '/mss/halld/RunPeriod-2019-11/rawdata/'
loccache = '/cache/halld/RunPeriod-2019-11/rawdata/'
DoneDir = 'localdir/fall19/'

if Period == 7: #fall19
    locmss = '/mss/halld/RunPeriod-2019-11/rawdata/'
    loccache = '/cache/halld/RunPeriod-2019-11/rawdata/'
    DoneDir = 'localdir/fall19/'

if Period == 6: # spring19
    locmss = '/mss/halld/RunPeriod-2019-01/rawdata/'
    loccache = '/cache/halld/RunPeriod-2019-01/rawdata/'
    DoneDir = 'localdir/spring19/'
    
if Period == 5: # fall18
    locmss = '/mss/halld/RunPeriod-2018-08/rawdata/'
    loccache = '/cache/halld/RunPeriod-2018-08/rawdata/'
    DoneDir = 'localdir/fall18/'
    
if Period == 4: # spring18
    locmss = '/mss/halld/RunPeriod-2018-01/rawdata/'
    loccache = '/cache/halld/RunPeriod-2018-01/rawdata/'
    DoneDir = 'localdir/fall18/'

if Period == 3: # spring17
    locmss = '/mss/halld/RunPeriod-2017-01/rawdata/'
    loccache = '/cache/halld/RunPeriod-2017-01/rawdata/'
    DoneDir = 'localdir/fall18/'

if Period == 2: # fall16
    locmss = '/mss/halld/RunPeriod-2016-10/rawdata/'
    loccache = '/cache/halld/RunPeriod-2016-10/rawdata/'
    DoneDir = 'localdir/fall18/'

if Period == 1: # spring16
    locmss = '/mss/halld/RunPeriod-2016-02/rawdata/'
    loccache = '/cache/halld/RunPeriod-2016-02/rawdata/'
    DoneDir = 'localdir/fall18/'

if Period<1:
    sys.exit()


# make list of runs already done

RunsAlreadyDone = {}
for f in os.listdir(DoneDir):
    if f.startswith('pstree'):
        r = f[17:22]
        #print r
        RunsAlreadyDone[r] = 1

RUNSmss = {}
for d in os.listdir(locmss):

    if d.startswith('Run'):
        #print d
        r = d[3:8]
        flist = []
        for f in os.listdir(locmss+d):
            if f.endswith('evio'):
                flist.append(f)

        if len(flist)>10:
            r = d[4:9]
            RUNSmss[r] = len(flist)

RUNScache = {}
for d in os.listdir(loccache):
    if d.startswith('Run'):
        #print d
        r = d[3:8]

        flist = []
        for f in os.listdir(loccache+d):
            if f.endswith('evio'):
                flist.append(f)

        r = d[4:9]
        RUNScache[r] = len(flist)



RUNS = RUNScache
if USEMSS:
    RUNS = RUNSmss

print 'Total number of runs: ',len(RUNS), len(RunsAlreadyDone)
for R in RUNS:

    cmd = './usejsub_ps.py -R '+str(R)
    print cmd
    if OK:
        os.system(cmd)
        #sys.exit()

