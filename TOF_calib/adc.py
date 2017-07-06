#!/usr/bin/python

# consolidate all data in the directory ADC_positions
# and create one file for each PMT to give the Integral (Peam Amplitude)
# of the landay peak as a function of run number
# this file then can be used by the root script drawp.C


import os,sys,getopt
import numpy as np

opts, args = getopt.getopt(sys.argv[1:],"a:A:hH")

AMP = 0
beg = 16
fin = 21
for opt,arg in opts:
    if opt in ("-A" or "-a"):
        AMP = int(arg)
        if AMP:
            beg = 18
            fin = 23
        else:
            beg = 16
            fin = 21

    if opt in ("-h" or "-H"):
        print "useage:   ./adc.py -A [0,1]   # 0 for Integral, 1 for amplitude"
        sys.exit()

Data = np.zeros((500,176,4))

loc = 'ADC_positions/'

str1 = 'adc_integral' 
if AMP>0:
    str1 = 'adc_amplitudes'

RUNS = []

for d in os.listdir(loc):
    if d.startswith(str1):
        print d
        RUNS.append(d)


RUNS.sort()

RunNum = []
Cnt = 0
for f in RUNS:
    inf = open(loc+f,'r')
    run = int(f[beg:fin])
    RunNum.append(run) 
    for k in range(0,176):
        line = inf.readline()
        l = line.split()
        Data[Cnt][int(l[0])][0] = l[1] 
        Data[Cnt][int(l[0])][1] = l[2] 
        Data[Cnt][int(l[0])][2] = l[3] 
        Data[Cnt][int(l[0])][3] = l[4] 

    Cnt += 1
    inf.close()

str2 = '';
if (AMP):
    str2 = 'amp_'
for k in range(0,176):
    fnam = loc+'pmt_'+str2+str(k+1)+'.dat'
    outf = open(fnam,'w')
    for n in range(0,Cnt):
        s1 = str(RunNum[n])
        s1 += '  '+str(Data[n][k][0])
        s1 += '  '+str(Data[n][k][1])+'\n'
        outf.write(s1)
    outf.close()



