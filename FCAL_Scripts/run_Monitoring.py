import csv
import datetime
import os

# Input file with two tab separated columns; old CAN ID in column 1 and new CAN ID in column 2
#infile1 = open('CAN_ID_20_08_2018.txt', 'r')
#replacementfile = csv.reader(infile1, delimiter='\t')

#lines1 = []

#for line in replacementfile:
#    lines1.append(line)
    
#old_can_id = [line[0] for line in lines1]
#new_can_id = [line[1] for line in lines1]    

#candict  = dict(zip(old_can_id, new_can_id))

# Latest db txt file. 
#infile = 'FCAL_DB3_FullFCAL_16_08_2018.txt'

dt = str(datetime.datetime.now().strftime("%d_%m_%Y"))
# Create new db txt file
#outfile = 'FCAL_DB3_FullFCAL_'+dt+'.txt'
#db_text_file = outfile
#outfile = open(outfile,'w')
#print 'Replacing CANIDs of',len(candict),'bases'
#with open(infile) as infile:
#    for line in infile:
#        for src, target in candict.iteritems():
#            line = line.replace(src, target)
#        outfile.write(line)

# Create sqlite db
#dbfile = outfile = 'FCAL_DB3_FullFCAL_'+dt+'.db'
cmd = 'root -l -b setup.C'
#print 'Now creating sqlite db'
print cmd
os.system(cmd)
string='gROOT->ProcessLine(".L MonitoringToRoot_Colin2.C++")'
command=[string]
subprocess.call(command)
os.system(cmd)
print 'Done!'
