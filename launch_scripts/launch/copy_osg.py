#!/usr/bin/env python3
#
# script to copy OSG launch output that is formatted in one directory per file to a lustre disk with the same
# format as a launch on the JLab farm
#
# example of how to run this script:
#   python3 copy_osg.py /osgpool/halld/sdobbs/hd_utilities/launch_scripts/launch sdobbs@dtn1902-ib /lustre19/expphy/volatile/halld/analysis/RunPeriod-2017-01/ver88

import os,sys
import glob

DONT_RENAME = False

# read some of this in from configuration file?
SRCDIR   = sys.argv[1]
HOSTNAME = sys.argv[2]
DESTDIR  = sys.argv[3]

# rename files first and keep some lists
runlist = set()
run_file_num_map = {}
rootfilelist = set()

output_directories = glob.glob(SRCDIR+'/out_??????_???')
#print(output_directories)

for dirpath in output_directories:
    try:
        dirname = dirpath.split('/')[-1]
        runno = dirname[4:10]
        fileno = dirname[-3:]
        #print(runno,fileno)  # DEBUG
    except:
        print("couldn't process file: "+dirpath)
        continue

    rootfiles = glob.glob(dirpath+"/*.root")
    if(len(rootfiles) == 0):
        continue

    runlist.add(runno)
    if runno not in run_file_num_map.keys():
        run_file_num_map[runno] = []
    run_file_num_map[runno].append(fileno)

    for rootfile in rootfiles:
        if not DONT_RENAME:
            # check to see if we should actually rename
            tag = "%s_%s"%(runno,fileno)
            #print("compare ",tag,rootfile[-15:-5])
            #continue
            if(tag == rootfile[-15:-5]):
                continue

            destfile = rootfile[:-5] + "_%s_%s.root"%(runno,fileno)
            rootfilelist.add(rootfile.split('/')[-1][:-5])
            cmd = "mv %s %s"%(rootfile,destfile)
            os.system(cmd)
        else:
            rootfilelist.add(rootfile.split('/')[-1][:-16])
            #print(rootfile.split('/')[-1])
            #print(rootfile.split('/')[-1][:-16])
            #exit(0)

print(runlist)
print(rootfilelist)
#exit(0)

for rootfiletype in rootfilelist:
    for runno in runlist:
        if rootfiletype == "hd_root":
            rootfiletype = "hists"
        destdir = "%s/%s/%s"%(DESTDIR,rootfiletype,runno)
        srcglob = "%s/out_%s_*/./%s_*.root"%(SRCDIR,runno,rootfiletype)
        cmd = "rsync --remove-source-files --progress --rsync-path=\"mkdir -p %s && rsync\" -avux %s %s:%s"%(destdir,srcglob,HOSTNAME,destdir)
        print(cmd)
        os.system(cmd)
