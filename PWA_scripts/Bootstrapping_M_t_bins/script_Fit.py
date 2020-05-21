"""
python script to submit a job on IU farm
"""
import os
import glob
import subprocess
from subprocess import call
import fileinput
from datetime import datetime, date, time, timedelta
baseDir = "/w/halld-scifs17exp/Mariana/PWA_challenge/AmpTools/sample1_mass_t_bins_bootstrap_SPD"
fitName = "EtaPi_fit"
outputDir = baseDir + "/" + fitName
scriptDir = outputDir + "/script"
template = scriptDir + "/fit_TEMPLATE.csh"
template2 = scriptDir + "/fit_TEMPLATE.py"
project = "gluex"
track = "analysis"
workflow = "bootstrap_SPD"
# create list of run directory names
#loop over run directories 30279,30280,...
#for (runDir) in runDirList:
nMBins=45
ntBins=4
nBootstrapBins=100
disk_space = 25
mem_requested = 12
time_limit = 6
os.chdir(scriptDir)
cwd = os.getcwd()
call("pwd")

for bin in range(0,nMBins):
    for bint in range(0,ntBins):

        #Create a directory for each run
        outSubDir = (outputDir+"/bin_"+str(bin)+"_"+str(bint))

#create log and out file for each run
        logFile = (outSubDir + "/logfile_" + str(bin)+"_"+str(bint) + ".log")
        script = (scriptDir + "/bin_" + str(bin)+"_"+str(bint) + ".csh")
        fitFile = (scriptDir + "/fit_bin_" + str(bin)+"_"+str(bint)+".py")

        replacements={"BINFIT":"bin_"+str(bin)+"_"+str(bint), "SCRIPTDIR":scriptDir, "DIRECTORY":baseDir, "LOGFILE":logFile}
        with open(template) as infile, open(script, "w") as outfile:
            for line in infile:
                for src, target in replacements.iteritems():
                    line=line.replace(src,target)
                outfile.write(line)
            print script
    
        replacements={"BINFIT":str(bin)+"_"+str(bint), "BOOTSTRAPBINS":str(nBootstrapBins), "DIRECTORY":baseDir}
        with open(template2) as infile, open(fitFile, "w") as outfile:
            for line in infile:
                for src, target in replacements.iteritems():
                    line=line.replace(src,target)
                outfile.write(line)
            print fitFile

        command=["chmod", "777", scriptDir + "/bin_" + str(bin)+"_"+str(bint) + ".csh"]
        subprocess.call(command)
        cmd = "swif add-job -workflow %s -project %s -track %s"%(workflow,project,track)
        cmd += " -name %s_-%d_-%d"%(workflow, bin,bint)
        cmd += " -os centos77"
        cmd += " -stdout file:%s/log/log_%d_%d.log"%(scriptDir, bin,bint)
        cmd += " -stderr file:%s/log/err_%d_%d.err"%(scriptDir, bin,bint)
        cmd += " -disk %d"%int(disk_space)
        cmd += " -ram %d"%int(mem_requested)
        cmd += " -time %dhours"%int(time_limit)
        cmd += " " + scriptDir + "/bin_%d_%d.csh"%(bin,bint)
        call(cmd, shell=True, stdout=None)
