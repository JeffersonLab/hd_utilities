#!/usr/bin/python

# the new slurm uses jsub os we use jsub
# the executables are located at /site/scicomp/auger-slurm/bin
# location of slurm log files /lustre/expphy/farm_out/zihlmann


import os,sys,getopt

opts, args = getopt.getopt(sys.argv[1:],"r:R:yYnN")

SetUpScript = '/home/zihlmann/ifarm70.csh'
DirLoc = '/halld/RunPeriod-2016-02/rawdata/'

RunNumber = 0
OK = 0
UseCacheOnly = 1
for opt,arg in opts:
    if opt in ("-r","-R"):
        RunNumber = int(arg)
    if opt in ("-Y","-y"):
        OK = 1
    if opt in ("-N","-n"):
        UseCacheOnly = 0

print("Run: ",RunNumber)

if (RunNumber>39999):
    DirLoc = '/halld/RunPeriod-2018-01/rawdata/'
if (RunNumber>49999):
    DirLoc = '/halld/RunPeriod-2018-08/rawdata/'
if (RunNumber>59999):
    DirLoc = '/halld/RunPeriod-2019-01/rawdata/'
if (RunNumber>69999):
    DirLoc = '/halld/RunPeriod-2019-11/rawdata/'
    
MssDir = '/mss'+DirLoc
DataDir = '/cache'+DirLoc

config = []
config.append('PROJECT: gluex')
config.append('TRACK: analysis')
config.append('COMMAND: /work/halld2/home/zihlmann/HallD/work/TOFmon/ScriptToRun.csh')
config.append('OS: centos77')
JOBNAMEBASE = 'TOFHVcalib' 
config.append('TIME: 180')
config.append('CPU: 1')
config.append('DISK_SPACE: 30000 MB')
config.append('MEMORY: 2 GB')

# the command expects at least 2 input variables
# these are specified int 'OPTIONS'
# $1 script to set environment variables
# $2 RunNumber


# make list of root files to process    
#loc = 'genroot/localdir/'
loc = DataDir+'Run{0:06d}'.format(RunNumber)+'/'
locmss = MssDir+'Run{0:06d}'.format(RunNumber)+'/'

file_list = []

if UseCacheOnly:
    for f in os.listdir(loc):
        if f.endswith('evio'):
            file_list.append(f)
else:
    for f in os.listdir(locmss):
        if f.endswith('evio'):
            file_list.append(f)
        
file_list.sort()

if (len(file_list)<30):
    print("Run has less than 30 files! Try another one!")
    sys.exit()
        
# do not submit runs that are already running/submit
cmd = '/site/scicomp/auger-slurm/bin/slurmJobs -u zihlmann > slurmsjobs.log'
os.system(cmd)
inf = open('slurmsjobs.log', 'r')
line = inf.readline()
nl = line.split()
locid = 7
RunsDone = []
for line in inf:
    nl = line.split()
    if len(nl)<7:
        continue
    if line.find('TOFHVcalib')>-1:
        job = nl[locid]
        r = int(job[len(nl[locid]-3):len(nl[locid])])
        RunsDone.append(r)
        #print job, r

inf.close()

idx = -1
for k in file_list:        
    idx +=1;
    rid = int(k[18:21])
    #print k, rid
    #sys.exit()
    
    if idx>30:
        break
    
    #check if calibration already exists
    for id in RunsDone:
        if id == rid:
            continue

    of = 'jsub'+str(RunNumber)+'.conf'
    OF = open(of, 'w')
    for n in range(0,len(config)):
        line = config[n]+'\n'
        OF.write(line)
        
    line = 'INPUT_FILES: '+locmss+k+'\n'
    OF.write(line)

    IDX = '{0:03d}'.format(rid)

    #print loc,k,RunNumber,IDX
    #sys.exit()

    line = 'JOBNAME: '+JOBNAMEBASE+str(RunNumber)+'_'+IDX+'\n'
    OF.write(line)
    line = 'OPTIONS: '+SetUpScript+' '+str(RunNumber)+' '+loc+k+' '+IDX
    OF.write(line)
    OF.close()
    cmd = '/site/scicomp/auger-slurm/bin/jsub '+of

    #print cmd
    if OK:
        os.system(cmd)
        #sys.exit()

        
