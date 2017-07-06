#!/usr/bin/python

# purpose: read either the timing offset parmaeters and generate a file
#          that contains these parameters as a function of run number for all 176 PMTs
#          OR: read the propagation speeds for all paddels and create a file that
#              contains these values for all paddles as a function of run number
#             
#          the resulting file then can be read by the root script calview.C to generate plots
#          showing the run/time dependence of these parameters.
#

import os,sys,getopt,math


PAR = 1
RANGE = 176
opts, args = getopt.getopt(sys.argv[1:],"SsHh")

for opt,arg in opts:
    if opt in ("-s","-S"):
        PAR = 2
        RANGE = 88
    if opt in ("-h","-H"):
        print "usage:  ./calview.py [-sS]   # without parameter plot offsets, with -s plot velocities"
        sys.exit()

loc = './'

DIRS = []

for d in os.listdir(loc):
    if d.startswith('calibration3'):
        DIRS.append(d)

DIRS.sort()

of = 'tofcalib_timing_parms.dat'
if PAR == 2:
    of = 'tofcalib_speed_parms.dat'

outf = open(of,'w')

for d in DIRS:
    rnum = d[11:16]
    f = d+'/'+'tofpmt_tdc_offsets_all_FULL_run'+rnum+'.DB'
    if PAR==2:
        f = d+'/'+'tofpaddles_propagation_speed_run'+rnum+'.DB'

    if os.path.isfile(f):
        inf = open(f,'r')
        lin = rnum
        for k in range(0,RANGE):
            l = inf.readline()
            s = l.split()
            val = s[0]
            lin += '  '+val
            
        lin += '\n'
        outf.write(lin)

outf.close()
    
