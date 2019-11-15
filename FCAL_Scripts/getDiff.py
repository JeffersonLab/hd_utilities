import os

os.system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetHVStatus > HVValues.txt")
os.system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetHVSetPoint > SetPointValues.txt")

d1 = {}
d2 = {}
d3 = {}
i = 0;

with open('SetPointValues.txt') as f1:
     for line1 in f1:
         tok1 = line1.replace(":v0set"," ").split()
         d1[tok1[0]] = tok1[1]
Set_HV =  dict((k,float(v)) for k,v in d1.iteritems())


with open('HVValues.txt') as f2:
     for line2 in f2:
         tok2 = line2.replace(":vmon"," ").split()
         d2[tok2[0]] = tok2[1]         

Measured_HV =  dict((k,float(v)) for k,v in d2.iteritems())

for k,v in Set_HV.items():
     if (abs((Set_HV[k] - Measured_HV[k])/Set_HV[k]*100)) > 3:
#      if (abs(Set_HV[k] - Measured_HV[k])) > 30:
         i = i+1
         print k, v - Measured_HV[k]

print 'There are', i , 'channels with HV difference > 3%'     

