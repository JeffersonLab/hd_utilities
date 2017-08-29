#!/usr/bin/python

import os,sys

# purpose: run the plugins monitoring_hists, TOF_test and TofHitTest
#          for all runs that were calibrated
#          from the resulting histograms in hd_root.root and tofhittest.root
#          one can test the overall TOF time offset and the timing offsets
#          between ADC and TDC timing.

loc = '/work/halld2/home/zihlmann/HallD/work/tofcalib/TOFCALIB2/'

DIRS = []
RUNS = []

RunPeriod = "3"

for d in os.listdir(loc):
    if d.startswith("calibration"+RunPeriod):
        r = d[11:16]
        if int(r)>30785 and int(r)<30900:
            RUNS.append(r)
            DIRS.append(d)

RUNS.sort()
DIRS.sort()

dataloc = '/cache/halld/RunPeriod-2017-01/rawdata/'
mssloc = '/mss/halld/RunPeriod-2017-01/rawdata/'

for r in RUNS:

    locdir = 'Run0'+r+ '/'
    fulldir = dataloc+locdir
    found = 0
    files = []
    for f in os.listdir(fulldir):
        if f.endswith('.evio'):
            files.append(f)
            found += 1

    if found == 0:
        cmd = 'jget '+mssloc+locdir+'hd_rawdata_0'+r+'_000.evio /work/halld/home/zihlmann/'
        print cmd
        files.append('hd_rawdata_0'+r+'_000.evio')
        os.system(cmd)
        found = 9999

    if found>0:
        files.sort()
        f1 = fulldir+files[0]
        if found == 9999:
            f1 = '/work/halld/home/zihlmann/'+files[0]

        cmd = 'hd_root --nthreads=6 -PPLUGINS=monitoring_hists,TOF_test,TofHitTest -PEVENTS_TO_KEEP=500000 '+f1

        localdir = 'run'+r
        scriptlines = []
        scriptlines.append("#!/bin/csh")
        #scriptlines.append("mkdir localdir")
        scriptlines.append("cd localdir")
        mdir = "mkdir run"+r
        scriptlines.append(mdir)
        godir = "cd run"+r
        scriptlines.append(godir)
        scriptlines.append(cmd)
        scriptlines.append("")

        fnam2 = "j"+r+".csh"
        fscript = open(fnam2,'w')
        
        for k in range(0,len(scriptlines)):
            fscript.write(scriptlines[k]+"\n")

        fscript.close()

        os.system("chmod +x "+fnam2)
        cmd1 = "./"+fnam2
        print cmd1
        os.system(cmd1)
        #sys.exit(0)


