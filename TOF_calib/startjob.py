#!/apps/python/PRO/bin/python

import sys,getopt,os;

opts, args = getopt.getopt(sys.argv[1:],"hr:R:d:D:")

Disk = "1"
for opt,arg in opts:
    if opt in ("-r","-R"):
        RunNumber = arg
    if opt in ("-d","-D"):
        Disk = arg

print "RunNumber is ",RunNumber

period = "RunPeriod-2016-10"
#period = "RunPeriod-2016-02"
#period = "RunPeriod-2015-03"
#period = "RunPeriod-2014-10"

location = "/gluex/data/rawdata/all"
RunDir = RunNumber.zfill(6)
loc = location+"/Run"+RunDir

files = []
for f in os.listdir(loc):
    if f.endswith(".evio"):
        print f
        files.append(f)

files.sort()

scriptlines = []

scriptlines.append("#!/bin/csh")
#scriptlines.append("mkdir localdir")
scriptlines.append("cd localdir")
mdir = "mkdir run"+RunNumber
scriptlines.append(mdir)
godir = "cd run"+RunNumber
scriptlines.append(godir)

# theline = "hd_ana --nthreads=10 -PPLUGINS=TOF_calib -PTHREAD_TIMEOUT=500 -PEVIO:ENABLE_DISENTANGLING=0 -PJANA:MAX_RELAUNCH_THREADS=10 -PBFIELD_TYPE=NoField "
theline = "hd_root --nthreads=12 -PPLUGINS=TOF_calib -PTHREAD_TIMEOUT=500 -PJANA:MAX_RELAUNCH_THREADS=10 "
#theline = "hd_ana --nthreads=10 -PPLUGINS=TOF_calib -PTHREAD_TIMEOUT=500 -PEVIO:ENABLE_DISENTANGLING=0 -PJANA:MAX_RELAUNCH_THREADS=10 -PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520 "
#theline = "hd_ana -PPLUGINS=cdc_basics -PJANA:MAX_RELAUNCH_THREADS=10 -PBFIELD_MAP=\'Magnets/Solenoid/solenoid_1000A_poisson_20141104\' "

max = len(files)
max = 40
#if max>25:
#    max = 25

for k in range(0,max):
    theline += (loc+"/"+files[k]+" ")


scriptlines.append(theline)
scriptlines.append("")

fnam2 = "j"+RunNumber+".csh"
fscript = open(fnam2,'w')

for k in range(0,len(scriptlines)):
    fscript.write(scriptlines[k]+"\n")

fscript.close()

os.system("chmod +x "+fnam2)

cmd = "./"+fnam2
os.system(cmd)

