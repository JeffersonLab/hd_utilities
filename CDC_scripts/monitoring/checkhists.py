
# This script expects to find the root files loose inside histdir, no extra subdirectories, and inspect_rootfile.C inside the current directory.
# It calls inspect_rootfile.C for each one, and concatenates the output into check.out

import sys
import os
import subprocess
import glob
#import ccdb
import re

if not os.path.exists('inspect_rootfile.C'):
  exit("Cannot find the root script inspect_rootfile.C - copy it from hd_utilities/CDC_scripts/monitoring to the working directory.")

script = sys.argv.pop(0)
nargs = len(sys.argv)

if nargs == 0 or nargs > 4 or sys.argv[0] == "-h" or sys.argv[0] == "--help":
  exit("Usage: checkhists.py path_to_histogram_directory [first_run_number last_run_number] [testing]")

histdir = sys.argv.pop(0)
#histdir = "/cache/halld/offline_monitoring/RunPeriod-2021-08/ver08/hists/hists_merged"   #path to find the root files
nargs -= 1

if not os.path.exists(histdir):
  exit("Cannot find the root files in " + histdir)


testing = 0  # process all files

if nargs > 0 and sys.argv[nargs-1] == "testing":
  testing = 1
  sys.argv.pop(nargs-1)
  nargs -=1


firstrun = 0         # 81472
lastrun = 1000000    # 81999

if nargs > 0 :
  try:
    firstrun = int(sys.argv[0])
  except ValueError:
    exit("The argument supplied for first_run_number is not an integer")

if nargs > 1 :
  try:
    lastrun = int(sys.argv[1])
  except ValueError:
    exit("The argument supplied for last_run_number is not an integer")


scriptname = "inspect_rootfile.C"

filelist = subprocess.check_output(["ls",histdir],universal_newlines=True).splitlines()

outfile = open("check.out","w")

for file in filelist:

    if os.path.splitext(file)[1] == '.root' :

        runfile = histdir + "/" + file
    
        findrunnum = re.findall('\d+',file)   #makes a list of numbers found in the filename
    
        if len(findrunnum) == 0:
            run = 0
            print('Cannot find run number in filename')
        else:
            run = int(findrunnum[0])
    
        #print 'run number ',run    
        
        if run < firstrun:
            continue

        if run > lastrun:
            break

        resultsfile = "_inspection.txt"       # inspect_rootfile writes into this file
        
        # root -l -q runfile inspect_rootfile.C 
        subprocess.call(["root", "-l", "-q", runfile,scriptname])
    
        # check whether the script completed & made a new file of calib consts
    
        if os.path.exists(resultsfile):
            infile = open(resultsfile)
            line=infile.readline()
            infile.close()
    
            output = str(run) + " " + line
            outfile.write(output)

            if not testing:
                os.remove(resultsfile)
            
        else:
            print('run ',run,' not processed')
    
    
        if testing:
            break
  
outfile.close()
