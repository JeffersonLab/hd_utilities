# This expects a command line argument of a directory containing monitoring files in directories named Run<run-number>
# eg /cache/halld/RunPeriod-2022-05/calib/ver01/hists"
# If the path starts with /cache, it checks /mss to see if all the files are present i cache and prints a message if the number of files differ. Use the argument 'nocheck' to skip this step.
# For each run it runs the script grabcdchistos.C on root files, extracts key CDC monitoring/calibration plots and merges the output to make one small file per run.

import sys
import os
import subprocess
import re

if len(sys.argv) != 2 and len(sys.argv) != 3:
  exit("Usage: sumcdchistos.py <calib_hists_directory> [nocheck]")

calibdir = sys.argv[1]
#calibdir="/cache/halld/RunPeriod-2022-05/calib/ver01/hists"

if not os.path.isdir(calibdir):
  exit("Directory " + calibdir + " not found")

if os.path.isdir("cdchistos"):
  exit("The directory cdchistos will be created to contain the output - please rename the existing cdchistos directory")


checktape = 0
mssdir=""    

if str.lower(sys.argv[len(sys.argv)-1]) == "nocheck" :
  checktape = 0
elif calibdir[0:7] == "/cache/":
  checktape = 1
  mssdir = "/mss/" + calibdir[7:]


dirlist = subprocess.check_output(["ls", calibdir],universal_newlines=True).splitlines()

if len(dirlist) == 0:
    exit

#    rundir="Run100907"

for rundir in dirlist:

    if rundir[0:3] != "Run":
        continue

    runnumber = rundir[3:9]

    if int(runnumber) > 100743:
        break

    thisdir = calibdir + "/" + rundir
    filelist = subprocess.check_output(["ls", thisdir],universal_newlines=True).splitlines()

    if len(filelist) == 0:
        exit

    if checktape == 1:

      tapedir = mssdir + "/" + rundir
      tapelist = subprocess.check_output(["ls", thisdir],universal_newlines=True).splitlines()

      if len(tapelist) != len(filelist):
        print("WARNING! Run "+ str(runnumber) + ": "+len(filelist)+" files on disk and " + len(tapelist) + " on tape")


    filecount=0

    # make the array of hadd arguments
    summedfile = "cdchistos/" + rundir + ".root"
    haddlist=["hadd","-k","-v","0",summedfile]
    smallfilelist=[]

    if not os.path.isdir("cdchistos") :
        os.mkdir("cdchistos")

    if not os.path.isdir(rundir) :
        os.mkdir(rundir)

    for runfile in filelist:

        scriptname = "grabcdchistos.C"

        # root -l -b -q runfile fit_dedx.C(run)
        subprocess.call(["root", "-l", "-b", "-q", thisdir + "/" + runfile,scriptname])

        if os.path.exists("cdchistos.root"):
           filecount = filecount + 1
           newname = rundir + "/cdc_" + str(filecount) + ".root"
           os.rename("cdchistos.root",newname)
           smallfilelist.append(newname)
           haddlist.append(newname)

    if filecount ==1:
        os.rename(newname,summedfile)

    if filecount > 1:

        returncode = subprocess.Popen(haddlist)
        returncode.wait()

        for eachfile in smallfilelist:
            os.remove(eachfile)

        dir = os.listdir(rundir)
        if len(dir) == 0:
            os.rmdir(rundir)
