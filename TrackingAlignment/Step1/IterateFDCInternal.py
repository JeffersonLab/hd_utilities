#!/usr/bin/env python

import sys
import os
from subprocess import call

#################################
# Setup Parameters for iterations
#################################

nIterations=10
filename='/raid12/gluex/RunPeriod-2017-01/rawdata/Run030778/hd_rawdata_030778_000.evio'
nThreads=4
nEvents=1000000

#############################################
# You shouldn't have to edit below this lines
#############################################

ccdb_connection = os.environ['CCDB_CONNECTION']

if 'mysql' in ccdb_connection:
   print "CONNECTED TO JLAB MYSQL DATABASE === ABORTING"
   quit()

if 'sqlite' not in ccdb_connection:
   print "UNKNOWN DATABASE TYPE %s == MUST USE SQLITE" % ccdb_connection
   quit()

try:
   os.mkdir('FDCInternalAlignIterations');
except OSError:
   pass

for iter in range(1,nIterations):
   print "Iteration %.2i of %.2i" % (iter, nIterations)
   call('hd_root -PPLUGINS=FDC_InternalAlignment %s -PNTHREADS=%i -PEVENTS_TO_KEEP=%i' % (filename, nThreads, nEvents), shell=True)
   call(r'root -l -b -q $HALLD_HOME/src/plugins/Alignment/FDC_InternalAlignment/FitScripts/FitCathodeAlignment.C\(\"hd_root.root\"\)',shell=True)
   call('mv Result.png FDCInternalAlignIterations/Result_Iter%.2i.png' % iter, shell=True)
   call('mv hd_root.root FDCInternalAlignIterations/hd_root_Planes_Iter%.2i.root' % iter, shell=True)
   call('ccdb add /FDC/cathode_alignment CathodeAlignment.txt', shell=True)
   call('hd_root -PPLUGINS=FDC_InternalAlignment %s -PNTHREADS=%i -PEVENTS_TO_KEEP=%i' % (filename, nThreads, nEvents), shell=True)
   call(r'root -l -b -q $HALLD_HOME/src/plugins/Alignment/FDC_InternalAlignment/FitScripts/FitCathodeProjections.C\(\"hd_root.root\"\)',shell=True)
   call('ccdb add /FDC/cathode_alignment CathodeAlignment.txt', shell=True)
   call('ccdb add /FDC/strip_pitches_v2 StripPitchesV2.txt',shell=True)
   call('mv hd_root.root FDCInternalAlignIterations/hd_root_Projections_Iter%.2i.root' % iter, shell=True)
   call('mv ResultU.png FDCInternalAlignIterations/ResultU_Iter%.2i.png' % iter, shell=True)
   call('mv ResultV.png FDCInternalAlignIterations/ResultV_Iter%.2i.png' % iter, shell=True)

print "======= FINAL ITERATION ======="
call('hd_root -PPLUGINS=FDC_InternalAlignment %s -PNTHREADS=%i -PEVENTS_TO_KEEP=%i' % (filename, nThreads, nEvents), shell=True)
call('mv hd_root.root FDCInternalAlignIterations/hd_root_Planes_Iter%.2i.root' % nIterations, shell=True)
call(r'root -l -b -q $HALLD_HOME/src/plugins/Alignment/FDC_InternalAlignment/FitScripts/FitCathodeAlignment.C\(\"hd_root.root\"\)',shell=True)
call('ccdb add /FDC/cathode_alignment CathodeAlignment.txt', shell=True)
call('mv Result.png FDCInternalAlignIterations/Result_Iter%.2i.png' % nIterations, shell=True)
call(r'root -l -b -q $HALLD_HOME/src/plugins/Alignment/FDC_InternalAlignment/FitScripts/FitCathodeProjections.C\(\"hd_root.root\"\)',shell=True)
call('mv ResultU.png FDCInternalAlignIterations/ResultU_Iter%.2i.png' % nIterations, shell=True)
call('mv ResultV.png FDCInternalAlignIterations/ResultV_Iter%.2i.png' % nIterations, shell=True)


