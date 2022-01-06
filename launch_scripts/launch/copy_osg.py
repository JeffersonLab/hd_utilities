#~/usr/bin/env python3

import os,sys
import glob

DONT_RENAME = True

# read some of this in from configuration file?
SRCDIR   = sys.argv[1]
HOSTNAME = sys.argv[2]
DESTDIR  = sys.argv[3]

# rename files first and keep some lists
runlist = set()
rootfilelist = set()

output_directories = glob.glob(SRCDIR+'/out_??????_???')
#print(output_directories)

for dirpath in output_directories:
    try:
        dirname = dirpath.split('/')[-1]
        runno = dirname[4:10]
        fileno = dirname[-3:]
        print(runno,fileno)  # DEBUG
    except:
        print("couldn't process file: "+dirpath)
        continue

    runlist.add(runno)

    rootfiles = glob.glob(dirpath+"/*.root")
    for rootfile in rootfiles:
        if not DONT_RENAME:
            destfile = rootfile[:-5] + "_%s_%s.root"%(runno,fileno)
            rootfilelist.add(rootfile.split('/')[-1][:-5])
            cmd = "mv %s %s"%(rootfile,destfile)
            os.system(cmd)
        else:
            rootfilelist.add(rootfile.split('/')[-1][:-16])


print(runlist)
print(rootfilelist)

for rootfiletype in rootfilelist:
    for runno in runlist:
        destdir = "%s/%s/%s"%(DESTDIR,rootfiletype,runno)
        srcglob = "%s/out_%s_*/./%s_*.root"%(SRCDIR,runno,rootfiletype)
        cmd = "rsync --progress --rsync-path=\"mkdir -p %s && rsync\" -avux %s %s:%s"%(destdir,srcglob,HOSTNAME,destdir)
        print(cmd)
        os.system(cmd)
        
