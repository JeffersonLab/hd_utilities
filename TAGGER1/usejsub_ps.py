#!/usr/bin/python

# the new slurm uses jsub os we use jsub
# the executables are located at /site/scicomp/auger-slurm/bin
# location of slurm log files /lustre/expphy/farm_out/zihlmann


import os,sys,getopt

RunNumber = 0

opts, args = getopt.getopt(sys.argv[1:],"v:V:r:R:")
for opt,arg in opts:
    if opt in ("-r","-R"):
        RunNumber = int(arg)

if RunNumber<1:
    print "no run  number specified bail!"
    sys.exit()



SetUpScript = '/home/zihlmann/ifarm70.csh'
#DataDir = '/cache/halld/RunPeriod-2018-01/analysis/ver02/tree_pi0pippim__B4/merged/'
#MssDir =    '/mss/halld/RunPeriod-2018-01/analysis/ver02/tree_pi0pippim__B4/merged/'
#DataDir = '/cache/halld/RunPeriod-2017-01/analysis/ver12/tree_pi0pippim__B4/merged/'
#MssDir = '/mss/halld/RunPeriod-2017-01/analysis/ver12/tree_pi0pippim__B4/merged/'

#DataDir = '/cache/halld/RunPeriod-2018-08/analysis/ver00/tree_pi0pippim__B4/merged/'
#MssDir = '/mss/halld/RunPeriod-2018-08/analysis/ver00/tree_pi0pippim__B4/merged/'

DataDir = '/cache/halld/RunPeriod-2018-01/rawdata/Run0'+str(RunNumber)+'/'
MssDir = '/mss/halld/RunPeriod-2018-01/rawdata/Run0'+str(RunNumber)+'/'
DoneDir = 'localdir/spring18/'

if (RunNumber<20000) and (RunNumber>9999):
    DataDir = '/cache/halld/RunPeriod-2016-02/rawdata/Run0'+str(RunNumber)+'/'
    MssDir = '/mss/halld/RunPeriod-2016-02/rawdata/Run0'+str(RunNumber)+'/'
    DoneDir = 'localdir/spring16/'
    
if (RunNumber<30000) and (RunNumber>19999):
    DataDir = '/cache/halld/RunPeriod-2016-10/rawdata/Run0'+str(RunNumber)+'/'
    MssDir = '/mss/halld/RunPeriod-2016-10/rawdata/Run0'+str(RunNumber)+'/'
    DoneDir = 'localdir/fall16/'
    
if (RunNumber<40000) and (RunNumber>29999):
    DataDir = '/cache/halld/RunPeriod-2017-01/rawdata/Run0'+str(RunNumber)+'/'
    MssDir = '/mss/halld/RunPeriod-2017-01/rawdata/Run0'+str(RunNumber)+'/'
    DoneDir = 'localdir/spring17/'
    
if (RunNumber<60000) and (RunNumber>49999):
    DataDir = '/cache/halld/RunPeriod-2018-08/rawdata/Run0'+str(RunNumber)+'/'
    MssDir = '/mss/halld/RunPeriod-2018-08/rawdata/Run0'+str(RunNumber)+'/'
    DoneDir = 'localdir/fall18/'

if (RunNumber<70000) and (RunNumber>59999):
    DataDir = '/cache/halld/RunPeriod-2019-01/rawdata/Run0'+str(RunNumber)+'/'
    MssDir = '/mss/halld/RunPeriod-2019-01/rawdata/Run0'+str(RunNumber)+'/'
    DoneDir = 'localdir/spring19/'

if (RunNumber<80000) and (RunNumber>69999):
    DataDir = '/cache/halld/RunPeriod-2019-11/rawdata/Run0'+str(RunNumber)+'/'
    MssDir = '/mss/halld/RunPeriod-2019-11/rawdata/Run0'+str(RunNumber)+'/'
    DoneDir = 'localdir/fall19/'

config = []
config.append('PROJECT: gluex')
config.append('TRACK: analysis')
config.append('COMMAND: /w/halld-scifs17exp/halld2/home/zihlmann/HallD/work/tagger/RunScript.csh')
config.append('OS: centos77')
JOBNAMEBASE = 'TaggerPSTest' 
config.append('TIME: 480')
config.append('CPU: 4')
config.append('DISK_SPACE: 30000 MB')
config.append('MEMORY: 2 GB')

file_list = []
RunList = []

for f in os.listdir(MssDir):
    if f.endswith(".evio"):
        #print f
        file_list.append(f)

file_list.sort()

IDX = []

for f in file_list:
    a1 = f.find('.evio')
    id = f[a1-3:a1]
    IDX.append(id)

if len(IDX)<4:
    print("Not enough evio files found! BAILfor run ",RunNumber)
    sys.exit()

for k in range(0,5):

    of = 'jsub'+str(RunNumber)+'_'+IDX[k]+'.conf'
    OF = open(of, 'w')
    for n in range(0,len(config)):
        line = config[n]+'\n'
        OF.write(line)
        
    line = 'INPUT_FILES: '+MssDir+file_list[k]+'\n'
    OF.write(line)
    TheFile = DataDir+file_list[k]

    line = 'JOBNAME: '+JOBNAMEBASE+str(RunNumber)+'_'+IDX[k]+'\n'
    OF.write(line)
    line = 'OPTIONS: '+SetUpScript+' '+str(RunNumber)+' '+TheFile+' '+IDX[k]+'\n'
    OF.write(line)
    OF.close()
    cmd = '/site/scicomp/auger-slurm/bin/jsub '+of
    print cmd

    os.system(cmd)
    #sys.exit()

        
