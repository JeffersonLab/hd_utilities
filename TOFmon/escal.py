#!/usr/bin/python

import os,sys,getopt

opts,args = getopt.getopt(sys.argv[1:],"p:P:")

Period = '7'
for opt,arg in opts:
    if opt in ("-p","-P"):
        Period = arg
 

ESCAL = 0.0046   # energy deposition in GeV of minum ionizing particle in TOF paddle

loc = 'results/'
inputs = []
RunNum = []
for f in os.listdir(loc):
    if f.startswith('landauMPV_run'+Period):
        if f.find('_integral')>1:
            inputs.append(f)
            RunNum.append(int(f[13:18]))

RunNum.sort()
for R in RunNum:
    inf = loc+'landauMPV_run'+str(R)+'_integral.dat'
    INF = open(inf,'r')
    outf = loc+'energycal_run'+str(R)+'_integral.dat'
    OUTF = open(outf,'w')
    print("RUN: ",R)
    for line in INF:
        val = line.split()
        conv = ESCAL/float(val[2])  # unit is GeV/ADC
        newline = line.rstrip()+'   '+str(conv)
        print(newline)
        OUTF.write(' '+str(conv)+'\n')

    INF.close()
    OUTF.close()
