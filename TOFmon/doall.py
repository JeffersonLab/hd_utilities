#!/usr/bin/python

# purpose: analyze data from root file with findamp.C
#         


import os,sys,getopt

opts, args = getopt.getopt(sys.argv[1:],"yY")

DirLoc = 'localdir/'

OK = 0
for opt,arg in opts:
    if opt in ("-y","-Y"):
        OK = 1


ListF = []
for f in os.listdir(DirLoc):
    if f.startswith('tofmon_run'):
        r = int(f[10:15])
        if r<60000:
            ListF.append(f)

ListF.sort()

for f in ListF:
    r = f[10:15]
    if os.path.isfile(DirLoc+'results/landauMPV_run'+r+'_integral.dat'):
        continue
    cmd = 'root -l -q "findamp.C('+r+',1)\"'

    print cmd
    if OK:
        os.system(cmd)
