#!/usr/bin/python

import os,sys


f1 = "list.dat"

INF1 = open(f1,'r')

for line in INF1:

    val= line.split();
    if len(val)>0:
        cmd = 'root -l -q \"BEAM_overview.C('+val[0]+',2)\"'
        #cmd = './usejsub_ps.py -R '+val[0]
        print(cmd)
        os.system(cmd)



        
