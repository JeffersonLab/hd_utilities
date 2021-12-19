#!/usr/bin/python


# this script add root files together for accidental scaling factor determintion
# it already takes into account the run periods from spring18 to spring19
#

import os,sys,getopt

RunList = {}

loc = './localdir/'

for d in os.listdir('./localdir/'):
    if d.startswith('Run'):
        rnum = d[3:8]
        rf = loc+d+'/TAGGER1_ps_test.root'
        if not os.path.isfile(rf):
            cmd = 'rm -rf '+loc+d
            os.system(cmd)
            continue
        fsize = os.path.getsize(rf)
        if fsize<100000:
            continue
        #print fsize
        if rnum in RunList:
            RunList[rnum] += 1
        else:
            RunList[rnum] = 1


dest = loc+'spring19/'
for r in RunList:

    if RunList[r]<1:
        continue
        
    if ( (int(r)>69999) and (int(r)<80000)):
        dest = loc+'fall19/'
    if ( (int(r)>59999) and (int(r)<70000)):
        dest = loc+'spring19/'
    if ( (int(r)>49999) and (int(r)<60000)):
        dest = loc+'fall18/'
    if ( (int(r)>39999) and (int(r)<50000)):
        dest = loc+'spring18/'
    if ( (int(r)>29999) and (int(r)<40000)):
        dest = loc+'spring17/'
    if ( (int(r)>19999) and (int(r)<30000)):
        dest = loc+'fall16/'
    if ( (int(r)>9999) and (int(r)<20000)):
        dest = loc+'spring16/'

    of = dest+'pstreeresults_run'+r+'.root'
    #if os.path.isfile(of):
        #cmd = 'rm -f '+of
        #os.system(cmd)
        #continue

    cmd = 'hadd -f '+of+'  '+loc+'Run'+r+'_*/TAGGER1_ps_test.root'
    print cmd
    os.system(cmd)

    of1 = dest+'hd_root_run'+r+'.root'
    cmd = 'hadd -f '+of1+'  '+loc+'Run'+r+'_*/hd_root.root'
    os.system(cmd)
#    sys.exit()
