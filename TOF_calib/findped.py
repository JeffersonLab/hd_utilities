#!/usr/bin/python


import os,sys,getopt

opts, args = getopt.getopt(sys.argv[1:],"yYP:p:")
OK = 0         # enable actual ccdb command

RunPeriod =0
# check input parameter to verify if you really want to execute the commands
for opt,arg in opts:
    if opt in ("-Y","-y"):
        OK = 1
    if opt in ("-P","-p"):
        RunPeriod = int(arg)


if RunPeriod<1:
    print "Missing Run Period, has to be specified: -P 7 [6,5,4]"
    sys.exit()

# default set to 7
loc = '/cache/halld/home/zihlmann/TOF_calib/fall19/'
loc = '/work/halld2/home/zihlmann/HallD/work/tofcalib/jobs/'

if RunPeriod == 6:
    loc = '/cache/halld/home/zihlmann/TOF_calib/spring19/'

if RunPeriod == 5:
    loc = '/cache/halld/home/zihlmann/TOF_calib/fall18/'

if RunPeriod == 4:
    loc = '/cache/halld/home/zihlmann/TOF_calib/spring18/'

RunList = []
if (RunPeriod<7):
    for f in os.listdir(loc):
        if f.endswith('.root'):
            r = f[11:16]
            RunList.append(int(r))
else:
    for d in os.listdir(loc):
        if d.startswith('Run'):
            r = d[3:8]
            RunList.append(str(r))

RunList.sort()
for R in RunList:
    cmd = 'root -q -b \"src/baseline.C('+str(R)+')\"'
    print cmd
    if OK:
        os.system(cmd)
        #sys.exit()
