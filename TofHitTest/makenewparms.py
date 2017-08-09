#!/usr/bin/python

import os,sys

loc = 'localdir/'
RUNS = []
DIRS = []
THRESH = 30488
for d in os.listdir(loc):
    if d.startswith('run'):
        r = int(d[3:8])
        if r>THRESH:
            RUNS.append(r)
            DIRS.append(d)

RUNS.sort()
DIRS.sort()

del RUNS[-1]
for r in RUNS:
    cmd = 'root -b -q \"getadcoff.C('+str(r)+')\"'
    print cmd
    os.system(cmd)
