import os
import sys
import datetime

print 'This will take about six minutes \n'
os.system("/gluonfs1/gluex/Subsystems/FCAL/FCAL_Analysis/MonitoringMisc/GetScaler > ScalerValues.txt")

d1={}
i = 0



threshold = int(sys.argv[1])

#if threshold > 0:
#     print 'Printing channels with threshold larger than', threshold, 'Hz \n'

with open('ScalerValues.txt') as f1:
     for line1 in f1:
         tok1 = line1.replace("FCAL:"," ").replace(":scaler_r1"," ").replace(":",",").split()
         d1[tok1[0]] = tok1[1]

Scaler =  dict((k,float(v)) for k,v in d1.iteritems())


outfile1 = '../MonitoringMisc/Scalers/scalers.txt'
f1 = open(outfile1,'w')

for k,v in Scaler.items():
     if v  > threshold:
         i = i+1
         #print >> f1 'Hot Channel location: ','(',k,')',  ' Scaler Rate (Hz): ', Scaler.get(k,0)
         print >> f1, k,'\t',Scaler.get(k,0)

print '\nThere are', i , 'channels with rate  > ', threshold, 'Hz'    

dt = str(datetime.datetime.now().strftime("%d_%m_%Y_%H_%M"))

thold = str(threshold)

newname1 = '../MonitoringMisc/Scalers/hotchannels_'+thold+'_'+dt+'.txt'
os.rename(outfile1, newname1)
