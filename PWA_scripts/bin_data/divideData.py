#!/usr/bin/env python

# Alex Austregesilo
# aaustreg@jlab.org

import sys
import os
import subprocess
import math
from optparse import OptionParser
import ROOT

#########################################################################################################################

def getEntries(ROOTFILE):
        f = ROOT.TFile(ROOTFILE)
        t = f.Get("kin")
        return t.GetEntries()
        


########################################################## MAIN ##########################################################

def main(argv):
        global VERBOSE # so can modify here

        lowT = 0.0
        highT = 1.00
        nBins = 10
        fitName = "name_your_directory"

        workingDir = os.getcwd()
        
        genMCFile = workingDir + "sample_thrown.root"
        accMCFile = workingDir + "sample_reco.root"
        dataFile = workingDir + "sample_signal.root"
        bkgndFile = workingDir + "sample_bkgnd.root"

        cfgTempl = workingDir + "gen_template.cfg"

        # this is where the goodies for the fit will end up
        fitDir = workingDir + "/" + fitName
        os.mkdir(fitDir)
        os.chdir(fitDir)

        split = "split_t " + dataFile + " " + fitName + "_data " + str(lowT) + " " + str(highT) + " " + str(nBins)
        print "Splitting data file"
        os.system(split)

        split = "split_t " + bkgndFile + " " + fitName + "_bkgnd " + str(lowT) + " " + str(highT) + " " + str(nBins)
        print "Splitting accidental file"
        os.system(split)

        split = "split_t " + accMCFile + " " + fitName + "_acc " + str(lowT) + " " + str(highT) + " " + str(nBins)
        print "Splitting accepted MC file"
        os.system(split)

        split = "split_t " + genMCFile + " " + fitName + "_gen " + str(lowT) + " " + str(highT) + " " + str(nBins)
        print "Splitting generated MC file"
        os.system(split)

        # make directories to perform the fits in
        for bin in range(0,nBins):

                bin_name = "bin_" + str(bin)
                os.mkdir(bin_name)
                
                mv_command = "mv " + "*_" + str(bin) + ".root " + bin_name
                os.system(mv_command)

                os.chdir(bin_name)

                genFile = fitName+"_gen_"+str(bin)+".root"
                accFile = fitName+"_acc_"+str(bin)+".root"
                dataFile = fitName+"_data_"+str(bin)+".root"
                bkgndFile = fitName+"_bkgnd_"+str(bin)+".root"

                # calculate initial guess for amplitude
                Ngen = getEntries(genFile)
                Nacc = getEntries(accFile)
                Ndata = getEntries(dataFile)
                amplitude = round(math.sqrt(Ndata*Ngen/Nacc) * math.pi, 2)

                f = open(cfgTempl,'r')
                filedata = f.read()
                f.close()

                filedata = filedata.replace("GENMCFILE", genFile)
                filedata = filedata.replace("ACCMCFILE", accFile)
                filedata = filedata.replace("DATAFILE", dataFile)
                filedata = filedata.replace("BKGNDFILE", bkgndFile)

                filedata = filedata.replace("FITNAME", bin_name)
                filedata = filedata.replace("NIFILE", bin_name+".ni")

                filedata = filedata.replace("AMPLITUDE", str(amplitude))
                
                cfgBin = bin_name + ".cfg"
                f = open(cfgBin,'w')
                f.write(filedata)
                f.close()

                f = open("param_init.cfg",'a')
                f.close()

                os.chdir("..")



if __name__ == "__main__":
   main(sys.argv[1:])
