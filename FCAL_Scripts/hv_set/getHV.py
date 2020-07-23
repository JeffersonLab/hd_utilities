#!/usr/bin/env python
import os
import csv
import math
from collections import OrderedDict
from  matplotlib import pyplot as plt
import numpy as np
from pylab import *

oldHV = OrderedDict()
gains = OrderedDict()
fitval_p1 = OrderedDict()
old_HV = []
newHV = []
diff_HV = []
chans = []
chan_x = []
chan_y = []
num = 0.0
nume = 0.0

#Open and read old burt file used for the FCAL. Replace it with the latest one on gluonxx 
with open('FCAL_HV_20_Mar_2019_10_14_08.snap') as f1:
     for line1 in f1:
         if line1.startswith("FCAL:"):
             tok1 = line1.replace("FCAL:hv:"," ").replace(":v0set"," ").replace(":","\t").split()
             oldHV[str((int(tok1[0]),int(tok1[1])) )] = float ( tok1[3] )
             chan_x.append(tok1[0])
             chan_y.append(tok1[1])
             old_HV.append(float(tok1[3]))

# Open and read gain calibration file. File format is gain, X, Y separated by tabs             
with open('gains_new.txt') as f2:
     for line2 in f2:
         tok2 = line2.split()
         gains[str( (int(tok2[1]),int(tok2[2])) )] = float ( tok2[0] )
         print ("line 2=",line2," tok2=",tok2)

#print "gains=",gains," tok2=",tok2
# Open and read fit values from LED data
with open('fit_values.txt') as f3:
     for line3 in f3:
         tok3 = line3.split()
         fitval_p1[str( (int(tok3[3]),int(tok3[4])) )] = float ( tok3[2] )
#print "line 3=",line3," tok3=",tok3
         
# Get new HV based on gain constants, old HV & the fit values from LED data
for key,value in oldHV.items():  
      newHV.append( float (math.log(gains[key]) / fitval_p1[key] + oldHV[key]))

# Write new burt file for the FCAL HV. Replace text where applicable
myfile = open('ProductionHV_Fall2019.snap', 'w')
myfile.write("--- Start BURT header\n")
myfile.write("Time:     Tue Nov 21  16:33:30 2016 \n")
myfile.write("Login ID: gleason (Colin Gleason) \n")
myfile.write("Eff  UID: 8178 \n")
myfile.write("Group ID: 765 \n")
myfile.write("Keywords: \n")
myfile.write("Comments: \n")
myfile.write("Type:     Absolute \n")
myfile.write("Directory /gluonfs1/gluex/controls/epics/R3-14-12-3-1/app/scripts/VoltageBURT \n")
myfile.write("Req File: REQS//fcal_hv_all.req \n")
myfile.write("--- End BURT header\n")

# Check if the V is within 1500-1800V range. If not set the values accordingly
for i in range(0,len(newHV)):
     if newHV[i] > 1800:
          #print "HV > 1800 V", newHV[i], "Setting to 1800 V"
          newHV[i] = 1800.00
          #print newHV[i]
     if newHV[i] < 1500:
          #print "HV < 1500 V", newHV[i], "Setting to 1500 V"
          newHV[i] = 1500.00
     diff_HV.append( newHV[i] - old_HV[i] )
     #print "FCAL:hv:%d:%d:v0set 1 %f" % (int(chan_x[i]),int(chan_y[i]),float(newHV[i]))
     cmdstring = "FCAL:hv:%d:%d:v0set 1 %f \n" % (int(chan_x[i]),int(chan_y[i]),float(newHV[i]))
     myfile.write(cmdstring)
myfile.close()          
#print newHV

