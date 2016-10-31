import os
import phadd

BASEDIR = "/cache/halld/RunPeriod-2016-02/analysis/ver04/hists"
merged_dir = "%s/%s"%(BASEDIR,"merged")

root_file_dirs = [ d for d in os.listdir(BASEDIR) if len(d)==6 ]

os.system("mkdir -p "+merged_dir)

for rundir in root_file_dirs:
    try:
        run = int(rundir)
    except:
        continue
    print "merging directory %s ..."%rundir
    merged_file = "%s/hd_root_%s.root"%(merged_dir,rundir)
    if os.path.isfile(merged_file):
        print "  skipping ..."
        continue
    root_files = [ BASEDIR+"/"+rundir+"/"+f for f in os.listdir(BASEDIR+"/"+rundir) if f[-5:]==".root" ]
    #print str(root_files)
    #exit(0)
    #hadder = phadd.phadd(merged_file, sorted(root_files), " -k -v 0 ", 6, 10)
    hadder = phadd.phadd(merged_file, sorted(root_files), " -k ", 6, 10)
    hadder.Add()
