#!/usr/bin/env python

# Alex Austregesilo, Colin Gleason
# aaustreg@jlab.org, gleasonc@jlab.org

import sys
import os
import subprocess
import math
import random 
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
        
        lowMass = 0.7
        highMass = 3.00
        nBins = 45
        lowt = 0
        hight = 1.2
        ntBins = 4
        nBootstrapBins = 100
        fitName = "EtaPi_fit"

        workingDir = os.getcwd()
        
        genMCFile = workingDir + "/flat_newhalldsim.root"
        accMCFile = workingDir + "/flat_newhalldsim.root"
        dataFile = workingDir + "/data_newhalldsim_SPD.root"

        cfgTempl = workingDir + "/fit_etapi_TEMPLATE.cfg"

        # this is where the goodies for the fit will end up
        fitDir = workingDir + "/" + fitName
        os.mkdir(fitDir)
        os.chdir(fitDir)
        os.mkdir("script")
	os.mkdir("script/log")
        
        split = "split_mass_t " + dataFile + " " + fitName + "_data " + str(lowMass) + " " + str(highMass) + " " + str(nBins)+" "+ str(lowt) + " " + str(hight) + " " + str(ntBins)
        print(split)
        print "Splitting data file"
        os.system(split)


        split = "split_mass_t " + accMCFile + " " + fitName + "_acc " + str(lowMass) + " " + str(highMass) + " " + str(nBins)+" "+ str(lowt) + " " + str(hight) + " " + str(ntBins)

        print "Splitting accepted MC file"
        os.system(split)

        split = "split_mass_t " + genMCFile + " " + fitName + "_gen " + str(lowMass) + " " + str(highMass) + " " + str(nBins)+" "+ str(lowt) + " " + str(hight) + " " + str(ntBins)

        print "Splitting generated MC file"
        os.system(split)

        # make directories to perform the fits in
        for bin in range(0,nBins):

                for t_bin in range(0,ntBins):
                        bin_name = "bin_" + str(bin)+"_"+str(t_bin)
                        os.mkdir(bin_name)
                
                        mv_command = "mv " + "*_" + str(bin) +"_"+str(t_bin) + ".root " + bin_name
                        os.system(mv_command)

                        os.chdir(bin_name)
                        print "Splitting data file" + str(bin) 
	
                        genFile = fitName+"_gen_"+str(bin)+"_"+str(t_bin)+".root"
                        accFile = fitName+"_acc_"+str(bin)+"_"+str(t_bin)+".root"
                        dataFile = fitName+"_data_"+str(bin)+"_"+str(t_bin)+".root"
                        #bkgndFile = fitName+"_bkgnd_"+str(bin)+".root"

                        # calculate initial guess for amplitude
                        Ngen = getEntries(genFile)
                        Nacc = getEntries(accFile)
                        Ndata = getEntries(dataFile)
                        amplitude = round(math.sqrt(Ndata*Ngen/(1+Nacc)) * math.pi, 2)

                        f = open(cfgTempl,'r')
                        filedata = f.read()
                        f.close()

                        filedata = filedata.replace("GENMCFILE", genFile)
                        filedata = filedata.replace("ACCMCFILE", accFile)
                        filedata = filedata.replace("DATAFILE", dataFile)
                        #filedata = filedata.replace("BKGNDFILE", bkgndFile)

                        filedata = filedata.replace("FITNAME", bin_name)
                        filedata = filedata.replace("NIFILE", bin_name+".ni")

                        filedata = filedata.replace("AMPLITUDE", str(amplitude))
                
                        cfgBin = bin_name + ".cfg"
                        f = open(cfgBin,'w')
                        f.write(filedata)
                        f.close()

                        f = open("param_init.cfg",'a')
                        f.close()

                        cfgTempl2 = workingDir + "/EtaPi_fit/" + bin_name + "/" + cfgBin	
                        for bin2 in range(0,nBootstrapBins):
                                bin_name = "bin_" + str(bin2)
                                os.mkdir(bin_name)
			
                                mv_command = "cp " + "*.cfg " + bin_name
                                os.system(mv_command)

                                os.chdir(bin_name)
                                #print "Splitting config file" + str(bin2)
                                f = open(cfgTempl2,'r')
                                filedata = f.read()
                                f.close()
				
                                randomNum = random.randint(0,1000)
                                filedata = filedata.replace("SEED", str(randomNum))
                                cfgBin = bin_name + ".cfg"
                                f = open(cfgBin,'w')
                                f.write(filedata)
                                f.close()

                                os.chdir("..")
                        os.chdir("..")
        
        seedFile = "param_init.cfg"
        
		

if __name__ == "__main__":
   main(sys.argv[1:])
