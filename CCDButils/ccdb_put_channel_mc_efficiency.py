"""
The channel_mc_efficiency list is read from ccdb into a file.
The file can be used to upload a new set of constants to ccdb

At the moment input selections are all specified in this file. The script could be modified to accept command line arguments.
"""
import os
import subprocess
import numpy
from matplotlib import pyplot as plt
from datetime import datetime, date, time, timedelta
from matplotlib.backends.backend_pdf import PdfPages

# bdate = "2015-08-10"
# edate = "2015-09-10"
system = "/BCAL"

in_variation = "mc"
out_variation = "mc"
# run1 = 0      # first run of validity
# run1 = 11366 
# run1 = 30274
run1 = 72316 
run2 = 72369     # last run of validity. 0 -> infinity
constant = "channel_mc_efficiency"

# initialize variables
effs = []


# create filename
filename = "ccdbfiles"+system+"-"+constant+"-"+str(run1)+".dat"
filename_out = "ccdbfiles"+system+"-"+constant+"-"+str(run1)+"_out.dat"

# generate file with constants

command =["ccdb","dump",system+"/"+constant,"-v ",in_variation," -r ",str(run1)]
print " command=", command
# print "ndx=",ndx,"file=",filenames[ndx]
fout = open(filename,"w")
subprocess.call(command,stdout=fout)
fout.close()

# now read file

fin = open(filename,"r")
entries = 0

for line in fin:
    linew = line.split()
    if linew[0].find("#") == 0:
        print "Comment dump:",line
        continue   # discard comment lines
    effs.append(float(linew[0]))
    if float(linew[0]) < 1: 
        print "entries=",entries,' eff=',linew[0]
        module = entries/32 + 1
        layer = (entries%32)/8 + 1
        sector = (entries%8)/2 + 1
        end = entries%2
        id = (module-1)*32 + (layer-1)*8 + (sector-1)*2 + end 
        print "entries=",entries," module=",module," layer=", layer," sector=",sector," end=",end," id=",id
    entries += 1


print "Number of entries=",entries," for ",fin  
fin.close()

# output new files:
print "Date:",datetime.now()

fout = open(filename_out,"w")
fout.write("#  ccdb_put_channel_mc_efficiency, Date: "+datetime.now().strftime("%Y-%m-%d %H:%M:%S")+", Run Range:{0:d}-{1:d}\n".format(run1,run2))

# Loop through efficiencies / modify as necessary and write out a file with entries
# id [0-1535]. Specify problematic channels and efficiencies

module = 37
layer = 1
sector = 4
end = 1
id = (module-1)*32 + (layer-1)*8 + (sector-1)*2 + end
effs[id] = 0

for eff in effs:
    # print "eff=",eff
    fout.write('{:5.2f}'.format(eff)+"\n")

fout.close()

command = ["printenv","JANA_CALIB_URL"]
print "print calib_url=",command
subprocess.call(command)

if run2 == 0:
    command =["ccdb","add",system+"/"+constant,"-v ",out_variation," -r ",str(run1)+"-",filename_out]
else:
    command =["ccdb","add",system+"/"+constant,"-v ",out_variation," -r ",str(run1)+"-"+str(run2),filename_out]

string = ' '.join(command)
print "Command=", string
# subprocess.call(command)




