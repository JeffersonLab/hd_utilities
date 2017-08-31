#!/usr/bin/python

import os,sys

loc = 'calibrations/'

RUNS = []

for f in os.listdir(loc):
    if f.startswith('base_time'):
        r = f[28:33]
        RUNS.append(r)

RUNS.sort()

start = '30000'
for k in range(0,len(RUNS)-1):
    stop = str(int(RUNS[k+1])-1)

    cmd0 = 'ccdb add -r '+start+'-'+stop
    cmd1 = ' /TOF/base_time_offset '
    f1 = loc+'base_time_offsets_NEWAMP_run'+RUNS[k]+'.dat'
    cmd2 = ' /TOF/adc_timing_offsets '
    f2 = loc+'adc_timing_offsets_NEWAMP_run'+RUNS[k]+'.dat'

    cmdfinal1 = cmd0+cmd1+f1
    cmdfinal2 = cmd0+cmd2+f2

    print cmdfinal1
    print cmdfinal2

    os.system(cmdfinal1)
    os.system(cmdfinal2)
    start = RUNS[k+1]

   
stop = '39999'
start = RUNS[len(RUNS)-1]
cmd0 = 'ccdb add -r '+start+'-'+stop
cmd1 = ' /TOF/base_time_offset '
f1 = loc+'base_time_offsets_NEWAMP_run'+start+'.dat'
cmd2 = ' /TOF/adc_timing_offsets '
f2 = loc+'adc_timing_offsets_NEWAMP_run'+start+'.dat'

cmdfinal1 = cmd0+cmd1+f1
cmdfinal2 = cmd0+cmd2+f2

print cmdfinal1
print cmdfinal2

os.system(cmdfinal1)
os.system(cmdfinal2)

