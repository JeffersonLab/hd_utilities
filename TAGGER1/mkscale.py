#!/usr/bin/python

# SCALING FACTOR DETERMINATION
# OPTION -1 USE HD_ROOT.ROOT FILE
# OPTION -2 USE PSTREERESULTS_RUNxxxxx.ROOT
#
# extract scaling factors for the fall18 data using a modified BEAM_overview.C script
# from the data in hd_root.root file sumed over the first 10 files of a run
# these root files are located in localdir/fall18
#

import os,sys,getopt


# use the following as default
sfile = 'pstreeresults_run'
sfilen = '2'
ID = 2

Period = 0
loc = 'localdir/spring19/'

# option 1 is default
opts, args = getopt.getopt(sys.argv[1:],"12p:P:")
for opt,arg in opts:
    if opt in ("-2"):
        sfile = 'pstreeresults_run'
        sfilen = '2'
        ID = 2
    if opt in ("-1"):
        sfile = 'hd_root'
        sfilen = '1'
        ID = 1
    if opt in ("-p","-P"):
        Period = int(arg)

if Period<1:
    print "Requires option: -P x (x=1,2,3,4,5,6,7)"

if Period == 1:  # runs starting at 60000
    loc = 'localdir/spring16/'

if Period == 2:  # runs starting at 60000
    loc = 'localdir/spring17/'

if Period == 3:  # runs starting at 60000
    loc = 'localdir/fall17/'

if Period == 4:  # runs starting at 60000
    loc = 'localdir/spring18/'

if Period == 5:  # runs starting at 60000
    loc = 'localdir/fall18/'
    
if Period == 6:  # runs starting at 60000
    loc = 'localdir/spring19/'
    
if Period == 7:  # runs starting at 70000
    loc = 'localdir/fall19/'
        
FileList = []
for f in os.listdir(loc):
    if f.startswith(sfile):
        fsize = os.path.getsize(loc+f)
        if fsize > 900000: # make sure there is some data in the file
            FileList.append(f)

FileList.sort()

for f in FileList:
    r = f[11:16]
    if ID == 2:
        r = f[17:22]

    cmd = 'root -l -q \"BEAM_overview.C('+r+', '+sfilen+')\"'
    os.system(cmd);
    #sys.exit()
