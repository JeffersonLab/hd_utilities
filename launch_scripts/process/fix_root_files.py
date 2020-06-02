import os

for root, dirs, files in os.walk('/cache/halld/offline_monitoring/RunPeriod-2016-02/ver02/ROOT'):
    for fname in files:
        #print fname[-4:]
        if fname[-4:] == ".old":
            #print "mv %s %s"%(os.path.join(root,fname), os.path.join(root,fname[:-4]))
            os.system("mv %s %s"%(os.path.join(root,fname), os.path.join(root,fname[:-4])))

        #data = [parse_file(os.path.join(root,f)) for f in files]
