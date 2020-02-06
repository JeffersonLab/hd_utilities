#!/usr/bin/env python

from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
import glob

##################### RCDB ENVIRONMENT ####################
os.environ["RCDB_HOME"] = "/group/halld/www/halldweb/html/rcdb_home"
sys.path.append("/group/halld/www/halldweb/html/rcdb_home/python")
import rcdb
db = rcdb.RCDBProvider("mysql://rcdb@hallddb.jlab.org/rcdb")
#db = rcdb.RCDBProvider("sqlite:////group/halld/www/halldweb/html/dist/rcdb.sqlite")

##################### Launch pair of runs ####################
def launch_run(MyRunNum, MyInDataDir, MyOutDataDir):

    fileNameDiamond = '%shd_root_0%s.root' % (MyInDataDir, MyRunNum)
    #fileNameDiamond = '%shdmon_online0%s.root' % (MyInDataDir, MyRunNum)
    subprocess.call(["rm", "hist_diamond.root"])
    subprocess.call(["ln", "-s", fileNameDiamond, "hist_diamond.root"])

    # Ken's fitter (integrate over entire run)
    subprocess.call(["root.exe", "-l", "-b", "-q", "cobremFit.C+"])
    subprocess.call(["mkdir", "-p", MyOutDataDir])
    subprocess.call(["mv", "enhancementFit.pdf", "%s/enhancementFit_%s.pdf" % (MyOutDataDir,MyRunNum)])
    subprocess.call(["mv", "enhancementFit.root", "%s/enhancementFit_%s.root" % (MyOutDataDir,MyRunNum)])
    

##################### main function ####################
def main(argv):

    parser_usage = "loopRunsRCDB.py minrun maxrun"
    parser = OptionParser(usage = parser_usage)
    (options, args) = parser.parse_args(argv)
    
    if(len(args) != 2):
        parser.print_help()
        return
        
    # GET ARGUMENTS
    MINRUN = int(args[0])
    MAXRUN = int(args[1])

    # GET LISTS OF GOOD RUNS TO CHOOSE FROM
    RCDB_RUNS = db.select_runs("@is_dirc_production", run_min=MINRUN, run_max=MAXRUN+1)
    RCDB_RUN_NUMBERS = [ run.number for run in RCDB_RUNS ]
    print RCDB_RUN_NUMBERS

    # define paths for input and output
    MyInDataDir="/work/halld/data_monitoring/RunPeriod-2019-11/mon_ver01/rootfiles/"
    MyOutDataDir="polarization/spring2020/"

    for RCDB_RUN in RCDB_RUNS:
        RUN = RCDB_RUN.number
        print RUN

        # link new amorphous run
        if 'Al' in str(RCDB_RUN.get_condition('radiator_type')):
	    print "new amorphous %d" % RUN
            subprocess.call(["rm", "hist_amorph.root"])
            subprocess.call(["ln", "-s", "%s/hd_root_0%d.root" % (MyInDataDir,RUN), "hist_amorph.root"])
	    #subprocess.call(["ln", "-s", "%s/hdmon_online0%d.root" % (MyInDataDir,RUN), "hist_amorph.root"])
        else:
            launch_run(RUN, MyInDataDir, MyOutDataDir)

if __name__=="__main__":
    main(sys.argv[1:])
 
