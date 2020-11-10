#!/usr/bin/env python

# Alex Austregesilo
# aaustreg@jlab.org

import sys
import os
import subprocess
import math
import random 
from optparse import OptionParser
import ROOT

#########################################################################################################################



########################################################## MAIN ##########################################################

def main(argv):
        global VERBOSE # so can modify here

	nBootstrapBins = BOOTSTRAPBINS
        fitName = "EtaPi_fit"

        #workingDir = "/home/fs11/colgleas/pippimeta/AmpAnalysis/BootStrap/data/FlatG3_Randoms"
        workingDir = "DIRECTORY"
        # this is where the goodies for the fit will end up
        fitDir = workingDir + "/" + fitName
	#fitaDir = FITDIR
        seedFile = "param_init.cfg"
        
        for bin in range(0,nBootstrapBins):
                print "Fitting bs bin "+str(bin)
                fit_command = "fit " + "-c " + "bin_" + str(bin) + "/bin_" + str(bin) + ".cfg" + " -s " + seedFile + " > " + "bin_" + str(bin) + ".log"
                os.system(fit_command)
		#print "Fitting in bin " + str(bin) + ", bootstrap bin " + str(bin2)
		mv_command = "mv bin_BINFIT.fit bin_bs_" + str(bin) + ".fit"
		os.system(mv_command)
if __name__ == "__main__":
   main(sys.argv[1:])
